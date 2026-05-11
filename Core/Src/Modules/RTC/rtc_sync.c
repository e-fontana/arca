#include "rtc_sync.h"

volatile RTC_Status_t rtc_status = {0};

uint8_t RTC_IsValid(void)
{
    uint32_t magic = HAL_RTCEx_BKUPRead(&hrtc, RTC_BACKUP_REG);
    if (magic == RTC_MAGIC_NUMBER)
    {
        rtc_status.had_backup = 1;
        rtc_status.is_valid   = 1;
        return 1;
    }
    rtc_status.had_backup = 0;
    rtc_status.is_valid   = 0;
    return 0;
}

void RTC_SetFromValues(uint8_t h, uint8_t m, uint8_t s,
                       uint8_t day, uint8_t month, uint8_t year_2d,
                       uint8_t weekday)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours          = h;
    sTime.Minutes        = m;
    sTime.Seconds        = s;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    sDate.WeekDay = weekday;
    sDate.Month   = month;
    sDate.Date    = day;
    sDate.Year    = year_2d;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BACKUP_REG, RTC_MAGIC_NUMBER);

    rtc_status.is_valid = 1;
}

void RTC_ReadCurrent(void)
{
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;

    if (HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN) == HAL_OK &&
        HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN) == HAL_OK)
    {
        rtc_status.hours   = t.Hours;
        rtc_status.minutes = t.Minutes;
        rtc_status.seconds = t.Seconds;
        rtc_status.day     = d.Date;
        rtc_status.month   = d.Month;
        rtc_status.year    = 2000 + d.Year;
        rtc_status.weekday = d.WeekDay;
    }
}

void RTC_SyncFromUART(UART_HandleTypeDef *huart)
{
    uint8_t buf[7];
    uint8_t ack;

    if (HAL_UART_Receive(huart, buf, 7, 5000) == HAL_OK && buf[0] == 'S')
    {
        RTC_SetFromValues(buf[1], buf[2], buf[3],
                          buf[4], buf[5], buf[6],
                          RTC_WEEKDAY_MONDAY);

        ack = 0x06;
        HAL_UART_Transmit(huart, &ack, 1, 100);
    }
    else
    {
        ack = 0x15;
        HAL_UART_Transmit(huart, &ack, 1, 100);
    }
}