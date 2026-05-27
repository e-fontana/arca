/*
 * cc1101.c
 *
 *  Created on: Mar 11, 2020
 *      Author: suleyman.eskil / ilynx
 *
 * Adaptado para STM32F401CEUx:
 *   - Power_up_reset() recebe hspi, cs_port e cs_pin para evitar uso de globais nulos
 */

#include "cc1101.h"
#include "dw_stm32_delay.h"

SPI_HandleTypeDef *hal_spi;
UART_HandleTypeDef *hal_uart;

uint16_t CS_Pin;
GPIO_TypeDef *CS_GPIO_Port;

#define WRITE_BURST     0x40
#define READ_SINGLE     0x80
#define READ_BURST      0xC0

#define BYTES_IN_RXFIFO 0x7F
#define LQI             1
#define CRC_OK          0x80

HAL_StatusTypeDef __spi_write(uint8_t *addr, uint8_t *pData, uint16_t size)
{
    HAL_StatusTypeDef status;
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
    status = HAL_SPI_Transmit(hal_spi, addr, 1, 0xFFFF);
    if (status == HAL_OK && pData != NULL)
        status = HAL_SPI_Transmit(hal_spi, pData, size, 0xFFFF);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
    return status;
}

HAL_StatusTypeDef __spi_read(uint8_t *addr, uint8_t *pData, uint16_t size)
{
    HAL_StatusTypeDef status;
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
    status = HAL_SPI_Transmit(hal_spi, addr, 1, 0xFFFF);
    status = HAL_SPI_Receive(hal_spi, pData, size, 0xFFFF);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
    return status;
}

void TI_write_reg(UINT8 addr, UINT8 value)
{
    __spi_write(&addr, &value, 1);
}

void TI_write_burst_reg(BYTE addr, BYTE *buffer, BYTE count)
{
    addr = (addr | WRITE_BURST);
    __spi_write(&addr, buffer, count);
}

void TI_strobe(BYTE strobe)
{
    __spi_write(&strobe, 0, 0);
}

BYTE TI_read_reg(BYTE addr)
{
    uint8_t data;
    addr = (addr | READ_SINGLE);
    __spi_read(&addr, &data, 1);
    return data;
}

BYTE TI_read_status(BYTE addr)
{
    uint8_t data;
    addr = (addr | READ_BURST);
    __spi_read(&addr, &data, 1);
    return data;
}

void TI_read_burst_reg(BYTE addr, BYTE *buffer, BYTE count)
{
    addr = (addr | READ_BURST);
    __spi_read(&addr, buffer, count);
}

BOOL TI_receive_packet(BYTE *rxBuffer, UINT8 *length)
{
    BYTE status[2];
    UINT8 packet_len;

    if (TI_read_status(CCxxx0_RXBYTES) & BYTES_IN_RXFIFO)
    {
        packet_len = TI_read_reg(CCxxx0_RXFIFO);

        if (packet_len <= *length)
        {
            TI_read_burst_reg(CCxxx0_RXFIFO, rxBuffer, packet_len);
            *length = packet_len;
            TI_read_burst_reg(CCxxx0_RXFIFO, status, 2);
            return (status[LQI] & CRC_OK);
        }
        else
        {
            *length = packet_len;
            TI_strobe(CCxxx0_SIDLE);
            TI_strobe(CCxxx0_SFRX);
            return (FALSE);
        }
    }
    else return (FALSE);
}

void init_serial(UART_HandleTypeDef *huart)
{
    hal_uart = huart;
}

void TI_send_packet(BYTE *txBuffer, UINT8 size)
{
    TI_strobe(CCxxx0_SIDLE);
    TI_write_reg(CCxxx0_TXFIFO, size);
    TI_write_burst_reg(CCxxx0_TXFIFO, txBuffer, size);
    TI_strobe(CCxxx0_STX);
}

BYTE paTable[] = {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0};

