#include "events.h"
#include "com.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* ── Log UART ──────────────────────────────────────────── */

static void uart_logf(const char *fmt, ...)
{
    UART_HandleTypeDef *huart = COM_GetUart();
    if (!huart) return;
    char buf[128];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n > 0)
        HAL_UART_Transmit(huart, (uint8_t *)buf, (uint16_t)n, HAL_MAX_DELAY);
}

static const char *direction_str(uint8_t dir)
{
    return (dir == 0) ? "ENTRADA" : "SAIDA";
}

static const char *nack_reason_str(uint8_t reason)
{
    switch (reason) {
        case NACK_CRC_ERROR:     return "CRC_ERROR";
        case NACK_UNKNOWN_TYPE:  return "TIPO_DESCONHECIDO";
        case NACK_ADDR_MISMATCH: return "ENDERECO_INVALIDO";
        default:                 return "DESCONHECIDO";
    }
}

/* ── Serialização de payloads ──────────────────────────── */

static void serialize_authorize(const EVENT_Authorize_t *auth,
                                 uint8_t *buf, uint8_t *len)
{
    memcpy(&buf[0], auth->uid, 7);
    buf[7] = auth->uid_len;
    buf[8] = auth->authorized;
    buf[9] = auth->direction;
    *len = 10;
}

static void serialize_status(const EVENT_Status_t *st,
                              uint8_t *buf, uint8_t *len)
{
    buf[0] = (uint8_t)(st->temp >> 8);
    buf[1] = (uint8_t)(st->temp);
    buf[2] = (uint8_t)(st->humidity >> 8);
    buf[3] = (uint8_t)(st->humidity);
    buf[4] = st->direction;
    buf[5] = st->door_open;
    *len = 6;
}

/* ── Desserialização de payloads ───────────────────────── */

static void deserialize_authorize(const uint8_t *buf,
                                   EVENT_Authorize_t *auth)
{
    memcpy(auth->uid, &buf[0], 7);
    auth->uid_len    = buf[7];
    auth->authorized = buf[8];
    auth->direction  = buf[9];
}

static void deserialize_status(const uint8_t *buf,
                                EVENT_Status_t *st)
{
    st->temp      = (int16_t)((buf[0] << 8) | buf[1]);
    st->humidity  = (uint16_t)((buf[2] << 8) | buf[3]);
    st->direction = buf[4];
    st->door_open = buf[5];
}

/* ── Log de structs ────────────────────────────────────── */

static void log_authorize(const EVENT_Authorize_t *auth)
{
    char uid_hex[22]; /* 7 * "XX " + '\0' */
    int  pos = 0;
    for (uint8_t i = 0; i < auth->uid_len && i < 7; i++)
        pos += snprintf(&uid_hex[pos], sizeof(uid_hex) - (size_t)pos,
                        "%02X ", auth->uid[i]);
    if (pos > 0) uid_hex[pos - 1] = '\0';

    uart_logf("[AUTHORIZE] UID: %s | uid_len=%u | autorizado=%s | direcao=%s\r\n",
              uid_hex,
              (unsigned)auth->uid_len,
              auth->authorized ? "SIM" : "NAO",
              direction_str(auth->direction));
}

static void log_status(const EVENT_Status_t *st)
{
    int16_t t     = st->temp;
    char    tsign = (t < 0) ? '-' : '+';
    if (t < 0) t = -t;

    uart_logf("[STATUS] temp=%c%d.%02d grC | umidade=%u.%02u%%RH | direcao=%s | porta=%s\r\n",
              tsign, (int)(t / 100), (int)(t % 100),
              (unsigned)(st->humidity / 100), (unsigned)(st->humidity % 100),
              direction_str(st->direction),
              st->door_open ? "ABERTA" : "FECHADA");
}

/* ── Envio com retentativas ────────────────────────────── */

#define SEND_TIMEOUT_MS   200u
#define SEND_MAX_RETRIES  3u

static uint8_t send_with_retry(uint8_t dst, uint8_t *buf, uint8_t len,
                                const char *tag)
{
    for (uint8_t attempt = 1; attempt <= SEND_MAX_RETRIES; attempt++) {
        uart_logf("[%s] Tentativa %u/%u -> dst=0x%02X\r\n",
                  tag, attempt, SEND_MAX_RETRIES, dst);
        COM_CC1101_Transmit(buf, len);

        COM_Frame_t resp;
        uint8_t rtype = COM_WaitResponse(dst, &resp, SEND_TIMEOUT_MS);

        if (rtype == TYPE_ACK) {
            uart_logf("[%s] ACK recebido na tentativa %u\r\n", tag, attempt);
            return 1;
        } else if (rtype == TYPE_NACK) {
            uint8_t reason = (resp.payload_len >= 1) ? resp.payload[0] : 0xFF;
            uart_logf("[%s] NACK na tentativa %u — razao: %s (0x%02X)\r\n",
                      tag, attempt, nack_reason_str(reason), reason);
        } else {
            uart_logf("[%s] Timeout na tentativa %u (%ums sem resposta)\r\n",
                      tag, attempt, SEND_TIMEOUT_MS);
        }
    }
    uart_logf("[%s] Falha: sem ACK apos %u tentativas (%ums total)\r\n",
              tag, SEND_MAX_RETRIES, SEND_TIMEOUT_MS * SEND_MAX_RETRIES);
    return 0;
}

