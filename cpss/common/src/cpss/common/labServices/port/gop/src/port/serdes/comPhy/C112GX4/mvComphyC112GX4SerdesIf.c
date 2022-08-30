/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvComphyC112GX4SerdesIf.c \
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#ifdef C112GX4
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_API.h"
#include "mcesdC112GX4_DeviceInit.h"
#include "mcesdC112GX4_HwCntl.h"
#include "mcesdC112GX4_RegRW.h"
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_main.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_SERDES_REF25MHz_SPDCHG0_LANE.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_SERDES_REF156MHz_SPDCHG0_LANE.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_SERDES_CMN.h>

#define HWS_PMD_LOG_ARRAY_DATA_SIZE 23

static GT_STATUS mvHwsComphyC112GX4SerdesOperation
(
    GT_U8                                  devNum,
    GT_U32                                 portGroup,
    GT_U32                                 serdesNum,
    MV_HWS_PORT_STANDARD                   portMode,
    MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
    GT_U32                                 *data,
    GT_U32                                 *result
)
{
    MCESD_DEV_PTR   sDev;

    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(data);
    GT_UNUSED_PARAM(result);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    switch ( operation )
    {
    case MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E:

        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x241c,(0x1<<2),(0x1<<2)));
        MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x241c,(0x1<<2),0));

        break;
    default:
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4GetDefaultParameters function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum     - system device number
* @param[in] serdesNum  - SerDes number
* @param[in] baudRate   - SerDes baud rate
* @param[out] txDefaultParameters - The default parameters for
*       the given baud rate
* @param[out] rxDefaultParameters - he default parameters for
*       the given baud rate
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4GetDefaultParameters
(
    IN  GT_U8                              devNum,
    IN  GT_UOPT                            serdesNum,
    IN  MV_HWS_SERDES_SPEED                baudRate,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT   *txDefaultParameters,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT   *rxDefaultParameters
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    MV_HWS_SERDES_SPEED         baudRateIterator;
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;

    if((txDefaultParameters == NULL) && (rxDefaultParameters == NULL))
    {
        return GT_BAD_PARAM;
    }
#ifndef WIN32
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);

#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

        /*dynamicly get the map*/
        tuneParams = mvHwsComphyTxRxTuneParamsGet(devNum,serdesNum);

         /*if not supported yet then use shared*/
        if(NULL==tuneParams)
        {
            tuneParams = serdesData->tuneParams;
        }
#else
        tuneParams = serdesData->tuneParams;
#endif

    for(baudRateIterator = (MV_HWS_SERDES_SPEED)0 ; tuneParams[baudRateIterator].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator++)
    {
        if(tuneParams[baudRateIterator].serdesSpeed == baudRate) break;
    }
    if(tuneParams[baudRateIterator].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    if(txDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&txDefaultParameters->txComphyC112G,
                            &tuneParams[baudRateIterator].txParams.txComphyC112G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&rxDefaultParameters->rxComphyC112G,
                            &tuneParams[baudRateIterator].rxParams.rxComphyC112G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
    }
#else
    GT_UNUSED_PARAM(serdesInfo);
    GT_UNUSED_PARAM(serdesData);
    GT_UNUSED_PARAM(baudRateIterator);

    if(txDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&txDefaultParameters->txComphyC112G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&rxDefaultParameters->rxComphyC112G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
    }

#endif
    return GT_OK;
}

MCESD_STATUS API_C112GX4_SetSignedTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN MCESD_32 paramValue
)
{
    E_C112GX4_POLARITY polarity = C112GX4_POLARITY_INVERTED;

    if ( param != C112GX4_TXEQ_EM_MAIN_CTRL )
    {
        polarity =  (paramValue < 0) ? C112GX4_POLARITY_INVERTED: C112GX4_POLARITY_NORMAL;
    }
    MCESD_CHECK_STATUS(API_C112GX4_SetTxEqPolarity(devPtr, lane, param, polarity));
    MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, param, ABS(paramValue)));
    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_GetSignedTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT MCESD_32 *paramValue
)
{
    MCESD_U32 val;
    E_C112GX4_POLARITY inverted;

    MCESD_ATTEMPT(API_C112GX4_GetTxEqParam(devPtr, lane, param, &val));
    if ( param == C112GX4_TXEQ_EM_MAIN_CTRL )
    {
        *paramValue = (MCESD_32)val;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_GetTxEqPolarity(devPtr, lane, param, &inverted));
        *paramValue = inverted ? (((MCESD_32)val)*(-1)): ((MCESD_32)val);
    }
    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_SetForceTxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD txFir  = F_C112GX4R1P0_TX_FIR_TAP_POL_F;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFir, state));

    return MCESD_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesManualTxConfig function
* @endinternal
*
* @brief   Config SERDES electrical parameters
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] txConfigPtr - pointer to Serdes Tx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesManualTxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.pre2)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE2_CTRL, (MCESD_32)txConfigPtr->txComphyC112G.pre2));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.pre)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE_CTRL,  (MCESD_32)txConfigPtr->txComphyC112G.pre));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.post)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_POST_CTRL, (MCESD_32)txConfigPtr->txComphyC112G.post));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.main)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_MAIN_CTRL, (MCESD_32)txConfigPtr->txComphyC112G.main));
    /** Static configurations */

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesManualRxConfig function
* @endinternal
*
* @brief   Config SERDES electrical parameters
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] RxConfigPtr - pointer to Serdes Rx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    S_C112GX4_TRAINING_TIMEOUT  timeout;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    /* Basic (CTLE) */
    /* 1st stage GM Main */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.current1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC112G.current1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_SEL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1Sel        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1Extra)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_EXTRA,        (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1Extra      ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.res1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RES1_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.res1Sel       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cap1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.cap1Sel       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cl1Ctrl)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CL1_CTRL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.cl1Ctrl       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.enMidFreq)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_EN_MID_FREQ,      (MCESD_U32)rxConfigPtr->rxComphyC112G.enMidFreq     ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cs1Mid)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CS1_MID,          (MCESD_U32)rxConfigPtr->rxComphyC112G.cs1Mid        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rs1Mid)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS1_MID,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rs1Mid        ));
    /* 1st stage TIA */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rfCtrl)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RF_CTRL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rfCtrl        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1TiaSel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_SEL,      (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1TiaSel     ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1TiaExtra)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_EXTRA,    (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1TiaExtra   ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.hpfRSel1st)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_1ST,     (MCESD_U32)rxConfigPtr->rxComphyC112G.hpfRSel1st    ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.current1TiaSel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_TIA_SEL, (MCESD_U32)rxConfigPtr->rxComphyC112G.current1TiaSel));
    /* 2nd Stage */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl2Tune)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_SEL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rl2Tune       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_TUNE,         (MCESD_U32)rxConfigPtr->rxComphyC112G.rl2Sel        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rs2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS2_SEL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rs2Sel        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.current2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT2_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC112G.current2Sel   ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cap2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.cap2Sel       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.hpfRsel2nd)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_2ND,     (MCESD_U32)rxConfigPtr->rxComphyC112G.hpfRsel2nd    ));

    /* Advanced (CDR) */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmufi)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmufi));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmuff)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmuff));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmupi)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmupi));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmupf)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmupf));

    if(NA_8BIT != rxConfigPtr->rxComphyC112G.squelch)
        MCESD_CHECK_STATUS(API_C112GX4_SetSquelchThreshold(sDev, serdesInfo->internalLane, (MCESD_16)rxConfigPtr->rxComphyC112G.squelch));

    MCESD_CHECK_STATUS(API_C112GX4_ExecuteCDS(sDev, serdesInfo->internalLane));

    /** Static configurations */

    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x2170, (0xFFFF << 16), 0x3FF << 16));

    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x2170, (0x1 << 12), 0x1   << 12));

    /* RX_TRAIN_TIMER_LANE - set to max value */
    timeout.enable = MCESD_TRUE;
    timeout.timeout = 0x1FFF;
    MCESD_CHECK_STATUS(API_C112GX4_SetTrainingTimeout(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX, &timeout));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesReset function
