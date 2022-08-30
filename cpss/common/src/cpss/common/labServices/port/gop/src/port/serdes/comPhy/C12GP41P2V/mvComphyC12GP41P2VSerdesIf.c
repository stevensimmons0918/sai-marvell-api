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
* @file mvComphyC12GP41P2VSerdesIf.c \
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsDiagnostic.h>

#ifdef C12GP41P2V
#include "mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_API.h"
#include "mcesdC12GP41P2V_DeviceInit.h"
#include "mcesdC12GP41P2V_HwCntl.h"
#include "mcesdC12GP41P2V_RegRW.h"

#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MIN_TEMPERATURE 30
#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_TEMPERATURE 80
#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_RES_SEL 0x7
#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_CAP_SEL 0xF

/**
* @internal mvHwsComphyC12GP41P2VSerdesDbTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_TX_CONFIG_DATA - pointer to Tx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSerdesDbTxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParamsPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       baudRateIterator0 = 0/*default param iterartor*/, baudRateIterator1 = 0/*override param iterartor*/, overrideSize = MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE/*override param size*/;

    GT_UNUSED_PARAM(portGroup);

    if(configParamsPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* array like alleycat5TxRxTuneParams is not initialized for 'WIN32' */
        return GT_OK;
    }
#endif /**/
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);

    for(baudRateIterator0 = 0 ; serdesData->tuneParams[baudRateIterator0].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator0++)
    {
        if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == baudRate) break;
    }
    if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    if(serdesData->tuneOverrideParams != NULL)
    {
        for(baudRateIterator1 = 0 ; baudRateIterator1 < overrideSize; baudRateIterator1++)
        {
            if(serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed == baudRate) break;
        }
    }

    if ((baudRateIterator1 == overrideSize) || (serdesData->tuneOverrideParams == NULL))/*override param not found - set default param*/
    {
        hwsOsMemCopyFuncPtr(&(configParamsPtr->txComphyC12GP41P2V),
                            &(serdesData->tuneParams[baudRateIterator0].txParams.txComphyC12GP41P2V),
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA));
    }
    else/*override param exist - go over tx params and set override value if exist or default if not exist*/
    {
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->txComphyC12GP41P2V.pre,    serdesData->tuneParams[baudRateIterator0].txParams.txComphyC12GP41P2V.pre,  serdesData->tuneOverrideParams[baudRateIterator1].txParams.txComphyC12GP41P2V.pre);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->txComphyC12GP41P2V.peak,   serdesData->tuneParams[baudRateIterator0].txParams.txComphyC12GP41P2V.peak, serdesData->tuneOverrideParams[baudRateIterator1].txParams.txComphyC12GP41P2V.peak);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->txComphyC12GP41P2V.post,   serdesData->tuneParams[baudRateIterator0].txParams.txComphyC12GP41P2V.post, serdesData->tuneOverrideParams[baudRateIterator1].txParams.txComphyC12GP41P2V.post);
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesDbRxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure RX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_RX_CONFIG_DATA - pointer to Rx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSerdesDbRxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParamsPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       baudRateIterator0 = 0/*default param iterartor*/, baudRateIterator1 = 0/*override param iterartor*/, overrideSize = MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE/*override param size*/;


    GT_UNUSED_PARAM(portGroup);

    if(configParamsPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);
#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* array like alleycat5TxRxTuneParams is not initialized for 'WIN32' */
        return GT_OK;
    }
#endif /**/

    for(baudRateIterator0 = 0 ; serdesData->tuneParams[baudRateIterator0].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator0++)
    {
        if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == baudRate) break;
    }
    if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    if(serdesData->tuneOverrideParams != NULL)
    {
        for(baudRateIterator1 = 0 ; baudRateIterator1 < overrideSize; baudRateIterator1++)
        {
            if(serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed == baudRate) break;
        }
    }

    if ((baudRateIterator1 == overrideSize) || (serdesData->tuneOverrideParams == NULL))/*override param not found - set default param*/
    {
        hwsOsMemCopyFuncPtr(&(configParamsPtr->rxComphyC12GP41P2V),
                            &(serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V),
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA));
    }
    else/*override param exist - go over rx params and set override value if exist or default if not exist*/
    {
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.resSel,     serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.resSel,   serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.resSel);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.resShift,   serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.resShift, serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.resShift);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.capSel,     serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.capSel,   serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.capSel);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmufi,    serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.selmufi,  serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.selmufi);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmuff,    serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.selmuff,  serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.selmuff);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmupi,    serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.selmupi,  serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.selmupi);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmupf,    serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.selmupf,  serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.selmupf);
        HWS_COMPHY_CHECK_AND_SET_DB_16BIT_SIGNED_VAL(configParamsPtr->rxComphyC12GP41P2V.squelch,    serdesData->tuneParams[baudRateIterator0].rxParams.rxComphyC12GP41P2V.squelch,  serdesData->tuneOverrideParams[baudRateIterator1].rxParams.rxComphyC12GP41P2V.squelch);
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesManualTxConfig function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesManualTxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
)
{
    MCESD_DEV_PTR sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* we got dummy info from mvHwsComphyC12GP41P2VSerdesDbTxConfigGet */
        /* see comments in function mvHwsComphyC12GP41P2VSerdesDbTxConfigGet */
        return GT_OK;
    }
