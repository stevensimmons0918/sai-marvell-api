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
* @file mvComphyN5XC56GP5X4SerdesIf.c \
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

#ifdef N5XC56GP5X4
#include "mcesdN5XC56GP5X4_Defs.h"
#include "mcesdN5XC56GP5X4_API.h"
#include "mcesdN5XC56GP5X4_DeviceInit.h"
#include "mcesdN5XC56GP5X4_HwCntl.h"
#include "mcesdN5XC56GP5X4_RegRW.h"
#include "mcesdN5XC56GP5X4_FwDownload.h"
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/N5XC56GP5X4_main.h>

GT_STATUS mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           serdesNum,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
);

static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesOperation
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

        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x401c,(0x1<<2),(0x1<<2)));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, PLL_INIT_POLLING_DELAY));
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x401c,(0x1<<2),0));

        break;

    default:
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4GetDefaultParameters function
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
GT_STATUS mvHwsComphyN5XC56GP5X4GetDefaultParameters
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

    if((txDefaultParameters == NULL) && (rxDefaultParameters == NULL))
    {
        return GT_BAD_PARAM;
    }
#ifndef WIN32
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);

    for(baudRateIterator = (MV_HWS_SERDES_SPEED)0 ; serdesData->tuneParams[baudRateIterator].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator++)
    {
        if(serdesData->tuneParams[baudRateIterator].serdesSpeed == baudRate) break;
    }
    if(serdesData->tuneParams[baudRateIterator].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    if(txDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&txDefaultParameters->txComphyC56G,
                            &serdesData->tuneParams[baudRateIterator].txParams.txComphyC56G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&rxDefaultParameters->rxComphyC56G,
                            &serdesData->tuneParams[baudRateIterator].rxParams.rxComphyC56G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
    }
#else
    GT_UNUSED_PARAM(serdesInfo);
    GT_UNUSED_PARAM(serdesData);
    GT_UNUSED_PARAM(baudRateIterator);

    if(txDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&txDefaultParameters->txComphyC56G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&rxDefaultParameters->rxComphyC56G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
    }

#endif
    return GT_OK;
}

