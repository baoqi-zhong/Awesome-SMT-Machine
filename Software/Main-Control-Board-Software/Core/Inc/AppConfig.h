/**
 * @file AppConfig.h
 * @author Ed Z (zouhetai@outlook.com) & Baoqi (zzhongas@connect.ust.hk)
 * @brief app config file to enable and disable certian modules
 * @version 0.2
 * @date 2024-08-10
 *
 * @copyright RM2025 Enterprize Copyright (c) 2023
 *
 */
// clang-format off

#pragma once


#define USE_BUZZER 1
    #if USE_BUZZER
        #ifndef BUZZER_TIM_CLOCK
        #define BUZZER_TIM_CLOCK 170000000
        #endif

        #ifndef BUZZER_TIM
        #define BUZZER_TIM htim2
        #endif

        #ifndef BUZZER_TIM_CHANNEL
        #define BUZZER_TIM_CHANNEL TIM_CHANNEL_1
        #endif

        #ifndef BUZZER_QUEUE_LENGTH
        #define BUZZER_QUEUE_LENGTH 16
        #endif
    #endif
