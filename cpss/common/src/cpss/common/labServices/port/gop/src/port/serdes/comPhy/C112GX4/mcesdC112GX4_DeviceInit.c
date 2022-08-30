/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_112G_X4
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_API.h"
#include "mcesdC112GX4_RegRW.h"
#include "mcesdC112GX4_FwDownload.h"
#include "mcesdC112GX4_HwCntl.h"
#include "mcesdC112GX4_DeviceInit.h"

#ifdef C112GX4

MCESD_STATUS API_C112GX4_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C112GX4_PowerOn powerOn
)
{
    MCESD_U8 lane;
    MCESD_FIELD pllTempCal   = F_C112GX4R1P0_PLL_TEMP_CAL_CONT;

    if (powerOn.downloadFw)
    {
        MCESD_ATTEMPT(API_C112GX4_DownloadFirmwareProgXData(devPtr, powerOn.fwCodePtr, powerOn.fwCodeSizeDW, powerOn.cmnXDataPtr, powerOn.cmnXDataSizeDW, powerOn.laneXDataPtr, powerOn.laneXDataSizeDW));
        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    }

    /* Enable all Lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_SetLaneEnable(devPtr, lane, MCESD_TRUE));
    }
    MCESD_ATTEMPT(API_C112GX4_SetLaneEnable(devPtr, 255 /* ignored */, MCESD_TRUE));

    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_C112GX4_SetPhyMode(devPtr, C112GX4_PHYMODE_SERDES));

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_C112GX4_SetRefFreq(devPtr, powerOn.refFreq, powerOn.refClkSel));

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255, &pllTempCal, 1));

    /* Configure TX and RX bitrates for all lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_SetTxRxBitRate(devPtr, lane, powerOn.speed));
    }

    /* Disable TX Output for all lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_SetTxOutputEnable(devPtr, lane, MCESD_FALSE));
    }

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_C112GX4_SetPowerIvRef(devPtr, MCESD_TRUE));

    /* Make sure PHY PLLs, Transmitter and Receiver powered is off for all lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_PowerOffLane(devPtr, lane));
    }

    /* Set Data Bus Width for all lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_SetDataBusWidth(devPtr, lane, powerOn.dataBusWidth, powerOn.dataBusWidth));
    }

    /* Enable DFE for speeds above 3.125 Gbps for all lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_SetDfeEnable(devPtr, lane, (powerOn.speed > 1) ? MCESD_TRUE : MCESD_FALSE));
    }

    /* Enable MCU on all lanes */
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_C112GX4_SetMcuEnable(devPtr, lane, MCESD_TRUE));
    }
    MCESD_ATTEMPT(API_C112GX4_SetMcuEnable(devPtr, 255 /* ignored */, MCESD_TRUE));

    /* Power on PHY PLLs, Transmitter and Receiver for all lanes*/
    for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
    {
        if (powerOn.powerLaneMask & (1 << lane))
        {
            if (powerOn.initTx)
                MCESD_ATTEMPT(API_C112GX4_SetPowerTx(devPtr, lane, MCESD_TRUE));
            if (powerOn.initRx)
                MCESD_ATTEMPT(API_C112GX4_SetPowerRx(devPtr, lane, MCESD_TRUE));
            MCESD_ATTEMPT(API_C112GX4_SetPowerPLL(devPtr, lane, MCESD_TRUE));
        }
    }

    /* Enable TX Output for all lanes */
    if (powerOn.txOutputEn)
    {
        for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
        {
            if (powerOn.powerLaneMask & (1 << lane))
                MCESD_ATTEMPT(API_C112GX4_SetTxOutputEnable(devPtr, lane, MCESD_TRUE));
        }
    }
#if 0
    /* Toggle RX_INIT */
    if (powerOn.initRx)
    {
        for (lane = 0; lane < C112GX4_TOTAL_LANES; lane++)
        {
            if (powerOn.powerLaneMask & (1 << lane))
                MCESD_ATTEMPT(API_C112GX4_RxInit(devPtr, lane));
        }
    }
#endif
    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr, lane, C112GX4_TRAINING_RX));
    MCESD_ATTEMPT(API_C112GX4_SetPowerPLL(devPtr, lane, MCESD_FALSE));
    MCESD_ATTEMPT(API_C112GX4_SetPowerTx(devPtr, lane, MCESD_FALSE));
    MCESD_ATTEMPT(API_C112GX4_SetPowerRx(devPtr, lane, MCESD_FALSE));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_DownloadFirmwareProgXData
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

    if (API_C112GX4_SetMcuBroadcast(devPtr, MCESD_TRUE) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_SetMcuBroadcast: Failed\n");
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DIRECT_ACCESS_EN, 1));
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PRAM_SOC_EN, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 10));

    /* Download MCU Firmware */
    if (API_C112GX4_DownloadFirmware(devPtr, fwCodePtr, fwCodeSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_PowerOnSeq: Failed firmware download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download common XDATA */
    if (API_C112GX4_ProgCmnXData(devPtr, cmnXDataPtr, cmnXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_PowerOnSeq: Failed common XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download lane XDATA - will be broadcasted to all lanes */
    if (API_C112GX4_ProgLaneXData(devPtr, 255, laneXDataPtr, laneXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PRAM_SOC_EN, 0));
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DIRECT_ACCESS_EN, 0));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 10));

    if (API_C112GX4_SetMcuBroadcast(devPtr, MCESD_FALSE) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_SetMcuBroadcast: Failed\n");
        return MCESD_FAIL;
    }

    /* Enable MCU */
    MCESD_ATTEMPT(API_C112GX4_SetLaneEnable(devPtr, 255, MCESD_TRUE));
    MCESD_ATTEMPT(API_C112GX4_SetMcuEnable(devPtr, 255, MCESD_TRUE));
    return MCESD_OK;
}

#endif /* C112GX4 */
