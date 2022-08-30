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
#include "../mcesdUtils.h"
#include "mcesdC28GP4X1_Defs.h"
#include "mcesdC28GP4X1_API.h"
#include "mcesdC28GP4X1_RegRW.h"
#include "mcesdC28GP4X1_HwCntl.h"
#include <string.h>

#ifdef C28GP4X1

MCESD_STATUS API_C28GP4X1_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
)
{
    MCESD_FIELD fwMajorVer  = F_C28GP4X1R1P0_FW_MAJOR_VER;
    MCESD_FIELD fwMinorVer  = F_C28GP4X1R1P0_FW_MINOR_VER;
    MCESD_FIELD fwPatchVer  = F_C28GP4X1R1P0_FW_PATCH_VER;
    MCESD_FIELD fwBuildVer  = F_C28GP4X1R1P0_FW_BUILD_VER;
    MCESD_FIELD mcuStatus0  = F_C28GP4X1R1P0_MCU_STATUS0;
    MCESD_FIELD laneSel     = E_C28GP4X1R1P0_LANE_SEL;
    MCESD_U32 data;
    MCESD_BOOL mcuActive;

    MCESD_ATTEMPT(mcesdGetMCUActive(devPtr, &mcuStatus0, &laneSel, 1, &mcuActive));

    if (!mcuActive)
    {
        /* Firmware version is invalid */
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &fwMajorVer, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &fwMinorVer, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &fwPatchVer, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &fwBuildVer, 0));
    }

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &fwMajorVer, &data));
    *major = (MCESD_U8) data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &fwMinorVer, &data));
    *minor = (MCESD_U8) data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &fwPatchVer, &data));
    *patch = (MCESD_U8) data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &fwBuildVer, &data));
    *build = (MCESD_U8) data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *locked
)
{
    MCESD_FIELD rxCkSel = F_C28GP4X1R1P0_RX_CK_SEL;
    MCESD_FIELD lcPll   = F_C28GP4X1R1P0_PLL_LOCK_LCPLL;
    MCESD_FIELD ringPll = F_C28GP4X1R1P0_PLL_LOCK_RING_RD;
    MCESD_U32 ckSelData;

    /* First check which clock is used (LCPLL or Ring PLL) */
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxCkSel, &ckSelData));

    if (ckSelData == 0)
    {
        MCESD_U32 lcPllData;
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &lcPll, &lcPllData));
        *locked = (lcPllData == 1) ? MCESD_TRUE : MCESD_FALSE;
    }
    else
    {
        MCESD_U32 ringPllData;
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ringPll, &ringPllData));
        *locked = (ringPllData == 1) ? MCESD_TRUE : MCESD_FALSE;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
)
{
    MCESD_FIELD pllReadyTx = F_C28GP4X1R1P0_PLL_READY_TX;
    MCESD_FIELD pllReadyRx = F_C28GP4X1R1P0_PLL_READY_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &pllReadyTx, &data));
    *txReady = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &pllReadyRx, &data));
    *rxReady = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetCDRLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD cdrLock = F_C28GP4X1R1P0_CDR_LOCK;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &cdrLock, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_RxInit
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD rxInit      = F_C28GP4X1R1P0_RX_INIT;
    MCESD_FIELD rxInitDone  = F_C28GP4X1R1P0_RX_INIT_DONE;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxInitDone, 0));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxInit, 1));
    MCESD_ATTEMPT(API_C28GP4X1_PollField(devPtr, &rxInitDone, 1, 5000));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxInit, 0));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_RxInit
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_INIT0, 1));
    MCESD_ATTEMPT(API_C28GP4X1_PollPin(devPtr, C28GP4X1_PIN_RX_INIT_DONE0, 1, 5000));
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_INIT0, 0));

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X1_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD txEmPoCtrl      = F_C28GP4X1R1P0_TX_EM_PO_CTRL;
    MCESD_FIELD txEmPeakCtrl    = F_C28GP4X1R1P0_TX_EM_PEAK_CTRL;
    MCESD_FIELD txEmPreCtrl     = F_C28GP4X1R1P0_TX_EM_PRE_CTRL;
    MCESD_FIELD txEmPoCtrlEn    = F_C28GP4X1R1P0_TX_EM_PO_CTRL_EN;
    MCESD_FIELD txEmPeakCtrlEn  = F_C28GP4X1R1P0_TX_EM_PEAK_CTRL_EN;
    MCESD_FIELD txEmPreCtrlEn   = F_C28GP4X1R1P0_TX_EM_PRE_CTRL_EN;
    MCESD_FIELD txMargin        = F_C28GP4X1R1P0_TX_MARGIN;

    switch(param)
    {
    case C28GP4X1_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txEmPoCtrlEn, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txEmPoCtrl, paramValue));
        break;
    case C28GP4X1_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txEmPeakCtrlEn, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txEmPeakCtrl, paramValue));
        break;
    case C28GP4X1_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txEmPreCtrlEn, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txEmPreCtrl, paramValue));
        break;
    case C28GP4X1_TXEQ_MARGIN:
        if ((0x1 == paramValue) || (0x2 == paramValue))
            return MCESD_FAIL;
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txMargin, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD txEmPoCtrl      = F_C28GP4X1R1P0_TX_EM_PO_CTRL;
    MCESD_FIELD txEmPeakCtrl    = F_C28GP4X1R1P0_TX_EM_PEAK_CTRL;
    MCESD_FIELD txEmPreCtrl     = F_C28GP4X1R1P0_TX_EM_PRE_CTRL;
    MCESD_FIELD txMargin        = F_C28GP4X1R1P0_TX_MARGIN;

    switch(param)
    {
    case C28GP4X1_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txEmPoCtrl, paramValue));
        break;
    case C28GP4X1_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txEmPeakCtrl, paramValue));
        break;
    case C28GP4X1_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txEmPreCtrl, paramValue));
        break;
    case C28GP4X1_TXEQ_MARGIN:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txMargin, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD ffeDataRate = F_C28GP4X1R1P0_FFE_DATA_RATE;
    MCESD_FIELD res1Sel     = F_C28GP4X1R1P0_RES1_SEL;
    MCESD_FIELD res2SelE    = F_C28GP4X1R1P0_RES2_SEL_E;
    MCESD_FIELD res2SelO    = F_C28GP4X1R1P0_RES2_SEL_O;
    MCESD_FIELD cap1Sel     = F_C28GP4X1R1P0_CAP1_SEL;
    MCESD_FIELD cap2SelE    = F_C28GP4X1R1P0_CAP2_SEL_E;
    MCESD_FIELD cap2SelO    = F_C28GP4X1R1P0_CAP2_SEL_O;

    switch(param)
    {
    case C28GP4X1_CTLE_DATA_RATE:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ffeDataRate, paramValue));
        break;
    case C28GP4X1_CTLE_RES1_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &res1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_RES2_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &res2SelE, paramValue));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &res2SelO, paramValue));
        break;
    case C28GP4X1_CTLE_CAP1_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &cap1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_CAP2_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &cap2SelE, paramValue));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &cap2SelO, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD ffeDataRate = F_C28GP4X1R1P0_FFE_DATA_RATE;
    MCESD_FIELD res1Sel     = F_C28GP4X1R1P0_RES1_SEL;
    MCESD_FIELD res2SelE    = F_C28GP4X1R1P0_RES2_SEL_E;
    MCESD_FIELD cap1Sel     = F_C28GP4X1R1P0_CAP1_SEL;
    MCESD_FIELD cap2SelE    = F_C28GP4X1R1P0_CAP2_SEL_E;

    switch(param)
    {
    case C28GP4X1_CTLE_DATA_RATE:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ffeDataRate, paramValue));
        break;
    case C28GP4X1_CTLE_RES1_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &res1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_RES2_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &res2SelE, paramValue));
        break;
    case C28GP4X1_CTLE_CAP1_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &cap1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_CAP2_SEL:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &cap2SelE, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD dfeEn = F_C28GP4X1R1P0_DFE_EN;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dfeEn, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DFE_EN0, state));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD dfeEn = F_C28GP4X1R1P0_DFE_EN;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &dfeEn, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_DFE_EN0, &pinValue));
    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X1_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    MCESD_U32 dataEven, dataOdd;
    MCESD_32 signedEven, signedOdd;

    /* For a given tap, Even & Odd samplers are averaged */
    switch(tap)
    {
    case C28GP4X1_DFE_F0_N:
        {
            MCESD_FIELD f0DNE = F_C28GP4X1R1P0_DFE_F0_D_N_E;
            MCESD_FIELD f0DNO = F_C28GP4X1R1P0_DFE_F0_D_N_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f0DNE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f0DNO, &dataOdd));
            *tapValue = (dataEven + dataOdd) / 2; /* F0 is always a positive value */
        }
        break;
    case C28GP4X1_DFE_F0_P:
        {
            MCESD_FIELD f0DPE = F_C28GP4X1R1P0_DFE_F0_D_P_E;
            MCESD_FIELD f0DPO = F_C28GP4X1R1P0_DFE_F0_D_P_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f0DPE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f0DPO, &dataOdd));
            *tapValue = (dataEven + dataOdd) / 2; /* F0 is always a positive value */
    }
        break;
    case C28GP4X1_DFE_F1:
        {
            MCESD_FIELD f1E = F_C28GP4X1R1P0_DFE_F1_E;
            MCESD_FIELD f1O = F_C28GP4X1R1P0_DFE_F1_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f1E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f1O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f1E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f1O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F2_N:
        {
            MCESD_FIELD f2E = F_C28GP4X1R1P0_DFE_F2_D_N_E;
            MCESD_FIELD f2O = F_C28GP4X1R1P0_DFE_F2_D_N_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f2E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f2O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f2E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f2O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F2_P:
        {
            MCESD_FIELD f2DPE = F_C28GP4X1R1P0_DFE_F2_D_P_E;
            MCESD_FIELD f2DPO = F_C28GP4X1R1P0_DFE_F2_D_P_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f2DPE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f2DPO, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f2DPE.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f2DPO.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F3_N:
        {
            MCESD_FIELD f3DNE = F_C28GP4X1R1P0_DFE_F3_D_N_E;
            MCESD_FIELD f3DNO = F_C28GP4X1R1P0_DFE_F3_D_N_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f3DNE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f3DNO, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f3DNE.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f3DNO.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F3_P:
        {
            MCESD_FIELD f3DPE = F_C28GP4X1R1P0_DFE_F3_D_P_E;
            MCESD_FIELD f3DPO = F_C28GP4X1R1P0_DFE_F3_D_P_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f3DPE, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f3DPO, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f3DPE.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f3DPO.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F4:
        {
            MCESD_FIELD f4E = F_C28GP4X1R1P0_DFE_F4_E;
            MCESD_FIELD f4O = F_C28GP4X1R1P0_DFE_F4_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f4E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f4O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f4E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f4O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F5:
        {
            MCESD_FIELD f5E = F_C28GP4X1R1P0_DFE_F5_E;
            MCESD_FIELD f5O = F_C28GP4X1R1P0_DFE_F5_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f5E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f5O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f5E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f5O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F6:
        {
            MCESD_FIELD f6E = F_C28GP4X1R1P0_DFE_F6_E;
            MCESD_FIELD f6O = F_C28GP4X1R1P0_DFE_F6_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f6E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f6O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f6E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f6O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F7:
        {
            MCESD_FIELD f7E = F_C28GP4X1R1P0_DFE_F7_E;
            MCESD_FIELD f7O = F_C28GP4X1R1P0_DFE_F7_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f7E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f7O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f7E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f7O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F8:
        {
            MCESD_FIELD f8E = F_C28GP4X1R1P0_DFE_F8_E;
            MCESD_FIELD f8O = F_C28GP4X1R1P0_DFE_F8_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f8E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f8O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f8E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f8O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F9:
        {
            MCESD_FIELD f9E = F_C28GP4X1R1P0_DFE_F9_E;
            MCESD_FIELD f9O = F_C28GP4X1R1P0_DFE_F9_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f9E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f9O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f9E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f9O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F10:
        {
            MCESD_FIELD f10E = F_C28GP4X1R1P0_DFE_F10_E;
            MCESD_FIELD f10O = F_C28GP4X1R1P0_DFE_F10_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f10E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f10O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f10E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f10O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F11:
        {
            MCESD_FIELD f11E = F_C28GP4X1R1P0_DFE_F11_E;
            MCESD_FIELD f11O = F_C28GP4X1R1P0_DFE_F11_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f11E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f11O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f11E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f11O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F12:
        {
            MCESD_FIELD f12E = F_C28GP4X1R1P0_DFE_F12_E;
            MCESD_FIELD f12O = F_C28GP4X1R1P0_DFE_F12_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f12E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f12O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f12E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f12O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F13:
        {
            MCESD_FIELD f13E = F_C28GP4X1R1P0_DFE_F13_E;
            MCESD_FIELD f13O = F_C28GP4X1R1P0_DFE_F13_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f13E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f13O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f13E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f13O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F14:
        {
            MCESD_FIELD f14E = F_C28GP4X1R1P0_DFE_F14_E;
            MCESD_FIELD f14O = F_C28GP4X1R1P0_DFE_F14_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f14E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f14O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f14E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f14O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_F15:
        {
            MCESD_FIELD f15E = F_C28GP4X1R1P0_DFE_F15_E;
            MCESD_FIELD f15O = F_C28GP4X1R1P0_DFE_F15_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f15E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &f15O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, f15E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, f15O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_FF0:
        {
            MCESD_FIELD ff0E = F_C28GP4X1R1P0_DFE_FF0_E;
            MCESD_FIELD ff0O = F_C28GP4X1R1P0_DFE_FF0_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff0E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff0O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff0E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff0O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_FF1:
        {
            MCESD_FIELD ff1E = F_C28GP4X1R1P0_DFE_FF1_E;
            MCESD_FIELD ff1O = F_C28GP4X1R1P0_DFE_FF1_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff1E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff1O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff1E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff1O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_FF2:
        {
            MCESD_FIELD ff2E = F_C28GP4X1R1P0_DFE_FF2_E;
            MCESD_FIELD ff2O = F_C28GP4X1R1P0_DFE_FF2_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff2E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff2O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff2E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff2O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_FF3:
        {
            MCESD_FIELD ff3E = F_C28GP4X1R1P0_DFE_FF3_E;
            MCESD_FIELD ff3O = F_C28GP4X1R1P0_DFE_FF3_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff3E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff3O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff3E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff3O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_FF4:
        {
            MCESD_FIELD ff4E = F_C28GP4X1R1P0_DFE_FF4_E;
            MCESD_FIELD ff4O = F_C28GP4X1R1P0_DFE_FF4_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff4E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff4O, &dataOdd));
            signedEven = ConvertSignedMagnitudeToI32(dataEven, ff4E.totalBits);
            signedOdd = ConvertSignedMagnitudeToI32(dataOdd, ff4O.totalBits);
            *tapValue = (signedEven + signedOdd) / 2;
        }
        break;
    case C28GP4X1_DFE_FF5:
        {
            MCESD_FIELD ff5E = F_C28GP4X1R1P0_DFE_FF5_E;
            MCESD_FIELD ff5O = F_C28GP4X1R1P0_DFE_FF5_O;

            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff5E, &dataEven));
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ff5O, &dataOdd));
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

MCESD_STATUS API_C28GP4X1_SetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD mcuEnLane0 = F_C28GP4X1R1P0_MCU_EN_LANE0;
    MCESD_FIELD mcuEnCmn = F_C28GP4X1R1P0_MCU_EN_CMN;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &mcuEnLane0, state));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &mcuEnCmn, state));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetMcuEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD mcuEnLane0 = F_C28GP4X1R1P0_MCU_EN_LANE0;
    MCESD_U32 data;
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &mcuEnLane0, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD enLane0 = F_C28GP4X1R1P0_EN_LANE0;
    MCESD_FIELD enCmn = F_C28GP4X1R1P0_EN_CMN;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &enLane0, state));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &enCmn, state));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetLaneEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD enLane0 = F_C28GP4X1R1P0_EN_LANE0;
    MCESD_U32 data;
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &enLane0, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD broadcast = F_C28GP4X1R1P0_BROADCAST;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &broadcast, state));
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD broadcast = F_C28GP4X1R1P0_BROADCAST;
    MCESD_U32 broadcastData;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &broadcast, &broadcastData));
    *state = (broadcastData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puIvref         = F_C28GP4X1R1P0_PU_IVREF;
    MCESD_FIELD puIvrefFmReg    = F_C28GP4X1R1P0_PU_IVREF_FM_REG;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puIvref, state));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puIvrefFmReg, 1));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PU_IVREF, state));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puIvref = F_C28GP4X1R1P0_PU_IVREF;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &puIvref, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_PU_IVREF, &pinValue));
    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puTx = F_C28GP4X1R1P0_PU_TX;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puTx, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PU_TX0, state));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puTx = F_C28GP4X1R1P0_PU_TX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &puTx, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_PU_TX0, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puRx = F_C28GP4X1R1P0_PU_RX;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puRx, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PU_RX0, state));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puRx = F_C28GP4X1R1P0_PU_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &puRx, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_PU_RX0, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD txIdle = F_C28GP4X1R1P0_TX_IDLE;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txIdle, state ? 0 : 1));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_IDLE0, state ? 0 : 1));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD txIdle = F_C28GP4X1R1P0_TX_IDLE;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txIdle, &data));
    *state = (0 == data) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_TX_IDLE0, &pinValue));
    *state = (0 == pinValue) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD puPll = F_C28GP4X1R1P0_PU_PLL;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puPll, state));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PU_PLL0, state));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD puPll = F_C28GP4X1R1P0_PU_PLL;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &puPll, &data));
    *state = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_PU_PLL0, &pinValue));
    *state = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_PHYMODE mode
)
{
    MCESD_FIELD phyMode = F_C28GP4X1R1P0_PHY_MODE;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &phyMode, mode));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PHY_MODE, mode));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_PHYMODE *mode
)
{
    MCESD_FIELD phyMode = F_C28GP4X1R1P0_PHY_MODE;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &phyMode, &data));
    *mode = (E_C28GP4X1_PHYMODE)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_PHYMODE *mode
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_PHY_MODE, &pinValue));
    *mode = (E_C28GP4X1_PHYMODE)pinValue;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_REFFREQ freq,
    IN E_C28GP4X1_REFCLK_SEL clkSel
)
{
    MCESD_FIELD refFrefSel  = F_C28GP4X1R1P0_REF_FREF_SEL;
    MCESD_FIELD refClkSel   = F_C28GP4X1R1P0_REFCLK_SEL;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &refFrefSel, freq));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &refClkSel, clkSel));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_REFFREQ freq,
    IN E_C28GP4X1_REFCLK_SEL clkSel
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_REF_FREF_SEL, freq));
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_REFCLK_SEL, clkSel));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_REFFREQ *freq,
    OUT E_C28GP4X1_REFCLK_SEL *clkSel
)
{
    MCESD_FIELD refFrefSel  = F_C28GP4X1R1P0_REF_FREF_SEL;
    MCESD_FIELD refClkSel   = F_C28GP4X1R1P0_REFCLK_SEL;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &refFrefSel, &data));
    *freq = (E_C28GP4X1_REFFREQ)data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &refClkSel, &data));
    *clkSel = (E_C28GP4X1_REFCLK_SEL)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_REFFREQ *freq,
    OUT E_C28GP4X1_REFCLK_SEL *clkSel
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_REF_FREF_SEL, &pinValue));
    *freq = (E_C28GP4X1_REFFREQ)pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_REFCLK_SEL, &pinValue));
    *clkSel = (E_C28GP4X1_REFCLK_SEL)pinValue;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_SERDES_SPEED speed
)
{
    MCESD_FIELD phyGenMax   = F_C28GP4X1R1P0_PHY_GEN_MAX;
    MCESD_FIELD phyGenTx    = F_C28GP4X1R1P0_PHY_GEN_TX;
    MCESD_FIELD phyGenRx    = F_C28GP4X1R1P0_PHY_GEN_RX;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &phyGenTx, speed));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &phyGenRx, speed));

    if (C28GP4X1_SERDES_10P3125G == speed)
        MCESD_ATTEMPT(API_C28GP4X1_SetTxEqParam(devPtr, C28GP4X1_TXEQ_MARGIN, 0x4));

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &phyGenMax, 0xD));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_SERDES_SPEED speed
)
{
    MCESD_FIELD phyGenMax = F_C28GP4X1R1P0_PHY_GEN_MAX;

    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PHY_GEN_TX0, speed));
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_PHY_GEN_RX0, speed));

    if (C28GP4X1_SERDES_10P3125G == speed)
        MCESD_ATTEMPT(API_C28GP4X1_SetTxEqParam(devPtr, C28GP4X1_TXEQ_MARGIN, 0x4));

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &phyGenMax, 0xD));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_SERDES_SPEED *speed
)
{
    MCESD_FIELD phyGenRx = F_C28GP4X1R1P0_PHY_GEN_RX;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &phyGenRx, &data));
    *speed = (E_C28GP4X1_SERDES_SPEED)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_SERDES_SPEED *speed
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_PHY_GEN_RX0, &pinValue));
    *speed = (E_C28GP4X1_SERDES_SPEED)pinValue;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X1_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_DATABUS_WIDTH txWidth,
    IN E_C28GP4X1_DATABUS_WIDTH rxWidth
)
{
    MCESD_FIELD txSelBits = F_C28GP4X1R1P0_TX_SEL_BITS;
    MCESD_FIELD rxSelBits = F_C28GP4X1R1P0_RX_SEL_BITS;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txSelBits, txWidth));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxSelBits, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_DATABUS_WIDTH *txWidth,
    OUT E_C28GP4X1_DATABUS_WIDTH *rxWidth
)
{
    MCESD_FIELD txSelBits = F_C28GP4X1R1P0_TX_SEL_BITS;
    MCESD_FIELD rxSelBits = F_C28GP4X1R1P0_RX_SEL_BITS;
    MCESD_U32 txData, rxData;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txSelBits, &txData));
    *txWidth = (E_C28GP4X1_DATABUS_WIDTH) txData;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxSelBits, &rxData));
    *rxWidth = (E_C28GP4X1_DATABUS_WIDTH) rxData;

    return MCESD_OK;
}

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_FIELD mcuFreq = F_C28GP4X1R1P0_MCU_FREQ;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &mcuFreq, clockMHz));

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_SetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_FIELD mcuFreq = F_C28GP4X1R1P0_MCU_FREQ;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &mcuFreq, &data));
    *clockMHz = (MCESD_U16)data;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetMcuClockFreq(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X1_SetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 align90
)
{
    MCESD_FIELD align90Ref = F_C28GP4X1R1P0_ALIGN90_REF;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &align90Ref, align90));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetAlign90(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *align90
)
{
    MCESD_FIELD align90Ref = F_C28GP4X1R1P0_ALIGN90_REF;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &align90Ref, &data));
    *align90 = (MCESD_U16)data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type,
    IN S_C28GP4X1_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C28GP4X1R1P0_TRX_TRAIN_TIMER;
    MCESD_FIELD rxTrainTimer        = F_C28GP4X1R1P0_RX_TRAIN_TIMER;
    MCESD_FIELD txTrainTimerEnable  = F_C28GP4X1R1P0_TX_TRAIN_TIMEREN;
    MCESD_FIELD rxTrainTimerEnable  = F_C28GP4X1R1P0_RX_TRAIN_TIMEREN;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &trxTrainTimer, training->timeout));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainTimer, training->timeout));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type,
    OUT S_C28GP4X1_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C28GP4X1R1P0_TRX_TRAIN_TIMER;
    MCESD_FIELD rxTrainTimer        = F_C28GP4X1R1P0_RX_TRAIN_TIMER;
    MCESD_FIELD txTrainTimerEnable  = F_C28GP4X1R1P0_TX_TRAIN_TIMEREN;
    MCESD_FIELD rxTrainTimerEnable  = F_C28GP4X1R1P0_RX_TRAIN_TIMEREN;
    MCESD_U32 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &trxTrainTimer, &timeoutData));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxTrainTimer, &timeoutData));
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = (MCESD_U16)timeoutData;

    return MCESD_OK;
}

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X1R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD txTrainComplete = F_C28GP4X1R1P0_TX_TRAIN_COM;
    MCESD_FIELD txTrainFailed   = F_C28GP4X1R1P0_TX_TRAIN_FAILED;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X1R1P0_RX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainComplete = F_C28GP4X1R1P0_RX_TRAIN_COM;
    MCESD_FIELD rxTrainFailed   = F_C28GP4X1R1P0_RX_TRAIN_FAILED;
    MCESD_U32 failed;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainEnable, 1));
        MCESD_ATTEMPT(API_C28GP4X1_PollField(devPtr, &txTrainComplete, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txTrainFailed, &failed));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainEnable, 0));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainEnable, 1));
        MCESD_ATTEMPT(API_C28GP4X1_PollField(devPtr, &rxTrainComplete, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxTrainFailed, &failed));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainEnable, 0));
    }

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}
#else
MCESD_STATUS API_C28GP4X1_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type
)
{
    MCESD_U16 failed;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_ENABLE0, 1));
        MCESD_ATTEMPT(API_C28GP4X1_PollPin(devPtr, C28GP4X1_PIN_TX_TRAIN_COMPLETE0, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_FAILED0, &failed));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_ENABLE0, 0));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_ENABLE0, 1));
        MCESD_ATTEMPT(API_C28GP4X1_PollPin(devPtr, C28GP4X1_PIN_RX_TRAIN_COMPLETE0, 1, 5000));
        MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_FAILED0, &failed));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_ENABLE0, 0));
    }

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X1R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X1R1P0_RX_TRAIN_ENABLE;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainEnable, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainEnable, 1));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type
)
{
    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DFE_PAT_DIS0, 1));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_ENABLE0, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DFE_PAT_DIS0, 1));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_ENABLE0, 1));
    }

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X1R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD txTrainComplete = F_C28GP4X1R1P0_TX_TRAIN_COM;
    MCESD_FIELD txTrainFailed   = F_C28GP4X1R1P0_TX_TRAIN_FAILED;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X1R1P0_RX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainComplete = F_C28GP4X1R1P0_RX_TRAIN_COM;
    MCESD_FIELD rxTrainFailed   = F_C28GP4X1R1P0_RX_TRAIN_FAILED;
    MCESD_U32 completeData, failedData = 0;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txTrainComplete, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txTrainFailed, &failedData));
            MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C28GP4X1_StopTraining(devPtr,type));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxTrainComplete, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxTrainFailed, &failedData));
            MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C28GP4X1_StopTraining(devPtr,type));
        }
    }

    *completed = (0 == completeData) ? MCESD_FALSE : MCESD_TRUE;
    *failed = (0 == failedData) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U16 completeData, failedData = 0;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_FAILED0, &failedData));
            MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C28GP4X1_StopTraining(devPtr,type));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_FAILED0, &failedData));
            MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C28GP4X1_StopTraining(devPtr,type));
        }
    }

    *completed = (0 == completeData) ? MCESD_FALSE : MCESD_TRUE;
    *failed = (0 == failedData) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type
)
{
    MCESD_FIELD txTrainEnable   = F_C28GP4X1R1P0_TX_TRAIN_ENABLE;
    MCESD_FIELD rxTrainEnable   = F_C28GP4X1R1P0_RX_TRAIN_ENABLE;

    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxTrainEnable, 0));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_TRAINING type
)
{
    if (type == C28GP4X1_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DFE_PAT_DIS0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_HwSetPinCfg(devPtr, C28GP4X1_PIN_DFE_PAT_DIS0, 0));
        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X1_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C28GP4X1_TRAINED_EYE_HEIGHT *trainedEyeHeight
)
{
    MCESD_FIELD trainF0a    = F_C28GP4X1R1P0_TRAIN_F0A;
    MCESD_FIELD trainF0aMax = F_C28GP4X1R1P0_TRAIN_F0A_MAX;
    MCESD_FIELD trainF0b    = F_C28GP4X1R1P0_TRAIN_F0B;
    MCESD_FIELD trainF0d    = F_C28GP4X1R1P0_TRAIN_F0D;
    MCESD_U32 f0aData, f0aMaxData, f0bData, f0dData;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &trainF0a, &f0aData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &trainF0aMax, &f0aMaxData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &trainF0b, &f0bData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &trainF0d, &f0dData));

    trainedEyeHeight->f0a = (MCESD_U8)f0aData;
    trainedEyeHeight->f0aMax = (MCESD_U8)f0aMaxData;
    trainedEyeHeight->f0b = (MCESD_U8)f0bData;
    trainedEyeHeight->f0d = (MCESD_U8)f0dData;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD selMufi = F_C28GP4X1R1P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C28GP4X1R1P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C28GP4X1R1P0_REG_SELMUPI;
    MCESD_FIELD selMupf = F_C28GP4X1R1P0_REG_SELMUPF;

    switch (param)
    {
    case C28GP4X1_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &selMufi, paramValue));
        break;
    case C28GP4X1_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &selMuff, paramValue));
        break;
    case C28GP4X1_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &selMupi, paramValue));
        break;
    case C28GP4X1_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &selMupf, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD selMufi = F_C28GP4X1R1P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C28GP4X1R1P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C28GP4X1R1P0_REG_SELMUPI;
    MCESD_FIELD selMupf = F_C28GP4X1R1P0_REG_SELMUPF;

    switch (param)
    {
    case C28GP4X1_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &selMufi, paramValue));
        break;
    case C28GP4X1_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &selMuff, paramValue));
        break;
    case C28GP4X1_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &selMupi, paramValue));
        break;
    case C28GP4X1_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &selMupf, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_SLEWRATE_EN setting
)
{
    MCESD_FIELD slewRateEn = F_C28GP4X1R1P0_SLEWRATE_EN;

    switch (setting)
    {
    case C28GP4X1_SR_EN_DISABLE:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &slewRateEn, 0));
        break;
    case C28GP4X1_SR_EN_6GBPS_TO_12GBPS:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &slewRateEn, 1));
        break;
    case C28GP4X1_SR_EN_LESS_THAN_6GBPS:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &slewRateEn, 3));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_SLEWRATE_EN *setting
)
{
    MCESD_FIELD slewRateEn = F_C28GP4X1R1P0_SLEWRATE_EN;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &slewRateEn, &data));
    *setting = (E_C28GP4X1_SLEWRATE_EN)data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_SLEWRATE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C28GP4X1R1P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C28GP4X1R1P0_SLEWCTRL1;

    switch (param)
    {
    case C28GP4X1_SR_CTRL0:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &slewCtrl0, paramValue));
        break;
    case C28GP4X1_SR_CTRL1:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &slewCtrl1, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_SLEWRATE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C28GP4X1R1P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C28GP4X1R1P0_SLEWCTRL1;

    switch (param)
    {
    case C28GP4X1_SR_CTRL0:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &slewCtrl0, paramValue));
        break;
    case C28GP4X1_SR_CTRL1:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &slewCtrl1, paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

