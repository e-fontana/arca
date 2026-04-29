/**
 * @file    cc1101.c
 * @brief   Driver CC1101 para STM32F401CEUx — versão corrigida
 *
 * Correções aplicadas:
 *   1. _WaitMISO() sem HAL_Delay (loop por ciclos, não bloqueia infinitamente)
 *   2. CC1101_Init() retorna uint8_t: 1=OK, 0=FALHA (módulo não respondeu)
 *   3. Reset via CS toggle manual conforme datasheet seção 10.1
 *   4. Verificação do chip (VERSION) antes de programar registradores
 */

#include "cc1101.h"

static SPI_HandleTypeDef *_hspi = NULL;

/* =========================================================
 * Tabela de registradores: 433 MHz / GFSK / 4800 bps
 * ========================================================= */
static const uint8_t cc1101_config[][2] = {
    {CC1101_IOCFG2,   0x0D},
    {CC1101_IOCFG0,   0x06},
    {CC1101_FIFOTHR,  0x47},
    {CC1101_SYNC1,    0xD3},
    {CC1101_SYNC0,    0x91},
    {CC1101_PKTLEN,   0x3D},
    {CC1101_PKTCTRL1, 0x04},
    {CC1101_PKTCTRL0, 0x05},
    {CC1101_ADDR,     0x00},
    {CC1101_CHANNR,   0x00},
    {CC1101_FSCTRL1,  0x06},
    {CC1101_FSCTRL0,  0x00},
    {CC1101_FREQ2,    0x10},
    {CC1101_FREQ1,    0xA7},
    {CC1101_FREQ0,    0x62},
    {CC1101_MDMCFG4,  0xC7},
    {CC1101_MDMCFG3,  0x83},
    {CC1101_MDMCFG2,  0x13},
    {CC1101_MDMCFG1,  0x22},
    {CC1101_MDMCFG0,  0xF8},
    {CC1101_DEVIATN,  0x35},
    {CC1101_MCSM2,    0x07},
    {CC1101_MCSM1,    0x30},
    {CC1101_MCSM0,    0x18},
    {CC1101_FOCCFG,   0x16},
    {CC1101_BSCFG,    0x6C},
    {CC1101_AGCCTRL2, 0x43},
    {CC1101_AGCCTRL1, 0x40},
    {CC1101_AGCCTRL0, 0x91},
    {CC1101_FREND1,   0x56},
    {CC1101_FREND0,   0x10},
    {CC1101_FSCAL3,   0xE9},
    {CC1101_FSCAL2,   0x2A},
    {CC1101_FSCAL1,   0x00},
    {CC1101_FSCAL0,   0x1F},
    {CC1101_TEST2,    0x81},
    {CC1101_TEST1,    0x35},
    {CC1101_TEST0,    0x09},
};

static const uint8_t cc1101_patable[8] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* =========================================================
 * _WaitMISO — sem HAL_Delay, timeout por contagem de ciclos
 *
 * HAL_Delay() depende de SysTick e não pode ser usado dentro de
 * funções chamadas antes do scheduler ou em contextos críticos.
 * Esta versão usa um loop simples com contagem de ciclos.
 *
 * @return 1 = MISO foi LOW (chip pronto), 0 = timeout
 * ========================================================= */
static uint8_t _WaitMISO(void)
{
    /* 84 MHz: ~4.200.000 ciclos = ~50 ms de timeout máximo */
    volatile uint32_t timeout = 4200000UL;
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET)
    {
        if (--timeout == 0)
        {
            CC1101_CS_HIGH();
            return 0;
        }
    }
    return 1;
}

/* Loop de delay por ciclos — independente de SysTick */
static void _DelayCycles(uint32_t cycles)
{
    volatile uint32_t c = cycles;
    while (c--);
}

/* =========================================================
 * Primitivas SPI
 * ========================================================= */
static uint8_t _SPI_Transfer(uint8_t byte)
{
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(_hspi, &byte, &rx, 1, 10);
    return rx;
}

