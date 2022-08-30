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
#include "../mcesdUtils.h"
#include "mcesdN5XC56GP5X4_Defs.h"
#include "mcesdN5XC56GP5X4_API.h"
#include "mcesdN5XC56GP5X4_RegRW.h"
#include "mcesdN5XC56GP5X4_HwCntl.h"
#include <string.h>

#ifdef N5XC56GP5X4

/* Forward internal function prototypes used only in this module */
static MCESD_U32 INT_N5XC56GP5X4_ComputeTxEqEmMain(IN MCESD_U32 pre3Cursor, IN MCESD_U32 pre2Cursor, IN MCESD_U32 preCursor, IN MCESD_U32 postCursor, IN MCESD_U32 naCursor);
static MCESD_STATUS INT_N5XC56GP5X4_SetTxEqPolarity(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_U8 position, IN E_N5XC56GP5X4_POLARITY polarity);
static MCESD_U32 INT_N5XC56GP5X4_Computef0(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane);
static MCESD_STATUS INT_N5XC56GP5X4_Computef0d(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, OUT MCESD_U32 *top, OUT MCESD_U32 *mid, OUT MCESD_U32 *bot);
static MCESD_STATUS INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_N5XC56GP5X4_GetDfeTap_ConvertToAverageMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldTopPtr, IN MCESD_FIELD_PTR fieldMidPtr, IN MCESD_FIELD_PTR fieldBotPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldMSBPtr, IN MCESD_FIELD_PTR fieldLSBPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_N5XC56GP5X4_SpeedGbpsToMbps(IN E_N5XC56GP5X4_SERDES_SPEED speed, OUT MCESD_U32 *mbps);

MCESD_STATUS API_N5XC56GP5X4_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_FW_MAJOR_VER, 255 /* ignored */, data);
    *major = (MCESD_U8)data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_FW_MINOR_VER, 255 /* ignored */, data);
    *minor = (MCESD_U8)data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_FW_PATCH_VER, 255 /* ignored */, data);
    *patch = (MCESD_U8)data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_FW_BUILD_VER, 255 /* ignored */, data);
    *build = (MCESD_U8)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *tsLocked,
    OUT MCESD_BOOL *rsLocked
)
{
    MCESD_U32 txData, rxData;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PLL_TS_LOCK, lane, txData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PLL_RS_LOCK, lane, rxData);
    *tsLocked = (MCESD_BOOL)txData;
    *rsLocked = (MCESD_BOOL)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
)
{
    MCESD_U32 txData, rxData;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PLL_READY_TX, lane, txData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PLL_READY_RX, lane, rxData);
    *txReady = (MCESD_BOOL)txData;
    *rxReady = (MCESD_BOOL)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 timeout
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT0, 1));
        MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_INITDON0, 1, timeout));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT0, 0));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT1, 1));
        MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_INITDON1, 1, timeout));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT1, 0));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT2, 1));
        MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_INITDON2, 1, timeout));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT2, 0));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT3, 1));
        MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_INITDON3, 1, timeout));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_INIT3, 0));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{
    switch (param)
    {
    case N5XC56GP5X4_TXEQ_EM_PRE3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C0_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C0, lane, paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C1_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C1, lane, paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C2_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C2, lane, paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_MAIN:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C3_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C3, lane, paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_POST:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C4_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C4, lane, paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_NA:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C5_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C5, lane, paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP_FORCE, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP, lane, 1);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 1));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP, lane, 0);

    /* The TX emphasis main is automatically calculated and updated unless user explicitly wants to manually set it */
    if (param != N5XC56GP5X4_TXEQ_EM_MAIN)
    {
        MCESD_U32 txEmPre3Data, txEmPre2Data, txEmPreData, txEmMainData, txEmPostData, txEmNAData;

        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C0, lane, txEmPre3Data);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C1, lane, txEmPre2Data);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C2, lane, txEmPreData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C4, lane, txEmPostData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C5, lane, txEmNAData);

        /* Calculate Tx emphasis Main based on full_swing setting */
        txEmMainData = INT_N5XC56GP5X4_ComputeTxEqEmMain(txEmPre3Data, txEmPre2Data, txEmPreData, txEmPostData, txEmNAData);

        /* Update Tx emphasis Main */
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C3_FORCE, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_C3, lane, txEmMainData);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP, lane, 1);
        MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 1));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP, lane, 0);
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    switch (param)
    {
    case N5XC56GP5X4_TXEQ_EM_PRE3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C0, lane, *paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C1, lane, *paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C2, lane, *paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_MAIN:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C3, lane, *paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_POST:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C4, lane, *paramValue);
        break;
    case N5XC56GP5X4_TXEQ_EM_NA:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_ANA_TX_C5, lane, *paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN E_N5XC56GP5X4_POLARITY polarity
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_FIR_TAP_POL_F, lane, 1);

    switch (param)
    {
    case N5XC56GP5X4_TXEQ_EM_PRE3:
        INT_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, 0, polarity);
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE2:
        INT_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, 1, polarity);
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE:
        INT_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, 2, polarity);
        break;
    case N5XC56GP5X4_TXEQ_EM_MAIN:
        INT_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, 3, polarity);
        break;
    case N5XC56GP5X4_TXEQ_EM_POST:
        INT_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, 4, polarity);
        break;
    case N5XC56GP5X4_TXEQ_EM_NA:
        INT_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, 5, polarity);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    OUT E_N5XC56GP5X4_POLARITY* polarity
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TO_ANA_TX_FIR_POL, lane, data);

    switch (param)
    {
    case N5XC56GP5X4_TXEQ_EM_PRE3:
        *polarity = data & 0x1;
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE2:
        *polarity = (data >> 1) & 0x1;
        break;
    case N5XC56GP5X4_TXEQ_EM_PRE:
        *polarity = (data >> 2) & 0x1;
        break;
    case N5XC56GP5X4_TXEQ_EM_MAIN:
        *polarity = (data >> 3) & 0x1;
        break;
    case N5XC56GP5X4_TXEQ_EM_POST:
        *polarity = (data >> 4) & 0x1;
        break;
    case N5XC56GP5X4_TXEQ_EM_NA:
        *polarity = (data >> 5) & 0x1;
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

static MCESD_STATUS INT_N5XC56GP5X4_SetTxEqPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 position,
    IN E_N5XC56GP5X4_POLARITY polarity
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_FIR_TAP_POL, lane, data);
    data = ((data & ~(1 << position)) | (polarity << position));    /* Modify bit at position with polarity */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_FIR_TAP_POL, lane, data);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    switch (param)
    {
    case N5XC56GP5X4_CTLE_CUR1_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CURRENT1_SEL, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL1_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RL1_SEL, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL1_EXTRA:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RL1_EXTRA, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_RES1_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RES1_SEL, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_CAP1_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CAP1_SEL_G, lane, ConvertU32ToGrayCode(paramValue));
        break;
    case N5XC56GP5X4_CTLE_EN_MIDFREQ:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_EN_MID_FREQ, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_CS1_MID:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CS1_MID, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_RS1_MID:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RS1_MID, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_CUR2_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CURRENT2_SEL, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL2_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RL2_SEL, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL2_TUNE_G:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RL2_TUNE_G, lane, ConvertU32ToGrayCode(paramValue));
        break;
    case N5XC56GP5X4_CTLE_RES2_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RES2_SEL, lane, paramValue);
        break;
    case N5XC56GP5X4_CTLE_CAP2_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CAP2_SEL, lane, paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_U32 grayCode;

    switch (param)
    {
    case N5XC56GP5X4_CTLE_CUR1_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_CURRENT1_SEL, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL1_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RL1_SEL, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL1_EXTRA:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RL1_EXTRA, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_RES1_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RES1_SEL, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_CAP1_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_CAP1_SEL_G, lane, grayCode);
        *paramValue = ConvertGrayCodeToU32(grayCode);
        break;
    case N5XC56GP5X4_CTLE_EN_MIDFREQ:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EN_MID_FREQ, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_CS1_MID:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_CS1_MID, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_RS1_MID:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RS1_MID, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_CUR2_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_CURRENT2_SEL, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL2_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RL2_SEL, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_RL2_TUNE_G:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RL2_TUNE_G, lane, grayCode);
        *paramValue = ConvertGrayCodeToU32(grayCode);
        break;
    case N5XC56GP5X4_CTLE_RES2_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RES2_SEL, lane, *paramValue);
        break;
    case N5XC56GP5X4_CTLE_CAP2_SEL:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_CAP2_SEL, lane, *paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_U32 INT_N5XC56GP5X4_ComputeTxEqEmMain