#ifdef C28GP4X1_ISOLATION
MCESD_STATUS API_C28GP4X1_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_FIELD sqDetected = F_C28GP4X1R1P0_DPHY_SQDETECTED;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &sqDetected, &data));
    *squelched = (0 == data) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#else
MCESD_STATUS API_C28GP4X1_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, C28GP4X1_PIN_SQ_DETECTED_LPF0, &pinValue));
    *squelched = (0 == pinValue) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}
#endif

MCESD_STATUS API_C28GP4X1_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_16 threshold
)
{
    MCESD_FIELD sqThresh = F_C28GP4X1R1P0_SQ_THRESH;
    MCESD_U32 data = threshold;

    if ((threshold < C28GP4X1_SQ_THRESH_MIN) || (threshold > C28GP4X1_SQ_THRESH_MAX))
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &sqThresh, data + 0x20));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_16 *threshold
)
{
    MCESD_FIELD sqThresh = F_C28GP4X1R1P0_SQ_THRESH;
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &sqThresh, &data));
    if (data >= 0x20)
        data -= 0x20;

    *threshold = (MCESD_16)data;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_DATAPATH path
)
{
    MCESD_FIELD digRxToTx               = F_C28GP4X1R1P0_DIG_RX2TX_LPBKEN;
    MCESD_FIELD anaTxToRx               = F_C28GP4X1R1P0_ANA_TX2RX_LPBKEN;
    MCESD_FIELD digTxToRx               = F_C28GP4X1R1P0_DIG_TX2RX_LPBKEN;
    MCESD_FIELD dtxFloopEn              = F_C28GP4X1R1P0_DTX_FLOOP_EN;
    MCESD_FIELD dtxFloopEnRing          = F_C28GP4X1R1P0_DTX_FLOOP_RING;
    MCESD_FIELD rxFoffsetDisableLane    = F_C28GP4X1R1P0_RX_FOFFSET_DIS;
    MCESD_FIELD dtxFoffsetSel           = F_C28GP4X1R1P0_DTX_FOFFSET;
    MCESD_FIELD dtxFoffsetSelRing       = F_C28GP4X1R1P0_DTX_FOFFSET_RING;
    MCESD_FIELD rxCkSel                 = F_C28GP4X1R1P0_RX_CK_SEL;
    MCESD_FIELD puLb                    = F_C28GP4X1R1P0_PU_LB;
    MCESD_FIELD puLbDly                 = F_C28GP4X1R1P0_PU_LB_DLY;
    MCESD_FIELD dtlSqDetEn              = F_C28GP4X1R1P0_DTL_SQ_DET_EN;
    MCESD_U32 ckSelData;

    switch (path)
    {
    case C28GP4X1_PATH_LOCAL_ANALOG_LB:
        /* First check which clock is used (LCPLL or Ring PLL) */
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxCkSel, &ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFloopEn, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFloopEnRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFoffsetDisableLane, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFoffsetSel, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFoffsetSelRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtlSqDetEn, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &anaTxToRx, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puLb, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puLbDly, 1));
        break;
    case C28GP4X1_PATH_EXTERNAL:
        /* First check which clock is used (LCPLL or Ring PLL) */
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxCkSel, &ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFloopEn, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFloopEnRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFoffsetDisableLane, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFoffsetSel, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFoffsetSelRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &anaTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puLb, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puLbDly, 0));
        break;
    case C28GP4X1_PATH_FAR_END_LB:
        /* First check which clock is used (LCPLL or Ring PLL) */
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxCkSel, &ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFloopEn, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFloopEnRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFoffsetDisableLane, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFoffsetSel, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtxFoffsetSelRing, ckSelData));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &digRxToTx, 1));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &anaTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puLb, 0));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &puLbDly, 0));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_DATAPATH *path
)
{
    MCESD_FIELD digRxToTx = F_C28GP4X1R1P0_DIG_RX2TX_LPBKEN;
    MCESD_FIELD anaTxToRx = F_C28GP4X1R1P0_ANA_TX2RX_LPBKEN;
    MCESD_FIELD digTxToRx = F_C28GP4X1R1P0_DIG_TX2RX_LPBKEN;
    MCESD_U32 digRxToTxData, anaTxToRxData, digTxToRxData;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &digRxToTx, &digRxToTxData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &anaTxToRx, &anaTxToRxData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &digTxToRx, &digTxToRxData));

    if ((digRxToTxData == 0) && (anaTxToRxData == 0) && (digTxToRxData == 0))
        *path = C28GP4X1_PATH_EXTERNAL;
    else if ((digRxToTxData == 1) && (anaTxToRxData == 0) && (digTxToRxData == 0))
        *path = C28GP4X1_PATH_FAR_END_LB;
    else if ((digRxToTxData == 0) && (anaTxToRxData == 1) && (digTxToRxData == 0))
        *path = C28GP4X1_PATH_LOCAL_ANALOG_LB;
    else
        *path = C28GP4X1_PATH_UNKNOWN;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
)
{
    MCESD_FIELD tsenAdcData = F_C28GP4X1R1P0_TSEN_ADC_DATA;
    MCESD_FIELD tsenAdcMode = F_C28GP4X1R1P0_TSEN_ADC_MODE;
    MCESD_U32 savedMode, adcData;
    MCESD_32 signedValue;

    /* Save current TSEN ADC Mode */
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &tsenAdcMode, &savedMode));

    if (savedMode != 0)
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &tsenAdcMode, 0));

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &tsenAdcData, &adcData));

    signedValue = (MCESD_32)adcData;

    if (signedValue >= 512)
        signedValue -= 1024;

    /* temperature is returned in milli-Celsius */
    *temperature = (signedValue * C28GP4X1_TSENE_GAIN) + C28GP4X1_TSENE_OFFSET;

    /* Restore TSEN ADC Mode, if necesssary */
    if (savedMode != 0)
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &tsenAdcMode, savedMode));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_POLARITY txPolarity,
    IN E_C28GP4X1_POLARITY rxPolarity
)
{
    MCESD_FIELD txdInv = F_C28GP4X1R1P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C28GP4X1R1P0_RXD_INV;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txdInv, txPolarity));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxdInv, rxPolarity));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C28GP4X1_POLARITY *txPolarity,
    OUT E_C28GP4X1_POLARITY *rxPolarity
)
{
    MCESD_FIELD txdInv = F_C28GP4X1R1P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C28GP4X1R1P0_RXD_INV;
    MCESD_U32 txdInvData, rxdInvData;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txdInv, &txdInvData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxdInv, &rxdInvData));

    *txPolarity = (txdInvData == 0) ? C28GP4X1_POLARITY_NORMAL : C28GP4X1_POLARITY_INVERTED;
    *rxPolarity = (rxdInvData == 0) ? C28GP4X1_POLARITY_NORMAL : C28GP4X1_POLARITY_INVERTED;
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 errors
)
{
    MCESD_FIELD addErrEn = F_C28GP4X1R1P0_ADD_ERR_EN;
    MCESD_FIELD addErrNum = F_C28GP4X1R1P0_ADD_ERR_NUM;

    if ((errors < 1) || (errors > 8))
        return MCESD_FAIL;

    /* 0 = 1 error, 1 = 2 errors, ... 7 = 8 errors */
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &addErrNum, errors - 1));

    /* Toggle F_C28GP4X1R1P0_ADD_ERR_EN 0 -> 1 to trigger error injection */
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &addErrEn, 0));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &addErrEn, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C28GP4X1_PATTERN *txPattern,
    IN S_C28GP4X1_PATTERN *rxPattern,
    IN E_C28GP4X1_SATA_LONGSHORT sataLongShort,
    IN E_C28GP4X1_SATA_INITIAL_DISPARITY sataInitialDisparity,
    IN const char *userPattern,
    IN MCESD_U8 userK
)
{
    MCESD_FIELD txPatternSel        = F_C28GP4X1R1P0_PT_TX_PATT_SEL;
    MCESD_FIELD rxPatternSel        = F_C28GP4X1R1P0_PT_RX_PATT_SEL;
    MCESD_FIELD prbsEnc             = F_C28GP4X1R1P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong            = F_C28GP4X1R1P0_PT_SATA_LONG;
    MCESD_FIELD startRd             = F_C28GP4X1R1P0_PT_START_RD;
    MCESD_FIELD userPattern_79_48   = F_C28GP4X1R1P0_PT_USER_PAT_7948;
    MCESD_FIELD userPattern_47_16   = F_C28GP4X1R1P0_PT_USER_PAT_4716;
    MCESD_FIELD userPattern_15_0    = F_C28GP4X1R1P0_PT_USER_PAT_1500;
    MCESD_FIELD userKChar           = F_C28GP4X1R1P0_PT_USER_K_CHAR;
    E_C28GP4X1_DATABUS_WIDTH txWidth;
    E_C28GP4X1_DATABUS_WIDTH rxWidth;

    if (txPattern->pattern == C28GP4X1_PAT_USER)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txPatternSel, (txPattern->enc8B10B == C28GP4X1_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txPatternSel, txPattern->pattern));
    }

    if (rxPattern->pattern == C28GP4X1_PAT_USER)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxPatternSel, (rxPattern->enc8B10B == C28GP4X1_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxPatternSel, rxPattern->pattern));
    }

    if (((txPattern->pattern >= C28GP4X1_PAT_PRBS7) && (txPattern->pattern <= C28GP4X1_PAT_PRBS32)) ||
        ((rxPattern->pattern >= C28GP4X1_PAT_PRBS7) && (rxPattern->pattern <= C28GP4X1_PAT_PRBS32)))
    {
        if ((txPattern->enc8B10B == C28GP4X1_ENC_8B10B_ENABLE) || (rxPattern->enc8B10B == C28GP4X1_ENC_8B10B_ENABLE))
        {
            MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &prbsEnc, 1));
        }
        else
            MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &prbsEnc, 0));
    }

    if ((sataLongShort == C28GP4X1_SATA_LONG) || (sataLongShort == C28GP4X1_SATA_SHORT))
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &sataLong, sataLongShort));

    if ((sataInitialDisparity == C28GP4X1_DISPARITY_NEGATIVE) || (sataInitialDisparity == C28GP4X1_DISPARITY_POSITIVE))
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &startRd, sataInitialDisparity));

    if (strlen(userPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(userPattern, u8Pattern));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &userPattern_79_48, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3])));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &userPattern_47_16, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7])));
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &userPattern_15_0, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9])));
    }

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &userKChar, userK));

    MCESD_ATTEMPT(API_C28GP4X1_GetDataBusWidth(devPtr, &txWidth, &rxWidth));
    if ((C28GP4X1_PAT_JITTER_8T == txPattern->pattern) || (C28GP4X1_PAT_JITTER_4T == txPattern->pattern))
        txWidth = C28GP4X1_DATABUS_32BIT;
    else if ((C28GP4X1_PAT_JITTER_10T == txPattern->pattern) || (C28GP4X1_PAT_JITTER_5T == txPattern->pattern))
        txWidth = C28GP4X1_DATABUS_40BIT;

    if ((C28GP4X1_PAT_JITTER_8T == rxPattern->pattern) || (C28GP4X1_PAT_JITTER_4T == rxPattern->pattern))
        rxWidth = C28GP4X1_DATABUS_32BIT;
    else if ((C28GP4X1_PAT_JITTER_10T == rxPattern->pattern) || (C28GP4X1_PAT_JITTER_5T == rxPattern->pattern))
        rxWidth = C28GP4X1_DATABUS_40BIT;
    MCESD_ATTEMPT(API_C28GP4X1_SetDataBusWidth(devPtr, txWidth, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C28GP4X1_PATTERN *txPattern,
    OUT S_C28GP4X1_PATTERN *rxPattern,
    OUT E_C28GP4X1_SATA_LONGSHORT *sataLongShort,
    OUT E_C28GP4X1_SATA_INITIAL_DISPARITY *sataInitialDisparity,
    OUT char *userPattern,
    OUT MCESD_U8 *userK
)
{
    MCESD_FIELD txPatternSel    = F_C28GP4X1R1P0_PT_TX_PATT_SEL;
    MCESD_FIELD rxPatternSel    = F_C28GP4X1R1P0_PT_RX_PATT_SEL;
    MCESD_FIELD prbsEnc         = F_C28GP4X1R1P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong        = F_C28GP4X1R1P0_PT_SATA_LONG;
    MCESD_FIELD startRd         = F_C28GP4X1R1P0_PT_START_RD;
    MCESD_FIELD userKChar       = F_C28GP4X1R1P0_PT_USER_K_CHAR;
    MCESD_FIELD userPatternFieldArray[] = { F_C28GP4X1R1P0_PT_USER_PAT_7948 , F_C28GP4X1R1P0_PT_USER_PAT_4716 , F_C28GP4X1R1P0_PT_USER_PAT_1500 };
    MCESD_32 userPatternFieldArrayCount = sizeof(userPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 i;
    MCESD_U32 txPatternSelData, rxPatternSelData, prbsEncData, sataLongData, startRdData, userPatternData, userKData;
    MCESD_U8 u8Pattern[10];

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txPatternSel, &txPatternSelData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxPatternSel, &rxPatternSelData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &prbsEnc, &prbsEncData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &sataLong, &sataLongData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &startRd, &startRdData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &userKChar, &userKData));

    if ((txPatternSelData == 1) || (txPatternSelData == 2))
    {
        txPattern->pattern = C28GP4X1_PAT_USER;
        txPattern->enc8B10B = (txPatternSelData == 1) ? C28GP4X1_ENC_8B10B_DISABLE : C28GP4X1_ENC_8B10B_ENABLE;
    }
    else
    {
        txPattern->pattern = (E_C28GP4X1_PATTERN)txPatternSelData;

        if (txPattern->pattern > C28GP4X1_PAT_SATA_LTDP)
            txPattern->enc8B10B = C28GP4X1_ENC_8B10B_ENABLE;
        else if ((txPattern->pattern >= C28GP4X1_PAT_PRBS7) && (txPattern->pattern <= C28GP4X1_PAT_PRBS32))
            txPattern->enc8B10B = (prbsEncData == 0) ? C28GP4X1_ENC_8B10B_DISABLE : C28GP4X1_ENC_8B10B_ENABLE;
        else
            txPattern->enc8B10B = C28GP4X1_ENC_8B10B_DISABLE;
    }

    if ((rxPatternSelData == 1) || (rxPatternSelData == 2))
    {
        rxPattern->pattern = C28GP4X1_PAT_USER;
        rxPattern->enc8B10B = (rxPatternSelData == 1) ? C28GP4X1_ENC_8B10B_DISABLE : C28GP4X1_ENC_8B10B_ENABLE;
    }
    else
    {
        rxPattern->pattern = (E_C28GP4X1_PATTERN)rxPatternSelData;

        if (rxPattern->pattern > C28GP4X1_PAT_SATA_LTDP)
            rxPattern->enc8B10B = C28GP4X1_ENC_8B10B_ENABLE;
        else if ((rxPattern->pattern >= C28GP4X1_PAT_PRBS7) && (rxPattern->pattern <= C28GP4X1_PAT_PRBS32))
            rxPattern->enc8B10B = (prbsEncData == 0) ? C28GP4X1_ENC_8B10B_DISABLE : C28GP4X1_ENC_8B10B_ENABLE;
        else
            rxPattern->enc8B10B = C28GP4X1_ENC_8B10B_DISABLE;
    }

    *sataLongShort = (E_C28GP4X1_SATA_LONGSHORT)sataLongData;
    *sataInitialDisparity = (E_C28GP4X1_SATA_INITIAL_DISPARITY)startRdData;

    for (i = 0; i < userPatternFieldArrayCount; i++)
    {
        MCESD_FIELD userPatternField = userPatternFieldArray[i];
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &userPatternField, &userPatternData));
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

