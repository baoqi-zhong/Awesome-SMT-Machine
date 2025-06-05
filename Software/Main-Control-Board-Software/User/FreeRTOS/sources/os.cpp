/**
 * @file os.c
 * @author JIANG Yicheng  RM2023 (EthenJ@outlook.sg)
 * @brief
 * @version 0.1
 * @date 2022-08-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "stm32g4xx.h"

#include "FreeRTOS.h"
#include "task.h"

#define HIGHEST_PRIORITY 0x0U

extern void startUserTasks();  // user should implement this cpp function in UserTask.cpp

extern "C"
{
    static void doNothing(void)
    { /* do nothing */
    }

    typedef void(SysTickHandle_t)(void);

    SysTickHandle_t *RTOSSysTickHandler = doNothing;  // do nothing before initialization to avoid hardfault

    void startRTOS(void)
    {
        traceSTART();

        configASSERT(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);  // already started?

        startUserTasks();  // create user tasks

        configASSERT(NVIC_GetPriority(SVCall_IRQn) == HIGHEST_PRIORITY);  // SVCall_IRQn should be the highest priority

        extern void xPortSysTickHandler(void);
        RTOSSysTickHandler = xPortSysTickHandler;  // set RTOSSysTickHandler to xPortSysTickHandler

        vTaskStartScheduler();  // start FreeRTOS scheduler

        configASSERT(0);  // should never reach here
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    /*
      vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
      equals to 1 and is required for static memory allocation support.
    */
    void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
    {
        /* Idle task control block and stack */
        static StaticTask_t Idle_TCB;
        static StackType_t Idle_Stack[configMINIMAL_STACK_SIZE];

        *ppxIdleTaskTCBBuffer = &Idle_TCB;
        *ppxIdleTaskStackBuffer = &Idle_Stack[0];
        *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
    }

    /*
      vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
      equals to 1 and is required for static memory allocation support.
    */
    void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
    {
        /* Timer task control block and stack */
        static StaticTask_t Timer_TCB;
        static StackType_t Timer_Stack[configTIMER_TASK_STACK_DEPTH];

        *ppxTimerTaskTCBBuffer = &Timer_TCB;
        *ppxTimerTaskStackBuffer = &Timer_Stack[0];
        *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
    }
#endif
}