(
    IN MCESD_U32 pre3Cursor,
    IN MCESD_U32 pre2Cursor,
    IN MCESD_U32 preCursor,
    IN MCESD_U32 postCursor,
    IN MCESD_U32 naCursor
)
{
    const MCESD_U32 fullSwing = 63;
    MCESD_U32 sum = pre3Cursor + pre2Cursor + preCursor + postCursor + naCursor;

    if (fullSwing >= sum)
        return fullSwing - sum;
    else
        return 0;
}

MCESD_STATUS API_N5XC56GP5X4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_EN, lane, state);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_DFE_EN, lane, data);
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_UP_DIS, lane, state);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_DFE_UP_DIS, lane, data);
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTmb,
    IN E_N5XC56GP5X4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    /* Validate eye parameter */
    if ((eyeTmb != N5XC56GP5X4_EYE_TOP) && (eyeTmb != N5XC56GP5X4_EYE_MID) && (eyeTmb != N5XC56GP5X4_EYE_BOT))
        return MCESD_FAIL;

    switch (tap)
    {
    case N5XC56GP5X4_DFE_DC:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_DC_D_T_E_SM, F_N5XC56GP5X4_DC_D_M_E_SM, F_N5XC56GP5X4_DC_D_B_E_SM };

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[eyeTmb]), tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_DC_E:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_DC_E_E_SM;
        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_VREF:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_VREF_T_E_SM, F_N5XC56GP5X4_VREF_M_E_SM, F_N5XC56GP5X4_VREF_B_E_SM };

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[eyeTmb]), tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F0:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_F0_D_T_E_SM, F_N5XC56GP5X4_F0_D_M_E_SM, F_N5XC56GP5X4_F0_D_B_E_SM };
        MCESD_U32 data;

        N5XC56GP5X4_READ_FIELD(devPtr, EXTRACT_FIELD(fieldList[eyeTmb]), lane, data);
        *tapValue = data * 1000; /* F0 tap is not signed. Unlike other DFE taps, we do not treat it like signed magnitude */
    }
    break;
    case N5XC56GP5X4_DFE_F1:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_F1_D_T_E_SM, F_N5XC56GP5X4_F1_D_M_E_SM, F_N5XC56GP5X4_F1_D_B_E_SM };

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[eyeTmb]), tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F1P5:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_F1P5_E_SM;
        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F2:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_F2_D_T_E_SM, F_N5XC56GP5X4_F2_D_M_E_SM, F_N5XC56GP5X4_F2_D_B_E_SM };

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &(fieldList[eyeTmb]), tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F3:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_F3_T_E_SM, F_N5XC56GP5X4_F3_M_E_SM, F_N5XC56GP5X4_F3_B_E_SM };

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &fieldList[0], &fieldList[1], &fieldList[2], tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F4:
    {
        MCESD_FIELD fieldList[3] = { F_N5XC56GP5X4_F4_T_E_SM, F_N5XC56GP5X4_F4_M_E_SM, F_N5XC56GP5X4_F4_B_E_SM };

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToAverageMilliCodes(devPtr, lane, &fieldList[0], &fieldList[1], &fieldList[2], tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F5:
    {
        MCESD_FIELD msbField = F_N5XC56GP5X4_F5_MSB_E_SM;
        MCESD_FIELD lsbField = F_N5XC56GP5X4_F5_LSB_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &msbField, &lsbField, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F6:
    {
        MCESD_FIELD msbField = F_N5XC56GP5X4_F6_MSB_E_SM;
        MCESD_FIELD lsbField = F_N5XC56GP5X4_F6_LSB_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &msbField, &lsbField, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F7:
    {
        MCESD_FIELD msbField = F_N5XC56GP5X4_F7_MSB_E_SM;
        MCESD_FIELD lsbField = F_N5XC56GP5X4_F7_LSB_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &msbField, &lsbField, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F8:
    {
        MCESD_FIELD msbField = F_N5XC56GP5X4_F8_MSB_E_SM;
        MCESD_FIELD lsbField = F_N5XC56GP5X4_F8_LSB_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &msbField, &lsbField, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F9:
    {
        MCESD_FIELD msbField = F_N5XC56GP5X4_F9_MSB_E_SM;
        MCESD_FIELD lsbField = F_N5XC56GP5X4_F9_LSB_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &msbField, &lsbField, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F10:
    {
        MCESD_FIELD msbField = F_N5XC56GP5X4_F10_MSB_E_SM;
        MCESD_FIELD lsbField = F_N5XC56GP5X4_F10_LSB_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes(devPtr, lane, &msbField, &lsbField, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F11:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_F11_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F12:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_F12_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F13:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_F13_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F14:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_F14_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_F15:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_F15_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_FF0:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_FF0_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_FF1:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_FF1_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_FF2:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_FF2_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_FF3:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_FF3_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_FF4:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_FF4_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    case N5XC56GP5X4_DFE_FF5:
    {
        MCESD_FIELD field = F_N5XC56GP5X4_FF5_E_SM;

        MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &field, tapValue));
    }
    break;
    default:
        return MCESD_FAIL; /* Invalid tap */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    OUT MCESD_32 *tapValue
)
{
    MCESD_U32 code;

    MCESD_ATTEMPT(API_N5XC56GP5X4_ReadField(devPtr, lane, fieldPtr, &code));
    *tapValue = ConvertSignedMagnitudeToI32(code, fieldPtr->totalBits) * 1000; /* milli-Codes */
    return MCESD_OK;
}

MCESD_STATUS INT_N5XC56GP5X4_GetDfeTap_ConvertToAverageMilliCodes
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldTopPtr,
    IN MCESD_FIELD_PTR fieldMidPtr,
    IN MCESD_FIELD_PTR fieldBotPtr,
    OUT MCESD_32 *tapValue
)
{
    MCESD_32 top, mid, bot;

    MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, fieldTopPtr, &top));
    MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, fieldMidPtr, &mid));
    MCESD_ATTEMPT(INT_N5XC56GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, fieldBotPtr, &bot));

    *tapValue = (top + mid + bot) / 3;
    return MCESD_OK;
}


