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
#include "../mcesdUtils.h"
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_API.h"
#include "mcesdC112GX4_RegRW.h"
#include "mcesdC112GX4_HwCntl.h"
#include <string.h>
#include <stdio.h>

#ifdef C112GX4

/* Forward internal function prototypes used only in this module */
static MCESD_U32 INT_C112GX4_ComputeTxEqEmMain(IN MCESD_U32 pre2Cursor, IN MCESD_U32 preCursor, IN MCESD_U32 postCursor, IN MCESD_U32 firCtrl1, IN MCESD_U32 firCtrl2, IN MCESD_U32 firCtrl3);
static MCESD_STATUS INT_C112GX4_R1P2_SetTxEqParam(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN E_C112GX4_TXEQ_PARAM param, IN MCESD_U32 paramValue);
static MCESD_STATUS INT_C112GX4_R1P2_GetTxEqParam(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN E_C112GX4_TXEQ_PARAM param, OUT MCESD_U32 *paramValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_ConvertToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldTopPtr, IN MCESD_FIELD_PTR fieldMidPtr, IN MCESD_FIELD_PTR fieldBotPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldMSBPtr, IN MCESD_FIELD_PTR fieldLSBPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F1(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, IN E_C112GX4_EYE_TMB eyeTmb, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F2(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, IN E_C112GX4_EYE_TMB eyeTmb, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F3(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, IN E_C112GX4_EYE_TMB eyeTmb, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F4(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, IN E_C112GX4_EYE_TMB eyeTmb, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F5(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F6(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F7(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F8(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F9(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F10(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F11(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F12(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F13(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F14(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_GetDfeTap_F15(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U32 p1DisableData, IN E_C112GX4_DATA_ACQ_RATE acqRate, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_C112GX4_DisplayEntryPAM2_0(IN MCESD_U16 data, OUT S_C112GX4_TRX_TRAINING_LOGENTRY* entry);
static MCESD_STATUS INT_C112GX4_DisplayEntryPAM2_1(IN MCESD_U16 data, OUT S_C112GX4_TRX_TRAINING_LOGENTRY* entry);
static MCESD_STATUS INT_C112GX4_DisplayEntryPAM4(IN MCESD_U16 data, OUT S_C112GX4_TRX_TRAINING_LOGENTRY* entry);

MCESD_STATUS API_C112GX4_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
)
{
    MCESD_FIELD fwMajorVer = F_C112GX4R1P0_FW_MAJOR_VER;
    MCESD_FIELD fwMinorVer = F_C112GX4R1P0_FW_MINOR_VER;
    MCESD_FIELD fwPatchVer = F_C112GX4R1P0_FW_PATCH_VER;
    MCESD_FIELD fwBuildVer = F_C112GX4R1P0_FW_BUILD_VER;
    MCESD_FIELD mcuStatus0 = F_C112GX4R1P0_MCU_STATUS0;
    MCESD_FIELD laneSel = F_C112GX4R1P0_LANE_SEL;
    MCESD_U32 data;

    MCESD_BOOL mcuActive;

    MCESD_ATTEMPT(mcesdGetMCUActive(devPtr, &mcuStatus0, &laneSel, 4, &mcuActive));

    if (!mcuActive)
    {
        /* Firmware version is invalid */
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &fwMajorVer, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &fwMinorVer, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &fwPatchVer, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &fwBuildVer, 0));
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &fwMajorVer, &data));
    *major = (MCESD_U8)data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &fwMinorVer, &data));
    *minor = (MCESD_U8)data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &fwPatchVer, &data));
    *patch = (MCESD_U8)data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &fwBuildVer, &data));
    *build = (MCESD_U8)data;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *locked
)
{
    MCESD_FIELD pllLock = F_C112GX4R1P0_ANA_PLL_LOCK_RD;
    MCESD_U32 pllLockData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &pllLock, &pllLockData));
    *locked = (1 == pllLockData) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
)
{
    MCESD_FIELD pllReadyTx = F_C112GX4R1P0_PLL_READY_TX;
    MCESD_FIELD pllReadyRx = F_C112GX4R1P0_PLL_READY_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &pllReadyTx, &data));
    *txReady = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &pllReadyRx, &data));
    *rxReady = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD rxInit      = F_C112GX4R1P0_RX_INIT;
    MCESD_FIELD rxInitDone  = F_C112GX4R1P0_RX_INIT_DONE;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxInitDone, 0));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxInit, 1));
    MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &rxInitDone, 1, 5000));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxInit, 0));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT0, 1));
        MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_INIT_DONE0, 1, 5000));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT0, 0));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT1, 1));
        MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_INIT_DONE1, 1, 5000));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT1, 0));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT2, 1));
        MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_INIT_DONE2, 1, 5000));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT2, 0));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT3, 1));
        MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_INIT_DONE3, 1, 5000));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_INIT3, 0));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C112GX4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD txEmPre2R1p0 = F_C112GX4R1P0_TX_EM_PRE2;
    MCESD_FIELD txEmPre2R1p1 = F_C112GX4R1P1_TX_EM_PRE2;
    MCESD_FIELD txEmPre = F_C112GX4R1P0_TX_EM_PRE;
    MCESD_FIELD txEmPreForce = F_C112GX4R1P0_TX_EM_PRE_FORCE;
    MCESD_FIELD txEmPost = F_C112GX4R1P0_TX_EM_POST;
    MCESD_FIELD txEmPostForce = F_C112GX4R1P0_TX_EM_POST_FORCE;
    MCESD_FIELD txEmMain = F_C112GX4R1P0_TX_EM_MAIN;
    MCESD_FIELD txEmMainForce = F_C112GX4R1P0_TX_EM_MAIN_FORCE;
    MCESD_FIELD txFirCCtrl1 = F_C112GX4R1P0_TX_FIR_C_CTRL1;
    MCESD_FIELD txFirCCtrl2 = F_C112GX4R1P0_TX_FIR_C_CTRL2;
    MCESD_FIELD txFirCCtrl3 = F_C112GX4R1P0_TX_FIR_C_CTRL3;
    MCESD_FIELD txEmPre2Force = F_C112GX4R1P0_TX_EM_PRE2_FORCE;
    MCESD_FIELD txAnaC0 = F_C112GX4R1P0_TO_ANA_TX_FIR_C0;
    MCESD_FIELD txAnaC1 = F_C112GX4R1P0_TO_ANA_TX_FIR_C1;
    MCESD_FIELD txAnaC2 = F_C112GX4R1P0_TO_ANA_TX_FIR_C2;
    MCESD_FIELD txAnaC3 = F_C112GX4R1P0_TO_ANA_TX_FIR_C3;
    MCESD_FIELD txAnaC4 = F_C112GX4R1P0_TO_ANA_TX_FIR_C4;
    MCESD_FIELD txAnaC5 = F_C112GX4R1P0_TO_ANA_TX_FIR_C5;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD txFirUpdate = F_C112GX4R1P0_TX_FIR_UPDATE;
    MCESD_U32 txPAM2EnData;
    MCESD_FIELD_PTR txEmPre2Ptr;
    if ((devPtr->ipMajorRev <= 1) && (devPtr->ipMinorRev < 1))
    {
        /* RX.X < R1.1 */
        txEmPre2Ptr = &txEmPre2R1p0;
    }
    else if ((devPtr->ipMajorRev >= 1) && (devPtr->ipMinorRev > 1))
    {
        /* RX.X > R1.1 */
        MCESD_ATTEMPT(INT_C112GX4_R1P2_SetTxEqParam(devPtr, lane, param, paramValue));
        return MCESD_OK;
    }
    else
    {
        /* RX.X == R1.1 */
        txEmPre2Ptr = &txEmPre2R1p1;
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txPAM2En, &txPAM2EnData));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 0));

    switch (param)
    {
    case C112GX4_TXEQ_EM_PRE2_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmPre2Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, txEmPre2Ptr, paramValue));
        break;
    case C112GX4_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmPreForce, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmPre, paramValue));
        break;
    case C112GX4_TXEQ_EM_MAIN_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmMainForce, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmMain, paramValue));
        break;
    case C112GX4_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmPostForce, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmPost, paramValue));
        break;
    case C112GX4_TXEQ_FIR_C_CTRL1:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirCCtrl1, paramValue));
        break;
    case C112GX4_TXEQ_FIR_C_CTRL2:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirCCtrl2, paramValue));
        break;
    case C112GX4_TXEQ_FIR_C_CTRL3:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirCCtrl3, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 0));

    /* The TX emphasis main is automatically calculated and updated unless user explicitly wants to manually set it */
    if (param != C112GX4_TXEQ_EM_MAIN_CTRL)
    {
        MCESD_U32 txEmPre2Data, txEmPreData, txEmMainData, txEmPostData, txFirCCtrl1Data, txFirCCtrl2Data;

        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC0, &txEmPre2Data));
        if (1 == txPAM2EnData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC2, &txEmPreData));
        }
        else
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC1, &txEmPreData));
        }
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC3, &txEmPostData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC4, &txFirCCtrl1Data));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC5, &txFirCCtrl2Data));

        /* Calculate Tx emphasis Main based on full_swing setting */
        txEmMainData = INT_C112GX4_ComputeTxEqEmMain(txEmPre2Data, txEmPreData, txEmPostData, txFirCCtrl1Data, txFirCCtrl2Data, 0);

        /* Update Tx emphasis Main */
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmMainForce, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txEmMain, txEmMainData));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 1));
        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 0));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD txAnaC0 = F_C112GX4R1P0_TO_ANA_TX_FIR_C0;
    MCESD_FIELD txAnaC1 = F_C112GX4R1P0_TO_ANA_TX_FIR_C1;
    MCESD_FIELD txAnaC2 = F_C112GX4R1P0_TO_ANA_TX_FIR_C2;
    MCESD_FIELD txAnaC3 = F_C112GX4R1P0_TO_ANA_TX_FIR_C3;
    MCESD_FIELD txAnaC4 = F_C112GX4R1P0_TO_ANA_TX_FIR_C4;
    MCESD_FIELD txAnaC5 = F_C112GX4R1P0_TO_ANA_TX_FIR_C5;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_U32 txPAM2EnData;

    if ((devPtr->ipMajorRev >= 1) && (devPtr->ipMinorRev > 1))
    {
        /* RX.X > R1.1 */
        MCESD_ATTEMPT(INT_C112GX4_R1P2_GetTxEqParam(devPtr, lane, param, paramValue));
        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txPAM2En, &txPAM2EnData));

    switch (param)
    {
    case C112GX4_TXEQ_EM_PRE2_CTRL:
        if (1 == txPAM2EnData)
            return MCESD_FAIL;  /* PAM2 does not have PRE2 */
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC0, paramValue));
        break;
    case C112GX4_TXEQ_EM_PRE_CTRL:
        if (1 == txPAM2EnData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC0, paramValue));
        }
        else
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC1, paramValue));
        }
        break;
    case C112GX4_TXEQ_EM_MAIN_CTRL:
        if (1 == txPAM2EnData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC1, paramValue));
        }
        else
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC2, paramValue));
        }
        break;
    case C112GX4_TXEQ_EM_POST_CTRL:
        if (1 == txPAM2EnData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC2, paramValue));
        }
        else
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC3, paramValue));
        }
        break;
    case C112GX4_TXEQ_FIR_C_CTRL1:
        if (1 == txPAM2EnData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC3, paramValue));
        }
        else
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC4, paramValue));
        }
        break;
    case C112GX4_TXEQ_FIR_C_CTRL2:
        if (1 == txPAM2EnData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC4, paramValue));
        }
        else
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC5, paramValue));
        }
        break;
    case C112GX4_TXEQ_FIR_C_CTRL3:
        if (0 == txPAM2EnData)
            return MCESD_FAIL;  /* PAM4 does not have CTRL3 */
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC5, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    switch (param)
    {
    case C112GX4_CTLE_CURRENT1_SEL:
        {
            MCESD_FIELD current1Sel = F_C112GX4R1P0_CURRENT1_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &current1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_SEL:
        {
            MCESD_FIELD rl1Sel = F_C112GX4R1P0_RL1_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_EXTRA:
        {
            MCESD_FIELD rl1Extra = F_C112GX4R1P0_RL1_EXTRA;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl1Extra, paramValue));
        }
        break;
    case C112GX4_CTLE_RES1_SEL:
        {
            MCESD_FIELD res1Sel = F_C112GX4R1P0_RES1_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &res1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_CAP1_SEL:
        {
            MCESD_FIELD cap1Sel = F_C112GX4R1P0_CAP1_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cap1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_CL1_CTRL:
        {
            MCESD_FIELD cl1Ctrl = F_C112GX4R1P0_CL1_CTRL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cl1Ctrl, paramValue));
        }
        break;
    case C112GX4_CTLE_EN_MID_FREQ:
        {
            MCESD_FIELD enMidFreq = F_C112GX4R1P0_EN_MID_FREQ;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &enMidFreq, paramValue));
        }
        break;
    case C112GX4_CTLE_CS1_MID:
        {
            MCESD_FIELD cs1Mid = F_C112GX4R1P0_CS1_MID;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cs1Mid, paramValue));
        }
        break;
    case C112GX4_CTLE_RS1_MID:
        {
            MCESD_FIELD rs1Mid = F_C112GX4R1P0_RS1_MID;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rs1Mid, paramValue));
        }
        break;
    case C112GX4_CTLE_RF_CTRL:
        {
            MCESD_FIELD rfCtrl = F_C112GX4R1P0_RF_CTRL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rfCtrl, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_TIA_SEL:
        {
            MCESD_FIELD rl1TiaSel = F_C112GX4R1P0_RL1_TIA_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl1TiaSel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_TIA_EXTRA:
        {
            MCESD_FIELD rl1TiaExtra = F_C112GX4R1P0_RL1_TIA_EXTRA;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl1TiaExtra, paramValue));
        }
        break;
    case C112GX4_CTLE_HPF_RSEL_1ST:
        {
            MCESD_FIELD hpfRSel1st = F_C112GX4R1P0_HPF_RSEL_1ST;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &hpfRSel1st, paramValue));
        }
        break;
    case C112GX4_CTLE_CURRENT1_TIA_SEL:
        {
            MCESD_FIELD current1TiaSel = F_C112GX4R1P0_CURRENT1_TIA_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &current1TiaSel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL2_SEL:
        {
            MCESD_FIELD rl2SelP1 = F_C112GX4R1P0_RL2_SEL_G_P1;
            MCESD_FIELD rl2SelP2 = F_C112GX4R1P0_RL2_SEL_G_P2;
            MCESD_FIELD rl2SelP3 = F_C112GX4R1P0_RL2_SEL_G_P3;
            MCESD_FIELD rl2SelP4 = F_C112GX4R1P0_RL2_SEL_G_P4;

            MCESD_U32 grayCode = ConvertU32ToGrayCode(paramValue);
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2SelP1, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2SelP2, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2SelP3, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2SelP4, grayCode));
        }
        break;
    case C112GX4_CTLE_RL2_TUNE:
        {
            MCESD_FIELD rl2TuneP1 = F_C112GX4R1P0_RL2_TUNE_P1;
            MCESD_FIELD rl2TuneP2 = F_C112GX4R1P0_RL2_TUNE_P2;
            MCESD_FIELD rl2TuneP3 = F_C112GX4R1P0_RL2_TUNE_P3;
            MCESD_FIELD rl2TuneP4 = F_C112GX4R1P0_RL2_TUNE_P4;

            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2TuneP1, paramValue));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2TuneP2, paramValue));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2TuneP3, paramValue));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rl2TuneP4, paramValue));
        }
        break;
    case C112GX4_CTLE_RS2_SEL:
        {
            MCESD_FIELD rs2SelP1 = F_C112GX4R1P0_RS2_SEL_G_P1;
            MCESD_FIELD rs2SelP2 = F_C112GX4R1P0_RS2_SEL_G_P2;
            MCESD_FIELD rs2SelP3 = F_C112GX4R1P0_RS2_SEL_G_P3;
            MCESD_FIELD rs2SelP4 = F_C112GX4R1P0_RS2_SEL_G_P4;

            MCESD_U32 grayCode = ConvertU32ToGrayCode(paramValue);
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rs2SelP1, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rs2SelP2, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rs2SelP3, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rs2SelP4, grayCode));
        }
        break;
    case C112GX4_CTLE_CURRENT2_SEL:
        {
            MCESD_FIELD current2SelP1 = F_C112GX4R1P0_CURRENT2_SEL_G_P1;
            MCESD_FIELD current2SelP2 = F_C112GX4R1P0_CURRENT2_SEL_G_P2;
            MCESD_FIELD current2SelP3 = F_C112GX4R1P0_CURRENT2_SEL_G_P3;
            MCESD_FIELD current2SelP4 = F_C112GX4R1P0_CURRENT2_SEL_G_P4;

            MCESD_U32 grayCode = ConvertU32ToGrayCode(paramValue);
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &current2SelP1, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &current2SelP2, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &current2SelP3, grayCode));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &current2SelP4, grayCode));
        }
        break;
    case C112GX4_CTLE_CAP2_SEL:
        {
            MCESD_FIELD cap2Sel = F_C112GX4R1P0_CAP2_SEL;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cap2Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_HPF_RSEL_2ND:
        {
            MCESD_FIELD hpfRSel2nd = F_C112GX4R1P0_HPF_RSEL_2ND;
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &hpfRSel2nd, paramValue));
        }
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }
#if 0
    MCESD_ATTEMPT(API_C112GX4_ExecuteCDS(devPtr, lane));
