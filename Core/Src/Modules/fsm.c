#include "fsm.h"
#include "nfc.h"
#include "stm32f4xx_hal_uart.h"
#include "system_types.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void _uart_send(Controller_Context *ctx, const char *msg)
{
    HAL_UART_Transmit(ctx->huart, (uint8_t *)msg, strlen(msg), 100);
}

static void _set_state(Controller_Context *ctx, Controller_State new_state)
{
    ctx->state = new_state;
    ctx->state_entry_tick = HAL_GetTick();
}

static uint8_t _timeout_expired(Controller_Context *ctx, uint32_t timeout_ms)
{
    return (HAL_GetTick() - ctx->state_entry_tick) >= timeout_ms;
}

static uint8_t _cancel_requested(Controller_Context *ctx)
{
    if (ctx->uart_line_ready &&
        ctx->uart_line_len == 1 &&
        ctx->uart_line[0] == CMD_CANCELAR)
    {
        ctx->uart_line_ready = 0;
        ctx->uart_line_len = 0;
        return 1u;
    }
    return 0u;
}

static uint8_t _parse_dados_cadastro(Controller_Context *ctx)
{
    char nome_buf[32] = {0};
    unsigned int nivel_uint = 0;  /* unsigned int em vez de uint8_t */

    int parsed = sscanf((char *)ctx->uart_line,
                        "NOME:%31[^;];NIVEL:%u",  /* %u em vez de %hhu */
                        nome_buf,
                        &nivel_uint);

    uint8_t nivel_buf = (uint8_t)nivel_uint;  /* cast após leitura */

    if (parsed == 2 && strlen(nome_buf) > 0 && nivel_buf >= 1 && nivel_buf <= MAX_USER_LEVEL)
    {
        strncpy(ctx->cadastro_nome, nome_buf, sizeof(ctx->cadastro_nome) - 1);
        ctx->cadastro_nivel = nivel_buf;
        return 1u;
    }

    return 0u;
}

void Controller_Init(Controller_Context *ctx, UART_HandleTypeDef *huart)
{
    memset(ctx, 0u, sizeof(Controller_Context));
    ctx->huart = huart;
    _set_state(ctx, STATE_IDLE);

    /* Arma a primeira recepção por interrupção (1 byte) */
    HAL_UART_Receive_IT(ctx->huart, &ctx->uart_rx_byte, 1);
}

void Controller_UART_RxCallback(Controller_Context *ctx)
{
    uint8_t byte = ctx->uart_rx_byte;

    if (byte == '\n' || byte == '\r')
    {
        if (ctx->uart_line_len > 0)
        {
            ctx->uart_line[ctx->uart_line_len] = '\0'; // termina a string
            ctx->uart_line_ready = 1;
        }
    }
    else if (byte == CMD_CANCELAR)
    {
        ctx->uart_line[0] = CMD_CANCELAR;
        ctx->uart_line_len = 1;
        ctx->uart_line_ready = 1;
    }
    else
    {
        if (ctx->uart_line_len < sizeof(ctx->uart_line) - 1)
        {
            ctx->uart_line[ctx->uart_line_len++] = byte;
        }
    }

    HAL_UART_Receive_IT(ctx->huart, &ctx->uart_rx_byte, 1);
}

