#include "com.h"
#include "cc1101.h"
#include "main.h"
#include <stdio.h>
#include <stdint.h>

static volatile uint8_t gdo0_flag = 0;

void COM_CC1101_Init(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart)
{
    (void)huart;

    Power_up_reset(hspi, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);
    TI_init(hspi, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);
    TI_write_reg(CCxxx0_MCSM1, 0x30);

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

void COM_CC1101_HandleInterrupt(uint16_t gpio_pin)
{
    if (gpio_pin != INT_CC1101_Pin) {
        return;
    }

    gdo0_flag = 1;
}

uint8_t COM_CC1101_ProcessInterrupt(COM_CC1101_RxEvent_t *event)
{
    if (event == NULL) {
        return 0;
    }

    if (!gdo0_flag) {
        return 0;
    }

    gdo0_flag = 0;
    event->has_packet = 1;
    event->payload_len = COM_CC1101_MAX_PAYLOAD_LEN;
    event->is_valid = COM_CC1101_Receive(event->payload, &event->payload_len);

    if (event->is_valid) {
        snprintf(event->message, sizeof(event->message), "RX len=%u", event->payload_len);
        return 1;
    } else {
        snprintf(event->message, sizeof(event->message), "RX invalido");
        event->payload_len = 0;
        return 0;
    }
}

uint8_t COM_CC1101_Poll(COM_CC1101_RxEvent_t *event)
{
    if (event == NULL) {
        return 0;
    }

    memset(event, 0, sizeof(*event));
    return COM_CC1101_ProcessInterrupt(event);
}