#endif
    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    switch (param)
    {
    case C112GX4_CTLE_CURRENT1_SEL:
        {
            MCESD_FIELD current1Sel = F_C112GX4R1P0_CURRENT1_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &current1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_SEL:
        {
            MCESD_FIELD rl1Sel = F_C112GX4R1P0_RL1_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rl1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_EXTRA:
        {
            MCESD_FIELD rl1Extra = F_C112GX4R1P0_RL1_EXTRA;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rl1Extra, paramValue));
        }
        break;
    case C112GX4_CTLE_RES1_SEL:
        {
            MCESD_FIELD res1Sel = F_C112GX4R1P0_RES1_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &res1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_CAP1_SEL:
        {
            MCESD_FIELD cap1Sel = F_C112GX4R1P0_CAP1_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &cap1Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_CL1_CTRL:
        {
            MCESD_FIELD cl1Ctrl = F_C112GX4R1P0_CL1_CTRL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &cl1Ctrl, paramValue));
        }
        break;
    case C112GX4_CTLE_EN_MID_FREQ:
        {
            MCESD_FIELD enMidFreq = F_C112GX4R1P0_EN_MID_FREQ;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &enMidFreq, paramValue));
        }
        break;
    case C112GX4_CTLE_CS1_MID:
        {
            MCESD_FIELD cs1Mid = F_C112GX4R1P0_CS1_MID;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &cs1Mid, paramValue));
        }
        break;
    case C112GX4_CTLE_RS1_MID:
        {
            MCESD_FIELD rs1Mid = F_C112GX4R1P0_RS1_MID;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rs1Mid, paramValue));
        }
        break;
    case C112GX4_CTLE_RF_CTRL:
        {
            MCESD_FIELD rfCtrl = F_C112GX4R1P0_RF_CTRL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rfCtrl, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_TIA_SEL:
        {
            MCESD_FIELD rl1TiaSel = F_C112GX4R1P0_RL1_TIA_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rl1TiaSel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL1_TIA_EXTRA:
        {
            MCESD_FIELD rl1TiaExtra = F_C112GX4R1P0_RL1_TIA_EXTRA;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rl1TiaExtra, paramValue));
        }
        break;
    case C112GX4_CTLE_HPF_RSEL_1ST:
        {
            MCESD_FIELD hpfRSel1st = F_C112GX4R1P0_HPF_RSEL_1ST;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &hpfRSel1st, paramValue));
        }
        break;
    case C112GX4_CTLE_CURRENT1_TIA_SEL:
        {
            MCESD_FIELD current1TiaSel = F_C112GX4R1P0_CURRENT1_TIA_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &current1TiaSel, paramValue));
        }
        break;
    case C112GX4_CTLE_RL2_SEL:
        {
            MCESD_FIELD rl2SelP1 = F_C112GX4R1P0_RL2_SEL_G_P1;
            MCESD_U32 grayCode;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rl2SelP1, &grayCode));
            *paramValue = ConvertGrayCodeToU32(grayCode);
        }
        break;
    case C112GX4_CTLE_RL2_TUNE:
        {
            MCESD_FIELD rl2TuneP1 = F_C112GX4R1P0_RL2_TUNE_P1;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rl2TuneP1, paramValue));
        }
        break;
    case C112GX4_CTLE_RS2_SEL:
        {
            MCESD_FIELD rs2SelP1 = F_C112GX4R1P0_RS2_SEL_G_P1;
            MCESD_U32 grayCode;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rs2SelP1, &grayCode));
            *paramValue = ConvertGrayCodeToU32(grayCode);
        }
        break;
    case C112GX4_CTLE_CURRENT2_SEL:
        {
            MCESD_FIELD current2SelP1 = F_C112GX4R1P0_CURRENT2_SEL_G_P1;
            MCESD_U32 grayCode;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &current2SelP1, &grayCode));
            *paramValue = ConvertGrayCodeToU32(grayCode);
        }
        break;
    case C112GX4_CTLE_CAP2_SEL:
        {
            MCESD_FIELD cap2Sel = F_C112GX4R1P0_CAP2_SEL;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &cap2Sel, paramValue));
        }
        break;
    case C112GX4_CTLE_HPF_RSEL_2ND:
        {
            MCESD_FIELD hpfRSel2nd = F_C112GX4R1P0_HPF_RSEL_2ND;
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &hpfRSel2nd, paramValue));
        }
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_U32 INT_C112GX4_ComputeTxEqEmMain
(
    IN MCESD_U32 pre2Cursor,
    IN MCESD_U32 preCursor,
    IN MCESD_U32 postCursor,
    IN MCESD_U32 firCtrl1,
    IN MCESD_U32 firCtrl2,
    IN MCESD_U32 firCtrl3
)
{
    const MCESD_U32 fullSwing = 63;
    MCESD_U32 sum = pre2Cursor + preCursor + postCursor + firCtrl1 + firCtrl2 + firCtrl3;

    if (fullSwing >= sum)
        return fullSwing - sum;
    else
        return 0;
}

static MCESD_STATUS INT_C112GX4_R1P2_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD txFirC0         = F_C112GX4R1P2_TX_FIR_C0;
    MCESD_FIELD txFirC0Force    = F_C112GX4R1P2_TX_FIR_C0_FORCE;
    MCESD_FIELD txFirC1         = F_C112GX4R1P2_TX_FIR_C1;
    MCESD_FIELD txFirC1Force    = F_C112GX4R1P2_TX_FIR_C1_FORCE;
    MCESD_FIELD txFirC2         = F_C112GX4R1P2_TX_FIR_C2;
    MCESD_FIELD txFirC2Force    = F_C112GX4R1P2_TX_FIR_C2_FORCE;
    MCESD_FIELD txFirC3         = F_C112GX4R1P2_TX_FIR_C3;
    MCESD_FIELD txFirC3Force    = F_C112GX4R1P2_TX_FIR_C3_FORCE;
    MCESD_FIELD txFirC4         = F_C112GX4R1P2_TX_FIR_C4;
    MCESD_FIELD txFirC4Force    = F_C112GX4R1P2_TX_FIR_C4_FORCE;
    MCESD_FIELD txFirC5         = F_C112GX4R1P2_TX_FIR_C5;
    MCESD_FIELD txFirC5Force    = F_C112GX4R1P2_TX_FIR_C5_FORCE;
    MCESD_FIELD txAnaC0         = F_C112GX4R1P0_TO_ANA_TX_FIR_C0;
    MCESD_FIELD txAnaC1         = F_C112GX4R1P0_TO_ANA_TX_FIR_C1;
    MCESD_FIELD txAnaC2         = F_C112GX4R1P0_TO_ANA_TX_FIR_C2;
    MCESD_FIELD txAnaC4         = F_C112GX4R1P0_TO_ANA_TX_FIR_C4;
    MCESD_FIELD txAnaC5         = F_C112GX4R1P0_TO_ANA_TX_FIR_C5;
    MCESD_FIELD txFirUpdate     = F_C112GX4R1P0_TX_FIR_UPDATE;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 0));

    switch (param)
    {
    case C112GX4_TXEQ_EM_PRE3_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC0Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC0, paramValue));
        break;
    case C112GX4_TXEQ_EM_PRE2_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC1Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC1, paramValue));
        break;
    case C112GX4_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC2Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC2, paramValue));
        break;
    case C112GX4_TXEQ_EM_MAIN_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC3Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC3, paramValue));
        break;
    case C112GX4_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC4Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC4, paramValue));
        break;
    case C112GX4_TXEQ_EM_NA:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC5Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC5, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 0));

    /* The TX emphasis main is automatically calculated and updated unless user explicitly wants to manually set it */
    if (param != C112GX4_TXEQ_EM_MAIN_CTRL)
    {
        MCESD_U32 txEmPre2Data, txEmPreData, txEmMainData, txEmPostData, txEmPre3Data, txEmNAData;

        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC0, &txEmPre3Data));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC1, &txEmPre2Data));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC2, &txEmPreData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC4, &txEmPostData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC5, &txEmNAData));

        /* Calculate Tx emphasis Main based on full_swing setting */
        txEmMainData = INT_C112GX4_ComputeTxEqEmMain(txEmPre3Data, txEmPre2Data, txEmPreData, txEmPostData, txEmNAData, 0);

        /* Update Tx emphasis Main */
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC3Force, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirC3, txEmMainData));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 1));
        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFirUpdate, 0));
    }

    return MCESD_OK;
}

static MCESD_STATUS INT_C112GX4_R1P2_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD txAnaC0 = F_C112GX4R1P0_TO_ANA_TX_FIR_C0;
    MCESD_FIELD txAnaC1 = F_C112GX4R1P0_TO_ANA_TX_FIR_C1;
    MCESD_FIELD txAnaC2 = F_C112GX4R1P0_TO_ANA_TX_FIR_C2;
    MCESD_FIELD txAnaC3 = F_C112GX4R1P0_TO_ANA_TX_FIR_C3;
    MCESD_FIELD txAnaC4 = F_C112GX4R1P0_TO_ANA_TX_FIR_C4;
    MCESD_FIELD txAnaC5 = F_C112GX4R1P0_TO_ANA_TX_FIR_C5;

    switch (param)
    {
    case C112GX4_TXEQ_EM_PRE3_CTRL:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC0, paramValue));
        break;
    case C112GX4_TXEQ_EM_PRE2_CTRL:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC1, paramValue));
        break;
    case C112GX4_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC2, paramValue));
        break;
    case C112GX4_TXEQ_EM_MAIN_CTRL:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC3, paramValue));
        break;
    case C112GX4_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC4, paramValue));
        break;
    case C112GX4_TXEQ_EM_NA:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txAnaC5, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD dfeEn = F_C112GX4R1P0_DFE_EN;
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dfeEn, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_EN0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_EN1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_EN2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_EN3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD dfeEn = F_C112GX4R1P0_DFE_EN;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &dfeEn, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_EN0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_EN1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_EN2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_EN3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD dfeUpdateDis = F_C112GX4R1P0_DFE_UPDATE_DIS;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dfeUpdateDis, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD dfeUpdateDis = F_C112GX4R1P0_DFE_UPDATE_DIS;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &dfeUpdateDis, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_DFE_UPDATE_DIS3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C112GX4_GetDataAcquisitionRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_DATA_ACQ_RATE *acqRate
)
{
    E_C112GX4_SERDES_SPEED speed;

    *acqRate = C112GX4_RATE_UNKNOWN;

    MCESD_ATTEMPT(API_C112GX4_GetTxRxBitRate(devPtr, lane, &speed));

    switch (speed)
    {
    case C112GX4_SERDES_1P25G:
    case C112GX4_SERDES_2P5G:
    case C112GX4_SERDES_3P125G:
    case C112GX4_SERDES_5G:
    case C112GX4_SERDES_5P15625G:
    case C112GX4_SERDES_6P25G:
    case C112GX4_SERDES_10G:
    case C112GX4_SERDES_10P3125G:
    case C112GX4_SERDES_20P625G:
    case C112GX4_SERDES_25P78125G:
    case C112GX4_SERDES_26P5625G:
    case C112GX4_SERDES_28P125G:
        *acqRate = C112GX4_RATE_FULL;
        break;
    case C112GX4_SERDES_53P125G:
    case C112GX4_SERDES_56P25G:
        *acqRate = C112GX4_RATE_HALF;
        break;
    case C112GX4_SERDES_106GP25G:
    case C112GX4_SERDES_112G:
        *acqRate = C112GX4_RATE_QUARTER;
        break;
    default:
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTmb,
    IN E_C112GX4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    MCESD_FIELD p1Disable = F_C112GX4R1P0_PATH_DISABLE_P1;
    MCESD_U32 p1DisableData;
    E_C112GX4_DATA_ACQ_RATE acqRate;

    /* Validate eye parameter */
    if ((eyeTmb != C112GX4_EYE_TOP) && (eyeTmb != C112GX4_EYE_MID) && (eyeTmb != C112GX4_EYE_BOT))
        return MCESD_FAIL;

    /* Get Data Acquisition Rate */
    MCESD_ATTEMPT(API_C112GX4_GetDataAcquisitionRate(devPtr, lane, &acqRate));

    /* Get P1 disable data */
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &p1Disable, &p1DisableData));

    switch (tap)
    {
    case C112GX4_DFE_DC:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_DC_D_TOP_P1, F_C112GX4R1P0_DFE_DC_D_MID_P1, F_C112GX4R1P0_DFE_DC_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_DC_D_TOP_P2, F_C112GX4R1P0_DFE_DC_D_MID_P2, F_C112GX4R1P0_DFE_DC_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_DFE_VREF:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_VREF_TOP_P1, F_C112GX4R1P0_DFE_VREF_MID_P1, F_C112GX4R1P0_DFE_VREF_BOT_P1 },
                { F_C112GX4R1P0_DFE_VREF_TOP_P2, F_C112GX4R1P0_DFE_VREF_MID_P2, F_C112GX4R1P0_DFE_VREF_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_DFE_F0:
        {
            MCESD_U32 tempVal;
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F0_D_TOP_P1, F_C112GX4R1P0_DFE_F0_D_MID_P1, F_C112GX4R1P0_DFE_F0_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F0_D_TOP_P2, F_C112GX4R1P0_DFE_F0_D_MID_P2, F_C112GX4R1P0_DFE_F0_D_BOT_P2 } };

            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), &tempVal));
            *tapValue = tempVal * 1000; /* F0 tap is not signed. Unlike other DFE taps, we do not treat it like signed magnitude */
        }
        break;
    case C112GX4_DFE_F1:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F1(devPtr, lane, p1DisableData, acqRate, eyeTmb, tapValue));
        break;
    case C112GX4_DFE_F2:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F2(devPtr, lane, p1DisableData, acqRate, eyeTmb, tapValue));
        break;
    case C112GX4_DFE_F3:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F3(devPtr, lane, p1DisableData, acqRate, eyeTmb, tapValue));
        break;
    case C112GX4_DFE_F4:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F4(devPtr, lane, p1DisableData, acqRate, eyeTmb, tapValue));
        break;
    case C112GX4_DFE_F5:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F5(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F6:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F6(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F7:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F7(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F8:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F8(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F9:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F9(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F10:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F10(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F11:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F11(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F12:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F12(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F13:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F13(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F14:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F14(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F15:
        MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_F15(devPtr, lane, p1DisableData, acqRate, tapValue));
        break;
    case C112GX4_DFE_F16:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F16_MSB_P1, F_C112GX4R1P0_DFE_F16_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F16_LSB_P1, F_C112GX4R1P0_DFE_F16_LSB_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F16 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F17:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F17_P1, F_C112GX4R1P0_DFE_F17_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F17 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F18:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F18_MSB_P1, F_C112GX4R1P0_DFE_F18_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F18_LSB_P1, F_C112GX4R1P0_DFE_F18_LSB_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F18 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F19:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F19_P1, F_C112GX4R1P0_DFE_F19_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F19 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F20:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F20_MSB_P1, F_C112GX4R1P0_DFE_F20_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F20_LSB_P1, F_C112GX4R1P0_DFE_F20_LSB_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F20 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F21:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F21_P1, F_C112GX4R1P0_DFE_F21_P2 };
            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F21 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F22:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F22_P1, F_C112GX4R1P0_DFE_F22_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F22 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F23:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F23_P1, F_C112GX4R1P0_DFE_F23_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F23 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F24:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F24_P1, F_C112GX4R1P0_DFE_F24_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F24 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F25:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F25_P1, F_C112GX4R1P0_DFE_F25_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F25 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F26:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F26_P1, F_C112GX4R1P0_DFE_F26_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F26 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F27:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F27_P1, F_C112GX4R1P0_DFE_F27_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F27 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F28:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F28_P1, F_C112GX4R1P0_DFE_F28_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F28 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F29:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F29_P1, F_C112GX4R1P0_DFE_F29_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F29 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_F30:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F30_P1, F_C112GX4R1P0_DFE_F30_P2 };

            if (acqRate != C112GX4_RATE_QUARTER)
                return MCESD_FAIL; /* F30 tap is only available in Quarter rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF0:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F15_P1, F_C112GX4R1P0_DFE_F15_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF0 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF1:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F17_P1, F_C112GX4R1P0_DFE_F17_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF1 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF2:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F19_P1, F_C112GX4R1P0_DFE_F19_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF2 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF3:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F21_P1, F_C112GX4R1P0_DFE_F21_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF3 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF4:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F23_P1, F_C112GX4R1P0_DFE_F23_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF4 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF5:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F25_P1, F_C112GX4R1P0_DFE_F25_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF5 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF6:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F27_P1, F_C112GX4R1P0_DFE_F27_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF6 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_DFE_FF7:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F29_P1, F_C112GX4R1P0_DFE_F29_P2 };

            if (acqRate != C112GX4_RATE_HALF)
                return MCESD_FAIL; /* FF7 tap is only availabe in Half rate */

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    default:
        return MCESD_FAIL; /* Invalid tap */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_ConvertToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldPtr, OUT MCESD_32 *tapValue)
{
    MCESD_U32 code;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, fieldPtr, &code));
    *tapValue = ConvertSignedMagnitudeToI32(code, fieldPtr->totalBits) * 1000; /* milli-Codes */
    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldTopPtr, IN MCESD_FIELD_PTR fieldMidPtr, IN MCESD_FIELD_PTR fieldBotPtr, OUT MCESD_32 *tapValue)
{
    MCESD_32 top, mid, bot;

    MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, fieldTopPtr, &top));
    MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, fieldMidPtr, &mid));
    MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, fieldBotPtr, &bot));

    *tapValue = (top + mid + bot) / 3;
    return MCESD_OK;
}


