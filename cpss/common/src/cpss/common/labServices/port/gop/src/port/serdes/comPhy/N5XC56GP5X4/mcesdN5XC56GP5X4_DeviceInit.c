/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
5FFP_COMPHY_56G_PIPE5_X4_4PLL
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdN5XC56GP5X4_Defs.h"
#include "mcesdN5XC56GP5X4_API.h"
#include "mcesdN5XC56GP5X4_RegRW.h"
#include "mcesdN5XC56GP5X4_FwDownload.h"
#include "mcesdN5XC56GP5X4_HwCntl.h"
#include "mcesdN5XC56GP5X4_DeviceInit.h"

#ifdef N5XC56GP5X4

MCESD_STATUS API_N5XC56GP5X4_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_N5XC56GP5X4_PowerOn powerOn
)
{
    MCESD_U8 lane;
    MCESD_U16 errCode;

#ifdef N5XC56GP5X4_ISOLATION
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_ISOLATE, 255 /* ignored */, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_BG_RDY, 255 /* ignored */, 1);
#endif

    /* Download MCU Firmware */
    if (powerOn.downloadFw)
    {
        errCode = MCESD_IO_ERROR;
        MCESD_ATTEMPT(powerOn.fwDownload(devPtr, powerOn.fwCodePtr, powerOn.fwCodeSizeDW, powerOn.address, &errCode));
        if (errCode != 0)
        {
            MCESD_DBG_ERROR("PowerOnSeq() Firmware Download callback failed. (Error Code %d)\n", errCode);
            return MCESD_FAIL;
        }
        MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 1));
    }

    /* Set PIN_AVDD_SEL */
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_AVDD_SEL, powerOn.avdd));

    /* Set PIN_PIPE_SEL */
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PIPE_SEL, MCESD_FALSE));

    /* Configure Reference Frequency and reference clock source group */
    for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
    {
        if (powerOn.u.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_N5XC56GP5X4_SetRefFreq(devPtr, lane, powerOn.txRefFreq, powerOn.rxRefFreq, powerOn.txRefClkSel, powerOn.rxRefClkSel ));
    }

    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetPhyMode(devPtr, N5XC56GP5X4_PHYMODE_SERDES));

    /* Set PIN_SPD_CFG */
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_SPD_CFG, powerOn.spdCfg));

    /* Configure TX and RX bitrates for all lanes */
    for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
    {
        if (powerOn.u.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_N5XC56GP5X4_SetTxRxBitRate(devPtr, lane, powerOn.txSpeed, powerOn.rxSpeed));
    }

    /* Disable TX Output for all lanes */
    for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
    {
        if (powerOn.u.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_N5XC56GP5X4_SetTxOutputEnable(devPtr, lane, MCESD_FALSE));
    }

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerIvRef(devPtr, MCESD_TRUE));

    /* Set Data Bus Width for all lanes */
    for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
    {
        if (powerOn.u.powerLaneMask & (1 << lane))
            MCESD_ATTEMPT(API_N5XC56GP5X4_SetDataBusWidth(devPtr, lane, powerOn.dataBusWidth, powerOn.dataBusWidth));
    }

#ifdef N5XC56GP5X4_ISOLATION
    if (powerOn.downloadFw)
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_FW_READY, 255 /* ignored */, 1);
        N5XC56GP5X4_POLL_FIELD(devPtr, F_N5XC56GP5X4_MCU_INIT_DONE, 255 /* ignored */, 1, 1000);
    }
#endif

    /* Power on PHY PLLs, Transmitter and Receiver for all lanes*/
    for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
    {
        if (powerOn.u.powerLaneMask & (1 << lane))
        {
            MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerPLL(devPtr, lane, MCESD_TRUE));
            if (powerOn.initTx)
                MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerTx(devPtr, lane, MCESD_TRUE));
            if (powerOn.initRx)
                MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerRx(devPtr, lane, MCESD_TRUE));
        }
    }

    /* Enable TX Output for all lanes */
    if (powerOn.txOutputEn)
    {
        for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
        {
            if (powerOn.u.powerLaneMask & (1 << lane))
                MCESD_ATTEMPT(API_N5XC56GP5X4_SetTxOutputEnable(devPtr, lane, MCESD_TRUE));
        }
    }
#if 0
    /* Toggle RX_INIT */
    if (powerOn.initRx)
    {
        MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 100));
        for (lane = 0; lane < N5XC56GP5X4_TOTAL_LANES; lane++)
        {
            if (powerOn.u.powerLaneMask & (1 << lane))
                MCESD_ATTEMPT(API_N5XC56GP5X4_RxInit(devPtr, lane, powerOn.rxInitTimeout));
        }
    }
#endif
    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, N5XC56GP5X4_TRAINING_RX));
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerTx(devPtr, lane, MCESD_FALSE));
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerRx(devPtr, lane, MCESD_FALSE));
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetPowerPLL(devPtr, lane, MCESD_FALSE));
    return MCESD_OK;
}

#endif /* N5XC56GP5X4 */