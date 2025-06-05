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
#define M1_nFAULT_Pin GPIO_PIN_13
#define M1_nFAULT_GPIO_Port GPIOC
#define VALVE_2_Pin GPIO_PIN_14
#define VALVE_2_GPIO_Port GPIOC
#define GPIO3_Pin GPIO_PIN_15
#define GPIO3_GPIO_Port GPIOC
#define GPIO1_Pin GPIO_PIN_0
#define GPIO1_GPIO_Port GPIOA
#define MICRO_SW_Pin GPIO_PIN_4
#define MICRO_SW_GPIO_Port GPIOA
#define VBUS_SENSE_Pin GPIO_PIN_11
#define VBUS_SENSE_GPIO_Port GPIOB
#define GPIO2_Pin GPIO_PIN_12
#define GPIO2_GPIO_Port GPIOB
#define VALVE_1_Pin GPIO_PIN_13
#define VALVE_1_GPIO_Port GPIOB
#define M2_nFAULT_Pin GPIO_PIN_4
#define M2_nFAULT_GPIO_Port GPIOB
#define M3_nFAULT_Pin GPIO_PIN_5
#define M3_nFAULT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