MCESD_STATUS INT_N5XC56GP5X4_GetDfeTap_ConvertMsbLsbToMilliCodes
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldMSBPtr,
    IN MCESD_FIELD_PTR fieldLSBPtr,
    OUT MCESD_32 *tapValue
)
{
    MCESD_U32 msbData, lsbData;
    MCESD_32 msb, lsb;

    MCESD_ATTEMPT(API_N5XC56GP5X4_ReadField(devPtr, lane, fieldMSBPtr, &msbData));
    MCESD_ATTEMPT(API_N5XC56GP5X4_ReadField(devPtr, lane, fieldLSBPtr, &lsbData));
    msb = ConvertSignedMagnitudeToI32(msbData, fieldMSBPtr->totalBits);
    lsb = ConvertSignedMagnitudeToI32(lsbData, fieldLSBPtr->totalBits);
    *tapValue = (((msb * 2) + lsb) * 1000) / 3; /* milli-Codes */
    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_BROADCAST, 255 /* ignored*/, state);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_BROADCAST, 255 /* ignored*/, data);
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_IVREF, state));
#ifdef N5XC56GP5X4_ISOLATION
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_IVREF_FM_REG, 255 /* ignored*/, 1);
#endif

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_IVREF, &data));
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPowerTx
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
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_TX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPowerRx
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
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_RX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE0, state ? 0 : 1));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE1, state ? 0 : 1));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE2, state ? 0 : 1));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE3, state ? 0 : 1));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxOutputEnable
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
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_IDLE3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_TRUE : MCESD_FALSE;    /* inverted */

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPowerPLL
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
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PU_PLL3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_N5XC56GP5X4_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_MODE, mode));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_N5XC56GP5X4_PHYMODE *mode
)
{
    MCESD_U16 data;

    MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_MODE, &data));
    *mode = (E_N5XC56GP5X4_PHYMODE)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_REFFREQ txFreq,
    IN E_N5XC56GP5X4_REFFREQ rxFreq,
    IN E_N5XC56GP5X4_REFCLK_SEL txClkSel,
    IN E_N5XC56GP5X4_REFCLK_SEL rxClkSel
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX0, txFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX0, rxFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX0, txClkSel));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX0, rxClkSel));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX1, txFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX1, rxFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX1, txClkSel));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX1, rxClkSel));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX2, txFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX2, rxFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX2, txClkSel));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX2, rxClkSel));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX3, txFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX3, rxFreq));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX3, txClkSel));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX3, rxClkSel));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_REFFREQ *txFreq,
    OUT E_N5XC56GP5X4_REFFREQ *rxFreq,
    OUT E_N5XC56GP5X4_REFCLK_SEL *txClkSel,
    OUT E_N5XC56GP5X4_REFCLK_SEL *rxClkSel
)
{
    MCESD_U16 txFreqData, rxFreqData, txClkSelData, rxClkSelData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX0, &txFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX0, &rxFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX0, &txClkSelData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX0, &rxClkSelData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX1, &txFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX1, &rxFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX1, &txClkSelData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX1, &rxClkSelData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX2, &txFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX2, &rxFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX2, &txClkSelData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX2, &rxClkSelData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_TX3, &txFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFFREF_RX3, &rxFreqData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_TX3, &txClkSelData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_REFCLK_RX3, &rxClkSelData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txFreq = (E_N5XC56GP5X4_REFFREQ)txFreqData;
    *rxFreq = (E_N5XC56GP5X4_REFFREQ)rxFreqData;
    *txClkSel = (E_N5XC56GP5X4_REFCLK_SEL)txClkSelData;
    *rxClkSel = (E_N5XC56GP5X4_REFCLK_SEL)rxClkSelData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_SERDES_SPEED txSpeed,
    IN E_N5XC56GP5X4_SERDES_SPEED rxSpeed
)
{
    if ((devPtr->ipMajorRev == 1) && (devPtr->ipMinorRev < 2))
    {
        /* RX.X < R1.2 */
        if ((txSpeed >= N5XC56GP5X4_SERDES_64G) || (rxSpeed >= N5XC56GP5X4_SERDES_64G))
            return MCESD_FAIL; /* Invalid speed for R1.1 revision */
    }

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX0, txSpeed));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX0, rxSpeed));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX1, txSpeed));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX1, rxSpeed));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX2, txSpeed));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX2, rxSpeed));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX3, txSpeed));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX3, rxSpeed));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_SERDES_SPEED *txSpeed,
    OUT E_N5XC56GP5X4_SERDES_SPEED *rxSpeed
)
{
    MCESD_U16 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX0, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX1, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX2, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_TX3, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_PHY_GEN_RX3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txSpeed = (E_N5XC56GP5X4_SERDES_SPEED)txData;
    *rxSpeed = (E_N5XC56GP5X4_SERDES_SPEED)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_DATABUS_WIDTH txWidth,
    IN E_N5XC56GP5X4_DATABUS_WIDTH rxWidth
)
{
    MCESD_U32 txDataSelBits = 0, txDataPAM2En = 0;
    MCESD_U32 rxDataSelBits = 0, rxDataPAM2En = 0;

    switch (txWidth)
    {
    case N5XC56GP5X4_DATABUS_80BIT:
    {
        txDataSelBits = 0;
        txDataPAM2En = 0;
    }
    break;
    case N5XC56GP5X4_DATABUS_40BIT:
    {
        txDataSelBits = 0;
        txDataPAM2En = 1;
    }
    break;
    case N5XC56GP5X4_DATABUS_64BIT:
    {
        txDataSelBits = 1;
        txDataPAM2En = 0;
    }
    break;
    case N5XC56GP5X4_DATABUS_32BIT:
    {
        txDataSelBits = 1;
        txDataPAM2En = 1;
    }
    break;
    default:
        break;
    }

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_SEL_BITS, lane, txDataSelBits);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_PAM2_EN, lane, txDataPAM2En);

    switch (rxWidth)
    {
    case N5XC56GP5X4_DATABUS_80BIT:
        {
            rxDataSelBits = 0;
            rxDataPAM2En = 0;
        }
        break;
    case N5XC56GP5X4_DATABUS_40BIT:
        {
            rxDataSelBits = 0;
            rxDataPAM2En = 1;
        }
        break;
    case N5XC56GP5X4_DATABUS_64BIT:
        {
            rxDataSelBits = 1;
            rxDataPAM2En = 0;
        }
        break;
    case N5XC56GP5X4_DATABUS_32BIT:
        {
            rxDataSelBits = 1;
            rxDataPAM2En = 1;
        }
        break;
    default:
        break;
    }

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_SEL_BITS, lane, rxDataSelBits);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PAM2_EN, lane, rxDataPAM2En);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_DATABUS_WIDTH *txWidth,
    OUT E_N5XC56GP5X4_DATABUS_WIDTH *rxWidth
)
{
    MCESD_U32 txDataSelBits, txDataPAM2En, rxDataSelBits, rxDataPAM2En;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_SEL_BITS, lane, txDataSelBits);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_PAM2_EN, lane, txDataPAM2En);

    if (0 == txDataPAM2En)
    {
        if (0 == txDataSelBits)
            *txWidth = N5XC56GP5X4_DATABUS_80BIT;
        else
            *txWidth = N5XC56GP5X4_DATABUS_64BIT;
    }
    else
    {
        if (0 == txDataSelBits)
            *txWidth = N5XC56GP5X4_DATABUS_40BIT;
        else
            *txWidth = N5XC56GP5X4_DATABUS_32BIT;
    }

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SEL_BITS, lane, rxDataSelBits);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_PAM2_EN, lane, rxDataPAM2En);

    if (0 == rxDataPAM2En)
    {
        if (0 == rxDataSelBits)
            *rxWidth = N5XC56GP5X4_DATABUS_80BIT;
        else
            *rxWidth = N5XC56GP5X4_DATABUS_64BIT;
    }
    else
    {
        if (0 == rxDataSelBits)
            *rxWidth = N5XC56GP5X4_DATABUS_40BIT;
        else
            *rxWidth = N5XC56GP5X4_DATABUS_32BIT;
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *clockMHz
)
{
    MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 align90
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PH_OS_DAT, lane, align90);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *align90
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PH_OS_DAT, lane, data);
    *align90 = (MCESD_U16)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    IN S_N5XC56GP5X4_TRAIN_TIMEOUT *training
)
{
    if (training == NULL)
        return MCESD_FAIL;

    if (type == N5XC56GP5X4_TRAINING_TRX)
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_TIMER_EN, lane, training->enable);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TRX_TIMER, lane, training->timeout);
    }
    else
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_TIMER_EN, lane, training->enable);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_TIMER, lane, training->timeout);
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    OUT S_N5XC56GP5X4_TRAIN_TIMEOUT *training
)
{
    MCESD_U32 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == N5XC56GP5X4_TRAINING_TRX)
    {
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TIMER_EN, lane, enableData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TRX_TIMER, lane, timeoutData);
    }
    else
    {
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TIMER_EN, lane, enableData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TIMER, lane, timeoutData);
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = (MCESD_U16)timeoutData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
)
{
    MCESD_U16 failed;

    MCESD_ATTEMPT(API_N5XC56GP5X4_StartTraining(devPtr, lane, type));
    if (type == N5XC56GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO0, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA0, &failed));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO1, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA1, &failed));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO2, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA2, &failed));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO3, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA3, &failed));
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
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO0, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA0, &failed));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO1, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA1, &failed));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO2, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA2, &failed));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_PollPin(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO3, 1, 90000));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA3, &failed));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}

