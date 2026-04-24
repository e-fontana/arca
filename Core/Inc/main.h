/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define INT_NFC_Pin GPIO_PIN_1
#define INT_NFC_GPIO_Port GPIOA
#define INT_NFC_EXTI_IRQn EXTI1_IRQn
#define NSS_NFC_Pin GPIO_PIN_3
#define NSS_NFC_GPIO_Port GPIOA
#define NFC_RESET_Pin GPIO_PIN_2
#define NFC_RESET_GPIO_Port GPIOB
#define INT_CC1101_Pin GPIO_PIN_15
#define INT_CC1101_GPIO_Port GPIOB
#define INT_CC1101_EXTI_IRQn EXTI15_10_IRQn
#define NSS_TEMP_Pin GPIO_PIN_9
#define NSS_TEMP_GPIO_Port GPIOA
#define NSS_CC1101_Pin GPIO_PIN_10
#define NSS_CC1101_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