static uint8_t _WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len)
{
    CC1101_CS_LOW();
    if (!_WaitMISO()) return 0;
    _SPI_Transfer(addr | CC1101_BURST_BIT);
    for (uint8_t i = 0; i < len; i++) _SPI_Transfer(data[i]);
    CC1101_CS_HIGH();
    return 1;
}

static uint8_t _ReadBurst(uint8_t addr, uint8_t *buf, uint8_t len)
{
    CC1101_CS_LOW();
    if (!_WaitMISO()) return 0;
    _SPI_Transfer(addr | CC1101_READ_BIT | CC1101_BURST_BIT);
    for (uint8_t i = 0; i < len; i++) buf[i] = _SPI_Transfer(0x00);
    CC1101_CS_HIGH();
    return 1;
}

/* =========================================================
 * API pública
 * ========================================================= */
void CC1101_WriteReg(uint8_t addr, uint8_t value)
{
    CC1101_CS_LOW();
    if (!_WaitMISO()) return;
    _SPI_Transfer(addr & 0x3F);
    _SPI_Transfer(value);
    CC1101_CS_HIGH();
}

uint8_t CC1101_ReadReg(uint8_t addr)
{
    uint8_t val = 0;
    CC1101_CS_LOW();
    if (!_WaitMISO()) return 0;
    _SPI_Transfer(addr | CC1101_READ_BIT);
    val = _SPI_Transfer(0x00);
    CC1101_CS_HIGH();
    return val;
}

static uint8_t _ReadStatusReg(uint8_t addr)
{
    uint8_t val = 0;
    CC1101_CS_LOW();
    if (!_WaitMISO()) return 0;
    _SPI_Transfer(addr | CC1101_READ_BIT | CC1101_BURST_BIT);
    val = _SPI_Transfer(0x00);
    CC1101_CS_HIGH();
    return val;
}

void CC1101_Strobe(uint8_t strobe)
{
    CC1101_CS_LOW();
    if (!_WaitMISO()) return;
    _SPI_Transfer(strobe);
    CC1101_CS_HIGH();
}

/* =========================================================
 * Inicialização
 *
 * Sequência de reset conforme datasheet CC1101 seção 10.1:
 *   1. CS HIGH
 *   2. CS LOW por >= 1 ciclo de SCLK
 *   3. CS HIGH, aguarda >= 40 us
 *   4. CS LOW, espera MISO=LOW, envia SRES, espera MISO=LOW
 *   5. CS HIGH
 *
 * @return 1 = inicializado com sucesso, 0 = módulo não encontrado
 * ========================================================= */
uint8_t CC1101_Init(SPI_HandleTypeDef *hspi)
{
    _hspi = hspi;

    /* 1. Garante CS HIGH e aguarda power-on (~10 ms) */
    CC1101_CS_HIGH();
    _DelayCycles(84000UL * 10);

    /* 2. Pulso no CS para resetar a interface SPI do chip */
    CC1101_CS_LOW();
    _DelayCycles(84UL * 10);   /* ~10 µs */
    CC1101_CS_HIGH();
    _DelayCycles(84000UL * 1); /* ~1 ms */

    /* 3. Reset via strobe SRES */
    CC1101_CS_LOW();
    if (!_WaitMISO())
    {
        /* MISO nunca baixou: módulo ausente ou sem alimentação */
        return 0;
    }
    _SPI_Transfer(CC1101_SRES);

    /* Após SRES, chip puxa MISO=HIGH durante o reset interno,
     * depois volta a MISO=LOW quando está pronto */
    if (!_WaitMISO())
    {
        CC1101_CS_HIGH();
        return 0;
    }
    CC1101_CS_HIGH();
    _DelayCycles(84000UL * 5); /* ~5 ms */

    /* 4. Verifica comunicação — VERSION do CC1101 deve ser 0x14 */
    uint8_t version = _ReadStatusReg(CC1101_VERSION);
    if (version != 0x14)
    {
        return 0;
    }

    /* 5. Aplica configuração de registradores */
    uint8_t n = sizeof(cc1101_config) / sizeof(cc1101_config[0]);
    for (uint8_t i = 0; i < n; i++)
    {
        CC1101_WriteReg(cc1101_config[i][0], cc1101_config[i][1]);
    }

    /* 6. PATABLE */
    _WriteBurst(CC1101_PATABLE, cc1101_patable, 8);

    /* 7. Limpa FIFOs e entra em RX */
    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFRX);
    CC1101_Strobe(CC1101_SFTX);
    CC1101_Strobe(CC1101_SRX);

    return 1;
}

