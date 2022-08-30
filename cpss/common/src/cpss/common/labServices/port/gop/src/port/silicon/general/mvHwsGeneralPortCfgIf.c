/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsGeneralPortCfgIf.c
*
* @brief This file contains API for port configuartion and tuning parameters
*
* @version   48
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>


/**************************** Definition ***************************************/
#define MV_HWS_AVAGO_GUI_ENABLED (0x1)
#define MV_HWS_AVAGO_GUI_MASK    (0x1)

/**************************** Pre-Declarations *********************************/
#ifdef CHX_FAMILY
GT_U32 mvHwsPortToD2dChannelConvert
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);
GT_U32 mvHwsPortToD2DIndexConvert
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);
#endif


#ifdef CHX_FAMILY
extern  GT_BOOL hwsPpHwTraceFlag;
#endif

#if ((!defined MV_HWS_REDUCED_BUILD))/* || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT))*/

/**
* @internal mvHwsUsxModeCheck function
* @endinternal
*
* @brief  Checks if port is USX.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_BOOL mvHwsUsxModeCheck
(
    IN GT_U8                           devNum,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode
)
{
    GT_BOOL isUsx = GT_FALSE;

    switch(HWS_DEV_SILICON_TYPE(devNum))
    {
        case AC5P:
            if((portMode == _10G_SXGMII)  || (portMode == _5G_SXGMII)   || (portMode == _10G_DXGMII) ||
               (portMode == _5G_DXGMII)   || (portMode == _20G_QXGMII)  || (portMode == _20G_DXGMII) ||
               (portMode == _2_5G_SXGMII) || (portMode == _10G_QXGMII)  || (portMode == _20G_OXGMII) ||
               (portMode == _5G_QUSGMII)  || (portMode == _10G_OUSGMII) || (portMode == QSGMII))
            {
                isUsx = GT_TRUE;
            }
            break;

        case AC5X:
            if(phyPortNum < MV_HWS_AC5X_GOP_PORT_CPU_0_CNS)
            {
                isUsx = GT_TRUE;
            }
            if((portMode == _10G_SXGMII)  || (portMode == _5G_SXGMII)   || (portMode == _10G_DXGMII) ||
               (portMode == _5G_DXGMII)   || (portMode == _20G_QXGMII)  || (portMode == _20G_DXGMII) ||
               (portMode == _2_5G_SXGMII) || (portMode == _10G_QXGMII)  || (portMode == _20G_OXGMII) ||
               (portMode == _5G_QUSGMII)  || (portMode == _10G_OUSGMII) || (portMode == QSGMII))
            {
                isUsx = GT_TRUE;
            }
            break;

        default:
            break;
    }

    return isUsx;
}
#endif

/**
* @internal mvHwsIsReducedPort function
* @endinternal
*
* @brief  Checks if port is CPU/reduced for Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ((!defined MV_HWS_REDUCED_BUILD) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT))
GT_BOOL mvHwsMtipIsReducedPort
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
    GT_BOOL isCpu = GT_FALSE;
#ifndef RAVEN_DEV_SUPPORT
#ifdef CHX_FAMILY
    if(hwsIsIronmanAsPhoenix())
    {
        return GT_FALSE;
    }
    else
    if(hwsDeviceSpecInfo[devNum].devType == AC5P)
    {
        if(phyPortNum == MV_HWS_AC5P_GOP_PORT_CPU_CNS)
        {
            isCpu = GT_TRUE;
        }
    }
    else if(hwsDeviceSpecInfo[devNum].devType == Harrier)
    {
        return GT_FALSE;
    }
    else if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        if((phyPortNum == MV_HWS_AC5X_GOP_PORT_CPU_0_CNS) || (phyPortNum == MV_HWS_AC5X_GOP_PORT_CPU_1_CNS))
        {
            isCpu = GT_TRUE;
        }
    }
    else
#endif /*#ifdef CHX_FAMILY*/
#endif
    if(hwsDeviceSpecInfo[devNum].devType == Raven) /* relevant to : RAVEN_DEV_SUPPORT */
    {
        if ( phyPortNum == 16) {
            return GT_TRUE;
        }
        return GT_FALSE;
    }
    else  /* Falcon */
    {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        GT_U32  numOfTiles = hwsDeviceSpecInfo[devNum].numOfTiles;
#else
        GT_U32  numOfTiles = 1;/* just for compilation of 'MV_HWS_REDUCED_BUILD_EXT_CM3' */
#endif
        /* each Raven hold CPU port - each Tile with 4 Ravens  */
        GT_U32  numCpuPorts = numOfTiles * 4;

        if(phyPortNum  < hwsDeviceSpecInfo[devNum].portsNum &&
           phyPortNum >= (hwsDeviceSpecInfo[devNum].portsNum-numCpuPorts))
        {
            isCpu = GT_TRUE;
        }
    }
#ifdef FALCON_DEV_SUPPORT
    isCpu = GT_FALSE;
#endif

    return isCpu;
}
#endif /*((!defined MV_HWS_REDUCED_BUILD) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT))*/


#if (!defined(ALDRIN_DEV_SUPPORT)) && (!defined(RAVEN_DEV_SUPPORT)) && (!defined(FALCON_DEV_SUPPORT))
static GT_STATUS mvHwsPortAvagoSerdesTunePreConfigSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 resetEnable
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32                      i, regData, counter;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortEnhanceTuneSet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if((HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO) && (HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO_16NM))
        {
            return GT_NOT_SUPPORTED;
        }
    }


    /* 50/40G KR2 NO FEC workaround */
    if((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        if((curPortParams.portFecMode == FEC_OFF) &&
          ((portMode == _50GBase_KR2) || (portMode == _40GBase_KR2) || (portMode == _50GBase_SR2) || (portMode == _50GBase_CR2) || (portMode == _52_5GBase_KR2)))
        {
            if(resetEnable == GT_FALSE)
            {
                for (i = 0; i < curPortParams.numOfActLanes; i++)
                {
                    counter = 0;

                    /* Poll for o_core_status[4] == 1 */
                    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), AVAGO_CORE_STATUS, &regData, 0));
                    while(((regData >> 4) & 1) != 1)
                    {
                        counter++;
                        hwsOsTimerWkFuncPtr(1);
                        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), AVAGO_CORE_STATUS, &regData, 0));
                        if(counter == 50)
                        {
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ARG_STRING_MAC("SD Clock to Gearbox is not ready\n"));
                        }
                    }
                }
            }

            regData = (resetEnable == GT_TRUE) ? 0 : 1;
            /* Disable rx_tlat - stop clock from SERDES to GearBox */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), SERDES_EXTERNAL_CONFIGURATION_0, (regData << 13), (1 << 13)));
            }

            /* Reset sd_rx_reset_ - GearBox RX reset*/
            if(phyPortNum % 4 == 0)
            {
                regData = (resetEnable == GT_TRUE) ? 0x0 : 0x3;
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0x3));
            }
            else if(phyPortNum % 2 == 0)
            {
                regData = (resetEnable == GT_TRUE) ? 0x0 : 0xC;
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0xC));
            }
            else
            {
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal port num for 50G/40G WA"));
            }
        }
    }

    return GT_OK;
}
#endif

/**
* @internal mvHwsGeneralPortSignalDetectMask function
* @endinternal
*
* @brief   mask/unmask-signal detect interrupt
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralPortSignalDetectMask
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsGeneralPortSignalDetectMask null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, (curLanesList[i] & 0xFFFF), SERDES_EXTERNAL_CONFIGURATION_0, (enable << 7), (1 << 7)));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortManualCtleConfig function
* @endinternal
*
* @brief   configures SERDES Ctle parameters for specific one SERDES lane or
*         for all SERDES lanes on port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] laneNum                  - Serdes number on port or
*                                      0xFF for all Serdes numbers on port
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualCtleConfig
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  startLaneNum;
    GT_U32  endLaneNum;
    GT_U32  i;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT  rxConfig;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
        || (HWS_DEV_SERDES_TYPE(devNum, laneNum) != AVAGO))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortManualCtleConfig null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* for all lanes on port */
    if (laneNum == 0xFF)
    {
        startLaneNum = 0;
        endLaneNum = (curPortParams.numOfActLanes) - 1;
    }
    else /* for specific one lane on port */
    {
        startLaneNum = endLaneNum = laneNum;
    }

    for (i=startLaneNum; i <= endLaneNum; i++)
    {
        if (laneNum == 0xFF)
        {
            hwsOsMemCopyFuncPtr(&rxConfig.rxAvago, &configParams[i], sizeof(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA));
        }
        else
        {
            hwsOsMemCopyFuncPtr(&rxConfig.rxAvago, configParams, sizeof(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA));
        }
        CHECK_STATUS(mvHwsAvagoSerdesManualCtleConfig(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &rxConfig));
    }

    return GT_OK;
}


/**
* @internal mvHwsPortManualCtleConfigGet function
* @endinternal
*
* @brief   Get SERDES Ctle parameters of each SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] laneNum                  - Serdes number on port or
*                                      0xFF for all Serdes numbers on port
*
* @param[out] configParams[]           - array of Ctle params structures, Ctle parameters of each SERDES lane.
*                                      Ctle params structures:
*                                      dcGain         DC-Gain value        (rang: 0-255)
*                                      lowFrequency   CTLE Low-Frequency   (rang: 0-15)
*                                      highFrequency  CTLE High-Frequency  (rang: 0-15)
*                                      bandWidth      CTLE Band-width      (rang: 0-15)
*                                      squelch        Signal OK threshold  (rang: 0-310)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualCtleConfigGet
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams[]
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  startLaneNum;
    GT_U32  endLaneNum;
    GT_U32  i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
        || (HWS_DEV_SERDES_TYPE(devNum, laneNum) != AVAGO))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortManualCtleConfigGet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* for all lanes on port */
    if (laneNum == 0xFF)
    {
        startLaneNum = 0;
        endLaneNum = (curPortParams.numOfActLanes) - 1;
    }
    else /* for specific one lane on port */
    {
        startLaneNum = endLaneNum = laneNum;
    }

    for (i=startLaneNum; i <= endLaneNum; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesManualCtleConfigGet(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &configParams[i]));
    }

    return GT_OK;
}
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)

