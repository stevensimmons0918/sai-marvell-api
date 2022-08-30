/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_28G_PIPE4_RPLL_X4_1P2V
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "../mcesdUtils.h"
#include "mcesdC28GP4X4_Defs.h"
#include "mcesdC28GP4X4_API.h"
#include "mcesdC28GP4X4_RegRW.h"
#include "mcesdC28GP4X4_HwCntl.h"
#include <string.h>

#ifdef C28GP4X4

MCESD_STATUS API_C28GP4X4_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
)
{
    MCESD_FIELD fwMajorVer  = F_C28GP4X4R1P0_FW_MAJOR_VER;
    MCESD_FIELD fwMinorVer  = F_C28GP4X4R1P0_FW_MINOR_VER;
    MCESD_FIELD fwPatchVer  = F_C28GP4X4R1P0_FW_PATCH_VER;
    MCESD_FIELD fwBuildVer  = F_C28GP4X4R1P0_FW_BUILD_VER;
    MCESD_FIELD mcuStatus0  = F_C28GP4X4R1P0_MCU_STATUS0;
    MCESD_FIELD laneSel     = E_C28GP4X4R1P0_LANE_SEL;
    MCESD_U32 data;
    MCESD_BOOL mcuActive;

    MCESD_ATTEMPT(mcesdGetMCUActive(devPtr, &mcuStatus0, &laneSel, 2, &mcuActive));

        if (!mcuActive)
        {
            /* Firmware version is invalid */
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &fwMajorVer, 0));
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &fwMinorVer, 0));
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &fwPatchVer, 0));
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &fwBuildVer, 0));
        }

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &fwMajorVer, &data));
    *major = (MCESD_U8) data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &fwMinorVer, &data));
    *minor = (MCESD_U8) data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &fwPatchVer, &data));
    *patch = (MCESD_U8) data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &fwBuildVer, &data));
    *build = (MCESD_U8) data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *locked
)
{
    MCESD_FIELD rxCkSel = F_C28GP4X4R1P0_RX_CK_SEL;
    MCESD_FIELD lcPll   = F_C28GP4X4R1P0_PLL_LOCK_LCPLL;
    MCESD_FIELD ringPll = F_C28GP4X4R1P0_PLL_LOCK_RING_RD;
    MCESD_U32 ckSelData;

    /* First check which clock is used (LCPLL or Ring PLL) */
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxCkSel, &ckSelData));

    if (0 == ckSelData)
    {
        MCESD_U32 lcPllData;
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &lcPll, &lcPllData));
        *locked = (1 == lcPllData) ? MCESD_TRUE : MCESD_FALSE;
    }
    else
    {
        MCESD_U32 ringPllData;
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ringPll, &ringPllData));
        *locked = (1 == ringPllData) ? MCESD_TRUE : MCESD_FALSE;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
)
{
    MCESD_FIELD pllReadyTx = F_C28GP4X4R1P0_PLL_READY_TX;
    MCESD_FIELD pllReadyRx = F_C28GP4X4R1P0_PLL_READY_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &pllReadyTx, &data));
    *txReady = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &pllReadyRx, &data));
    *rxReady = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetCDRLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD cdrLock = F_C28GP4X4R1P0_CDR_LOCK;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &cdrLock, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD rxInit      = F_C28GP4X4R1P0_RX_INIT;
    MCESD_FIELD rxInitDone  = F_C28GP4X4R1P0_RX_INIT_DONE;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxInitDone, 0));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxInit, 1));
    MCESD_ATTEMPT(API_C28GP4X4_PollField(devPtr, lane, &rxInitDone, 1, 5000));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxInit, 0));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT0, 1));
        MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_INIT_DONE0, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT0, 0));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT1, 1));
        MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_INIT_DONE1, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT1, 0));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT2, 1));
        MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_INIT_DONE2, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT2, 0));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT3, 1));
        MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_INIT_DONE3, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_INIT3, 0));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD txEmPoCtrl      = F_C28GP4X4R1P0_TX_EM_PO_CTRL;
    MCESD_FIELD txEmPeakCtrl    = F_C28GP4X4R1P0_TX_EM_PEAK_CTRL;
    MCESD_FIELD txEmPreCtrl     = F_C28GP4X4R1P0_TX_EM_PRE_CTRL;
    MCESD_FIELD txEmPoCtrlEn    = F_C28GP4X4R1P0_TX_EM_PO_CTRL_EN;
    MCESD_FIELD txEmPeakCtrlEn  = F_C28GP4X4R1P0_TX_EM_PEAK_CTRL_EN;
    MCESD_FIELD txEmPreCtrlEn   = F_C28GP4X4R1P0_TX_EM_PRE_CTRL_EN;
    MCESD_FIELD txMargin        = F_C28GP4X4R1P0_TX_MARGIN;

    switch(param)
    {
    case C28GP4X4_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txEmPoCtrlEn, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txEmPoCtrl, paramValue));
        break;
    case C28GP4X4_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txEmPeakCtrlEn, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txEmPeakCtrl, paramValue));
        break;
    case C28GP4X4_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txEmPreCtrlEn, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txEmPreCtrl, paramValue));
        break;
    case C28GP4X4_TXEQ_MARGIN:
        if ((0x1 == paramValue) || (0x2 == paramValue))
            return MCESD_FAIL;
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txMargin, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD txEmPoCtrl      = F_C28GP4X4R1P0_TX_EM_PO_CTRL;
    MCESD_FIELD txEmPeakCtrl    = F_C28GP4X4R1P0_TX_EM_PEAK_CTRL;
    MCESD_FIELD txEmPreCtrl     = F_C28GP4X4R1P0_TX_EM_PRE_CTRL;
    MCESD_FIELD txMargin        = F_C28GP4X4R1P0_TX_MARGIN;

    switch(param)
    {
    case C28GP4X4_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txEmPoCtrl, paramValue));
        break;
    case C28GP4X4_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txEmPeakCtrl, paramValue));
        break;
    case C28GP4X4_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txEmPreCtrl, paramValue));
        break;
    case C28GP4X4_TXEQ_MARGIN:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txMargin, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD ffeDataRate = F_C28GP4X4R1P0_FFE_DATA_RATE;
    MCESD_FIELD res1Sel     = F_C28GP4X4R1P0_RES1_SEL;
    MCESD_FIELD res2SelE    = F_C28GP4X4R1P0_RES2_SEL_E;
    MCESD_FIELD res2SelO    = F_C28GP4X4R1P0_RES2_SEL_O;
    MCESD_FIELD cap1Sel     = F_C28GP4X4R1P0_CAP1_SEL;
    MCESD_FIELD cap2SelE    = F_C28GP4X4R1P0_CAP2_SEL_E;
    MCESD_FIELD cap2SelO    = F_C28GP4X4R1P0_CAP2_SEL_O;

    switch(param)
    {
    case C28GP4X4_CTLE_DATA_RATE:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ffeDataRate, paramValue));
        break;
    case C28GP4X4_CTLE_RES1_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &res1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_RES2_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &res2SelE, paramValue));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &res2SelO, paramValue));
        break;
    case C28GP4X4_CTLE_CAP1_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &cap1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_CAP2_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &cap2SelE, paramValue));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &cap2SelO, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD ffeDataRate = F_C28GP4X4R1P0_FFE_DATA_RATE;
    MCESD_FIELD res1Sel     = F_C28GP4X4R1P0_RES1_SEL;
    MCESD_FIELD res2SelE    = F_C28GP4X4R1P0_RES2_SEL_E;
    MCESD_FIELD cap1Sel     = F_C28GP4X4R1P0_CAP1_SEL;
    MCESD_FIELD cap2SelE    = F_C28GP4X4R1P0_CAP2_SEL_E;

    switch(param)
    {
    case C28GP4X4_CTLE_DATA_RATE:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ffeDataRate, paramValue));
        break;
    case C28GP4X4_CTLE_RES1_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &res1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_RES2_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &res2SelE, paramValue));
        break;
    case C28GP4X4_CTLE_CAP1_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &cap1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_CAP2_SEL:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &cap2SelE, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD dfeEn = F_C28GP4X4R1P0_DFE_EN;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dfeEn, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD dfeEn = F_C28GP4X4R1P0_DFE_EN;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &dfeEn, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_DFE_EN3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    MCESD_U32 dataEven, dataOdd;
    MCESD_32 signedEven, signedOdd;

    /* For a given tap, Even & Odd samplers are averaged */
    switch(tap)
    {
    case C28GP4X4_DFE_F0_N:
        {
            MCESD_FIELD f0DNE = F_C28GP4X4R1P0_DFE_F0_D_N_E;
            MCESD_FIELD f0DNO = F_C28GP4X4R1P0_DFE_F0_D_N_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f0DNE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f0DNO, &dataOdd));
            *tapValue = (dataEven + dataOdd) / 2; /* F0 is always a positive value */
        }
        break;
    case C28GP4X4_DFE_F0_P:
        {
            MCESD_FIELD f0DPE = F_C28GP4X4R1P0_DFE_F0_D_P_E;
            MCESD_FIELD f0DPO = F_C28GP4X4R1P0_DFE_F0_D_P_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f0DPE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f0DPO, &dataOdd));
            *tapValue = (dataEven + dataOdd) / 2; /* F0 is always a positive value */
    }
        break;
    case C28GP4X4_DFE_F1:
        {
            MCESD_FIELD f1E = F_C28GP4X4R1P0_DFE_F1_E;
            MCESD_FIELD f1O = F_C28GP4X4R1P0_DFE_F1_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f1E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f1O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f1E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f1O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F2_N:
        {
            MCESD_FIELD f2E = F_C28GP4X4R1P0_DFE_F2_D_N_E;
            MCESD_FIELD f2O = F_C28GP4X4R1P0_DFE_F2_D_N_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f2E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f2O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f2E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f2O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F2_P:
        {
            MCESD_FIELD f2DPE = F_C28GP4X4R1P0_DFE_F2_D_P_E;
            MCESD_FIELD f2DPO = F_C28GP4X4R1P0_DFE_F2_D_P_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f2DPE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f2DPO, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f2DPE.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f2DPO.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F3_N:
        {
            MCESD_FIELD f3DNE = F_C28GP4X4R1P0_DFE_F3_D_N_E;
            MCESD_FIELD f3DNO = F_C28GP4X4R1P0_DFE_F3_D_N_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f3DNE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f3DNO, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f3DNE.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f3DNO.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F3_P:
        {
            MCESD_FIELD f3DPE = F_C28GP4X4R1P0_DFE_F3_D_P_E;
            MCESD_FIELD f3DPO = F_C28GP4X4R1P0_DFE_F3_D_P_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f3DPE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f3DPO, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f3DPE.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f3DPO.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F4:
        {
            MCESD_FIELD f4E = F_C28GP4X4R1P0_DFE_F4_E;
            MCESD_FIELD f4O = F_C28GP4X4R1P0_DFE_F4_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f4E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f4O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f4E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f4O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F5:
        {
            MCESD_FIELD f5E = F_C28GP4X4R1P0_DFE_F5_E;
            MCESD_FIELD f5O = F_C28GP4X4R1P0_DFE_F5_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f5E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f5O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f5E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f5O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F6:
        {
            MCESD_FIELD f6E = F_C28GP4X4R1P0_DFE_F6_E;
            MCESD_FIELD f6O = F_C28GP4X4R1P0_DFE_F6_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f6E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f6O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f6E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f6O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F7:
        {
            MCESD_FIELD f7E = F_C28GP4X4R1P0_DFE_F7_E;
            MCESD_FIELD f7O = F_C28GP4X4R1P0_DFE_F7_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f7E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f7O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f7E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f7O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F8:
        {
            MCESD_FIELD f8E = F_C28GP4X4R1P0_DFE_F8_E;
            MCESD_FIELD f8O = F_C28GP4X4R1P0_DFE_F8_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f8E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f8O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f8E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f8O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F9:
        {
            MCESD_FIELD f9E = F_C28GP4X4R1P0_DFE_F9_E;
            MCESD_FIELD f9O = F_C28GP4X4R1P0_DFE_F9_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f9E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f9O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f9E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f9O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F10:
        {
            MCESD_FIELD f10E = F_C28GP4X4R1P0_DFE_F10_E;
            MCESD_FIELD f10O = F_C28GP4X4R1P0_DFE_F10_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f10E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f10O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f10E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f10O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F11:
        {
            MCESD_FIELD f11E = F_C28GP4X4R1P0_DFE_F11_E;
            MCESD_FIELD f11O = F_C28GP4X4R1P0_DFE_F11_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f11E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f11O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f11E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f11O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F12:
        {
            MCESD_FIELD f12E = F_C28GP4X4R1P0_DFE_F12_E;
            MCESD_FIELD f12O = F_C28GP4X4R1P0_DFE_F12_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f12E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f12O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f12E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f12O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F13:
        {
            MCESD_FIELD f13E = F_C28GP4X4R1P0_DFE_F13_E;
            MCESD_FIELD f13O = F_C28GP4X4R1P0_DFE_F13_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f13E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f13O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f13E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f13O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F14:
        {
            MCESD_FIELD f14E = F_C28GP4X4R1P0_DFE_F14_E;
            MCESD_FIELD f14O = F_C28GP4X4R1P0_DFE_F14_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f14E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f14O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f14E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f14O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_F15:
        {
            MCESD_FIELD f15E = F_C28GP4X4R1P0_DFE_F15_E;
            MCESD_FIELD f15O = F_C28GP4X4R1P0_DFE_F15_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f15E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &f15O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f15E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f15O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_FF0:
        {
            MCESD_FIELD ff0E = F_C28GP4X4R1P0_DFE_FF0_E;
            MCESD_FIELD ff0O = F_C28GP4X4R1P0_DFE_FF0_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff0E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff0O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff0E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff0O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_FF1:
        {
            MCESD_FIELD ff1E = F_C28GP4X4R1P0_DFE_FF1_E;
            MCESD_FIELD ff1O = F_C28GP4X4R1P0_DFE_FF1_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff1E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff1O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff1E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff1O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_FF2:
        {
            MCESD_FIELD ff2E = F_C28GP4X4R1P0_DFE_FF2_E;
            MCESD_FIELD ff2O = F_C28GP4X4R1P0_DFE_FF2_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff2E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff2O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff2E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff2O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_FF3:
        {
            MCESD_FIELD ff3E = F_C28GP4X4R1P0_DFE_FF3_E;
            MCESD_FIELD ff3O = F_C28GP4X4R1P0_DFE_FF3_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff3E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff3O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff3E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff3O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_FF4:
        {
            MCESD_FIELD ff4E = F_C28GP4X4R1P0_DFE_FF4_E;
            MCESD_FIELD ff4O = F_C28GP4X4R1P0_DFE_FF4_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff4E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff4O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff4E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff4O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X4_DFE_FF5:
        {
            MCESD_FIELD ff5E = F_C28GP4X4R1P0_DFE_FF5_E;
            MCESD_FIELD ff5O = F_C28GP4X4R1P0_DFE_FF5_O;

            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff5E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ff5O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff5E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff5O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    default:
        return MCESD_FAIL; /* Unsupported DFE tap */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD mcuEnLane0 = F_C28GP4X4R1P0_MCU_EN_LANE0;
    MCESD_FIELD mcuEnLane1 = F_C28GP4X4R1P0_MCU_EN_LANE1;
    MCESD_FIELD mcuEnLane2 = F_C28GP4X4R1P0_MCU_EN_LANE2;
    MCESD_FIELD mcuEnLane3 = F_C28GP4X4R1P0_MCU_EN_LANE3;
    MCESD_FIELD mcuEnCmn   = F_C28GP4X4R1P0_MCU_EN_CMN;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &mcuEnLane3, state));
        break;
    case 255: /* Common */
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &mcuEnCmn, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD mcuEnLane0 = F_C28GP4X4R1P0_MCU_EN_LANE0;
    MCESD_FIELD mcuEnLane1 = F_C28GP4X4R1P0_MCU_EN_LANE1;
    MCESD_FIELD mcuEnLane2 = F_C28GP4X4R1P0_MCU_EN_LANE2;
    MCESD_FIELD mcuEnLane3 = F_C28GP4X4R1P0_MCU_EN_LANE3;
    MCESD_U32 data;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &mcuEnLane3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD enLane0 = F_C28GP4X4R1P0_EN_LANE0;
    MCESD_FIELD enLane1 = F_C28GP4X4R1P0_EN_LANE1;
    MCESD_FIELD enLane2 = F_C28GP4X4R1P0_EN_LANE2;
    MCESD_FIELD enLane3 = F_C28GP4X4R1P0_EN_LANE3;
    MCESD_FIELD enCmn   = F_C28GP4X4R1P0_EN_CMN;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &enLane0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &enLane1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &enLane2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &enLane3, state));
        break;
    case 255:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &enCmn, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD enLane0 = F_C28GP4X4R1P0_EN_LANE0;
    MCESD_FIELD enLane1 = F_C28GP4X4R1P0_EN_LANE1;
    MCESD_FIELD enLane2 = F_C28GP4X4R1P0_EN_LANE2;
    MCESD_FIELD enLane3 = F_C28GP4X4R1P0_EN_LANE3;
    MCESD_U32 data;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &enLane0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &enLane1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &enLane2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &enLane3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD broadcast = F_C28GP4X4R1P0_BROADCAST;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored*/, &broadcast, state));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD broadcast = F_C28GP4X4R1P0_BROADCAST;
    MCESD_U32 broadcastData;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored*/, &broadcast, &broadcastData));
    *state = (0 == broadcastData) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puIvref         = F_C28GP4X4R1P0_PU_IVREF;
    MCESD_FIELD puIvrefFmReg    = F_C28GP4X4R1P0_PU_IVREF_FM_REG;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &puIvref, state));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &puIvrefFmReg, 1));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_IVREF, state));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puIvref = F_C28GP4X4R1P0_PU_IVREF;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &puIvref, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_IVREF, &pinValue));
    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puTx = F_C28GP4X4R1P0_PU_TX;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puTx, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_TX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_TX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_TX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_TX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puTx = F_C28GP4X4R1P0_PU_TX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &puTx, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetPowerTx
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
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_TX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_TX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_TX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_TX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puRx = F_C28GP4X4R1P0_PU_RX;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puRx, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_RX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_RX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_RX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_RX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puRx = F_C28GP4X4R1P0_PU_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &puRx, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetPowerRx
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
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_RX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_RX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_RX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_RX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD txIdle = F_C28GP4X4R1P0_TX_IDLE;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txIdle, state ? 0 : 1));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE0, state ? 0 : 1));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE1, state ? 0 : 1));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE2, state ? 0 : 1));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE3, state ? 0 : 1));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD txIdle = F_C28GP4X4R1P0_TX_IDLE;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txIdle, &data));
    *state = (0 == data) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetTxOutputEnable
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
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_IDLE3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puPll = F_C28GP4X4R1P0_PU_PLL;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puPll, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puPll = F_C28GP4X4R1P0_PU_PLL;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &puPll, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetPowerPLL
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
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PU_PLL3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X4_PHYMODE mode
)
{
    MCESD_FIELD phyMode = F_C28GP4X4R1P0_PHY_MODE;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* Ignored */, &phyMode, mode));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X4_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_MODE, mode));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X4_PHYMODE *mode
)
{
    MCESD_FIELD phyMode = F_C28GP4X4R1P0_PHY_MODE;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* Ignored */, &phyMode, &data));
    *mode = (E_C28GP4X4_PHYMODE)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X4_PHYMODE *mode
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PHY_MODE, &pinValue));
    *mode = (E_C28GP4X4_PHYMODE)pinValue;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X4_REFFREQ freq,
    IN E_C28GP4X4_REFCLK_SEL clkSel
)
{
    MCESD_FIELD refFrefSel  = F_C28GP4X4R1P0_REF_FREF_SEL;
    MCESD_FIELD refClkSel   = F_C28GP4X4R1P0_REFCLK_SEL;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &refFrefSel, freq));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &refClkSel, clkSel));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X4_REFFREQ freq,
    IN E_C28GP4X4_REFCLK_SEL clkSel
)
{
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_REF_FREF_SEL, freq));
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_REFCLK_SEL, clkSel));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X4_REFFREQ *freq,
    OUT E_C28GP4X4_REFCLK_SEL *clkSel
)
{
    MCESD_FIELD refFrefSel  = F_C28GP4X4R1P0_REF_FREF_SEL;
    MCESD_FIELD refClkSel   = F_C28GP4X4R1P0_REFCLK_SEL;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &refFrefSel, &data));
    *freq = (E_C28GP4X4_REFFREQ)data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &refClkSel, &data));
    *clkSel = (E_C28GP4X4_REFCLK_SEL)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X4_REFFREQ *freq,
    OUT E_C28GP4X4_REFCLK_SEL *clkSel
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_REF_FREF_SEL, &pinValue));
    *freq = (E_C28GP4X4_REFFREQ) pinValue;

    MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_REFCLK_SEL, &pinValue));
    *clkSel = (E_C28GP4X4_REFCLK_SEL) pinValue;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SERDES_SPEED speed
)
{
    MCESD_FIELD phyGenTx    = F_C28GP4X4R1P0_PHY_GEN_TX;
    MCESD_FIELD phyGenRx    = F_C28GP4X4R1P0_PHY_GEN_RX;
    MCESD_FIELD phyGenMax   = F_C28GP4X4R1P0_PHY_GEN_MAX;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &phyGenTx, speed));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &phyGenRx, speed));

    if (C28GP4X4_SERDES_10P3125G == speed)
        MCESD_ATTEMPT(API_C28GP4X4_SetTxEqParam(devPtr, lane, C28GP4X4_TXEQ_MARGIN, 0x4));

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &phyGenMax, 0xD));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SERDES_SPEED speed
)
{
    MCESD_FIELD phyGenMax = F_C28GP4X4R1P0_PHY_GEN_MAX;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_TX0, speed));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX0, speed));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_TX1, speed));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX1, speed));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_TX2, speed));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX2, speed));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_TX3, speed));
        MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX3, speed));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    if (C28GP4X4_SERDES_10P3125G == speed)
        MCESD_ATTEMPT(API_C28GP4X4_SetTxEqParam(devPtr, lane, C28GP4X4_TXEQ_MARGIN, 0x4));

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &phyGenMax, 0xD));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_SERDES_SPEED *speed
)
{
    MCESD_FIELD phyGenRx = F_C28GP4X4R1P0_PHY_GEN_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &phyGenRx, &data));
    *speed = (E_C28GP4X4_SERDES_SPEED)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_SERDES_SPEED *speed
)
{
    MCESD_U16 pinValue;

    switch(lane)
    {
    case 0:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_PHY_GEN_RX3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *speed = (E_C28GP4X4_SERDES_SPEED)pinValue;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_DATABUS_WIDTH txWidth,
    IN E_C28GP4X4_DATABUS_WIDTH rxWidth
)
{
    MCESD_FIELD txSelBits = F_C28GP4X4R1P0_TX_SEL_BITS;
    MCESD_FIELD rxSelBits = F_C28GP4X4R1P0_RX_SEL_BITS;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txSelBits, txWidth));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxSelBits, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_DATABUS_WIDTH *txWidth,
    OUT E_C28GP4X4_DATABUS_WIDTH *rxWidth
)
{
    MCESD_FIELD txSelBits = F_C28GP4X4R1P0_TX_SEL_BITS;
    MCESD_FIELD rxSelBits = F_C28GP4X4R1P0_RX_SEL_BITS;
    MCESD_U32 txData, rxData;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txSelBits, &txData));
    *txWidth = (E_C28GP4X4_DATABUS_WIDTH) txData;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxSelBits, &rxData));
    *rxWidth = (E_C28GP4X4_DATABUS_WIDTH) rxData;

    return MCESD_OK;
}

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_FIELD mcuFreq = F_C28GP4X4R1P0_MCU_FREQ;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &mcuFreq, clockMHz));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_FIELD mcuFreq = F_C28GP4X4R1P0_MCU_FREQ;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &mcuFreq, &data));
    *clockMHz = (MCESD_U16)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X4_SetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 align90
)
{
    MCESD_FIELD align90Ref = F_C28GP4X4R1P0_ALIGN90_REF;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &align90Ref, align90));
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *align90
)
{
    MCESD_FIELD align90Ref = F_C28GP4X4R1P0_ALIGN90_REF;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &align90Ref, &data));
    *align90 = (MCESD_U16)data;
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    IN S_C28GP4X4_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C28GP4X4R1P0_TRX_TRAIN_TIMER;
    MCESD_FIELD rxTrainTimer        = F_C28GP4X4R1P0_RX_TRAIN_TIMER;
    MCESD_FIELD txTrainTimerEnable  = F_C28GP4X4R1P0_TX_TRAIN_TIMEREN;
    MCESD_FIELD rxTrainTimerEnable  = F_C28GP4X4R1P0_RX_TRAIN_TIMEREN;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &trxTrainTimer, training->timeout));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainTimer, training->timeout));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    OUT S_C28GP4X4_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C28GP4X4R1P0_TRX_TRAIN_TIMER;
    MCESD_FIELD rxTrainTimer        = F_C28GP4X4R1P0_RX_TRAIN_TIMER;
    MCESD_FIELD txTrainTimerEnable  = F_C28GP4X4R1P0_TX_TRAIN_TIMEREN;
    MCESD_FIELD rxTrainTimerEnable  = F_C28GP4X4R1P0_RX_TRAIN_TIMEREN;
    MCESD_U32 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &trxTrainTimer, &timeoutData));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxTrainTimer, &timeoutData));
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = (MCESD_U16)timeoutData;

    return MCESD_OK;
}

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD txTrainComplete = F_C28GP4X4R1P0_TX_TRAIN_COM;
    MCESD_FIELD txTrainFailed   = F_C28GP4X4R1P0_TX_TRAIN_FAILED;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X4R1P0_RX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainComplete = F_C28GP4X4R1P0_RX_TRAIN_COM;
    MCESD_FIELD rxTrainFailed   = F_C28GP4X4R1P0_RX_TRAIN_FAILED;
    MCESD_U32 failed;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainEnable, 1));
        MCESD_ATTEMPT(API_C28GP4X4_PollField(devPtr, lane, &txTrainComplete, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txTrainFailed, &failed));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainEnable, 0));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainEnable, 1));
        MCESD_ATTEMPT(API_C28GP4X4_PollField(devPtr, lane, &rxTrainComplete, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxTrainFailed, &failed));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainEnable, 0));
    }

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}
#else
MCESD_STATUS API_C28GP4X4_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
)
{
    MCESD_U16 failed;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE0, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE0, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED0, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE1, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE1, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED1, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE2, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE2, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED2, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE3, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE3, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED3, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE3, 0));
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
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE0, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE0, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED0, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE1, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE1, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED1, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE2, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE2, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED2, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE3, 1));
            MCESD_ATTEMPT(API_C28GP4X4_PollPin(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE3, 1, 5000));
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED3, &failed));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE3, 0));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X4R1P0_RX_TRAIN_ENABLE;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainEnable, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainEnable, 1));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
)
{
    if (type == C28GP4X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS0, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE0, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS1, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS2, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS3, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE3, 1));
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
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS0, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE0, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS1, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS2, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS3, 1));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE3, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD txTrainComplete = F_C28GP4X4R1P0_TX_TRAIN_COM;
    MCESD_FIELD txTrainFailed   = F_C28GP4X4R1P0_TX_TRAIN_FAILED;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X4R1P0_RX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainComplete = F_C28GP4X4R1P0_RX_TRAIN_COM;
    MCESD_FIELD rxTrainFailed   = F_C28GP4X4R1P0_RX_TRAIN_FAILED;
    MCESD_U32 completeData, failedData = 0;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txTrainComplete, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txTrainFailed, &failedData));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxTrainComplete, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxTrainFailed, &failedData));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
        }
    }

    *completed = (0 == completeData) ? MCESD_FALSE : MCESD_TRUE;
    *failed = (0 == failedData) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U16 completeData, failedData = 0;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED0, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED1, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED2, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_COMPLETE3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_FAILED3, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
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
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED0, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED1, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED2, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_COMPLETE3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_FAILED3, &failedData));
                MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 5));
                MCESD_ATTEMPT(API_C28GP4X4_StopTraining(devPtr,lane,type));
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

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X4R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X4R1P0_RX_TRAIN_ENABLE;

    if (type == C28GP4X4_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_TRAINING type
)
{
    if (type == C28GP4X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_TX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
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
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS0, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS1, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS2, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_RX_TRAIN_ENABLE3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DFE_PAT_DIS3, 0));
            MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C28GP4X4_TRAINED_EYE_HEIGHT *trainedEyeHeight
)
{
    MCESD_FIELD trainF0a    = F_C28GP4X4R1P0_TRAIN_F0A;
    MCESD_FIELD trainF0aMax = F_C28GP4X4R1P0_TRAIN_F0A_MAX;
    MCESD_FIELD trainF0b    = F_C28GP4X4R1P0_TRAIN_F0B;
    MCESD_FIELD trainF0d    = F_C28GP4X4R1P0_TRAIN_F0D;
    MCESD_U32 f0aData, f0aMaxData, f0bData, f0dData;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &trainF0a, &f0aData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &trainF0aMax, &f0aMaxData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &trainF0b, &f0bData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &trainF0d, &f0dData));

    trainedEyeHeight->f0a = (MCESD_U8)f0aData;
    trainedEyeHeight->f0aMax = (MCESD_U8)f0aMaxData;
    trainedEyeHeight->f0b = (MCESD_U8)f0bData;
    trainedEyeHeight->f0d = (MCESD_U8)f0dData;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD selMufi = F_C28GP4X4R1P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C28GP4X4R1P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C28GP4X4R1P0_REG_SELMUPI;
    MCESD_FIELD selMupf = F_C28GP4X4R1P0_REG_SELMUPF;

    switch (param)
    {
    case C28GP4X4_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &selMufi, paramValue));
        break;
    case C28GP4X4_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &selMuff, paramValue));
        break;
    case C28GP4X4_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &selMupi, paramValue));
        break;
    case C28GP4X4_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &selMupf, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD selMufi = F_C28GP4X4R1P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C28GP4X4R1P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C28GP4X4R1P0_REG_SELMUPI;
    MCESD_FIELD selMupf = F_C28GP4X4R1P0_REG_SELMUPF;

    switch (param)
    {
    case C28GP4X4_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &selMufi, paramValue));
        break;
    case C28GP4X4_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &selMuff, paramValue));
        break;
    case C28GP4X4_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &selMupi, paramValue));
        break;
    case C28GP4X4_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &selMupf, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SLEWRATE_EN setting
)
{
    MCESD_FIELD slewRateEn = F_C28GP4X4R1P0_SLEWRATE_EN;

    switch (setting)
    {
    case C28GP4X4_SR_EN_DISABLE:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &slewRateEn, 0));
        break;
    case C28GP4X4_SR_EN_6GBPS_TO_12GBPS:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &slewRateEn, 1));
        break;
    case C28GP4X4_SR_EN_LESS_THAN_6GBPS:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &slewRateEn, 3));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_SLEWRATE_EN *setting
)
{
    MCESD_FIELD slewRateEn = F_C28GP4X4R1P0_SLEWRATE_EN;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &slewRateEn, &data));
    *setting = (E_C28GP4X4_SLEWRATE_EN)data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SLEWRATE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C28GP4X4R1P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C28GP4X4R1P0_SLEWCTRL1;

    switch (param)
    {
    case C28GP4X4_SR_CTRL0:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &slewCtrl0, paramValue));
        break;
    case C28GP4X4_SR_CTRL1:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &slewCtrl1, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_SLEWRATE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C28GP4X4R1P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C28GP4X4R1P0_SLEWCTRL1;

    switch (param)
    {
    case C28GP4X4_SR_CTRL0:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &slewCtrl0, paramValue));
        break;
    case C28GP4X4_SR_CTRL1:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &slewCtrl1, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

