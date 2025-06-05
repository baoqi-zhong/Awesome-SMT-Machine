/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

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
#define VBUS_Pin GPIO_PIN_0
#define VBUS_GPIO_Port GPIOA
#define NTC_Pin GPIO_PIN_4
#define NTC_GPIO_Port GPIOA
#define nOTW_Pin GPIO_PIN_0
#define nOTW_GPIO_Port GPIOB
#define nFAULT_Pin GPIO_PIN_11
#define nFAULT_GPIO_Port GPIOB
#define NRST_AB_Pin GPIO_PIN_6
#define NRST_AB_GPIO_Port GPIOC
#define NRST_CD_Pin GPIO_PIN_12
#define NRST_CD_GPIO_Port GPIOA
#define MicroSwitch_Pin GPIO_PIN_11
#define MicroSwitch_GPIO_Port GPIOC
#define WS2812_Pin GPIO_PIN_3
#define WS2812_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
