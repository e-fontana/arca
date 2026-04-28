#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <stdint.h>

#define DIRECTION_ENTRY 0u
#define DIRECTION_EXIT  1u

#define MAX_USER_LEVEL  2u

typedef struct {
    uint8_t uid[7];
    uint8_t uid_len;
    uint8_t direction;
} NFC_Package_t;

#endif