* @endinternal
*
* @brief   Reset serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesReset
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     analogReset,
    IN GT_BOOL     digitalReset,
    IN GT_BOOL     syncEReset
)
{
    GT_UREG_DATA data;

    /* SERDES SD RESET/UNRESET init */
    data = (analogReset == GT_TRUE) ? 0 : 1;
    /*CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18)));*/

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == GT_TRUE) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18)));

    /* SERDES SYNCE RESET init */
    data = (syncEReset == GT_TRUE) ? 0 : 1;
    /*CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18));*/

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesCoreReset function
* @endinternal
*
* @brief   Core Serdes Reset
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] coreReset              - Analog Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_UREG_DATA data;

    data = (coreReset == UNRESET) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 17), (1 << 17)));

    return GT_OK;
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] numOfSer  - number of SERDESes to configure
* @param[in] serdesArr - collection of SERDESes to configure
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesArrayPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     numOfSer,
    IN GT_UOPT                     *serdesArr,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_UOPT                            serdesIterator;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT   txDefaultParameters;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT   rxDefaultParameters;
    MV_HWS_PER_SERDES_INFO_PTR         serdesInfo;
    GT_U16                             retries = PLL_INIT_POLLING_RETRIES;
    GT_U32                             data;
    GT_U32                             mcesdEnum;
    GT_BOOL                            allStable;

    MCESD_DEV_PTR                sDev;
    E_C112GX4_SERDES_SPEED      speed = 0;
    E_C112GX4_REFFREQ           refFreq = 0;
    E_C112GX4_REFCLK_SEL        refClkSel = 0;
    E_C112GX4_DATABUS_WIDTH     dataBusWidth = 0;
    S_C112GX4_PowerOn           powerOnConfig;
    MV_HWS_SERDES_SPEED          baudRate = serdesConfigPtr->baudRate;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);
    if(powerUp)
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Power-on lane */
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_C112GX4_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_C112GX4_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_C112GX4_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_C112GX4_DATABUS_WIDTH)mcesdEnum;
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);

            CHECK_STATUS(mvHwsComphyC112GX4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], UNRESET));

            powerOnConfig.powerLaneMask = 1 << serdesInfo->internalLane;
            powerOnConfig.initTx        = GT_TRUE;
            powerOnConfig.initRx        = GT_TRUE;
            powerOnConfig.txOutputEn    = GT_FALSE;
            powerOnConfig.dataPath      = C112GX4_PATH_EXTERNAL;
            powerOnConfig.downloadFw    = GT_FALSE;

            powerOnConfig.refClkSel     = refClkSel;
            powerOnConfig.dataBusWidth  = dataBusWidth;
            powerOnConfig.speed         = speed;
            powerOnConfig.refFreq       = refFreq;

            MCESD_CHECK_STATUS(API_C112GX4_PowerOnSeq(sDev, powerOnConfig));
        }

        /* Wait for stable PLLs */
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[0],NULL,NULL,&sDev));
        do
        {
            allStable = GT_TRUE;
            for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
            {
                CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SDW_LANE_STATUS_0, &data, PLL_INIT_POLLING_DONE));
                if (data != PLL_INIT_POLLING_DONE)
                {
                    allStable = GT_FALSE;
                    break;
                }
            }
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            retries--;
        } while ((retries != 0) && (allStable == GT_FALSE));
        if(allStable == GT_FALSE)
        {
            return GT_TIMEOUT;
        }

        for(serdesIterator = 0; serdesIterator < numOfSer; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Config Tx/Rx parameters */
            MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(serdesConfigPtr->opticalMode, baudRate);
            CHECK_STATUS(mvHwsComphyC112GX4GetDefaultParameters(devNum, serdesArr[serdesIterator], baudRate, &txDefaultParameters, &rxDefaultParameters));
#if 1
            CHECK_STATUS(mvHwsComphyC112GX4SerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txDefaultParameters));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxDefaultParameters));
