#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define PROTO_START     0xAA
#define PROTO_STOP      0x55
#define PROTO_MAX_DATA  57   /* 61 (CC1101 max) - 4 bytes de framing */

typedef struct {
    uint8_t data[PROTO_MAX_DATA];
    uint8_t length;
    uint8_t valid;
} Protocol_Frame_t;

/* Arma a recepção por interrupção — chamar após MX_USART1_UART_Init */
void    Protocol_Init(UART_HandleTypeDef *huart);

/* Constrói e envia frame via UART */
uint8_t Protocol_Send(const uint8_t *data, uint8_t length);

/* Retorna 1 e copia o frame se houver um completo disponível */
uint8_t Protocol_Receive(Protocol_Frame_t *dst);

/* Chamar dentro de HAL_UART_RxCpltCallback */
void    Protocol_UART_RxCallback(void);

#endif /* PROTOCOL_H */