MCESD_STATUS API_N5XC56GP5X4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_SAT_EN, lane, 0);
    if (type == N5XC56GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN0, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN0, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN1, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN2, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN3, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN3, 1));
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
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN0, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN0, 1));
            break;
        case 1:

            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN1, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN2, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN3, 0));
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN3, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U16 completeData, failedData = 0;

    if (type == N5XC56GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA0, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA1, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA2, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINCO3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINFA3, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
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
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA0, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA1, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA2, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINCO3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINFA3, &failedData));
                MCESD_ATTEMPT(API_N5XC56GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    *completed = (MCESD_BOOL)completeData;
    *failed = (MCESD_BOOL)failedData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
)
{
    if (type == N5XC56GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_TRAINEN3, 0));
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
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_TRAINEN3, 0));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 1));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5XC56GP5X4_TRAINED_EYE_H *trainedEyeHeight
)
{
    MCESD_U32 prevState, f0xData, f0bData, f0n1Data, f0n2Data, f0dData, f0d_tData, f0d_mData, f0d_bData;

    f0d_tData = 0;
    f0d_mData = 0;
    f0d_bData = 0;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_DFE_ADAPT, lane, prevState);

    /* turn off DFE continuous */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_CMD, lane, 0x18);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_ARGS, lane, 0x0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_START, lane, 1);

    /* f0b */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_CMD, lane, 0x19);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_ARGS, lane, 0x4000);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_START, lane, 1);
    f0bData = INT_N5XC56GP5X4_Computef0(devPtr, lane);

    /* fn1 */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_ARGS, lane, 0x8000);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_F0X_SEL, lane, 0x11);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_START, lane, 1);
    f0xData = INT_N5XC56GP5X4_Computef0(devPtr, lane);
    f0n1Data = f0bData - f0xData;

    /* fn2 */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_F0X_SEL, lane, 0x12);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_START, lane, 1);
    f0xData = INT_N5XC56GP5X4_Computef0(devPtr, lane);
    f0n2Data = f0bData - f0xData;

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_LOAD_EN, lane, 0);

    /* f0d */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_ARGS, lane, 0x20000);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_START, lane, 1);
    f0dData = INT_N5XC56GP5X4_Computef0(devPtr, lane);
    MCESD_ATTEMPT(INT_N5XC56GP5X4_Computef0d(devPtr, lane, &f0d_tData, &f0d_mData, &f0d_bData));

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DFE_LOAD_EN, lane, 1);

    if (prevState)
    {
        /* turn on DFE continuous */
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_CMD, lane, 0x18);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_ARGS, lane, 0x1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_CLI_START, lane, 1);
    }

    trainedEyeHeight->f0b = f0bData;
    trainedEyeHeight->f0n1 = f0n1Data;
    trainedEyeHeight->f0n2 = f0n2Data;
    trainedEyeHeight->f0d = f0dData;
    trainedEyeHeight->f0d_t = f0d_tData;
    trainedEyeHeight->f0d_m = f0d_mData;
    trainedEyeHeight->f0d_b = f0d_bData;

    return MCESD_OK;
}

MCESD_U32 INT_N5XC56GP5X4_Computef0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_U32 data;
    double sum;

    sum = 0;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_T_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_T_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_T_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_T_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_M_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_M_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_M_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_M_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_B_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_B_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_B_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_B_E_2C, lane, data);
    sum += data;

    /* value is returned in milli-units */
    return (MCESD_U32) (sum * 1000 / 12.0);
}