MCESD_STATUS API_N5XC56GP5X4_SetSignedTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN MCESD_32 paramValue
)
{
    E_N5XC56GP5X4_POLARITY polarity = N5XC56GP5X4_POLARITY_INV;

    if ( param != N5XC56GP5X4_TXEQ_EM_MAIN )
    {
        polarity =  (paramValue < 0) ? N5XC56GP5X4_POLARITY_INV: N5XC56GP5X4_POLARITY_NORMAL;
    }
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxEqPolarity(devPtr, lane, param, polarity));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxEqParam(devPtr, lane, param, (MCESD_U32)ABS(paramValue)));
    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_GetSignedTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN MCESD_32 *paramValue
)
{
    E_N5XC56GP5X4_POLARITY  inverted;
    MCESD_U32               unsignedParamValue;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxEqParam(devPtr, lane, param, &unsignedParamValue));
    if ( param == N5XC56GP5X4_TXEQ_EM_MAIN )
    {
         *paramValue = (MCESD_32)unsignedParamValue;
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxEqPolarity(devPtr, lane, param, &inverted));
        *paramValue = inverted ? (((MCESD_32)unsignedParamValue)*(-1)): ((MCESD_32)unsignedParamValue);
    }
    return MCESD_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesManualTxConfig function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesManualTxConfig
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

    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.pre2)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE2, (MCESD_32)txConfigPtr->txComphyC56G.pre2));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.pre)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE,  (MCESD_32)txConfigPtr->txComphyC56G.pre));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.post)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_POST, (MCESD_32)txConfigPtr->txComphyC56G.post));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.main)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_MAIN, (MCESD_32)txConfigPtr->txComphyC56G.main));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.usr)
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_5, (txConfigPtr->txComphyC56G.usr << 16), (1 << 16)));

    /** Static configurations */

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesManualRxConfig function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
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

    /* Basic (CTLE) */
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cur1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR1_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cur1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC56G.rl1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl1Extra)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_EXTRA,   (MCESD_U32)rxConfigPtr->rxComphyC56G.rl1Extra));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.res1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES1_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.res1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cap1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cap1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.enMidfreq)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_EN_MIDFREQ,  (MCESD_U32)rxConfigPtr->rxComphyC56G.enMidfreq));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cs1Mid)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CS1_MID,     (MCESD_U32)rxConfigPtr->rxComphyC56G.cs1Mid));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rs1Mid)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RS1_MID,     (MCESD_U32)rxConfigPtr->rxComphyC56G.rs1Mid));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cur2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR2_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cur2Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC56G.rl2Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl2TuneG)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_TUNE_G,  (MCESD_U32)rxConfigPtr->rxComphyC56G.rl2TuneG));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.res2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES2_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.res2Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cap2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cap2Sel));

    /* Advanced (CDR) */
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmufi)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmufi));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmuff)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmuff));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmupi)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmupi));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmupf)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmupf));

    if(NA_16BIT_SIGNED != rxConfigPtr->rxComphyC56G.squelch)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetSquelchThreshold(sDev, serdesInfo->internalLane, (MCESD_16)rxConfigPtr->rxComphyC56G.squelch));

    /** Static configurations */

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesReset function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesReset
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
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_1, (data << 4), (1 << 4)));

    /* SERDES SYNCE RESET init */
    data = (syncEReset == GT_TRUE) ? 0 : 1;
    /*CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18));*/

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesCoreReset function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_UREG_DATA data;

    data = (coreReset == UNRESET) ? 0 : 3;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_1, (data << 2), (3 << 2)));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxEnbale function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxEnbale
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

    /* need to uncomment and remove the next writing after CE updates MCESD */
    /*MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)txEnable));*/
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x3004, 31, 1, (txEnable) ? 0x0 : 0x1));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet function
* @endinternal
*
* @brief   Get Tx signal enbale
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  txEnable  - if true - enable Tx
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *txEnable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32 data;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* need to uncomment and remove the next writing after CE updates MCESD */
    /*MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL*)txEnable));*/
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x3004,31,1, &data));

    *txEnable = (0 == data) ? GT_TRUE : GT_FALSE;    /* inverted */

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl
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
    E_N5XC56GP5X4_SERDES_SPEED   speed = 0;
    E_N5XC56GP5X4_REFFREQ        refFreq = 0;
    E_N5XC56GP5X4_REFCLK_SEL     refClkSel = 0;
    E_N5XC56GP5X4_DATABUS_WIDTH  dataBusWidth = 0;
    S_N5XC56GP5X4_PowerOn        powerOnConfig;
    MV_HWS_SERDES_SPEED          baudRate = serdesConfigPtr->baudRate;
#if (!defined FW_253_6)
    GT_U32                       txRegVal, rxRegVal;
#endif
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);
    if(powerUp)
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Power-on lane */
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_N5XC56GP5X4_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_N5XC56GP5X4_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_N5XC56GP5X4_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_N5XC56GP5X4_DATABUS_WIDTH)mcesdEnum;
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));

            powerOnConfig.u.powerLaneMask = 1 << serdesInfo->internalLane;
            powerOnConfig.initTx          = GT_TRUE;
            powerOnConfig.initRx          = GT_TRUE;
            powerOnConfig.txOutputEn      = GT_FALSE;
            powerOnConfig.downloadFw      = GT_FALSE;
            powerOnConfig.dataPath        = N5XC56GP5X4_PATH_EXTERNAL;
            powerOnConfig.txRefClkSel     = refClkSel;
            powerOnConfig.rxRefClkSel     = refClkSel;
            powerOnConfig.txRefFreq       = refFreq;
            powerOnConfig.rxRefFreq       = refFreq;
            powerOnConfig.dataBusWidth    = dataBusWidth;
            powerOnConfig.txSpeed         = speed;
            powerOnConfig.rxSpeed         = speed;
            powerOnConfig.avdd            = N5XC56GP5X4_AVDD_1P2V;
            powerOnConfig.spdCfg          = N5XC56GP5X4_SPD_CFG_4_PLL;
            powerOnConfig.fwDownload      = NULL;

            MCESD_CHECK_STATUS(API_N5XC56GP5X4_PowerOnSeq(sDev, powerOnConfig));
        }

        /* Wait for stable PLLs */
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[0],NULL,NULL,&sDev));
        do
        {
            allStable = GT_TRUE;
            for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
            {
#if (!defined FW_253_6)
                CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesArr[serdesIterator], 0x3000,(18),1, &txRegVal));
                CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesArr[serdesIterator], 0x3200,(22),1, &rxRegVal));
                data = (txRegVal <<3) | (rxRegVal << 2);
