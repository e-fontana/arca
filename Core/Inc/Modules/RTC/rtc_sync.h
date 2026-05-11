#ifndef RTC_SYNC_H
#define RTC_SYNC_H

#include "main.h"

#define RTC_MAGIC_NUMBER  0xA55A
#define RTC_BACKUP_REG    RTC_BKP_DR0

typedef struct {
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
    uint8_t  weekday;
    uint8_t  is_valid;
    uint8_t  had_backup;
} RTC_Status_t;

extern volatile RTC_Status_t rtc_status;

uint8_t RTC_IsValid(void);
void    RTC_SetFromValues(uint8_t h, uint8_t m, uint8_t s,
                          uint8_t day, uint8_t month, uint8_t year_2d,
                          uint8_t weekday);
void    RTC_ReadCurrent(void);
void    RTC_SyncFromUART(UART_HandleTypeDef *huart);

#endif