/**
* @internal mvHwsPortEnhanceTuneSet function
* @endinternal
*
* @brief   Perform Enhance Tuning for finding the best peak of the eye
*         on specific port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneSet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_LF,
    GT_U8       max_LF
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      curLanesList[MAX_AVAGO_SERDES_NUMBER];
    GT_U32                       i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));


    if((portMode == _100GBase_MLG) && ((phyPortNum % 4) != 0))
    {
        phyPortNum &= 0xFFFFFFFC;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortEnhanceTuneSet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if(HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO)
        {
            return GT_NOT_SUPPORTED;
        }
    }
#if (!defined(ALDRIN_DEV_SUPPORT)) && (!defined(RAVEN_DEV_SUPPORT)) && (!defined(FALCON_DEV_SUPPORT))
    CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
#endif
    CHECK_STATUS(mvHwsAvagoSerdesEnhanceTune(devNum, portGroup, curLanesList, curPortParams.numOfActLanes, min_LF, max_LF));
#if (!defined(ALDRIN_DEV_SUPPORT)) && (!defined(RAVEN_DEV_SUPPORT)) && (!defined(FALCON_DEV_SUPPORT))
    CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));
#endif
    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

/**
* @internal mvHwsPortScanLowFrequency function
* @endinternal
*
* @brief   Perform Scan LF algorithm
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] minLf                   - Minimum LF value that can
*       be set on Serdes (0...15)
* @param[in] maxLf                   - Maximum LF value that can
*       be set on Serdes (0...15)
** @param[in] hf                   - HF value that can
*       be set on Serdes (0...15)
* @param[in] bw                   - BW value that can be set on
*       Serdes (0...15)
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortScanLowFrequency
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       minLf,
    GT_U8       maxLf,
    GT_U8       hf,
    GT_U8       bw
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32                      i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if((portMode == _100GBase_MLG) && ((phyPortNum % 4) != 0))
    {
        phyPortNum &= 0xFFFFFFFC;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortScanLowFrequency null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    for ( i = 0; i < curPortParams.numOfActLanes; i++ )
    {
        if(HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO)
        {
            return GT_NOT_SUPPORTED;
        }
    }
    CHECK_STATUS(mvHwsAvagoSerdesScanLowFrequency(devNum, portGroup, phyPortNum, portMode, curLanesList,
        curPortParams.numOfActLanes,minLf,maxLf,hf,bw));

    return GT_OK;
}


/**
* @internal mvHwsAvagoSerdesEnhanceTunePhaseAllocation function
* @endinternal
*
* @brief   Allocate memory (if needed) for enhance-tune-by-phase algorithm which
*         is used by port manager.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhaseAllocation
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] =
            (MV_HWS_AVAGO_PORT_MANAGER_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtr[phyPortNum] failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum],
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
    }

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr =
            (MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of enhanceTuneStcPtr failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr,
                       0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC));

        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->parametersInitialized = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesOneShotTunePhaseAllocation function
* @endinternal
*
* @brief   Allocate memory (if needed) for
*         one-shot-tune-by-phase algorithm which is used by port
*         manager.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTunePhaseAllocation
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] =
            (MV_HWS_AVAGO_PORT_MANAGER_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtr[phyPortNum] failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum],
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
    }

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr =
            (MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of mvHwsAvagoSerdesOneShotTunePhaseAllocation failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr,
                       0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC));

        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr->parametersInitialized = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEdgeDetectPhaseAllocation function
* @endinternal
*
* @brief   Allocate memory (if needed) for edge-detect-by-phase algorithm which
*         is used by port manager.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEdgeDetectPhaseAllocation
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] =
            (MV_HWS_AVAGO_PORT_MANAGER_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtr[phyPortNum] failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum],
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
    }

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr =
            (MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of edgeDetectStcPtr failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr,
                       0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC));
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr->initFlag
                           = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortManagerClose function
* @endinternal
*
* @brief   Free all port manager allocated memory.
*
* @param[in] devNum                   - system device number
*                                      phyPortNum - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManagerClose
(
    GT_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    GT_U32 portMacNum;

    /* port manager close */
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr != NULL)
    {
        for (portMacNum=0; portMacNum<hwsDeviceSpecInfo[devNum].portsNum; portMacNum++)
        {
            if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum] != NULL)
            {
                if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->edgeDetectStcPtr != NULL)
                {
                    hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->edgeDetectStcPtr);
                }
                if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->enhanceTuneStcPtr != NULL)
                {
                    hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->enhanceTuneStcPtr);
                }
                hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]);
            }
        }
        hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr);
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr = NULL;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortManagerInit function
* @endinternal
*
* @brief   Allocate inital memory for Hws port manager DB.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManagerInit
(
    GT_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    /* db not needed in FW, and also currently not needed in simulation
       as all APIs using this db return GT_OK in simulation, therefore currently
       we do not need this memory space in simulation also */
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr = (MV_HWS_AVAGO_PORT_MANAGER_STC_PTR*)
        hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC_PTR)*hwsDeviceSpecInfo[devNum].portsNum);
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtrPtr failed"));
    }
    hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr,
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC_PTR)*hwsDeviceSpecInfo[devNum].portsNum);
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEdgeDetectByPhase function
* @endinternal
*
* @brief   This function calls Edge-Detect algorithm phases, in a way that it returns
*         after triggering each RX tune performed inside the Edge-Detect, without
*         affecting the sequence of the Edge-Detect algorithm. This mode of operation
*         allows the caller to utilize the idle time after RX tune trigger.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase                    - current Edge-Detect  to run
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEdgeDetectByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_STATUS rc;
    GT_BOOL continueExecute,isInNonBlockWait;
    GT_U32 timeOut;
    GT_U32 i;

    if (!( HWS_DEV_SILICON_TYPE(devNum) == BobK ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin ||
           HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 ||
           HWS_DEV_SILICON_TYPE(devNum) == Pipe ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                     LOG_ARG_STRING_MAC("edge-detect by phase not supported for device"),
                                     LOG_ARG_GEN_PARAM_MAC((HWS_DEV_SILICON_TYPE(devNum))));
    }

    phaseFinishedPtr=phaseFinishedPtr;

    *phaseFinishedPtr = GT_FALSE;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsAvagoSerdesEdgeDetectByPhase null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if(HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO)
        {
            return GT_NOT_SUPPORTED;
        }
    }

    /* check that edge detect phase db is allocated */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesEdgeDetectPhaseAllocation(devNum, phyPortNum),
                     LOG_ARG_STRING_MAC("allocation of edge detect phase failed"));

    timeOut = 300; /* milisec */

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr->initFlag == GT_FALSE)
    {
        mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb(devNum, phyPortNum, timeOut);
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr->initFlag = GT_TRUE;
    }

    if (phase==1)
    {
        rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase1(devNum, portGroup, curPortParams.numOfActLanes,
                                                  &(curLanesList[0]),50,10);
        if (rc!=GT_OK)
        {
            return rc;
        }

        rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1(devNum,phyPortNum,portGroup,curPortParams.numOfActLanes,
                                          &(curLanesList[0]),50,10);
        if (rc!=GT_OK)
        {
            return rc;
        }
        /* phase1 finished */
        *phaseFinishedPtr = GT_TRUE;
    }
    else
    {
        rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2(devNum,phyPortNum,portGroup,curPortParams.numOfActLanes,
                                                  &(curLanesList[0]),50,10, &continueExecute, &isInNonBlockWait);
        if (rc!=GT_OK)
        {
            return rc;
        }

        if (continueExecute==GT_TRUE && isInNonBlockWait==GT_FALSE)
        {
            rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1(devNum, phyPortNum, portGroup, curPortParams.numOfActLanes,
                                                  &(curLanesList[0]),50,10);
            if (rc!=GT_OK)
            {
                return rc;
            }
        }

    }

    if (phase == 2 && continueExecute == GT_FALSE)
    {
        *phaseFinishedPtr = GT_TRUE;
        mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb(devNum, phyPortNum, timeOut);
    }
#endif

    return GT_OK;
}
/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhase function
* @endinternal
*
* @brief   running one shot tune (iCal) by phases, it take care
* of KR2/CR2 cases with Fec mode disabled, to unharm the
* sequence of port manager.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase....................- current phase
*
* @param[out] phaseFinishedPtr                 - (pointer to)
*       whether or not we finished the last phase.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_FEC_MODE fecCorrect;
    GT_STATUS rc;
    GT_U32 ii;
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *oneShotTuneStcPtr;

    if (!( HWS_DEV_SILICON_TYPE(devNum) == BobK ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin ||
           HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 ||
           HWS_DEV_SILICON_TYPE(devNum) == Pipe ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                     LOG_ARG_STRING_MAC("one-shot-tune by phase not supported for device"),
                                     LOG_ARG_GEN_PARAM_MAC((HWS_DEV_SILICON_TYPE(devNum))));
    }

    rc = GT_OK;
        /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsAvagoSerdesOneShotTuneByPhase null hws param ptr"));
    }
    fecCorrect = curPortParams.portFecMode;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    for (ii = 0; ii < curPortParams.numOfActLanes; ii++)
    {
        if ( HWS_DEV_SERDES_TYPE(devNum, curLanesList[ii]) != AVAGO )
        {
            return GT_NOT_SUPPORTED;
        }
    }

    /* checking that port manager one shot tune db is allocated for the port */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesOneShotTunePhaseAllocation(devNum ,phyPortNum),
                     LOG_ARG_STRING_MAC("one shot tune phase allocation failed"));

    oneShotTuneStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr;

    if (oneShotTuneStcPtr->parametersInitialized==GT_FALSE)
    {
        mvHwsAvagoSerdesOneShotTuneByPhaseInitDb(devNum, phyPortNum);
        oneShotTuneStcPtr->parametersInitialized=GT_TRUE;
    }

    switch (phase)
    {
        case 1:
            rc = mvHwsAvagoSerdesPrePostTuningByPhase(
            devNum,
            portGroup,
            phyPortNum,
            &(curLanesList[0]),
            curPortParams.numOfActLanes,
            GT_TRUE,
            &(oneShotTuneStcPtr->coreStatusReady[0]),
            fecCorrect,
            portMode);
            if (rc != GT_OK)
            {
                return rc;
            }
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL));
            *phaseFinishedPtr = GT_TRUE;
            break;

        case 2:
            rc = mvHwsAvagoSerdesPrePostTuningByPhase(
            devNum,
            portGroup,
            phyPortNum,
            &(curLanesList[0]),
            curPortParams.numOfActLanes,
            GT_FALSE,
            &(oneShotTuneStcPtr->coreStatusReady[0]),
            fecCorrect,
            portMode
            );

            *phaseFinishedPtr = GT_TRUE;
            for (ii=0; ii<curPortParams.numOfActLanes; ii++)
            {
                if (oneShotTuneStcPtr->coreStatusReady[ii] == GT_FALSE)
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
            break;

        default:
            hwsOsPrintf("\n** One Shot Tune: Default switch-case. N/A **\n");
    }

    if (phase==2 && *phaseFinishedPtr == GT_TRUE)
    {
       mvHwsAvagoSerdesOneShotTuneByPhaseInitDb(devNum, phyPortNum);
    }
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSerdesEnhanceTuneSetByPhase function
* @endinternal
*
* @brief   This function calls Enhance-Tune algorithm phases, in a way that it returns
*         after triggering each RX tune performed inside the Enhance-Tune, without
*         affecting the sequence of the Enhance-Tune algorithm. This mode of operation
*         allows the caller to utilize the idle time after RX tune trigger.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneSetByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_LF,
    GT_U8       max_LF,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
)
{
#ifdef ASIC_SIMULATION
    GT_STATUS rc = GT_OK;
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    min_LF = min_LF;
    max_LF = max_LF;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE  fecCorrect;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_STATUS rc;
    GT_BOOL prePostConfigTune = GT_FALSE;
    GT_U32 ii;
    MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC *enhanceTuneStcPtr;
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *oneShotTuneStcPtr = NULL;


    if (!( HWS_DEV_SILICON_TYPE(devNum) == BobK ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin ||
           HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 ||
           HWS_DEV_SILICON_TYPE(devNum) == Pipe ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                     LOG_ARG_STRING_MAC("enhance-tune by phase not supported for device"),
                                     LOG_ARG_GEN_PARAM_MAC((HWS_DEV_SILICON_TYPE(devNum))));
    }

    rc = GT_OK;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsAvagoSerdesEnhanceTuneSetByPhase null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    for ( ii = 0; ii < curPortParams.numOfActLanes; ii++ )
    {
        if ( HWS_DEV_SERDES_TYPE(devNum, curLanesList[ii]) != AVAGO )
        {
            return GT_NOT_SUPPORTED;
        }
    }


    /* checking that port manager enhace tune db is allocated for the port */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesEnhanceTunePhaseAllocation(devNum ,phyPortNum),
                     LOG_ARG_STRING_MAC("enhance tune phase allocation failed"));

    enhanceTuneStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr;


    if ((enhanceTuneStcPtr->parametersInitialized==GT_FALSE) || (phase == 1))
    {
        mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb(devNum, phyPortNum);
        enhanceTuneStcPtr->parametersInitialized=GT_TRUE;
    }
    fecCorrect = curPortParams.portFecMode;
    /* Align CG resets workaround (KR2/CR2 with FEC OFF) before Rx Training for Port Manager EnhanceTune Algorithm */
    if((fecCorrect == FEC_OFF) && HWS_TWO_LANES_MODE_CHECK(portMode))
    {
            prePostConfigTune = GT_TRUE;
            /* checking that port manager one shot tune db is allocated for the port */
            CHECK_STATUS_EXT(mvHwsAvagoSerdesOneShotTunePhaseAllocation(devNum ,phyPortNum),
                     LOG_ARG_STRING_MAC("one shot tune phase allocation failed"));

            oneShotTuneStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr;

            if (oneShotTuneStcPtr->parametersInitialized==GT_FALSE)
            {
                mvHwsAvagoSerdesOneShotTuneByPhaseInitDb(devNum, phyPortNum);
                oneShotTuneStcPtr->parametersInitialized=GT_TRUE;
            }
    }

    switch (phase)
    {
        case 1:
            if (prePostConfigTune)
            {
                rc = mvHwsAvagoSerdesPrePostTuningByPhase(
                devNum,
                portGroup,
                phyPortNum,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                GT_TRUE,
                &(oneShotTuneStcPtr->coreStatusReady[0]),
                fecCorrect,
                portMode);
            }
            /*hwsOsPrintf("\n** Enhance tune: phase 1 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase1(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &enhanceTuneStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneStcPtr->phase_CurrentDelaySize,
                &(enhanceTuneStcPtr->phase_Best_eye[0]));
        break;

        case 2:
            rc =  mvHwsAvagoSerdesEnhanceTunePhase2(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneStcPtr->phase_Best_LF[0]));
            *phaseFinishedPtr = GT_TRUE;
            break;

        case 3:
            /*hwsOsPrintf("\n** Enhance tune: phase 3 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase3(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                enhanceTuneStcPtr->phase_CurrentDelaySize,
                enhanceTuneStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneStcPtr->phase_InOutI,
                &(enhanceTuneStcPtr->phase_Best_LF[0]),
                &(enhanceTuneStcPtr->phase_Best_eye[0]),
                &(enhanceTuneStcPtr->phase_Best_dly[0]),
                &(enhanceTuneStcPtr->subPhase));
            if (rc==GT_OK)
            {
                if (enhanceTuneStcPtr->phase_InOutI == 0xFFFFFFFF)
                {
                    *phaseFinishedPtr = GT_TRUE;
                }
                else
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
            break;

        case 4:
            /*hwsOsPrintf("\n** Enhance tune: phase 4 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase4(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneStcPtr->phase_Best_dly[0]));
        break;

        case 5:
            /*hwsOsPrintf("\n** Enhance tune: phase 5 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase5(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &enhanceTuneStcPtr->phase_LF1_Arr[0]);
        break;

        case 6:
            if (enhanceTuneStcPtr->waitForCoreReady == GT_FALSE)
            {
                /*hwsOsPrintf("\n** Enhance tune: phase 6 **\n");*/
                    rc = mvHwsAvagoSerdesEnhanceTunePhase6(
                        devNum,
                        portGroup,
                        &(curLanesList[0]),
                        curPortParams.numOfActLanes,
                        min_LF,
                        max_LF,
                        /* phase dependant args */
                        &enhanceTuneStcPtr->phase_LF1_Arr[0],
                        &(enhanceTuneStcPtr->phase_Best_eye[0]),
                        &(enhanceTuneStcPtr->phase_Best_dly[0]),
                        &enhanceTuneStcPtr->phase_InOutKk,
                        &enhanceTuneStcPtr->phase_continueExecute);
            }
            if (rc==GT_OK)
            {
                if (enhanceTuneStcPtr->phase_continueExecute==GT_FALSE)
                {
                    if (prePostConfigTune)
                    {
                        rc = mvHwsAvagoSerdesPrePostTuningByPhase(
                        devNum,
                        portGroup,
                        phyPortNum,
                        &(curLanesList[0]),
                        curPortParams.numOfActLanes,
                        GT_FALSE,
                        &(oneShotTuneStcPtr->coreStatusReady[0]),
                        fecCorrect,
                        portMode);
                        *phaseFinishedPtr = GT_TRUE;
                        for (ii=0; ii<curPortParams.numOfActLanes; ii++)
                        {
                            if (oneShotTuneStcPtr->coreStatusReady[ii] == GT_FALSE)
                            {
                                enhanceTuneStcPtr->waitForCoreReady = GT_TRUE;
                                *phaseFinishedPtr = GT_FALSE;
                            }
                        }
                    }
                    else
                    {
                        *phaseFinishedPtr = GT_TRUE;
                    }
                }
                else
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
        break;

        default:
            hwsOsPrintf("\n** Enhance tune: Default switch-case. N/A **\n");
        break;
    }