#endif /**/

    if(NA_8BIT != txConfigPtr->txComphyC12GP41P2V.pre)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PRE_CTRL , (MCESD_U32)txConfigPtr->txComphyC12GP41P2V.pre));
    if(NA_8BIT != txConfigPtr->txComphyC12GP41P2V.peak)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PEAK_CTRL, (MCESD_U32)txConfigPtr->txComphyC12GP41P2V.peak));
    if(NA_8BIT != txConfigPtr->txComphyC12GP41P2V.post)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_POST_CTRL, (MCESD_U32)txConfigPtr->txComphyC12GP41P2V.post));

    /** Static configurations */
    MCESD_CHECK_STATUS(API_C12GP41P2V_SetSlewRateEnable(sDev, C12GP41P2V_SR_DISABLE));
    MCESD_CHECK_STATUS(API_C12GP41P2V_SetSlewRateParam(sDev, C12GP41P2V_SR_CTRL0, 0));
    MCESD_CHECK_STATUS(API_C12GP41P2V_SetSlewRateParam(sDev, C12GP41P2V_SR_CTRL1, 0));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesManualRxConfig function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    MCESD_DEV_PTR sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* we got dummy info from mvHwsComphyC12GP41P2VSerdesDbTxConfigGet */
        /* see comments in function mvHwsComphyC12GP41P2VSerdesDbTxConfigGet */
        return GT_OK;
    }
#endif /**/

    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.resSel)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL,   (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.resSel  ));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.resShift)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SHIFT, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.resShift));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.capSel)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL,   (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.capSel  ));

    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmufi)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmufi));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmuff)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmuff));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmupi)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmupi));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmupf)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmupf));

    if(NA_16BIT_SIGNED != rxConfigPtr->rxComphyC12GP41P2V.squelch)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xC9, 8, 1 ,1));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetSquelchThreshold(sDev, (MCESD_16)rxConfigPtr->rxComphyC12GP41P2V.squelch));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0X280, 4, 1, 1));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0X280, 4, 1, 0));
/* if 1 = fixed. if 0  = calibrated */
#if 1
        /* in Cider: address = 0x1BC8. Because CE reg numbering, the address by their method is : (address - 0x1000)/4 */
        /* sq_cal_thresh_force_en_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0X2F2, 0x1 << 15, 0x1 << 15));

        /* sq_cal_thresh_ext_lane (squelch value from DB + polarity (0x20) */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0X2F2, 9, 6, (GT_U32)(rxConfigPtr->rxComphyC12GP41P2V.squelch + 0x20)));
#endif
    }

    /** Static configurations */
    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0xCA, 0xFFFF, 0x3FF));

    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0xC9, (0x1 << 12), (0x1 << 12)));

    /* rcal_vth_rx */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x309, 0x7, 0x2));

    /* G1_TX_VREF_TXDRV_SEL_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x48, 0x7, 0x7));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesReset function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesReset
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
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 3), (1 << 3)));

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == GT_TRUE) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 6), (1 << 6)));

    /* SERDES SYNCE RESET init */
    data = (syncEReset == GT_TRUE) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_2, (data << 3), (1 << 3)));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesCoreReset function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_UREG_DATA data;

    data = (coreReset == UNRESET) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 5), (1 << 5)));

    return GT_OK;
}

