/**
 * @file FDCANManager.hpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "FDCANManager.hpp"

#include "stm32g431xx.h"

namespace FDCANManager
{
/*
trouble shooting:
需要打开 FDCAN 的 Interrupt1 和 Register callback
filter 只能用 mask 模式
*/

FDCANStatus_t FDCANStatus;
FDCAN_HandleTypeDef* hfdcan;

void Init(FDCAN_HandleTypeDef* _hfdcan)
{
    hfdcan = _hfdcan;
    FDCANStatus.rxIdentifier = 0;
    FDCANStatus.disconnectCounter = 0;

    FDCAN_FilterTypeDef filter;
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = FDCANMANAGER_FILTER_ID;
    filter.FilterID2 = FDCANMANAGER_FILTER_ID_MASK;  
    HAL_FDCAN_ConfigFilter(hfdcan, &filter);

    HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    HAL_FDCAN_Start(hfdcan);
}

// Controller
FDCAN_TxHeaderTypeDef FDCANTxHeader;

void SetTxHeader(uint32_t CANId)
{
    FDCANTxHeader.Identifier = CANId;

    FDCANTxHeader.IdType = FDCAN_STANDARD_ID;
    FDCANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    FDCANTxHeader.DataLength = FDCAN_DLC_BYTES_8;
    FDCANTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    FDCANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    FDCANTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    FDCANTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    FDCANTxHeader.MessageMarker = 0;
}

void Transmit(uint32_t CANId, uint8_t* txBuffer)
{
    SetTxHeader(CANId);
    // FIFO will only be cleaned when the transeiver is powered on.
    if (HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0)
        return;
    if(HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &FDCANTxHeader, txBuffer) != HAL_OK)
        return;
}

// Decoder
FDCAN_RxHeaderTypeDef FDCANRxHeader;
uint8_t FDCANRxData[8];

void (*DecodeMessage)(uint32_t CANId, uint8_t* rxBuffer) = NULL;

void RegisterCallback(void (*callback)(uint32_t CANId, uint8_t* rxBuffer))
{
    DecodeMessage = callback;
}


} // namespace FDCANManager

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan_, uint32_t RxFifo0ITs)
{
    HAL_FDCAN_GetRxMessage(hfdcan_, FDCAN_RX_FIFO0, &FDCANManager::FDCANRxHeader, FDCANManager::FDCANRxData);
    if(FDCANManager::DecodeMessage != NULL)
        FDCANManager::DecodeMessage(FDCANManager::FDCANRxHeader.Identifier, FDCANManager::FDCANRxData);
}