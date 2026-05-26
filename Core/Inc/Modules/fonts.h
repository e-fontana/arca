/* Core/Inc/fonts.h */
#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

typedef struct {
    const uint8_t width;
    const uint8_t height;
    const uint8_t *data; // <--- Mudou de uint16_t para uint8_t
} FontDef;

extern FontDef Font_5x8;

#endif