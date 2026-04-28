#include "nfc.h"
#include <string.h>

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>

static SPI_HandleTypeDef *_hspi = NULL;
volatile uint8_t pn532_card_ready = 0u;

static uint8_t _reverse_bits(uint8_t b)
{
    b = (b & 0xF0u) >> 4u | (b & 0x0Fu) << 4u;
    b = (b & 0xCCu) >> 2u | (b & 0x33u) << 2u;
    b = (b & 0xAAu) >> 1u | (b & 0x55u) << 1u;
    return b;
}

static void _cs_low(void)
{
    HAL_GPIO_WritePin(NSS_NFC_GPIO_Port, NSS_NFC_Pin, GPIO_PIN_RESET);
    HAL_Delay(2);
}

static void _cs_high(void)
{
    HAL_GPIO_WritePin(NSS_NFC_GPIO_Port, NSS_NFC_Pin, GPIO_PIN_SET);
}

static void _spi_write_byte(uint8_t byte)
{
    uint8_t tx = _reverse_bits(byte);
    HAL_SPI_Transmit(_hspi, &tx, 1, PN532_SPI_TIMEOUT);
}

static uint8_t _spi_read_byte(void)
{
    uint8_t rx = 0x00u;
    uint8_t tx = _reverse_bits(0x00u);
    HAL_SPI_TransmitReceive(_hspi, &tx, &rx, 1, PN532_SPI_TIMEOUT);
    return _reverse_bits(rx);
}

static uint8_t _wait_ready(uint32_t timeout_ms)
{
    uint32_t t_start = HAL_GetTick();

    while (HAL_GPIO_ReadPin(INT_NFC_GPIO_Port, INT_NFC_Pin) == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - t_start) > timeout_ms)
        {
            return 0u;
        }
        HAL_Delay(1);
    }

    return 1u;
}

static void _send_frame(uint8_t cmd, const uint8_t *data, uint8_t dlen)
{
    uint8_t len = 1u + 1u + dlen;
    uint8_t lcs = (uint8_t)(~len + 1u);

    uint8_t dcs = PN532_TFI_HOST_TO_PN + cmd;
    for (uint8_t i = 0u; i < dlen; i++)
    {
        dcs += data[i];
    }
    dcs = (uint8_t)(~dcs + 1u);

    _cs_low();

    _spi_write_byte(PN532_SPI_DATAWRITE);
    _spi_write_byte(0x00u);
    _spi_write_byte(0x00u);
    _spi_write_byte(0xFFu);
    _spi_write_byte(len);
    _spi_write_byte(lcs);
    _spi_write_byte(PN532_TFI_HOST_TO_PN);
    _spi_write_byte(cmd);

    for (uint8_t i = 0u; i < dlen; i++)
    {
        _spi_write_byte(data[i]);
    }

    _spi_write_byte(dcs);
    _spi_write_byte(0x00u);

    _cs_high();
}

static uint8_t _read_ack(void)
{
    static const uint8_t ack_ref[6] = { 0x00u, 0x00u, 0xFFu, 0x00u, 0xFFu, 0x00u };

    uint8_t buff[6] = { 0u };

    if (!_wait_ready(PN532_ACK_TIMEOUT))
    {
        return 0u;
    }

    _cs_low();

    _spi_write_byte(PN532_SPI_DATAREAD);
    for (uint8_t i = 0; i < 6u; i++)
    {
        buff[i] = _spi_read_byte();
    }

    _cs_high();

    return (memcmp(buff, ack_ref, 6u) == 0u) ? 1u : 0u;
}

static uint8_t _read_response(uint8_t *buf, uint8_t buf_size, uint8_t *out_len)
{
    if (!_wait_ready(PN532_RESP_TIMEOUT))
    {
        return 0u;
    }

    _cs_low();
    _spi_write_byte(PN532_SPI_DATAREAD);

    _spi_read_byte(); /* 0x00 - Preâmbulo */
    _spi_read_byte(); /* 0x00 - Start Code 1 */
    _spi_read_byte(); /* 0xFF - Start Code 2 */

    uint8_t len = _spi_read_byte();
    uint8_t lcs = _spi_read_byte();

    if ((uint8_t)(len + lcs) != 0x00u)
    {
        _cs_high();
        return 0u;
    }

    uint8_t tfi = _spi_read_byte();
    _spi_read_byte(); /* CMD+1 - descartado */

    if (tfi != PN532_TFI_PN_TO_HOST)
    {
        _cs_high();
        return 0u;
    }

    uint8_t dlen = len - 2u;
    uint8_t to_read = (dlen > buf_size) ? buf_size : dlen;

    for (uint8_t i = 0u; i < to_read; i++)
    {
        buf[i] = _spi_read_byte();
    }

    /* descarta todos os bytes excedentes para manter o frame sincronizado */
    for (uint8_t i = to_read; i < dlen; i++)
    {
        _spi_read_byte();
    }

    _spi_read_byte(); /* DCS */
    _spi_read_byte(); /* 0x00 - Postâmbulo */

    _cs_high();

    *out_len = dlen;
    return 1u;
}

