/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_28G_PIPE4_RPLL_1P2V
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdC28GP4X1_Defs.h"
#include "mcesdC28GP4X1_API.h"
#include "mcesdC28GP4X1_RegRW.h"
#include "mcesdC28GP4X1_HwCntl.h"
#include "mcesdC28GP4X1_FwDownload.h"
#include "mcesdC28GP4X1_DeviceInit.h"

#ifdef C28GP4X1

MCESD_STATUS API_C28GP4X1_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C28GP4X1_PowerOn powerOn
)
{
    MCESD_BOOL powerLane0 = powerOn.powerLaneMask & 0x1;
    if (powerOn.downloadFw)
    {
        MCESD_ATTEMPT(API_C28GP4X1_DownloadFirmwareProgXData(devPtr, powerOn.fwCodePtr, powerOn.fwCodeSizeDW, powerOn.cmnXDataPtr, powerOn.cmnXDataSizeDW, powerOn.laneXDataPtr, powerOn.laneXDataSizeDW));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }

    /* Enable Lane */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetLaneEnable(devPtr, MCESD_TRUE));

    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_C28GP4X1_SetPhyMode(devPtr, C28GP4X1_PHYMODE_SERDES));

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_C28GP4X1_SetRefFreq(devPtr, powerOn.refFreq, powerOn.refClkSel));

    /* Configure TX and RX bitrates */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetTxRxBitRate(devPtr, powerOn.speed));

    /* Disable TX Output */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetTxOutputEnable(devPtr, MCESD_FALSE));

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_C28GP4X1_SetPowerIvRef(devPtr, MCESD_TRUE));

    /* Make sure PHY PLLs, Transmitter and Receiver power is off */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_PowerOffLane(devPtr));

    /* Set Data Bus Width */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetDataBusWidth(devPtr, powerOn.dataBusWidth, powerOn.dataBusWidth));

    /* Enable DFE for speeds above 3.125 Gbps */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetDfeEnable(devPtr, (powerOn.speed > 1) ? MCESD_TRUE : MCESD_FALSE));

    /* Enable MCU */
    if (powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetMcuEnable(devPtr, MCESD_TRUE));

    /* Power on PHY PLLs, Transmitter and Receiver */
    if (powerLane0)
    {
        if (powerOn.initTx)
            MCESD_ATTEMPT(API_C28GP4X1_SetPowerTx(devPtr, MCESD_TRUE));
        if (powerOn.initRx)
            MCESD_ATTEMPT(API_C28GP4X1_SetPowerRx(devPtr, MCESD_TRUE));
        MCESD_ATTEMPT(API_C28GP4X1_SetPowerPLL(devPtr, MCESD_TRUE));
    }

    /* Enable TX Output */
    if (powerOn.txOutputEn && powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_SetTxOutputEnable(devPtr, MCESD_TRUE));
#if 0
    /* Toggle RX_INIT */
    if (powerOn.initRx && powerLane0)
        MCESD_ATTEMPT(API_C28GP4X1_RxInit(devPtr));
#endif
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_ATTEMPT(API_C28GP4X1_StopTraining(devPtr, C28GP4X1_TRAINING_RX));
    MCESD_ATTEMPT(API_C28GP4X1_SetPowerPLL(devPtr, MCESD_FALSE));
    MCESD_ATTEMPT(API_C28GP4X1_SetPowerTx(devPtr, MCESD_FALSE));
    MCESD_ATTEMPT(API_C28GP4X1_SetPowerRx(devPtr, MCESD_FALSE));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_DownloadFirmwareProgXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32 *cmnXDataPtr,
    IN MCESD_U32 cmnXDataSizeDW,
    IN MCESD_U32 *laneXDataPtr,
    IN MCESD_U32 laneXDataSizeDW
)
{
    MCESD_U16 errCode;

    if (API_C28GP4X1_SetMcuBroadcast(devPtr, MCESD_TRUE) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_SetMcuBroadcast: Failed\n");
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DIRECT_ACCESS_EN, 1));
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PRAM_SOC_EN, 1));
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 10));

    /* Download MCU Firmware */
    if (API_C28GP4X1_DownloadFirmware(devPtr, fwCodePtr, fwCodeSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_PowerOnSeq: Failed firmware download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download common XDATA */
    if (API_C28GP4X1_ProgCmnXData(devPtr, cmnXDataPtr, cmnXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_PowerOnSeq: Failed common XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download lane XDATA */
    if (API_C28GP4X1_ProgLaneXData(devPtr, laneXDataPtr, laneXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PRAM_SOC_EN, 0));
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DIRECT_ACCESS_EN, 0));
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 10));

    if (API_C28GP4X1_SetMcuBroadcast(devPtr, MCESD_FALSE) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_SetMcuBroadcast: Failed\n");
        return MCESD_FAIL;
    }

    /* Enable MCU */
    MCESD_ATTEMPT(API_C28GP4X1_SetLaneEnable(devPtr, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X1_SetMcuEnable(devPtr, MCESD_TRUE));
    return MCESD_OK;
}

#endif /* C28GP4X1 */
