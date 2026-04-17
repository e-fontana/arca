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
#define CLK_IN_SYS_Pin GPIO_PIN_0
#define CLK_IN_SYS_GPIO_Port GPIOH
#define CLK_OUT_SYS_Pin GPIO_PIN_1
#define CLK_OUT_SYS_GPIO_Port GPIOH
#define ADC_SENSOR_TEMP_Pin GPIO_PIN_4
#define ADC_SENSOR_TEMP_GPIO_Port GPIOA
#define SPI_CLK_Pin GPIO_PIN_5
#define SPI_CLK_GPIO_Port GPIOA
#define SPI_RECEIVER_Pin GPIO_PIN_6
#define SPI_RECEIVER_GPIO_Port GPIOA
#define SPI_SENDER_Pin GPIO_PIN_7
#define SPI_SENDER_GPIO_Port GPIOA
#define NSS_CC1101_Pin GPIO_PIN_9
#define NSS_CC1101_GPIO_Port GPIOA
#define NSS_2_Pin GPIO_PIN_10
#define NSS_2_GPIO_Port GPIOA
#define NSS_1_Pin GPIO_PIN_11
#define NSS_1_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
