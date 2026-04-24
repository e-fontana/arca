#include "pn532.h"
#include <string.h>

/* ── Instância SPI ──────────────────────────────────────────────────────── */
static SPI_HandleTypeDef *_hspi = NULL;

/* ── Flag de interrupção (setada na ISR, consumida no loop) ─────────────── */
volatile uint8_t pn532_card_ready = 0;

/* ════════════════════════════════════════════════════════════════════════
   Helpers de baixo nível
   ════════════════════════════════════════════════════════════════════════ */

static inline void _cs_low(void) {
    HAL_GPIO_WritePin(PN532_NSS_PORT, PN532_NSS_PIN, GPIO_PIN_RESET);
}

static inline void _cs_high(void) {
    HAL_GPIO_WritePin(PN532_NSS_PORT, PN532_NSS_PIN, GPIO_PIN_SET);
}

/* PN532 exige LSB-first — invertemos manualmente pois o SPI está em MSB */
static uint8_t _reverse_bits(uint8_t b) {
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) {
        r = (r << 1) | (b & 1);
        b >>= 1;
    }
    return r;
}

static void _spi_write_byte(uint8_t b) {
    uint8_t rb = _reverse_bits(b);
    HAL_SPI_Transmit(_hspi, &rb, 1, HAL_MAX_DELAY);
}

static uint8_t _spi_read_byte(void) {
    uint8_t tx = 0x00, rx = 0x00;
    HAL_SPI_TransmitReceive(_hspi, &tx, &rx, 1, HAL_MAX_DELAY);
    return _reverse_bits(rx);
}

/* ════════════════════════════════════════════════════════════════════════
   Protocolo PN532
   ════════════════════════════════════════════════════════════════════════ */

static uint8_t _wait_ready(uint32_t timeout_ms) {
    uint32_t t = HAL_GetTick();
    while (HAL_GPIO_ReadPin(PN532_IRQ_PORT, PN532_IRQ_PIN) == GPIO_PIN_SET) {
        if ((HAL_GetTick() - t) > timeout_ms) return 0;
        HAL_Delay(1);
    }
    return 1;
}

static void _send_frame(const uint8_t *data, uint8_t len) {
    uint8_t lcs = (uint8_t)(~(len + 1) + 1);
    uint8_t dcs = PN532_HOSTTOPN532;
    for (int i = 0; i < len; i++) dcs += data[i];
    dcs = (uint8_t)(~dcs + 1);

    _cs_low();
    HAL_Delay(2);

    _spi_write_byte(PN532_SPI_DATAWRITE);
    _spi_write_byte(PN532_PREAMBLE);
    _spi_write_byte(PN532_STARTCODE1);
    _spi_write_byte(PN532_STARTCODE2);
    _spi_write_byte((uint8_t)(len + 1));
    _spi_write_byte(lcs);
    _spi_write_byte(PN532_HOSTTOPN532);
    for (int i = 0; i < len; i++) _spi_write_byte(data[i]);
    _spi_write_byte(dcs);
    _spi_write_byte(PN532_POSTAMBLE);

    _cs_high();
}

static uint8_t _read_ack(void) {
    const uint8_t expected[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    uint8_t buf[6] = {0};

    if (!_wait_ready(100)) return 0;

    _cs_low();
    HAL_Delay(2);
    _spi_write_byte(PN532_SPI_DATAREAD);
    for (int i = 0; i < 6; i++) buf[i] = _spi_read_byte();
    _cs_high();

    return (memcmp(buf, expected, 6) == 0) ? 1 : 0;
}

static uint8_t _read_response(uint8_t *response, uint8_t max_len) {
    if (!_wait_ready(500)) return 0;

    _cs_low();
    HAL_Delay(2);
    _spi_write_byte(PN532_SPI_DATAREAD);

    uint8_t b;
    uint32_t t = HAL_GetTick();
    while (1) {
        b = _spi_read_byte();
        if (b == PN532_STARTCODE2) break;
        if ((HAL_GetTick() - t) > 200) { _cs_high(); return 0; }
    }

    uint8_t len = _spi_read_byte();
    uint8_t lcs = _spi_read_byte();
    if ((uint8_t)(len + lcs) != 0x00) { _cs_high(); return 0; }

    uint8_t tfi = _spi_read_byte();
    uint8_t cmd = _spi_read_byte();
    (void)tfi; (void)cmd;

    uint8_t data_len = len - 2;
    if (data_len > max_len) data_len = max_len;
    for (int i = 0; i < data_len; i++) response[i] = _spi_read_byte();

    _spi_read_byte(); /* DCS */
    _spi_read_byte(); /* Postamble */
    _cs_high();

    return data_len;
}

/* ════════════════════════════════════════════════════════════════════════
   API pública
   ════════════════════════════════════════════════════════════════════════ */

void PN532_Init(SPI_HandleTypeDef *hspi) {
    _hspi = hspi;
    _cs_high();

    /* Pulso de reset */
    HAL_GPIO_WritePin(PN532_RST_PORT, PN532_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PN532_RST_PORT, PN532_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
}

uint8_t PN532_GetFirmwareVersion(uint8_t *ver) {
    uint8_t cmd[1] = { PN532_CMD_GETFIRMWAREVERSION };
    uint8_t resp[4] = {0};

    _send_frame(cmd, 1);
    if (!_read_ack()) return 0;
    if (_read_response(resp, 4) < 4) return 0;

    if (ver) memcpy(ver, resp, 4);
    return 1;
}

uint8_t PN532_ReadPassiveTarget(PN532_Card_t *card) {
    uint8_t cmd[3] = {
        PN532_CMD_INLISTPASSIVETARGET,
        0x01,
        PN532_MIFARE_ISO14443A
    };
    uint8_t resp[20] = {0};

    memset(card, 0, sizeof(PN532_Card_t));

    _send_frame(cmd, 3);
    if (!_read_ack()) return 0;

    uint8_t n = _read_response(resp, sizeof(resp));
    if (n < 1) return 0;
    if (resp[0] == 0) return 0;

    card->atqa    = ((uint16_t)resp[3] << 8) | resp[2];
    card->sak     = resp[4];
    card->uid_len = resp[5];
    if (card->uid_len > PN532_MAX_UID_LEN) card->uid_len = PN532_MAX_UID_LEN;
    memcpy(card->uid, &resp[6], card->uid_len);
    card->valid   = 1;

    return 1;
}

void PN532_IRQ_Handler(void) {
    pn532_card_ready = 1;
}