MCESD_STATUS INT_N5XC56GP5X4_Computef0d
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U32 *top,
    OUT MCESD_U32 *mid,
    OUT MCESD_U32 *bot
)
{
    MCESD_U32 data;
    double sum;

    sum = 0;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_T_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_T_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_T_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_T_E_2C, lane, data);
    sum += data;

    /* value is returned in milli-units */
    *top = (MCESD_U32) (sum * 1000 / 4.0);

    sum = 0;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_M_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_M_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_M_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_M_E_2C, lane, data);
    sum += data;

    /* value is returned in milli-units */
    *mid = (MCESD_U32) (sum * 1000 / 4.0);

    sum = 0;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_B_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_B_O_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_D_B_E_2C, lane, data);
    sum += data;
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_F0_S_B_E_2C, lane, data);
    sum += data;

    /* value is returned in milli-units */
    *bot = (MCESD_U32) (sum * 1000 / 4.0);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    switch (param)
    {
    case N5XC56GP5X4_CDR_SELMUFI:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SELMUFI, lane, *paramValue);
        break;
    case N5XC56GP5X4_CDR_SELMUFF:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SELMUFF, lane, *paramValue);
        break;
    case N5XC56GP5X4_CDR_SELMUPI:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REG_SELMUPI, lane, *paramValue);
        break;
    case N5XC56GP5X4_CDR_SELMUPF:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REG_SELMUPF, lane, *paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    switch (param)
    {
    case N5XC56GP5X4_CDR_SELMUFI:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_SELMUFI, lane, paramValue);
        break;
    case N5XC56GP5X4_CDR_SELMUFF:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_SELMUFF, lane, paramValue);
        break;
    case N5XC56GP5X4_CDR_SELMUPI:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REG_SELMUPI, lane, paramValue);
        break;
    case N5XC56GP5X4_CDR_SELMUPF:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REG_SELMUPF, lane, paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_U16 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_SQ_DET_LPF0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_SQ_DET_LPF1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_SQ_DET_LPF2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_SQ_DET_LPF3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *squelched = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 threshold
)
{
    if (threshold > N5XC56GP5X4_SQ_THRESH_MAX)
        return MCESD_FAIL;

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_SQ_INDV, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_SQ_RES_EXT, lane, threshold);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_16 *threshold
)
{
    MCESD_U32 data;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_SQ_RES_RD, lane, data);
    *threshold = (MCESD_16)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_DATAPATH path
)
{
    switch (path)
    {
    case N5XC56GP5X4_PATH_NEAR_END_LB:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DTL_CLAMPING, lane, 7);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_RX2TX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_A_TX2RX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_TX2RX_LPBK, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_EN, lane, 0);
        break;
    case N5XC56GP5X4_PATH_LOCAL_LB:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DTL_CLAMPING, lane, 7);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_RX2TX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_A_TX2RX_LPBK, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_TX2RX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_LB, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_EN, lane, 0);
        break;
    case N5XC56GP5X4_PATH_EXTERNAL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DTL_CLAMPING, lane, 7);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_RX2TX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_A_TX2RX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_TX2RX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_LB, lane, 0);
        break;
    case N5XC56GP5X4_PATH_FAR_END_LB:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_DTL_CLAMPING, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_R2T_NO_STOP, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_R2T_RD_START, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_FOFFSET_DIS, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_RX2TX_LPBK, lane, 1);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_A_TX2RX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_D_TX2RX_LPBK, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_LB, lane, 0);
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_EN, lane, 0);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_DATAPATH *path
)
{
    MCESD_U32 digRxToTxData, anaTxToRxData, digTxToRxData;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_D_RX2TX_LPBK, lane, digRxToTxData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_A_TX2RX_LPBK, lane, anaTxToRxData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_D_TX2RX_LPBK, lane, digTxToRxData);

    if (1 == digTxToRxData)
        *path = N5XC56GP5X4_PATH_NEAR_END_LB;
    else if (1 == digRxToTxData)
        *path = N5XC56GP5X4_PATH_FAR_END_LB;
    else if (1 == anaTxToRxData)
        *path = N5XC56GP5X4_PATH_LOCAL_LB;
    else
        *path = N5XC56GP5X4_PATH_EXTERNAL;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
)
{
    MCESD_U32 data;
    MCESD_32 codeValue;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TSEN_DATA, 255 /* ignored */, data);
    codeValue = ConvertTwosComplementToI32(data, 12);
    *temperature = (MCESD_32)((codeValue * N5XC56GP5X4_TSENE_GAIN) + N5XC56GP5X4_TSENE_OFFSET);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_POLARITY txPolarity,
    IN E_N5XC56GP5X4_POLARITY rxPolarity
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXD_INV, lane, txPolarity);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXD_INV, lane, rxPolarity);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_POLARITY *txPolarity,
    OUT E_N5XC56GP5X4_POLARITY *rxPolarity
)
{
    MCESD_U32 txData, rxData;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXD_INV, lane, txData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXD_INV, lane, rxData);

    *txPolarity = (E_N5XC56GP5X4_POLARITY)txData;
    *rxPolarity = (E_N5XC56GP5X4_POLARITY)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
)
{
    if ((errors < 1) || (errors > 8))
        return MCESD_FAIL;

    /* 0 = 1 error, 1 = 2 errors, ... 7 = 8 errors */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ADD_ERR_NUM, lane, errors - 1);

    /* Toggle F_N5XC56GP5X4_ADD_ERR_EN 0 -> 1 to trigger error injection */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ADD_ERR_EN, lane, 0);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 5));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ADD_ERR_EN, lane, 1);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 5));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ADD_ERR_EN, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_PATTERN txPattern,
    IN E_N5XC56GP5X4_PATTERN rxPattern,
    IN const char *txUserPattern,
    IN const char *rxUserPattern
)
{
    MCESD_U32 txDataPAM2En, rxDataPAM2En;
    E_N5XC56GP5X4_DATABUS_WIDTH txWidth, rxWidth;

    if (N5XC56GP5X4_PAT_USER == txPattern)
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_PAT_SEL, lane, 1);
    }
    else
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_PAT_SEL, lane, txPattern);
    }

    if (strlen(txUserPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(txUserPattern, u8Pattern));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP_7948, lane, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3]));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP_4716, lane, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7]));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_UP_1500, lane, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9]));
    }

    if (N5XC56GP5X4_PAT_USER == rxPattern)
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PAT_SEL, lane, 1);
    }
    else
    {
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PAT_SEL, lane, rxPattern);
    }

    if (strlen(rxUserPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(rxUserPattern, u8Pattern));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_UP_7948, lane, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3]));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_UP_4716, lane, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7]));
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_UP_1500, lane, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9]));
    }

    MCESD_ATTEMPT(API_N5XC56GP5X4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_PAM2_EN, lane, txDataPAM2En);
    if (0 == txDataPAM2En)
    {
        if ((N5XC56GP5X4_PAT_JITTER_8T == txPattern) || (N5XC56GP5X4_PAT_JITTER_4T == txPattern))
            txWidth = N5XC56GP5X4_DATABUS_64BIT;
        else if ((N5XC56GP5X4_PAT_JITTER_10T == txPattern) || (N5XC56GP5X4_PAT_JITTER_5T == txPattern))
            txWidth = N5XC56GP5X4_DATABUS_80BIT;
    }
    else
    {
        if ((N5XC56GP5X4_PAT_JITTER_8T == txPattern) || (N5XC56GP5X4_PAT_JITTER_4T == txPattern))
            txWidth = N5XC56GP5X4_DATABUS_32BIT;
        else if ((N5XC56GP5X4_PAT_JITTER_10T == txPattern) || (N5XC56GP5X4_PAT_JITTER_5T == txPattern))
            txWidth = N5XC56GP5X4_DATABUS_40BIT;
    }

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_PAM2_EN, lane, rxDataPAM2En);
    if (0 == rxDataPAM2En)
    {
        if ((N5XC56GP5X4_PAT_JITTER_8T == rxPattern) || (N5XC56GP5X4_PAT_JITTER_4T == rxPattern))
            rxWidth = N5XC56GP5X4_DATABUS_64BIT;
        else if ((N5XC56GP5X4_PAT_JITTER_10T == rxPattern) || (N5XC56GP5X4_PAT_JITTER_5T == rxPattern))
            rxWidth = N5XC56GP5X4_DATABUS_80BIT;
    }
    else
    {
        if ((N5XC56GP5X4_PAT_JITTER_8T == rxPattern) || (N5XC56GP5X4_PAT_JITTER_4T == rxPattern))
            rxWidth = N5XC56GP5X4_DATABUS_32BIT;
        else if ((N5XC56GP5X4_PAT_JITTER_10T == rxPattern) || (N5XC56GP5X4_PAT_JITTER_5T == rxPattern))
            rxWidth = N5XC56GP5X4_DATABUS_40BIT;
    }
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetDataBusWidth(devPtr, lane, txWidth, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_PATTERN *txPattern,
    OUT E_N5XC56GP5X4_PATTERN *rxPattern,
    OUT char *txUserPattern,
    OUT char *rxUserPattern
)
{
    MCESD_FIELD txUserPatternFieldArray[] = { F_N5XC56GP5X4_TX_UP_7948 , F_N5XC56GP5X4_TX_UP_4716 , F_N5XC56GP5X4_TX_UP_1500 };
    MCESD_FIELD rxUserPatternFieldArray[] = { F_N5XC56GP5X4_RX_UP_7948 , F_N5XC56GP5X4_RX_UP_4716 , F_N5XC56GP5X4_RX_UP_1500 };
    MCESD_32 txUserPatternFieldArrayCount = sizeof(txUserPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 rxUserPatternFieldArrayCount = sizeof(rxUserPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 i;
    MCESD_U32 txPatternSelData, rxPatternSelData, txUserPatternData, rxUserPatternData;
    MCESD_U8 u8Pattern[10];

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_PAT_SEL, lane, txPatternSelData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_PAT_SEL, lane, rxPatternSelData);

    *txPattern = (1 == txPatternSelData) ? N5XC56GP5X4_PAT_USER : (E_N5XC56GP5X4_PATTERN)txPatternSelData;
    *rxPattern = (1 == rxPatternSelData) ? N5XC56GP5X4_PAT_USER : (E_N5XC56GP5X4_PATTERN)rxPatternSelData;

    for (i = 0; i < txUserPatternFieldArrayCount; i++)
    {
        N5XC56GP5X4_READ_FIELD(devPtr, EXTRACT_FIELD(txUserPatternFieldArray[i]), lane, txUserPatternData);
        if (i != txUserPatternFieldArrayCount - 1)
        {
            u8Pattern[i * 4 + 0] = (MCESD_U8)(txUserPatternData >> 24);
            u8Pattern[i * 4 + 1] = (MCESD_U8)((txUserPatternData >> 16) & 0xFF);
            u8Pattern[i * 4 + 2] = (MCESD_U8)((txUserPatternData >> 8) & 0xFF);
            u8Pattern[i * 4 + 3] = (MCESD_U8)(txUserPatternData & 0xFF);
        }
        else
        {
            u8Pattern[i * 4 + 0] = (MCESD_8)(txUserPatternData >> 8);
            u8Pattern[i * 4 + 1] = (MCESD_8)(txUserPatternData & 0xFF);
        }
    }
    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, txUserPattern));

    for (i = 0; i < rxUserPatternFieldArrayCount; i++)
    {
        N5XC56GP5X4_READ_FIELD(devPtr, EXTRACT_FIELD(rxUserPatternFieldArray[i]), lane, rxUserPatternData);
        if (i != rxUserPatternFieldArrayCount - 1)
        {
            u8Pattern[i * 4 + 0] = (MCESD_U8)(rxUserPatternData >> 24);
            u8Pattern[i * 4 + 1] = (MCESD_U8)((rxUserPatternData >> 16) & 0xFF);
            u8Pattern[i * 4 + 2] = (MCESD_U8)((rxUserPatternData >> 8) & 0xFF);
            u8Pattern[i * 4 + 3] = (MCESD_U8)(rxUserPatternData & 0xFF);
        }
        else
        {
            u8Pattern[i * 4 + 0] = (MCESD_8)(rxUserPatternData >> 8);
            u8Pattern[i * 4 + 1] = (MCESD_8)(rxUserPatternData & 0xFF);
        }
    }
    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, rxUserPattern));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_SWAP_MSB_LSB txSwapMsbLsb,
    IN E_N5XC56GP5X4_SWAP_MSB_LSB rxSwapMsbLsb
)
{
    /* TX Swap MSB LSB */
    if (N5XC56GP5X4_SWAP_NOT_USED != txSwapMsbLsb)
    {
        if (N5XC56GP5X4_SWAP_DISABLE == txSwapMsbLsb)
        {
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_SWAP, lane, 0);
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXD_SWAP, lane, 0);
        }
        else if (N5XC56GP5X4_SWAP_PRECODER == txSwapMsbLsb)
        {
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_SWAP, lane, 1);
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXD_SWAP, lane, 0);
        }
        else if (N5XC56GP5X4_SWAP_POSTCODER == txSwapMsbLsb)
        {
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_SWAP, lane, 0);
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXD_SWAP, lane, 1);
        }
    }

    /* RX Swap MSB LSB */
    if (N5XC56GP5X4_SWAP_NOT_USED != rxSwapMsbLsb)
    {
        if (N5XC56GP5X4_SWAP_DISABLE == rxSwapMsbLsb)
        {
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_SWAP, lane, 0);
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXD_SWAP, lane, 0);
        }
        else if (N5XC56GP5X4_SWAP_PRECODER == rxSwapMsbLsb)
        {
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_SWAP, lane, 1);
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXD_SWAP, lane, 0);
        }
        else if (N5XC56GP5X4_SWAP_POSTCODER == rxSwapMsbLsb)
        {
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_SWAP, lane, 0);
            N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXD_SWAP, lane, 1);
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_SWAP_MSB_LSB *txSwapMsbLsb,
    OUT E_N5XC56GP5X4_SWAP_MSB_LSB *rxSwapMsbLsb
)
{
    MCESD_U32 txDSwapData, txDataSwapData, rxDSwapData, rxDataSwapData;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXD_SWAP, lane, txDSwapData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_SWAP, lane, txDataSwapData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXD_SWAP, lane, rxDSwapData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_SWAP, lane, rxDataSwapData);

    /* TX Swap MSB LSB */
    if ((0 == txDataSwapData) && (0 == txDSwapData))
    {
        *txSwapMsbLsb = N5XC56GP5X4_SWAP_DISABLE;
    }
    else if ((1 == txDataSwapData) && (0 == txDSwapData))
    {
        *txSwapMsbLsb = N5XC56GP5X4_SWAP_PRECODER;
    }
    else if ((0 == txDataSwapData) && (1 == txDSwapData))
    {
        *txSwapMsbLsb = N5XC56GP5X4_SWAP_POSTCODER;
    }
    else
    {
        *txSwapMsbLsb = N5XC56GP5X4_SWAP_NOT_USED;
    }

    /* RX Swap MSB LSB */
    if ((0 == rxDataSwapData) && (0 == rxDSwapData))
    {
        *rxSwapMsbLsb = N5XC56GP5X4_SWAP_DISABLE;
    }
    else if ((1 == rxDataSwapData) && (0 == rxDSwapData))
    {
        *rxSwapMsbLsb = N5XC56GP5X4_SWAP_PRECODER;
    }
    else if ((0 == rxDataSwapData) && (1 == rxDSwapData))
    {
        *rxSwapMsbLsb = N5XC56GP5X4_SWAP_POSTCODER;
    }
    else
    {
        *rxSwapMsbLsb = N5XC56GP5X4_SWAP_NOT_USED;
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_GRAY_CODE txGrayCode,
    IN E_N5XC56GP5X4_GRAY_CODE rxGrayCode
)
{
    if (N5XC56GP5X4_GRAY_NOT_USED != txGrayCode)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE0, txGrayCode));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE1, txGrayCode));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE2, txGrayCode));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE3, txGrayCode));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    if (N5XC56GP5X4_GRAY_NOT_USED != rxGrayCode)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE0, rxGrayCode));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE1, rxGrayCode));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE2, rxGrayCode));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE3, rxGrayCode));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_GRAY_CODE *txGrayCode,
    OUT E_N5XC56GP5X4_GRAY_CODE *rxGrayCode
)
{
    MCESD_U16 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE0, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE1, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE2, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_GRAY_CODE3, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_GRAY_CODE3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txGrayCode = (E_N5XC56GP5X4_GRAY_CODE)txData;
    *rxGrayCode = (E_N5XC56GP5X4_GRAY_CODE)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txState,
    IN MCESD_BOOL rxState
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE0, txState));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE0, rxState));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE1, txState));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE1, rxState));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE2, txState));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE2, rxState));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE3, txState));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwSetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE3, rxState));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txState,
    OUT MCESD_BOOL *rxState
)
{
    MCESD_U16 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE0, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE1, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE2, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_TX_PRE_CODE3, &txData));
        MCESD_ATTEMPT(API_N5XC56GP5X4_HwGetPinCfg(devPtr, N5XC56GP5X4_PIN_RX_PRE_CODE3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txState = (MCESD_BOOL)txData;
    *rxState = (MCESD_BOOL)rxData;

    return MCESD_OK;

}

MCESD_STATUS API_N5XC56GP5X4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5XC56GP5X4_PATTERN_STATS *statistics
)
{
    E_N5XC56GP5X4_DATABUS_WIDTH txWidth, rxWidth;
    MCESD_U64 frames;
    MCESD_U32 passData, lockData, errData_47_32, errData_31_0, data_47_32, data_31_0;
    MCESD_32 dataBus;

    if (statistics == NULL)
        return MCESD_FAIL;

    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_PASS, lane, passData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_LOCK, lane, lockData);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_CNT_4732, lane, data_47_32);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_CNT_3100, lane, data_31_0);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_ERR_4732, lane, errData_47_32);
    N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_ERR_3100, lane, errData_31_0);
    MCESD_ATTEMPT(API_N5XC56GP5X4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));

    frames = ((MCESD_U64)data_47_32 << 32) + data_31_0;

    if (N5XC56GP5X4_DATABUS_32BIT == rxWidth)
        dataBus = 32;
    else if (N5XC56GP5X4_DATABUS_40BIT == rxWidth)
        dataBus = 40;
    else if (N5XC56GP5X4_DATABUS_64BIT == rxWidth)
        dataBus = 64;
    else
        dataBus = 80;

    statistics->totalBits = frames * dataBus;
    statistics->totalErrorBits = ((MCESD_U64)errData_47_32 << 32) + errData_31_0;
    statistics->pass = (MCESD_BOOL)passData;
    statistics->lock = (MCESD_BOOL)lockData;

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_CNT_RST, lane, 0);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 1));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_CNT_RST, lane, 1);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 1));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_CNT_RST, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    /* Tx */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_PHYREADY, lane, 0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_EN_MODE, lane, 2);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_EN, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_RST, lane, 0);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 5));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_RST, lane, 1);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 5));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_RST, lane, 0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_PHYREADY, lane, 1);

    /* Rx */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PHYREADY, lane, 0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_EN_MODE, lane, 2);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PRBS_LOAD, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_EN, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_RST, lane, 0);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 5));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_RST, lane, 1);
    MCESD_ATTEMPT(API_N5XC56GP5X4_Wait(devPtr, 5));
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_RST, lane, 0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PHYREADY, lane, 1);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    /* Rx */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_EN, lane, 0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_PHYREADY, lane, 0);

    /* Tx */
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_EN, lane, 0);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_PHYREADY, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_PATH_SEL, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_DFEADAPT, lane, 0xF);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ADAPT_EVEN, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ADAPT_ODD, lane, 1);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_EN, lane, 1);
    N5XC56GP5X4_POLL_FIELD(devPtr, F_N5XC56GP5X4_EOM_READY, lane, 1, 5000);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_LPNUM, lane, 0xFF);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_EN, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    OUT S_N5XC56GP5X4_EOM_DATA *measurement
)
{
    MCESD_U32 vldCntPData_39_32, vldCntPData_31_00, vldCntNData_39_32, vldCntNData_31_00, errCntPData, errCntNData;

    if (NULL == measurement)
        return MCESD_FAIL;

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_PHASE, lane, phase);
    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_ESM_VOLTAGE, lane, voltage);

    N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_EOM_DFE_CALL, lane, 1);
    N5XC56GP5X4_POLL_FIELD(devPtr, F_N5XC56GP5X4_EOM_DFE_CALL, lane, 0, 1000);

    switch (eyeTMB)
    {
    case N5XC56GP5X4_EYE_TOP:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_T_P_3932, lane, vldCntPData_39_32);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_T_P_3100, lane, vldCntPData_31_00);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_T_N_3932, lane, vldCntNData_39_32);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_T_N_3100, lane, vldCntNData_31_00);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EOM_EC_T_P, lane, errCntPData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EOM_EC_T_N, lane, errCntNData);
        break;
    case N5XC56GP5X4_EYE_MID:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_M_P_3932, lane, vldCntPData_39_32);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_M_P_3100, lane, vldCntPData_31_00);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_M_N_3932, lane, vldCntNData_39_32);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_M_N_3100, lane, vldCntNData_31_00);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EOM_EC_M_P, lane, errCntPData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EOM_EC_M_N, lane, errCntNData);
        break;
    case N5XC56GP5X4_EYE_BOT:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_B_P_3932, lane, vldCntPData_39_32);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_B_P_3100, lane, vldCntPData_31_00);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_B_N_3932, lane, vldCntNData_39_32);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_VC_B_N_3100, lane, vldCntNData_31_00);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EOM_EC_B_P, lane, errCntPData);
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_EOM_EC_B_N, lane, errCntNData);
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

