#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include "com.h"

/* ── Endereços dos nós ─────────────────────────────────── */
#define ADDR_ROOM_1     0x01
#define ADDR_ROOM_2     0x02
#define ADDR_BROADCAST  0x00

/* ── Tipos de mensagem ─────────────────────────────────── */
#define TYPE_AUTHORIZE_REQUEST  0x01
#define TYPE_AUTHORIZE_RESPONSE 0x02
#define TYPE_STATUS_REQUEST     0x03
#define TYPE_STATUS_RESPONSE    0x04
#define TYPE_ACK                0x05
#define TYPE_NACK               0x06

/* ── Códigos NACK ──────────────────────────────────────── */
#define NACK_CRC_ERROR      0x01
#define NACK_UNKNOWN_TYPE   0x02
#define NACK_ADDR_MISMATCH  0x03

/* ── Payloads ──────────────────────────────────────────── */
typedef struct {
    uint8_t uid[7];
    uint8_t uid_len;
    uint8_t authorized;
    uint8_t direction;
} EVENT_Authorize_t;

typedef struct {
    int16_t  temp;        /* 0.01 °C  — ex: 2350 = 23.50 °C */
    uint16_t humidity;    /* 0.01 %RH — ex: 6010 = 60.10 %  */
    uint8_t  direction;
    uint8_t  door_open;
} EVENT_Status_Response_t;

/* Retornam 1 em caso de ACK, 0 em caso de falha após todas as tentativas. */
uint8_t EVENT_SendAuthorizeRequest(uint8_t dst, const EVENT_Authorize_t *auth);
uint8_t EVENT_SendAuthorizeResponse(uint8_t dst, const EVENT_Authorize_t *auth);
uint8_t EVENT_SendStatusResponse(uint8_t dst, const EVENT_Status_Response_t *st);

void EVENT_Dispatch(const COM_Frame_t *frame);

#endif