#else
                CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SDW_LANE_STATUS_0, &data, PLL_INIT_POLLING_DONE));
#endif
                if (data != PLL_INIT_POLLING_DONE)
                {
                    allStable = GT_FALSE;
                    break;
                }
            }
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, PLL_INIT_POLLING_DELAY));
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

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4GetDefaultParameters(devNum, serdesArr[serdesIterator], baudRate, &txDefaultParameters, &rxDefaultParameters));

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txDefaultParameters));

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxDefaultParameters));

            /* SQ_LPF_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesArr[serdesIterator], 0x3270, (0xFFFF << 16), 0x3FF << 16));

            /* SQ_LPF_EN_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesArr[serdesIterator], 0x3270, (0x1 << 12), 0x1   << 12));

            if(serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
            {
#if (!defined FW_253_6)
                /*  N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_PHASE_F0B_HYST_HIGH_LANE, serdesArr[serdesIterator], 0x1e);*/
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesArr[serdesIterator], 0x6200, 0x1e, 0x1e));
#endif
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane, N5XC56GP5X4_GRAY_ENABLE, N5XC56GP5X4_GRAY_ENABLE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPreCode(sDev, serdesInfo->internalLane, MCESD_FALSE, MCESD_FALSE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMSBLSBSwap(sDev, serdesInfo->internalLane, N5XC56GP5X4_SWAP_PRECODER, N5XC56GP5X4_SWAP_PRECODER));
            }
            else
            {
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane, N5XC56GP5X4_GRAY_DISABLE, N5XC56GP5X4_GRAY_DISABLE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPreCode(sDev, serdesInfo->internalLane, MCESD_FALSE, MCESD_FALSE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMSBLSBSwap(sDev, serdesInfo->internalLane, N5XC56GP5X4_SWAP_DISABLE, N5XC56GP5X4_SWAP_DISABLE));
            }

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxEnbale(devNum, portGroup, serdesArr[serdesIterator], GT_TRUE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));

        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_PowerOffLane(sDev, serdesInfo->internalLane));

            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_FIR_TAP_POL_F, serdesInfo->internalLane, 0);
            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_C0_FORCE, serdesInfo->internalLane, 0);
            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_C1_FORCE, serdesInfo->internalLane, 0);
            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_C2_FORCE, serdesInfo->internalLane, 0);
            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_C3_FORCE, serdesInfo->internalLane, 0);
            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_C4_FORCE, serdesInfo->internalLane, 0);
            N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_TX_C5_FORCE, serdesInfo->internalLane, 0);
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane, N5XC56GP5X4_GRAY_DISABLE, N5XC56GP5X4_GRAY_DISABLE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMSBLSBSwap(sDev, serdesInfo->internalLane, N5XC56GP5X4_SWAP_DISABLE, N5XC56GP5X4_SWAP_DISABLE));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxEnbale(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_FALSE));

        }
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPowerCtrl function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStop function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneStart function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneStart
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
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_TRUE;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatus function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatus
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_CheckTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_CheckTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_RX_TRAINCO0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_RX_TRAINFA0 + serdesInfo->internalLane, &failedData));
        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_TX_TRAINCO0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_TX_TRAINFA0 + serdesInfo->internalLane, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneResult function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MV_HWS_PER_SERDES_INFO_PTR              serdesInfo;
    MCESD_DEV_PTR                           sDev;
    MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS    *results;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U16 val16;
    MCESD_16  vals16;
    MCESD_U32 tapsI;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC56GResults;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE2, &vals));
    results->txComphyC56G.pre2 = (GT_8)vals;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE,  &vals));
    results->txComphyC56G.pre  = (GT_8)vals;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_MAIN, &vals));
    results->txComphyC56G.main = (GT_8)vals;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_POST, &vals));
    results->txComphyC56G.post = (GT_8)vals;
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_5, &val, (1 << 16)));
    results->txComphyC56G.usr = (GT_8)(val >> 16);

    /* Basic (CTLE) */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR1_SEL,   &val));
    results->rxComphyC56G.cur1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_SEL,    &val));
    results->rxComphyC56G.rl1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_EXTRA,  &val));
    results->rxComphyC56G.rl1Extra= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES1_SEL,   &val));
    results->rxComphyC56G.res1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL,   &val));
    results->rxComphyC56G.cap1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_EN_MIDFREQ, &val));
    results->rxComphyC56G.enMidfreq= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CS1_MID,    &val));
    results->rxComphyC56G.cs1Mid= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RS1_MID,    &val));
    results->rxComphyC56G.rs1Mid= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR2_SEL,   &val));
    results->rxComphyC56G.cur2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_SEL,    &val));
    results->rxComphyC56G.rl2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_TUNE_G, &val));
    results->rxComphyC56G.rl2TuneG= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES2_SEL,   &val));
    results->rxComphyC56G.res2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL,   &val));
    results->rxComphyC56G.cap2Sel= (GT_U32)val;

    /* Advanced (CDR) */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFI, &val));
    results->rxComphyC56G.selmufi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFF, &val));
    results->rxComphyC56G.selmuff= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPI, &val));
    results->rxComphyC56G.selmupi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPF, &val));
    results->rxComphyC56G.selmupf= (GT_U32)val;

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSquelchThreshold(sDev, serdesInfo->internalLane, &vals16));
    results->rxComphyC56G.squelch= (GT_16)vals16;

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetAlign90(sDev, serdesInfo->internalLane, &val16));
    results->align90 = (GT_16)val16;

    results->eo = 0;
    results->sampler = 0;
    results->slewRateCtrl0 = 0;
    results->slewRateCtrl1 = 0;

    for(tapsI = 0 ; tapsI < N5XC56GP5X4_DFE_FF5 ; tapsI++)
    {
        if (MCESD_OK == API_N5XC56GP5X4_GetDfeTap(sDev, serdesInfo->internalLane, N5XC56GP5X4_EYE_MID, (E_N5XC56GP5X4_DFE_TAP)tapsI, &vals))
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
* @internal mvHwsComphyN5XC56GP5X4SerdesDigitalReset function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesDigitalReset
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
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_1, (data << 4), (1 << 4)));

    return GT_OK;
}
/**
* @internal mvHwsComphyN5XC56GP5X4SerdesLoopback function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_N5XC56GP5X4_DATAPATH      path;
    GT_U32                      mcesdEnum;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_N5XC56GP5X4_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetDataPath(sDev, serdesInfo->internalLane, path));

    switch ( lbMode )
    {
    case SERDES_LP_AN_TX_RX:
        /* Masks sq_detected indication. If set, SQ detect is always zero (signal detected). Masks sq_detected indication */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (1 << 27), (1 << 27)));
        break;
    default:
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (0 << 27), (1 << 27)));
        break;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesLoopbackGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_N5XC56GP5X4_DATAPATH          path;
    GT_U32                      tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetDataPath(sDev, serdesInfo->internalLane, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTestGen function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTestGen
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
    E_N5XC56GP5X4_PATTERN             pattern;
    E_N5XC56GP5X4_SERDES_SPEED        speed;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    MV_HWS_SERDES_ENCODING_TYPE txEncoding, rxEncoding;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U8)txPattern, &mcesdPattern));
    pattern = (E_N5XC56GP5X4_PATTERN)mcesdPattern;

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
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxRxPattern(sDev, serdesInfo->internalLane, pattern, pattern, userPattern, userPattern));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxBitRate(sDev, serdesInfo->internalLane,&speed,&speed));

    CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet(devNum,serdesNum,&txEncoding, &rxEncoding));
    if ( txEncoding ==  SERDES_ENCODING_PAM4 || rxEncoding ==  SERDES_ENCODING_PAM4 )
    {
        /* PT_TX_PRBS_INV_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3098, 7,  1, (SERDES_TEST == mode) ? 0x1 : 0x0));

        /* PT_RX_PRBS_INV_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3280, 14, 1, (SERDES_TEST == mode) ? 0x1 : 0x0));

        /* TXDATA_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3024, 5,  1, (SERDES_TEST == mode) ? 0x0 : 0x1));

        /* RXDATA_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3248, 24, 1, (SERDES_TEST == mode) ? 0x0 : 0x1));

        /* TXD_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3024, 18, 1, 0x0));

        /* RXD_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3248, 27, 1, 0x0));
    }

    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_ResetComparatorStats(sDev, serdesInfo->internalLane));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartPhyTest(sDev, serdesInfo->internalLane));
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopPhyTest(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTestGenGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTestGenGet
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
    E_N5XC56GP5X4_PATTERN             txPattern;
    E_N5XC56GP5X4_PATTERN             rxPattern;
    char                              userPattern[21];
    MCESD_FIELD                       ptEn = F_N5XC56GP5X4_TX_EN;
    MCESD_U32                         enable;
    GT_U32                            tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxPattern(sDev, serdesInfo->internalLane, &txPattern, &rxPattern, userPattern, userPattern));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_ReadField(sDev, serdesInfo->internalLane, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTestGenStatus
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTestGenStatus
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
    S_N5XC56GP5X4_PATTERN_STATS       statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetComparatorStats(sDev, serdesInfo->internalLane, &statistics));

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr   = (GT_U32)statistics.totalErrorBits;
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_ResetComparatorStats(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject (max=8)
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*
*/
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesErrorInject
(
    IN GT_U8                        devNum,
    IN GT_UOPT                      portGroup,
    IN GT_UOPT                      serdesNum,
    IN GT_UOPT                      numOfBits,
    IN MV_HWS_SERDES_DIRECTION      serdesDirection
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

    if(serdesDirection != TX_DIRECTION)
    {
        return GT_NOT_SUPPORTED;
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_TxInjectError(sDev, serdesInfo->internalLane, (MCESD_U8)numOfBits));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPolarityConfig function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPolarityConfig
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

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxRxPolarity(sDev, serdesInfo->internalLane, invertTx ? N5XC56GP5X4_POLARITY_INV : N5XC56GP5X4_POLARITY_NORMAL,
                                                        invertRx ? N5XC56GP5X4_POLARITY_INV : N5XC56GP5X4_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPolarityConfigGet
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPolarityConfigGet
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
    E_N5XC56GP5X4_POLARITY      txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxPolarity(sDev, serdesInfo->internalLane, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == N5XC56GP5X4_POLARITY_INV);
    *invertRx = (rxPolarity == N5XC56GP5X4_POLARITY_INV);

    return GT_OK;
}


/** 
* @internal mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet
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

    /* in serdes loopback analog tx2rx, we forced signal detect in mvHwsComphyN5XC56GP5X4SerdesLoopback */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, &data, (1 << 27)));
    if ( data != 0 )
    {
        *enable = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSquelchDetect(sDev, serdesInfo->internalLane, &squelched));
    *enable = squelched ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/** 
* @internal mvHwsComphyN5XC56GP5X4SerdesCdrLockStatusGet
*           function
* @endinternal
*
* @brief   Check if cdr is lock
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number to get its signal state
* @param[out] enable    - return cdr lock value
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *lock
)
{
/*    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      data = 0, isPam2, clampingTrigger=0,freqReady=0, rxTrain;
    GT_8                        dataSigned = 0,dataSigned2;*/

    /* check signal */
    MCESD_CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    /*TODO add CDR SUPPORT*/
    return GT_OK;
}
/**
* @internal mvHwsComphyN5XC56GP5X4SerdesFirmwareDownloadIter
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesFirmwareDownloadIter
(
    GT_U8                     devNum,
    GT_UOPT                   *serdesArr,
    GT_U8                     numOfSerdeses,
    MCESD_U32                 *fwCodePtr,
    MCESD_U32                 fwCodeSize,
    E_N5XC56GP5X4_REFCLK_SEL  refClkSel,
    E_N5XC56GP5X4_REFFREQ     refFreq
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U8 serdesIterator;
    MCESD_U16 errCode;

    for(serdesIterator = 0; serdesIterator < numOfSerdeses; serdesIterator++)
    {
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesArr[serdesIterator], &serdesInfo, NULL, &sDev));
        if(serdesInfo->internalLane != 0)
        {
            return GT_BAD_PARAM;
        }
        /* Set SERDES Mode */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPhyMode(sDev, N5XC56GP5X4_PHYMODE_SERDES));
        /* Power up current and voltage reference */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPowerIvRef(sDev, MCESD_TRUE));

        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMcuBroadcast(sDev, MCESD_TRUE));
        /* Configure Reference Frequency and reference clock source group */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetRefFreq(sDev, serdesInfo->internalLane, refFreq, refFreq, refClkSel, refClkSel));

        /* Clear N5XC56GP5X4_PIN_FW_READY */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_0, (0 << 3), (1 << 3)));
        /* Set N5XC56GP5X4_PIN_DIRECTACCES */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwSetPinCfg(sDev, N5XC56GP5X4_PIN_DIRECTACCES, 1));
        /* Set N5XC56GP5X4_PIN_PRAM_SOC_EN */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_1, (1 << 14), (1 << 14)));

        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 10));
        /* Download MCU Firmware */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_DownloadFirmware(sDev, fwCodePtr, fwCodeSize, N5XC56GP5X4_FW_BASE_ADDR, &errCode));

        /* Clear N5XC56GP5X4_PIN_PRAM_SOC_EN */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_1, (0 << 14), (1 << 14)));
        /* Clear N5XC56GP5X4_PIN_DIRECTACCES */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwSetPinCfg(sDev, N5XC56GP5X4_PIN_DIRECTACCES, 0));
        /* Set N5XC56GP5X4_PIN_FW_READY */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_0, (1 << 3), (1 << 3)));

        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 10));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMcuBroadcast(sDev, MCESD_FALSE));