#endif
            CHECK_STATUS(mvHwsComphyC112GX4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));
            MCESD_CHECK_STATUS(API_C112GX4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));
        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));
            MCESD_CHECK_STATUS(API_C112GX4_SetForceTxPolarity(sDev, serdesInfo->internalLane, MCESD_FALSE));
            MCESD_CHECK_STATUS(API_C112GX4_PowerOffLane(sDev, serdesInfo->internalLane));
            /* freq offset read request */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<16),(0x0<<16)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 2));
            /* freq offset reset*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<19),(0x1<<19)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<19),0));
            /* freq offset disable*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<20),(0x0<<20)));
}
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPowerCtrl function
* @endinternal
*
* @brief   Power up/down SERDES list.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyC112GX4SerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyC112GX4SerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Start Rx auto-tuning
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  training  - Start/Stop training
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C112GX4_StartTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C112GX4_StopTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Start Tx auto-tuning
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  training  - Start/Stop training
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C112GX4_StartTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C112GX4_StopTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Stop Tx auto-tuning
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyC112GX4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneStart function
* @endinternal
*
* @brief   Start Tx/Rx auto-tuning
*
* @param[in]  devNum     - system device number
* @param[in]  portGroup  - port group (core) number
* @param[in]  serdesNum  - serdes number
* @param[in]  rxTraining - Start/Stop Rx training
* @param[in]  txTraining - Start/Stop Tx training
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
)
{
    if(rxTraining)
    {
        CHECK_STATUS(mvHwsComphyC112GX4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyC112GX4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_TRUE;
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneStatus function
* @endinternal
*
* @brief   Get Rx and Tx auto-tuning status
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out] rxStatus  - Rx auto-tuning status
* @param[out] txStatus  - Tx auto-tuning status
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    MCESD_BOOL                  completed;
    MCESD_BOOL                  failed;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(NULL != rxStatus)
    {
        MCESD_CHECK_STATUS(API_C112GX4_CheckTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C112GX4_CheckTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Get Rx and Tx auto-tuning status
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out] rxStatus  - Rx auto-tuning status
* @param[out] txStatus  - Tx auto-tuning status
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    MCESD_U16                   completeData, failedData = 0;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(NULL != rxStatus)
    {
        MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_RX_TRAIN_COMPLETE0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_RX_TRAIN_FAILED0 + serdesInfo->internalLane, &failedData));
        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_TX_TRAIN_COMPLETE0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_TX_TRAIN_FAILED0 + serdesInfo->internalLane, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneResult function
* @endinternal
*
* @brief   Get Rx and Tx auto-tuning results
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out] results  - auto-tuning results
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MV_HWS_PER_SERDES_INFO_PTR              serdesInfo;
    MCESD_DEV_PTR                           sDev;
    MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS  *results;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U8  val8;
    MCESD_16  vals16;
    MCESD_U32 tapsI;
    MCESD_32 txParam;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC112GResults;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE2_CTRL, &txParam));
    results->txComphyC112G.pre2 = (GT_8)txParam;
    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE_CTRL,  &txParam));
    results->txComphyC112G.pre  = (GT_8)txParam;
    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_MAIN_CTRL, &txParam));
    results->txComphyC112G.main = (GT_8)txParam;
    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_POST_CTRL, &txParam));
    results->txComphyC112G.post = (GT_8)txParam;

    /* Basic (CTLE) */
    /* 1st stage GM Main */
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_SEL, &val));
    results->rxComphyC112G.current1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_SEL,      &val));
    results->rxComphyC112G.rl1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_EXTRA,    &val));
    results->rxComphyC112G.rl1Extra= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RES1_SEL,     &val));
    results->rxComphyC112G.res1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL,     &val));
    results->rxComphyC112G.cap1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CL1_CTRL,     &val));
    results->rxComphyC112G.cl1Ctrl= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_EN_MID_FREQ,  &val));
    results->rxComphyC112G.enMidFreq= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CS1_MID,      &val));
    results->rxComphyC112G.cs1Mid= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS1_MID,      &val));
    results->rxComphyC112G.rs1Mid= (GT_U32)val;
    /* 1st stage TIA */
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RF_CTRL,          &val));
    results->rxComphyC112G.rfCtrl= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_SEL,      &val));
    results->rxComphyC112G.rl1TiaSel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_EXTRA,    &val));
    results->rxComphyC112G.rl1TiaExtra= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_1ST,     &val));
    results->rxComphyC112G.hpfRSel1st= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_TIA_SEL, &val));
    results->rxComphyC112G.current1TiaSel= (GT_U32)val;
    /* 2nd Stage */
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_SEL,      &val));
    results->rxComphyC112G.rl2Tune= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_TUNE,     &val));
    results->rxComphyC112G.rl2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS2_SEL,      &val));
    results->rxComphyC112G.rs2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT2_SEL, &val));
    results->rxComphyC112G.current2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL,     &val));
    results->rxComphyC112G.cap2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_2ND, &val));
    results->rxComphyC112G.hpfRsel2nd= (GT_U32)val;

    /* Advanced (CDR) */
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFI,  &val));
    results->rxComphyC112G.selmufi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFF,  &val));
    results->rxComphyC112G.selmuff= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPI,  &val));
    results->rxComphyC112G.selmupi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPF,  &val));
    results->rxComphyC112G.selmupf= (GT_U32)val;

    MCESD_CHECK_STATUS(API_C112GX4_GetSquelchThreshold(sDev, serdesInfo->internalLane, &val8));
    results->rxComphyC112G.squelch= (MCESD_U8)val8;

    MCESD_CHECK_STATUS(API_C112GX4_GetAlign90(sDev, serdesInfo->internalLane, &val, &vals16));
    results->align90AnaReg = (GT_U32)val;
    results->align90 = (GT_16)vals16;

    results->eo = 0;
    results->sampler = 0;
    results->slewRateCtrl0 = 0;
    results->slewRateCtrl1 = 0;

    for(tapsI = 0 ; tapsI < C112GX4_DFE_FF7 ; tapsI++)
    {
        if (MCESD_OK == API_C112GX4_GetDfeTap(sDev, serdesInfo->internalLane, C112GX4_EYE_MID, (E_C112GX4_DFE_TAP)tapsI, &vals))
        {
            results->dfe[tapsI] = (GT_32)vals;
        }
        else
        {
            results->dfe[tapsI] = 0xffff;
        }

    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesDigitalReset function
* @endinternal
*
* @brief   Run digital reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesDigitalReset
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        digitalReset
)
{
    GT_UREG_DATA data;

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == RESET) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18)));

    return GT_OK;
}
/**
* @internal mvHwsComphyC112GX4SerdesLoopback function
* @endinternal
*
* @brief   Set SerDes loopback mode
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  lbMode    - serdes looback mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C112GX4_DATAPATH          path;
    GT_U32                      mcesdEnum;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_C112GX4_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_C112GX4_SetDataPath(sDev, serdesInfo->internalLane, path));

    switch ( lbMode )
    {
    case SERDES_LP_AN_TX_RX:
        /* Masks sq_detected indication. If set, SQ detect is always zero (signal detected). Masks sq_detected indication */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (1 << 25), (1 << 25)));
        break;
    default:
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (0 << 25), (1 << 25)));
        break;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesLoopbackGet function