#if ((!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))
#ifdef RUN_ADAPTIVE_CTLE
    /* save the serdes training LF and Delay values for adaptive CTLE */
    if ((phase == 6) && (*phaseFinishedPtr == GT_TRUE) && (rc == GT_OK))
    {
        for (ii = 0; ii < curPortParams.numOfActLanes; ii++)
        {
            /*serdes index = curLanesList[ii]*/
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[curLanesList[ii]].enhTrainDelay = (GT_U8)(enhanceTuneStcPtr->phase_Best_dly[ii] -15);
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[curLanesList[ii]].currSerdesDelay = (GT_U8)(enhanceTuneStcPtr->phase_Best_dly[ii] -15);
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[curLanesList[ii]].trainLf = (GT_U8)(enhanceTuneStcPtr->phase_Best_LF[ii]);
        }
    }
#endif /*RUN_ADAPTIVE_CTLE*/
#endif /*((!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))*/
    if ( (phase==6 && *phaseFinishedPtr == GT_TRUE) || rc != GT_OK)
    {
        mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb(devNum, phyPortNum);
    }
#endif

    return rc;
}

/**
* @internal mvHwsPortAvagoSerdesRxSignalOkStateGet function
* @endinternal
*
* @brief   Get the rx_signal_ok indication from the port lanes. If all lanes
*         have rx_signal_ok indication, then output value is true. Else,
*         output value is false.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoSerdesRxSignalOkStateGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *signalOk
)
{
    GT_BOOL tmpSignalOk;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];

    HWS_NULL_PTR_CHECK_MAC(signalOk);

    *signalOk = GT_TRUE;
    tmpSignalOk = GT_FALSE;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS_EXT(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList),
                 LOG_ARG_GEN_PARAM_MAC(portMode), LOG_ARG_STRING_MAC("rebuilding active lanes list"));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "RxSignalOkStateGet- curPortParams is NULL ");
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS_EXT(mvHwsAvagoSerdesRxSignalOkGet(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &tmpSignalOk),
                         LOG_ARG_SERDES_IDX_MAC(MV_HWS_SERDES_NUM(curLanesList[i])), LOG_ARG_STRING_MAC("RxSignalOK"));
        /* if at lease one of the serdeses is ont rx_signal_ok, then no rx_signal_ok */
        if (tmpSignalOk == GT_FALSE)
        {
            *signalOk = GT_FALSE;
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoSerdesRxSignalOkChangeGet function
* @endinternal
*
* @brief   Get the rx_signal_ok change indication from the port
*         lanes. If all lanes have rx_signal_ok indication, then
*         output value is true. Else, output value is false.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoSerdesRxSignalOkChangeGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *signalOkChange
)
{
    GT_BOOL tmpSignalOkChange;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];

    HWS_NULL_PTR_CHECK_MAC(signalOkChange);

    *signalOkChange = GT_FALSE;
    tmpSignalOkChange = GT_FALSE;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS_EXT(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList),
                 LOG_ARG_GEN_PARAM_MAC(portMode), LOG_ARG_STRING_MAC("rebuilding active lanes list"));
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "RxSignalOkChangeGet- curPortParams is NULL ");
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS_EXT(mvHwsAvagoSerdesSignalOkChange(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &tmpSignalOkChange),
                         LOG_ARG_SERDES_IDX_MAC(MV_HWS_SERDES_NUM(curLanesList[i])), LOG_ARG_STRING_MAC("RxSignalOKChange"));
        /* if at lease one of the serdeses is ont rx_signal_ok, then no rx_signal_ok */
        if (tmpSignalOkChange == GT_TRUE)
        {
            *signalOkChange = GT_TRUE;
        }
    }

    return GT_OK;
}

#endif /*MV_HWS_REDUCED_BUILD_EXT_CM3*/
#endif /* #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */

/*******************************************************************************
* mvHwsPortEnhanceTuneLitePhaseAllocation
*
* DESCRIPTION:
*       Allocate memory for EnhanceTuneLite by phase algorithm which
*       is used for AP port
*
* INPUTS:
*       devNum     - system device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_STATUS mvHwsPortEnhanceTuneLitePhaseDeviceInit
(
    GT_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    GT_U32 phyPortIndex;

    for (phyPortIndex=0; phyPortIndex < (MV_PORT_CTRL_MAX_AP_PORT_NUM); phyPortIndex++)
    {
        CHECK_STATUS(mvHwsPortEnhanceTuneLiteByPhaseInitDb(devNum, phyPortIndex));
    }
#endif

    return GT_OK;
}
#endif /* defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)*/

/*******************************************************************************
* mvHwsPortEnhanceTuneLiteByPhaseInitDb
*
* DESCRIPTION:
*       Init HWS DB of EnhanceTuneLite by phase algorithm used for AP port
*
* INPUTS:
*       devNum       - system device number
*       apPortIndex  - ap port number index
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_STATUS mvHwsPortEnhanceTuneLiteByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  apPortIndex
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    apPortIndex = apPortIndex;
#else
    GT_U32 i;

    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_CurrentDelaySize = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_CurrentDelayPtr = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_InOutI = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].subPhase = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phaseEnhanceTuneLiteParams.phase = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phaseEnhanceTuneLiteParams.phaseFinished = GT_FALSE;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].parametersInitialized = GT_TRUE;

    for (i=0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_Best_eye[i] = 4;
        hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_Best_dly[i] = 0;
    }
#endif

    return GT_OK;
}
#endif /* defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)*/

/**
* @internal mvHwsAvagoSerdesDefaultMinMaxDlyGet function
* @endinternal
*
* @brief   This function returns the default minimum and maximum delay
*         values according to the given port mode
* @param[in] portMode                 - mode type of port
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDefaultMinMaxDlyGet
(
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8                   *min_dly_ptr,
    GT_U8                   *max_dly_ptr
)
{
    HWS_NULL_PTR_CHECK_MAC(min_dly_ptr);
    HWS_NULL_PTR_CHECK_MAC(max_dly_ptr);

    if ((portMode == _10GBase_KR) ||
        (portMode == _20GBase_KR2) ||
        (portMode == _40GBase_KR4)||
        (portMode == _40GBase_CR4))
    {
        *min_dly_ptr = 25;
        *max_dly_ptr = 30;
    }
#ifndef ALDRIN_DEV_SUPPORT
    else if ((portMode == _25GBase_KR)    ||
             (portMode == _25GBase_CR)    ||
             (portMode == _25GBase_KR_C)  ||
             (portMode == _25GBase_KR_S)  ||
             (portMode == _25GBase_CR_S)  ||
             (portMode == _25GBase_CR_C)  ||
             (portMode == _50GBase_CR)    ||
             (portMode == _50GBase_SR_LR) ||
             (portMode == _50GBase_KR)    ||
             (portMode == _50GBase_KR2)   ||
             (portMode == _50GBase_CR2)   ||
             (portMode == _50GBase_SR2)   ||
             (portMode == _50GBase_KR2_C) ||
             (portMode == _50GBase_CR2_C) ||
             (portMode == _100GBase_KR4)  ||
             (portMode == _100GBase_CR4)  ||
             (portMode == _100GBase_SR4)  ||
             (portMode == _100GBase_KR2)  ||
             (portMode == _100GBase_CR2)  ||
             (portMode == _100GBase_SR_LR2)  ||
             (portMode == _200GBase_KR4)  ||
             (portMode == _200GBase_CR4)  ||
             (portMode == _200GBase_SR_LR4)  ||
             (portMode == _200GBase_KR8)  ||
             (portMode == _200GBase_CR8)  ||
             (portMode == _200GBase_SR_LR8)  ||
             (portMode == _400GBase_CR8)  ||
             (portMode == _400GBase_KR8)  ||
             (portMode == _400GBase_SR_LR8) ||
             (portMode == _424GBase_KR8))
    {
        *min_dly_ptr = 15;
        *max_dly_ptr = 21;
    }
#endif
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("portMode is not supported"));
    }

    return  GT_OK;
}

#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/**
* @internal mvHwsPortEnhanceTuneLiteSetByPhase function
* @endinternal
*
* @brief   This function calls Enhance-Tune Lite algorithm phases.
*         The operation is relevant only for AP port with Avago Serdes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase                    -  to perform
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLiteSetByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       phase,
    GT_U8       *phaseFinishedPtr,
    GT_U8       min_dly,
    GT_U8       max_dly
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
    etlMinDelay = etlMinDelay;
    etlMaxDelay = etlMaxDelay;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_PORT_AVAGO_SERDES_NUM];
    GT_STATUS rc = GT_OK;
    MV_HWS_PORT_ENHANCE_TUNE_LITE_STC   *enhanceTuneLiteStcPtr;
    GT_U8 default_min_dly, default_max_dly;
    GT_U32 i;

#if !(defined(ALDRIN_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
    GT_U32 ii = 0;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif
    /* For AP port: delay values for EnhanceTuneLite configuration */
    GT_U8  static_delay_10G[] = {25, 26, 28, 30}; /* best delay values for 10G speed */
#ifndef ALDRIN_DEV_SUPPORT
    GT_U8  static_delay_25G[] = {15, 17, 19, 21}; /* best delay values for 25G speed */
#endif
    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    for (i = 0; i < HWS_MAX_PORT_AVAGO_SERDES_NUM; i++)
    {
        if ( HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO )
        {
            return GT_NOT_SUPPORTED;
        }
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("EnhancedTuneLite null HWS param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    enhanceTuneLiteStcPtr = &(hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr
                            [MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE)]);
    if (enhanceTuneLiteStcPtr->parametersInitialized == GT_FALSE)
    {
        mvHwsPortEnhanceTuneLiteByPhaseInitDb(devNum,
                            MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE));
        enhanceTuneLiteStcPtr->parametersInitialized = GT_TRUE;
    }
#ifndef ALDRIN_DEV_SUPPORT
    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        if(curPortParams.serdesSpeed >= _20_625G)
        {
            enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_25G;
            enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U8);
        }
        else
        {
            enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_10G;
            enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U8);
        }
    }
    else
#endif
    {
        enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_10G;
        enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_10G) / sizeof(GT_U8);
    }

    CHECK_STATUS(mvHwsAvagoSerdesDefaultMinMaxDlyGet(portMode, &default_min_dly, &default_max_dly));

    if ( (min_dly>>7) & 1 )
    {
        min_dly = min_dly & 0x7F;
    }
    else
    {
        min_dly = default_min_dly;
    }

    if ( (max_dly>>7) & 1 )
    {
        max_dly = max_dly & 0x7F;
    }
    else
    {
        max_dly = default_max_dly;
    }

    switch (phase)
    {
        case 1:
            rc = mvHwsAvagoSerdesEnhanceTuneLitePhase1(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                curPortParams.serdesSpeed,
                enhanceTuneLiteStcPtr->phase_CurrentDelaySize,
                enhanceTuneLiteStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneLiteStcPtr->phase_InOutI,
                &(enhanceTuneLiteStcPtr->phase_Best_eye[0]),
                &(enhanceTuneLiteStcPtr->phase_Best_dly[0]),
                &(enhanceTuneLiteStcPtr->subPhase),
                min_dly,
                max_dly);
            if (rc != GT_OK)
                HWS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsAvagoSerdesEnhanceTuneLitePhase1 Fail");
            *phaseFinishedPtr = (enhanceTuneLiteStcPtr->phase_InOutI == 0xFF) ? GT_TRUE : GT_FALSE;
            break;

        case 2:
            rc = mvHwsAvagoSerdesEnhanceTuneLitePhase2(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneLiteStcPtr->phase_Best_dly[0]));
            if (rc != GT_OK)
                HWS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsAvagoSerdesEnhanceTuneLitePhase2 Fail");
            *phaseFinishedPtr = GT_TRUE;
            break;

        default:
            hwsOsPrintf("\n EnhancedTuneLite: phase number is not correct \n");
        break;
    }

#endif
#if !(defined(ALDRIN_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
    if ((phase == 2) && (*phaseFinishedPtr == GT_TRUE) && (rc == GT_OK))
    {
        for (ii = 0; ii < curPortParams.numOfActLanes; ii++)
        {
            /*serdes index = curLanesList[ii]*/
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[curLanesList[ii]].enhTrainDelay = (enhanceTuneLiteStcPtr->phase_Best_dly[ii] - 15);
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[curLanesList[ii]].currSerdesDelay = (enhanceTuneLiteStcPtr->phase_Best_dly[ii] - 15);
        }
    }
