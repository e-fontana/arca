#ifndef CONTROLLER_FSM_H
#define CONTROLLER_FSM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
typedef enum {
    STATE_IDLE,
    STATE_MENU_PC,

    /* sub-estados do cadastro */
    STATE_CADASTRO_RECEBE_DADOS,
    STATE_CADASTRO_AGUARDA_NFC,
    STATE_CADASTRO_VERIFICA_DUPLICATA,
    STATE_CADASTRO_GRAVA_FLASH,
    STATE_CADASTRO_CONFIRMA_PC,

   /* outros menus (esqueleto por enquanto) */ 
   STATE_DELETAR,
   STATE_EXPORTAR_LOGS,

   /* validação de acesso RF */
   STATE_VALIDANDO_ACESSO,
} Controller_State;

typedef struct {
    Controller_State state;

    /* buffer de recepção UART (1 byte por vez via interrupção) */
    uint8_t uart_rx_byte;
    uint8_t uart_line[128];
    uint8_t uart_line_len;
    uint8_t uart_line_ready;

    /* dados em trânsito do cadastro */
    char    cadastro_nome[32];
    uint8_t cadastro_nivel;
    uint8_t cadastro_uid[7]; // UID NFC (até 7 bytes)
    uint8_t cadastro_uid_len;

    /* handle da UART - preenchido no main */
    UART_HandleTypeDef *huart;

    /* timestamp de entrada no estado atual (para timeouts) */
    uint32_t state_entry_tick;
} Controller_Context;

/* Timeouts (ms) */
#define TIMEOUT_AGUARDA_NFC 5000u

/* Comandos do protocolo serial */
#define CMD_CANCELAR    0xFFu

/* API Pública */
void Controller_Init(Controller_Context *ctx, UART_HandleTypeDef *huart);
void Controller_Run(Controller_Context *ctx);
void Controller_UART_RxCallback(Controller_Context *ctx);

#endif /* CONTROLLER_FSM_H */