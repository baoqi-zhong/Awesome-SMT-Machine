
/**
 * @file FDCANManager.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "main.h"
#include "stdint.h"
#include "fdcan.h"

namespace FDCANManager
{
#define FDCANMANAGER_FILTER_ID      0x200       // FDCAN 接受的 ID
#define FDCANMANAGER_FILTER_ID_MASK 0x7F0       // FDCAN 接受的 ID 掩码

typedef struct
{
    uint32_t rxIdentifier;
    uint32_t disconnectCounter;
} FDCANStatus_t;

/**
 * @brief Initializes the FDCAN Manager.
 * 
 * @param _hfdcan Pointer to the FDCAN handle structure.
 */
void Init(FDCAN_HandleTypeDef* _hfdcan);

/**
 * @brief Transmits data over FDCAN.
 * 
 * @param CANId The CAN message identifier.
 * @param buffer Pointer to the data buffer to be transmitted.
 */
void Transmit(uint32_t CANId, uint8_t* buffer);

/**
 * @brief Registers a callback function for FDCAN message reception.
 * 
 * @param callback Pointer to the callback function that will be invoked upon message reception.
 * @param CANId The CAN message identifier.
 * @param rxBuffer Pointer to the receive buffer.
 */
void RegisterCallback(void (*callback)(uint32_t CANId, uint8_t* rxBuffer));


} // namespace FDCANManager