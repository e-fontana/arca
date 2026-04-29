/**
 * @file    cc1101.h
 * @brief   Driver CC1101 para STM32F401CEUx
 *
 * Pinagem do projeto (conforme .ioc):
 *   SPI1_SCK   -> PA5
 *   SPI1_MISO  -> PA6
 *   SPI1_MOSI  -> PA7
 *   NSS_CC1101 -> PA10  (GPIO Output, pull-up, estado inicial HIGH)
 *   INT_CC1101 -> PB15  (EXTI15, falling edge, pull-up)
 *
 * Baseado na estrutura da biblioteca suleymaneskil/CC1101_STM32_Library,
 * adaptado e expandido para STM32F401CEUx com HAL.
 */

#ifndef CC1101_H
#define CC1101_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

/* =========================================================
 * Pinos (mapeados exatamente do seu .ioc)
 * ========================================================= */
#define CC1101_CS_PORT      GPIOA
#define CC1101_CS_PIN       GPIO_PIN_10

#define CC1101_GDO0_PORT    GPIOB
#define CC1101_GDO0_PIN     GPIO_PIN_15

/* =========================================================
 * Macros de controle do CS
 * ========================================================= */
#define CC1101_CS_LOW()     HAL_GPIO_WritePin(CC1101_CS_PORT, CC1101_CS_PIN, GPIO_PIN_RESET)
#define CC1101_CS_HIGH()    HAL_GPIO_WritePin(CC1101_CS_PORT, CC1101_CS_PIN, GPIO_PIN_SET)

/* =========================================================
 * Registradores de configuração CC1101
 * ========================================================= */
#define CC1101_IOCFG2       0x00  // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01  // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02  // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03  // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04  // Sync word, high byte
#define CC1101_SYNC0        0x05  // Sync word, low byte
#define CC1101_PKTLEN       0x06  // Packet length
#define CC1101_PKTCTRL1     0x07  // Packet automation control 1
#define CC1101_PKTCTRL0     0x08  // Packet automation control 0
#define CC1101_ADDR         0x09  // Device address
#define CC1101_CHANNR       0x0A  // Channel number
#define CC1101_FSCTRL1      0x0B  // Frequency synthesizer control 1
#define CC1101_FSCTRL0      0x0C  // Frequency synthesizer control 0
#define CC1101_FREQ2        0x0D  // Frequency control word, high byte
#define CC1101_FREQ1        0x0E  // Frequency control word, middle byte
#define CC1101_FREQ0        0x0F  // Frequency control word, low byte
#define CC1101_MDMCFG4      0x10  // Modem configuration 4
#define CC1101_MDMCFG3      0x11  // Modem configuration 3
#define CC1101_MDMCFG2      0x12  // Modem configuration 2
#define CC1101_MDMCFG1      0x13  // Modem configuration 1
#define CC1101_MDMCFG0      0x14  // Modem configuration 0
#define CC1101_DEVIATN      0x15  // Modem deviation setting
#define CC1101_MCSM2        0x16  // Main radio control state machine config 2
#define CC1101_MCSM1        0x17  // Main radio control state machine config 1
#define CC1101_MCSM0        0x18  // Main radio control state machine config 0
#define CC1101_FOCCFG       0x19  // Frequency offset compensation config
#define CC1101_BSCFG        0x1A  // Bit synchronization config
#define CC1101_AGCCTRL2     0x1B  // AGC control 2
#define CC1101_AGCCTRL1     0x1C  // AGC control 1
#define CC1101_AGCCTRL0     0x1D  // AGC control 0
#define CC1101_WOREVT1      0x1E  // High byte event 0 timeout
#define CC1101_WOREVT0      0x1F  // Low byte event 0 timeout
#define CC1101_WORCTRL      0x20  // Wake on radio control
#define CC1101_FREND1       0x21  // Front end RX configuration
#define CC1101_FREND0       0x22  // Front end TX configuration
#define CC1101_FSCAL3       0x23  // Frequency synthesizer calibration 3
#define CC1101_FSCAL2       0x24  // Frequency synthesizer calibration 2
#define CC1101_FSCAL1       0x25  // Frequency synthesizer calibration 1
#define CC1101_FSCAL0       0x26  // Frequency synthesizer calibration 0
#define CC1101_RCCTRL1      0x27  // RC oscillator configuration 1
#define CC1101_RCCTRL0      0x28  // RC oscillator configuration 0
#define CC1101_FSTEST       0x29  // Frequency synthesizer calibration control
#define CC1101_PTEST        0x2A  // Production test
#define CC1101_AGCTEST      0x2B  // AGC test
#define CC1101_TEST2        0x2C  // Various test settings 2
#define CC1101_TEST1        0x2D  // Various test settings 1
#define CC1101_TEST0        0x2E  // Various test settings 0

/* =========================================================
 * Registradores de status (somente leitura, acesso burst)
 * ========================================================= */
