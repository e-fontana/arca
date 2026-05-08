/*
 * CC1101.H
 *
 *  Created on: Mar 11, 2020
 *      Author: suleyman.eskil
 *
 * Adaptado para STM32F401CEUx:
 *   - HAL include alterado para stm32f4xx_hal.h
 *   - Typedefs protegidos com #ifndef para evitar conflitos
 *   - Power_up_reset() corrigido para receber parâmetros (globais eram nulos antes de TI_init)
 *   - Removidas declarações sem implementação: TI_write_burst_reg_c, get_random_byte
 *
 * Pinagem do projeto:
 *   SPI1_SCK   -> PA5
 *   SPI1_MISO  -> PA6  (hardcoded na biblioteca)
 *   SPI1_MOSI  -> PA7
 *   NSS_CC1101 -> PA10 (passado como parâmetro)
 *   INT_CC1101 -> PB15 (GDO0, EXTI15)
 */

#ifndef INC_CC1101_H_
#define INC_CC1101_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

//-------------------------------------------------------------------------------------------------------
#ifndef BOOL
typedef unsigned char  BOOL;
#endif
#ifndef BYTE
typedef unsigned char  BYTE;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef DWORD
typedef unsigned long  DWORD;
#endif
#ifndef UINT8
typedef unsigned char  UINT8;
#endif
#ifndef UINT16
typedef unsigned short UINT16;
#endif
#ifndef UINT32
typedef unsigned long  UINT32;
#endif
#ifndef INT8
typedef signed char    INT8;
#endif
#ifndef INT16
typedef signed short   INT16;
#endif
#ifndef INT32
typedef signed long    INT32;
#endif

//-------------------------------------------------------------------------------------------------------
#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE 1
#endif

//------------------------------------------------------------------------------------------------------
// CC1101 STROBE, CONTROL AND STATUS REGISTERS
#define CCxxx0_IOCFG2       0x00
#define CCxxx0_IOCFG1       0x01
#define CCxxx0_IOCFG0       0x02
#define CCxxx0_FIFOTHR      0x03
#define CCxxx0_SYNC1        0x04
#define CCxxx0_SYNC0        0x05
#define CCxxx0_PKTLEN       0x06
#define CCxxx0_PKTCTRL1     0x07
#define CCxxx0_PKTCTRL0     0x08
#define CCxxx0_ADDR         0x09
#define CCxxx0_CHANNR       0x0A
#define CCxxx0_FSCTRL1      0x0B
#define CCxxx0_FSCTRL0      0x0C
#define CCxxx0_FREQ2        0x0D
#define CCxxx0_FREQ1        0x0E
#define CCxxx0_FREQ0        0x0F
#define CCxxx0_MDMCFG4      0x10
#define CCxxx0_MDMCFG3      0x11
#define CCxxx0_MDMCFG2      0x12
#define CCxxx0_MDMCFG1      0x13
#define CCxxx0_MDMCFG0      0x14
#define CCxxx0_DEVIATN      0x15
#define CCxxx0_MCSM2        0x16
#define CCxxx0_MCSM1        0x17
#define CCxxx0_MCSM0        0x18
#define CCxxx0_FOCCFG       0x19
#define CCxxx0_BSCFG        0x1A
#define CCxxx0_AGCCTRL2     0x1B
#define CCxxx0_AGCCTRL1     0x1C
#define CCxxx0_AGCCTRL0     0x1D
#define CCxxx0_WOREVT1      0x1E
#define CCxxx0_WOREVT0      0x1F
#define CCxxx0_WORCTRL      0x20
#define CCxxx0_FREND1       0x21
#define CCxxx0_FREND0       0x22
#define CCxxx0_FSCAL3       0x23
#define CCxxx0_FSCAL2       0x24
#define CCxxx0_FSCAL1       0x25
#define CCxxx0_FSCAL0       0x26
#define CCxxx0_RCCTRL1      0x27
#define CCxxx0_RCCTRL0      0x28
#define CCxxx0_FSTEST       0x29
#define CCxxx0_PTEST        0x2A
#define CCxxx0_AGCTEST      0x2B
#define CCxxx0_TEST2        0x2C
#define CCxxx0_TEST1        0x2D
#define CCxxx0_TEST0        0x2E

// Strobe commands
#define CCxxx0_SRES         0x30
#define CCxxx0_SFSTXON      0x31
#define CCxxx0_SXOFF        0x32
#define CCxxx0_SCAL         0x33
#define CCxxx0_SRX          0x34
#define CCxxx0_STX          0x35
#define CCxxx0_SIDLE        0x36
#define CCxxx0_SAFC         0x37
#define CCxxx0_SWOR         0x38
#define CCxxx0_SPWD         0x39
#define CCxxx0_SFRX         0x3A
#define CCxxx0_SFTX         0x3B
#define CCxxx0_SWORRST      0x3C
#define CCxxx0_SNOP         0x3D

// Status registers
#define CCxxx0_PARTNUM          0x30
#define CCxxx0_VERSION          0x31
#define CCxxx0_FREQEST          0x32
#define CCxxx0_LQI              0x33
#define CCxxx0_RSSI             0x34
#define CCxxx0_MARCSTATE        0x35
#define CCxxx0_WORTIME1         0x36
#define CCxxx0_WORTIME0         0x37
#define CCxxx0_PKTSTATUS        0x38
#define CCxxx0_VCO_VC_DAC       0x39
#define CCxxx0_TXBYTES          0x3A
#define CCxxx0_RXBYTES          0x3B
#define CCxxx0_RCCTRL1_STATUS   0x3C
#define CCxxx0_RCCTRL0_STATUS   0x3D
#define TI_CCxxx0_NUM_RXBYTES   0x7F

#define CCxxx0_PATABLE          0x3E
#define CCxxx0_TXFIFO           0x3F
#define CCxxx0_RXFIFO           0x3F

// Masks for appended status bytes
#define TI_CCxxx0_LQI_RX        0x01
#define TI_CCxxx0_CRC_OK        0x80

// Burst/single access bits
#define TI_CCxxx0_WRITE_BURST   0x40
#define TI_CCxxx0_READ_SINGLE   0x80
#define TI_CCxxx0_READ_BURST    0xC0

//-------------------------------------------------------------------------------------------------------
// Function declarations

HAL_StatusTypeDef __spi_write(uint8_t *addr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef __spi_read(uint8_t *addr, uint8_t *pData, uint16_t size);

void TI_init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
void Power_up_reset(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

void init_serial(UART_HandleTypeDef *huart);

void TI_write_reg(UINT8 addr, UINT8 value);
void TI_write_burst_reg(BYTE addr, BYTE *buffer, BYTE count);
void TI_strobe(BYTE strobe);
BYTE TI_read_reg(BYTE addr);
BYTE TI_read_status(BYTE addr);
void TI_read_burst_reg(BYTE addr, BYTE *buffer, BYTE count);
BOOL TI_receive_packet(BYTE *rxBuffer, UINT8 *length);
void TI_send_packet(BYTE *txBuffer, UINT8 size);
void TI_write_settings(void);

#endif /* INC_CC1101_H_ */
