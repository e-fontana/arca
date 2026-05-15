#include "com.h"
#include "cc1101.h"
#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static volatile uint8_t     gdo0_flag = 0;
static UART_HandleTypeDef  *s_huart   = NULL;

UART_HandleTypeDef *COM_GetUart(void) { return s_huart; }

static void com_log(const char *msg)
{
    if (!s_huart) return;
    HAL_UART_Transmit(s_huart, (uint8_t *)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}


void COM_CC1101_Init(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart)
{
    s_huart = huart;

    Power_up_reset(hspi, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);
    TI_init(hspi, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);
    TI_write_reg(CCxxx0_MCSM1, 0x30);
    TI_write_reg(CCxxx0_ADDR, MY_ADDR);

    #if defined(COM_DIAG_DISABLE_ADDR_CHK)
        /* Modo diagnóstico: desabilita ADDR_CHK em hardware para ver todos os pacotes.
        Ativar com -DCOM_DIAG_DISABLE_ADDR_CHK no CMakeLists.txt. */
        TI_write_reg(CCxxx0_PKTCTRL1, TI_read_reg(CCxxx0_PKTCTRL1) & ~0x06u);
    #endif

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

    /* A queda do GDO0 no fim do TX dispara o ISR — descarta para não
       confundir o próximo Poll com um evento de RX. */
    gdo0_flag = 0;
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
    if (!gdo0_flag) return 0;
    gdo0_flag = 0;

    uint8_t buf[64];
    uint8_t len = sizeof(buf);
    uint8_t crc_ok = COM_CC1101_Receive(buf, &len);

    if (!crc_ok) {
        com_log("[CC1101] CRC falhou ou mensagem é para outro target — pacote descartado\r\n");
        return 0;
    }

    COM_Frame_t frame;
    if (!COM_ParseFrame(buf, len, &frame)) {
        com_log("[CC1101] Falha ao parsear frame — descartado\r\n");
        return 0;
    }

    event->has_packet  = 1;
    event->is_valid    = 1;
    event->frame       = frame;
    return 1;
}

uint8_t COM_CC1101_Poll(COM_CC1101_RxEvent_t *event)
{
    if (event == NULL) {
        return 0;
    }

    memset(event, 0, sizeof(*event));
    return COM_CC1101_ProcessInterrupt(event);
}

uint8_t COM_WaitResponse(uint8_t expected_src, COM_Frame_t *out_frame, uint32_t timeout_ms)
{
    uint32_t t0 = HAL_GetTick();
    while ((HAL_GetTick() - t0) < timeout_ms) {
        COM_CC1101_RxEvent_t ev;
        if (COM_CC1101_Poll(&ev) && ev.is_valid) {
            if (ev.frame.src == expected_src &&
                (ev.frame.type == TYPE_ACK || ev.frame.type == TYPE_NACK)) {
                *out_frame = ev.frame;
                return ev.frame.type;
            }
        }
    }
    return 0;
}

uint8_t COM_BuildFrame(const COM_Frame_t *frame, uint8_t *buf, uint8_t *len)
{
    buf[0] = frame->dst;
    buf[1] = frame->type;
    buf[2] = frame->src;
    memcpy(&buf[3], frame->payload, frame->payload_len);
    *len = 3 + frame->payload_len;
    return 1;
}

uint8_t COM_ParseFrame(const uint8_t *buf, uint8_t len, COM_Frame_t *frame)
{
    if (len < 3) return 0;

    frame->dst         = buf[0];
    frame->type        = buf[1];
    frame->src         = buf[2];
    frame->payload_len = len - 3;
    memcpy(frame->payload, &buf[3], frame->payload_len);
    return 1;
}

// ACK & NACK

void COM_SendAck(uint8_t dst, uint8_t src)
{
    COM_Frame_t frame = {
        .dst         = dst,
        .type        = TYPE_ACK,
        .src         = src,
        .payload_len = 0
    };

    uint8_t buf[64];
    uint8_t len;
    COM_BuildFrame(&frame, buf, &len);
    COM_CC1101_Transmit(buf, len);
}

void COM_SendNack(uint8_t dst, uint8_t src, uint8_t reason)
{
    COM_Frame_t frame = {
        .dst         = dst,
        .type        = TYPE_NACK,
        .src         = src,
        .payload_len = 1
    };
    frame.payload[0] = reason;

    uint8_t buf[64];
    uint8_t len;
    COM_BuildFrame(&frame, buf, &len);
    COM_CC1101_Transmit(buf, len);
}