#endif /*RUN_ADAPTIVE_CTLE*/
#endif

    return GT_OK;
}
#endif /* defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)*/
/*******************************************************************************
* mvHwsPortAvagoRxTuneStartStausCheck
*
* DESCRIPTION:
*       Start the port Rx Tunning and get the status of port Rx Tunning
*       If the Rx Tunning pass, TAP1 mode will set to Enable
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
static GT_STATUS mvHwsPortAvagoRxTuneStartStausCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_AUTO_TUNE_STATUS rxStatus = TUNE_NOT_COMPLITED;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i, j;
    GT_U16  rxStatusBitmap = 0; /* bit per successful rx training result */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoRxTuneStartStausCheck null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])), GT_TRUE));
    }

    for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
    {
        /* Delay in 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        /* check the status of Rx-Training on each related serdes */
        for (j = 0; (j < HWS_MAX_SERDES_NUM) && (j < curPortParams.numOfActLanes); j++)
        {
            /* if lane tune not finished */
            if ( !(rxStatusBitmap & (1<<j)) )
            {
                /* run the Rx-Training status to enable the TAP1 if it passed */
                CHECK_STATUS(mvHwsSerdesAutoTuneStatus(devNum, (portGroup + ((curLanesList[j] >> 16) & 0xFF)),
                                                       (curLanesList[j] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[j] & 0xFFFF)), &rxStatus, NULL));
                if (rxStatus == TUNE_PASS)
                {
                    /* mark lane as finished */
                    rxStatusBitmap |= (1<<j);
                }
            }
        }
        /* if all lanes finished training */
        if ( (rxStatusBitmap & (0xFFFF>>(16-curPortParams.numOfActLanes))) ==
            (0xFFFF>>(16-curPortParams.numOfActLanes)) )
        {
            break;
        }
    }

    /* timeout handling */
    if (i == MV_AVAGO_TRAINING_TIMEOUT)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ARG_STRING_MAC("blocking rx-training reached timeout"));
    }

    return GT_OK;
}
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT) */

/**
* @internal mvHwsPortAvagoTxAutoTuneStatus function
* @endinternal
*
* @brief   Get the status of port Tx Tunning
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if SerdesAutoTuneStatus returns with TUNE_FAIL
*/
static GT_STATUS mvHwsPortAvagoTxAutoTuneStatus
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_AUTO_TUNE_STATUS txStatus;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        return GT_BAD_PTR;
    }
    if (curPortParams.numOfActLanes > HWS_MAX_SERDES_NUM)
    {
        return GT_BAD_PARAM;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        txStatus = TUNE_PASS;
        CHECK_STATUS(mvHwsSerdesAutoTuneStatus(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])),
                                               NULL, &txStatus));

        if (txStatus == TUNE_FAIL)
        {
            return GT_FAIL;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoTxAutoTuneStart function
* @endinternal
*
* @brief   Start the port Tx Tunning
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsPortAvagoTxAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoTxAutoTuneStart null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
#ifndef ASIC_SIMULATION
#ifndef ALDRIN_DEV_SUPPORT
        /* Set PMD PRBS Sequence (PMD Lane ID #) based on Value */
        if((HWS_100G_R4_MODE_CHECK(portMode)) || (HWS_25G_MODE_CHECK(portMode)) || (HWS_TWO_LANES_MODE_CHECK(portMode)))
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                    0x3d, ((3<<12) | i) /* ((Select<<12) | Value) - (Clause 92) Value here is laneNum = 0..3 */,
                    NULL));
        }
        else if (HWS_PAM4_MODE_CHECK(portMode))
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                    0x3d, ((3<<12) | 0x8 | (i%4/* 8 lanes set like 4 lanes twice */)) /* Sets up pattern for Clause 136*/,
                    NULL));
        }
        else
#endif
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                    0x3d, ((3<<12) | 0x4) /* restore default - 0x4 here means - (Clause 72) (default) */,
                    NULL));
        }
#endif
        /* enable TX Tunning */
        CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup,
                                                        MV_HWS_SERDES_NUM(curLanesList[i]),
                                                        IGNORE_RX_TRAINING, START_TRAINING));
    }

    return GT_OK;
}

#ifndef MV_HWS_FREE_RTOS
/**
* @internal mvHwsPortEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] min_dly                  - Minimum delay_cal value:
*                                      - for Serdes speed 10G (25...28)
*                                      - for Serdes speed 25G (15...19)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (21)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLite
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_dly,
    GT_U8       max_dly
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("light enhanced tune null hws param ptr"));
    }


    CHECK_STATUS(mvHwsAvagoSerdesDefaultMinMaxDlyGet(portMode, &min_dly, &max_dly));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if ( HWS_DEV_SERDES_TYPE(devNum, curLanesList[i]) != AVAGO )
        {
            return GT_NOT_SUPPORTED;
        }
    }

    CHECK_STATUS(mvHwsAvagoSerdesEnhanceTuneLite(devNum, portGroup, curLanesList, curPortParams.numOfActLanes, curPortParams.serdesSpeed, min_dly, max_dly));

    return GT_OK;
}
#endif /* MV_HWS_FREE_RTOS */


#ifndef MV_HWS_FREE_RTOS
#ifdef RUN_ADAPTIVE_CTLE
/**
* @internal mvHwsPortAdaptiveCtleBasedTemperature function
* @endinternal
*
* @brief  Run Steady State apdative ctle algorithm. this
*          feature do delay and LF Calibration based Temperature
*          to improve the EO per serdes. in case it does't
*          improved, do rollback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] phase                    - algorithm phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAdaptiveCtleBasedTemperature
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  phase
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_BOOL isLinkUp;
    GT_U32 bitMapIndex = 0;
    GT_BOOL oldValuesValid = GT_FALSE;

    /* avoid warnnings*/
    bitMapIndex = bitMapIndex;
    oldValuesValid = oldValuesValid;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("adaptive CTLE null hws param ptr"));
    }

    /* check that the port is in link up*/
    CHECK_STATUS(mvHwsPortLinkStatusGet(devNum, portGroup, phyPortNum, portMode,
                                        &isLinkUp));

    if (isLinkUp == GT_FALSE )
    {
        /* port up with delay != 0, port down, port up with delay == 0 ( no enh traning) -> have to do flush when link down */
       /* CHECK_STATUS(mvHwsAvagoAdaptiveCtleSerdesesPerPortResetDb(devNum,curLanesList,curPortParams.numOfActLanes));*/
        return GT_OK;
    }

    /* port in link up*/
    bitMapIndex = phyPortNum/HWS_MAX_ADAPT_CTLE_DB_SIZE;

    switch (phase)
    {
    case 1:
        CHECK_STATUS(mvHwsAvagoAdaptiveCtleBasedTemperaturePhase1(devNum, portGroup, phyPortNum, curPortParams.activeLanesList, curPortParams.numOfActLanes));
        break;
    case 2:
         /* in phase 1 the link was down and now the link is UP, we do not have the old EO for compare so just return*/
        oldValuesValid =
            ((hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapOldValuesValid[bitMapIndex] & (0x1 << (phyPortNum%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE);
        if (oldValuesValid == GT_TRUE)
        {
            CHECK_STATUS(mvHwsAvagoAdaptiveCtleBasedTemperaturePhase2(devNum, portGroup, phyPortNum, curPortParams.activeLanesList, curPortParams.numOfActLanes));
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /*RUN_ADAPTIVE_CTLE*/
#endif /* MV_HWS_FREE_RTOS */


/**
* @internal mvHwsPortAvagoDfeCfgSet function
* @endinternal
*
* @brief   Set the DFE (Rx-Traning) configuration mode on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
* @param[in] dfeMode                  - mode of DFE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if SerdesAutoTuneStatus returns with TUNE_FAIL
*/
GT_STATUS mvHwsPortAvagoDfeCfgSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_DFE_MODE         dfeMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;
    GT_32  portStandard, *dfeCfg;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoDfeCfgSet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        portStandard = (GT_32)curPortParams.portStandard;
        dfeCfg = (HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])) == AVAGO_16NM) ? &portStandard : NULL;
        CHECK_STATUS(mvHwsSerdesDfeConfig(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])),
                                          dfeMode, dfeCfg));
    }

    return GT_OK;
}


#ifdef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsPortAvagoRxSignalCheck function
* @endinternal
*
* @brief   Check for all Serdeses on port the Rx Signal.
*         This function need to be run before executing Rx or Tx Training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsPortAvagoRxSignalCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL                 signalDet;
    GT_U32                  i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoRxSignalCheck null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    *txTune = TUNE_NOT_READY;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &signalDet));
        if (!signalDet)
        {
            return GT_OK;
        }
    }
    *txTune = TUNE_READY;
    return GT_OK;
}
#endif /*RAVEN_DEV_SUPPORT*/

/**
* @internal mvHwsPortAvagoTxAutoTuneStateCheck function
* @endinternal
*
* @brief   Get the status of port Tx Tunning for Avago Serdes
*         For AP process: This function doesn't contains a polling checking on
*         Tx status
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsPortAvagoTxAutoTuneStateCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *rxTune,
    MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_AUTO_TUNE_STATUS txStatus;
    GT_U32 i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoTxAutoTuneStateCheck null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    if ( txTune != NULL)
    {
        *txTune = TUNE_PASS;

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            txStatus = TUNE_PASS;
#ifndef ASIC_SIMULATION
            CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStatusShort(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &txStatus));
#endif
            if (txStatus != TUNE_PASS)
            {
                *txTune = txStatus;
                return GT_OK;
            }
        }
    }
    else     if ( rxTune != NULL)
    {
        *rxTune = TUNE_PASS;

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            txStatus = TUNE_PASS;
#ifndef ASIC_SIMULATION
            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &txStatus));
#endif
            if (txStatus != TUNE_PASS)
            {
                *rxTune = txStatus;
                return GT_OK;
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoTRxTuneStop function
* @endinternal
*
* @brief   Stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsPortAvagoTRxTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoTRxTuneStop null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStop(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i])));
    }

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsPortAvagoIsAPGet function
* @endinternal
*
* @brief   This function gets flag is device works in AP mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortAvagoIsAPGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroup,
    IN  GT_U32                      phyPortNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  GT_BOOL                    *isAPPtr
)
{
    GT_UREG_DATA     data;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(NULL == isAPPtr)
    {
        return GT_FAIL;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_FAIL;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT,
                                    curPortParams.portPcsNumber,
                                    AP_STATUS_REG_0, &data, 0));

    *isAPPtr = (data > 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}
#endif /*# MV_HWS_REDUCED_BUILD*/

/**
* @internal mvHwsPortAvagoAutoTuneSet function
* @endinternal
*
* @brief   Calling to port Rx or Tx Tunning function
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - RxTunning or TxTunning
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsPortAvagoAutoTuneSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    void                        *results
)
{
    MV_HWS_AUTO_TUNE_STATUS_RES *tuneRes;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    switch (portTuningMode)
    {
        case RxStatusNonBlocking:
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode, &tuneRes->txTune, NULL));

            break;
        case TRxTuneCfg:
            /* TRxTuneCfg mode is not relevant for Avago Serdes */
            return GT_OK;

        case RxTrainingOnly:
#if (!defined(ALDRIN_DEV_SUPPORT)) && (!defined(RAVEN_DEV_SUPPORT)) && (!defined(FALCON_DEV_SUPPORT))
            CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
#endif
#ifdef RAVEN_DEV_SUPPORT
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvHwsPortAvagoRxSignalCheck(devNum, portGroup, phyPortNum, portMode, &tuneRes->txTune));
            if (tuneRes->txTune == TUNE_NOT_READY)
            {
                return GT_OK;
            }
#endif
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL));
#if (!defined(ALDRIN_DEV_SUPPORT)) && (!defined(RAVEN_DEV_SUPPORT)) && (!defined(FALCON_DEV_SUPPORT))
            CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));
#endif
            break;

            /* blocking rx training not needed in AP FW */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
        case RxTrainingWaitForFinish:
            /* if Rx-Training pass, we need to enable the TAP1 mode */
            CHECK_STATUS(mvHwsPortAvagoRxTuneStartStausCheck(devNum, portGroup, phyPortNum, portMode));
            break;
#endif /* (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)) */

        case TRxTuneStart:
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            tuneRes->txTune = TUNE_READY;
/*#ifndef RAVEN_DEV_SUPPORT*/
#if 0
            CHECK_STATUS(mvHwsPortAvagoRxSignalCheck(devNum, portGroup, phyPortNum, portMode, &tuneRes->txTune));
#endif
            /* this check needed otherwise TRX training frequently fails */
            if(tuneRes->txTune == TUNE_READY)
            {
                CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStart(devNum, portGroup, phyPortNum, portMode));
            }
            if (tuneRes->txTune == TUNE_NOT_READY)
            {
                return GT_OK;
            }
            break;

        case TRxTuneStatus:
            CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStatus(devNum, portGroup, phyPortNum, portMode));
            break;

        case TRxTuneStatusNonBlocking: /* for AP mode */
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode, NULL, &tuneRes->txTune));
            break;

        case TRxTuneStop:
            CHECK_STATUS(mvHwsPortAvagoTRxTuneStop(devNum, portGroup, phyPortNum, portMode));
            break;

        case RxTrainingAdative: /* running Adaptive pCal */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            {
                MV_HWS_AUTO_TUNE_STATUS rxTune;
                GT_U32                  cnt = 0;
                /* stop adaptive before strting again*/
                CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode, &rxTune, NULL));
                if ( rxTune == TUNE_NOT_COMPLITED)
                    CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_STOP_ADAPTIVE));
                do
                {
                    /* check that pcal finished */
                    CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode, &rxTune, NULL));
                    hwsOsExactDelayPtr(devNum, portGroup, 1);
                    cnt++;
                    if ( cnt > 5000 )
                    {
                        HWS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ARG_STRING_MAC("mvHwsPortAvagoTxAutoTuneStateCheck can't stop adaptive"));
                    }
                } while ( rxTune == TUNE_NOT_COMPLITED);
            }
#endif
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_START_ADAPTIVE));
            break;

        case RxTrainingStopAdative:
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_STOP_ADAPTIVE));
            break;

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
        case RxTrainingVsr:
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL_VSR));
            break;