MCESD_STATUS API_C28GP4X1_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C28GP4X1_PATTERN_STATISTICS *statistics
)
{
    MCESD_FIELD ptPass      = F_C28GP4X1R1P0_PT_PASS;
    MCESD_FIELD ptLock      = F_C28GP4X1R1P0_PT_LOCK;
    MCESD_FIELD ptCnt_47_16 = F_C28GP4X1R1P0_PT_CNT_47_16;
    MCESD_FIELD ptCnt_15_0  = F_C28GP4X1R1P0_PT_CNT_15_0;
    MCESD_FIELD errCnt      = F_C28GP4X1R1P0_PT_ERR_CNT;
    MCESD_U32 passData, lockData, ErrData, data_47_16, data_15_0;
    E_C28GP4X1_DATABUS_WIDTH txWidth;
    E_C28GP4X1_DATABUS_WIDTH rxWidth;
    MCESD_U64 frames;

    if (statistics == NULL)
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ptPass, &passData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ptLock, &lockData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ptCnt_47_16, &data_47_16));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &ptCnt_15_0, &data_15_0));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &errCnt, &ErrData));
    MCESD_ATTEMPT(API_C28GP4X1_GetDataBusWidth(devPtr, &txWidth, &rxWidth));

    frames = ((MCESD_U64)data_47_16 << 16) + data_15_0;

    statistics->totalBits = frames * ((rxWidth == C28GP4X1_DATABUS_32BIT) ? 32 : 40);
    statistics->totalErrorBits = ErrData;
    statistics->pass = (passData == 0) ? MCESD_FALSE : MCESD_TRUE;
    statistics->lock = (lockData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptCntRst = F_C28GP4X1R1P0_PT_CNT_RST;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptCntRst, 0));
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptCntRst, 1));
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptCntRst, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptEn            = F_C28GP4X1R1P0_PT_EN;
    MCESD_FIELD ptEnMode        = F_C28GP4X1R1P0_PT_EN_MODE;
    MCESD_FIELD ptPhyReadyForce = F_C28GP4X1R1P0_PT_PHYREADYFORCE;
    MCESD_FIELD ptRst           = F_C28GP4X1R1P0_PT_RST;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptEn, 0));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptEnMode, 2));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptEn, 1));

    /* Reset PHY Test */
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptRst, 0));
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptRst, 1));
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptRst, 0));

    /* Wait 10 ms for CDR to lock... */
    MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 10));

    /* ...before starting pattern checker */
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptPhyReadyForce, 1));
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptEn            = F_C28GP4X1R1P0_PT_EN;
    MCESD_FIELD ptPhyReadyForce = F_C28GP4X1R1P0_PT_PHYREADYFORCE;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ptEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_EOMInit
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD esmPathSel          = F_C28GP4X1R1P0_ESM_PATH_SEL;
    MCESD_FIELD esmDfeAdaptSplrEn   = F_C28GP4X1R1P0_ESM_DFE_SPLR_EN;
    MCESD_FIELD esmEn               = F_C28GP4X1R1P0_ESM_EN;
    MCESD_FIELD eomReady            = F_C28GP4X1R1P0_EOM_READY;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &esmPathSel, 1));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &esmDfeAdaptSplrEn, 0xC));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &esmEn, 1));
    MCESD_ATTEMPT(API_C28GP4X1_PollField(devPtr, &eomReady, 1, 3000));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD esmEn = F_C28GP4X1R1P0_ESM_EN;
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &esmEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_C28GP4X1_EOM_DATA *measurement
)
{
    MCESD_FIELD esmPhase    = F_C28GP4X1R1P0_ESM_PHASE;
    MCESD_FIELD esmVoltage  = F_C28GP4X1R1P0_ESM_VOLTAGE;
    MCESD_FIELD eomDfeCall  = F_C28GP4X1R1P0_DFE_CALL;
    MCESD_FIELD eomVldCntPE = F_C28GP4X1R1P0_EOM_VLD_CNT_P_E;
    MCESD_FIELD eomVldCntPO = F_C28GP4X1R1P0_EOM_VLD_CNT_P_O;
    MCESD_FIELD eomVldCntNE = F_C28GP4X1R1P0_EOM_VLD_CNT_N_E;
    MCESD_FIELD eomVldCntNO = F_C28GP4X1R1P0_EOM_VLD_CNT_N_O;
    MCESD_FIELD eomErrCntPE = F_C28GP4X1R1P0_EOM_ERR_CNT_P_E;
    MCESD_FIELD eomErrCntPO = F_C28GP4X1R1P0_EOM_ERR_CNT_P_O;
    MCESD_FIELD eomErrCntNE = F_C28GP4X1R1P0_EOM_ERR_CNT_N_E;
    MCESD_FIELD eomErrCntNO = F_C28GP4X1R1P0_EOM_ERR_CNT_N_O;
    MCESD_U32 vldCntPEData, vldCntPOData, vldCntNEData, vldCntNOData;
    MCESD_U32 errCntPEData, errCntPOData, errCntNEData, errCntNOData;

    if (measurement == NULL)
        return MCESD_FAIL;

    /* Set Phase & Voltage */
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &esmPhase, phase));
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &esmVoltage, voltage));

    /* Call DFE to collect statistics */
    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &eomDfeCall, 1));
    MCESD_ATTEMPT(API_C28GP4X1_PollField(devPtr, &eomDfeCall, 0, 2000));

    /* Retrieve valid counts */
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomVldCntPE, &vldCntPEData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomVldCntPO, &vldCntPOData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomVldCntNE, &vldCntNEData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomVldCntNO, &vldCntNOData));

    /* Retrieve error counts */
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomErrCntPE, &errCntPEData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomErrCntPO, &errCntPOData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomErrCntNE, &errCntNEData));
    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &eomErrCntNO, &errCntNOData));

    /* Update data structure */
    measurement->phase = phase;
    measurement->voltage = voltage;
    measurement->upperBitCount = vldCntPEData + vldCntPOData;
    measurement->upperBitErrorCount = errCntPEData + errCntPOData;
    measurement->lowerBitCount = vldCntNEData + vldCntNOData;
    measurement->lowerBitErrorCount = errCntNEData + errCntNOData;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
)
{
    MCESD_32 leftEdge = 0, rightEdge = 0, upperVoltage = -64, lowerVoltage = 63;
    MCESD_32 phase, voltage;

    MCESD_ATTEMPT(API_C28GP4X1_EOMInit(devPtr));

    /* find left edge */
    for (phase = 0; phase < 512; phase++)
    {
        S_C28GP4X1_EOM_DATA measurement;
        MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, phase, 0, &measurement));

        if ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0))
        {
            leftEdge = phase; /* found left edge */
            break;
        }
    }

    /* find right edge */
    for (phase = 0; phase > -512; phase--)
    {
        S_C28GP4X1_EOM_DATA measurement;
        MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, phase, 0, &measurement));

        if ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0))
        {
            rightEdge = phase; /* found right edge */
            break;
        }
    }

    /* find upper and lower boundaries */
    for (voltage = 0; voltage < 64; voltage++)
    {
        S_C28GP4X1_EOM_DATA measurement;
        MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, 0, (MCESD_U8)voltage, &measurement));

        if ((upperVoltage == -64) && ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == 63) && ((measurement.lowerBitCount == 0) || (measurement.lowerBitErrorCount > 0)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -64) && (lowerVoltage != 63))
            break;
    }

    *width  = (MCESD_U16) ((leftEdge == rightEdge) ? 0 : (leftEdge - rightEdge) - 1);
    *height = (MCESD_U16) ((upperVoltage == lowerVoltage) ? 0 : (upperVoltage - lowerVoltage) - 1);

    MCESD_ATTEMPT(API_C28GP4X1_EOMFinalize(devPtr));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C28GP4X1_EYE_RAW_PTR eyeRawDataPtr
)
{
    MCESD_16 maxVoltageStep, leftPhase, rightPhase, phaseIdx, voltIdx, voltage, startPt, tmp;
    MCESD_32 phase;
    MCESD_32 upperVoltage = -C28GP4X1_EYE_MAX_VOLT_STEPS;
    MCESD_32 lowerVoltage = C28GP4X1_EYE_MAX_VOLT_STEPS - 1;
    S_C28GP4X1_EOM_DATA measurement;

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_EOMGetEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if (0 == voltageSteps)
        voltageSteps = C28GP4X1_EYE_DEFAULT_VOLT_STEPS;

    if (0 == phaseLevels)
        phaseLevels = C28GP4X1_EYE_DEFAULT_PHASE_LEVEL;

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

    MCESD_ATTEMPT(API_C28GP4X1_EOMInit(devPtr));

    MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, 0, 0, &measurement));

    if (0 != measurement.upperBitErrorCount || 0 != measurement.lowerBitErrorCount)
    {
        MCESD_ATTEMPT(API_C28GP4X1_EOMFinalize(devPtr));
        MCESD_DBG_ERROR("API_C28GP4X1_EOMGetEyeData: BER > 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
    {
        MCESD_ATTEMPT(API_C28GP4X1_EOMFinalize(devPtr));
        MCESD_DBG_ERROR("API_C28GP4X1_EOMGetEyeData: Total Bit Count == 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    /* Sweep Voltage at Phase 0 */
    phaseIdx = phaseLevels;
    for (voltage = 0; voltage < voltageSteps; voltage++)
    {
        MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, 0, (MCESD_U8)voltage, &measurement));

        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

        if ((upperVoltage == -C28GP4X1_EYE_MAX_VOLT_STEPS) && ((0 == measurement.upperBitCount) || (0 < measurement.upperBitErrorCount)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == (C28GP4X1_EYE_MAX_VOLT_STEPS - 1)) && ((0 == measurement.lowerBitCount) || (0 < measurement.lowerBitErrorCount)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -C28GP4X1_EYE_MAX_VOLT_STEPS) && (lowerVoltage != (C28GP4X1_EYE_MAX_VOLT_STEPS - 1)))
            break;
    }

    /* Left Sweep from Phase 1 with Voltage Sweep */
    phaseIdx -= 1;
    startPt = (upperVoltage > -lowerVoltage) ? upperVoltage : lowerVoltage;
    for (phase = 1; phase < phaseLevels; phase++)
    {
        voltage = ((startPt + C28GP4X1_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C28GP4X1_EYE_VOLT_OFFSET);
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, phase, (MCESD_U8)voltage, &measurement));

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
        voltage = ((startPt + C28GP4X1_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C28GP4X1_EYE_VOLT_OFFSET);
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C28GP4X1_EOMMeasPoint(devPtr, phase, (MCESD_U8)voltage, &measurement));

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

    MCESD_ATTEMPT(API_C28GP4X1_EOMFinalize(devPtr));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_EOMPlotEyeData
(
    IN S_C28GP4X1_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
)
{
    MCESD_U16 phaseIndex, voltageIndex, phaseInterval, timeUnit;
    char line[C28GP4X1_EYE_MAX_PHASE_LEVEL * 2 + 2];

    memset(&line, 'X', C28GP4X1_EYE_MAX_PHASE_LEVEL * 2 + 2);

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C28GP4X1_EOMPlotEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if ((0 == voltageSteps) || (voltageSteps > C28GP4X1_EYE_MAX_VOLT_STEPS))
        voltageSteps = C28GP4X1_EYE_DEFAULT_VOLT_STEPS;

    if ((0 == phaseLevels) || (voltageSteps > C28GP4X1_EYE_MAX_PHASE_LEVEL))
        phaseLevels = C28GP4X1_EYE_DEFAULT_PHASE_LEVEL;

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
            else if (C28GP4X1_EYE_LOW_ERROR_THRESH > eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex])
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

MCESD_STATUS API_C28GP4X1_GetDRO
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C28GP4X1_DRO_DATA *freq
)
{
    E_C28GP4X1_REFFREQ refFreq;
    E_C28GP4X1_REFCLK_SEL clkSel;
    MCESD_U8 i;
    MCESD_U32 data;

    if (freq == NULL)
        return MCESD_FAIL;

    /* The counter is based on reference clock. Count period is 1 us or 4 us. */
	/* If reference clock is 20 MHz, set 20 * 4 us - 1 = 79 (dec) to count for 4 us, set 19 for 1 us. */
	/* If reference clock is 62.5 MHz, set 63 * 4 us -1 = 251 (dec) to count for 4 us, set 63 for 1 us. */
    MCESD_ATTEMPT(API_C28GP4X1_GetRefFreq(devPtr, &refFreq, &clkSel));
    switch (refFreq)
    {
    case C28GP4X1_REFFREQ_25MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 99);     /* 25 * 4 - 1 = 99 */
        break;
    case C28GP4X1_REFFREQ_30MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 119);    /* 30 * 4 - 1 = 119 */
        break;
    case C28GP4X1_REFFREQ_40MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 159);    /* 40 * 4 - 1 = 159 */
        break;
    case C28GP4X1_REFFREQ_50MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 199);    /* 50 * 4 - 1 = 199 */
        break;
    case C28GP4X1_REFFREQ_62P25MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 247);    /* 62 * 4 - 1 = 247 */
        break;
    case C28GP4X1_REFFREQ_100MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 399);    /* 100 * 4 - 1 = 399 */
        break;
    case C28GP4X1_REFFREQ_125MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 499);    /* 125 * 4 - 1 = 499 */
        break;
    case C28GP4X1_REFFREQ_156P25MHZ:
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_TIMER, 623);    /* 156 * 4 - 1 = 623 */
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_PCM_EN, 1);
    C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_DRO_EN, 1);
    C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FCLK_EN, 1);

    for (i = 1; i <= 0xA; i++)
    {
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_DRO_SEL, i);
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_START, 1);
        C28GP4X1_POLL_FIELD(devPtr, F_C28GP4X1R1P0_FBC_PLLCAL_CNT_R, 1, 1000);
        C28GP4X1_READ_FIELD(devPtr, F_C28GP4X1R1P0_FBC_PLLCAL_CNT, data);
        freq->dro[i - 1] = (MCESD_U16)data;
        C28GP4X1_WRITE_FIELD(devPtr, F_C28GP4X1R1P0_FBC_CNT_START, 0);
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetTxLocalPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 value
)
{
    MCESD_FIELD localTxPreset = F_C28GP4X1R1P0_LOCAL_TX_PRESET;

    if (value > 0xB)
        return MCESD_FAIL;  /* Invalid value (RESERVED) */

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &localTxPreset, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetTxLocalPreset
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U32 *value
)
{
    MCESD_FIELD localTxPreset = F_C28GP4X1R1P0_LOCAL_TX_PRESET;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &localTxPreset, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetRemotePreset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 value
)
{
    MCESD_FIELD txPresetIndex = F_C28GP4X1R1P0_TX_PRESET_INDEX;

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &txPresetIndex, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetRemotePreset
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U32 *value
)
{
    MCESD_FIELD txPresetIndex = F_C28GP4X1R1P0_TX_PRESET_INDEX;

    MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &txPresetIndex, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_SetCTLEPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_CTLE_PRESET parameter,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD rxFfeForceEn  = F_C28GP4X1R1P0_RXFFE_FORCE_EN;
    MCESD_FIELD rxFfeRes1Sel  = F_C28GP4X1R1P0_RXFFE_RES1_SEL;
    MCESD_FIELD rxFfeCap1Sel  = F_C28GP4X1R1P0_RXFFE_CAP1_SEL;
    MCESD_FIELD rxFfeRes2SelO = F_C28GP4X1R1P0_RXFFE_RES2_SEL_O;
    MCESD_FIELD rxFfeRes2SelE = F_C28GP4X1R1P0_RXFFE_RES2_SEL_E;
    MCESD_FIELD rxFfeCap2SelO = F_C28GP4X1R1P0_RXFFE_CAP2_SEL_O;
    MCESD_FIELD rxFfeCap2SelE = F_C28GP4X1R1P0_RXFFE_CAP2_SEL_E;
    MCESD_FIELD align90RefG0  = F_C28GP4X1R1P0_ALIGN90_REF_G0;


    if (C28GP4X1_CTLE_PRESET_DEFAULT == parameter)
    {
        /* Use Firmware Default values */
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeForceEn, 0));
        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeForceEn, 1));
    switch (parameter)
    {
    case C28GP4X1_CTLE_PRESET_RES1:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeRes1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_CAP1:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeCap1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_RES2_O:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeRes2SelO, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_RES2_E:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeRes2SelE, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_CAP2_O:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeCap2SelO, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_CAP2_E:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &rxFfeCap2SelE, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_PHASE:
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &align90RefG0, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Invalid parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_GetCTLEPreset
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_CTLE_PRESET parameter,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD rxFfeForceEn  = F_C28GP4X1R1P0_RXFFE_FORCE_EN;
    MCESD_FIELD rxFfeRes1Sel  = F_C28GP4X1R1P0_RXFFE_RES1_SEL;
    MCESD_FIELD rxFfeCap1Sel  = F_C28GP4X1R1P0_RXFFE_CAP1_SEL;
    MCESD_FIELD rxFfeRes2SelO = F_C28GP4X1R1P0_RXFFE_RES2_SEL_O;
    MCESD_FIELD rxFfeRes2SelE = F_C28GP4X1R1P0_RXFFE_RES2_SEL_E;
    MCESD_FIELD rxFfeCap2SelO = F_C28GP4X1R1P0_RXFFE_CAP2_SEL_O;
    MCESD_FIELD rxFfeCap2SelE = F_C28GP4X1R1P0_RXFFE_CAP2_SEL_E;
    MCESD_FIELD align90RefG0  = F_C28GP4X1R1P0_ALIGN90_REF_G0;

    *paramValue = 0;

    switch (parameter)
    {
    case C28GP4X1_CTLE_PRESET_DEFAULT:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeForceEn, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_RES1:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeRes1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_CAP1:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeCap1Sel, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_RES2_O:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeRes2SelO, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_RES2_E:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeRes2SelE, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_CAP2_O:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeCap2SelO, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_CAP2_E:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &rxFfeCap2SelE, paramValue));
        break;
    case C28GP4X1_CTLE_PRESET_PHASE:
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, &align90RefG0, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Invalid parameter */
    }

    return MCESD_OK;
}

#endif /* C28GP4X1 */