static GT_STATUS mvHwsComphyC12GP41P2VSerdesTxRxExtraConfiguraion
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_SERDES_SPEED     serdesSpeed
)
{
    if (_3_125G == serdesSpeed || _1_25G == serdesSpeed || _5_15625G == serdesSpeed)
    {
        /* G1_FFE_SETTING_FORCE_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x4e, 0x1000, 0x1000));
    }

    /** JIRA CPSS-12073 */
    if ( _5_15625G == serdesSpeed )
    {
        /* convert from Cider to CE address : address = (cider_add - 0x1000) / 4 */

        /* G1_RX_CTLE_DATA_RATE1_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x1168 - 0x1000) / 4, 0x7, 0x3));

        /* G1_RX_CTLE_DATA_RATE2_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x116C - 0x1000) / 4, 0x7, 0x3));

        /* G1_RX_CTLE_DATA_RATE3_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x1170 - 0x1000) / 4, 0x7, 0x3));

        /* DFE_RES_F0A_HIGH_THRES_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x15D4 - 0x1000) / 4, 0xF0, 6 << 4));

        /* G1_FFE_RES_SHIFT_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x117C - 0x1000) / 4, 0xF, 0x2));
    }

    if (_10_3125G == serdesSpeed || _10G == serdesSpeed || _11_5625G == serdesSpeed)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xc9  ,  8  ,  1  ,   0x1  )); /* SQ_DEGLITCH_EN_LANE          */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xCA  ,  0  ,  16 ,   0x3FF)); /* SQ_LPF_LANE                  */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xC9  ,  12 ,  1  ,   0x1  )); /* SQ_LPF_EN_LANE               */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x309 ,  0  ,  3  ,   0x2  )); /* rcal_vth_rx                  */

        /* TX_ADAPT_GN1_EN_LANE, TX_ADAPT_G0_EN_LANE, TX_ADAPT_G1_EN_LANE*/
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1a6  ,  0x7  ,   0x7  ));

        /* FFE_RES_SEL10_LANE, FFE_RES_SEL11_LANE, FFE_RES_SEL12_LANE, FFE_RES_SEL13_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1fc  ,  0xFFFF  ,   0xCA86  ));

        /* FFE_RES_SEL14_LANE, FFE_RES_SEL15_LANE, FFE_RES_SEL16_LANE, FFE_RES_SEL17_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1fd  ,  0xFFFF  ,   0x4200  ));

        /* ffe_res_sel20_lane, ffe_res_sel21_lane, ffe_res_sel22_lane, ffe_res_sel23_lane  */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1fe  ,  0xFFFF  ,   0x0  ));

        /* ffe_res_sel24_lane, ffe_res_sel25_lane, ffe_res_sel26_lane, ffe_res_sel27_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1ff  ,  0xFFFF  ,   0x0247  ));

        /* ffe_res_sel30_lane, ffe_res_sel31_lane, ffe_res_sel32_lane, ffe_res_sel33_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f8  ,  0xFFFF  ,   0x0  ));

        /* ffe_res_sel34_lane, ffe_res_sel35_lane, ffe_res_sel36_lane, ffe_res_sel37_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f9  ,  0xFFFF  ,   0x0  ));

        /* ffe_res_sel38_lane, ffe_res_sel39_lane, ffe_res_sel3a_lane, ffe_res_sel3b_lane*/
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1fa  ,  0xFFFF  ,   0x0  ));

        /* FFE_RES_SEL3C_LANE, FFE_RES_SEL3D_LANE, FFE_RES_SEL3E_LANE, FFE_RES_SEL3F_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1fb  ,  0xFFFF  ,   0x0  ));

        /* FFE_CAP_SEL10_LANE, FFE_CAP_SEL11_LANE, FFE_CAP_SEL12_LANE, FFE_CAP_SEL13_LANE*/
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f0  ,  0xFFFF  ,   0x0  ));

        /* FFE_CAP_SEL14_LANE, FFE_CAP_SEL15_LANE, FFE_CAP_SEL16_LANE, FFE_CAP_SEL17_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f1  ,  0xFFFF  ,   0x0123  ));

        /* FFE_CAP_SEL18_LANE, FFE_CAP_SEL19_LANE, FFE_CAP_SEL1A_LANE, FFE_CAP_SEL1B_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f2  ,  0xFFFF  ,   0x57AF  ));

        /* FFE_CAP_SEL1C_LANE, FFE_CAP_SEL1D_LANE, FFE_CAP_SEL1E_LANE, FFE_CAP_SEL1F_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f3  ,  0xFFFF  ,   0xFFFF  ));

        /* FFE_CAP_SEL20_LANE, FFE_CAP_SEL21_LANE, FFE_CAP_SEL22_LANE, ,FFE_CAP_SEL23_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f4  ,  0xFFFF  ,   0x0  ));

        /* FFE_CAP_SEL24_LANE, ffe_cap_sel25_lane, ffe_cap_sel26_lane, ffe_cap_sel27_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f5  ,  0xFFFF  ,   0x0  ));

        /* ffe_cap_sel28_lane, ffe_cap_sel29_lane, ffe_cap_sel2a_lane, ffe_cap_sel2b_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f6  ,  0xFFFF  ,   0x0  ));

        /* ffe_cap_sel2c_lane, FFE_CAP_SEL2D_LANE, FFE_CAP_SEL2E_LANE, FFE_CAP_SEL2F_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x1f7  ,  0xFFFF  ,   0x259F  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x170 ,  9  ,  1  ,   0    )); /* rx_ctle_vicm_smplr_sel_lane  */

        /* rx_ctle_vicm1_ctrl_lane, rx_ctle_vicm2_ctrl_lane, rx_vicm_smplr_ctrl_lane, rx_vicm_dfe_ctrl_lane, */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0xcd  ,  0xFF0  ,   0x5A0  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x88  ,  8  ,  2  ,   3    )); /* vref_sampler_vcm_sel         */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x198 ,  7  ,  4  ,   0    )); /* sumf_boost_target_k_lane     */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x1eb ,  0  ,  10 ,   0x3F0)); /* sumf_boost_target_c_lane     */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x161 ,  15 ,  1  ,   0    )); /* EO_BASED_LANE                */

        /* RX_MAX_DFE_ADAPT_0_LANE, RX_MAX_DFE_ADAPT_1_LANE, CDR_MAX_DFE_ADAPT_0_LANE, CDR_MAX_DFE_ADAPT_2_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x156  ,  0xFFC0  ,   0x3300  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x15a ,  14 ,   2 ,   3    )); /* TX_MAX_DFE_ADAPT_LANE        */

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x4e  , 0 , 3,  0x0)); /* G1_FFE_SETTING_FORCE_LANE     */

        /* G1_TX_VREF_TXDRV_SEL_LANE[2:0], G1_RX_TRAIN_MODE[12:12] */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x48  ,  0x1007  ,   0x1007  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x4b  , 12, 1,  0x1)); /* G1_RX_DFE_EN                  */

        /* G1_TX_SLEWRATE_EN_LANE[1:0], G1_TX_SLEWCTRL0_LANE[3:2], G1_TX_SLEWCTRL1_LANE[4:3] */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x46  ,  0x3F  ,   0x0  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x305 , 1 , 3,  0x4)); /* VTH_TXIMPCAL                  */

        /* DFE_RES_F0A_HIGH_THRES_LANE[7:4], DFE_RES_F0A_LOW_THRES_LANE[3:0] */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x175  ,  0xFF  ,   0x52  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x5F  , 0 , 4,  0x0)); /* G1_FFE_RES_SHIFT_LANE         */

        /* RX_CTLE_DATA_RATE1_FORCE_LANE[10:10], RX_CTLE_DATA_RATE2_FORCE_LANE[11:11], RX_CTLE_DATA_RATE3_FORCE_LANE[12:12] */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x6a  ,  0x1C00  ,   0x1C00  ));

        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x5A  , 0 , 3,  0x7)); /* G1_RX_CTLE_DATA_RATE1_LANE    */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x5B  , 0 , 3,  0x7)); /* G1_RX_CTLE_DATA_RATE2_LANE    */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x5C  , 0 , 3,  0x7)); /* G1_RX_CTLE_DATA_RATE2_LANE    */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x53  , 12, 1,  0x1)); /* G1_REAL_TIME_PHASE_ADAPT_LANE */
    }

    if (_10_3125G_SR_LR == serdesSpeed)
    {
        /* G1_RX_CTLE_DATA_RATE1_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x1168 - 0x1000) / 4, 0x7, 0x5));

        /* G1_RX_CTLE_DATA_RATE2_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x116C - 0x1000) / 4, 0x7, 0x5));

        /* G1_RX_CTLE_DATA_RATE3_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, (0x1170 - 0x1000) / 4, 0x7, 0x5));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesArrayPowerCtrl function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesArrayPowerCtrl
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
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT   txParameters;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT   rxParameters;
    MV_HWS_PER_SERDES_INFO_PTR         serdesInfo;
    GT_U32                             mcesdEnum;
    GT_U16                             retries = PLL_INIT_POLLING_RETRIES;
    GT_U32                             data;
    GT_BOOL                            allStable;

    MCESD_DEV_PTR                  sDev;
    E_C12GP41P2V_SERDES_SPEED      speed = 0;
    E_C12GP41P2V_REFFREQ           refFreq = 0;
    E_C12GP41P2V_REFCLK_SEL        refClkSel = 0;
    E_C12GP41P2V_DATABUS_WIDTH     dataBusWidth = 0;
    S_C12GP41P2V_PowerOn           powerOnConfig;
    MV_HWS_SERDES_SPEED            baudRate = serdesConfigPtr->baudRate;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);

    if(powerUp)
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Power-on lane */
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_C12GP41P2V_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_C12GP41P2V_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_C12GP41P2V_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_C12GP41P2V_DATABUS_WIDTH)mcesdEnum;
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);

            /* Analog Unreset */
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], UNRESET));

            /* For 10Bit - need to use half bus (SerDes bus-width is set to 20Bit) */
            if(serdesConfigPtr->busWidth == _10BIT_ON)
            {
                CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SERDES_EXTERNAL_CONFIGURATION_0, (1 << 14), (1 << 14)));
            }
            else
            {
                CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SERDES_EXTERNAL_CONFIGURATION_0, (0 << 14), (1 << 14)));
            }

            powerOnConfig.powerLaneMask = 0x1;
            powerOnConfig.initTx        = GT_TRUE;
            powerOnConfig.initRx        = GT_TRUE;
            powerOnConfig.txOutputEn    = GT_FALSE;
            powerOnConfig.dataPath      = C12GP41P2V_PATH_EXTERNAL;
            powerOnConfig.refClkSel     = refClkSel;
            powerOnConfig.dataBusWidth  = dataBusWidth;
            powerOnConfig.speed         = speed;
            powerOnConfig.refFreq       = refFreq;

            MCESD_CHECK_STATUS(API_C12GP41P2V_PowerOnSeq(sDev, powerOnConfig));

        }

        /* Wait for stable PLLs */
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[0],NULL,NULL,&sDev));
        do
        {
            allStable = GT_TRUE;
            for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
            {
                CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SERDES_EXTERNAL_STATUS_0, &data, PLL_INIT_POLLING_DONE));
                if (data != PLL_INIT_POLLING_DONE)
                {
                    allStable = GT_FALSE;
                    break;
                }
            }
            MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, PLL_INIT_POLLING_DELAY));
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
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesDbTxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, &txParameters));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txParameters));

            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesDbRxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, &rxParameters));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxParameters));

            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesTxRxExtraConfiguraion(devNum, portGroup, serdesArr[serdesIterator], serdesConfigPtr->baudRate));

            MCESD_CHECK_STATUS(API_C12GP41P2V_HwSetPinCfg(sDev, C12GP41P2V_PIN_RX_INIT0, 1));
            MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 1));
            MCESD_CHECK_STATUS(API_C12GP41P2V_HwSetPinCfg(sDev, C12GP41P2V_PIN_RX_INIT0, 0));

            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));

            MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxOutputEnable(sDev, MCESD_TRUE));
        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],NULL,NULL,&sDev));
            MCESD_CHECK_STATUS(API_C12GP41P2V_PowerOffLane(sDev));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], RESET));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesArr[serdesIterator], 0x4  , 11, 1,  0x1)); /* sft_rst_only_reg */
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesPowerCtrl function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyC12GP41P2VSerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesRxAutoTuneStart function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StartTraining(sDev, C12GP41P2V_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StopTraining(sDev, C12GP41P2V_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesTxAutoTuneStart function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_SERDES_SPEED speed;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(training)
    {
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x24b , 0x3FF,  0x3FF);        /* TRX_TRAIN_TIMER                                       */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x24a , 0x1,    0x0);          /* LINK_TRAIN_MODE_LANE                                  */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x24a , 0x200,  0x200);        /* TX_TRAIN_CHK_INIT                                     */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x4e  , 0x1000, 0x0);          /* G1_FFE_SETTING_FORCE_LANE                             */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x104 , 0xFF,   0x3F);         /* DFE_UPDATE_EN_LANE                                    */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x5   , 0x4000, 0x0);          /* DET_BYPASS_LANE                                       */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x240 , 0x1,    0x1);          /* ETHERNET_MODE                                         */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x71  , 0x30,   0x10);         /* TX_TRAIN_PAT_SEL_LANE                                 */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x252 , 0x800,  0x800);        /* TX_TRAIN_PAT_TWO_ZERO                                 */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x252 , 0x200,  0x0);          /* TX_TRAIN_PAT_TOGGLE                                   */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x252 , 0x1FF,  0x88);         /* TRAIN_PAT_NUM                                         */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x1b4 , 0xF,    0x1);          /* TX_PRESET_INDEX_LANE                                  */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x171 , 0x400,  0x0);          /* TX_TRAIN_START_NO_WAIT_LANE                           */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x171 , 0x800,  0x0);          /* TX_TRAIN_START_SQ_EN_LANE                             */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x171 , 0x1000, 0x0);          /* TX_TRAIN_START_FRAME_DETECTED_EN_LANE                 */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x171 , 0x2000, 0x2000);       /* TX_TRAIN_START_FRAME_LOCK_EN_LANE                     */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x171 , 0x4000, 0x4000);       /* TX_TRAIN_START_WAIT_TIME_EN_LANE                      */
        mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x24a , 0x100,  0x0);          /* PATTERN_LOCK_LOST_TIMEOUT_EN_LANE (Not in the manual) */

        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxBitRate(sDev, &speed));
        
        if ( speed == C12GP41P2V_SERDES_10P3125G || speed == C12GP41P2V_SERDES_11P5625G )
        {
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x259, 8, 6,  0x10);  /* tx_amp_default1 */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x259 ,4, 4,  0x3);   /* tx_emph1_default1 */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x259 ,0, 4,  0x3);   /* tx_emph0_default1 */

            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x25A , 8, 6,  0x1D); /* tx_amp_default2 */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x25A , 4, 4,  0x0);  /* tx_emph1_default2 */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x25A , 0, 4,  0x0);  /* tx_emph0_default2 */

            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x252 , 10, 1, 0x0);  /* TX_TRAIN_PAT_MODE */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x258 , 4, 4,  0x2);  /* local_tx_preset_index */
        }

        MCESD_CHECK_STATUS(API_C12GP41P2V_StartTraining(sDev, C12GP41P2V_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StopTraining(sDev, C12GP41P2V_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesTxAutoTuneStop function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyC12GP41P2VSerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesAutoTuneStart function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesAutoTuneStart
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
        CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_TRUE;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesAutoTuneStatus function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesAutoTuneStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    MCESD_DEV_PTR              sDev;
    MCESD_BOOL                 completed;
    MCESD_BOOL                 failed;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(NULL != rxStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_CheckTraining(sDev, C12GP41P2V_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_CheckTraining(sDev, C12GP41P2V_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesAutoTuneStatusShort
*           function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesAutoTuneStatusShort
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
        MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_RX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_RX_TRAIN_FAILED0, &failedData));
        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_TX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_TX_TRAIN_FAILED0, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesAutoTuneResult function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MCESD_DEV_PTR                               sDev;
    MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS  *results;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U16 val16;
    MCESD_16  vals16;
    MCESD_U32 tapsI;
    GT_U32    gtVal;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC12GP41P2VResults;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x258, 0, 1, &gtVal));

    if ( gtVal )
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PRE_CTRL, &val));
        results->txComphyC12GP41P2V.pre = (GT_U8)val;
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PEAK_CTRL, &val));
        results->txComphyC12GP41P2V.peak = (GT_U8)val;
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_POST_CTRL, &val));
        results->txComphyC12GP41P2V.post = (GT_U8)val;
    }
    else
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x257, 4 ,4, &gtVal));
        results->txComphyC12GP41P2V.pre = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x257, 8 ,6, &gtVal));
        results->txComphyC12GP41P2V.peak = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x257, 0 ,4, &gtVal));
        results->txComphyC12GP41P2V.post = gtVal;
    }
    
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xA7, 4 ,4, &gtVal));
    results->analogPre = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xA7, 0 ,4, &gtVal));
    results->analogPeak = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xA7, 8 ,4, &gtVal));
    results->analogPost = gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SHIFT, &val));
    results->rxComphyC12GP41P2V.resShift = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL,   &val));
    results->adaptedResSel              = (GT_U32)val;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x4E, 0, 3, &gtVal));
    results->rxComphyC12GP41P2V.resSel = (GT_U32)gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL,   &val));
    results->adaptedCapSel             = (GT_U32)val;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x4D, 0, 4, &gtVal));
    results->rxComphyC12GP41P2V.capSel = (GT_U32)gtVal;

    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x4E, 12, 1, &gtVal));
    results->ffeSettingForce           = (GT_U32)gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUFI, &val));
    results->rxComphyC12GP41P2V.selmufi = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUFF, &val));
    results->rxComphyC12GP41P2V.selmuff = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUPI, &val));
    results->rxComphyC12GP41P2V.selmupi = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUPF, &val));
    results->rxComphyC12GP41P2V.selmupf = (GT_U32)val;

    /* indicator to eye opening */
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x14B, 8, 6, &gtVal)); /* sav_f0d  */
    results->eo = gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetAlign90(sDev, &val16));
    results->align90 = (GT_U16)val16;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetSquelchThreshold(sDev, &vals16));
    results->rxComphyC12GP41P2V.squelch = (GT_16)vals16;

    results->sampler = 0;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetSlewRateParam(sDev, C12GP41P2V_SR_CTRL0, &val));
    results->slewRateCtrl0 = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetSlewRateParam(sDev, C12GP41P2V_SR_CTRL1, &val));
    results->slewRateCtrl1 = (GT_U32)val;

    for(tapsI = 0 ; tapsI < C12GP41P2V_DFE_F7 ; tapsI++)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetDfeTap(sDev, (E_C12GP41P2V_DFE_TAP)tapsI, &vals));
        results->dfe[tapsI] = (GT_32)vals;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesDigitalReset function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesDigitalReset
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        digitalReset
)
{
    GT_UREG_DATA data;

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == RESET) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 6), (1 << 6)));

    return GT_OK;
}
/**
* @internal mvHwsComphyC12GP41P2VSerdesLoopback function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;

    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_DATAPATH      path;
    GT_U32                     mcesdEnum;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* cannot run PRBS with loopback */
    if ( SERDES_LP_DIG_RX_TX == lbMode )
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x70, 15, 1, 0x0));  /* PT_EN_LANE */
    }

    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_C12GP41P2V_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_C12GP41P2V_SetDataPath(sDev, path));
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesLoopbackGet function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;

    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_DATAPATH      path;
    GT_U32                     tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetDataPath(sDev, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesTestGen function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTestGen
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN  MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    MCESD_DEV_PTR                 sDev;
    S_C12GP41P2V_PATTERN          pattern;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &mcesdPattern));
    pattern.pattern  = mcesdPattern;
    pattern.enc8B10B = C12GP41P2V_ENC_8B10B_DISABLE;

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
    MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxRxPattern(sDev, &pattern, &pattern, C12GP41P2V_SATA_NOT_USED, C12GP41P2V_DISPARITY_NOT_USED, userPattern, 0));
    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_ResetComparatorStats(sDev));
        MCESD_CHECK_STATUS(API_C12GP41P2V_StartPhyTest(sDev));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StopPhyTest(sDev));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesTestGenGet function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTestGenGet
(
    GT_U8                        devNum,
    GT_UOPT                      portGroup,
    GT_UOPT                      serdesNum,
    MV_HWS_SERDES_TX_PATTERN     *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE  *modePtr
)
{
    MCESD_DEV_PTR                       sDev;
    S_C12GP41P2V_PATTERN                txPattern;
    S_C12GP41P2V_PATTERN                rxPattern;
    E_C12GP41P2V_SATA_LONGSHORT         sataLongShort;
    E_C12GP41P2V_SATA_INITIAL_DISPARITY sataInitialDisparity;
    char                                userPattern[21];
    MCESD_U8                            userK;
    MCESD_FIELD                         ptEn = F_C12GP41P2VR2P0_PT_EN;
    MCESD_U16                           enable;
    GT_U32                              tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxPattern(sDev, &txPattern, &rxPattern, &sataLongShort,&sataInitialDisparity, userPattern, &userK));
    MCESD_CHECK_STATUS(API_C12GP41P2V_ReadField(sDev, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C12GP41P2V, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern.pattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyC12GP41P2VSerdesTestGenStatus
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTestGenStatus
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN        txPattern,
    IN GT_BOOL                         counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
    MCESD_DEV_PTR                       sDev;
    S_C12GP41P2V_PATTERN_STATISTICS     statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetComparatorStats(sDev, &statistics));

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr   = (GT_U32)statistics.totalErrorBits;
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_ResetComparatorStats(sDev));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesPolarityConfig function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesCheckAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxRxPolarity(sDev, invertTx ? C12GP41P2V_POLARITY_INVERTED : C12GP41P2V_POLARITY_NORMAL,
                                                        invertRx ? C12GP41P2V_POLARITY_INVERTED : C12GP41P2V_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesPolarityConfigGet
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesPolarityConfigGet
(
  GT_U8             devNum,
  GT_UOPT           portGroup,
  GT_UOPT           serdesNum,
  GT_BOOL           *invertTx,
  GT_BOOL           *invertRx
)
{
    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_POLARITY      txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesCheckAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxPolarity(sDev, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == C12GP41P2V_POLARITY_INVERTED);
    *invertRx = (rxPolarity == C12GP41P2V_POLARITY_INVERTED);

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesTxEnbale function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTxEnbale
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL txEnable
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxOutputEnable(sDev, (MCESD_BOOL)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesTxEnbale function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesTxEnbaleGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *txEnable
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxOutputEnable(sDev, (MCESD_BOOL*)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSerdesSignalDetectGet function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesSignalDetectGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
)
{
    MCESD_DEV_PTR              sDev;
    MCESD_BOOL                 squelched;
    MV_HWS_SERDES_LB           lbMode;
    GT_U8 hitCounter = 0;
    GT_U32 i;

    GT_UNUSED_PARAM(portGroup);

    /* in serdes loopback analog tx2rx, we force signal detect */
    CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesLoopbackGet(devNum, portGroup, serdesNum, &lbMode));
    if ( lbMode ==  SERDES_LP_AN_TX_RX )
    {
        *enable = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    for (i = 0; i < 1200; i++)
    {
        /* no cable */
        if ( i > 100 && hitCounter == 0)
        {
            *enable = GT_FALSE;
            return GT_OK;
        }
        /* if sq == 0 meaning we have a signal */
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetSquelchDetect(sDev, &squelched));
        /**enable = squelched ? GT_FALSE : GT_TRUE;*/
        if ( squelched == 0 )
        {
            hitCounter++;
            if ( hitCounter >= 20 )
            {
                *enable = GT_TRUE;
                return GT_OK;
            }
        }
    }
    *enable = GT_FALSE; /* signal is not stable */

    return GT_OK;
}

/** 
* @internal mvHwsComphyC12GP41P2VSerdesCdrLockStatusGet function
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
static GT_STATUS mvHwsComphyC12GP41P2VSerdesCdrLockStatusGet
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
    MCESD_CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    /*TODO add CDR SUPPORT*/
    return GT_OK;
}

extern GT_STATUS mvHwsComH28nmSerdesTxIfSelect
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
);

