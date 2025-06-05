/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "cordic.h"
#include "dma.h"
#include "fdcan.h"
#include "opamp.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fdcan.h"
#include "math.h"
#include "stdint.h"

#include "FOC.h"
#include "ErrorHandler.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t signal500Hz = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FDCAN1_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_ADC2_Init();
  MX_CORDIC_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  // FLASH_OBProgramInitTypeDef pOBInit;
  // HAL_FLASHEx_OBGetConfig(&pOBInit);
  // if (!((pOBInit.USERConfig & FLASH_OPTR_nSWBOOT0_Msk) == OB_BOOT0_FROM_OB &&
  //       (pOBInit.USERConfig & FLASH_OPTR_nBOOT0_Msk) == OB_nBOOT0_SET &&
  //       (pOBInit.USERConfig & FLASH_OPTR_NRST_MODE_Msk) == OB_NRST_MODE_GPIO &&
  //       (pOBInit.USERConfig & FLASH_OPTR_BOR_LEV_Msk) == OB_BOR_LEVEL_4 &&
  //       (pOBInit.USERConfig & FLASH_OPTR_nRST_STOP_Msk) == OB_STOP_RST &&
  //       (pOBInit.USERConfig & FLASH_OPTR_nRST_STDBY_Msk) == OB_STANDBY_RST &&
  //       (pOBInit.USERConfig & FLASH_OPTR_nRST_SHDW_Msk) == OB_SHUTDOWN_RST
  // )) {
  //     __disable_irq();
  //     HAL_FLASH_Unlock();
  //     HAL_FLASH_OB_Unlock();

  //     pOBInit.OptionType = OPTIONBYTE_USER;
  //     pOBInit.USERType = 0;
  //     pOBInit.USERConfig = 0;

  //     pOBInit.USERType |= OB_USER_nSWBOOT0;
  //     pOBInit.USERConfig |= OB_BOOT0_FROM_OB;
  //     pOBInit.USERType |= OB_USER_nBOOT0;
  //     pOBInit.USERConfig |= OB_nBOOT0_SET;
  //     pOBInit.USERType |= OB_USER_NRST_MODE;
  //     pOBInit.USERConfig |= OB_NRST_MODE_GPIO;
  //     pOBInit.USERType |= OB_USER_BOR_LEV;
  //     pOBInit.USERConfig |= OB_BOR_LEVEL_4;
  //     pOBInit.USERType |= OB_USER_nRST_STOP;
  //     pOBInit.USERConfig |= OB_STOP_RST;
  //     pOBInit.USERType |= OB_USER_nRST_STDBY;
  //     pOBInit.USERConfig |= OB_STANDBY_RST;
  //     pOBInit.USERType |= OB_USER_nRST_SHDW;
  //     pOBInit.USERConfig |= OB_SHUTDOWN_RST;
  //     pOBInit.USERType |= OB_USER_IRHEN;
  //     pOBInit.USERConfig |= OB_IRH_ENABLE;
  //     HAL_FLASHEx_OBProgram(&pOBInit);
  //     HAL_FLASH_OB_Launch();
  //     HAL_FLASH_OB_Lock();
  //     HAL_FLASH_Lock();
  //     __enable_irq();
  // }

  controlStatus.initializing = 1;
  HAL_GPIO_WritePin(VALVE_1_GPIO_Port, VALVE_1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(VALVE_2_GPIO_Port, VALVE_2_Pin, GPIO_PIN_RESET);


  ErrorHandlerInit();
  controlInit();

  HAL_OPAMP_Start(&hopamp1);
  HAL_OPAMP_Start(&hopamp2);
  HAL_OPAMP_Start(&hopamp3);

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

  HAL_ADC_Start(&hadc2);
  extern uint32_t adcBuffer[2];
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adcBuffer, 2);

  disableFOC();

  // 这样做的目的是修改TIM1 Update Event 的相�?
  HAL_TIM_Base_Start_IT(&htim1);
  htim1.Instance->RCR = 1;
  // HAL_TIM_Base_Start(&htim3);
  // HAL_TIM_Base_Start(&htim2);
  // HAL_TIM_Base_Start(&htim8);
  // HAL_TIM_Base_Start(&htim4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  HAL_Delay(50); // 上电顺序

  resetAndEnableFOC();
  controlStatus.initializing = 0;



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    for(int i = 0; i < 3; i++)
    {
      moveMotor(i, 1);
      HAL_Delay(1000);
    }
    for(int i = 0; i < 3; i++)
    {
      moveMotor(i, -1);
      HAL_Delay(1000);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