void Controller_Run(Controller_Context *ctx)
{
    switch (ctx->state)
    {
        case STATE_IDLE:
            if (ctx->uart_line_ready)
            {
                ctx->uart_line_ready = 0;
                ctx->uart_line_len = 0;
                memset(ctx->uart_line, 0, sizeof(ctx->uart_line));
                _uart_send(ctx, "ARCA:MENU\n");
                _set_state(ctx, STATE_MENU_PC);
            }

            /* TODO: checar requisições de acesso recebidas via tag NFC aqui */
            break;

        case STATE_MENU_PC:
            if (!ctx->uart_line_ready) break;

            if (strcmp((char *)ctx->uart_line, "CMD:0") == 0)
            {
                _uart_send(ctx, "ARCA:DESCONECTADO\n");
                _set_state(ctx, STATE_IDLE);
            }
            else if (strcmp((char *)ctx->uart_line, "CMD:1") == 0)
            {
                _uart_send(ctx, "CADASTRO:AGUARDA_DADOS\n");
                memset(ctx->cadastro_nome, 0, sizeof(ctx->cadastro_nome));
                ctx->cadastro_nivel = 0;
                _set_state(ctx, STATE_CADASTRO_RECEBE_DADOS);
            }
            else if (strcmp((char *)ctx->uart_line, "CMD:2") == 0)
            {
                _set_state(ctx, STATE_DELETAR);
            }
            else if (strcmp((char *)ctx->uart_line, "CMD:3") == 0)
            {
                _set_state(ctx, STATE_EXPORTAR_LOGS);
            }

            ctx->uart_line_ready = 0;
            ctx->uart_line_len = 0;
            break;

        case STATE_CADASTRO_RECEBE_DADOS:
            if (_cancel_requested(ctx))
            {
                _uart_send(ctx, "CADASTRO:CANCELADO\n");
                _set_state(ctx, STATE_MENU_PC);
                break;
            }

            if (!ctx->uart_line_ready) break;

            if (_parse_dados_cadastro(ctx))
            {
                _uart_send(ctx, "CADASTRO:AGUARDA_NFC\n");
                NFC_StartRead();
                _set_state(ctx, STATE_CADASTRO_AGUARDA_NFC);
            }
            else
            {
                _uart_send(ctx, "CADASTRO:ERR_DADOS\n");
            }
            
            ctx->uart_line_ready = 0;
            ctx->uart_line_len = 0;
            break;
        case STATE_CADASTRO_AGUARDA_NFC:
            if (_cancel_requested(ctx))
            {
                NFC_StopRead();
                _uart_send(ctx, "CADASTRO:CANCELADO\n");
                _set_state(ctx, STATE_MENU_PC);
                break;
            }

            if (_timeout_expired(ctx, TIMEOUT_AGUARDA_NFC))
            {
                NFC_StopRead();
                _uart_send(ctx, "CADASTRO:NFC_TIMEOUT\n");
                _set_state(ctx, STATE_CADASTRO_RECEBE_DADOS);
                break;
            }

            if (pn532_card_ready)
            {
                pn532_card_ready = 0u;
                PN532_Card_t card;
                if (NFC_GetCard(&card))
                {
                    NFC_StopRead();
                    memcpy(ctx->cadastro_uid, card.uid, card.uid_len);
                    ctx->cadastro_uid_len = card.uid_len;
                    _uart_send(ctx, "CADASTRO:NFC_OK\n");
                    _set_state(ctx, STATE_CADASTRO_VERIFICA_DUPLICATA);
                }
            }
            break;
        case STATE_CADASTRO_VERIFICA_DUPLICATA:
            /* TODO */
            _set_state(ctx, STATE_CADASTRO_GRAVA_FLASH);
            break;
        case STATE_CADASTRO_GRAVA_FLASH:
            /* TODO */
            _set_state(ctx, STATE_CADASTRO_CONFIRMA_PC);
            break;
        case STATE_CADASTRO_CONFIRMA_PC:
            _uart_send(ctx, "ACK:OK\n");
            _set_state(ctx, STATE_MENU_PC);
            break;

        case STATE_DELETAR:
            /* TODO */
            _set_state(ctx, STATE_MENU_PC);
            break;
        case STATE_EXPORTAR_LOGS:
            /* TODO */
            _set_state(ctx, STATE_MENU_PC);
            break;
        
        case STATE_VALIDANDO_ACESSO:
            /* TODO */
            _set_state(ctx, STATE_IDLE);
            break;
        
        default:
            _set_state(ctx, STATE_IDLE);
            break;
    }
}