* @endinternal
*
* @brief   Get SerDes loopbabk mode
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out]  lbMode    - serdes looback mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C112GX4_DATAPATH          path;
    GT_U32                      tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetDataPath(sDev, serdesInfo->internalLane, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTestGen
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN  MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    MV_HWS_PER_SERDES_INFO_PTR    serdesInfo;
    MCESD_DEV_PTR                 sDev;
    E_C112GX4_PATTERN             pattern;
    E_C112GX4_SERDES_SPEED        speed;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U8)txPattern, &mcesdPattern));
    pattern = (E_C112GX4_PATTERN)mcesdPattern;

    switch (txPattern)
    {
        case _1T:
            userPattern = pattern1T;
            break;
        case _2T:
            userPattern = pattern2T;
            break;
        case _5T:
            userPattern = pattern5T;
            break;
        case _10T:
            userPattern = pattern10T;
            break;
        default:
            userPattern = patternEmpty;
            break;
    }
    MCESD_CHECK_STATUS(API_C112GX4_SetTxRxPattern(sDev, serdesInfo->internalLane, pattern, pattern, userPattern));
    MCESD_CHECK_STATUS(API_C112GX4_GetTxRxBitRate(sDev, serdesInfo->internalLane,&speed));

    if (txPattern == PRBS13)
    {
        if (speed == C112GX4_SERDES_53P125G)
        {
            MCESD_CHECK_STATUS(API_C112GX4_SetGrayCode(sDev, serdesInfo->internalLane,
                                                       ((SERDES_TEST == mode) ? C112GX4_GRAY_CODE_ENABLE : C112GX4_GRAY_CODE_DISABLE),
                                                       ((SERDES_TEST == mode) ? C112GX4_GRAY_CODE_ENABLE : C112GX4_GRAY_CODE_DISABLE)));
        }

        /* PT_PRBS_INV_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2304, (0x1 << 23), ((SERDES_TEST == mode) ? 0x0 : (0x1 << 23))));

    }

    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_C112GX4_ResetComparatorStats(sDev, serdesInfo->internalLane));
        MCESD_CHECK_STATUS(API_C112GX4_StartPhyTest(sDev, serdesInfo->internalLane));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C112GX4_StopPhyTest(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTestGenGet function
* @endinternal
*
* @brief  Get configuration of the Serdes test
*         generator/checker. Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txPatternPtr             - pattern to transmit ("Other" means any mode not
*                                      included explicitly in MV_HWS_SERDES_TX_PATTERN type)
* @param[out] modePtr                  - test mode or normal
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTestGenGet
(
    GT_U8                        devNum,
    GT_UOPT                      portGroup,
    GT_UOPT                      serdesNum,
    MV_HWS_SERDES_TX_PATTERN     *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE  *modePtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR        serdesInfo;
    MCESD_DEV_PTR                     sDev;
    E_C112GX4_PATTERN                 txPattern;
    E_C112GX4_PATTERN                 rxPattern;
    char                              userPattern[21];
    MCESD_FIELD                       ptEn = F_C112GX4R1P0_PT_EN;
    MCESD_U32                         enable;
    GT_U32                            tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C112GX4_GetTxRxPattern(sDev, serdesInfo->internalLane, &txPattern, &rxPattern, userPattern));
    MCESD_CHECK_STATUS(API_C112GX4_ReadField(sDev, serdesInfo->internalLane, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyC112GX4SerdesTestGenStatus
 function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Enable/Disable reset the accumulation of error counters
*
* @param[out] status                   - test generator status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTestGenStatus
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN        txPattern,
    IN GT_BOOL                         counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
    MV_HWS_PER_SERDES_INFO_PTR        serdesInfo;
    MCESD_DEV_PTR                     sDev;
    S_C112GX4_PATTERN_STATISTICS     statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C112GX4_GetComparatorStats(sDev, serdesInfo->internalLane, &statistics));

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr   = (GT_U32)statistics.totalErrorBits;
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_C112GX4_ResetComparatorStats(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPolarityConfig function
* @endinternal
*
* @brief   Set Tx/Rx polarity invert
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  invertTx  - if true - invert Tx polarity
* @param[in]  invertRx  - if true - invert Rx polarity
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_SetTxRxPolarity(sDev, serdesInfo->internalLane, invertTx ? C112GX4_POLARITY_INVERTED : C112GX4_POLARITY_NORMAL,
                                                        invertRx ? C112GX4_POLARITY_INVERTED : C112GX4_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPolarityConfigGet
*           function
* @endinternal
*
* @brief   Get Tx/Rx polarity invert
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out]  invertTx  - if true - Tx polarity is inverted
* @param[out]  invertRx  - if true - Rx polarity is inverted
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesPolarityConfigGet
(
  GT_U8             devNum,
  GT_UOPT           portGroup,
  GT_UOPT           serdesNum,
  GT_BOOL           *invertTx,
  GT_BOOL           *invertRx
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C112GX4_POLARITY         txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetTxRxPolarity(sDev, serdesInfo->internalLane, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == C112GX4_POLARITY_INVERTED);
    *invertRx = (rxPolarity == C112GX4_POLARITY_INVERTED);

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxEnbale function
* @endinternal
*
* @brief   Set Tx signal enbale
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  txEnable  - if true - enable Tx
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTxEnbale
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL txEnable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_SetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxEnbale function
* @endinternal
*
* @brief   Set Tx signal enbale
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  txEnable  - if true - enable Tx
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTxEnbaleGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *txEnable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL*)txEnable));

    return GT_OK;
}


/**
* @internal mvHwsComphyC112GX4SerdesSignalDetectGet function
* @endinternal
*
* @brief   Check if signal is present
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number to get its signal state
* @param[out] enable    - return signal detect value
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesSignalDetectGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    MCESD_BOOL                  squelched;
    GT_U32                      data = 0;

    GT_UNUSED_PARAM(portGroup);

    /* in serdes loopback analog tx2rx, we forced signal detect in mvHwsComphyC112GX4SerdesLoopback */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, &data, (1 << 25)));
    if ( data != 0 )
    {
        *enable = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetSquelchDetect(sDev, serdesInfo->internalLane, &squelched));
    *enable = squelched ? GT_FALSE : GT_TRUE;

    return GT_OK;
}