#endif /* (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)) */

        case FixedCtleTraining:
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL_BYPASS_CTLE));
            break;

        default:
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("training mode not supported"));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortGeneralAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum             - system device number
* @param[in] portGroup          - port group (core) number
* @param[in] phyPortNum         - physical port number
* @param[in] portMode           - port standard metric
* @param[in] portTuningMode     - port tuning mode
* @param[in] optAlgoMask        - bit mask for optimization algorithms
* @param[out] results           - port tuning results (optional)
*
* @retval 0                     - on success
* @retval 1                     - on error
*/
GT_STATUS mvHwsPortGeneralAutoTuneSetExt
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                      optAlgoMask,
    void                        *results
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i;

    optAlgoMask = optAlgoMask;
    results     = results;
    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("hwsPortModeParamsGetToBuffer failed"));
    }

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (portTuningMode)
    {
        case RxTrainingOnly:
            for (i = 0 ; i < curPortParams.numOfActLanes ; i++)
            {
                CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, portGroup, curPortParams.activeLanesList[i], HWS_DEV_SERDES_TYPE(devNum,curPortParams.activeLanesList[i]),GT_TRUE));
            }
            break;
        default:
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("training mode not supported"));
    }

    return GT_OK;
}

#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined MICRO_INIT
/**
* @internal mvHwsAvagoSerdesLaunchOneShotiCal function
* @endinternal
*
* @brief   run one shot dfe tune without checking the tune
*          status
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesLaunchOneShotiCal
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL));

    return GT_OK;
}
#endif

/**
* @internal mvHwsPortAvagoAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoAutoTuneSetExt
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                      optAlgoMask,
    void                        *results
)
{
    /* avoid warnings */
    optAlgoMask = optAlgoMask;

    if ((portTuningMode == RxTrainingOnly) || (portTuningMode == TRxTuneStart) || (portTuningMode == TRxTuneStatusNonBlocking) || (portTuningMode == RxStatusNonBlocking))
    {
        if (results == NULL)
        {
            return GT_BAD_PARAM;
        }
    }

    return mvHwsPortAvagoAutoTuneSet(devNum, portGroup, phyPortNum, portMode, portTuningMode, results);
}

#if !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT)
/**
* @internal mvHwsPortTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) of BobK device
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] temperature              - device  (in C)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTemperatureGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      chipIndex,
    GT_32       *temperature
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    temperature = temperature;
    chipIndex = chipIndex;
#else
    GT_32 temperature_tmp;

    if ((hwsDeviceSpecInfo[devNum].serdesType != AVAGO) && (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM))
    {
        return GT_NOT_SUPPORTED;
    }
    /* Get the Temperature (in C) from Avago Serdes, use Serdes #0 as temperature reference of device */
    CHECK_STATUS(mvHwsAvagoSerdesTemperatureGet(devNum, portGroup, chipIndex, &temperature_tmp));
    *temperature = (GT_32)temperature_tmp; /* because GT_32 can be sometimes 'int' sometimes 'long' */

#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)

/**
* @internal mvHwsPortVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) of BobK device
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] voltage                  - device  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVoltageGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      chipIndex,
    GT_U32      *voltage
)
{
    /*if ((HWS_DEV_SERDES_TYPE(devNum) != AVAGO) && (HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM))*/
    if (( hwsDeviceSpecInfo[devNum].serdesType !=  AVAGO ) && ( hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM ))
    {
        return GT_NOT_SUPPORTED;
    }

    /* Get the voltage (in mV) from Avago Serdes, use Serdes #0 as voltage reference of device */
    CHECK_STATUS(mvHwsAvagoSerdesVoltageGet(devNum, portGroup, chipIndex, voltage));

    return GT_OK;
}



#endif /* (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)*/

#if ((!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) || defined(RAVEN_DEV_SUPPORT))
extern GT_STATUS prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
);

#if defined CHX_FAMILY || defined(FALCON_DEV_SUPPORT)
GT_U32 mvHwsPortToD2DIndexConvert
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
    GT_U32 d2dIdx = 0xFFFFFFFF;

#ifndef FALCON_DEV_SUPPORT
    GT_STATUS st;
    GT_U32 channel;

    st = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, phyPortNum, &d2dIdx, &channel);
    if(st != GT_OK)
    {
        return d2dIdx;
    }
#else
    d2dIdx = phyPortNum/8;
#endif
    return d2dIdx;
}

GT_U32 mvHwsPortToD2dChannelConvert
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
    GT_U32 channel = 0xFFFFFFFF;
#ifndef FALCON_DEV_SUPPORT
    GT_U32 d2dIdx = 0xFFFFFFFF;
    GT_STATUS st;
#endif

    if(phyPortNum >= 1024)
    {
        channel = ((phyPortNum-1024)>>1) % HWS_D2D_PORTS_NUM_CNS;
    }
    else
    {
#ifndef FALCON_DEV_SUPPORT
        st = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, phyPortNum, &d2dIdx, &channel);
        if(st != GT_OK)
        {
            return channel;
        }
#else
        channel = phyPortNum % 8;
#endif
    }

    return channel;
}

/**
* @internal mvHwsPortMacPcsStatus function
* @endinternal
*
* @brief   port mac pcs status collector.
*
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatusPtr           - port Status
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS mvHwsPortMacPcsStatus
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   portNum,
    OUT MV_HWS_PORT_STATUS_STC *portStatusPtr
)
{
    GT_U32 address;
    GT_U32 data, data1,data2;
    GT_U32 unitAddr,unitIndex,unitNum;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    if(NULL == portStatusPtr )
        return GT_BAD_PTR;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, NON_SUP_MODE, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    if (HWS_IS_PORT_MULTI_SEGMENT(curPortParams.portStandard))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, portNum, &unitAddr, &unitIndex, &unitNum));

        if (unitAddr == 0) {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        address = MTIP_EXT_SEG_PORT_STATUS + ((portNum % 8) / 4) * 0x14 + unitAddr;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 0x1C003));

        /*hi_ser*/
        portStatusPtr->hiSer = (data >> 16) & 0x1;

        /*degrade_ser*/
        portStatusPtr->degradeSer = (data >> 15) & 0x1;

        /*align_lock*/
        portStatusPtr->alignLock = (data >> 14) & 0x1;

        /*link_status*/
        portStatusPtr->linkStatus = (data >> 1) & 0x1;

        /*link_ok*/
        portStatusPtr->linkOk = data & 0x1;
    }
    else
    {

        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, portNum, &unitAddr, &unitIndex, &unitNum));

        address = MTIP_EXT_PORT_STATUS + (portNum % 8) * 0x18 + unitAddr;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 0x1A01C));

        /*rsfec_aligned*/
        portStatusPtr->rsfecAligned = (data >> 16) & 0x1;

        /*align_done - Relevant only for even ports (0,2,4,6) and only when set to 100G*/
        if (((HWS_100G_R4_MODE_CHECK(curPortParams.portStandard)) || (curPortParams.portStandard == _100GBase_KR2)||
             (curPortParams.portStandard == _100GBase_CR2) || (curPortParams.portStandard == _100GBase_SR_LR2)) && portNum % 2 == 0)
        {
            portStatusPtr->alignDone = (data >> 13) & 0x1;
        }

        /*lpcs_an_done*/
        portStatusPtr->lpcsAnDone = (data >> 4) & 0x1;

        /*lpcs_rx_sync*/
        portStatusPtr->lpcsRxSync = (data >> 3) & 0x1;

        /*lpcs_link_status*/
        portStatusPtr->lpcsLinkStatus = (data >> 2) & 0x1;
    }


    if (curPortParams.portMacType == MTI_MAC_100)
    {

        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_MAC100_UNIT, portNum, &unitAddr, &unitIndex,&unitNum));

        address = MTI_MAC_STATUS +  unitAddr;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 0x7));

        portStatusPtr->phyLos = (data >> 2) & 1;
        portStatusPtr->rxRemFault = (data >> 1) & 1;
        portStatusPtr->rxLocFault = data & 1;


    }
    else if (curPortParams.portMacType == MTI_MAC_400)
    {

        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_MAC400_UNIT, portNum, &unitAddr, &unitIndex,&unitNum));

        address = MTI_MAC_STATUS +  unitAddr;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 0x3));

        portStatusPtr->rxRemFault = (data >> 1) & 1;
        portStatusPtr->rxLocFault = data & 1;
    }

    switch (curPortParams.portPcsType)
    {
    case MTI_PCS_50:
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS50_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS1, &data, 0x84));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS50_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS2, &data1, 0xCC00));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS50_UNIT, curPortParams.portPcsNumber, MTI_PCS_BASER_STATUS2, &data2, 0x7FFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS50_UNIT, curPortParams.portPcsNumber, MTI_PCS_BER_HIGH_ORDER_CNT, &(portStatusPtr->berCounter), 0xFFFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS50_UNIT, curPortParams.portPcsNumber, MTI_PCS_ERR_BLK_HIGH_ORDER_CNT, &(portStatusPtr->errorBlockCounter), 0x3FFF));
        break;
    case MTI_PCS_100:
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS1, &data, 0x84));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS2, &data1, 0xCC00));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_BASER_STATUS2, &data2, 0x7FFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_BER_HIGH_ORDER_CNT, &(portStatusPtr->berCounter), 0xFFFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_ERR_BLK_HIGH_ORDER_CNT, &(portStatusPtr->errorBlockCounter), 0x3FFF));
        break;
    case MTI_PCS_200:
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS200_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS1, &data, 0x84));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS200_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS2, &data1, 0xCC00));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS200_UNIT, curPortParams.portPcsNumber, MTI_PCS_BASER_STATUS2, &data2, 0x7FFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS200_UNIT, curPortParams.portPcsNumber, MTI_PCS_BER_HIGH_ORDER_CNT, &(portStatusPtr->berCounter), 0xFFFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS200_UNIT, curPortParams.portPcsNumber, MTI_PCS_ERR_BLK_HIGH_ORDER_CNT, &(portStatusPtr->errorBlockCounter), 0x3FFF));
        break;
    case MTI_PCS_400:
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS400_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS1, &data, 0x84));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS400_UNIT, curPortParams.portPcsNumber, MTI_PCS_STATUS2, &data1, 0xCC00));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS400_UNIT, curPortParams.portPcsNumber, MTI_PCS_BASER_STATUS2, &data2, 0x7FFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS400_UNIT, curPortParams.portPcsNumber, MTI_PCS_BER_HIGH_ORDER_CNT, &(portStatusPtr->berCounter), 0xFFFF));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS400_UNIT, curPortParams.portPcsNumber, MTI_PCS_ERR_BLK_HIGH_ORDER_CNT, &(portStatusPtr->errorBlockCounter), 0x3FFF));
        break;
    default:
        return GT_BAD_PARAM;
    }

    portStatusPtr->fault = (data >> 7) & 0x1;
    portStatusPtr->pcsReceiveLink = (data >> 2) & 0x1;
    portStatusPtr->devicePresent = (data1 >> 14) & 0x3;
    portStatusPtr->transmitFault = (data1 >> 11) & 0x1;
    portStatusPtr->receiveFault = (data1 >> 10) & 0x1;
    portStatusPtr->berCounter = ((portStatusPtr->berCounter << 6) | ((data2 & 0x3F00)>> 8));
    portStatusPtr->errorBlockCounter = ((portStatusPtr->errorBlockCounter << 8) | (data2 & 0xff ));
    portStatusPtr->hiBer = (data2 >> 14) & 0x1;

    return GT_OK;
}

#endif /*CHX_FAMILY*/

