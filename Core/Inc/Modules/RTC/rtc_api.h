#ifndef RTC_API_H
#define RTC_API_H

#include "stm32f4xx_hal.h"
#include "rtc_sync.h"

#define CMD_GET_TIME  'G'   /* 0x47 — retorna JSON com data/hora */
#define CMD_SET_TIME  'S'   /* 0x53 — seta hora (protocolo já existente) */

void RTC_API_Process(UART_HandleTypeDef *huart);
void RTC_SendStatus(UART_HandleTypeDef *huart);

#endif