GT_STATUS mvHwsComphyC112GX4SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *lock
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      data = 0, isPam2, clampingTrigger=0,freqReady=0, rxTrain;
    GT_16                       dataSigned = 0,dataSigned2/*,ppmVal= 0*/;

    /* check signal */
    MCESD_CHECK_STATUS(mvHwsComphyC112GX4SerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    if (*lock == GT_TRUE)
    {
        *lock = GT_FALSE;
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

        /* read freq offset ready*/
        MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,31,1,&freqReady));
        if (freqReady == 0)
        {
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2f04,17,1,&rxTrain));
            if ( rxTrain != 0)
            {
                *lock = GT_TRUE;
                return GT_OK;
            }
            MCESD_CHECK_STATUS(API_C112GX4_RxInit(sDev, serdesInfo->internalLane));

            /* freq offset enable*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<20),(0x1<<20)));
            /* freq offset reset*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<19),(0x1<<19)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<19),0));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            /* freq offset read request */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<16),(0x1<<16)));
            /* read freq offset ready*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,31,1,&freqReady));
        }

        /* check PAM2/PAM4*/
        MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2034,29,1,&isPam2));
        if ( isPam2 == 0)
        {   /* PAM4 */
            /* clear dtl clamping */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<12),(0x1<<12)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<12),(0x0)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 2));

            /* read dtl clamping triggerd*/
            MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,15,1,&clampingTrigger));
        }

        if ( freqReady == 1 )
        {
            if ( clampingTrigger == 1 )
            {
                if ( isPam2 == 0)
                {
                    *lock = GT_TRUE;
                }
            }
            else
            {
                /* read freq value twice and check if changed*/
                MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,18,13,&data));
                dataSigned = data;
                MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 2));
                MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,18,13,&data));
                dataSigned2= data;
                if (dataSigned == dataSigned2 )
                {
                    if ( data & 0x1000 )
                    {
                        dataSigned |= 0xe000;
                    }
                    if ( ABS(dataSigned) < 50 )
                    {
                        *lock = GT_TRUE;
                        /* ppmVal = dataSigned;
                        printf("cdrLockGetPrint serdes %d ppm %d lock %d \n",serdesNum, ppmVal, *lock);*/
                    }
                    /*else
                    {
                        printf("cdrLockGetPrint fail serdes %d ppm %d lock %d\n",serdesNum, ABS(dataSigned), *lock);
                    }*/
                }
            }
        }
        else
        {
            if ( clampingTrigger==1)
            {
                *lock = GT_TRUE;
            }
        }
        if (*lock == GT_FALSE)
        {
            /* reset dfe */
            mvHwsComphyC112GX4SerdesOperation(devNum,0, serdesNum,NON_SUP_MODE /* not used*/,MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E, NULL,NULL);
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesFirmwareDownloadIter
*           function
* @endinternal
*
* @brief   Download FW
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - num of serdeses in serdesArr
* @param[in] fwCodePtr         - firmware buffer
* @param[in] fwCodeSize        - firmware size
* @param[in] cmnXDataPtr       - cmnXData bufer
* @param[in] cmnXDataSize      - cmnXData size
* @param[in] laneXDataPtr      - laneXData buffer
* @param[in] laneXDataSize     - laneXData size
* @param[in] refClkSel         - Serdes reference clock select
* @param[in] refFreq           - SerDes reference clock frquency
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesFirmwareDownloadIter
(
    GT_U8                 devNum,
    GT_UOPT               *serdesArr,
    GT_U8                 numOfSerdeses,
    MCESD_U32             *fwCodePtr,
    MCESD_U32             fwCodeSize,
    MCESD_U32             *cmnXDataPtr,
    MCESD_U32             cmnXDataSize,
    MCESD_U32             *laneXDataPtr,
    MCESD_U32             laneXDataSize,
    E_C112GX4_REFCLK_SEL  refClkSel,
    E_C112GX4_REFFREQ     refFreq
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U8 serdesIterator;

    for(serdesIterator = 0; serdesIterator < numOfSerdeses; serdesIterator++)
    {
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesArr[serdesIterator], &serdesInfo, NULL, &sDev));
        if(serdesInfo->internalLane != 0)
        {
            return GT_BAD_PARAM;
        }
        /* Set SERDES Mode */
        MCESD_CHECK_STATUS(API_C112GX4_SetPhyMode(sDev, C112GX4_PHYMODE_SERDES));
        /* Configure Reference Frequency and reference clock source group */
        MCESD_CHECK_STATUS(API_C112GX4_SetRefFreq(sDev, refFreq, refClkSel));
        /* Power up current and voltage reference */
        MCESD_CHECK_STATUS(API_C112GX4_SetPowerIvRef(sDev, MCESD_TRUE));

        MCESD_CHECK_STATUS(API_C112GX4_DownloadFirmwareProgXData(sDev,
                                          fwCodePtr,    fwCodeSize,
                                          cmnXDataPtr,  cmnXDataSize,
                                          laneXDataPtr, laneXDataSize));
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesFirmwareDownload function
* @endinternal
*
* @brief   Download FW
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - num of serdeses in serdesArr
* @param[in] firmwareSelector  - FW to download
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesFirmwareDownload
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     *serdesArr,
    GT_U8       numOfSerdeses,
    GT_U8       firmwareSelector
)
{
    MCESD_U16              errCode;
    GT_STATUS              rc = GT_OK;

    E_C112GX4_REFCLK_SEL   refClkSel = C112GX4_REFCLK_SEL_GROUP1;
    E_C112GX4_REFFREQ      refFreq;
    GT_BOOL                freeNeeded = GT_FALSE;

    MCESD_U32 *fwCodePtr;
    MCESD_U32 fwCodeSize;

    MCESD_U32 *cmnXDataPtr;
    MCESD_U32 cmnXDataSize;

    MCESD_U32 *laneXDataPtr;
    MCESD_U32 laneXDataSize;

    GT_UNUSED_PARAM(portGroup);
    switch(firmwareSelector)
    {
        case MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE:
        case MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE:
            fwCodePtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C112GX4_FW_MAX_SIZE);
            if ( fwCodePtr == NULL )
            {
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//main.dat",
                                        fwCodePtr,       C112GX4_FW_MAX_SIZE,         &fwCodeSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }

            cmnXDataPtr  = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C112GX4_XDATA_MAX_SIZE);
            if ( cmnXDataPtr == NULL )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//cmn.dat",
                                        cmnXDataPtr,     C112GX4_XDATA_MAX_SIZE,  &cmnXDataSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                hwsOsFreeFuncPtr(cmnXDataPtr);
                return GT_FAIL;
            }

            laneXDataPtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C112GX4_XDATA_MAX_SIZE);
            if ( laneXDataPtr == NULL )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                hwsOsFreeFuncPtr(cmnXDataPtr);
                return GT_FAIL;
            }
            if(firmwareSelector == MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE)
            {
                refFreq = C112GX4_REFFREQ_156P25MHZ;
                if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//lane_156.dat",
                                            laneXDataPtr,    C112GX4_XDATA_MAX_SIZE, &laneXDataSize, &errCode) != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(fwCodePtr);
                    hwsOsFreeFuncPtr(cmnXDataPtr);
                    hwsOsFreeFuncPtr(laneXDataPtr);
                    return GT_FAIL;
                }
            }
            else
            {
                refFreq = C112GX4_REFFREQ_25MHZ;
                if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//lane_25.dat",
                                            laneXDataPtr,    C112GX4_XDATA_MAX_SIZE, &laneXDataSize, &errCode) != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(fwCodePtr);
                    hwsOsFreeFuncPtr(cmnXDataPtr);
                    hwsOsFreeFuncPtr(laneXDataPtr);
                    return GT_FAIL;
                }
            }

            freeNeeded = GT_TRUE;
            break;
        case MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE:
            fwCodePtr     = (MCESD_U32*)C112GX4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_C112GX4_MAIN_DATA_IMAGE_SIZE;

            cmnXDataPtr   = (MCESD_U32*)C112GX4_SERDES_CMN_Data;
            cmnXDataSize  = (MCESD_U32)COMPHY_C112GX4_SERDES_CMN_DATA_IMAGE_SIZE;

            laneXDataPtr  = (MCESD_U32*)C112GX4_SERDES_REF25MHz_SPDCHG0_LANE_Data;
            laneXDataSize = (MCESD_U32)COMPHY_C112GX4_SERDES_REF25MHZ_SPDCHG0_LANE_DATA_IMAGE_SIZE;

            refFreq       = C112GX4_REFFREQ_25MHZ;
            break;
        case MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE:
            fwCodePtr     = (MCESD_U32*)C112GX4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_C112GX4_MAIN_DATA_IMAGE_SIZE;

            cmnXDataPtr   = (MCESD_U32*)C112GX4_SERDES_CMN_Data;
            cmnXDataSize  = (MCESD_U32)COMPHY_C112GX4_SERDES_CMN_DATA_IMAGE_SIZE;

            laneXDataPtr  = (MCESD_U32*)C112GX4_SERDES_REF156MHz_SPDCHG0_LANE_Data;
            laneXDataSize = (MCESD_U32)COMPHY_C112GX4_SERDES_REF156MHZ_SPDCHG0_LANE_DATA_IMAGE_SIZE;

            refFreq       = C112GX4_REFFREQ_156P25MHZ;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    comphyPrintf("--fwCodeSize[%d]-cmnXDataSize[%d]-laneXDataSize[%d]--\n",fwCodeSize, cmnXDataSize,laneXDataSize);
    if ( freeNeeded )
    {
        hwsOsPrintf("Using SerDes FW from file: fwCodeSize=[%d], cmnXDataSize=[%d], laneXDataSize=[%d]\n",fwCodeSize, cmnXDataSize,laneXDataSize);
    }
    rc = mvHwsComphyC112GX4SerdesFirmwareDownloadIter(devNum, serdesArr, numOfSerdeses,
                                                       fwCodePtr, fwCodeSize,
                                                       cmnXDataPtr, cmnXDataSize,
                                                       laneXDataPtr, laneXDataSize,
                                                       refClkSel, refFreq);

    if ( freeNeeded )
    {
        hwsOsFreeFuncPtr(fwCodePtr);
        hwsOsFreeFuncPtr(cmnXDataPtr);
        hwsOsFreeFuncPtr(laneXDataPtr);
    }

    return rc;
}


