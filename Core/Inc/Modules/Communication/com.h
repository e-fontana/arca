#ifndef MODULES_COM_H
#define MODULES_COM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ── Tamanhos ──────────────────────────────────────────── */
#define COM_CC1101_MAX_PAYLOAD_LEN  64u
#define COM_CC1101_MAX_MESSAGE_LEN  128u

/* ── Endereços ─────────────────────────────────────────── */
#define ADDR_ROOM_1     0x01
#define ADDR_ROOM_2     0x02
#define ADDR_BROADCAST  0x00

/* ── Tipos de mensagem ─────────────────────────────────── */
#define TYPE_AUTHORIZE  0x01
#define TYPE_STATUS     0x02
#define TYPE_ACK        0x03
#define TYPE_NACK       0x04

/* ── Códigos NACK ──────────────────────────────────────── */
#define NACK_CRC_ERROR      0x01
#define NACK_UNKNOWN_TYPE   0x02
#define NACK_ADDR_MISMATCH  0x03

/* ── Endereço deste nó ─────────────────────────────────── */
#if !defined(ROOM_ID)
    #error "ROOM_ID não definido — compile com -DROOM_ID=0, -DROOM_ID=1 ou -DROOM_ID=2"
#endif
#if ROOM_ID == 0
    #define MY_ADDR  ADDR_BROADCAST
#elif ROOM_ID == 1
    #define MY_ADDR  ADDR_ROOM_1
#elif ROOM_ID == 2
    #define MY_ADDR  ADDR_ROOM_2
#endif

/* ── Structs de payload ────────────────────────────────── */
typedef struct {
    uint8_t uid[7];
    uint8_t uid_len;
    uint8_t authorized;
    uint8_t direction;
    uint8_t target_id[4];
} COM_Payload_Authorize_t;

typedef struct {
    int16_t  temp;        /* 0.01 °C  */
    uint16_t humidity;    /* 0.01 %RH */
    uint8_t  direction;
    uint8_t  door_open;
    uint8_t  reset_id;
} COM_Payload_Status_t;

/* ── Frame de protocolo ────────────────────────────────── */
typedef struct {
    uint8_t dst;
    uint8_t type;
    uint8_t src;
    uint8_t payload[52];
    uint8_t payload_len;
} COM_Frame_t;

/* ── Evento de recepção ────────────────────────────────── */
typedef struct {
    uint8_t     has_packet;
    uint8_t     is_valid;
    uint8_t     gdo0;
    uint8_t     marcstate;
    uint8_t     rxbytes;
    uint8_t     payload_len;
    uint8_t     payload[COM_CC1101_MAX_PAYLOAD_LEN];
    COM_Frame_t frame;
    char        message[COM_CC1101_MAX_MESSAGE_LEN];
} COM_CC1101_RxEvent_t;

/* ── API do driver ─────────────────────────────────────── */
void    COM_CC1101_Init(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart);
void    COM_CC1101_Transmit(const uint8_t *data, uint8_t length);
uint8_t COM_CC1101_Receive(uint8_t *buf, uint8_t *len);
void    COM_CC1101_HandleInterrupt(uint16_t gpio_pin);
uint8_t COM_CC1101_ProcessInterrupt(COM_CC1101_RxEvent_t *event);
uint8_t COM_CC1101_Poll(COM_CC1101_RxEvent_t *event);
UART_HandleTypeDef *COM_GetUart(void);
/* Aguarda ACK ou NACK de expected_src por até timeout_ms.
   Retorna TYPE_ACK, TYPE_NACK, ou 0 em caso de timeout. */
uint8_t COM_WaitResponse(uint8_t expected_src, COM_Frame_t *out_frame, uint32_t timeout_ms);

/* ── API de protocolo ──────────────────────────────────── */
uint8_t COM_BuildFrame(const COM_Frame_t *frame, uint8_t *buf, uint8_t *len);
uint8_t COM_ParseFrame(const uint8_t *buf, uint8_t len, COM_Frame_t *frame);
void    COM_SendAck(uint8_t dst, uint8_t src);
void    COM_SendNack(uint8_t dst, uint8_t src, uint8_t reason);

#endif /* MODULES_COM_H */