#include "rtc_api.h"
#include <stdio.h>
#include <string.h>

static const char* weekday_str[] = {
    "", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

void RTC_SendStatus(UART_HandleTypeDef *huart)
{
    RTC_ReadCurrent();

    char json[128];
    snprintf(json, sizeof(json),
        "{"
        "\"hours\":%d,"
        "\"minutes\":%d,"
        "\"seconds\":%d,"
        "\"day\":%d,"
        "\"month\":%d,"
        "\"year\":%d,"
        "\"weekday\":\"%s\","
        "\"is_valid\":%d"
        "}\n",
        rtc_status.hours,
        rtc_status.minutes,
        rtc_status.seconds,
        rtc_status.day,
        rtc_status.month,
        rtc_status.year,
        weekday_str[rtc_status.weekday],
        rtc_status.is_valid
    );

    HAL_UART_Transmit(huart, (uint8_t*)json, strlen(json), 200);
}

void RTC_API_Process(UART_HandleTypeDef *huart)
{
    /* Sem dado disponível — limpa erros pendentes com segurança (RXNE=0, DR sem dado válido) */
    if (!__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE))
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) ||
            __HAL_UART_GET_FLAG(huart, UART_FLAG_FE)  ||
            __HAL_UART_GET_FLAG(huart, UART_FLAG_NE))
        {
            __HAL_UART_CLEAR_OREFLAG(huart);
            huart->ErrorCode = HAL_UART_ERROR_NONE;
            huart->RxState   = HAL_UART_STATE_READY;
        }
        return;
    }

    /* Há byte no DR — lê agora */
    uint8_t cmd = 0;
    if (HAL_UART_Receive(huart, &cmd, 1, 100) != HAL_OK)
    {
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->RxState   = HAL_UART_STATE_READY;
        return;
    }

    /* Diagnóstico: imprime o byte recebido em hex */
    char dbg[16];
    snprintf(dbg, sizeof(dbg), "RX:0x%02X\r\n", cmd);
    HAL_UART_Transmit(huart, (uint8_t *)dbg, strlen(dbg), 100);

    if (cmd == '\r' || cmd == '\n' || cmd == '\0')
        return;

    if (cmd == CMD_GET_TIME)
    {
        HAL_UART_Transmit(huart, (uint8_t *)"Command received: G\n", 20, 200);

        RTC_ReadCurrent();

        char json[128];
        snprintf(json, sizeof(json),
            "{"
            "\"hours\":%d,"
            "\"minutes\":%d,"
            "\"seconds\":%d,"
            "\"day\":%d,"
            "\"month\":%d,"
            "\"year\":%d,"
            "\"weekday\":\"%s\","
            "\"is_valid\":%d"
            "}\n",
            rtc_status.hours,
            rtc_status.minutes,
            rtc_status.seconds,
            rtc_status.day,
            rtc_status.month,
            rtc_status.year,
            weekday_str[rtc_status.weekday],
            rtc_status.is_valid
        );

        HAL_UART_Transmit(huart, (uint8_t*)json, strlen(json), 200);
    }
    else if (cmd == CMD_SET_TIME)
    {
        /* Avisa que está pronto para receber o payload — Python aguarda este ACK */
        HAL_UART_Transmit(huart, (uint8_t *)"Command received: S\n", 20, 200);

        uint8_t buf[6];
        if (HAL_UART_Receive(huart, buf, 6, 500) == HAL_OK)
        {
            RTC_SetFromValues(buf[0], buf[1], buf[2],
                              buf[3], buf[4], buf[5],
                              RTC_WEEKDAY_MONDAY);
            uint8_t ack = 0x06;
            HAL_UART_Transmit(huart, &ack, 1, 100);
        }
    }
}