uint8_t EVENT_SendAuthorize(uint8_t dst, const EVENT_Authorize_t *auth)
{
    uint8_t payload[10];
    uint8_t payload_len;
    serialize_authorize(auth, payload, &payload_len);

    uart_logf("[SEND] AUTHORIZE src=0x%02X -> dst=0x%02X\r\n", (uint8_t)MY_ADDR, dst);
    log_authorize(auth);

    COM_Frame_t frame = {
        .dst         = dst,
        .type        = TYPE_AUTHORIZE,
        .src         = MY_ADDR,
        .payload_len = payload_len
    };
    memcpy(frame.payload, payload, payload_len);

    uint8_t buf[64];
    uint8_t len;
    COM_BuildFrame(&frame, buf, &len);
    return send_with_retry(dst, buf, len, "AUTHORIZE");
}

uint8_t EVENT_SendStatus(uint8_t dst, const EVENT_Status_t *st)
{
    uint8_t payload[6];
    uint8_t payload_len;
    serialize_status(st, payload, &payload_len);

    uart_logf("[SEND] STATUS src=0x%02X -> dst=0x%02X\r\n", (uint8_t)MY_ADDR, dst);
    log_status(st);

    COM_Frame_t frame = {
        .dst         = dst,
        .type        = TYPE_STATUS,
        .src         = MY_ADDR,
        .payload_len = payload_len
    };
    memcpy(frame.payload, payload, payload_len);

    uint8_t buf[64];
    uint8_t len;
    COM_BuildFrame(&frame, buf, &len);
    return send_with_retry(dst, buf, len, "STATUS");
}

/* ── Recebimento — despacha pelo type ──────────────────── */

void EVENT_Dispatch(const COM_Frame_t *frame)
{
    static const char *type_names[] = { "?", "AUTHORIZE", "STATUS", "ACK", "NACK" };
    const char *tname = (frame->type <= 4) ? type_names[frame->type] : "?";
    uart_logf("[DISPATCH] Frame recebido: src=0x%02X dst=0x%02X tipo=%s\r\n",
              frame->src, frame->dst, tname);

    switch (frame->type)
    {
        case TYPE_AUTHORIZE:
        {
            if (frame->payload_len < 10) {
                uart_logf("[DISPATCH] AUTHORIZE rejeitado: payload curto (%u bytes, min=10) — NACK enviado\r\n",
                          (unsigned)frame->payload_len);
                COM_SendNack(frame->src, frame->dst, NACK_UNKNOWN_TYPE);
                return;
            }
            COM_SendAck(frame->src, frame->dst);
            uart_logf("[DISPATCH] ACK enviado para 0x%02X — processando AUTHORIZE\r\n", frame->src);

            EVENT_Authorize_t auth;
            deserialize_authorize(frame->payload, &auth);
            log_authorize(&auth);

            /*
             * TODO: passar auth para a lógica de negócio
             * ex: ACCESS_CheckUid(&auth);
             */
            break;
        }

        case TYPE_STATUS:
        {
            if (frame->payload_len < 6) {
                uart_logf("[DISPATCH] STATUS rejeitado: payload curto (%u bytes, min=6) — NACK enviado\r\n",
                          (unsigned)frame->payload_len);
                COM_SendNack(frame->src, frame->dst, NACK_UNKNOWN_TYPE);
                return;
            }
            COM_SendAck(frame->src, frame->dst);
            uart_logf("[DISPATCH] ACK enviado para 0x%02X — processando STATUS\r\n", frame->src);

            EVENT_Status_t st;
            deserialize_status(frame->payload, &st);
            log_status(&st);

            /*
             * TODO: passar st para a lógica de negócio
             * ex: MONITOR_UpdateStatus(&st);
             */
            break;
        }

        case TYPE_ACK:
            uart_logf("[DISPATCH] ACK recebido de 0x%02X — confirmacao de envio\r\n", frame->src);
            /* TODO: sinalizar à aplicação que o envio foi confirmado */
            break;

        case TYPE_NACK:
        {
            uint8_t reason = (frame->payload_len >= 1) ? frame->payload[0] : 0xFF;
            uart_logf("[DISPATCH] NACK recebido de 0x%02X — razao: %s (0x%02X)\r\n",
                      frame->src, nack_reason_str(reason), reason);
            /* TODO: tratar retransmissão ou sinalizar erro */
            break;
        }

        default:
            uart_logf("[DISPATCH] Tipo desconhecido: 0x%02X — NACK enviado para 0x%02X\r\n",
                      frame->type, frame->src);
            COM_SendNack(frame->src, frame->dst, NACK_UNKNOWN_TYPE);
            break;
    }
}