#if 0
        /* Enable MCU */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetLaneEnable(sDev, 255, MCESD_TRUE));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMcuEnable(sDev, 255, MCESD_TRUE));
#endif
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesFirmwareDownload function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesFirmwareDownload
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

    E_N5XC56GP5X4_REFCLK_SEL   refClkSel = N5XC56GP5X4_REFCLK_SEL_G1;
    E_N5XC56GP5X4_REFFREQ      refFreq;
    GT_BOOL                    freeNeeded = GT_FALSE;

    MCESD_U32 *fwCodePtr;
    MCESD_U32 fwCodeSize;
    GT_UNUSED_PARAM(portGroup);
    switch(firmwareSelector)
    {
        case MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE:
        case MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE:
            fwCodePtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*N5XC56GP5X4_FW_MAX_SIZE);
            if ( fwCodePtr == NULL )
            {
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//N5XC56GP5X4//main.dat",
                                        fwCodePtr,       N5XC56GP5X4_FW_MAX_SIZE,         &fwCodeSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }
            freeNeeded = GT_TRUE;
            break;
            case MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE:
            case MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE:
            fwCodePtr     = (MCESD_U32*)N5XC56GP5X4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_N5XC56GP5X4_MAIN_DATA_IMAGE_SIZE;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    if((MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE == firmwareSelector) || (MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE == firmwareSelector))
    {
        refFreq = N5XC56GP5X4_REFFREQ_156MHZ;
    }
    else if((MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE == firmwareSelector) || (MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE == firmwareSelector))
    {
        refFreq = N5XC56GP5X4_REFFREQ_25MHZ;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    comphyPrintf("--fwCodeSize[%d]--\n",fwCodeSize);
    if ( freeNeeded )
    {
        hwsOsPrintf("Using SerDes FW from file: version=[%u.%u.%u.%u], fwCodeSize=[%d]\n",
                    (fwCodePtr[128] & 0xFF000000)>>24,
                    (fwCodePtr[128] & 0x00FF0000)>>16,
                    (fwCodePtr[128] & 0x0000FF00)>>8,
                    (fwCodePtr[128] & 0x000000FF)>>0,
                    fwCodeSize);
    }
    rc = mvHwsComphyN5XC56GP5X4SerdesFirmwareDownloadIter(devNum, serdesArr, numOfSerdeses,
                                                       fwCodePtr, fwCodeSize,
                                                       refClkSel, refFreq);

    if ( freeNeeded )
    {
        hwsOsFreeFuncPtr(fwCodePtr);
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
GT_STATUS mvHwsComphyN5XC56GP5X4AnpPowerUp
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
    GT_U32                       mcesdEnum;
    E_N5XC56GP5X4_REFFREQ        refFreq = 0;
    E_N5XC56GP5X4_REFCLK_SEL     refClkSel = 0;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetDataBusWidth(sDev, serdesInfo->internalLane, N5XC56GP5X4_DATABUS_40BIT, N5XC56GP5X4_DATABUS_40BIT));
    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x3270, (0xFFFF << 16), 0x3FF << 16));
    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x3270, (1<<12), (regData<<12)));

    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
    refFreq = (E_N5XC56GP5X4_REFFREQ)mcesdEnum;

    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
    refClkSel = (E_N5XC56GP5X4_REFCLK_SEL)mcesdEnum;

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_N5XC56GP5X4_SetRefFreq(sDev, serdesInfo->internalLane, refFreq, refFreq, refClkSel, refClkSel ));

    CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesNum, GT_FALSE, GT_FALSE, GT_FALSE));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTemperatureGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTemperatureGet
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

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTemperature(sDev, &temp));
    *serdesTemperature = (GT_32)temp/10000;

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet
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
    CHECK_STATUS(mvHwsComphyN5XC56GP5X4GetDefaultParameters(devNum, serdesNum, baudRate, txConfigPtr, NULL));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet
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
    CHECK_STATUS(mvHwsComphyN5XC56GP5X4GetDefaultParameters(devNum, serdesNum, baudRate, NULL, rxConfigPtr));

    /*TODO - get override params*/
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           serdesNum,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_FIELD txPAM2En = F_N5XC56GP5X4_TX_PAM2_EN;
    MCESD_FIELD rxPAM2En = F_N5XC56GP5X4_RX_PAM2_EN;
    MCESD_U32 data;
    MCESD_BOOL txReady, rxReady;

    if( NULL == txEncodingPtr || NULL == rxEncodingPtr )
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* check if serdes powered-up */
    MCESD_ATTEMPT(API_N5XC56GP5X4_GetTxRxReady(sDev, serdesInfo->internalLane, &txReady, &rxReady));

    if ( txReady == MCESD_FALSE )
    {
        *txEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_ATTEMPT(API_N5XC56GP5X4_ReadField(sDev, serdesInfo->internalLane, &txPAM2En, &data));
        *txEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    if ( rxReady == MCESD_FALSE )
    {
        *rxEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_ATTEMPT(API_N5XC56GP5X4_ReadField(sDev, serdesInfo->internalLane, &rxPAM2En, &data));
        *rxEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4IfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyN5XC56GP5X4IfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_N5XC56GP5X4])
    {
        funcPtrArray[COM_PHY_N5XC56GP5X4] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_N5XC56GP5X4])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_N5XC56GP5X4], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }

    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc          ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc       ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc      ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc      ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc               ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset            ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc             ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc          ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc       ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc      ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc            ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc            ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc        ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesFirmwareDownloadFunc    ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesFirmwareDownload);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTemperatureFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTemperatureGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc          ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc          ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc        ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4AnpPowerUp);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesOperation);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEncodingTypeGetFunc     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesErrorInjectFunc         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesErrorInject);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc    ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesCdrLockStatusGet);

    return GT_OK;
}
#endif /* N5XC56GP5X4 */