MCESD_STATUS INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldMSBPtr, IN MCESD_FIELD_PTR fieldLSBPtr, OUT MCESD_32 *tapValue)
{
    MCESD_U32 msbData, lsbData;
    MCESD_32 msb, lsb;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, fieldMSBPtr, &msbData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, fieldLSBPtr, &lsbData));
    msb = ConvertSignedMagnitudeToI32(msbData, fieldMSBPtr->totalBits);
    lsb = ConvertSignedMagnitudeToI32(lsbData, fieldLSBPtr->totalBits);
    *tapValue = (((msb * 2) + lsb) * 1000) / 3; /* milli-Codes */
    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F1
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    IN E_C112GX4_EYE_TMB eyeTmb,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        *tapValue = 0;
        return MCESD_FAIL; /* F1 Tap not available for quarter rate */
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F2_D_TOP_P1, F_C112GX4R1P0_DFE_F2_D_MID_P1, F_C112GX4R1P0_DFE_F2_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F2_D_TOP_P2, F_C112GX4R1P0_DFE_F2_D_MID_P2, F_C112GX4R1P0_DFE_F2_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F4_D_TOP_P1, F_C112GX4R1P0_DFE_F4_D_MID_P1, F_C112GX4R1P0_DFE_F4_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F4_D_TOP_P2, F_C112GX4R1P0_DFE_F4_D_MID_P2, F_C112GX4R1P0_DFE_F4_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F2
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    IN E_C112GX4_EYE_TMB eyeTmb,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F2_D_TOP_P1, F_C112GX4R1P0_DFE_F2_D_MID_P1, F_C112GX4R1P0_DFE_F2_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F2_D_TOP_P2, F_C112GX4R1P0_DFE_F2_D_MID_P2, F_C112GX4R1P0_DFE_F2_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F4_D_TOP_P1, F_C112GX4R1P0_DFE_F4_D_MID_P1, F_C112GX4R1P0_DFE_F4_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F4_D_TOP_P2, F_C112GX4R1P0_DFE_F4_D_MID_P2, F_C112GX4R1P0_DFE_F4_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F8_TOP_P1, F_C112GX4R1P0_DFE_F8_MID_P1, F_C112GX4R1P0_DFE_F8_BOT_P1 },
                { F_C112GX4R1P0_DFE_F8_TOP_P2, F_C112GX4R1P0_DFE_F8_MID_P2, F_C112GX4R1P0_DFE_F8_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F3
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    IN E_C112GX4_EYE_TMB eyeTmb,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F3_D_TOP_P1, F_C112GX4R1P0_DFE_F3_D_MID_P1, F_C112GX4R1P0_DFE_F3_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F3_D_TOP_P2, F_C112GX4R1P0_DFE_F3_D_MID_P2, F_C112GX4R1P0_DFE_F3_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F6_TOP_P1, F_C112GX4R1P0_DFE_F6_MID_P1, F_C112GX4R1P0_DFE_F6_BOT_P1 },
                { F_C112GX4R1P0_DFE_F6_TOP_P2, F_C112GX4R1P0_DFE_F6_MID_P2, F_C112GX4R1P0_DFE_F6_BOT_P2 } };

            /* Top, Mid and Bot values are averaged */
            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &(fieldList[p1DisableData][0]), &(fieldList[p1DisableData][1]), &(fieldList[p1DisableData][2]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F12_MSB_P1, F_C112GX4R1P0_DFE_F12_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F12_LSB_P1, F_C112GX4R1P0_DFE_F12_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F4
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    IN E_C112GX4_EYE_TMB eyeTmb,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F4_D_TOP_P1, F_C112GX4R1P0_DFE_F4_D_MID_P1, F_C112GX4R1P0_DFE_F4_D_BOT_P1 },
                { F_C112GX4R1P0_DFE_F4_D_TOP_P2, F_C112GX4R1P0_DFE_F4_D_MID_P2, F_C112GX4R1P0_DFE_F4_D_BOT_P2 } };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData][eyeTmb]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F8_TOP_P1, F_C112GX4R1P0_DFE_F8_MID_P1, F_C112GX4R1P0_DFE_F8_BOT_P1 },
                { F_C112GX4R1P0_DFE_F8_TOP_P2, F_C112GX4R1P0_DFE_F8_MID_P2, F_C112GX4R1P0_DFE_F8_BOT_P2 } };

            /* Top, Mid and Bot values are averaged */
            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &(fieldList[p1DisableData][0]), &(fieldList[p1DisableData][1]), &(fieldList[p1DisableData][2]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F16_MSB_P1, F_C112GX4R1P0_DFE_F16_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F16_LSB_P1, F_C112GX4R1P0_DFE_F16_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F5
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F5_TOP_P1, F_C112GX4R1P0_DFE_F5_MID_P1, F_C112GX4R1P0_DFE_F5_BOT_P1 },
                { F_C112GX4R1P0_DFE_F5_TOP_P2, F_C112GX4R1P0_DFE_F5_MID_P2, F_C112GX4R1P0_DFE_F5_BOT_P2 } };

            /* Top, Mid and Bot values are averaged */
            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &(fieldList[p1DisableData][0]), &(fieldList[p1DisableData][1]), &(fieldList[p1DisableData][2]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F10_MSB_P1, F_C112GX4R1P0_DFE_F10_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F10_LSB_P1, F_C112GX4R1P0_DFE_F10_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F20_MSB_P1, F_C112GX4R1P0_DFE_F20_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F20_LSB_P1, F_C112GX4R1P0_DFE_F20_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F6
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F6_TOP_P1, F_C112GX4R1P0_DFE_F6_MID_P1, F_C112GX4R1P0_DFE_F6_BOT_P1 },
                { F_C112GX4R1P0_DFE_F6_TOP_P2, F_C112GX4R1P0_DFE_F6_MID_P2, F_C112GX4R1P0_DFE_F6_BOT_P2 } };

            /* Top, Mid and Bot values are averaged */
            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &(fieldList[p1DisableData][0]), &(fieldList[p1DisableData][1]), &(fieldList[p1DisableData][2]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F12_MSB_P1, F_C112GX4R1P0_DFE_F12_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F12_LSB_P1, F_C112GX4R1P0_DFE_F12_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F24_P1, F_C112GX4R1P0_DFE_F24_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F7
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F7_TOP_P1, F_C112GX4R1P0_DFE_F7_MID_P1, F_C112GX4R1P0_DFE_F7_BOT_P1 },
                { F_C112GX4R1P0_DFE_F7_TOP_P2, F_C112GX4R1P0_DFE_F7_MID_P2, F_C112GX4R1P0_DFE_F7_BOT_P2 } };

            /* Top, Mid and Bot values are averaged */
            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &(fieldList[p1DisableData][0]), &(fieldList[p1DisableData][1]), &(fieldList[p1DisableData][2]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F14_MSB_P1, F_C112GX4R1P0_DFE_F14_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F14_LSB_P1, F_C112GX4R1P0_DFE_F14_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F28_P1, F_C112GX4R1P0_DFE_F28_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F8
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[2][3] = {
                { F_C112GX4R1P0_DFE_F8_TOP_P1, F_C112GX4R1P0_DFE_F8_MID_P1, F_C112GX4R1P0_DFE_F8_BOT_P1 },
                { F_C112GX4R1P0_DFE_F8_TOP_P2, F_C112GX4R1P0_DFE_F8_MID_P2, F_C112GX4R1P0_DFE_F8_BOT_P2 } };

            /* Top, Mid and Bot values are averaged */
            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &(fieldList[p1DisableData][0]), &(fieldList[p1DisableData][1]), &(fieldList[p1DisableData][2]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F16_MSB_P1, F_C112GX4R1P0_DFE_F16_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F16_LSB_P1, F_C112GX4R1P0_DFE_F16_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F8 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F9
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F9_MSB_P1, F_C112GX4R1P0_DFE_F9_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F9_LSB_P1, F_C112GX4R1P0_DFE_F9_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F18_MSB_P1, F_C112GX4R1P0_DFE_F18_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F18_LSB_P1, F_C112GX4R1P0_DFE_F18_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F9 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F10
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F10_MSB_P1, F_C112GX4R1P0_DFE_F10_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F10_LSB_P1, F_C112GX4R1P0_DFE_F10_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F20_MSB_P1, F_C112GX4R1P0_DFE_F20_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F20_LSB_P1, F_C112GX4R1P0_DFE_F20_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F10 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F11
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F11_P1, F_C112GX4R1P0_DFE_F11_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F22_P1, F_C112GX4R1P0_DFE_F22_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F11 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F12
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F12_MSB_P1, F_C112GX4R1P0_DFE_F12_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F12_LSB_P1, F_C112GX4R1P0_DFE_F12_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F24_P1, F_C112GX4R1P0_DFE_F24_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F12 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F13
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F13_P1, F_C112GX4R1P0_DFE_F13_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F26_P1, F_C112GX4R1P0_DFE_F26_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F13 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F14
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldListMSB[] = { F_C112GX4R1P0_DFE_F14_MSB_P1, F_C112GX4R1P0_DFE_F14_MSB_P2 };
            MCESD_FIELD fieldListLSB[] = { F_C112GX4R1P0_DFE_F14_LSB_P1, F_C112GX4R1P0_DFE_F14_LSB_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &(fieldListMSB[p1DisableData]), &(fieldListLSB[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F28_P1, F_C112GX4R1P0_DFE_F28_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F14 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C112GX4_GetDfeTap_F15
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 p1DisableData,
    IN E_C112GX4_DATA_ACQ_RATE acqRate,
    OUT MCESD_32 *tapValue
)
{
    switch (acqRate)
    {
    case C112GX4_RATE_QUARTER:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F15_P1, F_C112GX4R1P0_DFE_F15_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_HALF:
        {
            MCESD_FIELD fieldList[] = { F_C112GX4R1P0_DFE_F30_P1, F_C112GX4R1P0_DFE_F30_P2 };

            MCESD_ATTEMPT(INT_C112GX4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[p1DisableData]), tapValue));
        }
        break;
    case C112GX4_RATE_FULL:
        *tapValue = 0;
        return MCESD_FAIL; /* F15 Tap not available for full rate */
    default:
        *tapValue = 0;
        return MCESD_FAIL; /* Unsupported data acquisition rate */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD mcuEnLane0 = F_C112GX4R1P0_MCU_EN_LANE0;
    MCESD_FIELD mcuEnLane1 = F_C112GX4R1P0_MCU_EN_LANE1;
    MCESD_FIELD mcuEnLane2 = F_C112GX4R1P0_MCU_EN_LANE2;
    MCESD_FIELD mcuEnLane3 = F_C112GX4R1P0_MCU_EN_LANE3;
    MCESD_FIELD mcuEnCmn = F_C112GX4R1P0_MCU_EN_CMN;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane3, state));
        break;
    case 255: /* Common */
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &mcuEnCmn, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD mcuEnLane0 = F_C112GX4R1P0_MCU_EN_LANE0;
    MCESD_FIELD mcuEnLane1 = F_C112GX4R1P0_MCU_EN_LANE1;
    MCESD_FIELD mcuEnLane2 = F_C112GX4R1P0_MCU_EN_LANE2;
    MCESD_FIELD mcuEnLane3 = F_C112GX4R1P0_MCU_EN_LANE3;
    MCESD_FIELD mcuEnCmn = F_C112GX4R1P0_MCU_EN_CMN;
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane3, &data));
        break;
    case 255:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &mcuEnCmn, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD enLane0 = F_C112GX4R1P0_EN_LANE0;
    MCESD_FIELD enLane1 = F_C112GX4R1P0_EN_LANE1;
    MCESD_FIELD enLane2 = F_C112GX4R1P0_EN_LANE2;
    MCESD_FIELD enLane3 = F_C112GX4R1P0_EN_LANE3;
    MCESD_FIELD enCmn = F_C112GX4R1P0_EN_CMN;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &enLane0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &enLane1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &enLane2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &enLane3, state));
        break;
    case 255:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &enCmn, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD enLane0 = F_C112GX4R1P0_EN_LANE0;
    MCESD_FIELD enLane1 = F_C112GX4R1P0_EN_LANE1;
    MCESD_FIELD enLane2 = F_C112GX4R1P0_EN_LANE2;
    MCESD_FIELD enLane3 = F_C112GX4R1P0_EN_LANE3;
    MCESD_FIELD enCmn = F_C112GX4R1P0_EN_CMN;
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &enLane0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &enLane1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &enLane2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &enLane3, &data));
        break;
    case 255:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &enCmn, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD broadcast = F_C112GX4R1P0_BROADCAST;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* unused */, &broadcast, state));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD broadcast = F_C112GX4R1P0_BROADCAST;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* unused */, &broadcast, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puPll = F_C112GX4R1P0_PU_PLL;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &puPll, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_PLL0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_PLL1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_PLL2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_PLL3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puPll = F_C112GX4R1P0_PU_PLL;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &puPll, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;

}
#else
MCESD_STATUS API_C112GX4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_PLL0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_PLL1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_PLL2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_PLL3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puTx = F_C112GX4R1P0_PU_TX;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &puTx, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_TX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_TX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_TX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_TX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puTx = F_C112GX4R1P0_PU_TX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &puTx, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_TX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_TX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_TX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_TX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puRx = F_C112GX4R1P0_PU_RX;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &puRx, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_RX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_RX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_RX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_RX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puRx = F_C112GX4R1P0_PU_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &puRx, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_RX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_RX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_RX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_RX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_PHYMODE mode
)
{
    MCESD_FIELD phyMode = F_C112GX4R1P0_PHY_MODE;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* Ignored */, &phyMode, mode));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_MODE, mode));

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C112GX4_PHYMODE *mode
)
{
    MCESD_FIELD phyMode = F_C112GX4R1P0_PHY_MODE;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* Ignored */, &phyMode, &data));
    *mode = (E_C112GX4_PHYMODE)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C112GX4_PHYMODE *mode
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PHY_MODE, &pinValue));
    *mode = (E_C112GX4_PHYMODE)pinValue;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_REFFREQ freq,
    IN E_C112GX4_REFCLK_SEL clkSel
)
{
    MCESD_FIELD refFrefSel = F_C112GX4R1P0_REF_FREF_SEL;
    MCESD_FIELD refClkSel = F_C112GX4R1P0_REFCLK_SEL;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &refFrefSel, freq));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &refClkSel, clkSel));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_REFFREQ freq,
    IN E_C112GX4_REFCLK_SEL clkSel
)
{
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_REF_FREF_SEL, freq));
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_REFCLK_SEL, clkSel));

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C112GX4_REFFREQ *freq,
    OUT E_C112GX4_REFCLK_SEL *clkSel
)
{
    MCESD_FIELD refFrefSel  = F_C112GX4R1P0_REF_FREF_SEL;
    MCESD_FIELD refClkSel   = F_C112GX4R1P0_REFCLK_SEL;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &refFrefSel, &data));
    *freq = (E_C112GX4_REFFREQ)data;
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &refClkSel, &data));
    *clkSel = (E_C112GX4_REFCLK_SEL)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C112GX4_REFFREQ *freq,
    OUT E_C112GX4_REFCLK_SEL *clkSel
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_REF_FREF_SEL, &pinValue));
    *freq = (E_C112GX4_REFFREQ)pinValue;

    MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_REFCLK_SEL, &pinValue));
    *clkSel = (E_C112GX4_REFCLK_SEL)pinValue;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_SERDES_SPEED speed
)
{
    MCESD_FIELD phyGenMax   = F_C112GX4R1P0_PHY_GEN_MAX;
    MCESD_FIELD phyGenTx    = F_C112GX4R1P0_PHY_GEN_TX;
    MCESD_FIELD phyGenRx    = F_C112GX4R1P0_PHY_GEN_RX;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &phyGenTx, speed));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &phyGenRx, speed));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &phyGenMax, 0x17));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_SERDES_SPEED speed
)
{
    MCESD_FIELD phyGenMax = F_C112GX4R1P0_PHY_GEN_MAX;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_TX0, speed));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX0, speed));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_TX1, speed));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX1, speed));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_TX2, speed));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX2, speed));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_TX3, speed));
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX3, speed));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &phyGenMax, 0x17));

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_SERDES_SPEED *speed
)
{
    MCESD_FIELD phyGenRx = F_C112GX4R1P0_PHY_GEN_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &phyGenRx, &data));
    *speed = (E_C112GX4_SERDES_SPEED)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_SERDES_SPEED *speed
)
{
    MCESD_U16 pinValue;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PHY_GEN_RX3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *speed = (E_C112GX4_SERDES_SPEED)pinValue;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C112GX4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_DATABUS_WIDTH txWidth,
    IN E_C112GX4_DATABUS_WIDTH rxWidth
)
{
    MCESD_FIELD txSelBits       = F_C112GX4R1P0_TX_SEL_BITS;
    MCESD_FIELD txHalfRateEn    = F_C112GX4R1P0_TX_HALFRATE_EN;
    MCESD_FIELD txPam2En        = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD rxSelBits       = F_C112GX4R1P0_RX_SEL_BITS;
    MCESD_FIELD rxHalfRateEn    = F_C112GX4R1P0_RX_HALFRATE_EN;
    MCESD_FIELD rxPam2En        = F_C112GX4R1P0_RX_PAM2_EN;

    /* TX Width */
    switch (txWidth)
    {
    case C112GX4_DATABUS_160BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txSelBits, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txHalfRateEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPam2En, 0));
        break;
    case C112GX4_DATABUS_128BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txSelBits, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txHalfRateEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPam2En, 0));
        break;
    case C112GX4_DATABUS_80BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txSelBits, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPam2En, 0));
        break;
    case C112GX4_DATABUS_64BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txSelBits, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPam2En, 0));
        break;
    case C112GX4_DATABUS_40BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txSelBits, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPam2En, 1));
        break;
    case C112GX4_DATABUS_32BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txSelBits, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPam2En, 1));
        break;
    default:
        return MCESD_FAIL; /* Unsupported data bus width */
    }

    /* RX Width */
    switch (rxWidth)
    {
    case C112GX4_DATABUS_160BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxSelBits, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxHalfRateEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPam2En, 0));
        break;
    case C112GX4_DATABUS_128BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxSelBits, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxHalfRateEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPam2En, 0));
        break;
    case C112GX4_DATABUS_80BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxSelBits, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPam2En, 0));
        break;
    case C112GX4_DATABUS_64BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxSelBits, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPam2En, 0));
        break;
    case C112GX4_DATABUS_40BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxSelBits, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPam2En, 1));
        break;
    case C112GX4_DATABUS_32BIT:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxSelBits, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxHalfRateEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPam2En, 1));
        break;
    default:
        return MCESD_FAIL; /* Unsupported data bus width */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_DATABUS_WIDTH *txWidth,
    OUT E_C112GX4_DATABUS_WIDTH *rxWidth
)
{
    MCESD_FIELD txSelBits       = F_C112GX4R1P0_TX_SEL_BITS;
    MCESD_FIELD txHalfRateEn    = F_C112GX4R1P0_TX_HALFRATE_EN;
    MCESD_FIELD txPam2En        = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD rxSelBits       = F_C112GX4R1P0_RX_SEL_BITS;
    MCESD_FIELD rxHalfRateEn    = F_C112GX4R1P0_RX_HALFRATE_EN;
    MCESD_FIELD rxPam2En        = F_C112GX4R1P0_RX_PAM2_EN;
    MCESD_U32 dataTxSelBits, dataTxHalfRateEn, dataTxPam2En, dataRxSelBits, dataRxHalfRateEn, dataRxPam2En;

    /* TX Width */
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txSelBits, &dataTxSelBits));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txHalfRateEn, &dataTxHalfRateEn));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txPam2En, &dataTxPam2En));

    *txWidth = C112GX4_DATABUS_UNSUPPORTED;

    if (0 == dataTxSelBits)
    {
        if (0 == dataTxHalfRateEn)
            *txWidth = (0 == dataTxPam2En) ? C112GX4_DATABUS_160BIT : C112GX4_DATABUS_UNSUPPORTED;
        else
            *txWidth = (0 == dataTxPam2En) ? C112GX4_DATABUS_80BIT : C112GX4_DATABUS_40BIT;
    }
    else
    {
        if (0 == dataTxHalfRateEn)
            *txWidth = (0 == dataTxPam2En) ? C112GX4_DATABUS_128BIT : C112GX4_DATABUS_UNSUPPORTED;
        else
            *txWidth = (0 == dataTxPam2En) ? C112GX4_DATABUS_64BIT : C112GX4_DATABUS_32BIT;
    }

    /* RX Width */
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxSelBits, &dataRxSelBits));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxHalfRateEn, &dataRxHalfRateEn));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxPam2En, &dataRxPam2En));

    *rxWidth = C112GX4_DATABUS_UNSUPPORTED;

    if (0 == dataRxSelBits)
    {
        if (0 == dataRxHalfRateEn)
            *rxWidth = (0 == dataRxPam2En) ? C112GX4_DATABUS_160BIT : C112GX4_DATABUS_UNSUPPORTED;
        else
            *rxWidth = (0 == dataRxPam2En) ? C112GX4_DATABUS_80BIT : C112GX4_DATABUS_40BIT;
    }
    else
    {
        if (0 == dataRxHalfRateEn)
            *rxWidth = (0 == dataRxPam2En) ? C112GX4_DATABUS_128BIT : C112GX4_DATABUS_UNSUPPORTED;
        else
            *rxWidth = (0 == dataRxPam2En) ? C112GX4_DATABUS_64BIT : C112GX4_DATABUS_32BIT;
    }

    return MCESD_OK;
}

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_FIELD mcuFreq = F_C112GX4R1P0_MCU_FREQ;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &mcuFreq, clockMHz));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_FIELD mcuFreq = F_C112GX4R1P0_MCU_FREQ;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &mcuFreq, &data));
    *clockMHz = (MCESD_U16)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD txIdle = F_C112GX4R1P0_TX_IDLE;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txIdle, state ? 0 : 1));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_IDLE0, state ? 0 : 1));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_IDLE1, state ? 0 : 1));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_IDLE2, state ? 0 : 1));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_IDLE3, state ? 0 : 1));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD txIdle = F_C112GX4R1P0_TX_IDLE;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txIdle, &data));
    *state = (0 == data) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_IDLE0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_IDLE1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_IDLE2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_IDLE3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puIvref         = F_C112GX4R1P0_PU_IVREF;
    MCESD_FIELD puIvrefFmReg    = F_C112GX4R1P0_PU_IVREF_FM_REG;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &puIvref, state));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* ignored */, &puIvrefFmReg, 1));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PU_IVREF, state));

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puIvref = F_C112GX4R1P0_PU_IVREF;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &puIvref, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_PU_IVREF, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C112GX4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD txTrainComplete = F_C112GX4R1P0_TX_TRAIN_COMPLETE;
    MCESD_FIELD txTrainFailed   = F_C112GX4R1P0_TX_TRAIN_FAILED;
    MCESD_FIELD cliStart        = F_C112GX4R1P0_CLI_START;
    MCESD_FIELD cliCmd          = F_C112GX4R1P0_CLI_CMD;
    MCESD_U32 failed;

    if (type == C112GX4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainEnable, 1));
        MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &txTrainComplete, 1, 90000));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txTrainFailed, &failed));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainEnable, 0));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliCmd, 0xF7));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 1));
        MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &cliStart, 0, 90000));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &cliStart, &failed));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 0));
    }

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}
#else
MCESD_STATUS API_C112GX4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
)
{
    MCESD_U16 failed;

    if (type == C112GX4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE0, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE0, 1, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED0, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE1, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE1, 1, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED1, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE2, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE2, 1, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED2, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE3, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE3, 1, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED3, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE3, 0));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE0, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE0, 0, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED0, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE0, 0));
            break;
        case 1:

            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE1, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE1, 0, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED1, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE2, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE2, 0, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED2, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE3, 1));
            MCESD_ATTEMPT(API_C112GX4_PollPin(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE3, 0, 90000));
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED3, &failed));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE3, 0));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C112GX4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD cliStart        = F_C112GX4R1P0_CLI_START;
    MCESD_FIELD cliCmd          = F_C112GX4R1P0_CLI_CMD;

    if (type == C112GX4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainEnable, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliCmd, 0xF7));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 1));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
)
{
    if (type == C112GX4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS0, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE0, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS1, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS2, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS3, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE3, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS0, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE0, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS1, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS2, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS3, 1));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE3, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_FIELD txTrainEnable   = F_C112GX4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD txTrainComplete = F_C112GX4R1P0_TX_TRAIN_COMPLETE;
    MCESD_FIELD txTrainFailed   = F_C112GX4R1P0_TX_TRAIN_FAILED;
    MCESD_FIELD cliStart        = F_C112GX4R1P0_CLI_START;
    MCESD_U32 completeData, failedData = 0;

    if (type == C112GX4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txTrainComplete, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txTrainFailed, &failedData));

            MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
        }
        *completed = (0 == completeData) ? MCESD_FALSE : MCESD_TRUE;
        *failed = (0 == failedData) ? MCESD_FALSE : MCESD_TRUE;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &cliStart, &completeData));
        *completed = (0 == completeData) ? MCESD_TRUE : MCESD_FALSE;
        *failed = MCESD_FALSE;
    }
    if (completed)
    {
        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
        MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U16 completeData, failedData = 0;
    if (type == C112GX4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED0, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED1, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED2, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_COMPLETE3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_FAILED3, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED0, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED1, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED2, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_COMPLETE3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_FAILED3, &failedData));
                MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C112GX4_StopTraining(devPtr,lane,type));
            }
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    *completed = (0 == completeData) ? MCESD_FALSE : MCESD_TRUE;
    *failed = (0 == failedData) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C112GX4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD cliStart        = F_C112GX4R1P0_CLI_START;

    if (type == C112GX4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 0));
        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type
)
{
    if (type == C112GX4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS0, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS1, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS2, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_TX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS3, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS0, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS1, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS2, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_RX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DFE_PAT_DIS3, 0));
            MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C112GX4_SetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    IN S_C112GX4_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD rxTrainTimer        = F_C112GX4R1P0_RX_TRAIN_TIMER;
    MCESD_FIELD trxTrainTimer       = F_C112GX4R1P0_TRX_TRAIN_TIMER;
    MCESD_FIELD txTrainTimerEnable  = F_C112GX4R1P0_TX_TRAIN_TIMER_EN;
    MCESD_FIELD rxTrainTimerEnable  = F_C112GX4R1P0_RX_TRAIN_TIMER_EN;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C112GX4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &trxTrainTimer, training->timeout));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxTrainTimer, training->timeout));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TRAINING type,
    OUT S_C112GX4_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD rxTrainTimer        = F_C112GX4R1P0_RX_TRAIN_TIMER;
    MCESD_FIELD trxTrainTimer       = F_C112GX4R1P0_TRX_TRAIN_TIMER;
    MCESD_FIELD txTrainTimerEnable  = F_C112GX4R1P0_TX_TRAIN_TIMER_EN;
    MCESD_FIELD rxTrainTimerEnable  = F_C112GX4R1P0_RX_TRAIN_TIMER_EN;
    MCESD_U32 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C112GX4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &trxTrainTimer, &timeoutData));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxTrainTimer, &timeoutData));
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = (MCESD_U16)timeoutData;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD eomReady        = F_C112GX4R1P0_EOM_READY;
    MCESD_FIELD dfeAdaptSplrEn  = F_C112GX4R1P0_DFE_ADAPT_SPLR_EN;
    MCESD_FIELD esmPathSel      = F_C112GX4R1P0_ESM_PATH_SEL;
    MCESD_FIELD esmEn           = F_C112GX4R1P0_ESM_EN;
    MCESD_FIELD esmLpnum        = F_C112GX4R1P0_ESM_LPNUM;
    MCESD_FIELD adaptEven       = F_C112GX4R1P0_ADAPT_EVEN;
    MCESD_FIELD adaptOdd        = F_C112GX4R1P0_ADAPT_ODD;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &esmPathSel, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dfeAdaptSplrEn, 0xF));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &adaptEven, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &adaptOdd, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &esmEn, 1));
    MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &eomReady, 1, 5000));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &esmLpnum, 0xFF));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD esmEn = F_C112GX4R1P0_ESM_EN;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &esmEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTMB,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_C112GX4_EOM_DATA *measurement
)
{
    MCESD_FIELD esmPhase            = F_C112GX4R1P0_ESM_PHASE;
    MCESD_FIELD esmVoltage          = F_C112GX4R1P0_ESM_VOLTAGE;
    MCESD_FIELD eomDfeCall          = F_C112GX4R1P0_EOM_DFE_CALL;
    MCESD_FIELD eomVldCntTP_39_32   = F_C112GX4R1P0_EOM_VC_T_P_39_32;
    MCESD_FIELD eomVldCntTP_31_00   = F_C112GX4R1P0_EOM_VC_T_P_31_00;
    MCESD_FIELD eomVldCntTN_39_32   = F_C112GX4R1P0_EOM_VC_T_N_39_32;
    MCESD_FIELD eomVldCntTN_31_00   = F_C112GX4R1P0_EOM_VC_T_N_31_00;
    MCESD_FIELD eomVldCntMP_39_32   = F_C112GX4R1P0_EOM_VC_M_P_39_32;
    MCESD_FIELD eomVldCntMP_31_00   = F_C112GX4R1P0_EOM_VC_M_P_31_00;
    MCESD_FIELD eomVldCntMN_39_32   = F_C112GX4R1P0_EOM_VC_M_N_39_32;
    MCESD_FIELD eomVldCntMN_31_00   = F_C112GX4R1P0_EOM_VC_M_N_31_00;
    MCESD_FIELD eomVldCntBP_39_32   = F_C112GX4R1P0_EOM_VC_B_P_39_32;
    MCESD_FIELD eomVldCntBP_31_00   = F_C112GX4R1P0_EOM_VC_B_P_31_00;
    MCESD_FIELD eomVldCntBN_39_32   = F_C112GX4R1P0_EOM_VC_B_N_39_32;
    MCESD_FIELD eomVldCntBN_31_00   = F_C112GX4R1P0_EOM_VC_B_N_31_00;
    MCESD_FIELD eomErrCntTP         = F_C112GX4R1P0_EOM_EC_T_P;
    MCESD_FIELD eomErrCntTN         = F_C112GX4R1P0_EOM_EC_T_N;
    MCESD_FIELD eomErrCntMP         = F_C112GX4R1P0_EOM_EC_M_P;
    MCESD_FIELD eomErrCntMN         = F_C112GX4R1P0_EOM_EC_M_N;
    MCESD_FIELD eomErrCntBP         = F_C112GX4R1P0_EOM_EC_B_P;
    MCESD_FIELD eomErrCntBN         = F_C112GX4R1P0_EOM_EC_B_N;
    MCESD_U32 vldCntPData_39_32, vldCntPData_31_00, vldCntNData_39_32, vldCntNData_31_00, errCntPData, errCntNData;

    if (NULL == measurement)
    {
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &esmPhase, phase));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &esmVoltage, voltage));

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &eomDfeCall, 1));
    MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &eomDfeCall, 0, 1000));

    switch (eyeTMB)
    {
    case C112GX4_EYE_TOP:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntTP_39_32, &vldCntPData_39_32));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntTP_31_00, &vldCntPData_31_00));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntTN_39_32, &vldCntNData_39_32));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntTN_31_00, &vldCntNData_31_00));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomErrCntTP, &errCntPData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomErrCntTN, &errCntNData));
        break;
    case C112GX4_EYE_MID:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntMP_39_32, &vldCntPData_39_32));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntMP_31_00, &vldCntPData_31_00));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntMN_39_32, &vldCntNData_39_32));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntMN_31_00, &vldCntNData_31_00));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomErrCntMP, &errCntPData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomErrCntMN, &errCntNData));
        break;
    case C112GX4_EYE_BOT:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntBP_39_32, &vldCntPData_39_32));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntBP_31_00, &vldCntPData_31_00));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntBN_39_32, &vldCntNData_39_32));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomVldCntBN_31_00, &vldCntNData_31_00));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomErrCntBP, &errCntPData));
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &eomErrCntBN, &errCntNData));
        break;
    default:
        return MCESD_FAIL;  /* Invalid eye */
    }

    measurement->phase = phase;
    measurement->voltage = voltage;
    measurement->upperBitCount = MAKEU64FROMU32((MCESD_U64)vldCntPData_39_32, vldCntPData_31_00);
    measurement->upperBitErrorCount = errCntPData;
    measurement->lowerBitCount = MAKEU64FROMU32((MCESD_U64)vldCntNData_39_32, vldCntNData_31_00);
    measurement->lowerBitErrorCount = errCntNData;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *phaseStepCount,
    OUT MCESD_U16 *voltageStepCount
)
{
    E_C112GX4_SERDES_SPEED speed;

    MCESD_ATTEMPT(API_C112GX4_GetTxRxBitRate(devPtr, lane, &speed));
    if ((C112GX4_SERDES_106GP25G == speed) || (C112GX4_SERDES_112G == speed))
    {
        *phaseStepCount = 64;
    }
    else if ((C112GX4_SERDES_20P625G == speed) || (C112GX4_SERDES_25P78125G == speed) || (C112GX4_SERDES_28P125G == speed) || (C112GX4_SERDES_53P125G == speed) || (C112GX4_SERDES_56P25G == speed))
    {
        *phaseStepCount = 128;
    }
    else if (C112GX4_SERDES_10P3125G == speed)
    {
        *phaseStepCount = 256;
    }
    else if ((C112GX4_SERDES_5P15625G == speed) || (C112GX4_SERDES_6P25G == speed))
    {
        *phaseStepCount = 512;
    }
    else if ((C112GX4_SERDES_2P5G == speed) || (C112GX4_SERDES_3P125G == speed))
    {
        *phaseStepCount = 1024;
    }
    else
    {
        *phaseStepCount = 2048;
    }

    *voltageStepCount = 64;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTMB,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *heightUpper,
    OUT MCESD_U16* heightLower
)
{
    MCESD_32 leftEdge, rightEdge, upperEdge, lowerEdge, maxPhase, minPhase, maxVoltage, phase, voltage;
    MCESD_U16 phaseStepCount, voltageStepCount;
    S_C112GX4_EOM_DATA measurement;

    MCESD_ATTEMPT(API_C112GX4_EOM1UIStepCount(devPtr, lane, &phaseStepCount, &voltageStepCount));

    maxPhase = phaseStepCount / 2;
    minPhase = -phaseStepCount / 2;
    maxVoltage = voltageStepCount;

    /* Set default edge values */
    leftEdge = minPhase;
    rightEdge = maxPhase;
    upperEdge = -maxVoltage;
    lowerEdge = maxVoltage - 1;

    MCESD_ATTEMPT(API_C112GX4_EOMInit(devPtr, lane));

    /* Scan Left */
    for (phase = 0; phase < maxPhase; phase++)
    {
        MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, phase, 0, &measurement));

        /* Stop when no bits read or error */
        if ((0 == measurement.upperBitCount) || (measurement.upperBitErrorCount > 0) || (0 == measurement.lowerBitCount) || (measurement.lowerBitErrorCount > 0))
        {
            leftEdge = phase;
            break;
        }
    }

    /* Scan Right */
    for (phase = -1; phase > minPhase; phase--)
    {
        MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, phase, 0, &measurement));

        /* Stop when no bits read or error */
        if ((0 == measurement.upperBitCount) || (measurement.upperBitErrorCount > 0) || (0 == measurement.lowerBitCount) || (measurement.lowerBitErrorCount > 0))
        {
            rightEdge = phase;
            break;
        }
    }

    /* Default if either edge did not update */
    if ((leftEdge == minPhase) || (rightEdge == maxPhase))
    {
        leftEdge = 0;
        rightEdge = 0;
    }

    /* Scan up and down */
    for (voltage = 0; voltage < maxVoltage; voltage++)
    {
        MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, 0, (MCESD_U8)voltage, &measurement));

        /* Update once when no bits read or error */
        if ((upperEdge == -maxVoltage) && ((0 == measurement.upperBitCount) || (measurement.upperBitErrorCount > 0)))
        {
            upperEdge = voltage;
        }

        /* Update once when no bits read or error */
        if ((lowerEdge == (maxVoltage - 1)) && ((0 == measurement.lowerBitCount) || (measurement.lowerBitErrorCount > 0)))
        {
            lowerEdge = -voltage;
        }

        /* Stop when both edges are found */
        if ((upperEdge != -maxVoltage) && (lowerEdge != (maxVoltage - 1)))
        {
            break;
        }
    }

    /* Default if either edge did not update */
    if ((upperEdge == -maxVoltage) || (lowerEdge == (maxVoltage - 1)))
    {
        upperEdge = maxVoltage / 2;
        lowerEdge = -maxVoltage / 2;
    }

    *width = (MCESD_U16)((leftEdge == rightEdge) ? 0 : (leftEdge - rightEdge - 1));
    if (upperEdge == lowerEdge)
    {
        *heightUpper = 0;
        *heightLower = 0;
    }
    else
    {
        *heightUpper = (MCESD_U16) upperEdge;
        *heightLower = (MCESD_U16) ((lowerEdge < 0) ? -lowerEdge : lowerEdge);
    }
    MCESD_ATTEMPT(API_C112GX4_EOMFinalize(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C112GX4_TRAINED_EYE_HEIGHT *trainedEyeHeight
)
{
    MCESD_FIELD trainF0a    = F_C112GX4R1P0_TRAIN_F0A;
    MCESD_FIELD trainF0aMax = F_C112GX4R1P0_TRAIN_F0A_MAX;
    MCESD_FIELD trainF0b    = F_C112GX4R1P0_TRAIN_F0B;
    MCESD_FIELD trainF0d    = F_C112GX4R1P0_TRAIN_F0D;
    MCESD_FIELD trainF0x    = F_C112GX4R1P0_TRAIN_F0X;
    MCESD_U32 f0aData, f0aMaxData, f0bData, f0dData, f0xData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &trainF0a, &f0aData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &trainF0aMax, &f0aMaxData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &trainF0b, &f0bData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &trainF0d, &f0dData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &trainF0x, &f0xData));

    trainedEyeHeight->f0a = (MCESD_U8)f0aData;
    trainedEyeHeight->f0aMax = (MCESD_U8)f0aMaxData;
    trainedEyeHeight->f0b = (MCESD_U8)f0bData;
    trainedEyeHeight->f0d = (MCESD_U8)f0dData;
    trainedEyeHeight->f0x = (MCESD_U8)f0xData;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_PATTERN txPattern,
    IN E_C112GX4_PATTERN rxPattern,
    IN const char *userPattern
)
{
    MCESD_FIELD rxPatternSel        = F_C112GX4R1P0_PT_RX_PATTERN_SEL;
    MCESD_FIELD userPattern_79_48   = F_C112GX4R1P0_PT_USER_PAT_79_48;
    MCESD_FIELD userPattern_47_16   = F_C112GX4R1P0_PT_USER_PAT_47_16;
    MCESD_FIELD userPattern_15_00   = F_C112GX4R1P0_PT_USER_PAT_15_00;
    MCESD_FIELD txPatternSelR1p0    = F_C112GX4R1P0_PT_TX_PATTERN_SEL;
    MCESD_FIELD txPatternSelR1p1    = F_C112GX4R1P1_PT_TX_PATTERN_SEL;
    MCESD_FIELD_PTR txPatternSelPtr;
    E_C112GX4_DATA_ACQ_RATE acqRate;
    E_C112GX4_DATABUS_WIDTH txWidth;
    E_C112GX4_DATABUS_WIDTH rxWidth;

    if ((devPtr->ipMajorRev <= 1) && (devPtr->ipMinorRev < 1))
    {
        txPatternSelPtr = &txPatternSelR1p0;
    }
    else
    {
        txPatternSelPtr = &txPatternSelR1p1;
    }

    /* TX Pattern */
    if (C112GX4_PAT_USER == txPattern)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, txPatternSelPtr, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, txPatternSelPtr, txPattern));
    }

    /* RX Pattern */
    if (C112GX4_PAT_USER == rxPattern)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPatternSel, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPatternSel, rxPattern));
    }

    /* User Pattern */
    if (strlen(userPattern) > 0) {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(userPattern, u8Pattern));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &userPattern_79_48, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3])));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &userPattern_47_16, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7])));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &userPattern_15_00, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9])));
    }

    MCESD_ATTEMPT(API_C112GX4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));
    MCESD_ATTEMPT(API_C112GX4_GetDataAcquisitionRate(devPtr, lane, &acqRate));
    if (C112GX4_RATE_FULL == acqRate)
    {
        if ((C112GX4_PAT_JITTER_8T == txPattern) || (C112GX4_PAT_JITTER_4T == txPattern))
            txWidth = C112GX4_DATABUS_32BIT;
        else if ((C112GX4_PAT_JITTER_10T == txPattern) || (C112GX4_PAT_JITTER_5T == txPattern))
            txWidth = C112GX4_DATABUS_40BIT;

        if ((C112GX4_PAT_JITTER_8T == rxPattern) || (C112GX4_PAT_JITTER_4T == rxPattern))
            rxWidth = C112GX4_DATABUS_32BIT;
        else if ((C112GX4_PAT_JITTER_10T == rxPattern) || (C112GX4_PAT_JITTER_5T == rxPattern))
            rxWidth = C112GX4_DATABUS_40BIT;
    }
    else if (C112GX4_RATE_HALF == acqRate)
    {
        if ((C112GX4_PAT_JITTER_8T == txPattern) || (C112GX4_PAT_JITTER_4T == txPattern))
            txWidth = C112GX4_DATABUS_64BIT;
        else if ((C112GX4_PAT_JITTER_10T == txPattern) || (C112GX4_PAT_JITTER_5T == txPattern))
            txWidth = C112GX4_DATABUS_80BIT;

        if ((C112GX4_PAT_JITTER_8T == rxPattern) || (C112GX4_PAT_JITTER_4T == rxPattern))
            rxWidth = C112GX4_DATABUS_64BIT;
        else if ((C112GX4_PAT_JITTER_10T == rxPattern) || (C112GX4_PAT_JITTER_5T == rxPattern))
            rxWidth = C112GX4_DATABUS_80BIT;
    }
    else if (C112GX4_RATE_QUARTER == acqRate)
    {
        if ((C112GX4_PAT_JITTER_8T == txPattern) || (C112GX4_PAT_JITTER_4T == txPattern))
            txWidth = C112GX4_DATABUS_128BIT;
        else if ((C112GX4_PAT_JITTER_10T == txPattern) || (C112GX4_PAT_JITTER_5T == txPattern))
            txWidth = C112GX4_DATABUS_160BIT;

        if ((C112GX4_PAT_JITTER_8T == rxPattern) || (C112GX4_PAT_JITTER_4T == rxPattern))
            rxWidth = C112GX4_DATABUS_128BIT;
        else if ((C112GX4_PAT_JITTER_10T == rxPattern) || (C112GX4_PAT_JITTER_5T == rxPattern))
            rxWidth = C112GX4_DATABUS_160BIT;
    }
    else
    {
        return MCESD_FAIL;  /* Unsupported acquisition rate */
    }
    MCESD_ATTEMPT(API_C112GX4_SetDataBusWidth(devPtr, lane, txWidth, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_PATTERN *txPattern,
    OUT E_C112GX4_PATTERN *rxPattern,
    OUT char *userPattern
)
{
    MCESD_FIELD rxPatternSel        = F_C112GX4R1P0_PT_RX_PATTERN_SEL;
    MCESD_FIELD txPatternSelR1p0    = F_C112GX4R1P0_PT_TX_PATTERN_SEL;
    MCESD_FIELD txPatternSelR1p1    = F_C112GX4R1P1_PT_TX_PATTERN_SEL;
    MCESD_FIELD userPatternFieldArray[] = { F_C112GX4R1P0_PT_USER_PAT_79_48 , F_C112GX4R1P0_PT_USER_PAT_47_16 , F_C112GX4R1P0_PT_USER_PAT_15_00 };
    MCESD_U8 userPatternFieldArrayCount = sizeof(userPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_U32 txPatternSelData, rxPatternSelData, userPatternData;
    MCESD_32 i;
    MCESD_U8 u8Pattern[10];
    MCESD_FIELD_PTR txPatternSelPtr;

    if ((devPtr->ipMajorRev <= 1) && (devPtr->ipMinorRev < 1))
    {
        txPatternSelPtr = &txPatternSelR1p0;
    }
    else
    {
        txPatternSelPtr = &txPatternSelR1p1;
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, txPatternSelPtr, &txPatternSelData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxPatternSel, &rxPatternSelData));

    *txPattern = (1 == txPatternSelData) ? C112GX4_PAT_USER : txPatternSelData;
    *rxPattern = (1 == rxPatternSelData) ? C112GX4_PAT_USER : rxPatternSelData;

    for (i = 0; i < userPatternFieldArrayCount; i++)
    {
        MCESD_FIELD userPatternField = userPatternFieldArray[i];
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &userPatternField, &userPatternData));
        if (i != userPatternFieldArrayCount - 1)
        {
            u8Pattern[i * 4 + 0] = (MCESD_U8)(userPatternData >> 24);
            u8Pattern[i * 4 + 1] = (MCESD_U8)((userPatternData >> 16) & 0xFF);
            u8Pattern[i * 4 + 2] = (MCESD_U8)((userPatternData >> 8) & 0xFF);
            u8Pattern[i * 4 + 3] = (MCESD_U8)(userPatternData & 0xFF);
        }
        else
        {
            u8Pattern[i * 4 + 0] = (MCESD_8)(userPatternData >> 8);
            u8Pattern[i * 4 + 1] = (MCESD_8)(userPatternData & 0xFF);
        }
    }
    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, userPattern));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_SWAP_MSB_LSB txSwapMsbLsb,
    IN E_C112GX4_SWAP_MSB_LSB rxSwapMsbLsb
)
{
    MCESD_FIELD txDSwap     = F_C112GX4R1P0_TXD_SWAP;
    MCESD_FIELD txDataSwap  = F_C112GX4R1P0_TXDATA_SWAP;
    MCESD_FIELD rxDSwap     = F_C112GX4R1P0_RXD_SWAP;
    MCESD_FIELD rxDataSwap  = F_C112GX4R1P0_RXDATA_SWAP;

    /* TX Swap MSB LSB */
    if (C112GX4_SWAP_NOT_USED != txSwapMsbLsb)
    {
        if (C112GX4_SWAP_DISABLE == txSwapMsbLsb)
        {
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txDataSwap, 0));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txDSwap, 0));
        }
        else if (C112GX4_SWAP_PRECODER == txSwapMsbLsb)
        {
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txDataSwap, 1));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txDSwap, 0));
        }
        else if (C112GX4_SWAP_POSTCODER == txSwapMsbLsb)
        {
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txDataSwap, 0));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txDSwap, 1));
        }
    }

    /* RX Pattern */
    if (C112GX4_SWAP_NOT_USED != rxSwapMsbLsb)
    {
        if (C112GX4_SWAP_DISABLE == rxSwapMsbLsb)
        {
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxDataSwap, 0));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxDSwap, 0));
        }
        else if (C112GX4_SWAP_PRECODER == rxSwapMsbLsb)
        {
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxDataSwap, 1));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxDSwap, 0));
        }
        else if (C112GX4_SWAP_POSTCODER == rxSwapMsbLsb)
        {
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxDataSwap, 0));
            MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxDSwap, 1));
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_SWAP_MSB_LSB *txSwapMsbLsb,
    OUT E_C112GX4_SWAP_MSB_LSB *rxSwapMsbLsb
)
{
    MCESD_FIELD txDSwap         = F_C112GX4R1P0_TXD_SWAP;
    MCESD_FIELD txDataSwap      = F_C112GX4R1P0_TXDATA_SWAP;
    MCESD_FIELD rxDSwap         = F_C112GX4R1P0_RXD_SWAP;
    MCESD_FIELD rxDataSwap      = F_C112GX4R1P0_RXDATA_SWAP;
    MCESD_FIELD txGrayCodeEn    = F_C112GX4R1P0_TX_GRAY_CODE_EN;
    MCESD_FIELD rxGrayCodeEn    = F_C112GX4R1P0_RX_GRAY_CODE_EN;
    MCESD_U32 txDSwapData, txDataSwapData, rxDSwapData, rxDataSwapData, txGrayCodeData, rxGrayCodeData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txDSwap, &txDSwapData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txDataSwap, &txDataSwapData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxDSwap, &rxDSwapData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxDataSwap, &rxDataSwapData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txGrayCodeEn, &txGrayCodeData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxGrayCodeEn, &rxGrayCodeData));

    /* TX Swap MSB LSB */
    if ((0 == txDataSwapData) && (0 == txDSwapData))
    {
        *txSwapMsbLsb = C112GX4_SWAP_DISABLE;
    }
    else if ((1 == txDataSwapData) && (0 == txDSwapData))
    {
        *txSwapMsbLsb = C112GX4_SWAP_PRECODER;
    }
    else if ((0 == txDataSwapData) && (1 == txDSwapData))
    {
        *txSwapMsbLsb = C112GX4_SWAP_POSTCODER;
    }
    else
    {
        *txSwapMsbLsb = C112GX4_SWAP_NOT_USED;
    }

    /* RX Swap MSB LSB */
    if ((0 == rxDataSwapData) && (0 == rxDSwapData))
    {
        *rxSwapMsbLsb = C112GX4_SWAP_DISABLE;
    }
    else if ((1 == rxDataSwapData) && (0 == rxDSwapData))
    {
        *rxSwapMsbLsb = C112GX4_SWAP_PRECODER;
    }
    else if ((0 == rxDataSwapData) && (1 == rxDSwapData))
    {
        *rxSwapMsbLsb = C112GX4_SWAP_POSTCODER;
    }
    else
    {
        *rxSwapMsbLsb = C112GX4_SWAP_NOT_USED;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_GRAY_CODE txGrayCode,
    IN E_C112GX4_GRAY_CODE rxGrayCode
)
{
    MCESD_FIELD txGrayCodeEn    = F_C112GX4R1P0_TX_GRAY_CODE_EN;
    MCESD_FIELD rxGrayCodeEn    = F_C112GX4R1P0_RX_GRAY_CODE_EN;

    /* TX Gray Code */
    if (C112GX4_GRAY_CODE_NOT_USED != txGrayCode)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txGrayCodeEn, txGrayCode));
    }

    /* RX Gray Code */
    if (C112GX4_GRAY_CODE_NOT_USED != rxGrayCode)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxGrayCodeEn, rxGrayCode));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_GRAY_CODE *txGrayCode,
    OUT E_C112GX4_GRAY_CODE *rxGrayCode
)
{
    MCESD_FIELD txGrayCodeEn    = F_C112GX4R1P0_TX_GRAY_CODE_EN;
    MCESD_FIELD rxGrayCodeEn    = F_C112GX4R1P0_RX_GRAY_CODE_EN;
    MCESD_U32 txGrayCodeData, rxGrayCodeData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txGrayCodeEn, &txGrayCodeData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxGrayCodeEn, &rxGrayCodeData));

    /* TX Gray Code */
    *txGrayCode = 1 == txGrayCodeData ? C112GX4_GRAY_CODE_ENABLE : C112GX4_GRAY_CODE_DISABLE;

    /* RX Gray Code */
    *rxGrayCode = 1 == rxGrayCodeData ? C112GX4_GRAY_CODE_ENABLE : C112GX4_GRAY_CODE_DISABLE;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txState,
    IN MCESD_BOOL rxState
)
{
    MCESD_FIELD txPreCode   = F_C112GX4R1P0_TXDATA_PRE_CODE;
    MCESD_FIELD rxPreCode   = F_C112GX4R1P0_RXDATA_PRE_CODE;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txPreCode, MCESD_TRUE == txState ? 1 : 0));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxPreCode, MCESD_TRUE == rxState ? 1 : 0));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txState,
    OUT MCESD_BOOL *rxState
)
{
    MCESD_FIELD txPreCode   = F_C112GX4R1P0_TXDATA_PRE_CODE;
    MCESD_FIELD rxPreCode   = F_C112GX4R1P0_RXDATA_PRE_CODE;
    MCESD_U32 txPreCodeData, rxPreCodeData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txPreCode, &txPreCodeData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxPreCode, &rxPreCodeData));

    *txState = 1 == txPreCodeData ? MCESD_TRUE : MCESD_FALSE;
    *rxState = 1 == rxPreCodeData ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD ptEn                = F_C112GX4R1P0_PT_EN;
    MCESD_FIELD ptRst               = F_C112GX4R1P0_PT_RST;
    MCESD_FIELD ptPrbsLoad          = F_C112GX4R1P0_PT_PRBS_LOAD;
    MCESD_FIELD ptEnModeR1p0        = F_C112GX4R1P0_PT_EN_MODE;
    MCESD_FIELD ptEnModeR1p1        = F_C112GX4R1P1_PT_EN_MODE;
    MCESD_FIELD ptPhyReadyForceR1p0 = F_C112GX4R1P0_PT_PHYREADY_FORCE;
    MCESD_FIELD ptPhyReadyForceR1p1 = F_C112GX4R1P1_PT_PHYREADY_FORCE;
    MCESD_FIELD_PTR ptEnModePtr;
    MCESD_FIELD_PTR ptPhyReadyForcePtr;

    if ((devPtr->ipMajorRev <= 1) && (devPtr->ipMinorRev < 1))
    {
        ptEnModePtr = &ptEnModeR1p0;
        ptPhyReadyForcePtr = &ptPhyReadyForceR1p0;
    }
    else
    {
        ptEnModePtr = &ptEnModeR1p1;
        ptPhyReadyForcePtr = &ptPhyReadyForceR1p1;
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptEn, 0));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, ptPhyReadyForcePtr, 0));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, ptEnModePtr, 2));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptPrbsLoad, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptEn, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptRst, 0));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptRst, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptRst, 0));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, ptPhyReadyForcePtr, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD ptEn = F_C112GX4R1P0_PT_EN;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_ExecuteCDS
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD cliCmd          = F_C112GX4R1P0_CLI_CMD;
    MCESD_FIELD cliStart        = F_C112GX4R1P0_CLI_START;
    MCESD_FIELD cliArgs         = F_C112GX4R1P0_CLI_ARGS;
    MCESD_FIELD cdrMode         = F_C112GX4R1P0_CDR_MODE;
    MCESD_FIELD dfeAdaptLpnum   = F_C112GX4R1P0_DFE_ADAPT_LPNUM;
    MCESD_FIELD dfePolLpnum     = F_C112GX4R1P0_DFE_POL_LPNUM;
    E_C112GX4_DATA_ACQ_RATE acqRate;

    /* Disable DFE continuous mode */
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliCmd, 0x8));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliArgs, 0));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 1));
    MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &cliStart, 0, 10000));

    /* CDS */
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliCmd, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliArgs, 0xC3DFBF));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 1));
    MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &cliStart, 0, 10000));

    /* Enable DFE continuous mode */
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliCmd, 0x8));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliArgs, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cliStart, 1));
    MCESD_ATTEMPT(API_C112GX4_PollField(devPtr, lane, &cliStart, 0, 10000));

    /* Quarter Rate */
    MCESD_ATTEMPT(API_C112GX4_GetDataAcquisitionRate(devPtr, lane, &acqRate));
    if (C112GX4_RATE_QUARTER == acqRate)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &cdrMode, 0x7));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dfeAdaptLpnum, 0xFF));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dfePolLpnum, 0xFF));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C112GX4_PATTERN_STATISTICS *statistics
)
{
    MCESD_FIELD ptLock          = F_C112GX4R1P0_PT_LOCK;
    MCESD_FIELD ptPass          = F_C112GX4R1P0_PT_PASS;
    MCESD_FIELD ptCnt_47_32     = F_C112GX4R1P0_PT_CNT_47_32;
    MCESD_FIELD ptCnt_31_00     = F_C112GX4R1P0_PT_CNT_31_00;
    MCESD_FIELD ptErrCnt_47_32  = F_C112GX4R1P0_PT_ERR_CNT_47_32;
    MCESD_FIELD ptErrCnt_31_00  = F_C112GX4R1P0_PT_ERR_CNT_31_00;
    E_C112GX4_DATABUS_WIDTH busTxWidth;
    E_C112GX4_DATABUS_WIDTH busRxWidth;
    MCESD_U16 width;
    MCESD_U32 data;
    MCESD_U64 cnt, errCnt;

    API_C112GX4_GetDataBusWidth(devPtr, lane, &busTxWidth, &busRxWidth);
    switch (busRxWidth)
    {
    case C112GX4_DATABUS_32BIT:
        width = 32;
        break;
    case C112GX4_DATABUS_40BIT:
        width = 40;
        break;
    case C112GX4_DATABUS_64BIT:
        width = 64;
        break;
    case C112GX4_DATABUS_80BIT:
        width = 80;
        break;
    case C112GX4_DATABUS_128BIT:
        width = 128;
        break;
    case C112GX4_DATABUS_160BIT:
        width = 160;
        break;
    default:
        width = 1;
    }
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptCnt_31_00, &data));
    cnt = data;
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptCnt_47_32, &data));
    cnt += (MCESD_U64)data << 32;
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptErrCnt_31_00, &data));
    errCnt = data;
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptErrCnt_47_32, &data));
    errCnt += (MCESD_U64)data << 32;
    statistics->totalBits = cnt * width;
    statistics->totalErrorBits = errCnt;
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptLock, &data));
    statistics->lock = (data == 0) ? MCESD_FALSE : MCESD_TRUE;
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptPass, &data));
    statistics->pass = (data == 0) ? MCESD_FALSE : MCESD_TRUE;
    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD ptCntRst = F_C112GX4R1P0_PT_CNT_RST;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptCntRst, 0));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 3));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptCntRst, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 3));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptCntRst, 0));

    return MCESD_OK;
}

