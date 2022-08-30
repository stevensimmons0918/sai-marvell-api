/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_12G_PIPE4_1P2V
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_RegRW.h"
#include "mcesdC12GP41P2V_HwCntl.h"
#include "mcesdC12GP41P2V_API.h"
#include "../mcesdUtils.h"
#ifndef AC5_DEV_SUPPORT
#include <stdio.h>
#endif
#include <string.h>
#include <math.h>

#ifdef C12GP41P2V

/* Forward internal function prototypes used only in this module */
static MCESD_STATUS INT_C12GP41P2V_ProgramNewSpeedsSequence(IN MCESD_DEV_PTR devPtr);

static MCESD_STATUS API_C12GP41P2V_EOMSetPhase
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 softwarePhase,
    IN MCESD_32 targetPhase,
    IN MCESD_16 phaseOffset
);

MCESD_STATUS API_C12GP41P2V_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *locked
)
{
    MCESD_FIELD pllLock = F_C12GP41P2VR2P0_PLL_LOCK;
    MCESD_U16 pllData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &pllLock, &pllData));
    *locked = (pllData == 1) ? MCESD_TRUE : MCESD_FALSE;
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
)
{
    MCESD_FIELD pllReadyTx = F_C12GP41P2VR2P0_PLL_READY_TX;
    MCESD_FIELD pllReadyRx = F_C12GP41P2VR2P0_PLL_READY_RX;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &pllReadyTx, &data));
    *txReady = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &pllReadyRx, &data));
    *rxReady = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetCDRLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD cdrLock = F_C12GP41P2VR2P0_CDR_LOCK;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &cdrLock, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_RxInit
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_INIT0, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_PollPin(devPtr, C12GP41P2V_PIN_RX_INIT_DONE0, 1, 5000));
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_INIT0, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{

    MCESD_FIELD txEmPreEn       = F_C12GP41P2VR2P0_G1_TX_PRE_EN;
    MCESD_FIELD txEmPreCtrl     = F_C12GP41P2VR2P0_G1_TX_PRE;
    MCESD_FIELD txEmPeakEn      = F_C12GP41P2VR2P0_G1_TX_PEAK_EN;
    MCESD_FIELD txEmPeakCtrl    = F_C12GP41P2VR2P0_G1_TX_PEAK;
    MCESD_FIELD txEmPostEn      = F_C12GP41P2VR2P0_G1_TX_POST_EN;
    MCESD_FIELD txEmPostCtrl    = F_C12GP41P2VR2P0_G1_TX_POST;

    switch (param)
    {
    case C12GP41P2V_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPreEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPreCtrl, paramValue));
        break;
    case C12GP41P2V_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPeakEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPeakCtrl, paramValue));
        break;
    case C12GP41P2V_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPostEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPostCtrl, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD txEmPreCtrl     = F_C12GP41P2VR2P0_G1_TX_PRE;
    MCESD_FIELD txEmPeakCtrl    = F_C12GP41P2VR2P0_G1_TX_PEAK;
    MCESD_FIELD txEmPostCtrl    = F_C12GP41P2VR2P0_G1_TX_POST;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txEmPreCtrl, &data));
        break;
    case C12GP41P2V_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txEmPeakCtrl, &data));
        break;
    case C12GP41P2V_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txEmPostCtrl, &data));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    *paramValue = (MCESD_U32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD resSel      = F_C12GP41P2VR2P0_RES_SEL;
    MCESD_FIELD resShift    = F_C12GP41P2VR2P0_RES_SHIFT;
    MCESD_FIELD capSel      = F_C12GP41P2VR2P0_CAP_SEL;
    MCESD_FIELD ffeForce    = F_C12GP41P2VR2P0_FFE_SETTING_F;

    switch (param)
    {
    case C12GP41P2V_CTLE_RES_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &resSel, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CTLE_RES_SHIFT:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &resShift, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CTLE_CAP_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &capSel, (MCESD_U16)paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeForce, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD resSel      = F_C12GP41P2VR2P0_ADAPTEDFFE_RES;
    MCESD_FIELD resShift    = F_C12GP41P2VR2P0_RES_SHIFT;
    MCESD_FIELD capSel      = F_C12GP41P2VR2P0_ADAPTEDFFE_CAP;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_CTLE_RES_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &resSel, &data));
        break;
    case C12GP41P2V_CTLE_RES_SHIFT:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &resShift, &data));
        break;
    case C12GP41P2V_CTLE_CAP_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &capSel, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    *paramValue = (MCESD_32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    MCESD_U16 data;

    switch (tap)
    {
    case C12GP41P2V_DFE_DC_S:
        {
            MCESD_FIELD dcS = F_C12GP41P2VR2P0_DFE_DC_S;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dcS, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, dcS.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F0_S:
        {
            MCESD_FIELD f0S = F_C12GP41P2VR2P0_DFE_F0_S;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f0S, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f0S.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F1_S:
        {
            MCESD_FIELD f1S = F_C12GP41P2VR2P0_DFE_F1_S;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f1S, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f1S.totalBits);
        }
        break;
    case C12GP41P2V_DFE_DC_D:
        {
            MCESD_FIELD dcD = F_C12GP41P2VR2P0_DFE_DC_D;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dcD, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, dcD.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F0_D:
        {
            MCESD_FIELD f0D = F_C12GP41P2VR2P0_DFE_F0_D;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f0D, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f0D.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F1_D:
        {
            MCESD_FIELD f1D = F_C12GP41P2VR2P0_DFE_F1_D;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f1D, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f1D.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F2:
    {
        MCESD_FIELD f2 = F_C12GP41P2VR2P0_DFE_F2;

        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f2, &data));
        *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f2.totalBits);
    }
        break;
    case C12GP41P2V_DFE_F3:
        {
            MCESD_FIELD f3 = F_C12GP41P2VR2P0_DFE_F3;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f3, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f3.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F4:
        {
            MCESD_FIELD f4 = F_C12GP41P2VR2P0_DFE_F4;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f4, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f4.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F5:
        {
            MCESD_FIELD f5 = F_C12GP41P2VR2P0_DFE_F5;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f5, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f5.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F6:
        {
            MCESD_FIELD f6 = F_C12GP41P2VR2P0_DFE_F6;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f6, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f6.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F7:
        {
            MCESD_FIELD f7 = F_C12GP41P2VR2P0_DFE_F7;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f7, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f7.totalBits);
        }
        break;
    default:
        return MCESD_FAIL; /* Unsupported DFE tap */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_IVREF, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_IVREF, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_TX0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_TX0, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}


MCESD_STATUS API_C12GP41P2V_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_RX0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_RX0, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_IDLE0, state ? 0 : 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_IDLE0, &pinValue));
    *state = (pinValue == 0) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    E_C12GP41P2V_REFFREQ freq;
    E_C12GP41P2V_REFCLK_SEL clkSel;
    E_C12GP41P2V_SERDES_SPEED speed;
    MCESD_FIELD g1PllIcp = F_C12GP41P2VR2P0_G1_PLL_ICP;

    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_PLL0, state));

    while (MCESD_TRUE == state)
    {
        /* Speed is C12GP41P2V_SERDES_10P3125G */
        MCESD_ATTEMPT(API_C12GP41P2V_GetTxRxBitRate(devPtr, &speed));
        if (C12GP41P2V_SERDES_10P3125G != speed)
            break;

        MCESD_ATTEMPT(API_C12GP41P2V_GetRefFreq(devPtr, &freq, &clkSel));
        if ((C12GP41P2V_REFFREQ_25MHZ != freq) && (C12GP41P2V_REFFREQ_156P25MHZ != freq))
            break;

        if (C12GP41P2V_REFFREQ_25MHZ == freq)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0xC));
            break;
        }

        /* Reference Frequency is C12GP41P2V_REFFREQ_156P25MHZ */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0x7));
        break;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_PLL0, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_MODE, mode));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_PHYMODE *mode
)
{
    MCESD_U16 pinValue;
    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PHY_MODE, &pinValue));
    *mode = (E_C12GP41P2V_PHYMODE)pinValue;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_REFFREQ freq,
    IN E_C12GP41P2V_REFCLK_SEL clkSel
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_REF_FREF_SEL, freq));
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_REFCLK_SEL, clkSel));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_REFFREQ *freq,
    OUT E_C12GP41P2V_REFCLK_SEL *clkSel
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_REF_FREF_SEL, &pinValue));
    *freq = (E_C12GP41P2V_REFFREQ)pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_REFCLK_SEL, &pinValue));
    *clkSel = (E_C12GP41P2V_REFCLK_SEL)pinValue;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SERDES_SPEED speed
)
{
    MCESD_FIELD rxCtleDataRate1Force    = F_C12GP41P2VR2P0_RX_CTLE_1_F;
    MCESD_FIELD rxCtleDataRate2Force    = F_C12GP41P2VR2P0_RX_CTLE_2_F;
    MCESD_FIELD rxCtleDataRate3Force    = F_C12GP41P2VR2P0_RX_CTLE_3_F;
    MCESD_FIELD g1RxCtleDataRate1       = F_C12GP41P2VR2P0_G1_RX_CTLE_1;
    MCESD_FIELD g1RxCtleDataRate2       = F_C12GP41P2VR2P0_G1_RX_CTLE_2;
    MCESD_FIELD g1RxCtleDataRate3       = F_C12GP41P2VR2P0_G1_RX_CTLE_3;
    MCESD_FIELD phyGenMax               = F_C12GP41P2VR2P0_PHY_GEN_MAX;
    CPSS_ADDITION_START
    MCESD_FIELD rxDclk2xEnLane          = F_C12GP41P2VR2P0_RXDCLK_2X_EN_LANE;
    MCESD_FIELD txDclk2xEnLane          = F_C12GP41P2VR2P0_TXDCLK_2X_EN_LANE;
    CPSS_ADDITION_END

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate1Force, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate2Force, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate3Force, 0x0));

    if (C12GP41P2V_SERDES_11P5625G != speed)
    {
        if (C12GP41P2V_SERDES_10P3125G == speed)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate1Force, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate2Force, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate3Force, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate1, 0x6));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate2, 0x6));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate3, 0x6));
        }
        CPSS_ADDITION_START
        if ((C12GP41P2V_SERDES_1P25G == speed) || (C12GP41P2V_SERDES_3P125G == speed))
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDclk2xEnLane, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txDclk2xEnLane, 0x1));
        }
        else
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDclk2xEnLane, 0x0));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txDclk2xEnLane, 0x0));
        }
        CPSS_ADDITION_END
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_TX0, speed));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_RX0, speed));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyGenMax, 0x3));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_TX0, C12GP41P2V_SERDES_10P3125G));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_RX0, C12GP41P2V_SERDES_10P3125G));
        MCESD_ATTEMPT(INT_C12GP41P2V_ProgramNewSpeedsSequence(devPtr));
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_ProgramNewSpeedsSequence
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD txTxregSpeedtrkClkForce     = F_C12GP41P2VR2P0_TX_CLK_FORCE;
    MCESD_FIELD g1TxTxregSpeedtrkClk        = F_C12GP41P2VR2P0_G1_TX_CLK;
    MCESD_FIELD txTxregSpeedtrkDataForce    = F_C12GP41P2VR2P0_TX_DATA_FORCE;
    MCESD_FIELD g1TxTxregSpeedtrkData       = F_C12GP41P2VR2P0_G1_TX_DATA;
    MCESD_FIELD txSpeedDivForce             = F_C12GP41P2VR2P0_TX_SPDIV_FORCE;
    MCESD_FIELD txSpeedDiv                  = F_C12GP41P2VR2P0_TX_SPEED_DIV;
    MCESD_FIELD txVddCalEnForce             = F_C12GP41P2VR2P0_TX_VDD_CAL_F;
    MCESD_FIELD g1TxVddCalEn                = F_C12GP41P2VR2P0_G1_TX_VDDCALEN;

    MCESD_FIELD rxSpeedDivForce             = F_C12GP41P2VR2P0_RX_SPDIV_FORCE;
    MCESD_FIELD rxSpeedDiv                  = F_C12GP41P2VR2P0_RX_SPEED_DIV;
    MCESD_FIELD rxDtlclkDivForce            = F_C12GP41P2VR2P0_RX_DTLCLK_F;
    MCESD_FIELD g1RxDtlclkDiv               = F_C12GP41P2VR2P0_G1_RX_DTLCLK_D;
    MCESD_FIELD rxIntpiForce                = F_C12GP41P2VR2P0_RX_INTPI_FORCE;
    MCESD_FIELD g1RxIntpi                   = F_C12GP41P2VR2P0_G1_RX_INTPI;
    MCESD_FIELD rxIntprForce                = F_C12GP41P2VR2P0_RX_INTPR_FORCE;
    MCESD_FIELD g1RxIntpr                   = F_C12GP41P2VR2P0_G1_RX_INTPR;
    MCESD_FIELD rxEomIntprForce             = F_C12GP41P2VR2P0_RX_EOM_INTPR_F;
    MCESD_FIELD g1RxEomIntpr                = F_C12GP41P2VR2P0_G1_RXEOM_INTPR;
    MCESD_FIELD rxRxregSpeedtrkClkForce     = F_C12GP41P2VR2P0_RX_CLK_FORCE;
    MCESD_FIELD g1RxRxregSpeedtrkClk        = F_C12GP41P2VR2P0_G1_RX_CLK;
    MCESD_FIELD rxRxregSpeedtrkDataForce    = F_C12GP41P2VR2P0_RX_DATA_FORCE;
    MCESD_FIELD g1RxRxregSpeedtrkData       = F_C12GP41P2VR2P0_G1_RX_DATA;
    MCESD_FIELD rxCkpwCntForce              = F_C12GP41P2VR2P0_RX_CKPW_CNT_F;
    MCESD_FIELD g1RxCkpwCnt                 = F_C12GP41P2VR2P0_G1_RX_CKPW_CNT;
    MCESD_FIELD rxPuAlign90Force            = F_C12GP41P2VR2P0_RX_PU_A90_F;
    MCESD_FIELD g1RxPuAlign90               = F_C12GP41P2VR2P0_G1_RX_PU_A90;
    MCESD_FIELD rxCtleDataRate1Force        = F_C12GP41P2VR2P0_RX_CTLE_1_F;
    MCESD_FIELD g1RxCtleDataRate1           = F_C12GP41P2VR2P0_G1_RX_CTLE_1;
    MCESD_FIELD rxCtleDataRate2Force        = F_C12GP41P2VR2P0_RX_CTLE_2_F;
    MCESD_FIELD g1RxCtleDataRate2           = F_C12GP41P2VR2P0_G1_RX_CTLE_2;
    MCESD_FIELD rxCtleDataRate3Force        = F_C12GP41P2VR2P0_RX_CTLE_3_F;
    MCESD_FIELD g1RxCtleDataRate3           = F_C12GP41P2VR2P0_G1_RX_CTLE_3;
    MCESD_FIELD rxVddCalEnForce             = F_C12GP41P2VR2P0_RX_VDD_CAL_F;
    MCESD_FIELD g1RxVddCalEn                = F_C12GP41P2VR2P0_G1_RX_VDDCALEN;
    MCESD_FIELD rxDccCalEnForce             = F_C12GP41P2VR2P0_RX_DCC_CAL_F;
    MCESD_FIELD g1RxDccCalEn                = F_C12GP41P2VR2P0_G1_RX_DCCCA_EN;
    MCESD_FIELD rxclk25MForce               = F_C12GP41P2VR2P0_RX_25M_FORCE;
    MCESD_FIELD anaRxclk25MCtrl             = F_C12GP41P2VR2P0_RX_25M_CTRL;
    MCESD_FIELD anaRxclk25MDiv1p5En         = F_C12GP41P2VR2P0_RX_25M_DIV1P5;
    MCESD_FIELD anaRxclk25MDiv              = F_C12GP41P2VR2P0_RX_25M_DIV;
    MCESD_FIELD anaRxclk25MFixDivEn         = F_C12GP41P2VR2P0_RX_25M_FIX_DIV;

    MCESD_FIELD phyModeFmReg                = F_C12GP41P2VR2P0_PHY_MODE_FMREG;
    MCESD_FIELD phyMode                     = F_C12GP41P2VR2P0_PHY_MODE;
    MCESD_FIELD refFrefSelFmReg             = F_C12GP41P2VR2P0_FREF_SEL_FMREG;
    MCESD_FIELD refFrefSel                  = F_C12GP41P2VR2P0_REF_FREF_SEL;
    MCESD_FIELD pllRefdivForce              = F_C12GP41P2VR2P0_PLL_REFDIV_F;
    MCESD_FIELD pllRefDiv                   = F_C12GP41P2VR2P0_PLL_REFDIV;
    MCESD_FIELD pllFbdivForce               = F_C12GP41P2VR2P0_PLL_FBDIV_F;
    MCESD_FIELD pllFbdiv                    = F_C12GP41P2VR2P0_PLL_FBDIV;
    MCESD_FIELD pllBwSelForce               = F_C12GP41P2VR2P0_PLL_BW_SEL_F;
    MCESD_FIELD g1PllBwSel                  = F_C12GP41P2VR2P0_G1_PLL_BW_SEL;
    MCESD_FIELD pllLpfC2SelForce            = F_C12GP41P2VR2P0_PLL_LPF_C2_F;
    MCESD_FIELD g1PllLpfC2Sel               = F_C12GP41P2VR2P0_G1_PLL_LPF_C2;
    MCESD_FIELD pllIcpForce                 = F_C12GP41P2VR2P0_PLL_ICP_FORCE;
    MCESD_FIELD g1PllIcp                    = F_C12GP41P2VR2P0_G1_PLL_ICP;
    MCESD_FIELD pllSpeedThreshForce         = F_C12GP41P2VR2P0_PLL_SPEED_F;
    MCESD_FIELD pllSpeedThresh              = F_C12GP41P2VR2P0_PLL_SPEED;
    MCESD_FIELD pllFbdivCalForce            = F_C12GP41P2VR2P0_FBDIV_CAL_F;
    MCESD_FIELD pllFbdivCal                 = F_C12GP41P2VR2P0_FBDIV_CAL;
    MCESD_FIELD txintpiForce                = F_C12GP41P2VR2P0_TXINTPI_FORCE;
    MCESD_FIELD r0Txintpi                   = F_C12GP41P2VR2P0_R0_TXINTPI;
    MCESD_FIELD txintprForce                = F_C12GP41P2VR2P0_TXINTPR_FORCE;
    MCESD_FIELD r0Txintpr                   = F_C12GP41P2VR2P0_R0_TXINTPR;
    MCESD_FIELD txdtxclkDivForce            = F_C12GP41P2VR2P0_TXDTXCLK_DIV_F;
    MCESD_FIELD r0TxdtxclkDiv               = F_C12GP41P2VR2P0_R0_TXDTXCLKDIV;
    MCESD_FIELD pllRegSpeedTrkForce         = F_C12GP41P2VR2P0_PLL_REG_FORCE;
    MCESD_FIELD r0PllRegSpeedTrk            = F_C12GP41P2VR2P0_R0_REG_SPD_TRK;
    MCESD_FIELD initTxfoffsForce            = F_C12GP41P2VR2P0_INIT_TXFOFFS_F;
    MCESD_FIELD initTxfoffs                 = F_C12GP41P2VR2P0_INIT_TXFOFFS;
    MCESD_FIELD initRxfoffsForce            = F_C12GP41P2VR2P0_INIT_RXFOFFS_F;
    MCESD_FIELD initRxfoffs                 = F_C12GP41P2VR2P0_INIT_RXFOFFS;
    E_C12GP41P2V_REFFREQ freq;
    E_C12GP41P2V_REFCLK_SEL clkSel;

    /* TX Programming */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTxregSpeedtrkClkForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1TxTxregSpeedtrkClk, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTxregSpeedtrkDataForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1TxTxregSpeedtrkData, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txSpeedDivForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txSpeedDiv, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txVddCalEnForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1TxVddCalEn, 0x1));

    /* RX Programming */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxSpeedDivForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxSpeedDiv, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDtlclkDivForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxDtlclkDiv, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxIntpiForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxIntpi, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxIntprForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxIntpr, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomIntprForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxEomIntpr, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxRxregSpeedtrkClkForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxRxregSpeedtrkClk, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxRxregSpeedtrkDataForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxRxregSpeedtrkData, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCkpwCntForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCkpwCnt, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxPuAlign90Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxPuAlign90, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate1Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate1, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate2Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate2, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate3Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate3, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxVddCalEnForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxVddCalEn, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDccCalEnForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxDccCalEn, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxclk25MForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MCtrl, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MDiv1p5En, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MDiv, 0x73));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MFixDivEn, 0x0));

    /* PLL Programming */
    MCESD_ATTEMPT(API_C12GP41P2V_GetRefFreq(devPtr, &freq, &clkSel));
    if (C12GP41P2V_REFFREQ_25MHZ == freq)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyModeFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyMode, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSelFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSel, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdiv, 0xE7));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllBwSelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllBwSel, 0x2));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllLpfC2SelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllLpfC2Sel, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllIcpForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0xC));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThreshForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThresh, 0xFB));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCalForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCal, 0x17));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintpiForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpi, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintprForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpr, 0x2));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txdtxclkDivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0TxdtxclkDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRegSpeedTrkForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0PllRegSpeedTrk, 0x6));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffs, 0x7CB));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffs, 0x7CB));
    }
    else if (C12GP41P2V_REFFREQ_156P25MHZ == freq)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyModeFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyMode, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSelFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSel, 0xC));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdiv, 0x25));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllBwSelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllBwSel, 0x0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllLpfC2SelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllLpfC2Sel, 0x0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllIcpForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0xB));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThreshForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThresh, 0xFD));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCalForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCal, 0x17));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintpiForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpi, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintprForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpr, 0x2));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txdtxclkDivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0TxdtxclkDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRegSpeedTrkForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0PllRegSpeedTrk, 0x6));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffs, 0x0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffs, 0x0));
    }
    else
    {
        return MCESD_FAIL; /* No sequence for this reference clock */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_SERDES_SPEED *speed
)
{
    MCESD_FIELD anaRxclk25MCtrl             = F_C12GP41P2VR2P0_RX_25M_CTRL;
    MCESD_FIELD anaRxclk25MDiv1p5En         = F_C12GP41P2VR2P0_RX_25M_DIV1P5;
    MCESD_FIELD anaRxclk25MDiv              = F_C12GP41P2VR2P0_RX_25M_DIV;
    MCESD_U16 pinValue, data;

    /* Check ANA_RX_CLK_25M values to see if ProgramNewSpeedsSequence set 11.5625G. */
    while(1)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxclk25MCtrl, &data));
        if (0x2 != data)
            break;
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxclk25MDiv1p5En, &data));
        if (0x0 != data)
            break;
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxclk25MDiv, &data));
        if (0x73 != data)
            break;
        *speed = C12GP41P2V_SERDES_11P5625G;
        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_RX0, &pinValue));
    *speed = (E_C12GP41P2V_SERDES_SPEED)pinValue;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DATABUS_WIDTH width
)
{
    MCESD_FIELD selBits = F_C12GP41P2VR2P0_SEL_BITS;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selBits, width));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_DATABUS_WIDTH *width
)
{
    MCESD_FIELD selBits = F_C12GP41P2VR2P0_SEL_BITS;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selBits, &data));
    *width = (E_C12GP41P2V_DATABUS_WIDTH)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 align90
)
{
    MCESD_FIELD align90Force    = F_C12GP41P2VR2P0_ALIGN90_FORCE;
    MCESD_FIELD align90Ext      = F_C12GP41P2VR2P0_ALIGN90_EXT;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &align90Force, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &align90Ext, align90));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetAlign90(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *align90
)
{
    MCESD_FIELD align90Ref = F_C12GP41P2VR2P0_ALIGN90_REF;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &align90Ref, &data));
    *align90 = data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    IN S_C12GP41P2V_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C12GP41P2VR2P0_TRX_TRAINTIMER;
    MCESD_FIELD rxTrainTimer        = F_C12GP41P2VR2P0_RX_TRAIN_TIMER;
    MCESD_FIELD trxTrainTimerEnable = F_C12GP41P2VR2P0_TRX_TRAIN_T_EN;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trxTrainTimer, training->timeout));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxTrainTimer, training->timeout));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    OUT S_C12GP41P2V_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C12GP41P2VR2P0_TRX_TRAINTIMER;
    MCESD_FIELD rxTrainTimer        = F_C12GP41P2VR2P0_RX_TRAIN_TIMER;
    MCESD_FIELD trxTrainTimerEnable = F_C12GP41P2VR2P0_TRX_TRAIN_T_EN;
    MCESD_U16 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &trxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &trxTrainTimer, &timeoutData));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &trxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxTrainTimer, &timeoutData));
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = timeoutData;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
)
{
    MCESD_FIELD ffeForce = F_C12GP41P2VR2P0_FFE_SETTING_F;
    MCESD_U16 failed;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeForce, 0));

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_PollPin(devPtr, C12GP41P2V_PIN_TX_TRAIN_COMPLETE0, 1, 5000));
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_FAILED0, &failed));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_PollPin(devPtr, C12GP41P2V_PIN_RX_TRAIN_COMPLETE0, 1, 5000));
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_FAILED0, &failed));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
    }

    return (failed == 0) ? MCESD_OK : MCESD_FAIL;
}