#define CC1101_PARTNUM      0x30  // Part number
#define CC1101_VERSION      0x31  // Current version number
#define CC1101_FREQEST      0x32  // Frequency offset estimate
#define CC1101_LQI          0x33  // Demodulator estimate for link quality
#define CC1101_RSSI         0x34  // Received signal strength indication
#define CC1101_MARCSTATE    0x35  // Control state machine state
#define CC1101_WORTIME1     0x36  // High byte of WOR timer
#define CC1101_WORTIME0     0x37  // Low byte of WOR timer
#define CC1101_PKTSTATUS    0x38  // Current GDOx status and packet status
#define CC1101_VCO_VC_DAC   0x39  // Current setting from PLL calibration module
#define CC1101_TXBYTES      0x3A  // Underflow and # of bytes in the TX FIFO
#define CC1101_RXBYTES      0x3B  // Overflow and # of bytes in the RX FIFO
#define CC1101_RCCTRL1_STATUS 0x3C
#define CC1101_RCCTRL0_STATUS 0x3D

/* =========================================================
 * Strobes (comandos de 1 byte)
 * ========================================================= */
#define CC1101_SRES         0x30  // Reset chip
#define CC1101_SFSTXON      0x31  // Enable/calibrate freq synthesizer
#define CC1101_SXOFF        0x32  // Turn off crystal oscillator
#define CC1101_SCAL         0x33  // Calibrate frequency synthesizer
#define CC1101_SRX          0x34  // Enable RX
#define CC1101_STX          0x35  // Enable TX
#define CC1101_SIDLE        0x36  // Exit RX/TX
#define CC1101_SWOR         0x38  // Start automatic RX polling
#define CC1101_SPWD         0x39  // Enter power down mode
#define CC1101_SFRX         0x3A  // Flush RX FIFO
#define CC1101_SFTX         0x3B  // Flush TX FIFO
#define CC1101_SWORRST      0x3C  // Reset real time clock
#define CC1101_SNOP         0x3D  // No operation

/* =========================================================
 * Acesso às FIFOs e PATABLE
 * ========================================================= */
#define CC1101_TXFIFO       0x3F  // TX FIFO
#define CC1101_RXFIFO       0x3F  // RX FIFO
#define CC1101_PATABLE      0x3E  // PA power control table

/* =========================================================
 * Bits de controle SPI
 * ========================================================= */
#define CC1101_READ_BIT     0x80  // Bit R/W: leitura
#define CC1101_BURST_BIT    0x40  // Bit burst access

/* =========================================================
 * Tamanho máximo de pacote e FIFO
 * ========================================================= */
#define CC1101_MAX_PACKET_LEN   61  // Max payload (64 FIFO - 2 status - 1 length)
#define CC1101_FIFO_SIZE        64

/* =========================================================
 * Estrutura de pacote recebido
 * ========================================================= */
typedef struct {
    uint8_t length;                          // Tamanho do payload
    uint8_t data[CC1101_MAX_PACKET_LEN];     // Dados recebidos
    int8_t  rssi;                            // RSSI em dBm
    uint8_t lqi;                             // Link Quality Indicator (0-127)
    uint8_t crc_ok;                          // 1 = CRC OK, 0 = CRC FAIL
} CC1101_Packet_t;

/* =========================================================
 * Protótipos de funções
 * ========================================================= */

/**
 * @brief Inicializa o CC1101 com configuração padrão para 433 MHz, GFSK, 4800 bps.
 * @param hspi Ponteiro para o handle SPI1 (ex: &hspi1)
 * @return 1 se inicializado com sucesso, 0 se módulo não respondeu
 */
uint8_t CC1101_Init(SPI_HandleTypeDef *hspi);

/**
 * @brief Envia um pacote de dados.
 * @param data    Ponteiro para o buffer de dados
 * @param length  Número de bytes (máx. CC1101_MAX_PACKET_LEN)
 * @return 1 se enviou com sucesso, 0 se erro
 */
uint8_t CC1101_SendPacket(const uint8_t *data, uint8_t length);

/**
 * @brief Lê um pacote recebido (chamar dentro do callback EXTI de INT_CC1101).
 * @param pkt Ponteiro para estrutura de pacote
 * @return 1 se pacote válido (CRC OK), 0 caso contrário
 */
uint8_t CC1101_ReceivePacket(CC1101_Packet_t *pkt);

/**
 * @brief Coloca o chip em modo RX contínuo.
 */
void CC1101_SetRxMode(void);

/**
 * @brief Coloca o chip em modo IDLE.
 */
void CC1101_SetIdle(void);

/**
 * @brief Lê o RSSI atual em dBm.
 * @return Valor de RSSI em dBm
 */
int8_t CC1101_GetRSSI(void);

/**
 * @brief Lê um registrador de configuração.
 * @param addr Endereço do registrador
 * @return Valor lido
 */
uint8_t CC1101_ReadReg(uint8_t addr);

/**
 * @brief Escreve em um registrador de configuração.
 * @param addr  Endereço do registrador
 * @param value Valor a escrever
 */
void CC1101_WriteReg(uint8_t addr, uint8_t value);

/**
 * @brief Envia um strobe (comando de 1 byte).
 * @param strobe Código do strobe (ex: CC1101_SRX)
 */
void CC1101_Strobe(uint8_t strobe);

/**
 * @brief Verifica se o chip está respondendo (lê PARTNUM, deve ser 0x00 e VERSION 0x14).
 * @return 1 se OK, 0 se falha de comunicação
 */
uint8_t CC1101_CheckPartNumber(void);

#endif /* CC1101_H */