#ifdef C28GP4X4_ISOLATION
MCESD_STATUS API_C28GP4X4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_FIELD sqDetected = F_C28GP4X4R1P0_DPHY_SQDETECTED;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &sqDetected, &data));
    *squelched = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X4_GetSquelchDetect
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
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_SQ_DETECTED_LPF0, &pinValue));
        break;
    case 1:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_SQ_DETECTED_LPF1, &pinValue));
        break;
    case 2:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_SQ_DETECTED_LPF2, &pinValue));
        break;
    case 3:
        MCESD_ATTEMPT(API_C28GP4X4_HwGetPinCfg(devPtr, C28GP4X4_PIN_SQ_DETECTED_LPF3, &pinValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *squelched = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 threshold
)
{
    MCESD_FIELD sqThresh = F_C28GP4X4R1P0_SQ_THRESH;
    MCESD_U32 data = threshold;

    if ((threshold < C28GP4X4_SQ_THRESH_MIN) || (threshold > C28GP4X4_SQ_THRESH_MAX))
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &sqThresh, data + 0x20));
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_16 *threshold
)
{
    MCESD_FIELD sqThresh = F_C28GP4X4R1P0_SQ_THRESH;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &sqThresh, &data));
    if (data >= 0x20)
        data -= 0x20;

    *threshold = (MCESD_16)data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_DATAPATH path
)
{
    MCESD_FIELD digRxToTx               = F_C28GP4X4R1P0_DIG_RX2TX_LPBKEN;
    MCESD_FIELD anaTxToRx               = F_C28GP4X4R1P0_ANA_TX2RX_LPBKEN;
    MCESD_FIELD digTxToRx               = F_C28GP4X4R1P0_DIG_TX2RX_LPBKEN;
    MCESD_FIELD dtxFloopEn              = F_C28GP4X4R1P0_DTX_FLOOP_EN;
    MCESD_FIELD dtxFloopEnRing          = F_C28GP4X4R1P0_DTX_FLOOP_RING;
    MCESD_FIELD rxFoffsetDisableLane    = F_C28GP4X4R1P0_RX_FOFFSET_DIS;
    MCESD_FIELD dtxFoffsetSel           = F_C28GP4X4R1P0_DTX_FOFFSET;
    MCESD_FIELD dtxFoffsetSelRing       = F_C28GP4X4R1P0_DTX_FOFFSET_RING;
    MCESD_FIELD rxCkSel                 = F_C28GP4X4R1P0_RX_CK_SEL;
    MCESD_FIELD puLb                    = F_C28GP4X4R1P0_PU_LB;
    MCESD_FIELD puLbDly                 = F_C28GP4X4R1P0_PU_LB_DLY;
    MCESD_FIELD dtlSqDetEn              = F_C28GP4X4R1P0_DTL_SQ_DET_EN;
    MCESD_U32 ckSelData;

    switch (path)
    {
    case C28GP4X4_PATH_LOCAL_ANALOG_LB:
        /* First check which clock is used (LCPLL or Ring PLL) */
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxCkSel, &ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFloopEn, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFloopEnRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFoffsetDisableLane, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFoffsetSel, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFoffsetSelRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtlSqDetEn, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &anaTxToRx, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puLb, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puLbDly, 1));
        break;
    case C28GP4X4_PATH_EXTERNAL:
        /* First check which clock is used (LCPLL or Ring PLL) */
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxCkSel, &ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFloopEn, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFloopEnRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFoffsetDisableLane, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFoffsetSel, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFoffsetSelRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &anaTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puLb, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puLbDly, 0));
        break;
    case C28GP4X4_PATH_FAR_END_LB:
        /* First check which clock is used (LCPLL or Ring PLL) */
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxCkSel, &ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFloopEn, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFloopEnRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFoffsetDisableLane, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFoffsetSel, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtxFoffsetSelRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &digRxToTx, 1));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &anaTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puLb, 0));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &puLbDly, 0));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_DATAPATH *path
)
{
    MCESD_FIELD digRxToTx = F_C28GP4X4R1P0_DIG_RX2TX_LPBKEN;
    MCESD_FIELD anaTxToRx = F_C28GP4X4R1P0_ANA_TX2RX_LPBKEN;
    MCESD_FIELD digTxToRx = F_C28GP4X4R1P0_DIG_TX2RX_LPBKEN;
    MCESD_U32 digRxToTxData, anaTxToRxData, digTxToRxData;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &digRxToTx, &digRxToTxData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &anaTxToRx, &anaTxToRxData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &digTxToRx, &digTxToRxData));

    if ((digRxToTxData == 0) && (anaTxToRxData == 0) && (digTxToRxData == 0))
        *path = C28GP4X4_PATH_EXTERNAL;
    else if ((digRxToTxData == 1) && (anaTxToRxData == 0) && (digTxToRxData == 0))
        *path = C28GP4X4_PATH_FAR_END_LB;
    else if ((digRxToTxData == 0) && (anaTxToRxData == 1) && (digTxToRxData == 0))
        *path = C28GP4X4_PATH_LOCAL_ANALOG_LB;
    else
        *path = C28GP4X4_PATH_UNKNOWN;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
)
{
    MCESD_FIELD tsenAdcData = F_C28GP4X4R1P0_TSEN_ADC_DATA;
    MCESD_FIELD tsenAdcMode = F_C28GP4X4R1P0_TSEN_ADC_MODE;
    MCESD_U32 savedMode, adcData;
    MCESD_32 signedValue;

    /* Save current TSEN ADC Mode */
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* Unused */, &tsenAdcMode, &savedMode));

    if (savedMode != 0)
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* Unused */, &tsenAdcMode, 0));

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* Unused */, &tsenAdcData, &adcData));

    signedValue = (MCESD_32)adcData;

    if (signedValue >= 512)
        signedValue -= 1024;

    /* temperature is returned in milli-Celsius */
    *temperature = (signedValue * C28GP4X4_TSENE_GAIN) + C28GP4X4_TSENE_OFFSET;

    /* Restore TSEN ADC Mode, if necesssary */
    if (savedMode != 0)
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* Unused*/, &tsenAdcMode, savedMode));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_POLARITY txPolarity,
    IN E_C28GP4X4_POLARITY rxPolarity
)
{
    MCESD_FIELD txdInv = F_C28GP4X4R1P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C28GP4X4R1P0_RXD_INV;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txdInv, txPolarity));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxdInv, rxPolarity));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_C28GP4X4_POLARITY *txPolarity,
    OUT E_C28GP4X4_POLARITY *rxPolarity
)
{
    MCESD_FIELD txdInv = F_C28GP4X4R1P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C28GP4X4R1P0_RXD_INV;
    MCESD_U32 txdInvData, rxdInvData;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txdInv, &txdInvData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxdInv, &rxdInvData));

    *txPolarity = (txdInvData == 0) ? C28GP4X4_POLARITY_NORMAL : C28GP4X4_POLARITY_INVERTED;
    *rxPolarity = (rxdInvData == 0) ? C28GP4X4_POLARITY_NORMAL : C28GP4X4_POLARITY_INVERTED;
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
)
{
    MCESD_FIELD addErrEn = F_C28GP4X4R1P0_ADD_ERR_EN;
    MCESD_FIELD addErrNum = F_C28GP4X4R1P0_ADD_ERR_NUM;

    if ((errors < 1) || (errors > 8))
        return MCESD_FAIL;

    /* 0 = 1 error, 1 = 2 errors, ... 7 = 8 errors */
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &addErrNum, errors - 1));

    /* Toggle F_C28GP4X4R1P0_ADD_ERR_EN 0 -> 1 to trigger error injection */
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &addErrEn, 0));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &addErrEn, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN S_C28GP4X4_PATTERN *txPattern,
    IN S_C28GP4X4_PATTERN *rxPattern,
    IN E_C28GP4X4_SATA_LONGSHORT sataLongShort,
    IN E_C28GP4X4_SATA_INITIAL_DISPARITY sataInitialDisparity,
    IN const char *userPattern,
    IN MCESD_U8 userK
)
{
    MCESD_FIELD txPatternSel        = F_C28GP4X4R1P0_PT_TX_PATT_SEL;
    MCESD_FIELD rxPatternSel        = F_C28GP4X4R1P0_PT_RX_PATT_SEL;
    MCESD_FIELD prbsEnc             = F_C28GP4X4R1P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong            = F_C28GP4X4R1P0_PT_SATA_LONG;
    MCESD_FIELD startRd             = F_C28GP4X4R1P0_PT_START_RD;
    MCESD_FIELD userPattern_79_48   = F_C28GP4X4R1P0_PT_USER_PAT_7948;
    MCESD_FIELD userPattern_47_16   = F_C28GP4X4R1P0_PT_USER_PAT_4716;
    MCESD_FIELD userPattern_15_0    = F_C28GP4X4R1P0_PT_USER_PAT_1500;
    MCESD_FIELD userKChar           = F_C28GP4X4R1P0_PT_USER_K_CHAR;
    E_C28GP4X4_DATABUS_WIDTH txWidth;
    E_C28GP4X4_DATABUS_WIDTH rxWidth;

    if (txPattern->pattern == C28GP4X4_PAT_USER)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txPatternSel, (txPattern->enc8B10B == C28GP4X4_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txPatternSel, txPattern->pattern));
    }

    if (rxPattern->pattern == C28GP4X4_PAT_USER)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxPatternSel, (rxPattern->enc8B10B == C28GP4X4_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxPatternSel, rxPattern->pattern));
    }

    if (((txPattern->pattern >= C28GP4X4_PAT_PRBS7) && (txPattern->pattern <= C28GP4X4_PAT_PRBS32)) ||
        ((rxPattern->pattern >= C28GP4X4_PAT_PRBS7) && (rxPattern->pattern <= C28GP4X4_PAT_PRBS32)))
    {
        if ((txPattern->enc8B10B == C28GP4X4_ENC_8B10B_ENABLE) || (rxPattern->enc8B10B == C28GP4X4_ENC_8B10B_ENABLE))
        {
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &prbsEnc, 1));
        }
        else
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &prbsEnc, 0));
    }

    if ((sataLongShort == C28GP4X4_SATA_LONG) || (sataLongShort == C28GP4X4_SATA_SHORT))
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &sataLong, sataLongShort));

    if ((sataInitialDisparity == C28GP4X4_DISPARITY_NEGATIVE) || (sataInitialDisparity == C28GP4X4_DISPARITY_POSITIVE))
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &startRd, sataInitialDisparity));

    if (strlen(userPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(userPattern, u8Pattern));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &userPattern_79_48, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3])));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &userPattern_47_16, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7])));
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &userPattern_15_0, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9])));
    }

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &userKChar, userK));

    MCESD_ATTEMPT(API_C28GP4X4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));
    if ((C28GP4X4_PAT_JITTER_8T == txPattern->pattern) || (C28GP4X4_PAT_JITTER_4T == txPattern->pattern))
        txWidth = C28GP4X4_DATABUS_32BIT;
    else if ((C28GP4X4_PAT_JITTER_10T == txPattern->pattern) || (C28GP4X4_PAT_JITTER_5T == txPattern->pattern))
        txWidth = C28GP4X4_DATABUS_40BIT;

    if ((C28GP4X4_PAT_JITTER_8T == rxPattern->pattern) || (C28GP4X4_PAT_JITTER_4T == rxPattern->pattern))
        rxWidth = C28GP4X4_DATABUS_32BIT;
    else if ((C28GP4X4_PAT_JITTER_10T == rxPattern->pattern) || (C28GP4X4_PAT_JITTER_5T == rxPattern->pattern))
        rxWidth = C28GP4X4_DATABUS_40BIT;
    MCESD_ATTEMPT(API_C28GP4X4_SetDataBusWidth(devPtr, lane, txWidth, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C28GP4X4_PATTERN *txPattern,
    OUT S_C28GP4X4_PATTERN *rxPattern,
    OUT E_C28GP4X4_SATA_LONGSHORT *sataLongShort,
    OUT E_C28GP4X4_SATA_INITIAL_DISPARITY *sataInitialDisparity,
    OUT char *userPattern,
    OUT MCESD_U8 *userK
)
{
    MCESD_FIELD txPatternSel    = F_C28GP4X4R1P0_PT_TX_PATT_SEL;
    MCESD_FIELD rxPatternSel    = F_C28GP4X4R1P0_PT_RX_PATT_SEL;
    MCESD_FIELD prbsEnc         = F_C28GP4X4R1P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong        = F_C28GP4X4R1P0_PT_SATA_LONG;
    MCESD_FIELD startRd         = F_C28GP4X4R1P0_PT_START_RD;
    MCESD_FIELD userKChar       = F_C28GP4X4R1P0_PT_USER_K_CHAR;
    MCESD_FIELD userPatternFieldArray[] = { F_C28GP4X4R1P0_PT_USER_PAT_7948 , F_C28GP4X4R1P0_PT_USER_PAT_4716 , F_C28GP4X4R1P0_PT_USER_PAT_1500 };
    MCESD_32 userPatternFieldArrayCount = sizeof(userPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 i;
    MCESD_U32 txPatternSelData, rxPatternSelData, prbsEncData, sataLongData, startRdData, userPatternData, userKData;
    MCESD_U8 u8Pattern[10];

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txPatternSel, &txPatternSelData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxPatternSel, &rxPatternSelData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &prbsEnc, &prbsEncData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &sataLong, &sataLongData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &startRd, &startRdData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &userKChar, &userKData));

    if ((txPatternSelData == 1) || (txPatternSelData == 2))
    {
        txPattern->pattern = C28GP4X4_PAT_USER;
        txPattern->enc8B10B = (txPatternSelData == 1) ? C28GP4X4_ENC_8B10B_DISABLE : C28GP4X4_ENC_8B10B_ENABLE;
    }
    else
    {
        txPattern->pattern = (E_C28GP4X4_PATTERN)txPatternSelData;

        if (txPattern->pattern > C28GP4X4_PAT_SATA_LTDP)
            txPattern->enc8B10B = C28GP4X4_ENC_8B10B_ENABLE;
        else if ((txPattern->pattern >= C28GP4X4_PAT_PRBS7) && (txPattern->pattern <= C28GP4X4_PAT_PRBS32))
            txPattern->enc8B10B = (prbsEncData == 0) ? C28GP4X4_ENC_8B10B_DISABLE : C28GP4X4_ENC_8B10B_ENABLE;
        else
            txPattern->enc8B10B = C28GP4X4_ENC_8B10B_DISABLE;
    }

    if ((rxPatternSelData == 1) || (rxPatternSelData == 2))
    {
        rxPattern->pattern = C28GP4X4_PAT_USER;
        rxPattern->enc8B10B = (rxPatternSelData == 1) ? C28GP4X4_ENC_8B10B_DISABLE : C28GP4X4_ENC_8B10B_ENABLE;
    }
    else
    {
        rxPattern->pattern = (E_C28GP4X4_PATTERN)rxPatternSelData;

        if (rxPattern->pattern > C28GP4X4_PAT_SATA_LTDP)
            rxPattern->enc8B10B = C28GP4X4_ENC_8B10B_ENABLE;
        else if ((rxPattern->pattern >= C28GP4X4_PAT_PRBS7) && (rxPattern->pattern <= C28GP4X4_PAT_PRBS32))
            rxPattern->enc8B10B = (prbsEncData == 0) ? C28GP4X4_ENC_8B10B_DISABLE : C28GP4X4_ENC_8B10B_ENABLE;
        else
            rxPattern->enc8B10B = C28GP4X4_ENC_8B10B_DISABLE;
    }

    *sataLongShort = (E_C28GP4X4_SATA_LONGSHORT)sataLongData;
    *sataInitialDisparity = (E_C28GP4X4_SATA_INITIAL_DISPARITY)startRdData;

    for (i = 0; i < userPatternFieldArrayCount; i++)
    {
        MCESD_FIELD userPatternField = userPatternFieldArray[i];
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &userPatternField, &userPatternData));
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

    *userK = (MCESD_U8)(userKData & 0xFF);

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_C28GP4X4_PATTERN_STATISTICS *statistics
)
{
    MCESD_FIELD ptPass      = F_C28GP4X4R1P0_PT_PASS;
    MCESD_FIELD ptLock      = F_C28GP4X4R1P0_PT_LOCK;
    MCESD_FIELD ptCnt_47_16 = F_C28GP4X4R1P0_PT_CNT_47_16;
    MCESD_FIELD ptCnt_15_0  = F_C28GP4X4R1P0_PT_CNT_15_0;
    MCESD_FIELD errCnt      = F_C28GP4X4R1P0_PT_ERR_CNT;
    MCESD_U32 passData, lockData, ErrData, data_47_16, data_15_0;
    E_C28GP4X4_DATABUS_WIDTH txWidth;
    E_C28GP4X4_DATABUS_WIDTH rxWidth;
    MCESD_U64 frames;

    if (statistics == NULL)
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ptPass, &passData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ptLock, &lockData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ptCnt_47_16, &data_47_16));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &ptCnt_15_0, &data_15_0));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &errCnt, &ErrData));
    MCESD_ATTEMPT(API_C28GP4X4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));

    frames = ((MCESD_U64)data_47_16 << 16) + data_15_0;

    statistics->totalBits = frames * ((rxWidth == C28GP4X4_DATABUS_32BIT) ? 32 : 40);
    statistics->totalErrorBits = ErrData;
    statistics->pass = (passData == 0) ? MCESD_FALSE : MCESD_TRUE;
    statistics->lock = (lockData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD ptCntRst = F_C28GP4X4R1P0_PT_CNT_RST;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptCntRst, 0));
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptCntRst, 1));
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptCntRst, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD ptEn            = F_C28GP4X4R1P0_PT_EN;
    MCESD_FIELD ptEnMode        = F_C28GP4X4R1P0_PT_EN_MODE;
    MCESD_FIELD ptPhyReadyForce = F_C28GP4X4R1P0_PT_PHYREADYFORCE;
    MCESD_FIELD ptRst           = F_C28GP4X4R1P0_PT_RST;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptEn, 0));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptEnMode, 2));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptEn, 1));

    /* Reset PHY Test */
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptRst, 0));
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptRst, 1));
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptRst, 0));

    /* Wait 10 ms for CDR to lock... */
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 10));

    /* ...before starting pattern checker */
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptPhyReadyForce, 1));
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD ptEn = F_C28GP4X4R1P0_PT_EN;
    MCESD_FIELD ptPhyReadyForce = F_C28GP4X4R1P0_PT_PHYREADYFORCE;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &ptEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD esmPathSel          = F_C28GP4X4R1P0_ESM_PATH_SEL;
    MCESD_FIELD esmDfeAdaptSplrEn   = F_C28GP4X4R1P0_ESM_DFE_SPLR_EN;
    MCESD_FIELD esmEn               = F_C28GP4X4R1P0_ESM_EN;
    MCESD_FIELD eomReady            = F_C28GP4X4R1P0_EOM_READY;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &esmPathSel, 1));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &esmDfeAdaptSplrEn, 0xC));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &esmEn, 1));
    MCESD_ATTEMPT(API_C28GP4X4_PollField(devPtr, lane, &eomReady, 1, 3000));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_FIELD esmEn = F_C28GP4X4R1P0_ESM_EN;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &esmEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_C28GP4X4_EOM_DATA *measurement
)
{
    MCESD_FIELD esmPhase    = F_C28GP4X4R1P0_ESM_PHASE;
    MCESD_FIELD esmVoltage  = F_C28GP4X4R1P0_ESM_VOLTAGE;
    MCESD_FIELD eomDfeCall  = F_C28GP4X4R1P0_DFE_CALL;
    MCESD_FIELD eomVldCntPE = F_C28GP4X4R1P0_EOM_VLD_CNT_P_E;
    MCESD_FIELD eomVldCntPO = F_C28GP4X4R1P0_EOM_VLD_CNT_P_O;
    MCESD_FIELD eomVldCntNE = F_C28GP4X4R1P0_EOM_VLD_CNT_N_E;
    MCESD_FIELD eomVldCntNO = F_C28GP4X4R1P0_EOM_VLD_CNT_N_O;
    MCESD_FIELD eomErrCntPE = F_C28GP4X4R1P0_EOM_ERR_CNT_P_E;
    MCESD_FIELD eomErrCntPO = F_C28GP4X4R1P0_EOM_ERR_CNT_P_O;
    MCESD_FIELD eomErrCntNE = F_C28GP4X4R1P0_EOM_ERR_CNT_N_E;
    MCESD_FIELD eomErrCntNO = F_C28GP4X4R1P0_EOM_ERR_CNT_N_O;
    MCESD_U32 vldCntPEData, vldCntPOData, vldCntNEData, vldCntNOData;
    MCESD_U32 errCntPEData, errCntPOData, errCntNEData, errCntNOData;

    if (measurement == NULL)
        return MCESD_FAIL;

    /* Set Phase & Voltage */
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &esmPhase, phase));
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &esmVoltage, voltage));

    /* Call DFE to collect statistics */
    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &eomDfeCall, 1));
    MCESD_ATTEMPT(API_C28GP4X4_PollField(devPtr, lane, &eomDfeCall, 0, 2000));

    /* Retrieve valid counts */
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomVldCntPE, &vldCntPEData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomVldCntPO, &vldCntPOData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomVldCntNE, &vldCntNEData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomVldCntNO, &vldCntNOData));

    /* Retrieve error counts */
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomErrCntPE, &errCntPEData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomErrCntPO, &errCntPOData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomErrCntNE, &errCntNEData));
    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &eomErrCntNO, &errCntNOData));

    /* Update data structure */
    measurement->phase = phase;
    measurement->voltage = voltage;
    measurement->upperBitCount = vldCntPEData + vldCntPOData;
    measurement->upperBitErrorCount = errCntPEData + errCntPOData;
    measurement->lowerBitCount = vldCntNEData + vldCntNOData;
    measurement->lowerBitErrorCount = errCntNEData + errCntNOData;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
)
{
    MCESD_32 leftEdge = 0, rightEdge = 0, upperVoltage = -64, lowerVoltage = 63;
    MCESD_32 phase, voltage;

    MCESD_ATTEMPT(API_C28GP4X4_EOMInit(devPtr, lane));

    /* find left edge */
    for (phase = 0; phase < 512; phase++)
    {
        S_C28GP4X4_EOM_DATA measurement;
        MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, phase, 0, &measurement));

        if ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0))
        {
            leftEdge = phase; /* found left edge */
            break;
        }
    }

    /* find right edge */
    for (phase = 0; phase > -512; phase--)
    {
        S_C28GP4X4_EOM_DATA measurement;
        MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, phase, 0, &measurement));

        if ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0))
        {
            rightEdge = phase; /* found right edge */
            break;
        }
    }

    /* find upper and lower boundaries */
    for (voltage = 0; voltage < 64; voltage++)
    {
        S_C28GP4X4_EOM_DATA measurement;
        MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, 0, (MCESD_U8)voltage, &measurement));

        if ((upperVoltage == -64) && ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == 63) && ((measurement.lowerBitCount == 0) || (measurement.lowerBitErrorCount > 0)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -64) && (lowerVoltage != 63))
            break;
    }

    *width  = (MCESD_U16) ((leftEdge == rightEdge) ? 0 : (leftEdge - rightEdge) - 1);
    *height = (MCESD_U16) ((upperVoltage == lowerVoltage) ? 0 : (upperVoltage - lowerVoltage) - 1);

    MCESD_ATTEMPT(API_C28GP4X4_EOMFinalize(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C28GP4X4_EYE_RAW_PTR eyeRawDataPtr
)
{
    MCESD_16 maxVoltageStep, leftPhase, rightPhase, phaseIdx, voltIdx, voltage, startPt, tmp;
    MCESD_32 phase;
    MCESD_32 upperVoltage = -C28GP4X4_EYE_MAX_VOLT_STEPS;
    MCESD_32 lowerVoltage = C28GP4X4_EYE_MAX_VOLT_STEPS - 1;
    S_C28GP4X4_EOM_DATA measurement;

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C28GP4X4_EOMGetEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if (0 == voltageSteps)
        voltageSteps = C28GP4X4_EYE_DEFAULT_VOLT_STEPS;

    if (0 == phaseLevels)
        phaseLevels = C28GP4X4_EYE_DEFAULT_PHASE_LEVEL;

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

    MCESD_ATTEMPT(API_C28GP4X4_EOMInit(devPtr, lane));

    MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, 0, 0, &measurement));

    if (0 != measurement.upperBitErrorCount || 0 != measurement.lowerBitErrorCount)
    {
        MCESD_ATTEMPT(API_C28GP4X4_EOMFinalize(devPtr, lane));
        MCESD_DBG_ERROR("API_C28GP4X4_EOMGetEyeData: BER > 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
    {
        MCESD_ATTEMPT(API_C28GP4X4_EOMFinalize(devPtr, lane));
        MCESD_DBG_ERROR("API_C28GP4X4_EOMGetEyeData: Total Bit Count == 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    /* Sweep Voltage at Phase 0 */
    phaseIdx = phaseLevels;
    for (voltage = 0; voltage < voltageSteps; voltage++)
    {
        MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, 0, (MCESD_U8)voltage, &measurement));

        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

        if ((upperVoltage == -C28GP4X4_EYE_MAX_VOLT_STEPS) && ((0 == measurement.upperBitCount) || (0 < measurement.upperBitErrorCount)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == (C28GP4X4_EYE_MAX_VOLT_STEPS - 1)) && ((0 == measurement.lowerBitCount) || (0 < measurement.lowerBitErrorCount)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -C28GP4X4_EYE_MAX_VOLT_STEPS) && (lowerVoltage != (C28GP4X4_EYE_MAX_VOLT_STEPS - 1)))
            break;
    }

    /* Left Sweep from Phase 1 with Voltage Sweep */
    phaseIdx -= 1;
    startPt = (upperVoltage > -lowerVoltage) ? upperVoltage : lowerVoltage;
    for (phase = 1; phase < phaseLevels; phase++)
    {
        voltage = ((startPt + C28GP4X4_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C28GP4X4_EYE_VOLT_OFFSET);
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, phase, (MCESD_U8)voltage, &measurement));

            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

            if ((0 == measurement.lowerBitErrorCount) && (0 == measurement.upperBitErrorCount))
            {
                startPt = voltage;
                break;
            }

            if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
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
        voltage = ((startPt + C28GP4X4_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C28GP4X4_EYE_VOLT_OFFSET);
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C28GP4X4_EOMMeasPoint(devPtr, lane, phase, (MCESD_U8)voltage, &measurement));

            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

            if ((0 == measurement.lowerBitErrorCount) && (0 == measurement.upperBitErrorCount))
            {
                startPt = voltage;
                break;
            }

            if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
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

    MCESD_ATTEMPT(API_C28GP4X4_EOMFinalize(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_EOMPlotEyeData
(
    IN S_C28GP4X4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
)
{
    MCESD_U16 phaseIndex, voltageIndex, phaseInterval, timeUnit;
    char line[C28GP4X4_EYE_MAX_PHASE_LEVEL * 2 + 2];

    memset(&line, 'X', C28GP4X4_EYE_MAX_PHASE_LEVEL * 2 + 2);

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C28GP4X4_EOMPlotEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if ((0 == voltageSteps) || (voltageSteps > C28GP4X4_EYE_MAX_VOLT_STEPS))
        voltageSteps = C28GP4X4_EYE_DEFAULT_VOLT_STEPS;

    if ((0 == phaseLevels) || (voltageSteps > C28GP4X4_EYE_MAX_PHASE_LEVEL))
        phaseLevels = C28GP4X4_EYE_DEFAULT_PHASE_LEVEL;

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
            else if (C28GP4X4_EYE_LOW_ERROR_THRESH > eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex])
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

MCESD_STATUS API_C28GP4X4_GetDRO
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C28GP4X4_DRO_DATA *freq
)
{
    E_C28GP4X4_REFFREQ refFreq;
    E_C28GP4X4_REFCLK_SEL clkSel;
    MCESD_U8 i;
    MCESD_U32 data;

    if (freq == NULL)
        return MCESD_FAIL;

    /* The counter is based on reference clock. Count period is 1 us or 4 us. */
	/* If reference clock is 20 MHz, set 20 * 4 us - 1 = 79 (dec) to count for 4 us, set 19 for 1 us. */
	/* If reference clock is 62.5 MHz, set 63 * 4 us -1 = 251 (dec) to count for 4 us, set 63 for 1 us. */
    MCESD_ATTEMPT(API_C28GP4X4_GetRefFreq(devPtr, &refFreq, &clkSel));
    switch (refFreq)
    {
    case C28GP4X4_REFFREQ_25MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 99);  /* 25 * 4 - 1 = 99 */
        break;
    case C28GP4X4_REFFREQ_30MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 119); /* 30 * 4 - 1 = 119 */
        break;
    case C28GP4X4_REFFREQ_40MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 159); /* 40 * 4 - 1 = 159 */
        break;
    case C28GP4X4_REFFREQ_50MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 199); /* 50 * 4 - 1 = 199 */
        break;
    case C28GP4X4_REFFREQ_62P25MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 247); /* 62 * 4 - 1 = 247 */
        break;
    case C28GP4X4_REFFREQ_100MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 399); /* 100 * 4 - 1 = 399 */
        break;
    case C28GP4X4_REFFREQ_125MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 499); /* 125 * 4 - 1 = 499 */
        break;
    case C28GP4X4_REFFREQ_156P25MHZ:
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_TIMER, 255 /* ignored */, 623); /* 156 * 4 - 1 = 623 */
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_PCM_EN, 255 /* ignored */, 1);
    C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_DRO_EN, 255 /* ignored */, 1);
    C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FCLK_EN, 255 /* ignored */, 1);

    for (i = 1; i <= 0xA; i++)
    {
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_DRO_SEL, 255 /* ignored */, i);
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_START, 255 /* ignored */, 1);
        C28GP4X4_POLL_FIELD(devPtr, F_C28GP4X4R1P0_FBC_PLLCAL_CNT_R, 255 /* ignored */, 1, 1000);
        C28GP4X4_READ_FIELD(devPtr, F_C28GP4X4R1P0_FBC_PLLCAL_CNT, 255 /* ignored */, data);
        freq->dro[i - 1] = (MCESD_U16)data;
        C28GP4X4_WRITE_FIELD(devPtr, F_C28GP4X4R1P0_FBC_CNT_START, 255 /* ignored */, 0);
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetTxLocalPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 value
)
{
    MCESD_FIELD localTxPreset = F_C28GP4X4R1P0_LOCAL_TX_PRESET;

    if (value > 0xB)
        return MCESD_FAIL;  /* Invalid value (RESERVED) */

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, 255 /* ignored */, &localTxPreset, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetTxLocalPreset
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U32 *value
)
{
    MCESD_FIELD localTxPreset = F_C28GP4X4R1P0_LOCAL_TX_PRESET;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, 255 /* ignored */, &localTxPreset, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetRemotePreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 value
)
{
    MCESD_FIELD txPresetIndex = F_C28GP4X4R1P0_TX_PRESET_INDEX;

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &txPresetIndex, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetRemotePreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U32 *value
)
{
    MCESD_FIELD txPresetIndex = F_C28GP4X4R1P0_TX_PRESET_INDEX;

    MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &txPresetIndex, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_SetCTLEPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PRESET parameter,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD rxFfeForceEn  = F_C28GP4X4R1P0_RXFFE_FORCE_EN;
    MCESD_FIELD rxFfeRes1Sel  = F_C28GP4X4R1P0_RXFFE_RES1_SEL;
    MCESD_FIELD rxFfeCap1Sel  = F_C28GP4X4R1P0_RXFFE_CAP1_SEL;
    MCESD_FIELD rxFfeRes2SelO = F_C28GP4X4R1P0_RXFFE_RES2_SEL_O;
    MCESD_FIELD rxFfeRes2SelE = F_C28GP4X4R1P0_RXFFE_RES2_SEL_E;
    MCESD_FIELD rxFfeCap2SelO = F_C28GP4X4R1P0_RXFFE_CAP2_SEL_O;
    MCESD_FIELD rxFfeCap2SelE = F_C28GP4X4R1P0_RXFFE_CAP2_SEL_E;
    MCESD_FIELD align90RefG0  = F_C28GP4X4R1P0_ALIGN90_REF_G0;

    if (C28GP4X4_CTLE_PRESET_DEFAULT == parameter)
    {
        /* Use Firmware Default values */
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeForceEn, 0));
        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeForceEn, 1));
    switch (parameter)
    {
    case C28GP4X4_CTLE_PRESET_RES1:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeRes1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_CAP1:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeCap1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_RES2_O:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeRes2SelO, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_RES2_E:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeRes2SelE, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_CAP2_O:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeCap2SelO, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_CAP2_E:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &rxFfeCap2SelE, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_PHASE:
        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, lane, &align90RefG0, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Invalid parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X4_GetCTLEPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C28GP4X4_CTLE_PRESET parameter,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD rxFfeForceEn  = F_C28GP4X4R1P0_RXFFE_FORCE_EN;
    MCESD_FIELD rxFfeRes1Sel  = F_C28GP4X4R1P0_RXFFE_RES1_SEL;
    MCESD_FIELD rxFfeCap1Sel  = F_C28GP4X4R1P0_RXFFE_CAP1_SEL;
    MCESD_FIELD rxFfeRes2SelO = F_C28GP4X4R1P0_RXFFE_RES2_SEL_O;
    MCESD_FIELD rxFfeRes2SelE = F_C28GP4X4R1P0_RXFFE_RES2_SEL_E;
    MCESD_FIELD rxFfeCap2SelO = F_C28GP4X4R1P0_RXFFE_CAP2_SEL_O;
    MCESD_FIELD rxFfeCap2SelE = F_C28GP4X4R1P0_RXFFE_CAP2_SEL_E;
    MCESD_FIELD align90RefG0  = F_C28GP4X4R1P0_ALIGN90_REF_G0;

    *paramValue = 0;

    switch (parameter)
    {
    case C28GP4X4_CTLE_PRESET_DEFAULT:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeForceEn, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_RES1:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeRes1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_CAP1:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeCap1Sel, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_RES2_O:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeRes2SelO, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_RES2_E:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeRes2SelE, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_CAP2_O:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeCap2SelO, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_CAP2_E:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &rxFfeCap2SelE, paramValue));
        break;
    case C28GP4X4_CTLE_PRESET_PHASE:
        MCESD_ATTEMPT(API_C28GP4X4_ReadField(devPtr, lane, &align90RefG0, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Invalid parameter */
    }

    return MCESD_OK;
}

#endif /* C28GP4X4 */