#if !defined ALDRIN_DEV_SUPPORT && !defined(BOBK_DEV_SUPPORT) && !defined(RAVEN_DEV_SUPPORT)
/**
* @internal mvHwsRsfecStatusGet function
* @endinternal
*
* @brief   RSFEC status collector.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portGroup                - port group (core) number
*
* @param[out] rsfecStatusPtr          - RSFEC status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS mvHwsRsfecStatusGet
(
    IN GT_U8    devNum,
    IN GT_U32    portGroup,
    IN GT_U32    portNum,
    OUT MV_HWS_RSFEC_STATUS_STC *rsfecStatusPtr
)
{
    GT_U32                      localPortIndex;
    GT_U32                      baseAddr, unitIndex, unitNum;
    GT_U32                      regAddr=0, regAddr1=0;
    GT_UREG_DATA                regData;
    GT_U32                      i;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_PORT_FEC_MODE        fecMode;
    MV_HWS_UNITS_ID             unitId;
    MV_HWS_RSFEC_UNIT_FIELDS_E  fieldName1, fieldName2, fieldName3, fieldName4, fieldName5;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      pam4Factor = 1, nrz_R8_Factor = 1, startIndex, iterNum;

    if (NULL == rsfecStatusPtr)
    {
        return GT_BAD_PTR;
    }

    hwsOsMemSetFuncPtr(rsfecStatusPtr, 0, sizeof(MV_HWS_RSFEC_STATUS_STC));
#if !defined (RAVEN_DEV_SUPPORT)
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, NON_SUP_MODE, &curPortParams));
    /* Currently on AP ports the curPortParams.portFecMode recieved from hwsPortModeParamsGetToBuffer is not always valid
       if this function is called from host CPU, therefore we need to read the fecMode directly from the hardware
       TODO: Delete this check and replace is to curPortParams.portFecMode after fixing hwsPortModeParamsGetToBuffer to return a valid fecMode */
    CHECK_STATUS(mvHwsPortFecCofigGet(devNum, portGroup, portNum, curPortParams.portStandard, &fecMode));

    if ((fecMode != RS_FEC) && (fecMode != RS_FEC_544_514))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("FEC type not supported"));
    }

    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, curPortParams.portStandard, &convertIdx));

        if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
        {
            fieldName1 = CPU_PCS_RSFEC_UNITS_RSFEC_STATUS_FEC_ALIGN_STATUS_E;
            fieldName2 = CPU_PCS_RSFEC_UNITS_RSFEC_STATUS_AMPS_LOCK_E;
            fieldName3 = CPU_PCS_RSFEC_UNITS_RSFEC_STATUS_HIGH_SER_E;
            fieldName4 = CPU_PCS_RSFEC_UNITS_RSFEC_SYMBLERR0_LO_SYMBLERR0_LO_E;
            fieldName5 = CPU_PCS_RSFEC_UNITS_RSFEC_SYMBLERR0_HI_SYMBLERR0_HI_E;
            unitId = MTI_CPU_RSFEC_UNIT;
        }
        else if (mvHwsUsxModeCheck(devNum, portNum, curPortParams.portStandard) == GT_TRUE)
        {
            fieldName1 = USX_RSFEC_UNITS_RSFEC_STATUS_FEC_ALIGN_STATUS_E;
            fieldName2 = USX_RSFEC_UNITS_RSFEC_STATUS_AMPS_LOCK_E;
            fieldName3 = USX_RSFEC_UNITS_RSFEC_STATUS_HIGH_SER_E;
            fieldName4 = USX_RSFEC_UNITS_RSFEC_SYMBLERR0_LO_SYMBLERR0_LO_E;
            fieldName5 = USX_RSFEC_UNITS_RSFEC_SYMBLERR0_HI_SYMBLERR0_HI_E;
            unitId = MTI_USX_RSFEC_UNIT;
        }
        else
        {
            if(hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                fieldName1 = PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_FEC_ALIGN_STATUS_E;
                fieldName2 = PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_AMPS_LOCK_E;
                fieldName3 = PHOENIX_RSFEC_UNIT_RSFEC_STATUS_P0_HIGH_SER_E;
                fieldName4 = PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR0_LO_SYMBLERR0_LO_E;
                fieldName5 = PHOENIX_RSFEC_UNIT_RSFEC_SYMBLERR0_HI_SYMBLERR0_HI_E;
            }
            else
            {
                fieldName1 = RSFEC_UNIT_RSFEC_STATUS_P0_FEC_ALIGN_STATUS_E;
                fieldName2 = RSFEC_UNIT_RSFEC_STATUS_P0_AMPS_LOCK_E;
                fieldName3 = RSFEC_UNIT_RSFEC_STATUS_P0_HIGH_SER_E;
                fieldName4 = RSFEC_UNIT_RSFEC_SYMBLERR0_LO_SYMBLERR0_LO_E;
                fieldName5 = RSFEC_UNIT_RSFEC_SYMBLERR0_HI_SYMBLERR0_HI_E;
            }
            unitId = MTI_RSFEC_UNIT;
        }

        /* RSFEC STATUS - FEC align status*/
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &(rsfecStatusPtr->fecAlignStatus), NULL));

        /* RSFEC STATUS - amps lock */
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName2, &(rsfecStatusPtr->ampsLock), NULL));
        /*Amps_lock - for 400/200 or 200 when Port=4 , only LSB bit is relevant.*/
        if (HWS_IS_PORT_MULTI_SEGMENT(curPortParams.portStandard))
        {
            rsfecStatusPtr->ampsLock &= 1;
        }
        else
        {
            rsfecStatusPtr->ampsLock &= 0xF;
        }

        /* RSFEC STATUS - high SER */
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName3, &(rsfecStatusPtr->highSer), NULL));

        /****************/
        /*RSFEC_SYMBLERR*/
        /****************/
        if((curPortParams.serdesSpeed == _26_5625G_PAM4) ||
           (curPortParams.serdesSpeed == _27_1875_PAM4) ||
           (curPortParams.serdesSpeed == _28_125G_PAM4) ||
           (curPortParams.serdesSpeed == _26_5625G_PAM4_SR_LR))
        {
            pam4Factor = 2;
        }
        if ((curPortParams.portStandard == _200GBase_KR8) ||
            (curPortParams.portStandard == _200GBase_CR8) ||
            (curPortParams.portStandard == _200GBase_SR_LR8))
        {
            nrz_R8_Factor = 2;
        }

        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            startIndex = convertIdx.ciderIndexInUnit * 2;
        }
        else
        {
            startIndex = convertIdx.ciderIndexInUnit * 4;
        }
        iterNum = curPortParams.numOfActLanes * pam4Factor;
        for(i = 0; i < iterNum; i++)
        {
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, (fieldName4 + startIndex + i*2*nrz_R8_Factor), &regData, NULL));
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, (fieldName5 + startIndex + i*2*nrz_R8_Factor), &(rsfecStatusPtr->symbolError[i]), NULL));
            rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;

        }
    }
    else
    {
        /*RSFEC STATUS*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_RSFEC_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTI_RSFEC_STATUS + localPortIndex * 0x20;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &(rsfecStatusPtr->fecAlignStatus), 0x4000));
        rsfecStatusPtr->fecAlignStatus = (rsfecStatusPtr->fecAlignStatus >> 14) & 1;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &(rsfecStatusPtr->ampsLock), 0xF00));

        /*Amps_lock - for 400/200 or 200 when Port=4 , only LSB bit is relevant.*/
        if (HWS_IS_PORT_MULTI_SEGMENT(curPortParams.portStandard))
        {
            rsfecStatusPtr->ampsLock = (rsfecStatusPtr->ampsLock >> 8) & 1;
        }
        else
        {
            rsfecStatusPtr->ampsLock = (rsfecStatusPtr->ampsLock >> 8) & 0xF;
        }

        /*High_ser*/
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &(rsfecStatusPtr->highSer), 0x4));
        rsfecStatusPtr->highSer = (rsfecStatusPtr->highSer >> 2)& 1;

        /*RSFEC_SYMBLERR*/
        if (curPortParams.portStandard == _400GBase_KR8 || curPortParams.portStandard == _400GBase_CR8 ||
            curPortParams.portStandard == _400GBase_SR_LR8 || curPortParams.portStandard == _424GBase_KR8)
        {
            for (i = 0; i < 16; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + i * 0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + i * 0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (curPortParams.portStandard == _200GBase_KR8 || curPortParams.portStandard == _200GBase_CR8 || curPortParams.portStandard == _200GBase_SR_LR8 )
        {
            for (i = 0; i < 8; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + i * 2 * 0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + i * 2 * 0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (curPortParams.portStandard == _200GBase_KR4 || curPortParams.portStandard == _200GBase_CR4
                 || curPortParams.portStandard == _200GBase_SR_LR4)
        {
            for (i=0; i<8; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + (portNum % 8)*0x8*2 + i*0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + (portNum % 8)*0x8*2 + i*0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (HWS_100G_R4_MODE_CHECK(curPortParams.portStandard))
        {
            for (i=0; i<4; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + (portNum % 8)*0x8*2 + i*0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + (portNum % 8)*0x8*2 + i*0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (curPortParams.portStandard ==  _100GBase_KR2 || curPortParams.portStandard ==  _100GBase_CR2 || curPortParams.portStandard ==  _100GBase_SR_LR2)
        {
            for (i=0; i<4; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + (portNum % 8)*0x8*2 + i*0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + (portNum % 8)*0x8*2 + i*0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            for (i=0; i<2; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + (portNum % 8)*0x8*2 + i*0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + (portNum % 8)*0x8*2 + i*0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (curPortParams.portStandard == _50GBase_KR || curPortParams.portStandard == _50GBase_CR || curPortParams.portStandard == _50GBase_SR_LR)
        {
            for (i=0; i<2; i++)
            {
                regAddr = baseAddr + MTI_RSFEC_SYMBLERR0_LO + (portNum % 8)*0x8*2 + i*0x8;
                regAddr1 = baseAddr + MTI_RSFEC_SYMBLERR0_HI + (portNum % 8)*0x8*2 + i*0x8;

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[i]), 0));
                rsfecStatusPtr->symbolError[i] = ((rsfecStatusPtr->symbolError[i]) << 16) + regData;
            }
        }
        else if (HWS_25G_MODE_CHECK(curPortParams.portStandard))
        {

            regAddr= baseAddr + MTI_RSFEC_SYMBLERR0_LO + (portNum % 8)*0x8*2;
            regAddr1= baseAddr + MTI_RSFEC_SYMBLERR0_HI + (portNum % 8)*0x8*2;

            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &(rsfecStatusPtr->symbolError[0]), 0));
            rsfecStatusPtr->symbolError[0] = ((rsfecStatusPtr->symbolError[0]) << 16) + regData;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsFcFecCounterGet function
* @endinternal
*
* @brief   Return FC-FEC counters.
*
* @note   APPLICABLE DEVICES:         AC5P; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portGroup                - port group (core) number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - non FC FEC
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS mvHwsFcFecCounterGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     portGroup,
    IN  GT_U32                     portNum,
    OUT MV_HWS_FCFEC_COUNTERS_STC  *fcfecCountersPtr
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_U32                    regData;
    GT_UREG_DATA              regAddr;
    GT_U32                    fieldName;
    GT_U32                    offset, laneIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg = {0, 0, 0, 0};
    GT_U32                    nextPortShift = 1;
    MV_HWS_HAWK_CONVERT_STC   convertIdx;
    GT_STATUS                 rc = GT_OK;

    if(fcfecCountersPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("fcfecCountersPtr is null"));
    }

    hwsOsMemSetFuncPtr(fcfecCountersPtr, 0, sizeof(MV_HWS_FCFEC_COUNTERS_STC));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, NON_SUP_MODE, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("hwsPortModeParamsGetToBuffer failed"));
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, curPortParams.portStandard, &convertIdx);
    if (rc != GT_OK)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsGlobalMacToLocalIndexConvert failed"));
    }

    if (curPortParams.portFecMode != FC_FEC)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("fec is not FC_FEC"));
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Harrier || HWS_DEV_SILICON_TYPE(devNum) == AC5P)
    {
        GT_U32 hawkLocalIndexArr[8]    = {0,1,2,6,10,14,18,22};

        for (laneIdx = 0; laneIdx < curPortParams.numOfActLanes; laneIdx++)
        {
            /* each physical lane works with 2 virtual lanes at most.
               10G/25G/40G-R4 – always work with single virtual lane, so only VL0 is relevant.
               For 40G-R4 since we have 4 virtual lanes, need to read them from VL0 of Port, Port+1, Port+2, Port+3.
               40G-R2/50G-R2 – each physical lane works with 2 virtual lanes.
               To read all 4, need to read VL0 and VL1 of Port, Port+1. */

            if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
            {
                nextPortShift = hawkLocalIndexArr[convertIdx.ciderIndexInUnit + laneIdx] - hawkLocalIndexArr[convertIdx.ciderIndexInUnit];
            }
            else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
            {
                nextPortShift = 2*laneIdx;
            }
            else /* AC5x */
            {
                nextPortShift = laneIdx;
            }

            fieldName = RSFEC_UNIT_RSFEC_CCW_LO_P0_RSFEC_CCW_LO_E;
            regAddr = genUnitFindAddressPrv(devNum, portNum + nextPortShift, MTI_RSFEC_UNIT, curPortParams.portStandard, fieldName, &fieldReg);
            if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
            {
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
            }

            /* VL0 FC FEC offset */
            offset = 0x304;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + offset, &regData, 0));
            fcfecCountersPtr->blocksCorrectedError[laneIdx] = regData;
            fcfecCountersPtr->numReceivedBlocksCorrectedError += regData;

            if (curPortParams.numOfActLanes == 2)
            {
                /* VL1 FC FEC offset */
                offset = 0x30C;
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + offset, &regData, 0));
                fcfecCountersPtr->blocksCorrectedError[laneIdx] += regData;
                fcfecCountersPtr->numReceivedBlocksCorrectedError += regData;
            }

            fieldName = RSFEC_UNIT_RSFEC_NCCW_LO_P0_RSFEC_NCCW_LO_E;
            regAddr = genUnitFindAddressPrv(devNum, portNum + nextPortShift, MTI_RSFEC_UNIT, curPortParams.portStandard, fieldName, &fieldReg);
            if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
            {
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
            }

            /* VL0 FC FEC offset */
            offset = 0x304;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + offset, &regData, 0));
            fcfecCountersPtr->blocksUncorrectedError[laneIdx] = regData;
            fcfecCountersPtr->numReceivedBlocksUncorrectedError += regData;


            if (curPortParams.numOfActLanes == 2)
            {
                /* VL1 FC FEC offset */
                offset = 0x30C;
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + offset, &regData, 0));
                fcfecCountersPtr->blocksUncorrectedError[laneIdx] += regData;
                fcfecCountersPtr->numReceivedBlocksUncorrectedError += regData;
            }
        }

    }

    return GT_OK;
}

