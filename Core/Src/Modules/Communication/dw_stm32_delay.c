/*
 * dw_stm32_delay.c
 *
 *  Created on: Mar 11, 2020
 *      Author: suleyman.eskil
 */
#include "dw_stm32_delay.h"

uint32_t DWT_Delay_Init(void) {
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;

  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
  DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;

  DWT->CYCCNT = 0;

  __ASM volatile ("NOP");
  __ASM volatile ("NOP");
  __ASM volatile ("NOP");

  if (DWT->CYCCNT)
    return 0;
  else
    return 1;
}
