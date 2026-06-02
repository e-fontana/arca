#ifndef RTC_API_H
#define RTC_API_H

#include "stm32f4xx_hal.h"
#include "rtc_sync.h"
#include "uart_protocol.h"

#define CMD_GET_TIME  'G'   /* 0x47 */
#define CMD_SET_TIME  'S'   /* 0x53 */

/* Processa um frame recebido via Protocol_Receive() */
void RTC_API_Process(void);

/* Debug: envia JSON do RTC atual via UART diretamente */
void RTC_SendStatus(UART_HandleTypeDef *huart);

#endif