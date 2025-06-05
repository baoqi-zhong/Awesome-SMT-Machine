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
#define BUTTON0_Pin GPIO_PIN_13
#define BUTTON0_GPIO_Port GPIOC
#define TriggerY_Pin GPIO_PIN_14
#define TriggerY_GPIO_Port GPIOC
#define WS2812_TIM_Pin GPIO_PIN_0
#define WS2812_TIM_GPIO_Port GPIOA
#define PUMP_Pin GPIO_PIN_1
#define PUMP_GPIO_Port GPIOA
#define SERVO_TIM_Pin GPIO_PIN_2
#define SERVO_TIM_GPIO_Port GPIOA
#define LCD_RST_Pin GPIO_PIN_3
#define LCD_RST_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define LCD_DC_Pin GPIO_PIN_4
#define LCD_DC_GPIO_Port GPIOC
#define LCD_BG_Pin GPIO_PIN_0
#define LCD_BG_GPIO_Port GPIOB
#define LED_ACT_Pin GPIO_PIN_14
#define LED_ACT_GPIO_Port GPIOB
#define APM_PDN_Pin GPIO_PIN_6
#define APM_PDN_GPIO_Port GPIOC
#define APM_ADR_Pin GPIO_PIN_10
#define APM_ADR_GPIO_Port GPIOA
#define PSTOP_Pin GPIO_PIN_9
#define PSTOP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
