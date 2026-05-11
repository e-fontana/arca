/*
 * dw_stm32_delay.h
 *
 *  Created on: Mar 11, 2020
 *      Author: suleyman.eskil
 */

#ifndef INC_DW_STM32_DELAY_H_
#define INC_DW_STM32_DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"

uint32_t DWT_Delay_Init(void);

__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
{
  uint32_t clk_cycle_start = DWT->CYCCNT;
  microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);
  while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

#ifdef __cplusplus
}
#endif

#endif /* INC_DW_STM32_DELAY_H_ */