#ifdef C112GX4_ISOLATION
MCESD_STATUS API_C112GX4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_FIELD sqDetected = F_C112GX4R1P0_RX_SQ_OUT_LPF_RD;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &sqDetected, &data));
    *squelched = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C112GX4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_U16 pinValue;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_SQ_DETECTED_LPF0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_SQ_DETECTED_LPF1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_SQ_DETECTED_LPF2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, C112GX4_PIN_SQ_DETECTED_LPF3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }
    *squelched = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C112GX4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 threshold
)
{
    MCESD_FIELD sqThresh    = F_C112GX4R1P0_SQ_CAL_RESULT_EXT;
    MCESD_FIELD sqIndvR1p0  = F_C112GX4R1P0_SQ_CAL_INDV_EXT;
    MCESD_FIELD sqIndvR1p1  = F_C112GX4R1P1_SQ_CAL_INDV_EXT;
    MCESD_FIELD_PTR sqIndvPtr;

    if ((devPtr->ipMajorRev <= 1) && (devPtr->ipMinorRev < 1))
        sqIndvPtr = &sqIndvR1p0;
    else
        sqIndvPtr = &sqIndvR1p1;

    if (threshold > C112GX4_SQ_THRESH_MAX)
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, sqIndvPtr, 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &sqThresh, threshold + 0x20));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U8 *threshold
)
{
    MCESD_FIELD sqThresh = F_C112GX4R1P0_SQ_CAL_RESULT_RD;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &sqThresh, &data));
    if (data > 0x20)
    {
        *threshold = (MCESD_U8)data - 0x20;
    }
    else
    {
        *threshold = (MCESD_U8)data;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 align90
)
{
    MCESD_FIELD phaseOffset = F_C112GX4R1P0_PH_OS_DAT;

    if (align90 < -400)
    {
        align90 = -400;
    }
    if (align90 > 64)
    {
        align90 = 64;
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &phaseOffset, (MCESD_U32)align90));
    MCESD_ATTEMPT(API_C112GX4_ExecuteCDS(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U32 *align90,
    OUT MCESD_16 *trainingResult
)
{
    MCESD_FIELD phaseOffset    = F_C112GX4R1P0_PH_OS_DAT;
    MCESD_FIELD deCalSet_10_03 = F_C112GX4R1P0_DE_CAL_SET_10_03;
    MCESD_FIELD deCalSet_02_00 = F_C112GX4R1P0_DE_CAL_SET_02_00;
    MCESD_U32 upper, lower, data;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &deCalSet_10_03, &upper));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &deCalSet_02_00, &lower));
    *align90 = (upper << 3) + lower;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &phaseOffset, &data));
    *trainingResult = (MCESD_16)data;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
)
{
    MCESD_FIELD addErrNum   = F_C112GX4R1P0_ADD_ERR_NUM;
    MCESD_FIELD addErrEn    = F_C112GX4R1P0_ADD_ERR_EN;

    if ((errors < 1) || (errors > 8))
    {
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &addErrNum, errors - 1));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &addErrEn, 0));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &addErrEn, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 5));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &addErrEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD selMupf = F_C112GX4R1P0_REG_SELMUPF;
    MCESD_FIELD selMupi = F_C112GX4R1P0_REG_SELMUPI;
    MCESD_FIELD selMufi = F_C112GX4R1P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C112GX4R1P0_RX_SELMUFF;

    switch (param)
    {
    case C112GX4_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &selMufi, paramValue));
        break;
    case C112GX4_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &selMuff, paramValue));
        break;
    case C112GX4_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &selMupi, paramValue));
        break;
    case C112GX4_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &selMupf, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD selMupf = F_C112GX4R1P0_REG_SELMUPF;
    MCESD_FIELD selMupi = F_C112GX4R1P0_REG_SELMUPI;
    MCESD_FIELD selMufi = F_C112GX4R1P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C112GX4R1P0_RX_SELMUFF;

    switch (param)
    {
    case C112GX4_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &selMufi, paramValue));
        break;
    case C112GX4_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &selMuff, paramValue));
        break;
    case C112GX4_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &selMupi, paramValue));
        break;
    case C112GX4_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &selMupf, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
)
{
    MCESD_FIELD tsenAdcMode = F_C112GX4R1P0_TSEN_ADC_MODE;
    MCESD_FIELD tsenAdcData = F_C112GX4R1P0_TSEN_ADC_DATA;
    MCESD_U32 savedMode, adcData;
    MCESD_32 signedValue;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* Unused */, &tsenAdcMode, &savedMode));

    if (0 != savedMode)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* Unused */, &tsenAdcMode, 0));
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* Unused */, &tsenAdcData, &adcData));
    signedValue = (MCESD_32)adcData;

    if (signedValue >= 512)
    {
        signedValue -= 1024;
    }
    *temperature = (signedValue * C112GX4_TSENE_GAIN) + C112GX4_TSENE_OFFSET;

    if (0 != savedMode)
    {
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, 255 /* Unused */, &tsenAdcMode, savedMode));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_POLARITY txPolarity,
    IN E_C112GX4_POLARITY rxPolarity
)
{
    MCESD_FIELD txdInv = F_C112GX4R1P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C112GX4R1P0_RXD_INV;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txdInv, txPolarity));
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxdInv, rxPolarity));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_POLARITY *txPolarity,
    OUT E_C112GX4_POLARITY *rxPolarity
)
{
    MCESD_FIELD txdInv = F_C112GX4R1P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C112GX4R1P0_RXD_INV;
    MCESD_U32 txdInvData, rxdInvData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txdInv, &txdInvData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &rxdInv, &rxdInvData));
    *txPolarity = (0 == txdInvData) ? C112GX4_POLARITY_NORMAL : C112GX4_POLARITY_INVERTED;
    *rxPolarity = (0 == rxdInvData) ? C112GX4_POLARITY_NORMAL : C112GX4_POLARITY_INVERTED;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_DATAPATH path
)
{
    MCESD_FIELD dtlClampingSel  = F_C112GX4R1P0_DTL_CLAMPING_SEL;
    MCESD_FIELD rx2txNoStop     = F_C112GX4R1P0_RX2TX_NO_STOP;
    MCESD_FIELD rx2txRdStart    = F_C112GX4R1P0_RX2TX_RD_START;
    MCESD_FIELD rxFoffsetDis    = F_C112GX4R1P0_RX_FOFFSET_DIS;
    MCESD_FIELD dtxFoffsetSel   = F_C112GX4R1P0_DTX_FOFFSET_SEL;
    MCESD_FIELD digRx2txLpbkEn  = F_C112GX4R1P0_DIG_RX2TX_LPBK_EN;
    MCESD_FIELD anaTx2rxLpbkEn  = F_C112GX4R1P0_ANA_TX2RX_LPBK_EN;
    MCESD_FIELD digTx2rxLpbkEn  = F_C112GX4R1P0_DIG_TX2RX_LPBK_EN;
    MCESD_FIELD puLb            = F_C112GX4R1P0_PU_LB;
    MCESD_FIELD ptEn            = F_C112GX4R1P0_PT_EN;
    MCESD_FIELD vrefShift       = F_C112GX4R1P0_VREF_SHIFT;

    switch (path)
    {
    case C112GX4_PATH_LOCAL_ANALOG_LB:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dtlClampingSel, 6));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rx2txNoStop, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rx2txRdStart, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxFoffsetDis, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dtxFoffsetSel, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &digRx2txLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &anaTx2rxLpbkEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &digTx2rxLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &puLb, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &vrefShift, 2));

        /* Clear all Tx FIR Taps and Set Main to 0x3F */
        if ((devPtr->ipMajorRev <= 1) && (devPtr->ipMinorRev < 2))
        {
            /* RX.X < R1.2 */
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_PRE2_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_PRE_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_POST_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_FIR_C_CTRL1, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_FIR_C_CTRL2, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_FIR_C_CTRL3, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_MAIN_CTRL, 0x3F));
        }
        else
        {
            /* RX.X >= R1.2 */
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_PRE3_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_PRE2_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_PRE_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_POST_CTRL, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_NA, 0));
            MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, C112GX4_TXEQ_EM_MAIN_CTRL, 0x3F));
        }
        break;
    case C112GX4_PATH_EXTERNAL:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dtlClampingSel, 6));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rx2txNoStop, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rx2txRdStart, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxFoffsetDis, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dtxFoffsetSel, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &digRx2txLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &anaTx2rxLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &digTx2rxLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &puLb, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &vrefShift, 0));
        break;
    case C112GX4_PATH_FAR_END_LB:
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dtlClampingSel, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rx2txNoStop, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rx2txRdStart, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &rxFoffsetDis, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &dtxFoffsetSel, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &digRx2txLpbkEn, 1));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &anaTx2rxLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &digTx2rxLpbkEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &puLb, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &ptEn, 0));
        MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &vrefShift, 0));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C112GX4_DATAPATH *path
)
{
    MCESD_FIELD digRx2txLpbkEn  = F_C112GX4R1P0_DIG_RX2TX_LPBK_EN;
    MCESD_FIELD anaTx2rxLpbkEn  = F_C112GX4R1P0_ANA_TX2RX_LPBK_EN;
    MCESD_FIELD digTx2rxLpbkEn  = F_C112GX4R1P0_DIG_TX2RX_LPBK_EN;
    MCESD_FIELD ptEn            = F_C112GX4R1P0_PT_EN;
    MCESD_U32 digRx2txLpbkEnData, anaTx2rxLpbkEnData, digTx2rxLpbkEnData, ptEnData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &digRx2txLpbkEn, &digRx2txLpbkEnData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &anaTx2rxLpbkEn, &anaTx2rxLpbkEnData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &digTx2rxLpbkEn, &digTx2rxLpbkEnData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &ptEn, &ptEnData));

    if ((0 == digRx2txLpbkEnData) && (0 == anaTx2rxLpbkEnData) && (0 == digTx2rxLpbkEnData) && (0 == ptEnData))
    {
        *path = C112GX4_PATH_EXTERNAL;
    }
    else if ((1 == digRx2txLpbkEnData) && (0 == anaTx2rxLpbkEnData) && (0 == digTx2rxLpbkEnData) && (0 == ptEnData))
    {
        *path = C112GX4_PATH_FAR_END_LB;
    }
    else if ((0 == digRx2txLpbkEnData) && (1 == anaTx2rxLpbkEnData) && (0 == digTx2rxLpbkEnData) && (0 == ptEnData))
    {
        *path = C112GX4_PATH_LOCAL_ANALOG_LB;
    }
    else
    {
        *path = C112GX4_PATH_UNKNOWN;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetDfeF0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 value,
    OUT MCESD_U16 *value100uV
)
{
    MCESD_FIELD dfeF0Res        = F_C112GX4R1P0_DFE_RES_F0;
    MCESD_FIELD dfeF0ResDouble  = F_C112GX4R1P0_DFE_F0_RES_DOUBLE;
    MCESD_U16 data;
    MCESD_U32 dfeF0ResData, dfeF0ResDoubleData;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &dfeF0Res, &dfeF0ResData));
    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &dfeF0ResDouble, &dfeF0ResDoubleData));

    switch (dfeF0ResData)
    {
    case 0:
        {
            static const MCESD_U16 dfeF0Res0[64] =
            {
                00, 19, 24, 33, 36, 44, 50, 56,
                59, 64, 73, 78, 84, 90, 98, 101,
                107, 110, 113, 118, 121, 127, 130, 136,
                139, 144, 147, 153, 156, 161, 167, 170,
                176, 181, 184, 190, 196, 201, 207, 210,
                216, 221, 227, 233, 238, 244, 250, 256,
                259, 264, 270, 276, 281, 287, 293, 299,
                307, 313, 318, 324, 330, 336, 341, 347,
            };
            data = dfeF0Res0[value];
            break;
        }
    case 1:
        {
            static const MCESD_U16 dfeF0Res1[64] =
            {
                00, 21, 33, 44, 56, 70, 87, 99,
                113, 124, 139, 156, 170, 187, 204, 213,
                221, 230, 239, 247, 258, 267, 276, 285,
                296, 304, 316, 324, 336, 344, 353, 364,
                373, 384, 394, 404, 416, 424, 436, 444,
                456, 464, 476, 485, 496, 507, 516, 527,
                536, 547, 556, 567, 576, 587, 596, 607,
                616, 627, 636, 647, 656, 667, 676, 684,
            };
            data = dfeF0Res1[value];
            break;
        }
    case 2:
        {
            static const MCESD_U16 dfeF0Res2[64] =
            {
                00, 21, 39, 61, 84, 110, 138, 159,
                181, 204, 230, 256, 281, 307, 336, 350,
                364, 378, 390, 404, 419, 433, 447, 461,
                476, 490, 504, 519, 533, 547, 561, 576,
                590, 604, 618, 633, 644, 659, 673, 687,
                699, 713, 727, 739, 753, 767, 779, 793,
                804, 818, 830, 841, 856, 867, 879, 890,
                902, 916, 927, 938, 950, 961, 973, 982,
            };
            data = dfeF0Res2[value];
            break;
        }
    case 3:
        {
            static const MCESD_U16 dfeF0Res3[64] =
            {
                00, 21, 44, 73, 99, 130, 167, 193,
                221, 250, 279, 310, 341, 373, 407, 421,
                438, 456, 470, 487, 504, 519, 536, 553,
                567, 584, 601, 616, 633, 647, 664, 679,
                696, 710, 724, 741, 756, 770, 784, 799,
                813, 827, 842, 856, 870, 884, 898, 913,
                924, 938, 950, 964, 976, 990, 1001, 1013,
                1027, 1038, 1050, 1061, 1073, 1084, 1096, 1104,
            };
            data = dfeF0Res3[value];
            break;
        }
    default:
        return MCESD_FAIL; /* Invalid dfeF0Res */
    }

    *value100uV = (1 == dfeF0ResDoubleData) ? 2 * data : data;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMConvertWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 width,
    IN MCESD_U16 heightUpper,
    IN MCESD_U16 heightLower,
    OUT MCESD_U16 *widthmUI,
    OUT MCESD_U16 *height100uV
)
{
    MCESD_U16 upper100uV, lower100uV, phaseStepCount, voltageStepCount;

    *widthmUI = 0;
    *height100uV = 0;

    if (0 == width)
        return MCESD_FAIL; /* Division by 0 Error */

    /* Convert width */
    MCESD_ATTEMPT(API_C112GX4_EOM1UIStepCount(devPtr, lane, &phaseStepCount, &voltageStepCount));
    *widthmUI = (MCESD_U32)width * 1000 / phaseStepCount;

    /* Convert height */
    MCESD_ATTEMPT(API_C112GX4_GetDfeF0(devPtr, lane, heightUpper, &upper100uV));
    MCESD_ATTEMPT(API_C112GX4_GetDfeF0(devPtr, lane, heightLower, &lower100uV));
    *height100uV = upper100uV + lower100uV;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_EYE_TMB eyeTMB,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C112GX4_EYE_RAW_PTR eyeRawDataPtr
)
{
    MCESD_16 maxVoltageStep, leftPhase, rightPhase, phaseIdx, voltIdx, voltage, startPt, tmp;
    MCESD_32 phase;
    MCESD_32 upperVoltage = -C112GX4_EYE_MAX_VOLT_STEPS;
    MCESD_32 lowerVoltage = C112GX4_EYE_MAX_VOLT_STEPS - 1;
    S_C112GX4_EOM_DATA measurement;

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C112GX4_EOMGetEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if (0 == voltageSteps)
        voltageSteps = C112GX4_EYE_DEFAULT_VOLT_STEPS;

    if (0 == phaseLevels)
        phaseLevels = C112GX4_EYE_DEFAULT_PHASE_LEVEL;

    maxVoltageStep = voltageSteps;
    leftPhase = phaseLevels;
    rightPhase = -phaseLevels;

    /* Initialize eyeRawData */
    phaseIdx = 0;
    for (phase = leftPhase; phase >= rightPhase; phase--)
    {
        voltIdx = 0;
        for (voltage = maxVoltageStep; voltage >= -maxVoltageStep; voltage--)
            eyeRawDataPtr->eyeRawData[phaseIdx][voltIdx++] = -1;
        phaseIdx++;
    }

    phase = 0;
    voltage = 0;

    MCESD_ATTEMPT(API_C112GX4_EOMInit(devPtr, lane));

    MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, 0, 0, &measurement));

    if (0 != measurement.upperBitErrorCount && 0 != measurement.lowerBitErrorCount)
    {
        MCESD_ATTEMPT(API_C112GX4_EOMFinalize(devPtr, lane));
        MCESD_DBG_ERROR("API_C112GX4_EOMGetEyeData: BER > 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
    {
        MCESD_ATTEMPT(API_C112GX4_EOMFinalize(devPtr, lane));
        MCESD_DBG_ERROR("API_C112GX4_EOMGetEyeData: Total Bit Count == 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    /* Sweep Voltage at Phase 0 */
    phaseIdx = phaseLevels;
    for (voltage = 0; voltage < voltageSteps; voltage++)
    {
        MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, 0, (MCESD_U8)voltage, &measurement));

        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

        if ((upperVoltage == -C112GX4_EYE_MAX_VOLT_STEPS) && ((0 == measurement.upperBitCount) || (0 < measurement.upperBitErrorCount)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == (C112GX4_EYE_MAX_VOLT_STEPS - 1)) && ((0 == measurement.lowerBitCount) || (0 < measurement.lowerBitErrorCount)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -C112GX4_EYE_MAX_VOLT_STEPS) && (lowerVoltage != (C112GX4_EYE_MAX_VOLT_STEPS - 1)))
            break;
    }

    /* Left Sweep from Phase 1 with Voltage Sweep */
    phaseIdx -= 1;
    startPt = (upperVoltage > -lowerVoltage) ? upperVoltage : lowerVoltage;
    for (phase = 1; phase < phaseLevels; phase++)
    {
        voltage = ((startPt + C112GX4_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C112GX4_EYE_VOLT_OFFSET);
        startPt = 0;
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, phase, (MCESD_U8)voltage, &measurement));

            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

            if ((0 == startPt) && ((0 == measurement.lowerBitErrorCount) || (0 == measurement.upperBitErrorCount)))
            {
                startPt = voltage;
                break;
            }

            if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount || ((0 == measurement.lowerBitErrorCount) && (0 == measurement.upperBitErrorCount)))
                break;

            voltage--;
        }

        /* Patch zeros in between */
        if (voltage > 0)
        {
            for (tmp = voltage - 1; tmp >= 0; tmp--)
            {
                eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - tmp] = 0;
                eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + tmp] = 0;
            }
        }
        else
        {
            break;
        }
        phaseIdx--;
    }

    /* Right Sweep from Phase -1 with Voltage Sweep */
    phaseIdx = phaseLevels + 1;
    startPt = (upperVoltage > -lowerVoltage) ? upperVoltage : lowerVoltage;
    for (phase = -1; phase > -phaseLevels; phase--)
    {
        voltage = ((startPt + C112GX4_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C112GX4_EYE_VOLT_OFFSET);
        startPt = 0;
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C112GX4_EOMMeasPoint(devPtr, lane, eyeTMB, phase, (MCESD_U8)voltage, &measurement));

            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

            if ((0 == startPt) && ((0 == measurement.lowerBitErrorCount) || (0 == measurement.upperBitErrorCount)))
            {
                startPt = voltage;
                break;
            }

            if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount || ((0 == measurement.lowerBitErrorCount) && (0 == measurement.upperBitErrorCount)))
                break;

            voltage--;
        }

        /* Patch zeros in between */
        if (voltage > 0)
        {
            for (tmp = voltage - 1; tmp >= 0; tmp--)
            {
                eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - tmp] = 0;
                eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + tmp] = 0;
            }
        }
        else
        {
            break;
        }
        phaseIdx++;
    }

    /* Fill untouched outer area with maxBit (error) */
    for (voltIdx = maxVoltageStep * 2; voltIdx >= 0; voltIdx--)
    {
        for (phaseIdx = 0; phaseIdx < leftPhase - rightPhase + 1; phaseIdx++)
        {
            if (eyeRawDataPtr->eyeRawData[phaseIdx][voltIdx] == -1)
                eyeRawDataPtr->eyeRawData[phaseIdx][voltIdx] = 0xFFFFFF;
        }
    }

    MCESD_ATTEMPT(API_C112GX4_EOMFinalize(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_EOMPlotEyeData
(
    IN S_C112GX4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
)
{
    MCESD_U16 phaseIndex, voltageIndex, phaseInterval, timeUnit;
    char line[C112GX4_EYE_MAX_PHASE_LEVEL * 2 + 2];

    memset(&line, 'X', C112GX4_EYE_MAX_PHASE_LEVEL * 2 + 2);

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C112GX4_EOMPlotEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if ((0 == voltageSteps) || (voltageSteps > C112GX4_EYE_MAX_VOLT_STEPS))
        voltageSteps = C112GX4_EYE_DEFAULT_VOLT_STEPS;

    if ((0 == phaseLevels) || (voltageSteps > C112GX4_EYE_MAX_PHASE_LEVEL))
        phaseLevels = C112GX4_EYE_DEFAULT_PHASE_LEVEL;

    phaseInterval = ((phaseLevels + 1) * 2);
    timeUnit = (voltageSteps * 2) + 1;
    line[phaseInterval + 1] = '\0';

    for (voltageIndex = 0; voltageIndex < timeUnit; voltageIndex++)
    {
        for (phaseIndex = 0; phaseIndex < phaseInterval; phaseIndex++)
        {
            if ((phaseIndex + 1) >= phaseInterval)
            {
                MCESD_DBG_INFO("\n");
            }
            else if (voltageIndex == voltageSteps + 1)
            {
                MCESD_DBG_INFO("-");
            }
            else if (phaseIndex == phaseLevels)
            {
                MCESD_DBG_INFO("|");
            }
            else if (0 == eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex])
            {
                MCESD_DBG_INFO(" ");
            }
            else if (C112GX4_EYE_LOW_ERROR_THRESH > eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex])
            {
                MCESD_DBG_INFO(".");
            }
            else
            {
                MCESD_DBG_INFO("X");
            }
        }
    }

    return MCESD_OK;
}