MCESD_STATUS API_C12GP41P2V_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
)
{
    MCESD_FIELD ffeForce = F_C12GP41P2VR2P0_FFE_SETTING_F;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeForce, 0));

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0,  0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_PAT_DIS0,     1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0,          1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 1));

    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0,  0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_PAT_DIS0,     1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0,          1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 1));    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U16 completeData, failedData = 0;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_FAILED0, &failedData));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C12GP41P2V_StopTraining(devPtr, type));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_FAILED0, &failedData));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C12GP41P2V_StopTraining(devPtr, type));
        }
    }

    *completed = (completeData == 0) ? MCESD_FALSE : MCESD_TRUE;
    *failed = (failedData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
)
{
    MCESD_U16 enableData;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, &enableData));
        if (1 == enableData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_PAT_DIS0,     0));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, &enableData));
        if (1 == enableData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD selMufi = F_C12GP41P2VR2P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C12GP41P2VR2P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C12GP41P2VR2P0_RX_SELMUPI;
    MCESD_FIELD selMupf = F_C12GP41P2VR2P0_RX_SELMUPF;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMufi, &data));
        break;
    case C12GP41P2V_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMuff, &data));
        break;
    case C12GP41P2V_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMupi, &data));
        break;
    case C12GP41P2V_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMupf, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    *paramValue = (MCESD_U32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD selMufi = F_C12GP41P2VR2P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C12GP41P2VR2P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C12GP41P2VR2P0_RX_SELMUPI;
    MCESD_FIELD selMupf = F_C12GP41P2VR2P0_RX_SELMUPF;

    switch (param)
    {
    case C12GP41P2V_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMufi, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMuff, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMupi, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMupf, (MCESD_U16)paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_EN setting
)
{
    MCESD_FIELD slewRateEn = F_C12GP41P2VR2P0_SLEWRATE_EN;

    switch (setting)
    {
    case C12GP41P2V_SR_DISABLE:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewRateEn, 0));
        break;
    case C12GP41P2V_SR_ENABLE:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewRateEn, 3));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_SLEWRATE_EN *setting
)
{
    MCESD_FIELD slewRateEn = F_C12GP41P2VR2P0_SLEWRATE_EN;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &slewRateEn, &data));
    *setting = (E_C12GP41P2V_SLEWRATE_EN)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C12GP41P2VR2P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C12GP41P2VR2P0_SLEWCTRL1;

    switch (param)
    {
    case C12GP41P2V_SR_CTRL0:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewCtrl0, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_SR_CTRL1:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewCtrl1, (MCESD_U16)paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C12GP41P2VR2P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C12GP41P2VR2P0_SLEWCTRL1;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_SR_CTRL0:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &slewCtrl0, &data));
        break;
    case C12GP41P2V_SR_CTRL1:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &slewCtrl1, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    *paramValue = (MCESD_U32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_U16 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_SQ_DETECTED_LPF0, &pinValue));
    *squelched = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_16 threshold
)
{
    MCESD_FIELD sqThresh = F_C12GP41P2VR2P0_SQ_THRESH;

    if ((threshold < C12GP41P2V_SQ_THRESH_MIN) || (threshold > C12GP41P2V_SQ_THRESH_MAX))
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &sqThresh, threshold + 0x10));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_16 *threshold
)
{
    MCESD_FIELD sqThresh = F_C12GP41P2VR2P0_SQ_THRESH;
    MCESD_U16 value;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &sqThresh, &value));
    if (value >= 0x10)
        value -= 0x10;

    *threshold = value;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DATAPATH path
)
{
    MCESD_FIELD digRxToTx   = F_C12GP41P2VR2P0_DIG_RX2TX_EN;
    MCESD_FIELD digTxToRx   = F_C12GP41P2VR2P0_DIG_TX2RX_EN;
    MCESD_FIELD cmnFarend   = F_C12GP41P2VR2P0_CMN_FAREND_EN;
    MCESD_FIELD rxFarend    = F_C12GP41P2VR2P0_RX_FAREND_EN;
    MCESD_FIELD dtlSqDetEn  = F_C12GP41P2VR2P0_DTL_SQ_DET_EN;
    MCESD_FIELD anaRxPu     = F_C12GP41P2VR2P0_ANA_PU_LB;
    MCESD_FIELD anaRxPuDly  = F_C12GP41P2VR2P0_ANA_PU_LB_DLY;

    switch (path)
    {
    case C12GP41P2V_PATH_EXTERNAL:
        /* Enable C12GP41P2V_PATH_EXTERNAL */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digTxToRx, 0));
        /* Disable C12GP41P2V_PATH_LOCAL_ANALOG_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPu, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPuDly, 0));
        /* Disable C12GP41P2V_PATH_FAR_END_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cmnFarend, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxFarend, 0));
        break;
    case C12GP41P2V_PATH_FAR_END_LB:
        /* Enable C12GP41P2V_PATH_FAR_END_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digRxToTx, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cmnFarend, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxFarend, 1));
        /* Disable C12GP41P2V_PATH_LOCAL_ANALOG_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPu, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPuDly, 0));
        break;
    case C12GP41P2V_PATH_LOCAL_ANALOG_LB:
        /* Enable C12GP41P2V_PATH_LOCAL_ANALOG_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dtlSqDetEn, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPu, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPuDly, 1));
        /* Disable C12GP41P2V_PATH_FAR_END_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cmnFarend, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxFarend, 0));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_DATAPATH *path
)
{
    MCESD_FIELD digRxToTx   = F_C12GP41P2VR2P0_DIG_RX2TX_EN;
    MCESD_FIELD digTxToRx   = F_C12GP41P2VR2P0_DIG_TX2RX_EN;
    MCESD_FIELD cmnFarend   = F_C12GP41P2VR2P0_CMN_FAREND_EN;
    MCESD_FIELD rxFrarend   = F_C12GP41P2VR2P0_RX_FAREND_EN;
    MCESD_FIELD dtlSqDetEn  = F_C12GP41P2VR2P0_DTL_SQ_DET_EN;
    MCESD_FIELD anaRxPu     = F_C12GP41P2VR2P0_ANA_PU_LB;
    MCESD_FIELD anaRxPuDly  = F_C12GP41P2VR2P0_ANA_PU_LB_DLY;
    MCESD_U16 digRxToTxData, digTxToRxData, cmnFarendData, rxFarendData, dtlSqDetEnData, anaRxPuData, anaRxPuDlyData;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &digRxToTx, &digRxToTxData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &digTxToRx, &digTxToRxData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &cmnFarend, &cmnFarendData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxFrarend, &rxFarendData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dtlSqDetEn, &dtlSqDetEnData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxPu, &anaRxPuData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxPuDly, &anaRxPuDlyData));

    if ((digRxToTxData == 0) && (digTxToRxData == 0))
    {
        if ((0 == dtlSqDetEnData ) && (1 == anaRxPuData) && (1 == anaRxPuDlyData))
            *path = C12GP41P2V_PATH_LOCAL_ANALOG_LB;
        else
            *path = C12GP41P2V_PATH_EXTERNAL;
    }
    else if ((digRxToTxData == 1) && (digTxToRxData == 0) && (cmnFarendData == 1) && (rxFarendData == 1))
    {
        *path = C12GP41P2V_PATH_FAR_END_LB;
    }
    else
    {
        *path = C12GP41P2V_PATH_UNKNOWN;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_POLARITY txPolarity,
    IN E_C12GP41P2V_POLARITY rxPolarity
)
{
    MCESD_FIELD txdInv = F_C12GP41P2VR2P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C12GP41P2VR2P0_RXD_INV;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txdInv, txPolarity));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxdInv, rxPolarity));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_POLARITY *txPolarity,
    OUT E_C12GP41P2V_POLARITY *rxPolarity
)
{
    MCESD_FIELD txdInv = F_C12GP41P2VR2P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C12GP41P2VR2P0_RXD_INV;
    MCESD_U16 txdInvData, rxdInvData;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txdInv, &txdInvData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxdInv, &rxdInvData));

    *txPolarity = (txdInvData == 0) ? C12GP41P2V_POLARITY_NORMAL : C12GP41P2V_POLARITY_INVERTED;
    *rxPolarity = (rxdInvData == 0) ? C12GP41P2V_POLARITY_NORMAL : C12GP41P2V_POLARITY_INVERTED;
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 errors
)
{
    MCESD_FIELD addErrEn    = F_C12GP41P2VR2P0_ADD_ERR_EN;
    MCESD_FIELD addErrNum   = F_C12GP41P2VR2P0_ADD_ERR_NUM;

    if ((errors < 1) || (errors > 8))
        return MCESD_FAIL;

    /* 0 = 1 error, 1 = 2 errors, ... 7 = 8 errors */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &addErrNum, errors - 1));

    /* Toggle F_C12GP41P2VR2P0_ADD_ERR_EN 0 -> 1 to trigger error injection */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &addErrEn, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &addErrEn, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_PATTERN *txPattern,
    IN S_C12GP41P2V_PATTERN *rxPattern,
    IN E_C12GP41P2V_SATA_LONGSHORT sataLongShort,
    IN E_C12GP41P2V_SATA_INITIAL_DISPARITY sataInitialDisparity,
    IN const char *userPattern,
    IN MCESD_U8 userK
)
{
    MCESD_FIELD txPatternSel        = F_C12GP41P2VR2P0_PT_TX_PAT_SEL;
    MCESD_FIELD rxPatternSel        = F_C12GP41P2VR2P0_PT_RX_PAT_SEL;
    MCESD_FIELD prbsEnc             = F_C12GP41P2VR2P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong            = F_C12GP41P2VR2P0_PT_SATA_LONG;
    MCESD_FIELD startRd             = F_C12GP41P2VR2P0_PT_START_RD;
    MCESD_FIELD userPattern_79_64   = F_C12GP41P2VR2P0_USER_PAT_79_64;
    MCESD_FIELD userPattern_63_48   = F_C12GP41P2VR2P0_USER_PAT_63_48;
    MCESD_FIELD userPattern_47_32   = F_C12GP41P2VR2P0_USER_PAT_47_32;
    MCESD_FIELD userPattern_31_16   = F_C12GP41P2VR2P0_USER_PAT_31_16;
    MCESD_FIELD userPattern_15_0    = F_C12GP41P2VR2P0_USER_PAT_15_0;
    MCESD_FIELD userKChar           = F_C12GP41P2VR2P0_USER_K_CHAR;

    if (txPattern->pattern == C12GP41P2V_PAT_USER)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txPatternSel, (txPattern->enc8B10B == C12GP41P2V_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txPatternSel, txPattern->pattern));
    }

    if (rxPattern->pattern == C12GP41P2V_PAT_USER)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxPatternSel, (rxPattern->enc8B10B == C12GP41P2V_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxPatternSel, rxPattern->pattern));
    }

    if (((txPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (txPattern->pattern <= C12GP41P2V_PAT_PRBS32)) ||
        ((rxPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (rxPattern->pattern <= C12GP41P2V_PAT_PRBS32)))
    {
        if ((txPattern->enc8B10B == C12GP41P2V_ENC_8B10B_ENABLE) || (rxPattern->enc8B10B == C12GP41P2V_ENC_8B10B_ENABLE))
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &prbsEnc, 1));
        }
        else
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &prbsEnc, 0));
    }

    if ((sataLongShort == C12GP41P2V_SATA_LONG) || (sataLongShort == C12GP41P2V_SATA_SHORT))
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &sataLong, sataLongShort));

    if ((sataInitialDisparity == C12GP41P2V_DISPARITY_NEGATIVE) || (sataInitialDisparity == C12GP41P2V_DISPARITY_POSITIVE))
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &startRd, sataInitialDisparity));

    if (strlen(userPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(userPattern, u8Pattern));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_79_64, MAKEU16FROMU8(u8Pattern[0], u8Pattern[1])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_63_48, MAKEU16FROMU8(u8Pattern[2], u8Pattern[3])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_47_32, MAKEU16FROMU8(u8Pattern[4], u8Pattern[5])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_31_16, MAKEU16FROMU8(u8Pattern[6], u8Pattern[7])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_15_0, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9])));
    }

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userKChar, userK));

    if ((C12GP41P2V_PAT_JITTER_8T == txPattern->pattern) || (C12GP41P2V_PAT_JITTER_4T == txPattern->pattern))
    {
        MCESD_ATTEMPT(API_C12GP41P2V_SetDataBusWidth(devPtr, C12GP41P2V_DATABUS_32BIT));
    }
    else if ((C12GP41P2V_PAT_JITTER_10T == txPattern->pattern) || (C12GP41P2V_PAT_JITTER_5T == txPattern->pattern))
    {
        MCESD_ATTEMPT(API_C12GP41P2V_SetDataBusWidth(devPtr, C12GP41P2V_DATABUS_40BIT));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_PATTERN *txPattern,
    OUT S_C12GP41P2V_PATTERN *rxPattern,
    OUT E_C12GP41P2V_SATA_LONGSHORT *sataLongShort,
    OUT E_C12GP41P2V_SATA_INITIAL_DISPARITY *sataInitialDisparity,
    OUT char *userPattern,
    OUT MCESD_U8 *userK
)
{
    MCESD_FIELD txPatternSel    = F_C12GP41P2VR2P0_PT_TX_PAT_SEL;
    MCESD_FIELD rxPatternSel    = F_C12GP41P2VR2P0_PT_RX_PAT_SEL;
    MCESD_FIELD prbsEnc         = F_C12GP41P2VR2P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong        = F_C12GP41P2VR2P0_PT_SATA_LONG;
    MCESD_FIELD startRd         = F_C12GP41P2VR2P0_PT_START_RD;
    MCESD_FIELD userKChar       = F_C12GP41P2VR2P0_USER_K_CHAR;
    MCESD_FIELD userPatternFieldArray[] = { F_C12GP41P2VR2P0_USER_PAT_79_64, F_C12GP41P2VR2P0_USER_PAT_63_48, F_C12GP41P2VR2P0_USER_PAT_47_32, F_C12GP41P2VR2P0_USER_PAT_31_16, F_C12GP41P2VR2P0_USER_PAT_15_0 };
    MCESD_32 userPatternFieldArrayCount = sizeof(userPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 i;
    MCESD_U16 txPatternSelData, rxPatternSelData, prbsEncData, sataLongData, startRdData, userPatternData, userKData;
    MCESD_U8 u8Pattern[10];

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txPatternSel, &txPatternSelData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxPatternSel, &rxPatternSelData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &prbsEnc, &prbsEncData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &sataLong, &sataLongData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &startRd, &startRdData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &userKChar, &userKData));

    if ((txPatternSelData == 1) || (txPatternSelData == 2))
    {
        txPattern->pattern = C12GP41P2V_PAT_USER;
        txPattern->enc8B10B = (txPatternSelData == 1) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
    }
    else
    {
        txPattern->pattern = (E_C12GP41P2V_PATTERN)txPatternSelData;

        if (txPattern->pattern > C12GP41P2V_PAT_SATA_LTDP)
            txPattern->enc8B10B = C12GP41P2V_ENC_8B10B_ENABLE;
        else if ((txPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (txPattern->pattern <= C12GP41P2V_PAT_PRBS32))
            txPattern->enc8B10B = (prbsEncData == 0) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
        else
            txPattern->enc8B10B = C12GP41P2V_ENC_8B10B_DISABLE;
    }

    if ((rxPatternSelData == 1) || (rxPatternSelData == 2))
    {
        rxPattern->pattern = C12GP41P2V_PAT_USER;
        rxPattern->enc8B10B = (rxPatternSelData == 1) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
    }
    else
    {
        rxPattern->pattern = (E_C12GP41P2V_PATTERN)rxPatternSelData;

        if (rxPattern->pattern > C12GP41P2V_PAT_SATA_LTDP)
            rxPattern->enc8B10B = C12GP41P2V_ENC_8B10B_ENABLE;
        else if ((rxPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (rxPattern->pattern <= C12GP41P2V_PAT_PRBS32))
            rxPattern->enc8B10B = (prbsEncData == 0) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
        else
            rxPattern->enc8B10B = C12GP41P2V_ENC_8B10B_DISABLE;
    }

    *sataLongShort = (E_C12GP41P2V_SATA_LONGSHORT)sataLongData;
    *sataInitialDisparity = (E_C12GP41P2V_SATA_INITIAL_DISPARITY)startRdData;

    for (i = 0; i < userPatternFieldArrayCount; i++)
    {
        MCESD_FIELD userPatternField = userPatternFieldArray[i];
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &userPatternField, &userPatternData));
        u8Pattern[i * 2 + 0] = (MCESD_U8)(userPatternData >> 8);
        u8Pattern[i * 2 + 1] = (MCESD_U8)(userPatternData & 0xFF);
    }

    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, userPattern));

    *userK = (MCESD_U8)(userKData & 0xFF);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_PATTERN_STATISTICS *statistics
)
{
    MCESD_FIELD ptPass          = F_C12GP41P2VR2P0_PT_PASS;
    MCESD_FIELD ptLock          = F_C12GP41P2VR2P0_PT_LOCK;
    MCESD_FIELD ptCnt_47_32     = F_C12GP41P2VR2P0_PT_CNT_47_32;
    MCESD_FIELD ptCnt_31_16     = F_C12GP41P2VR2P0_PT_CNT_31_16;
    MCESD_FIELD ptCnt_15_0      = F_C12GP41P2VR2P0_PT_CNT_15_0;
    MCESD_FIELD errCnt_31_16    = F_C12GP41P2VR2P0_ERR_CNT_31_16;
    MCESD_FIELD errCnt_15_0     = F_C12GP41P2VR2P0_ERR_CNT_15_0;
    MCESD_U16 passData, lockData, errData_31_16, errData_15_0, data_47_32, data_31_16, data_15_0;
    E_C12GP41P2V_DATABUS_WIDTH width;
    MCESD_32 widthBit;
    MCESD_U64 frames;

    if (statistics == NULL)
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptPass, &passData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptLock, &lockData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptCnt_47_32, &data_47_32));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptCnt_31_16, &data_31_16));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptCnt_15_0, &data_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &errCnt_31_16, &errData_31_16));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &errCnt_15_0, &errData_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_GetDataBusWidth(devPtr, &width));

    frames = ((MCESD_U64)data_47_32 << 32) + ((MCESD_U64)data_31_16 << 16) + data_15_0;

    switch (width)
    {
    case C12GP41P2V_DATABUS_16BIT:
        widthBit = 16;
        break;
    case C12GP41P2V_DATABUS_20BIT:
        widthBit = 20;
        break;
    case C12GP41P2V_DATABUS_32BIT:
        widthBit = 32;
        break;
    case C12GP41P2V_DATABUS_40BIT:
        widthBit = 40;
        break;
    default:
        widthBit = 16;
        break;
    }

    statistics->totalBits = frames * widthBit;
    statistics->totalErrorBits = ((MCESD_U64)errData_31_16 << 16) + (MCESD_U64)errData_15_0;
    statistics->pass = (passData == 0) ? MCESD_FALSE : MCESD_TRUE;
    statistics->lock = (lockData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptCntRst = F_C12GP41P2VR2P0_PT_CNT_RST;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptCntRst, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptCntRst, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptCntRst, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptEn            = F_C12GP41P2VR2P0_PT_EN;
    MCESD_FIELD ptEnMode        = F_C12GP41P2VR2P0_PT_EN_MODE;
    MCESD_FIELD ptPhyReadyForce = F_C12GP41P2VR2P0_PT_PHYREADY_F;
    MCESD_FIELD ptRst           = F_C12GP41P2VR2P0_PT_RST;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEn, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEnMode, 2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEn, 1));

    /* Reset PHY Test */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptRst, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptRst, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptRst, 0));

    /* Wait 10 ms for CDR to lock... */
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 10));

    /* ...before starting pattern checker */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptPhyReadyForce, 1));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptEn            = F_C12GP41P2VR2P0_PT_EN;
    MCESD_FIELD ptPhyReadyForce = F_C12GP41P2VR2P0_PT_PHYREADY_F;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_EOM_SAVED *savedFields,
    OUT MCESD_16 *phaseOffset
)
{
    MCESD_FIELD skipNormEomF1Align  = F_C12GP41P2VR2P0_SKIP_F1_ALIGN;
    MCESD_FIELD skipNormPhAdapt     = F_C12GP41P2VR2P0_SKIP_PH;
    MCESD_FIELD skipNormDfeAccu     = F_C12GP41P2VR2P0_SKIP_DFE_ACCU;
    MCESD_FIELD eomEn               = F_C12GP41P2VR2P0_EOM_EN;
    MCESD_FIELD eomReady            = F_C12GP41P2VR2P0_EOM_READY;
    MCESD_FIELD normUseS            = F_C12GP41P2VR2P0_NORM_USE_S;
    MCESD_FIELD normUseD            = F_C12GP41P2VR2P0_NORM_USE_D;
    MCESD_FIELD dfeF0SSM            = F_C12GP41P2VR2P0_DFE_F0_S;
    MCESD_FIELD dfeAdaptSplrEn      = F_C12GP41P2VR2P0_DFE_SPLR_EN;
    MCESD_FIELD dfeAdaptLpnum       = F_C12GP41P2VR2P0_DFE_LPNUM;
    MCESD_FIELD phCtrlModeForce     = F_C12GP41P2VR2P0_PH_CTRL_FORCE;
    MCESD_FIELD phCtrlModeExt       = F_C12GP41P2VR2P0_PH_CTRL_EXT;
    MCESD_FIELD rxEomEnDForce       = F_C12GP41P2VR2P0_EOM_EN_D_FORCE;
    MCESD_FIELD rxEomEnDExt         = F_C12GP41P2VR2P0_EOM_EN_D_EXT;
    MCESD_FIELD rxEomEnSForce       = F_C12GP41P2VR2P0_EOM_EN_S_FORCE;
    MCESD_FIELD rxEomEnSExt         = F_C12GP41P2VR2P0_EOM_EN_S_EXT;
    MCESD_FIELD dfeCdsBypass        = F_C12GP41P2VR2P0_DFE_CDS_BYPASS;
    MCESD_FIELD rxEomPhOffsetForce  = F_C12GP41P2VR2P0_EOM_PH_FORCE;
    MCESD_FIELD eomAlignCalStart    = F_C12GP41P2VR2P0_EOM_ALIGNSTART;
    MCESD_FIELD eomAlignCalDone     = F_C12GP41P2VR2P0_EOM_ALIGN_DONE;
    MCESD_FIELD cdsDoneIsr          = F_C12GP41P2VR2P0_CDS_DONE_ISR;
    MCESD_FIELD eomUiAlignStart     = F_C12GP41P2VR2P0_UI_ALIGN_START;
    MCESD_FIELD eomUiAlignDone      = F_C12GP41P2VR2P0_UI_ALIGN_DONE;
    MCESD_FIELD rxEomPhOffset       = F_C12GP41P2VR2P0_EOM_PH_OFFSET;
    MCESD_FIELD dfePolLpnum         = F_C12GP41P2VR2P0_DFE_POL_LPNUM;
    MCESD_U16 fieldData;

    /* Skip Real Time Phase Adaption */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &skipNormEomF1Align, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &skipNormPhAdapt, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &skipNormDfeAccu, 1));

    /* Enable Eye Monitor */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomEn, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomEn, 1));

    /* Wait for EOM_READY */
    MCESD_ATTEMPT(API_C12GP41P2V_PollField(devPtr, &eomReady, 1, 3000));

    /* Save Fields */
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &skipNormEomF1Align, &fieldData));
    savedFields->skipNormEomF1Align = fieldData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &skipNormPhAdapt, &fieldData));
    savedFields->skipNormPhAdapt = fieldData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &skipNormDfeAccu, &fieldData));
    savedFields->skipNormDfeAccu = fieldData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &normUseS, &fieldData));
    savedFields->normUseS = fieldData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &normUseD, &fieldData));
    savedFields->normUseD = fieldData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dfeF0SSM, &fieldData));
    savedFields->dfeF0SSM = fieldData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dfeAdaptSplrEn, &fieldData));
    savedFields->dfeAdaptSplrEn = fieldData;

    /* Set Path */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &normUseS, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &normUseD, 0));

    /* Set DFE Samplers */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeAdaptSplrEn, 2));

    /* Set Sampler Loop Count for DFE */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfePolLpnum, 0x20));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeAdaptLpnum, 0x40));

    /* Select EOM Clock and Phase Control Mode */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phCtrlModeForce, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phCtrlModeExt, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomEnSForce, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomEnDForce, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomEnSExt, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomEnDExt, 0));

    /* Alignment Calibration */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeCdsBypass, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomPhOffsetForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomAlignCalStart, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomAlignCalStart, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_PollField(devPtr, &eomAlignCalDone, 1, 3000));

    /* UI Alignment */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cdsDoneIsr, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomUiAlignStart, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomUiAlignStart, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_PollField(devPtr, &cdsDoneIsr, 1, 3000));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cdsDoneIsr, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_PollField(devPtr, &eomUiAlignDone, 1, 3000));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeCdsBypass, 1));

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxEomPhOffset, &fieldData));
    *phaseOffset = ConvertTwosComplementToI32(fieldData, 5);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_EOM_SAVED savedFields,
    IN MCESD_32 softwarePhase,
    IN MCESD_16 phaseOffset
)
{
    MCESD_FIELD dfeAdaptSplrEn      = F_C12GP41P2VR2P0_DFE_SPLR_EN;
    MCESD_FIELD rxEomPhOffsetForce  = F_C12GP41P2VR2P0_EOM_PH_FORCE;
    MCESD_FIELD dfeF0SExt           = F_C12GP41P2VR2P0_DFE_F0_S_EXT;
    MCESD_FIELD dfeFxForce          = F_C12GP41P2VR2P0_DFE_FX_FORCE;
    MCESD_FIELD normUseS            = F_C12GP41P2VR2P0_NORM_USE_S;
    MCESD_FIELD normUseD            = F_C12GP41P2VR2P0_NORM_USE_D;
    MCESD_FIELD skipNormEomF1Align  = F_C12GP41P2VR2P0_SKIP_F1_ALIGN;
    MCESD_FIELD skipNormPhAdapt     = F_C12GP41P2VR2P0_SKIP_PH;
    MCESD_FIELD skipNormDfeAccu     = F_C12GP41P2VR2P0_SKIP_DFE_ACCU;
    MCESD_FIELD dfeCdsByPass        = F_C12GP41P2VR2P0_DFE_CDS_BYPASS;
    MCESD_FIELD phCtrlModeForce     = F_C12GP41P2VR2P0_PH_CTRL_FORCE;
    MCESD_FIELD phCtrlModeExt       = F_C12GP41P2VR2P0_PH_CTRL_EXT;
    MCESD_FIELD rxEomEnDForce       = F_C12GP41P2VR2P0_EOM_EN_D_FORCE;
    MCESD_FIELD rxEomEnSForce       = F_C12GP41P2VR2P0_EOM_EN_S_FORCE;
    MCESD_FIELD eomEn               = F_C12GP41P2VR2P0_EOM_EN;

    /* Restore DFE Adaptation Samplers */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeAdaptSplrEn, savedFields.dfeAdaptSplrEn));

    /* Disable Phase Offset */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomPhOffsetForce, 0));

    /* Restore Voltage */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeF0SExt, savedFields.dfeF0SSM));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeFxForce, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeFxForce, 0));

    /* Restore Saved Fields*/
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &normUseS, savedFields.normUseS));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &normUseD, savedFields.normUseD));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &skipNormEomF1Align, savedFields.skipNormEomF1Align));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &skipNormPhAdapt, savedFields.skipNormPhAdapt));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &skipNormDfeAccu, savedFields.skipNormDfeAccu));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeCdsByPass, 0));

    /* Restore EOM Phase to 0 */
    MCESD_ATTEMPT(API_C12GP41P2V_EOMSetPhase(devPtr, softwarePhase, 0, phaseOffset));

    /* Restore Phase Control Mode */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phCtrlModeForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phCtrlModeExt, 0));

    /* Select Data Clock for Slicer Sampler */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomEnDForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomEnSForce, 0));

    /* Disable EOM Monitor */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    IN MCESD_32 softwarePhase,
    IN MCESD_16 phaseOffset,
    OUT S_C12GP41P2V_EOM_DATA *measurement
)
{
    MCESD_FIELD dfeF0SExt           = F_C12GP41P2VR2P0_DFE_F0_S_EXT;
    MCESD_FIELD dfeFxForce          = F_C12GP41P2VR2P0_DFE_FX_FORCE;
    MCESD_FIELD eomCntClr           = F_C12GP41P2VR2P0_EOM_CNT_CLR;
    MCESD_FIELD dfeStart            = F_C12GP41P2VR2P0_DFE_START;
    MCESD_FIELD dfeDone             = F_C12GP41P2VR2P0_DFE_DONE;
    MCESD_FIELD eomVldCntP_15_0     = F_C12GP41P2VR2P0_VLDCNT_P_15_0;
    MCESD_FIELD eomVldCntP_31_16    = F_C12GP41P2VR2P0_VLDCNT_P_31_16;
    MCESD_FIELD eomVldCntN_15_0     = F_C12GP41P2VR2P0_VLDCNT_N_15_0;
    MCESD_FIELD eomVldCntN_31_16    = F_C12GP41P2VR2P0_VLDCNT_N_31_16;
    MCESD_FIELD eomErrCntP_15_0     = F_C12GP41P2VR2P0_ERRCNT_P_15_0;
    MCESD_FIELD eomErrCntP_31_16    = F_C12GP41P2VR2P0_ERRCNT_P_31_16;
    MCESD_FIELD eomErrCntN_15_0     = F_C12GP41P2VR2P0_ERRCNT_N_15_0;
    MCESD_FIELD eomErrCntN_31_16    = F_C12GP41P2VR2P0_ERRCNT_N_31_16;
    MCESD_U16 vldCntPData_15_0, vldCntPData_31_16, vldCntNData_15_0, vldCntNData_31_16;
    MCESD_U16 errCntPData_15_0, errCntPData_31_16, errCntNData_15_0, errCntNData_31_16;

    /* Set phase */
    MCESD_ATTEMPT(API_C12GP41P2V_EOMSetPhase(devPtr, softwarePhase, phase, phaseOffset));

    /* Set voltage */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeF0SExt, voltage));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeFxForce, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeFxForce, 0));

    /* Clear Counters */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomCntClr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eomCntClr, 0));

    /* Call DFE to collect statistics */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeStart, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_PollField(devPtr, &dfeDone, 1, 3000));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeStart, 0));

    /* Retrieve valid counts */
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomVldCntP_15_0, &vldCntPData_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomVldCntP_31_16, &vldCntPData_31_16));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomVldCntN_15_0, &vldCntNData_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomVldCntN_31_16, &vldCntNData_31_16));

    /* Retrieve error counts */
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomErrCntP_15_0, &errCntPData_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomErrCntP_31_16, &errCntPData_31_16));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomErrCntN_15_0, &errCntNData_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &eomErrCntN_31_16, &errCntNData_31_16));

    /* Update data structure */
    measurement->phase = phase;
    measurement->voltage = voltage;
    measurement->upperBitCount = MAKEU32FROMU16(vldCntPData_31_16, vldCntPData_31_16);
    measurement->upperBitErrorCount = MAKEU32FROMU16(errCntPData_31_16, errCntPData_15_0);
    measurement->lowerBitCount = MAKEU32FROMU16(vldCntNData_31_16, vldCntNData_15_0);
    measurement->lowerBitErrorCount = MAKEU32FROMU16(errCntNData_31_16, errCntNData_15_0);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
)
{
    MCESD_32 leftEdge = -256, rightEdge = 256, upperVoltage = -64, lowerVoltage = 63;
    MCESD_32 centerPhase = 0;
    MCESD_32 softwarePhase = 0;
    MCESD_32 phase, voltage;
    S_C12GP41P2V_EOM_DATA measurement;
    S_C12GP41P2V_EOM_SAVED savedFields;
    MCESD_16 phaseOffset;

    MCESD_ATTEMPT(API_C12GP41P2V_EOMInit(devPtr, &savedFields, &phaseOffset));

    MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, 0, 0, softwarePhase, phaseOffset, &measurement));

    /* find left edge from 1 to 256 */
    for (phase = 1; phase < 256; phase++)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, 0, softwarePhase, phaseOffset, &measurement));

        softwarePhase = phase;

        if ((0 == measurement.upperBitCount) || (measurement.upperBitErrorCount > 0))
        {
            leftEdge = phase; /* found left edge */
            break;
        }
    }

    /* find right edge from -1 to -256 */
    for (phase = -1; phase > -256; phase--)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, 0, softwarePhase, phaseOffset, &measurement));

        softwarePhase = phase;

        if ((0 == measurement.upperBitCount) || (measurement.upperBitErrorCount > 0))
        {
            rightEdge = phase; /* found right edge */
            break;
        }
    }

    centerPhase = rightEdge + (leftEdge - rightEdge) / 2;

    /* find upper and lower boundaries */
    for (voltage = 0; voltage < 64; voltage++)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, centerPhase, (MCESD_U8)voltage, softwarePhase, phaseOffset, &measurement));

        softwarePhase = centerPhase;

        if ((upperVoltage == -64) && ((measurement.upperBitCount == 0) || (measurement.upperBitErrorCount > 0)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == 63) && ((measurement.lowerBitCount == 0) || (measurement.lowerBitErrorCount > 0)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -64) && (lowerVoltage != 63))
            break;
    }

    *width = (MCESD_U16)((leftEdge == rightEdge) ? 0 : (leftEdge - rightEdge) - 1);
    *height = (MCESD_U16)((upperVoltage == lowerVoltage) ? 0 : (upperVoltage - lowerVoltage) - 1);

    MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));

    return MCESD_OK;
}