void TI_write_settings(void)
{
    // 433 MHz / GFSK / 1.2 kbps — gerado via SmartRF Studio 7
    TI_write_reg(CCxxx0_IOCFG2,   0x29);
    TI_write_reg(CCxxx0_IOCFG1,   0x2E);
    TI_write_reg(CCxxx0_IOCFG0,   0x06);
    TI_write_reg(CCxxx0_FIFOTHR,  0x47);
    TI_write_reg(CCxxx0_SYNC1,    0xD3);
    TI_write_reg(CCxxx0_SYNC0,    0x91);
    TI_write_reg(CCxxx0_PKTLEN,   0x3D);
    TI_write_reg(CCxxx0_PKTCTRL1, 0x0E);
    TI_write_reg(CCxxx0_PKTCTRL0, 0x05);
    TI_write_reg(CCxxx0_ADDR,     0x00);
    TI_write_reg(CCxxx0_CHANNR,   0x00);
    TI_write_reg(CCxxx0_FSCTRL1,  0x08);
    TI_write_reg(CCxxx0_FSCTRL0,  0x00);
    TI_write_reg(CCxxx0_FREQ2,    0x10);
    TI_write_reg(CCxxx0_FREQ1,    0xB4);
    TI_write_reg(CCxxx0_FREQ0,    0x2E);
    TI_write_reg(CCxxx0_MDMCFG4,  0xCA);
    TI_write_reg(CCxxx0_MDMCFG3,  0x83);
    TI_write_reg(CCxxx0_MDMCFG2,  0x93);
    TI_write_reg(CCxxx0_MDMCFG1,  0x22);
    TI_write_reg(CCxxx0_MDMCFG0,  0xF8);
    TI_write_reg(CCxxx0_DEVIATN,  0x34);
    TI_write_reg(CCxxx0_MCSM2,    0x07);
    TI_write_reg(CCxxx0_MCSM1,    0x30);
    TI_write_reg(CCxxx0_MCSM0,    0x18);
    TI_write_reg(CCxxx0_FOCCFG,   0x16);
    TI_write_reg(CCxxx0_BSCFG,    0x6C);
    TI_write_reg(CCxxx0_AGCCTRL2, 0x43);
    TI_write_reg(CCxxx0_AGCCTRL1, 0x40);
    TI_write_reg(CCxxx0_AGCCTRL0, 0x91);
    TI_write_reg(CCxxx0_WOREVT1,  0x87);
    TI_write_reg(CCxxx0_WOREVT0,  0x6B);
    TI_write_reg(CCxxx0_WORCTRL,  0xF8);
    TI_write_reg(CCxxx0_FREND1,   0x56);
    TI_write_reg(CCxxx0_FREND0,   0x10);
    TI_write_reg(CCxxx0_FSCAL3,   0xE9);
    TI_write_reg(CCxxx0_FSCAL2,   0x2A);
    TI_write_reg(CCxxx0_FSCAL1,   0x00);
    TI_write_reg(CCxxx0_FSCAL0,   0x1F);
    TI_write_reg(CCxxx0_RCCTRL1,  0x41);
    TI_write_reg(CCxxx0_RCCTRL0,  0x00);
    TI_write_reg(CCxxx0_FSTEST,   0x59);
    TI_write_reg(CCxxx0_PTEST,    0x7F);
    TI_write_reg(CCxxx0_AGCTEST,  0x3F);
    TI_write_reg(CCxxx0_TEST2,    0x81);
    TI_write_reg(CCxxx0_TEST1,    0x35);
    TI_write_reg(CCxxx0_TEST0,    0x09);
}

void Power_up_reset(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    hal_spi      = hspi;
    CS_GPIO_Port = cs_port;
    CS_Pin       = cs_pin;

    DWT_Delay_Init();
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
    DWT_Delay_us(1);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
    DWT_Delay_us(1);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
    DWT_Delay_us(41);

    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6));
    TI_strobe(CCxxx0_SRES);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

void TI_init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    hal_spi      = hspi;
    CS_GPIO_Port = cs_port;
    CS_Pin       = cs_pin;

    for (int i = 0; i < 10; i++)
        TI_read_status(CCxxx0_VERSION);

    TI_strobe(CCxxx0_SFRX);
    TI_strobe(CCxxx0_SFTX);
    TI_write_settings();
    TI_write_burst_reg(CCxxx0_PATABLE, paTable, 8);
    TI_write_reg(CCxxx0_FIFOTHR, 0x07);
    TI_strobe(CCxxx0_SIDLE);
    TI_strobe(CCxxx0_SFRX);
    TI_strobe(CCxxx0_SFTX);
    TI_strobe(CCxxx0_SIDLE);
}