static void _set_led(uint8_t state)
{
    #if defined (NFC_LED_STATUS_Pin) && defined (NFC_LED_STATUS_GPIO_Port)
        #if PN532_LED_STATUS_ACTIVE_LOW
            HAL_GPIO_WritePin(NFC_LED_STATUS_GPIO_Port, NFC_LED_STATUS_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
        #else
            HAL_GPIO_WritePin(NFC_LED_STATUS_GPIO_Port, NFC_LED_STATUS_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        #endif
    #endif
}

uint8_t NFC_GetFirmwareVersion(void)
{
    uint8_t resp[4] = { 0u };
    uint8_t resp_len = 0u;

    uint8_t dummy[] = {0};
    _send_frame(PN532_CMD_GETFIRMWAREVERSION, dummy, 0u);
    
    if (!_read_ack()) return 0u;
    if (!_read_response(resp, sizeof(resp), &resp_len)) return 0u;

    return 1u;
}

void NFC_Init(SPI_HandleTypeDef *hspi)
{
    hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    HAL_SPI_Init(hspi);

    _hspi = hspi;
    _set_led(0u);

    HAL_GPIO_WritePin(NFC_RESET_GPIO_Port, NFC_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(NFC_RESET_GPIO_Port, NFC_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

    _cs_high();
    HAL_Delay(10);

    static const uint8_t sam_data[] = { 0x01u, 0x00u };
    _send_frame(PN532_CMD_SAMCONFIGURATION, sam_data, sizeof(sam_data));
    _read_ack();

    uint8_t resp[PN532_RESP_BUF_SIZE];
    uint8_t resp_len = 0u;
    _read_response(resp, sizeof(resp), &resp_len);
}

void NFC_StartRead(void)
{
    static const uint8_t ilpt_data[] = { 0x01u, 0x00u }; // 0x01u = Procura no máx. 1 cartão por vez, 0x00u = Protocolo MIFARE
    _send_frame(PN532_CMD_INLISTPASSIVETARGET, ilpt_data, sizeof(ilpt_data));
    
    if (!_read_ack())
    {
        _set_led(0u);
        return;
    }

    _set_led(1u);
}

void NFC_Begin(SPI_HandleTypeDef *hspi)
{
    NFC_Init(hspi);
    NFC_StartRead();
}

void NFC_StopRead(void)
{
    /* Cancela leitura pendente reenviando SAMConfiguration */
    static const uint8_t sam_data[] = { 0x01u, 0x00u };
    _send_frame(PN532_CMD_SAMCONFIGURATION, sam_data, sizeof(sam_data));
    _read_ack();

    uint8_t resp[PN532_RESP_BUF_SIZE];
    uint8_t resp_len = 0u;
    _read_response(resp, sizeof(resp), &resp_len);

    _set_led(0u);

    /* Reseta flag de interrupção caso tenha disparado */
    pn532_card_ready = 0u;
}

uint8_t NFC_GetCard(PN532_Card_t *card)
{
    uint8_t resp[PN532_RESP_BUF_SIZE] = { 0u };
    uint8_t resp_len = 0u;

    memset(card, 0u, sizeof(PN532_Card_t));

    if (!_read_response(resp, sizeof(resp), &resp_len))
    {
        return 0u;
    }

    if (resp_len < 7u)
    {
        return 0u;
    }

    uint8_t n_targets = resp[0];
    if (n_targets == 0u)
    {
        return 0u;
    }

    /* OBS: resp[1] informa número identificador do Target no caso de leituras simultâneas.
    Configuramos máx. de 1 cartão por vez, portanto, não será necessário. */ 

    card->atqa      = (uint16_t)resp[2] | ((uint16_t)resp[3] << 8u);
    card->sak       = resp[4];
    card->uid_len   = resp[5];

    if (card->uid_len > 7u)
    {
        card->uid_len = 7u;
    }

    if (resp_len < (6u + card->uid_len))
    {
        return 0u;
    }

    memcpy(card->uid, &resp[6], card->uid_len);
    card->valid = 1u;

    return 1u;
}

void NFC_IRQ_Handler(void)
{
    pn532_card_ready = 1u;
}

void NFC_Process(void)
{
    if (pn532_card_ready)
    {
        pn532_card_ready = 0u;
        PN532_Card_t card;
        if (NFC_GetCard(&card))
        {
            NFC_CardDetected(&card);
        }
        NFC_StartRead();
    }
}