static MCESD_STATUS INT_C112GX4_SetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 position,
    IN E_C112GX4_POLARITY polarity
)
{
    MCESD_U32 data;

    MCESD_FIELD txFir  = F_C112GX4R1P0_TX_FIR_TAP_POL;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txFir, &data));
    data = ((data & ~(1 << position)) | (polarity << position));    /* Modify bit at position with polarity */
    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFir, data));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN E_C112GX4_POLARITY polarity
)
{
    MCESD_U32 txPAM2EnData;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD txFir  = F_C112GX4R1P0_TX_FIR_TAP_POL_F;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFir, 1));

    if ((devPtr->ipMajorRev >= 1) && (devPtr->ipMinorRev > 1))
    {
        /* RX.X > R1.1 */
        switch (param)
        {
        case C112GX4_TXEQ_EM_PRE3_CTRL:
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 0, polarity);
            break;
        case C112GX4_TXEQ_EM_PRE2_CTRL:
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 1, polarity);
            break;
        case C112GX4_TXEQ_EM_PRE_CTRL:
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 2, polarity);
            break;
        case C112GX4_TXEQ_EM_MAIN_CTRL:
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 3, polarity);
            break;
        case C112GX4_TXEQ_EM_POST_CTRL:
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 4, polarity);
            break;
        case C112GX4_TXEQ_EM_NA:
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 5, polarity);
            break;
        default:
            return MCESD_FAIL; /* Unsupported parameter */
        }

        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane,&txPAM2En, &txPAM2EnData));

    switch (param)
    {
    case C112GX4_TXEQ_EM_PRE2_CTRL:
        if (1 == txPAM2EnData)
            return MCESD_FAIL;  /* PAM2 does not have PRE2 */
        INT_C112GX4_SetTxEqPolarity(devPtr, lane, 0, polarity);
        break;
    case C112GX4_TXEQ_EM_PRE_CTRL:
        if (1 == txPAM2EnData)
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 0, polarity);
        else
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 1, polarity);
        break;
    case C112GX4_TXEQ_EM_MAIN_CTRL:
        if (1 == txPAM2EnData)
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 1, polarity);
        else
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 2, polarity);
        break;
    case C112GX4_TXEQ_EM_POST_CTRL:
        if (1 == txPAM2EnData)
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 2, polarity);
        else
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 3, polarity);
        break;
    case C112GX4_TXEQ_FIR_C_CTRL1:
        if (1 == txPAM2EnData)
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 3, polarity);
        else
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 4, polarity);
        break;
    case C112GX4_TXEQ_FIR_C_CTRL2:
        if (1 == txPAM2EnData)
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 4, polarity);
        else
            INT_C112GX4_SetTxEqPolarity(devPtr, lane, 5, polarity);
        break;
    case C112GX4_TXEQ_FIR_C_CTRL3:
        if (0 == txPAM2EnData)
            return MCESD_FAIL;  /* PAM4 does not have CTRL3 */
        INT_C112GX4_SetTxEqPolarity(devPtr, lane, 5, polarity);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT E_C112GX4_POLARITY* polarity
)
{
    MCESD_U32 data, txPAM2EnData;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD anTxFir  = F_C112GX4R1P0_TO_ANA_TX_FIR_POL;

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &anTxFir, &data));

    if ((devPtr->ipMajorRev >= 1) && (devPtr->ipMinorRev > 1))
    {
        /* RX.X > R1.1 */
        switch (param)
        {
        case C112GX4_TXEQ_EM_PRE3_CTRL:
            *polarity = data & 0x1;
            break;
        case C112GX4_TXEQ_EM_PRE2_CTRL:
            *polarity = (data >> 1) & 0x1;
            break;
        case C112GX4_TXEQ_EM_PRE_CTRL:
            *polarity = (data >> 2) & 0x1;
            break;
        case C112GX4_TXEQ_EM_MAIN_CTRL:
            *polarity = (data >> 3) & 0x1;
            break;
        case C112GX4_TXEQ_EM_POST_CTRL:
            *polarity = (data >> 4) & 0x1;
            break;
        case C112GX4_TXEQ_EM_NA:
            *polarity = (data >> 5) & 0x1;
            break;
        default:
            return MCESD_FAIL; /* Unsupported parameter */
        }

        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txPAM2En, &txPAM2EnData));

    switch (param)
    {
    case C112GX4_TXEQ_EM_PRE2_CTRL:
        if (1 == txPAM2EnData)
            return MCESD_FAIL;  /* PAM2 does not have PRE2 */
        *polarity = data & 0x1;
        break;
    case C112GX4_TXEQ_EM_PRE_CTRL:
        if (1 == txPAM2EnData)
            *polarity = data & 0x1;
        else
            *polarity = (data >> 1) & 0x1;
        break;
    case C112GX4_TXEQ_EM_MAIN_CTRL:
        if (1 == txPAM2EnData)
            *polarity = (data >> 1) & 0x1;
        else
            *polarity = (data >> 2) & 0x1;
        break;
    case C112GX4_TXEQ_EM_POST_CTRL:
        if (1 == txPAM2EnData)
            *polarity = (data >> 2) & 0x1;
        else
            *polarity = (data >> 3) & 0x1;
        break;
    case C112GX4_TXEQ_FIR_C_CTRL1:
        if (1 == txPAM2EnData)
            *polarity = (data >> 3) & 0x1;
        else
            *polarity = (data >> 4) & 0x1;
        break;
    case C112GX4_TXEQ_FIR_C_CTRL2:
        if (1 == txPAM2EnData)
            *polarity = (data >> 4) & 0x1;
        else
            *polarity = (data >> 5) & 0x1;
        break;
    case C112GX4_TXEQ_FIR_C_CTRL3:
        if (0 == txPAM2EnData)
            return MCESD_FAIL;  /* PAM4 does not have CTRL3 */
        *polarity = (data >> 5) & 0x1;
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_DisplayTrainingLog
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN S_C112GX4_TRX_TRAINING_LOGENTRY logArrayDataPtr[],
    IN MCESD_U32 logArraySizeEntries,
    OUT MCESD_U32* validEntries
)
{
    S_C112GX4_TRX_TRAINING_LOGENTRY entry;
    MCESD_U32 data, baseAddr, addrOffset;
    MCESD_U16 logCount, iterLog, hiBit, loBit;
    MCESD_U8 index = 0;
    MCESD_BOOL isPAM2;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD pmdLogCounterLane0 = F_C112GX4R1P0_PMD_LOG_COUNTER_LANE0;
    MCESD_FIELD pmdLogCounterLane1 = F_C112GX4R1P0_PMD_LOG_COUNTER_LANE1;
    MCESD_FIELD pmdLogCounterLane2 = F_C112GX4R1P0_PMD_LOG_COUNTER_LANE2;
    MCESD_FIELD pmdLogCounterLane3 = F_C112GX4R1P0_PMD_LOG_COUNTER_LANE3;
    MCESD_FIELD temp;

    MCESD_DBG_INFO("--------------------------------------------------------------------------------\n");
    MCESD_DBG_INFO("API_C112GX4_DisplayTrainingLog(lane=%d)\n", lane);

    switch ( lane )
    {
    case 0:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &pmdLogCounterLane0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &pmdLogCounterLane1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &pmdLogCounterLane2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &pmdLogCounterLane3, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    logCount = (MCESD_U16)data;
    if (0 == logCount)
    {
        MCESD_DBG_INFO("No entries in log\n");
        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, &txPAM2En, &data));
    isPAM2 = (MCESD_BOOL)data;
    baseAddr = 0xE740 + lane * 0x30;

    for (iterLog = logCount; iterLog > ((logCount - 23) > 0 ? logCount - 23 : 0); iterLog--)
    {

        /* Calculates entry's address offset from base address */
        addrOffset = (((iterLog - 1) % 23) + 1) / 2;

        /* Calculates entry's location in register [31:16|15:00] */
        hiBit = (iterLog / 23 + ((iterLog - 1) / 23) + (iterLog % 23)) % 2 ? 31 : 15;
        loBit = (iterLog / 23 + ((iterLog - 1) / 23) + (iterLog % 23)) % 2 ? 16 : 0;

        temp.reg = baseAddr + 0x4 * addrOffset;
        temp.hiBit = hiBit;
        temp.loBit = loBit;
        temp.totalBits = (hiBit-loBit) + 1;
        temp.mask = (MCESD_U32) ((((MCESD_U64) 1 << ((hiBit-loBit) + 1)) - 1) << loBit);
        temp.retainMask = (MCESD_U32) ~((((MCESD_U64) 1 << ((hiBit-loBit) + 1)) - 1) << loBit);

        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, 255 /* ignored */, &temp, &data));

        if (isPAM2)
        {
            MCESD_DBG_INFO("Log Entry: %d (PAM2)\n", iterLog);
            if (1 == (data >> 15))
            {
                MCESD_ATTEMPT(INT_C112GX4_DisplayEntryPAM2_1(data, &entry));
            }
            else
            {
                MCESD_ATTEMPT(INT_C112GX4_DisplayEntryPAM2_0(data, &entry));
            }
        }
        else
        {
            MCESD_DBG_INFO("Log Entry: %d (PAM4)\n", iterLog);
            MCESD_ATTEMPT(INT_C112GX4_DisplayEntryPAM4(data, &entry));
        }

        if ((logArrayDataPtr != NULL) && (index < logArraySizeEntries))
        {
            logArrayDataPtr[index++] = entry;
        }
    }

    *validEntries = index;

    MCESD_DBG_INFO("--------------------------------------------------------------------------------\n");

    return MCESD_OK;
}