MCESD_STATUS API_N5XC56GP5X4_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *phaseStepCount,
    OUT MCESD_U16 *voltageStepCount
)
{
    E_N5XC56GP5X4_SERDES_SPEED txSpeed, rxSpeed;
    MCESD_U32 mbps;

    MCESD_ATTEMPT(API_N5XC56GP5X4_GetTxRxBitRate(devPtr, lane, &txSpeed, &rxSpeed));
    INT_N5XC56GP5X4_SpeedGbpsToMbps(rxSpeed, &mbps);

    if (mbps >= 16000)          /* speed >= 16.0 Gbps */
    {
        *phaseStepCount = 128;
    }
    else if (mbps >= 8000)      /* speed >= 8.0 Gbps */
    {
        *phaseStepCount = 256;
    }
    else if (mbps >= 4000)      /* speed >= 4.0 Gbps */
    {
        *phaseStepCount = 512;
    }
    else if (mbps >= 2000)      /* speed >= 2.0 Gbps */
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

MCESD_STATUS INT_N5XC56GP5X4_SpeedGbpsToMbps
(
    IN E_N5XC56GP5X4_SERDES_SPEED speed,
    OUT MCESD_U32 *mbps
)
{
    *mbps = 0;

    switch (speed)
    {
    case N5XC56GP5X4_SERDES_1P0625G:        /* 1.0625 Gbps */
        *mbps = 1062;
        break;
    case N5XC56GP5X4_SERDES_1P2288G:        /* 1.2288 Gbps */
        *mbps = 1228;
        break;
    case N5XC56GP5X4_SERDES_1P25G:          /* 1.25 Gbps */
        *mbps = 1250;
        break;
    case N5XC56GP5X4_SERDES_2P02752G:       /* 2.02752 Gbps */
        *mbps = 2027;
        break;
    case N5XC56GP5X4_SERDES_2P125G:         /* 2.125 Gbps */
        *mbps = 2125;
        break;
    case N5XC56GP5X4_SERDES_2P4576G:        /* 2.4576 Gbps */
        *mbps = 2457;
        break;
    case N5XC56GP5X4_SERDES_2P5G:           /* 2.5 Gbps */
        *mbps = 2500;
        break;
    case N5XC56GP5X4_SERDES_2P57812G:       /* 2.578125 Gbps */
        *mbps = 2578;
        break;
    case N5XC56GP5X4_SERDES_3P072G:         /* 3.072 Gbps */
        *mbps = 3072;
        break;
    case N5XC56GP5X4_SERDES_3P125G:         /* 3.125 Gbps */
        *mbps = 3125;
        break;
    case N5XC56GP5X4_SERDES_4P08804G:       /* 4.08804 Gbps */
        *mbps = 4088;
        break;
    case N5XC56GP5X4_SERDES_4P25G:          /* 4.25 Gbps */
        *mbps = 4250;
        break;
    case N5XC56GP5X4_SERDES_4P9152G:        /* 4.9152 Gbps */
        *mbps = 4915;
        break;
    case N5XC56GP5X4_SERDES_5G:            /* 5.0 Gbps */
        *mbps = 5000;
        break;
    case N5XC56GP5X4_SERDES_5P15625G:       /* 5.15625 Gbps */
        *mbps = 5156;
        break;
    case N5XC56GP5X4_SERDES_6P144G:         /* 6.144 Gbps */
        *mbps = 6144;
        break;
    case N5XC56GP5X4_SERDES_6P25G:          /* 6.25 Gbps */
        *mbps = 6250;
        break;
    case N5XC56GP5X4_SERDES_7P3728G:        /* 7.3728 Gbps */
        *mbps = 7372;
        break;
    case N5XC56GP5X4_SERDES_7P5G:           /* 7.5 Gbps */
        *mbps = 7500;
        break;
    case N5XC56GP5X4_SERDES_8P11008G:       /* 8.11008 Gbps */
        *mbps = 8110;
        break;
    case N5XC56GP5X4_SERDES_8P5G:           /* 8.5 Gbps */
        *mbps = 8500;
        break;
    case N5XC56GP5X4_SERDES_9P8304G:        /* 9.8304 Gbps */
        *mbps = 9830;
        break;
    case N5XC56GP5X4_SERDES_10G:            /* 10.0 Gbps */
        *mbps = 10000;
        break;
    case N5XC56GP5X4_SERDES_10P137G:        /* 10.137 Gbps */
        *mbps = 10137;
        break;
    case N5XC56GP5X4_SERDES_10P3125G:       /* 10.3125 Gbps */
        *mbps = 10312;
        break;
    case N5XC56GP5X4_SERDES_10P5187G:       /* 10.51875 Gbps */
        *mbps = 10518;
        break;
    case N5XC56GP5X4_SERDES_12P1651G:       /* 12.16512 Gbps */
        *mbps = 12165;
        break;
    case N5XC56GP5X4_SERDES_12P1875G:       /* 12.1875 Gbps */
        *mbps = 12187;
        break;
    case N5XC56GP5X4_SERDES_12P288G:        /* 12.288 Gbps */
        *mbps = 12288;
        break;
    case N5XC56GP5X4_SERDES_12P5G:          /* 12.5 Gbps */
        *mbps = 12500;
        break;
    case N5XC56GP5X4_SERDES_12P8906G:       /* 12.8906 Gbps */
        *mbps = 12890;
        break;
    case N5XC56GP5X4_SERDES_14P025G:        /* 14.025 Gbps */
        *mbps = 14025;
        break;
    case N5XC56GP5X4_SERDES_14P7456G:       /* 14.7456 Gbps */
        *mbps = 14745;
        break;
    case N5XC56GP5X4_SERDES_15G:            /* 15.0 Gbps */
        *mbps = 15000;
        break;
    case N5XC56GP5X4_SERDES_16P2201G:       /* 16.22016 Gbps */
        *mbps = 16220;
        break;
    case N5XC56GP5X4_SERDES_19P6608G:       /* 19.6608 Gbps */
        *mbps = 19660;
        break;
    case N5XC56GP5X4_SERDES_20P625G:        /* 20.625 Gbps */
        *mbps = 20625;
        break;
    case N5XC56GP5X4_SERDES_24P3302G:       /* 24.33024 Gbps */
        *mbps = 24330;
        break;
    case N5XC56GP5X4_SERDES_25P7812G:       /* 25.78125 Gbps */
        *mbps = 25781;
        break;
    case N5XC56GP5X4_SERDES_26P5625G:       /* 26.5625 Gbps */
        *mbps = 26562;
        break;
    case N5XC56GP5X4_SERDES_275G:           /* 27.5 Gbps */
        *mbps = 27500;
        break;
    case N5XC56GP5X4_SERDES_28P05G:         /* 28.05 Gbps */
        *mbps = 28050;
        break;
    case N5XC56GP5X4_SERDES_28P125G:        /* 28.125 Gbps */
        *mbps = 28125;
        break;
    case N5XC56GP5X4_SERDES_32G:            /* 32.0 Gbps */
        *mbps = 32000;
        break;
    case N5XC56GP5X4_SERDES_46P25G:         /* 46.25 Gbps */
        *mbps = 46250;
        break;
    case N5XC56GP5X4_SERDES_51P5625G:       /* 51.5625 Gbps */
        *mbps = 51562;
        break;
    case N5XC56GP5X4_SERDES_53P125G:        /* 53.125 Gbps */
        *mbps = 53125;
        break;
    case N5XC56GP5X4_SERDES_56G:            /* 56.0 Gbps */
        *mbps = 56000;
        break;
    case N5XC56GP5X4_SERDES_56P1G:          /* 56.1 Gbps */
        *mbps = 56100;
        break;
    case N5XC56GP5X4_SERDES_56P25G:         /* 56.25 Gbps */
        *mbps = 56250;
        break;
    case N5XC56GP5X4_SERDES_64G:            /* 64.0 Gbps */
        *mbps = 64000;
        break;
    default:
        return MCESD_FAIL;
    }

    return MCESD_OK;
}


MCESD_STATUS API_N5XC56GP5X4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *height
)
{
    MCESD_32 leftEdge, rightEdge, upperEdge, lowerEdge, maxPhase, minPhase, maxVoltage, phase, voltage;
    MCESD_U16 phaseStepCount, voltageStepCount;
    S_N5XC56GP5X4_EOM_DATA measurement;

    MCESD_ATTEMPT(API_N5XC56GP5X4_EOM1UIStepCount(devPtr, lane, &phaseStepCount, &voltageStepCount));

    maxPhase = phaseStepCount / 2;
    minPhase = -phaseStepCount / 2;
    maxVoltage = voltageStepCount;

    /* Set default edge values */
    leftEdge = minPhase;
    rightEdge = maxPhase;
    upperEdge = -maxVoltage;
    lowerEdge = maxVoltage - 1;

    MCESD_ATTEMPT(API_N5XC56GP5X4_EOMInit(devPtr, lane));

    /* Scan Left */
    for (phase = 0; phase < maxPhase; phase++)
    {
        MCESD_ATTEMPT(API_N5XC56GP5X4_EOMMeasPoint(devPtr, lane, eyeTMB, phase, 0, &measurement));

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
        MCESD_ATTEMPT(API_N5XC56GP5X4_EOMMeasPoint(devPtr, lane, eyeTMB, phase, 0, &measurement));

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
        MCESD_ATTEMPT(API_N5XC56GP5X4_EOMMeasPoint(devPtr, lane, eyeTMB, 0, (MCESD_U8)voltage, &measurement));

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

    *width = (MCESD_16)((leftEdge == rightEdge) ? 0 : (leftEdge - rightEdge - 1));
    *height = (MCESD_16)((upperEdge == lowerEdge) ? 0 : (upperEdge - lowerEdge - 1));
    MCESD_ATTEMPT(API_N5XC56GP5X4_EOMFinalize(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_ExecuteCDS
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    (void)devPtr;
    (void)lane;
    return MCESD_OK;
}

#endif /* N5XC56GP5X4 */