static MCESD_STATUS API_C12GP41P2V_EOMSetPhase
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 softwarePhase,
    IN MCESD_32 targetPhase,
    IN MCESD_16 phaseOffset
)
{
    MCESD_U16 step, rawPhase;
    MCESD_16 phase, difference;
    MCESD_FIELD rxEomPhOffsetForce      = F_C12GP41P2VR2P0_EOM_PH_FORCE;
    MCESD_FIELD rxEomPhOffsetExt        = F_C12GP41P2VR2P0_EOM_PH_EXT;
    MCESD_FIELD rxEomPhOffsetExtValid   = F_C12GP41P2VR2P0_EOM_PH_VALID;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomPhOffsetForce, 1));

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxEomPhOffsetExt, &rawPhase));

    phase = softwarePhase;

    if ( phase == targetPhase )
        return MCESD_OK;
    else if ( phase > targetPhase )
        step = -4;
    else
        step = 4;

    do
    {
        difference = phase - targetPhase;
        if (difference < 0)
            difference *= -1;

        if (difference <= 4)
            phase = (MCESD_16)targetPhase;
        else
            phase += step;

        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomPhOffsetExt, (phase + phaseOffset) % 32));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomPhOffsetExtValid, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomPhOffsetExtValid, 0));
    } while (phase != targetPhase);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels,
    OUT S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr
)
{
    MCESD_32 upperVoltage = -C12GP41P2V_EYE_MAX_VOLT_STEPS;
    MCESD_32 lowerVoltage = C12GP41P2V_EYE_MAX_VOLT_STEPS - 1;
    MCESD_32 softwarePhase = 0;
    MCESD_32 phase, voltage;
    S_C12GP41P2V_EOM_DATA measurement;
    S_C12GP41P2V_EOM_SAVED savedFields;
    MCESD_16 phaseOffset, leftPhase, rightPhase, maxVoltageStep, phaseIdx, voltIdx, startPt, tmp;

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMGetEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if (0 == voltageSteps)
        voltageSteps = C12GP41P2V_EYE_DEFAULT_VOLT_STEPS;

    if (0 == phaseLevels)
        phaseLevels = C12GP41P2V_EYE_DEFAULT_PHASE_LEVEL;

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

    MCESD_ATTEMPT(API_C12GP41P2V_EOMInit(devPtr, &savedFields, &phaseOffset));

    MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, 0, 0, 0, phaseOffset, &measurement));

    if (0 != measurement.upperBitErrorCount || 0 != measurement.lowerBitErrorCount)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMGetEyeData: BER > 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMGetEyeData: Total Bit Count == 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    /* Sweep Voltage at Phase 0 */
    phaseIdx = phaseLevels;
    for (voltage = 0; voltage < voltageSteps; voltage++)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, 0, (MCESD_U8)voltage, 0, phaseOffset, &measurement));
        softwarePhase = 0;

        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep - voltage] = measurement.upperBitErrorCount;
        eyeRawDataPtr->eyeRawData[phaseIdx][maxVoltageStep + voltage] = measurement.lowerBitErrorCount;

        if ((upperVoltage == -C12GP41P2V_EYE_MAX_VOLT_STEPS) && ((0 == measurement.upperBitCount) || (0 < measurement.upperBitErrorCount)))
            upperVoltage = voltage; /* found upper edge */

        if ((lowerVoltage == (C12GP41P2V_EYE_MAX_VOLT_STEPS - 1)) && ((0 == measurement.lowerBitCount) || (0 < measurement.lowerBitErrorCount)))
            lowerVoltage = -voltage; /* found lower edge */

        if ((upperVoltage != -C12GP41P2V_EYE_MAX_VOLT_STEPS) && (lowerVoltage != (C12GP41P2V_EYE_MAX_VOLT_STEPS - 1)))
            break;
    }

    /* Left Sweep from Phase 1 with Voltage Sweep */
    phaseIdx -= 1;
    startPt = (upperVoltage > -lowerVoltage) ? upperVoltage : lowerVoltage;
    for (phase = 1; phase < phaseLevels; phase++)
    {
        voltage = ((startPt + C12GP41P2V_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C12GP41P2V_EYE_VOLT_OFFSET);
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, (MCESD_U8)voltage, softwarePhase, phaseOffset, &measurement));
            softwarePhase = phase;

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
        voltage = ((startPt + C12GP41P2V_EYE_VOLT_OFFSET) > maxVoltageStep) ? maxVoltageStep : (startPt + C12GP41P2V_EYE_VOLT_OFFSET);
        while (voltage >= 0)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, (MCESD_U8)voltage, softwarePhase, phaseOffset, &measurement));
            softwarePhase = phase;

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

    MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMPlotEyeData
(
    IN S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U16 voltageSteps,
    IN MCESD_U16 phaseLevels
)
{
    MCESD_U16 phaseIndex, voltageIndex, phaseInterval, timeUnit;
    char line[C12GP41P2V_EYE_MAX_PHASE_LEVEL * 2 + 2];

    memset(&line, 'X', C12GP41P2V_EYE_MAX_PHASE_LEVEL * 2 + 2);

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMPlotEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    if ((0 == voltageSteps) || (voltageSteps > C12GP41P2V_EYE_MAX_VOLT_STEPS))
        voltageSteps = C12GP41P2V_EYE_DEFAULT_VOLT_STEPS;

    if ((0 == phaseLevels) || (voltageSteps > C12GP41P2V_EYE_MAX_PHASE_LEVEL))
        phaseLevels = C12GP41P2V_EYE_DEFAULT_PHASE_LEVEL;

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
            else if (C12GP41P2V_EYE_LOW_ERROR_THRESH > eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex])
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

#endif /* C12GP41P2V */