extern GT_STATUS mvHwsComH28nmSerdesTxIfSelectGet
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32     *serdesTxIfNum
);

static GT_STATUS mvHwsComphyC12GP41P2VSerdesOperation
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
    MCESD_U32 resSel, capSel;
    GT_32     temperature = 0;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(operation);
    GT_UNUSED_PARAM(data);
    GT_UNUSED_PARAM(result);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    switch ( operation )
    {
    case MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_E:

        /* get Temperature!! */
        if (GT_OK != mvHwsDeviceTemperatureGet(devNum, &temperature))
            return GT_FAIL;

        MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL,   &resSel));
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL,   &capSel));

        if ( temperature <= MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MIN_TEMPERATURE )
        {
            resSel += 2;
            capSel += 5;
        }
        else if ( temperature > MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MIN_TEMPERATURE && temperature <= MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_TEMPERATURE )
        {
            resSel += 1;
            capSel += 1;
        }
        else if ( temperature > MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_TEMPERATURE )
        {
            resSel += 0;
            capSel += 0;
        }
        /* res_sel and cap_sel are limited by 0x7 and 0xF, respectively. Update is permitted to the max values (maxResSel = 0x7, maxCapSel = 0xF) */
        resSel = (resSel < MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_RES_SEL) ? resSel : MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_RES_SEL;
        capSel = (capSel < MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_CAP_SEL) ? capSel : MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_CAP_SEL;

        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL, resSel));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL, capSel));

        break;

    case MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E:

        /* Reset For Analog DFE Registers */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x14, 14, 1, 0x1));
        MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, DFE_RESET_DELAY));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x14, 14, 1, 0x0));

        break;

    default:
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VIfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC12GP41P2VIfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_C12GP41P2V])
    {
        funcPtrArray[COM_PHY_C12GP41P2V] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_C12GP41P2V])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_C12GP41P2V], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc     ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc          ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc        ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc         ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc         ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc     ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc     ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc       ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc      ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc      ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc               ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset            ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc         ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc         ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc             ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc          ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc       ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc         ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc      ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc            ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc         ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxIfSelectFunc          ,COM_PHY_C12GP41P2V, mvHwsComH28nmSerdesTxIfSelect);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxIfSelectGetFunc       ,COM_PHY_C12GP41P2V, mvHwsComH28nmSerdesTxIfSelectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc        ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc     ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesOperation);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc          ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesDbRxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc          ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc    ,COM_PHY_C12GP41P2V, mvHwsComphyC12GP41P2VSerdesCdrLockStatusGet);

    return GT_OK;
}

#endif /* C12GP41P2V */