/**
* @internal mvHwsRsFecCorrectedError function
* @endinternal
*
* @brief   Return RS-FEC counters.
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2; Pipe; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portGroup                - port group (core) number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS mvHwsRsFecCorrectedError
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       portNum,
    OUT MV_HWS_RSFEC_COUNTERS_STC   *rsfecCountersPtr
)
{
    GT_U32                      offset,data;
    GT_U32                      i;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_PORT_FEC_MODE        fecMode;
    GT_U32                      baseAddr, unitIndex, unitNum, regAddr,regData,localPortIndex;
    MV_HWS_UNITS_ID             unitId;
    MV_HWS_RSFEC_UNIT_FIELDS_E  fieldName1, fieldName2;
    GT_U32 timeOut = 0;
    MV_HWS_HAWK_CONVERT_STC     ciderIndex;
    GT_STATUS                   rc = GT_OK;

    if (NULL == rsfecCountersPtr)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("rsfecCountersPtr is null"));
    }

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, NON_SUP_MODE, &curPortParams));
    /* Currently on AP ports the curPortParams.portFecMode recieved from hwsPortModeParamsGetToBuffer is not always valid
       if this function is called from host CPU, therefore we need to read the fecMode directly from the hardware
       TODO: Delete this check and replace is to curPortParams.portFecMode after fixing hwsPortModeParamsGetToBuffer to return a valid fecMode */
    CHECK_STATUS(mvHwsPortFecCofigGet(devNum, portGroup, portNum, curPortParams.portStandard, &fecMode));

    if ((fecMode != RS_FEC) && (fecMode != RS_FEC_544_514))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("FEC type not supported"));
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2
        || HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        if ((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2 && portNum >= 24) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe && portNum < 12)) {
            /*Aldrin2 RS FEC support only 0-23 ports . Pipe Rs FEC support only 12-15 ports*/
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal port num for RS-FEC"));
        }

        if ((portNum % 4 == 0))
            offset = 0;
        else if ((portNum % 4) == 1)
            offset = 0x100;
        else if ((portNum % 4) == 2)
            offset = 0x120;
        else
            offset = 0x140;

        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_CCW_LO + offset, &data, 0));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_CCW_HI + offset, &(rsfecCountersPtr->correctedFecCodeword.l[0]), 0));
        rsfecCountersPtr->correctedFecCodeword.l[0] = ((rsfecCountersPtr->correctedFecCodeword.l[0]) << 16) + data;

        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_NCCW_LO + offset, &data, 0));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_NCCW_HI + offset, &(rsfecCountersPtr->uncorrectedFecCodeword.l[0]), 0));
        rsfecCountersPtr->uncorrectedFecCodeword.l[0] = ((rsfecCountersPtr->uncorrectedFecCodeword.l[0]) << 16) + data;

        if (HWS_100G_R4_MODE_CHECK(curPortParams.portStandard))
        {

            for (i=0; i<4; i++)
            {
                CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR0_LO + i*0x8, &data, 0));
                CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR0_HI + i*0x8, &(rsfecCountersPtr->symbolError[i]), 0));
                rsfecCountersPtr->symbolError[i] = ((rsfecCountersPtr->symbolError[i]) << 16) + data;
            }

        }
        else if (HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            if (portNum % 4 == 0)
            {
                for (i=0; i<2; i++)
                {
                    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR0_LO + i*0x8, &data, 0));
                    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR0_HI + i*0x8, &(rsfecCountersPtr->symbolError[i]), 0));
                    rsfecCountersPtr->symbolError[i] = ((rsfecCountersPtr->symbolError[i]) << 16) + data;
                }
            }
            else
            {
                for (i=0; i<2; i++)
                {
                    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR2_LO + i*0x8, &data, 0));
                    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR2_HI + i*0x8, &(rsfecCountersPtr->symbolError[i]), 0));
                    rsfecCountersPtr->symbolError[i] = ((rsfecCountersPtr->symbolError[i]) << 16) + data;
                }

            }
        }
        else if(HWS_25G_MODE_CHECK(curPortParams.portStandard))
        {
            if ((portNum % 4 == 0))
                offset = 0;
            else if ((portNum % 4) == 1)
                offset = 0x8;
            else if ((portNum % 4) == 2)
                offset = 0x10;
            else
                offset = 0x18;

                CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR0_LO + offset, &data, 0));
                CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_SYMBLERR0_HI + offset, &(rsfecCountersPtr->symbolError[0]), 0));
                rsfecCountersPtr->symbolError[0] = ((rsfecCountersPtr->symbolError[0]) << 16) + data;
        }
    }
    else if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        rsfecCountersPtr->symbolError[0] = 0;
        rsfecCountersPtr->symbolError[1] = 0;
        rsfecCountersPtr->symbolError[2] = 0;
        rsfecCountersPtr->symbolError[3] = 0;
#if !defined (RAVEN_DEV_SUPPORT)
        localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
        localPortIndex = portNum %8;
#endif
        /* RSFEC corrected erros*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_RSFEC_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum));

        regAddr = baseAddr + MTI_RSFEC_CCW_LO + localPortIndex * 0x20;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));

        regAddr = baseAddr + MTI_RSFEC_CCW_HI + localPortIndex * 0x20;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &(rsfecCountersPtr->correctedFecCodeword.l[0]), 0));

        rsfecCountersPtr->correctedFecCodeword.l[0] = ((rsfecCountersPtr->correctedFecCodeword.l[0]) << 16) + regData;

        /* RSFEC uncorrected erros */
        regAddr = baseAddr + MTI_RSFEC_NCCW_LO + localPortIndex * 0x20;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));

        regAddr = baseAddr + MTI_RSFEC_NCCW_HI + localPortIndex * 0x20;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &(rsfecCountersPtr->uncorrectedFecCodeword.l[0]), 0));

        rsfecCountersPtr->uncorrectedFecCodeword.l[0] = ((rsfecCountersPtr->uncorrectedFecCodeword.l[0]) << 16) + regData;
    }
    else if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum) && hwsDeviceSpecInfo[devNum].devType != AC5X)
    {
        rsfecCountersPtr->symbolError[0] = 0;
        rsfecCountersPtr->symbolError[1] = 0;
        rsfecCountersPtr->symbolError[2] = 0;
        rsfecCountersPtr->symbolError[3] = 0;

        if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
        {
            return GT_OK;
        }
        else if (mvHwsUsxModeCheck(devNum, portNum, curPortParams.portStandard) == GT_TRUE)
        {
            unitId = MTI_USX_RSFEC_STATISTICS_UNIT;
        }
        else
        {
            unitId = MTI_RSFEC_STATISTICS_UNIT;
        }
        /* set clesr on read */
        fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONFIG_CLEAR_ON_READ;
        CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, 0x1, NULL));

        /* CMD CAPTURE and CMD CHANMASK has to be written in same write, if seperately written CHANMASK self clears before capture is written*/
        fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL;
        /* Read Index in unit for the specified mac number */
        rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, curPortParams.portStandard, &ciderIndex);
        if(GT_OK != rc)
            return rc;

        regData = (1<< ciderIndex.ciderIndexInUnit);
        regData |= (1 << 28); /* Enable CMD_CAPTURE */
        CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, regData, NULL));

        /* Wait just to be sure that CMD CAPTURE bit is cleared */
        do{
            fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL_CMD_CAPTURE;
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &regData, NULL));
            timeOut++;
        } while((regData != 0) && (timeOut < 10)); /* Clears in first cycle, 10 is a random wait. */
        if(timeOut == 10)
        {
            return GT_TIMEOUT;
        }

        fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CORRECTED;
        fieldName2 = RSFEC_STATISTICS_UNITS_RSFEC_SLOW_DATA_HI_LATCHED_COUNTER_HI_SLOW;

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &regData, NULL));
        rsfecCountersPtr->correctedFecCodeword.l[0] = regData;
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName2, &regData, NULL));
        rsfecCountersPtr->correctedFecCodeword.l[1] = regData;

        fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CORRECT;
        fieldName2 = RSFEC_STATISTICS_UNITS_RSFEC_SLOW_DATA_HI_LATCHED_COUNTER_HI_SLOW;

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &regData, NULL));
        rsfecCountersPtr->correctFecCodeword.l[0] = regData;
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName2, &regData, NULL));
        rsfecCountersPtr->correctFecCodeword.l[1] = regData;

        fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_UNCORRECTED;
        fieldName2 = RSFEC_STATISTICS_UNITS_RSFEC_SLOW_DATA_HI_LATCHED_COUNTER_HI_SLOW;

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &regData, NULL));
        rsfecCountersPtr->uncorrectedFecCodeword.l[0] = regData;
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName2, &regData, NULL));
        rsfecCountersPtr->uncorrectedFecCodeword.l[1] = regData;

        fieldName1 = RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_TOTAL_CW_RECEIVED;
        fieldName2 = RSFEC_STATISTICS_UNITS_RSFEC_SLOW_DATA_HI_LATCHED_COUNTER_HI_SLOW;

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &regData, NULL));
        rsfecCountersPtr->total_cw_received.l[0] = regData;
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName2, &regData, NULL));
        rsfecCountersPtr->total_cw_received.l[1] = regData;


    }
    else if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        MV_HWS_RSFEC_UNIT_FIELDS_E  fieldName3, fieldName4;
        /* this will be delete when ac5x will have support with RS Statistic*/
        rsfecCountersPtr->symbolError[0] = 0;
        rsfecCountersPtr->symbolError[1] = 0;
        rsfecCountersPtr->symbolError[2] = 0;
        rsfecCountersPtr->symbolError[3] = 0;

        if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
        {
            fieldName1 = CPU_PCS_RSFEC_UNITS_RSFEC_CCW_LO_RSFEC_CCW_LO_E;
            fieldName2 = CPU_PCS_RSFEC_UNITS_RSFEC_CCW_HI_RSFEC_CCW_HI_E;
            fieldName3 = CPU_PCS_RSFEC_UNITS_RSFEC_NCCW_LO_RSFEC_NCCW_LO_E;
            fieldName4 = CPU_PCS_RSFEC_UNITS_RSFEC_NCCW_HI_RSFEC_NCCW_HI_E;
            unitId = MTI_CPU_RSFEC_UNIT;
        }
        else if (mvHwsUsxModeCheck(devNum, portNum, curPortParams.portStandard) == GT_TRUE)
        {
            fieldName1 = USX_RSFEC_UNITS_RSFEC_CCW_LO_RSFEC_CCW_LO_E;
            fieldName2 = USX_RSFEC_UNITS_RSFEC_CCW_HI_RSFEC_CCW_HI_E;
            fieldName3 = USX_RSFEC_UNITS_RSFEC_NCCW_LO_RSFEC_NCCW_LO_E;
            fieldName4 = USX_RSFEC_UNITS_RSFEC_NCCW_HI_RSFEC_NCCW_HI_E;
            unitId = MTI_USX_RSFEC_UNIT;
        }
        else
        {
            if(hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                fieldName1 = PHOENIX_RSFEC_UNIT_RSFEC_CCW_LO_P0_RSFEC_CCW_LO_E;
                fieldName2 = PHOENIX_RSFEC_UNIT_RSFEC_CCW_HI_P0_RSFEC_CCW_HI_E;
                fieldName3 = PHOENIX_RSFEC_UNIT_RSFEC_NCCW_LO_P0_RSFEC_NCCW_LO_E;
                fieldName4 = PHOENIX_RSFEC_UNIT_RSFEC_NCCW_HI_P0_RSFEC_NCCW_HI_E;
            }
            else
            {
                fieldName1 = RSFEC_UNIT_RSFEC_CCW_LO_P0_RSFEC_CCW_LO_E;
                fieldName2 = RSFEC_UNIT_RSFEC_CCW_HI_P0_RSFEC_CCW_HI_E;
                fieldName3 = RSFEC_UNIT_RSFEC_NCCW_LO_P0_RSFEC_NCCW_LO_E;
                fieldName4 = RSFEC_UNIT_RSFEC_NCCW_HI_P0_RSFEC_NCCW_HI_E;
            }
            unitId = MTI_RSFEC_UNIT;
        }

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName1, &regData, NULL));
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName2, &(rsfecCountersPtr->correctedFecCodeword.l[0]), NULL));
        rsfecCountersPtr->correctedFecCodeword.l[0] = ((rsfecCountersPtr->correctedFecCodeword.l[0]) << 16) + regData;

        /* RSFEC uncorrected erros */
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName3, &regData, NULL));
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, unitId, curPortParams.portStandard, fieldName4, &(rsfecCountersPtr->uncorrectedFecCodeword.l[0]), NULL));
        rsfecCountersPtr->uncorrectedFecCodeword.l[0] = ((rsfecCountersPtr->uncorrectedFecCodeword.l[0]) << 16) + regData;
    }
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("silicon type not supported"));
    }
    return GT_OK;
}
#endif /* BOBK_DEV_SUPPORT */

#endif /* #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */

#if ((defined CHX_FAMILY) || defined(RAVEN_DEV_SUPPORT) || defined (FALCON_DEV_SUPPORT))