static MCESD_STATUS INT_C112GX4_DisplayEntryPAM2_0
(
    IN MCESD_U16 data,
    OUT S_C112GX4_TRX_TRAINING_LOGENTRY* entry
)
{
    MCESD_U16 rawValue;
    char msg[80];
    char* leftString;
    char* rightString;

    entry->isPAM2 = MCESD_TRUE;
    entry->pam2.errorCode = 0;
    entry->pam4.txPresetIndex = 0;
    entry->pam4.localCtrlPat = 0;
    entry->pam4.localCtrlSel = 0;
    entry->pam4.localCtrlG = 0;
    entry->pam4.remoteStatusAck = 0;
    entry->pam4.remoteStatusG = 0;

    /* TX_PRESET_INDEX */
    rawValue = data >> 13 & 0x3;
    rightString = (1 == rawValue) ? C112GX4_STRING_PRESET1 : (2 == rawValue ? C112GX4_STRING_PRESET2 : C112GX4_STRING_CHAR_DASH);
    if (rawValue > 2)
        rawValue = 0;
    entry->pam2.txPresetIndex = (E_C112GX4_TRX_LOG_TX_PRESET)rawValue;

    /* PATTERN_TYPE */
    rawValue = data >> 15;
    entry->pam2.patternType = (E_C112GX4_TRX_LOG_PAM2_PAT)rawValue;

    sprintf(msg, "    PATTERN_TYPE        0x%-18XTX_PRESET_INDEX     %s\n", rawValue, rightString);
    MCESD_DBG_INFO(msg);

    /* REMOTE_STATUS_G1 */
    rawValue = data >> 4 & 0x3;
    leftString = (rawValue > 1) ? (2 == rawValue ? C112GX4_STRING_MIN : C112GX4_STRING_MAX) : (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_UPDATED);
    entry->pam2.remoteStatusG1 = (E_C112GX4_TRX_LOG_PAM2_REMOTE)rawValue;

    /* LOCAL_CTRL_G1 */
    rawValue = data >> 11 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam2.localCtrlG1 = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    sprintf(msg, "    REMOTE_STATUS_G1    %-20sLOCAL_CTRL_G1       %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    /* REMOTE_STATUS_G0 */
    rawValue = data >> 2 & 0x3;
    leftString = (rawValue > 1) ? (2 == rawValue ? C112GX4_STRING_MIN : C112GX4_STRING_MAX) : (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_UPDATED);
    entry->pam2.remoteStatusG0 = (E_C112GX4_TRX_LOG_PAM2_REMOTE)rawValue;

    /* LOCAL_CTRL_G0 */
    rawValue = data >> 9 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam2.localCtrlG0 = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    sprintf(msg, "    REMOTE_STATUS_G0    %-20sLOCAL_CTRL_G0       %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    /* REMOTE_STATUS_GN1 */
    rawValue = data & 0x3;
    leftString = (rawValue > 1) ? (2 == rawValue ? C112GX4_STRING_MIN : C112GX4_STRING_MAX) : (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_UPDATED);
    entry->pam2.remoteStatusGN1 = (E_C112GX4_TRX_LOG_PAM2_REMOTE)rawValue;

    /* LOCAL_CTRL_GN1 */
    rawValue = data >> 7 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam2.localCtrlGN1 = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    sprintf(msg, "    REMOTE_STATUS_GN1   %-20sLOCAL_CTRL_GN1      %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    return MCESD_OK;
}

static MCESD_STATUS INT_C112GX4_DisplayEntryPAM2_1
(
    IN MCESD_U16 data,
    OUT S_C112GX4_TRX_TRAINING_LOGENTRY* entry
)
{
    MCESD_U16 rawValue;
    char msg[80];
    char* leftString;
    char* rightString;

    entry->isPAM2 = MCESD_TRUE;
    entry->pam2.remoteStatusG1 = 0;
    entry->pam2.remoteStatusG0 = 0;
    entry->pam2.remoteStatusGN1 = 0;
    entry->pam4.txPresetIndex = 0;
    entry->pam4.localCtrlPat = 0;
    entry->pam4.localCtrlSel = 0;
    entry->pam4.localCtrlG = 0;
    entry->pam4.remoteStatusAck = 0;
    entry->pam4.remoteStatusG = 0;

    /* LOCAL_CTRL_G1 */
    rawValue = data >> 11 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam2.localCtrlG1 = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    /* PATTERN_TYPE */
    rawValue = data >> 15;
    entry->pam2.patternType = (E_C112GX4_TRX_LOG_PAM2_PAT)rawValue;

    sprintf(msg, "    PATTERN_TYPE        0x%-18XLOCAL_CTRL_G1       %s\n", rawValue, rightString);
    MCESD_DBG_INFO(msg);

    /* TX_PRESET_INDEX */
    rawValue = data >> 13 & 0x3;
    leftString = (1 == rawValue) ? C112GX4_STRING_PRESET1 : (2 == rawValue ? C112GX4_STRING_PRESET2 : C112GX4_STRING_CHAR_DASH);
    if (rawValue > 2)
        rawValue = 0;
    entry->pam2.txPresetIndex = (E_C112GX4_TRX_LOG_TX_PRESET)rawValue;

    /* LOCAL_CTRL_G0 */
    rawValue = data >> 9 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam2.localCtrlG0 = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    sprintf(msg, "    TX_PRESET_INDEX     %-20sLOCAL_CTRL_G0       %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    /* LOCAL_CTRL_GN1 */
    rawValue = data >> 7 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam2.localCtrlGN1 = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    /* ERROR_CODE */
    rawValue = data & 0x7F;
    entry->pam2.errorCode = (MCESD_U8)rawValue;

    sprintf(msg, "    ERROR_CODE          0x%-18XLOCAL_CTRL_GN1      %s\n", rawValue, rightString);
    MCESD_DBG_INFO(msg);

    return MCESD_OK;
}

static MCESD_STATUS INT_C112GX4_DisplayEntryPAM4
(
    IN MCESD_U16 data,
    OUT S_C112GX4_TRX_TRAINING_LOGENTRY* entry
)
{
    MCESD_U16 rawValue;
    char msg[80];
    char* leftString;
    char* rightString;

    entry->isPAM2 = MCESD_FALSE;
    entry->pam2.errorCode = 0;
    entry->pam2.localCtrlG0 = 0;
    entry->pam2.localCtrlG1 = 0;
    entry->pam2.localCtrlGN1 = 0;
    entry->pam2.patternType = 0;
    entry->pam2.remoteStatusG0 = 0;
    entry->pam2.remoteStatusG1 = 0;
    entry->pam2.remoteStatusGN1 = 0;
    entry->pam2.txPresetIndex = 0;

    /* TX_PRESET_INDEX */
    rawValue = data >> 13;
    switch (rawValue)
    {
    case (1):
        leftString = C112GX4_STRING_PRESET1;
        entry->pam4.txPresetIndex = C112GX4_TRX_LOG_TX_PRESET1;
        break;
    case (2):
        leftString = C112GX4_STRING_PRESET2;
        entry->pam4.txPresetIndex = C112GX4_TRX_LOG_TX_PRESET2;
        break;
    case (3):
        leftString = C112GX4_STRING_PRESET3;
        entry->pam4.txPresetIndex = C112GX4_TRX_LOG_TX_PRESET3;
        break;
    default:
        leftString = C112GX4_STRING_CHAR_DASH;
        entry->pam4.txPresetIndex = C112GX4_TRX_LOG_TX_PRESET_NA;
    }

    /* LOCAL_CTRL_PAT */
    rawValue = data >> 11 & 0x3;
    switch (rawValue)
    {
    case (2):
        rightString = C112GX4_STRING_GRAY_CODE;
        entry->pam4.localCtrlPat = C112GX4_TRX_LOG_PAT_GRAY_CODE;
        break;
    case (3):
        rightString = C112GX4_STRING_PRE_CODE;
        entry->pam4.localCtrlPat = C112GX4_TRX_LOG_PAT_PRE_CODE;
        break;
    default:
        rightString = C112GX4_STRING_CHAR_DASH;
        entry->pam4.localCtrlPat = C112GX4_TRX_LOG_PAT_NA;
    }

    sprintf(msg, "    TX_PRESET_INDEX     %-20sLOCAL_CTRL_PAT      %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    /* REMOTE_STATUS_ACK */
    rawValue = data >> 3 & 0x1;
    leftString = (0 == rawValue) ? C112GX4_STRING_HOLD : C112GX4_STRING_UPDATED;
    entry->pam4.remoteStatusAck = (E_C112GX4_TRX_LOG_REMOTE_ACK)rawValue;

    /* LOCAL_CTRL_SEL */
    rawValue = data >> 8 & 0x7;
    switch (rawValue)
    {
    case (0):
        rightString = C112GX4_STRING_MAIN;
        entry->pam4.localCtrlSel = C112GX4_TRX_LOG_SEL_MAIN;
        break;
    case (1):
        rightString = C112GX4_STRING_POST;
        entry->pam4.localCtrlSel = C112GX4_TRX_LOG_SEL_POST;
        break;
    case (7):
        rightString = C112GX4_STRING_PRE1;
        entry->pam4.localCtrlSel = C112GX4_TRX_LOG_SEL_PRE1;
        break;
    case (6):
        rightString = C112GX4_STRING_PRE2;
        entry->pam4.localCtrlSel = C112GX4_TRX_LOG_SEL_PRE2;
        break;
    default:
        rightString = C112GX4_STRING_CHAR_DASH;
        entry->pam4.localCtrlSel = C112GX4_TRX_LOG_SEL_NA;
    }

    sprintf(msg, "    REMOTE_STATUS_ACK   %-20sLOCAL_CTRL_SEL      %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    /* REMOTE_STATUS_G */
    rawValue = data & 0x7;
    switch (rawValue)
    {
    case (2):
    case (4):
    case (6):
        leftString = C112GX4_STRING_HIT_LIMIT;
        entry->pam4.remoteStatusG = C112GX4_TRX_LOG_PAM4_HIT_LIMIT;
        break;
    case (0):
        leftString = C112GX4_STRING_HOLD;
        entry->pam4.remoteStatusG = C112GX4_TRX_LOG_PAM4_HOLD;
        break;
    case (1):
        leftString = C112GX4_STRING_UPDATED;
        entry->pam4.remoteStatusG = C112GX4_TRX_LOG_PAM4_UPDATED;
        break;
    case (3):
        leftString = C112GX4_STRING_NOT_SUPPORTED;
        entry->pam4.remoteStatusG = C112GX4_TRX_LOG_PAM4_NOTSUPPORT;
        break;
    default:
        leftString = C112GX4_STRING_CHAR_DASH;
        entry->pam4.remoteStatusG = C112GX4_TRX_LOG_PAM4_NA;
    }

    /* LOCAL_CTRL_G */
    rawValue = data >> 6 & 0x3;
    rightString = (rawValue < 2) ? (0 == rawValue ? C112GX4_STRING_HOLD : C112GX4_STRING_INC) : (2 == rawValue ? C112GX4_STRING_DEC : C112GX4_STRING_CHAR_DASH);
    entry->pam4.localCtrlG = (E_C112GX4_TRX_LOG_LOCAL_REQ)rawValue;

    sprintf(msg, "    REMOTE_STATUS_G     %-20sLOCAL_CTRL_G        %s\n", leftString, rightString);
    MCESD_DBG_INFO(msg);

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetDRO
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C112GX4_DRO_DATA *freq
)
{
    MCESD_U8 i;
    MCESD_U32 data;
    E_C112GX4_REFFREQ refFreq;
    E_C112GX4_REFCLK_SEL clkSel;

    if (freq == NULL)
        return MCESD_FAIL;

    /* The counters are based on reference clock. Count period is 1 us or 4 us */
    /* If reference clock is 20 MHz, set 20 * 4 - 1 = 79 (decimal) to count for 4 us, set 19 for 1 us */
    /* If reference clock is 62.5 MHz, set 63 * 4 - 1 = 251 (decimal) to count for 4 us, set 63 for 1 us */
    MCESD_ATTEMPT(API_C112GX4_GetRefFreq(devPtr, &refFreq, &clkSel));
    switch (refFreq)
    {
    case C112GX4_REFFREQ_25MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 99);     /* 25 * 4 - 1 = 99 */
        break;
    case C112GX4_REFFREQ_30MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 119);    /* 30 * 4 - 1 = 119 */
        break;
    case C112GX4_REFFREQ_40MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 159);    /* 40 * 4 - 1 = 159 */
        break;
    case C112GX4_REFFREQ_50MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 199);    /* 50 * 4 - 1 = 199 */
        break;
    case C112GX4_REFFREQ_62P25MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 247);    /* 62 * 4 - 1 = 247 */
        break;
    case C112GX4_REFFREQ_100MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 399);    /* 100 * 4 - 1 = 399 */
        break;
    case C112GX4_REFFREQ_125MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 499);    /* 125 * 4 - 1 = 499 */
        break;
    case C112GX4_REFFREQ_156P25MHZ:
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_TIMER, 255 /* ignored */, 623);    /* 156 * 4 - 1 = 623 */
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PCM_EN, 255 /* ignored */, 1);
    C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_DRO_EN, 255 /* ignored */, 1);

    for (i = 1; i <= 0xA; i++)
    {
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_DRO_SEL, 255 /* ignored */, i);
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_START, 255 /* ignored */, 1);
        C112GX4_POLL_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_READY, 255 /* ignored */, 1, 1000);
        C112GX4_POLL_FIELD(devPtr, F_C112GX4R1P0_ANA_FCLK_RDY_RD, 255 /* ignored */, 1, 1000);
        C112GX4_READ_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_CNT, 255 /* ignored */, data);
        freq->dro[i - 1] = (MCESD_U16)data;
        C112GX4_WRITE_FIELD(devPtr, F_C112GX4R1P0_PROCESSMON_START, 255 /* ignored */, 0);
    }

    return MCESD_OK;
}
#endif /* C112GX4 */
