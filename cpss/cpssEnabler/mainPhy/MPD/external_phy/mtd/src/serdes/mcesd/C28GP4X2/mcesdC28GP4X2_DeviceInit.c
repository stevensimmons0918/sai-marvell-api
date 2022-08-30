/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP: 
COMPHY_28G_PIPE4_X2
********************************************************************/
#include <serdes/mcesd/mcesdTop.h>
#include <serdes/mcesd/mcesdApiTypes.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_Defs.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_API.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_RegRW.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_FwDownload.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_HwCntl.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_DeviceInit.h>

#ifdef C28GP4X2

MCESD_STATUS API_C28GP4X2_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32* fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32* cmnXDataPtr,
    IN MCESD_U32 cmnXDataSizeDW,
    IN MCESD_U32* laneXDataPtr,
    IN MCESD_U32 laneXDataSizeDW,
    IN E_C28GP4X2_SERDES_SPEED speed,
    IN E_C28GP4X2_REFFREQ refFreq,
    IN E_C28GP4X2_REFCLK_SEL refClkSel,
    IN E_C28GP4X2_DATABUS_WIDTH dataBusWidth
)
{
    MCESD_U16 errCode;

    MCESD_ATTEMPT(API_C28GP4X2_HwSetPinCfg(devPtr, C28GP4X2_PIN_DIRECT_ACCESS_EN, 1));
    MCESD_ATTEMPT(API_C28GP4X2_Wait(devPtr, 1));

    /* Download MCU Firmware */
    if (API_C28GP4X2_DownloadFirmware(devPtr, fwCodePtr, fwCodeSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed firmware download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download common XDATA */
    if (API_C28GP4X2_ProgCmnXData(devPtr, cmnXDataPtr, cmnXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed common XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download lane 0 XDATA */
    if (API_C28GP4X2_ProgLaneXData(devPtr, 0, laneXDataPtr, laneXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download lane 1 XDATA */
    if (API_C28GP4X2_ProgLaneXData(devPtr, 1, laneXDataPtr, laneXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C28GP4X2_HwSetPinCfg(devPtr, C28GP4X2_PIN_DIRECT_ACCESS_EN, 0));

    MCESD_ATTEMPT(API_C28GP4X2_Wait(devPtr, 1));

    /* Enable all Lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetLaneEnable(devPtr, 0, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetLaneEnable(devPtr, 1, MCESD_TRUE));

    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_C28GP4X2_SetPhyMode(devPtr, C28GP4X2_PHYMODE_SERDES));

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_C28GP4X2_SetRefFreq(devPtr, refFreq, refClkSel));

    /* Configure TX and RX bitrates for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetTxRxBitRate(devPtr, 0, speed));
    MCESD_ATTEMPT(API_C28GP4X2_SetTxRxBitRate(devPtr, 1, speed));

    /* Disable TX Output for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetTxOutputEnable(devPtr, 0, MCESD_FALSE));
    MCESD_ATTEMPT(API_C28GP4X2_SetTxOutputEnable(devPtr, 1, MCESD_FALSE));

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerIvRef(devPtr, MCESD_TRUE));    

    /* Make sure PHY PLLs, Transmitter and Receiver power is off for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_PowerOffLane(devPtr, 0));
    MCESD_ATTEMPT(API_C28GP4X2_PowerOffLane(devPtr, 1));

    /* Set Data Bus Width for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetDataBusWidth(devPtr, 0, dataBusWidth, dataBusWidth));
    MCESD_ATTEMPT(API_C28GP4X2_SetDataBusWidth(devPtr, 1, dataBusWidth, dataBusWidth));

    /* Enable DFE for speeds above 3.125 Gbps for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetDfeEnable(devPtr, 0, (speed > 1) ? MCESD_TRUE : MCESD_FALSE));
    MCESD_ATTEMPT(API_C28GP4X2_SetDfeEnable(devPtr, 1, (speed > 1) ? MCESD_TRUE : MCESD_FALSE));

    /* Enable MCU on all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetMcuEnable(devPtr, 0, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetMcuEnable(devPtr, 1, MCESD_TRUE));
    
    /* Power on PHY PLLs, Transmitter and Receiver for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerPLL(devPtr, 0, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerPLL(devPtr, 1, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerTx(devPtr, 0, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerTx(devPtr, 1, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerRx(devPtr, 0, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerRx(devPtr, 1, MCESD_TRUE));

    /* Enable TX Output for all lanes */
    MCESD_ATTEMPT(API_C28GP4X2_SetTxOutputEnable(devPtr, 0, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetTxOutputEnable(devPtr, 1, MCESD_TRUE));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_PowerOnLane
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X2_SERDES_SPEED speed,
    IN E_C28GP4X2_REFFREQ refFreq,
    IN E_C28GP4X2_REFCLK_SEL refClkSel,
    IN E_C28GP4X2_DATABUS_WIDTH dataBusWidth
)
{
    /* Enable Lane */
    MCESD_ATTEMPT(API_C28GP4X2_SetLaneEnable(devPtr, lane, MCESD_TRUE));

    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_C28GP4X2_SetPhyMode(devPtr, C28GP4X2_PHYMODE_SERDES));

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_C28GP4X2_SetRefFreq(devPtr, refFreq, refClkSel));

    /* Configure TX and RX bitrates */
    MCESD_ATTEMPT(API_C28GP4X2_SetTxRxBitRate(devPtr, lane, speed));

    /* Disable TX Output */
    MCESD_ATTEMPT(API_C28GP4X2_SetTxOutputEnable(devPtr, lane, MCESD_FALSE));

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerIvRef(devPtr, MCESD_TRUE));

    /* Make sure PHY PLLs, Transmitter and Receiver power is off */
    MCESD_ATTEMPT(API_C28GP4X2_PowerOffLane(devPtr, lane));

    /* Set Data Bus Width */
    MCESD_ATTEMPT(API_C28GP4X2_SetDataBusWidth(devPtr, lane, dataBusWidth, dataBusWidth));

    /* Enable DFE for speeds above 3.125 Gbps */
    MCESD_ATTEMPT(API_C28GP4X2_SetDfeEnable(devPtr, lane, (speed > 1) ? MCESD_TRUE : MCESD_FALSE));

    /* Enable MCU */
    MCESD_ATTEMPT(API_C28GP4X2_SetMcuEnable(devPtr, lane, MCESD_TRUE));

    /* Power on PHY PLLs, Transmitter and Receiver */
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerPLL(devPtr, lane, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerTx(devPtr, lane, MCESD_TRUE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerRx(devPtr, lane, MCESD_TRUE));

    /* Enable TX Output */
    MCESD_ATTEMPT(API_C28GP4X2_SetTxOutputEnable(devPtr, lane, MCESD_TRUE));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerPLL(devPtr, lane, MCESD_FALSE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerTx(devPtr, lane, MCESD_FALSE));
    MCESD_ATTEMPT(API_C28GP4X2_SetPowerRx(devPtr, lane, MCESD_FALSE));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_DownloadFirmwareProgXData
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

    MCESD_ATTEMPT(API_C28GP4X2_HwSetPinCfg(devPtr, C28GP4X2_PIN_DIRECT_ACCESS_EN, 1));
    MCESD_ATTEMPT(API_C28GP4X2_Wait(devPtr, 1));

    /* Download MCU Firmware */
    if (API_C28GP4X2_DownloadFirmware(devPtr, fwCodePtr, fwCodeSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed firmware download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download common XDATA */
    if (API_C28GP4X2_ProgCmnXData(devPtr, cmnXDataPtr, cmnXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed common XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download lane 0 XDATA */
    if (API_C28GP4X2_ProgLaneXData(devPtr, 0, laneXDataPtr, laneXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    /* Download lane 1 XDATA */
    if (API_C28GP4X2_ProgLaneXData(devPtr, 1, laneXDataPtr, laneXDataSizeDW, &errCode) == MCESD_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X2_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", errCode);
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C28GP4X2_HwSetPinCfg(devPtr, C28GP4X2_PIN_DIRECT_ACCESS_EN, 0));

    return MCESD_OK;
}

#endif /* C28GP4X2 */