/**
* @internal mvHwsComphyC28GP4X4AnpPowerUp function
* @endinternal
*
* @brief   Prepare Comphy Serdes for ANP operation.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] powerUp   - power up status
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4AnpPowerUp
(
    IN GT_U8    devNum,
    IN  GT_UOPT portGroup,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR   serdesInfo;
    MCESD_DEV_PTR                sDev;
    GT_U32 regData = (powerUp == GT_TRUE) ? 1 : 0;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesConfigPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_SetLaneEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)powerUp));
    MCESD_CHECK_STATUS(API_C112GX4_SetDataBusWidth(sDev, serdesInfo->internalLane, C112GX4_DATABUS_40BIT, C112GX4_DATABUS_40BIT));

    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2170, (0xFFFF << 16), 0x3FF << 16));
    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2170, (1<<12), (regData<<12)));

    MCESD_CHECK_STATUS(API_C112GX4_SetMcuEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)powerUp));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTemperatureGet function
* @endinternal
*
* @brief   Read SD temperature
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] serdesTemperature       - (pointer to) temperature
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesTemperatureGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  portGroup,
    IN  GT_UOPT  serdesNum,
    OUT GT_32    *serdesTemperature
)
{
    MCESD_DEV_PTR   sDev;
    MCESD_32        temp;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetTemperature(sDev, &temp));
    *serdesTemperature = (GT_32)temp;

    return GT_OK;
}


/**
* @internal mvHwsComphyC112GX4SerdesDbTxConfigGet function
* @endinternal
*
* @brief   Get config SERDES electrical parameters from DB
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[out] txConfigPtr - pointer to Serdes Tx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesDbTxConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_SPEED                 baudRate,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvHwsComphyC112GX4GetDefaultParameters(devNum, serdesNum, baudRate, txConfigPtr, NULL));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesDbRxConfigGet function
* @endinternal
*
* @brief   Get config SERDES electrical parameters from DB
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[out] txConfigPtr - pointer to Serdes Tx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesDbRxConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_SPEED                 baudRate,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvHwsComphyC112GX4GetDefaultParameters(devNum, serdesNum, baudRate, NULL, rxConfigPtr));

    /*TODO - get override params*/
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPMDLogGet function
* @endinternal
*
* @brief   Display's Tx Training Log
*
* @param[in] devNum                   - physical device number
* @param[in] serdesNum                - serdes number
*
* @param[out] validEntriesPtr         - (pointer to) number of
*                                        valid entries
* @param[out] pmdLogStr               - (pointer to) struct
*                                       aligned to string
* @param[in] pmdLogStrSize            - max size of pmdLogStr
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesPMDLogGet
(
    IN  GT_U8                  devNum,
    IN  GT_UOPT                serdesNum,
    OUT GT_U32                 *validEntriesPtr,
    OUT GT_CHAR                *pmdLogStr,
    IN  GT_U32                 pmdLogStrSize
)
{
    MV_HWS_PER_SERDES_INFO_PTR      serdesInfo;
    MCESD_DEV_PTR                   sDev;
    S_C112GX4_TRX_TRAINING_LOGENTRY logArrayData[HWS_PMD_LOG_ARRAY_DATA_SIZE];
    MCESD_U32                       logArraySizeEntries = HWS_PMD_LOG_ARRAY_DATA_SIZE;
    GT_U32                          i, shift = 0, offset = 0;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(validEntriesPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    hwsOsMemSetFuncPtr(logArrayData, 0, sizeof(S_C112GX4_TRX_TRAINING_LOGENTRY) * logArraySizeEntries);

    MCESD_CHECK_STATUS(API_C112GX4_DisplayTrainingLog(sDev, serdesInfo->internalLane, logArrayData, logArraySizeEntries, validEntriesPtr));

    if ( logArrayData[0].isPAM2 )
    {
        shift  = hwsSprintf(&pmdLogStr[offset], "PAM2\n====\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "Entry # | patternType | txPresetIndex | localCtrlG1 | localCtrlG0 | localCtrlGN1 | remoteStatusG1 | remoteStatusG0 | remoteStatusGN1 | errorCode\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "-------------------------------------------------------------------------------------------------------------------------------------------------\n");
        offset += shift;
    }
    else
    {
        shift  = hwsSprintf(&pmdLogStr[offset], "PAM4\n====\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "Entry # | txPresetIndex | localCtrlPat | PRE2 | PRE | MAIN | POST | remoteStatusAck | remoteStatusG\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "----------------------------------------------------------------------------------------------------\n");
        offset += shift;
    }

    for (i = 0; i < *validEntriesPtr; i++)
    {
        if ( offset >  pmdLogStrSize - 200 )
        {
            hwsSprintf(&pmdLogStr[offset - 1], "out of space for next entry\n\0");
            return GT_OK;
        }
        shift = hwsSprintf(&pmdLogStr[offset], (i < 10) ? "%d       |" : "%d      |", i);
        offset += shift;
        if ( logArrayData[i].isPAM2 )
        {
            shift = hwsSprintf(&pmdLogStr[offset], (logArrayData[i].pam2.patternType) ? "PAT0         |" : "PAT1         |");
            offset += shift;

            switch ( logArrayData[i].pam2.txPresetIndex )
            {
            case C112GX4_TRX_LOG_TX_PRESET_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA             |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET1:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET1        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET2:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET2        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET3:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET3        |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.localCtrlG1 )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD         |");
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                shift = hwsSprintf(&pmdLogStr[offset], "INC          |");
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                shift = hwsSprintf(&pmdLogStr[offset], "DEC          |");
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA           |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.localCtrlG0 )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD         |");
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                shift = hwsSprintf(&pmdLogStr[offset], "INC          |");
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                shift = hwsSprintf(&pmdLogStr[offset], "DEC          |");
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA           |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.localCtrlGN1 )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD          |");
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                shift = hwsSprintf(&pmdLogStr[offset], "INC           |");
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                shift = hwsSprintf(&pmdLogStr[offset], "DEC           |");
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA            |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.remoteStatusG1 )
            {
            case C112GX4_TRX_LOG_PAM2_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD            |");
                break;
            case C112GX4_TRX_LOG_PAM2_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED         |");
                break;
            case C112GX4_TRX_LOG_PAM2_MIN:
                shift = hwsSprintf(&pmdLogStr[offset], "MIN             |");
                break;
            case C112GX4_TRX_LOG_PAM2_MAX:
                shift = hwsSprintf(&pmdLogStr[offset], "MAX             |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.remoteStatusG0 )
            {
            case C112GX4_TRX_LOG_PAM2_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD            |");
                break;
            case C112GX4_TRX_LOG_PAM2_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED         |");
                break;
            case C112GX4_TRX_LOG_PAM2_MIN:
                shift = hwsSprintf(&pmdLogStr[offset], "MIN             |");
                break;
            case C112GX4_TRX_LOG_PAM2_MAX:
                shift = hwsSprintf(&pmdLogStr[offset], "MAX             |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.remoteStatusGN1 )
            {
            case C112GX4_TRX_LOG_PAM2_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD             |");
                break;
            case C112GX4_TRX_LOG_PAM2_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED          |");
                break;
            case C112GX4_TRX_LOG_PAM2_MIN:
                shift = hwsSprintf(&pmdLogStr[offset], "MIN              |");
                break;
            case C112GX4_TRX_LOG_PAM2_MAX:
                shift = hwsSprintf(&pmdLogStr[offset], "MAX              |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], "%d\n", logArrayData[i].pam2.errorCode);
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], "-------------------------------------------------------------------------------------------------------------------------------------------------\n");
            offset += shift;
        } /* logArrayData[i].isPAM2 */
        else /* PAM4*/
        {
            GT_CHAR temp[5];

            switch ( logArrayData[i].pam4.txPresetIndex )
            {
            case C112GX4_TRX_LOG_TX_PRESET_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA             |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET1:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET1        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET2:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET2        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET3:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET3        |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam4.localCtrlPat )
            {
            case C112GX4_TRX_LOG_PAT_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA            |");
                break;
            case C112GX4_TRX_LOG_PAT_GRAY_CODE:
                shift = hwsSprintf(&pmdLogStr[offset], "GRAY_CODE     |");
                break;
            case C112GX4_TRX_LOG_PAT_PRE_CODE:
                shift = hwsSprintf(&pmdLogStr[offset], "PRE_CODE     |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam4.localCtrlG )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                hwsOsMemCopyFuncPtr(temp, "HOLD\0", 5 * sizeof(GT_CHAR));
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                hwsOsMemCopyFuncPtr(temp, "INC \0", 5 * sizeof(GT_CHAR));
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                hwsOsMemCopyFuncPtr(temp, "DEC \0", 5 * sizeof(GT_CHAR));
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                hwsOsMemCopyFuncPtr(temp, "NA  \0", 5 * sizeof(GT_CHAR));
                break;
            default:
                return GT_BAD_PARAM;
            }
            switch ( logArrayData[i].pam4.localCtrlSel )
            {
            case C112GX4_TRX_LOG_SEL_MAIN:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD  |HOLD |%s  |HOLD  |", temp);
                break;
            case C112GX4_TRX_LOG_SEL_POST:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD  |HOLD |HOLD  |%s  |", temp);
                break;
            case C112GX4_TRX_LOG_SEL_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA    |NA   |NA    |NA    |");
                break;
            case C112GX4_TRX_LOG_SEL_PRE2:
                shift = hwsSprintf(&pmdLogStr[offset], "%s  |HOLD |HOLD  |HOLD  |", temp);
                break;
            case C112GX4_TRX_LOG_SEL_PRE1:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD  |%s |HOLD  |HOLD  |", temp);
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], (logArrayData[i].pam4.remoteStatusAck == C112GX4_TRX_LOG_ACK_HOLD) ? "HOLD             |" : "UPDATED          |");
            offset += shift;
            switch ( logArrayData[i].pam4.remoteStatusG )
            {
            case C112GX4_TRX_LOG_PAM4_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD          |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED       |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_HIT_LIMIT:
                shift = hwsSprintf(&pmdLogStr[offset], "HIT_LIMIT     |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_NOTSUPPORT:
                shift = hwsSprintf(&pmdLogStr[offset], "NOTSUPPORT    |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA            |\n");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], "----------------------------------------------------------------------------------------------------\n");
            offset += shift;
        } /* PAM4*/
    }/* end for */

    hwsSprintf(&pmdLogStr[offset], "\0");

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesEncodingTypeGet function
* @endinternal
*
* @brief   Retrieves the Tx and Rx line encoding values
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
*
* @param[out] txEncodingPtr           - NRZ/PAM4
* @param[out] rxEncodingPtr           - NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesEncodingTypeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           serdesNum,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD rxPAM2En = F_C112GX4R1P0_RX_PAM2_EN;
    MCESD_U32 data;
    MCESD_BOOL txReady, rxReady;

    if( NULL == txEncodingPtr || NULL == rxEncodingPtr )
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* check if serdes powered-up */
    MCESD_ATTEMPT(API_C112GX4_GetTxRxReady(sDev, serdesInfo->internalLane, &txReady, &rxReady));

    if ( txReady == MCESD_FALSE )
    {
        *txEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(sDev, serdesInfo->internalLane, &txPAM2En, &data));
        *txEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    if ( rxReady == MCESD_FALSE )
    {
        *rxEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(sDev, serdesInfo->internalLane, &rxPAM2En, &data));
        *rxEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesDumpInfo function
* @endinternal
*
* @brief   Dump SerDes info
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - SerDes number to power up/down
* @param[in]  dumpType  - dump type
* @param[in]  printBool - print to terminal
* @param[out] outputPtr - generic pointer to output data
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesDumpInfo
(
    IN  GT_U8                           devNum,
    IN  GT_UOPT                         portGroup,
    IN  GT_UOPT                         serdesNum,
    IN  MV_HWS_SERDES_DUMP_TYPE_ENT     dumpType,
    IN  GT_BOOL                         printBool,
    OUT GT_U32                          *outputPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      i;
    GT_U16                      pinVal;
    GT_U32                      regVal;
    static GT_CHAR              *pinsList[] =
    {
        "C112GX4_PIN_RESET",
        "C112GX4_PIN_ISOLATION_ENB",
        "C112GX4_PIN_BG_RDY",
        "C112GX4_PIN_SIF_SEL",
        "C112GX4_PIN_MCU_CLK",
        "C112GX4_PIN_DIRECT_ACCESS_EN",
        "C112GX4_PIN_PRAM_FORCE_RESET",
        "C112GX4_PIN_PRAM_RESET",
        "C112GX4_PIN_PRAM_SOC_EN",
        "C112GX4_PIN_PRAM_SIF_SEL",
        "C112GX4_PIN_PHY_MODE",
        "C112GX4_PIN_REFCLK_SEL",
        "C112GX4_PIN_REF_FREF_SEL",
        "C112GX4_PIN_PHY_GEN_TX0",
        "C112GX4_PIN_PHY_GEN_TX1",
        "C112GX4_PIN_PHY_GEN_TX2",
        "C112GX4_PIN_PHY_GEN_TX3",
        "C112GX4_PIN_PHY_GEN_RX0",
        "C112GX4_PIN_PHY_GEN_RX1",
        "C112GX4_PIN_PHY_GEN_RX2",
        "C112GX4_PIN_PHY_GEN_RX3",
        "C112GX4_PIN_DFE_EN0",
        "C112GX4_PIN_DFE_EN1",
        "C112GX4_PIN_DFE_EN2",
        "C112GX4_PIN_DFE_EN3",
        "C112GX4_PIN_DFE_UPDATE_DIS0",
        "C112GX4_PIN_DFE_UPDATE_DIS1",
        "C112GX4_PIN_DFE_UPDATE_DIS2",
        "C112GX4_PIN_DFE_UPDATE_DIS3",
        "C112GX4_PIN_PU_PLL0",
        "C112GX4_PIN_PU_PLL1",
        "C112GX4_PIN_PU_PLL2",
        "C112GX4_PIN_PU_PLL3",
        "C112GX4_PIN_PU_RX0",
        "C112GX4_PIN_PU_RX1",
        "C112GX4_PIN_PU_RX2",
        "C112GX4_PIN_PU_RX3",
        "C112GX4_PIN_PU_TX0",
        "C112GX4_PIN_PU_TX1",
        "C112GX4_PIN_PU_TX2",
        "C112GX4_PIN_PU_TX3",
        "C112GX4_PIN_TX_IDLE0",
        "C112GX4_PIN_TX_IDLE1",
        "C112GX4_PIN_TX_IDLE2",
        "C112GX4_PIN_TX_IDLE3",
        "C112GX4_PIN_PU_IVREF",
        "C112GX4_PIN_RX_TRAIN_ENABLE0",
        "C112GX4_PIN_RX_TRAIN_ENABLE1",
        "C112GX4_PIN_RX_TRAIN_ENABLE2",
        "C112GX4_PIN_RX_TRAIN_ENABLE3",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_FAILED0",
        "C112GX4_PIN_RX_TRAIN_FAILED1",
        "C112GX4_PIN_RX_TRAIN_FAILED2",
        "C112GX4_PIN_RX_TRAIN_FAILED3",
        "C112GX4_PIN_TX_TRAIN_ENABLE0",
        "C112GX4_PIN_TX_TRAIN_ENABLE1",
        "C112GX4_PIN_TX_TRAIN_ENABLE2",
        "C112GX4_PIN_TX_TRAIN_ENABLE3",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_FAILED0",
        "C112GX4_PIN_TX_TRAIN_FAILED1",
        "C112GX4_PIN_TX_TRAIN_FAILED2",
        "C112GX4_PIN_TX_TRAIN_FAILED3",
        "C112GX4_PIN_SQ_DETECTED_LPF0",
        "C112GX4_PIN_SQ_DETECTED_LPF1",
        "C112GX4_PIN_SQ_DETECTED_LPF2",
        "C112GX4_PIN_SQ_DETECTED_LPF3",
        "C112GX4_PIN_RX_INIT0",
        "C112GX4_PIN_RX_INIT1",
        "C112GX4_PIN_RX_INIT2",
        "C112GX4_PIN_RX_INIT3",
        "C112GX4_PIN_RX_INIT_DONE0",
        "C112GX4_PIN_RX_INIT_DONE1",
        "C112GX4_PIN_RX_INIT_DONE2",
        "C112GX4_PIN_RX_INIT_DONE3",
        "C112GX4_PIN_DFE_PAT_DIS0",
        "C112GX4_PIN_DFE_PAT_DIS1",
        "C112GX4_PIN_DFE_PAT_DIS2",
        "C112GX4_PIN_DFE_PAT_DIS3"
    };

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(outputPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    osPrintfIf(printBool,"Dump SerDes Number[%d] information\n");
    osPrintfIf(printBool,"SerDes Type: COM_PHY_C112GX4\n");

    switch(dumpType)
    {
        case SERDES_DUMP_INTERNAL_REGISTERS:
            osPrintfIf(printBool,"Dump type - Internal SerDes registers:\n");
            for(i = 0 ; i <= 0xE68C ; i = i+0x4)
            {
                CHECK_STATUS(mvHwsComphySerdesRegisterRead(devNum, portGroup,serdesNum,i,0xffffffff,&regVal));
                osPrintfIf(printBool,"Address: 0x%04x, Data: 0x%08x\n", i, regVal);
                if(NULL != outputPtr) outputPtr[i] = regVal;
            }
            break;
        case SERDES_DUMP_PINS:
            osPrintfIf(printBool,"Dump type - SerDes external pins:\n");
            for(i = 0 ; i <= (GT_U32)C112GX4_PIN_DFE_PAT_DIS3 ; i++)
            {
                mvHwsComphySerdesPinGet(devNum, portGroup, serdesNum, (E_C112GX4_PIN)i, &pinVal);
                osPrintfIf(printBool,"(%3d) %#-40s %3d\n", i, pinsList[i],pinVal);
                if(NULL != outputPtr) outputPtr[i] = pinVal;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesRxPllLockGet function
* @endinternal
*
* @brief   Returns whether or not the RX PLL is frequency locked
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] lockPtr                 - (pointer to) lock state
*                                        GT_TRUE =  locked
*                                        GT_FALSE = not locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesRxPllLockGet
(
    GT_U8                 devNum,
    GT_U32                serdesNum,
    GT_BOOL               *lockPtr
)
{
    GT_U32 data;

    if (NULL == lockPtr)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, 0, EXTERNAL_REG, serdesNum, SDW_LANE_STATUS_0, &data, PLL_INIT_POLLING_DONE));

    if (data == PLL_INIT_POLLING_DONE)
    {
        *lockPtr = GT_TRUE;
    }
    else
    {
         *lockPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesDroGet function
* @endinternal
*
* @brief   Get DRO (Device Ring Oscillator).
*           Indicates performance of device
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - SerDes number
*
* @param[out] dro                     - (pointer to)dro
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesDroGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          serdesNum,
    OUT GT_U16                          *dro
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    S_C112GX4_DRO_DATA freq;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetDRO(sDev,&freq));
    hwsOsMemCopyFuncPtr(dro, freq.dro, sizeof(S_C112GX4_DRO_DATA));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4IfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4IfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_C112GX4])
    {
        funcPtrArray[COM_PHY_C112GX4] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_C112GX4])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_C112GX4], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }

    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc     ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc          ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc     ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc     ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc       ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc      ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc      ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc               ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc             ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc          ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc       ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc      ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc        ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc     ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesFirmwareDownloadFunc    ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesFirmwareDownload);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTemperatureFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTemperatureGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc          ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc          ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDbRxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDumpInfoFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDumpInfo);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc        ,COM_PHY_C112GX4, mvHwsComphyC112GX4AnpPowerUp);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPMDLogGetFunc           ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPMDLogGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEncodingTypeGetFunc     ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesEncodingTypeGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxPllLockGetFunc        ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesRxPllLockGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDroGetFunc              ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDroGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesOperation);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc    ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesCdrLockStatusGet);

    return GT_OK;
}
#endif /* C112GX4 */