/* =========================================================
 * Envio de pacote
 * ========================================================= */
uint8_t CC1101_SendPacket(const uint8_t *data, uint8_t length)
{
    if (length == 0 || length > CC1101_MAX_PACKET_LEN) return 0;

    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFTX);

    CC1101_CS_LOW();
    if (!_WaitMISO()) return 0;
    _SPI_Transfer(CC1101_TXFIFO | CC1101_BURST_BIT);
    _SPI_Transfer(length);
    for (uint8_t i = 0; i < length; i++) _SPI_Transfer(data[i]);
    CC1101_CS_HIGH();

    CC1101_Strobe(CC1101_STX);

    /* Polling com timeout por ciclos (sem HAL_Delay) */
    uint32_t timeout;

    timeout = 84000UL * 200;
    while (HAL_GPIO_ReadPin(CC1101_GDO0_PORT, CC1101_GDO0_PIN) == GPIO_PIN_RESET)
    {
        if (--timeout == 0) { CC1101_Strobe(CC1101_SIDLE); return 0; }
    }

    timeout = 84000UL * 200;
    while (HAL_GPIO_ReadPin(CC1101_GDO0_PORT, CC1101_GDO0_PIN) == GPIO_PIN_SET)
    {
        if (--timeout == 0) { CC1101_Strobe(CC1101_SIDLE); return 0; }
    }

    CC1101_Strobe(CC1101_SRX);
    return 1;
}

/* =========================================================
 * Recepção — chamar dentro de HAL_GPIO_EXTI_Callback (GPIO_PIN_15)
 * ========================================================= */
uint8_t CC1101_ReceivePacket(CC1101_Packet_t *pkt)
{
    uint8_t rx_bytes = _ReadStatusReg(CC1101_RXBYTES);

    if ((rx_bytes & 0x80) || (rx_bytes == 0))
    {
        CC1101_Strobe(CC1101_SIDLE);
        CC1101_Strobe(CC1101_SFRX);
        CC1101_Strobe(CC1101_SRX);
        return 0;
    }

    uint8_t length = 0;
    _ReadBurst(CC1101_RXFIFO, &length, 1);

    if (length == 0 || length > CC1101_MAX_PACKET_LEN)
    {
        CC1101_Strobe(CC1101_SIDLE);
        CC1101_Strobe(CC1101_SFRX);
        CC1101_Strobe(CC1101_SRX);
        return 0;
    }

    _ReadBurst(CC1101_RXFIFO, pkt->data, length);
    pkt->length = length;

    uint8_t status[2];
    _ReadBurst(CC1101_RXFIFO, status, 2);

    pkt->rssi   = (status[0] >= 128) ? (int8_t)((status[0] - 256) / 2) - 74
                                      : (int8_t)(status[0] / 2) - 74;
    pkt->lqi    = status[1] & 0x7F;
    pkt->crc_ok = (status[1] & 0x80) >> 7;

    CC1101_Strobe(CC1101_SRX);
    return pkt->crc_ok;
}

/* =========================================================
 * Utilitários
 * ========================================================= */
void CC1101_SetRxMode(void)
{
    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFRX);
    CC1101_Strobe(CC1101_SRX);
}

void CC1101_SetIdle(void)
{
    CC1101_Strobe(CC1101_SIDLE);
}

int8_t CC1101_GetRSSI(void)
{
    uint8_t raw = _ReadStatusReg(CC1101_RSSI);
    return (raw >= 128) ? (int8_t)((raw - 256) / 2) - 74
                        : (int8_t)(raw / 2) - 74;
}

uint8_t CC1101_CheckPartNumber(void)
{
    uint8_t partnum = _ReadStatusReg(CC1101_PARTNUM);
    uint8_t version = _ReadStatusReg(CC1101_VERSION);
    return (partnum == 0x00 && version == 0x14) ? 1 : 0;
}