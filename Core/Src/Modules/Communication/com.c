#include "com.h"
#include "cc1101.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void COM_CC1101_Init(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart)
{
    char dbg[128];

    Power_up_reset(hspi, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);
    TI_init(hspi, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);
    TI_write_reg(CCxxx0_MCSM1, 0x30);

    /* Diagnóstico 1: estado após init */
    uint8_t st1 = TI_read_status(CCxxx0_MARCSTATE) & 0x1F;
    snprintf(dbg, sizeof(dbg), "[1] STATE=%d (esperado 1=IDLE)\r\n", st1);
    HAL_UART_Transmit(huart, (uint8_t*)dbg, strlen(dbg), HAL_MAX_DELAY);

    /* Diagnóstico 2: SIDLE explícito */
    TI_strobe(CCxxx0_SIDLE);
    HAL_Delay(1);
    uint8_t st2 = TI_read_status(CCxxx0_MARCSTATE) & 0x1F;
    snprintf(dbg, sizeof(dbg), "[2] Apos SIDLE STATE=%d\r\n", st2);
    HAL_UART_Transmit(huart, (uint8_t*)dbg, strlen(dbg), HAL_MAX_DELAY);

    /* Diagnóstico 3: entra em RX e aguarda calibração */
    TI_strobe(CCxxx0_SRX);
    HAL_Delay(10);
    uint8_t st3 = TI_read_status(CCxxx0_MARCSTATE) & 0x1F;
    snprintf(dbg, sizeof(dbg), "[3] Apos SRX+10ms STATE=%d (esperado 13=RX)\r\n", st3);
    HAL_UART_Transmit(huart, (uint8_t*)dbg, strlen(dbg), HAL_MAX_DELAY);

    /* Diagnóstico 4: lê status byte via SNOP */
    HAL_GPIO_WritePin(NSS_CC1101_GPIO_Port, NSS_CC1101_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    uint8_t tx = CCxxx0_SNOP;
    uint8_t status_byte = 0;
    HAL_SPI_TransmitReceive(hspi, &tx, &status_byte, 1, 10);
    HAL_GPIO_WritePin(NSS_CC1101_GPIO_Port, NSS_CC1101_Pin, GPIO_PIN_SET);
    uint8_t chip_state = (status_byte >> 4) & 0x07;
    snprintf(dbg, sizeof(dbg), "[4] SNOP status=0x%02X chip_state=%d (1=RX)\r\n",
             status_byte, chip_state);
    HAL_UART_Transmit(huart, (uint8_t*)dbg, strlen(dbg), HAL_MAX_DELAY);

    /* Habilita EXTI do GDO0 (PB15) e entra em modo RX */
    __HAL_GPIO_EXTI_CLEAR_IT(INT_CC1101_Pin);
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    TI_strobe(CCxxx0_SFRX);
    TI_strobe(CCxxx0_SRX);
}

void COM_CC1101_Transmit(const uint8_t *data, uint8_t length)
{
    TI_strobe(CCxxx0_SIDLE);
    TI_strobe(CCxxx0_SFTX);

    TI_send_packet((BYTE *)data, length);

    /* Aguarda GDO0 subir (início do TX) e descer (fim do TX) */
    while (!HAL_GPIO_ReadPin(INT_CC1101_GPIO_Port, INT_CC1101_Pin));
    while ( HAL_GPIO_ReadPin(INT_CC1101_GPIO_Port, INT_CC1101_Pin));

    /* Volta ao modo RX após o envio */
    TI_strobe(CCxxx0_SFRX);
    TI_strobe(CCxxx0_SRX);
}

uint8_t COM_CC1101_Receive(uint8_t *buf, uint8_t *len)
{
    uint8_t fifo_bytes = TI_read_status(CCxxx0_RXBYTES);

    if (!(fifo_bytes & 0x7F)) {
        TI_strobe(CCxxx0_SFRX);
        TI_strobe(CCxxx0_SRX);
        return 0;
    }

    uint8_t ok = TI_receive_packet(buf, len);

    TI_strobe(CCxxx0_SFRX);
    TI_strobe(CCxxx0_SRX);
    return ok;
}
