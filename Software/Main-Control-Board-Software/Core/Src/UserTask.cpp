/**
 * @file UserTask.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "AppConfig.h"  // Include our customized configuration
#include "FreeRTOS.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "task.h"
#include "tick.hpp"
#include "tim.h"
#include "usart.h"
#include "i2s.h"

// Include private drivers
#include "ACM8625S.hpp"
#include "MotorManager.hpp"
#include "RGBEffect.hpp"
#include "SC8815.hpp"
#include "ws2812.hpp"
#include "GCodeDecoder.hpp"
#include "FDCANManager.hpp"
#include "CoreXY.hpp"
#include "Servo.hpp"
#include "UIManager.hpp"


#define UARTRxBufferSize 32
static uint8_t UARTRxData[UARTRxBufferSize];
uint8_t txData[1];
int16_t setSMTLightEffectRainbowAfterDelay = -1;


StackType_t uxPowerManagementTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xPowerManagementTaskTCB;

StackType_t uxMainTaskStack[512];
StaticTask_t xMainTaskTCB;

void mainTask(void *pvPara)
{
    // HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    Tick_Init();
    WS2812_Init();
    vTaskDelay(1);

    HAL_GPIO_WritePin(LCD_BG_GPIO_Port, LCD_BG_Pin, GPIO_PIN_RESET);
    UIManager::init();

    RGBeffect::startLightEffect();


    while (1)
    {
        UIManager::update();

        // 在一段时间之后切换到彩虹灯效果
        if(setSMTLightEffectRainbowAfterDelay != -1)
        {
            setSMTLightEffectRainbowAfterDelay -= 100;
            if(setSMTLightEffectRainbowAfterDelay <= 0)
            {
                RGBeffect::rainbow(1000, 255, 255, 255, 255, 100);
                setSMTLightEffectRainbowAfterDelay = -1;
            }
        }
        vTaskDelay(100);
    }
}



void powerManagementTask(void *pvPara)
{
    SC8815::init();

    while (true)
    {
        SC8815::readADC();
        vTaskDelay(1);
    }
}

void UART_RxCpltCallbackForIdleDetection(UART_HandleTypeDef *huart, uint16_t size)
{
    // HAL_UART_Receive_IT(&huart3, rxData1, 10);
    GCodeDecoder::decodeMessage(UARTRxData, size);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, UARTRxData, UARTRxBufferSize);
}



StackType_t uxaudioAPMTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xaudioAPMTaskTCB;
void audioAPMTask(void *pvPara)
{
    ACM8625S::init();
    while (true)
    {
        ACM8625S::readStatus();
        vTaskDelay(100);        
    }
}


void startUserTasks()
{
    HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);
    GCodeDecoder::init();
    FDCANManager::Init(&hfdcan1);
    MotorManager::Init();
    CoreXY::Init();
    
    HAL_UART_RegisterRxEventCallback(&huart3, UART_RxCpltCallbackForIdleDetection);
    HAL_UART_ReceiverTimeout_Config(&huart3, huart3.Init.BaudRate / 50); // 20ms timeout
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, UARTRxData, UARTRxBufferSize);

    xTaskCreateStatic(mainTask, "mainTask", 512, NULL, 0, uxMainTaskStack, &xMainTaskTCB);
    xTaskCreateStatic(powerManagementTask, "powerManagementTask", configMINIMAL_STACK_SIZE, NULL, 0, uxPowerManagementTaskStack, &xPowerManagementTaskTCB);
    // xTaskCreateStatic(audioAPMTask, "audioAPMTask", configMINIMAL_STACK_SIZE, NULL, 0, uxaudioAPMTaskStack, &xaudioAPMTaskTCB);
}
