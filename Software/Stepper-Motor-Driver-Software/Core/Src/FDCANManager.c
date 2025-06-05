/**
 * @file FDCANManager.h
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "FDCANManager.h"
#include "FOC.h"
/*
trouble shooting:
需要打开 FDCAN 的 Interrupt1 和 Register callback
filter 只能用 mask 模式
*/

FDCANStatus_t FDCANStatus;
FDCAN_HandleTypeDef* hfdcan;

void FDCANManagerInit(FDCAN_HandleTypeDef* _hfdcan)
{
    hfdcan = _hfdcan;
    FDCANStatus.rxIdentifier = 0;
    FDCANStatus.disconnectCounter = 0;

    FDCAN_FilterTypeDef filter;
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK; // 实测 RANGE 模式不能用
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = FDCANMANAGER_FILTER_ID;
    filter.FilterID2 = FDCANMANAGER_FILTER_ID_MASK;  
    HAL_FDCAN_ConfigFilter(hfdcan, &filter);

    HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_ACCEPT_IN_RX_FIFO0 , FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    HAL_FDCAN_Start(hfdcan);
}

// Controller
FDCAN_TxHeaderTypeDef FDCANTxHeader;

void FDCANManagerSetTxHeader(uint32_t CANId)
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

void FDCANManagerTransmit(uint32_t CANId, uint8_t* txBuffer)
{
    FDCANManagerSetTxHeader(CANId);
    // FIFO will only be cleaned when the transeiver is powered on.
    if (HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0)
        return;
    if(HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &FDCANTxHeader, txBuffer) != HAL_OK)
        return;
}

// Decoder
FDCAN_RxHeaderTypeDef FDCANRxHeader;
uint8_t FDCANRxData[8];

void (*FDCANManagerDecodeMessage)(uint32_t CANId, uint8_t* rxBuffer) = NULL;

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *_hfdcan, uint32_t RxFifo0ITs)
{
    HAL_FDCAN_GetRxMessage(_hfdcan, FDCAN_RX_FIFO0, &FDCANRxHeader, FDCANRxData);
    if(FDCANManagerDecodeMessage != NULL && motorControlStatus.initializing == 0)
        FDCANManagerDecodeMessage(FDCANRxHeader.Identifier, FDCANRxData);
}

void FDCANManagerRegisterCallback(void (*callback)(uint32_t CANId, uint8_t* rxBuffer))
{
    FDCANManagerDecodeMessage = callback;
}