/**
* @internal mvHwsMtipExtFecTypeGet function
* @endinternal
*
* @brief  Get port fec type
*
* @param[in] devNum          - system device number
* @param[in] portGroup       - port group (core) number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] fecTypePtr      - fec type
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtFecTypeGet
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_FEC_MODE        *fecTypePtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum, i;
    GT_U32 regAddr, regData, regMask, localPortIndex;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_PORT_FEC_MODE        fecType;
    regData = 0;

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_CONTROL;
        regMask = 0x2;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
        *fecTypePtr = ( regData )? FC_FEC: FEC_OFF;
        return GT_OK;
    }
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_FEC_CONTROL;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
#ifndef RAVEN_DEV_SUPPORT
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif

    regMask = 0;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    *fecTypePtr = FEC_OFF;
    fecType = FEC_OFF;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch ((regData>>(localPortIndex+i)) & 0x1010001){
            case 1: /*fc_fec */
                if ((fecType !=FEC_OFF) && (fecType !=FC_FEC)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                fecType = FC_FEC;
                break;
            case 0x0010000: /*rs_fec_91*/
                if ((fecType !=FEC_OFF) && (fecType !=RS_FEC)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                fecType = RS_FEC;
                break;
            case 0x1010000: /* rs_fec_kp */
                if ((fecType !=FEC_OFF) && (fecType !=RS_FEC_544_514)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                fecType = RS_FEC_544_514;
                break;
            default: /* no fec*/
                if (fecType !=FEC_OFF) {
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                break;
        }
    }
    *fecTypePtr = fecType;

    return GT_OK;
}
/**
* @internal mvHwsMtipExtFecTypeSet function
* @endinternal
*
* @brief  Set port fec type
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] fecType         - port fec type
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtFecTypeSet
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_FEC_MODE        fecType
)
{
    GT_U32 baseAddr, unitIndex, unitNum, i;
    GT_U32 fecRegAddr, fecRegData, regMask, localPortIndex;
    GT_U32 channelRegAddr, channelRegData;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

#if !defined (RAVEN_DEV_SUPPORT) && !defined(FALCON_DEV_SUPPORT)
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtFecTypeSet ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        fecRegAddr = baseAddr + MTIP_CPU_EXT_PORT_CONTROL;
        switch (fecType) {
            case FC_FEC:
                fecRegData =  0x2;
                break;
            case FEC_OFF:
            case FEC_NA:
                fecRegData =  0;
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeSet hws bad width param"));
                break;
        }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, fecRegAddr, fecRegData, 0x2));
#if !defined (RAVEN_DEV_SUPPORT) && !defined(FALCON_DEV_SUPPORT)
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of   mvHwsMtipExtFecTypeSet ******\n");
        }
#endif
        return GT_OK;
    }
#if !defined (RAVEN_DEV_SUPPORT) && !defined(FALCON_DEV_SUPPORT)
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif

    fecRegData = 0;
    channelRegData = 0;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    fecRegAddr = baseAddr + MTIP_EXT_GLOBAL_FEC_CONTROL;
    channelRegAddr  = baseAddr + MTIP_EXT_GLOBAL_CHANNEL_CONTROL;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    regMask = 0;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (fecType) {
            case RS_FEC_544_514:
                if ((!HWS_PAM4_MODE_CHECK(portMode)) && !((portMode == _200GBase_KR8)  || (portMode == _200GBase_CR8 ) || (portMode == _200GBase_SR_LR8 )
                                                                                       || (portMode == _106GBase_KR4) || (portMode == _53GBase_KR2))){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                /* set gc_fec91_ena_in[23,16] and gc_kp_mode_in[31,24]*/
                fecRegData |=  (0x1010000 << (localPortIndex + i));
                break;
            case RS_FEC:
                if ((!HWS_25G_MODE_CHECK(portMode)) && (!HWS_TWO_LANES_MODE_CHECK(portMode)) &&
                   (!HWS_100G_R4_MODE_CHECK(portMode))){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                fecRegData |=  (0x0010000<<(localPortIndex+i));
                break;
            case FC_FEC:
                if ((portMode != _5GBaseR ) && (!HWS_10G_MODE_CHECK(portMode)) &&
                    (!HWS_25G_MODE_CHECK(portMode)) &&(!HWS_40G_R4_MODE_CHECK(portMode)) &&
                    (!HWS_TWO_LANES_MODE_CHECK(portMode))){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                fecRegData |=  (0x0000001<<(localPortIndex+i));
                break;
            case FEC_OFF:
            case FEC_NA:
                fecRegData =  0;
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeSet hws bad width param"));
                break;
        }
        regMask |= (0x0000001<<(localPortIndex+i)) | (0x1010000 << (localPortIndex + i));
     }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, fecRegAddr, fecRegData, regMask));

    if ((localPortIndex % 2) == 0) {
        /* we need to udpate Rs FEC Enable in global channel control register */
        regMask = 0x4400<<(localPortIndex/2);
        /* we need to udpate Rs FEC Enable in global channel control register */
        if (HWS_TWO_LANES_MODE_CHECK(portMode))
        {
            switch (fecType) {
                case RS_FEC:
                    channelRegData = 0;
                    break;
                case FEC_NA: /* when disabling port - return to default*/
                    channelRegData = 0x400<<(localPortIndex/2);
                    break;
                default:
                    channelRegData = 0x4000<<(localPortIndex/2);
                    break;
            }
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, channelRegAddr, channelRegData, regMask));
        }
    }
    if(HWS_100G_R4_MODE_CHECK(portMode) && (fecType == RS_FEC))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_RSFEC_UNIT, curPortParams.portPcsNumber, &baseAddr, &unitIndex, &unitNum ));
        fecRegAddr = baseAddr + MTI_RSFEC_DEC_THRESH + localPortIndex * 0x20;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, fecRegAddr, 0x0, 0x3F));
    }
#if !defined (RAVEN_DEV_SUPPORT) && !defined (FALCON_DEV_SUPPORT)
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtFecTypeSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtipExtFecClockEnable function
* @endinternal
*
* @brief  EXT unit fec clock enable + EXT fec release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] fecType         - port fec type
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtFecClockEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_FEC_MODE        fecType,
    IN GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

#if !defined (RAVEN_DEV_SUPPORT) && !defined (FALCON_DEV_SUPPORT)
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtFecClockEnable ******\n");
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtFecClockEnable ******\n");
        }
        return GT_OK;
    }
#endif
    portMode = portMode;
    if ((fecType != RS_FEC) && (fecType != RS_FEC_544_514))
    {
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;

    regMask = (1<<11); /*bit 11*/
    regData = (enable == GT_TRUE) ? regMask : 0;

    if (enable == GT_FALSE)
    {
        /* TODO - need to support power reduction*/
        return GT_OK;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* release from reset */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;

    regMask = (1<<29); /*bit 29*/
    regData = (enable == GT_TRUE) ? regMask : 0;

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));


#if !defined (RAVEN_DEV_SUPPORT) && !defined (FALCON_DEV_SUPPORT)
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtFecClockEnable ******\n");
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsMarkerIntervalCalc function
* @endinternal
*
* @brief  MAC/PCS marker interval calc function
*
*
* @param[in] devNum          - system device number
* @param[in] portMode        - port mode
* @param[in] portFecMode     - port fec type
* @param[out] markerIntervalPtr - (pointer to) marker interval value
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMarkerIntervalCalc
(
    GT_U8                   devNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    portFecMode,
    GT_U32                  *markerIntervalPtr
)
{
    devNum = devNum;

    switch(portMode)
    {
        case HWS_SX_MODE_CASE:
        case HWS_10G_MODE_CASE:
        case _1000Base_X:
        case _2500Base_X:
        case SGMII:
        case SGMII2_5:
        case _5GBaseR:
        case _10G_OUSGMII:
        case _5G_QUSGMII:
        case QSGMII:
            *markerIntervalPtr = 0;
            break;

        case HWS_D_USX_MODE_CASE:
        case HWS_Q_USX_MODE_CASE:
            if(portFecMode == RS_FEC)
            {
                *markerIntervalPtr = 0x3e7c;
            }
            else
            {
                *markerIntervalPtr = 0x4010;
            }
            break;

        case HWS_50G_PAM4_MODE_CASE:
        case HWS_200G_PAM4_MODE_CASE:
        case HWS_400G_PAM4_MODE_CASE:
        case HWS_200G_R8_MODE_CASE:
            *markerIntervalPtr = 0x4FFF;
            break;

        case HWS_40G_R4_MODE_CASE:
        case HWS_100G_R4_MODE_CASE:
        case HWS_100G_PAM4_MODE_CASE:
            *markerIntervalPtr = 0x3FFF;
            break;

        case HWS_25G_MODE_CASE:
            if(portFecMode == RS_FEC)
            {
                *markerIntervalPtr = 0x4FFF;
            }
            else
            {
                *markerIntervalPtr = 0;
            }
            break;

        case HWS_40G_R2_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
            if(portFecMode == RS_FEC)
            {
                *markerIntervalPtr = 0x4FFF;
            }
            else
            {
                *markerIntervalPtr = 0x3FFF;
            }
            break;

            break;

        default:
            *markerIntervalPtr = 0xFFFFFFFF;
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtipExtMacClockEnableGet function
* @endinternal
*
* @brief  EXT unit clock enable Get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtMacClockEnableGet
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT GT_BOOL                    *enablePtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask, channel;


    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /*
            mtip_cpu_ext_units_RegFile.Port_Clocks.app_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.mac_tx_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.mac_rx_clk_en.set(1'b1);
        */
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_CLOCKS;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0x2));
        *enablePtr = (regData) ? GT_TRUE : GT_FALSE;
        return GT_OK;
    }

#if !defined (RAVEN_DEV_SUPPORT)
    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    channel = portNum %8;
#endif

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if(portNum % 8 == 0)
        {
            regMask = 0x100; /* 400G_R8, 200G_R8 or 200G_R4 on index 0 */
        }
        else
        {
            regMask = 0x200; /* 200G_R4 on index 4 */
        }
    }
    else
    {
        regMask = 1 << channel;

     }

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    *enablePtr = (regData) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMtipExtMacClockEnable function
* @endinternal
*
* @brief  EXT unit clock enable + EXT MAC release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtMacClockEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask, channel;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtMacClockEnable ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /*
            mtip_cpu_ext_units_RegFile.Port_Clocks.app_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.mac_tx_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.mac_rx_clk_en.set(1'b1);
        */
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_CLOCKS;
        regData = (enable == GT_TRUE) ? 0x7 : 0;

        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, 0x7));
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtMacClockEnable ******\n");
        }
#endif

        return GT_OK;
    }

#if !defined (RAVEN_DEV_SUPPORT)
    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    channel = portNum %8;
#endif

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if(portNum % 8 == 0)
        {
            regMask = 0x100; /* 400G_R8, 200G_R8 or 200G_R4 on index 0 */
        }
        else
        {
            regMask = 0x200; /* 200G_R4 on index 4 */
        }
    }
    else
    {
        regMask = 1 << channel;

        /* Port power reduction (part1):
             Low speed PCS MAC clock should be enabled if at least 1 port from 8
             is configured.
             Disable configuration should be done only when power down of last
             low speed port is performed - this part still TBD!!!! */
        if(((portMode == _1000Base_X) || (portMode == _2500Base_X) || (portMode == SGMII))&& (enable == GT_TRUE))
        {
            regMask |= (1 << 15); /*Enable Low rates PCS mac clk*/
        }
    }
    regData = (enable == GT_TRUE) ? regMask : 0;

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtMacClockEnable ******\n");
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsMtipExtPcsClockEnable function
* @endinternal
*
* @brief  EXT unit xpcs clock enable + EXT xpcs release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtPcsClockEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask,localPortIndex,quadPort;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtPcsClockEnable ******\n");
    }
#endif
    portMode = portMode;
#if !defined (RAVEN_DEV_SUPPORT)
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif
    quadPort = localPortIndex / 4;

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /*
            mtip_cpu_ext_units_RegFile.Port_Clocks.sd_pcs_tx_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.sd_pcs_rx_clk_en.set(1'b1);

        */
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_CLOCKS;
        regData = (enable == GT_TRUE) ? 0x18 : 0;

        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, 0x18));
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtPcsClockEnable ******\n");
        }
#endif

        return GT_OK;
    }

    if (enable == GT_FALSE)
    {
        /* TODO - need to support power reduction*/
        hwsOsPrintf("TODO - mvHwsMtipExtPcsClockEnable: need to add support for power reduction*\n");
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        regMask = (1<<(14)); /*bit 14*/
    }
    else
    {
        regMask = (1<<(12 + quadPort)); /*bit 12,13*/
    }
    regData = (enable == GT_TRUE) ? regMask : 0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* release from reset */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        regMask = (1<<(26)); /*bit 26*/
    }
    else
    {
        regMask = (1<<(27 + quadPort)); /*bit [28:27]*/
    }
    regData = (enable == GT_TRUE) ? regMask : 0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtPcsClockEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtipExtMacResetRelease function
* @endinternal
*
* @brief  EXT unit MAC release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtMacResetRelease
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regOffset, channel;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtMacResetRelease ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        /*
            mtip_cpu_ext_units_RegFile.Port_Reset.mac_reset_.set(Value);
            mtip_cpu_ext_units_RegFile.Port_Reset.app_reset_.set(Value);
        */
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_RESET;
        regData = (enable == GT_TRUE) ? 0xC : 0;

        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, 0xC));
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtMacResetRelease ******\n");
        }
#endif

        return GT_OK;
    }

#if !defined (RAVEN_DEV_SUPPORT)
    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    channel = portNum %8;
#endif
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if(portNum % 8 == 0)
        {
            regOffset = 24;     /* 400G_R8, 200G_R8 or 200G_R4 on index 0 */
        }
        else
        {
            regOffset = 25;     /* 200G_R4 on index 4 */
        }
    }
    else
    {
        regOffset = 16 + channel;
    }

    regData = (enable == GT_TRUE) ? 1 : 0;

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, regOffset, 1, regData));

    /* Port power reduction (part1):
         Low speed reset should be enabled if at least 1 port from 8
         is configured.
         Disable configuration should be done only when power down of last
         low speed port is performed - this part still TBD!!!! */
    if(((portMode == _1000Base_X)||(portMode == _2500Base_X)||(portMode == SGMII))&& (enable == GT_TRUE))
    {
        /* Unreset sg mac clk (low rates) */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 1, 1));
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtMacResetRelease ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtipExtMacResetReleaseGet function
* @endinternal
*
* @brief  EXT unit MAC release get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtMacResetReleaseGet
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT GT_BOOL                    *enablePtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regOffset, channel;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtMacResetRelease ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        /*
            mtip_cpu_ext_units_RegFile.Port_Reset.mac_reset_.set(Value);
            mtip_cpu_ext_units_RegFile.Port_Reset.app_reset_.set(Value);
        */
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_RESET;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xC));

        *enablePtr = (regData) ? GT_TRUE : GT_FALSE;
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtMacResetReleaseGet ******\n");
        }
#endif

        return GT_OK;
    }

#if !defined (RAVEN_DEV_SUPPORT)
    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    channel = portNum %8;
#endif
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if(portNum % 8 == 0)
        {
            regOffset = 24;     /* 400G_R8, 200G_R8 or 200G_R4 on index 0 */
        }
        else
        {
            regOffset = 25;     /* 200G_R4 on index 4 */
        }
    }
    else
    {
        regOffset = 16 + channel;
    }


    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, (1<<regOffset)));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtMacResetReleaseGet ******\n");
    }
#endif
    *enablePtr = (regData) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMtipExtMacAccessCheck function
* @endinternal
*
* @brief  EXT unit clock enable and reset release Get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtMacAccessCheck
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT GT_BOOL                    *enablePtr
)
{
    GT_STATUS rc;
    GT_BOOL enable;
    *enablePtr = GT_FALSE;
    rc = mvHwsMtipExtMacClockEnableGet(devNum, portNum, portMode, &enable);
    if (( rc == GT_OK ) && (enable == GT_TRUE))
    {
        rc = mvHwsMtipExtMacResetReleaseGet(devNum, portNum, portMode, &enable);
        if (( rc == GT_OK ) && (enable == GT_TRUE))
        {
            *enablePtr = GT_TRUE;
        }
    }
    return GT_OK;
}
#endif /*if ((!defined MV_HWS_REDUCED_BUILD) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT))*/

