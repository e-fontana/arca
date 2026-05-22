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

void RTC_API_Process(void)
{
    Protocol_Frame_t frame;
    if (!Protocol_Receive(&frame) || frame.length == 0) return;

    uint8_t cmd = frame.data[0];

    if (cmd == CMD_GET_TIME)
    {
        RTC_ReadCurrent();
        uint8_t year_2d = (uint8_t)(rtc_status.year > 2000
                                    ? rtc_status.year - 2000
                                    : (uint8_t)rtc_status.year);
        uint8_t resp[8] = {
            rtc_status.hours,
            rtc_status.minutes,
            rtc_status.seconds,
            rtc_status.day,
            rtc_status.month,
            year_2d,
            rtc_status.weekday,
            rtc_status.is_valid
        };
        Protocol_Send(resp, 8);
    }
    else if (cmd == CMD_SET_TIME && frame.length == 8)
    {
        /* data: [S, HH, MM, SS, DD, Mo, YY, weekday] */
        RTC_SetFromValues(
            frame.data[1], frame.data[2], frame.data[3],
            frame.data[4], frame.data[5], frame.data[6],
            frame.data[7]
        );
        uint8_t ack = 0x06;
        Protocol_Send(&ack, 1);
    }
}
