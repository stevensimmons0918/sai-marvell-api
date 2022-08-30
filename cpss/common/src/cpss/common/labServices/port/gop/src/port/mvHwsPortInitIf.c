/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
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
* @file mvHwsPortInitIf.c
*
* @brief
*
* @version   55
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#endif
#ifdef MV_HWS_REDUCED_BUILD
GT_BOOL  multiLaneConfig = GT_TRUE;
#endif
#ifdef RAVEN_DEV_SUPPORT
extern void mvPortCtrlLogAdd(GT_U32 entry);
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApLogDefs.h>
#endif

extern  GT_BOOL hwsPpHwTraceFlag;

#ifdef MV_HWS_REDUCED_BUILD
#define PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(_var) _var
#else
#define PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(_var) \
                PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.portSrc._var)
#endif
/* List of all SerDes speeds that supports Tx/Rx parameters override
   and their corresponding indexes inside the databases */
const GT_U8 sdParamsOverridePermmittedSpeedIndexesSip6[][2] =
{/*  SerDes Speed     Index*/
    {_5G                ,  0    },
    {_10_3125G          ,  0    },
    {_10_9375G          ,  0    },
    {_12_1875G          ,  0    },
    {_10_3125G_SR_LR    ,  0    },
    {_20_625G           ,  1    },
    {_25_78125G         ,  1    },
    {_25_78125G_SR_LR   ,  1    },
    {_27_34375G         ,  1    },
    {_26_5625G_PAM4     ,  2    },
    {_27_1875_PAM4      ,  2    },
    {_26_5625G_PAM4_SR_LR, 2    },
    {_28_125G_PAM4      ,  2    },
    {LAST_MV_HWS_SERDES_SPEED, UNPERMITTED_SD_SPEED_INDEX} /* last - DO NOT DELETE */
};
const GT_U8 sdParamsOverridePermmittedSpeedIndexesSip5[][2] =
{/*  SerDes Speed     Index*/
    {_5G                ,  0    },
    {_10_3125G          ,  0    },
    {_10_9375G          ,  0    },
    {_12_1875G          ,  0    },
#ifndef ALDRIN_DEV_SUPPORT
    {_10_3125G_SR_LR    ,  0    },
    {_20_625G           ,  1    },
    {_25_78125G         ,  1    },
    {_25_78125G_SR_LR   ,  1    },
    {_27_34375G         ,  1    },
#endif
    {_1_25G             ,  2    },
    {LAST_MV_HWS_SERDES_SPEED, UNPERMITTED_SD_SPEED_INDEX} /* last - DO NOT DELETE */
};

#define PRV_CPSS_OPTICAL_PORT_MODE_MAC(_portMode) \
        (((_portMode == _10GBase_SR_LR) || (_portMode == _40GBase_SR_LR4) || (_portMode == _40GBase_SR_LR)) || \
         ((_portMode == _25GBase_SR) || (_portMode == _100GBase_SR4)  || (_portMode == _50GBase_SR2)) || \
         ((_portMode == _200GBase_SR_LR4) || (_portMode == _400GBase_SR_LR8) || (_portMode == _50GBase_SR_LR) || (_portMode == _100GBase_SR_LR2)))

#define NEW_PORT_INIT_SEQ
/*****************************************************************************************/
/**
* @internal mvHwsSerdesSpeedToIndex function
* @endinternal
*
* @brief  convert serdes speed to index
*
* @param[in] devNum                - system device number
* @param[in] sdSpeed               - serdes speed
* @param[out] *speedIdxPtr       - (pointer to) serdes index
*
* @retval 0                  - on success
* @retval 1                  - on error
*/

GT_U8 mvHwsSerdesSpeedToIndex
(
    IN GT_U8                    devNum,
    IN GT_U32                   sdSpeed
)
{
    GT_U8 sdSpeedIterator = 0;
#ifndef RAVEN_DEV_SUPPORT
    if (HWS_DEV_SIP_6_CHECK_MAC(devNum))
#endif
    {
        for(sdSpeedIterator = 0 ;
           (sdParamsOverridePermmittedSpeedIndexesSip6[sdSpeedIterator][0] != sdSpeed) &&
                (sdParamsOverridePermmittedSpeedIndexesSip6[sdSpeedIterator][0] != LAST_MV_HWS_SERDES_SPEED);
            sdSpeedIterator++);
        return (sdParamsOverridePermmittedSpeedIndexesSip6[sdSpeedIterator][1]);
    }
#ifndef RAVEN_DEV_SUPPORT
    else
    {
        for(sdSpeedIterator = 0 ;
           (sdParamsOverridePermmittedSpeedIndexesSip5[sdSpeedIterator][0] != sdSpeed) &&
                (sdParamsOverridePermmittedSpeedIndexesSip5[sdSpeedIterator][0] != LAST_MV_HWS_SERDES_SPEED);
            sdSpeedIterator++);
        return (sdParamsOverridePermmittedSpeedIndexesSip5[sdSpeedIterator][1]);
    }
#endif
    return (UNPERMITTED_SD_SPEED_INDEX);
}

GT_BOOL multiLaneConfigSupport(GT_BOOL enable)
{
    GT_BOOL oldState = PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(multiLaneConfig);
    PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(multiLaneConfig) = enable;
    return oldState;
}

#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port init parameters,
*                                       this function using:
*                                       lbPort - if true, init port without serdes activity
*                                       refClock - Reference clock frequency
*                                       refClock - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc;

#ifndef MV_HWS_REDUCED_BUILD
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortInitIpc(devNum,portGroup, phyPortNum,portMode, lbPort, refClock, refClockSource));
#endif
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc == NULL)
    {
        return GT_BAD_PTR;
    }
NEW_PORT_INIT_SEQ
    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc(devNum, portGroup, phyPortNum, portMode, portInitInParamPtr);
}


/**
* @internal mvHwsPortAnpStart function
* @endinternal
*
* @brief start an machine to find resolution
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfgPtr                 - port ap parameters,
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAnpStart
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_AP_CFG                  *apCfgPtr

)
{
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAnpStartFunc == NULL)
    {
        return GT_OK;
    }
    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAnpStartFunc(devNum, portGroup, phyPortNum, apCfgPtr);
}

/**
* @internal mvHwsPortAccessCheck function
* @endinternal
*
* @brief   Validate port access
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[out] accessEnabledPtr        - (pointer to) port access enabled/disabled
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAccessCheck
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *accessEnabledPtr
)
{
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if(accessEnabledPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    /* validation */
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        (portMode >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }


    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc == NULL)
    {
        return GT_BAD_PTR;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc(devNum, phyPortNum, portMode, accessEnabledPtr);
}

#endif

/**
* @internal mvHwsApPortInit function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port init parameters,
*                                       this function using:
*                                       lbPort - if true, init port without serdes activity
*                                       refClock - Reference clock frequency
*                                       refClock - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortInit
(
    IN GT_U8                          devNum,
    IN GT_U32                         portGroup,
    IN GT_U32                         phyPortNum,
    IN MV_HWS_PORT_STANDARD           portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS  *portInitInParamPtr
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

#ifndef MV_HWS_REDUCED_BUILD
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortInitIpc(devNum, portGroup, phyPortNum, portMode, lbPort, refClock, refClockSource));
#endif
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc == NULL)
    {
        return GT_BAD_PTR;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc(devNum, portGroup, phyPortNum, portMode, portInitInParamPtr);
}

/**
* @internal mvHwsPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortReset
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION  action
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortResetIpc(devNum,portGroup,phyPortNum,portMode,action));
#endif

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc == NULL)
    {
        return GT_BAD_PTR;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc(devNum, portGroup, phyPortNum, portMode, action);
}

/**
* @internal mvHwsPortFlavorInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly to specified flavor.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port init parameters,
*                                       this function using:
*                                       lbPort - if true, init port without serdes activity
*                                       refClock - Reference clock frequency
*                                       refClock - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlavorInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{

    return mvHwsPortInit(devNum,portGroup,phyPortNum,portMode, portInitInParamPtr);
}

/**
* @internal mvHwsPortValidate function
* @endinternal
*
* @brief   Validate port API's input parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortValidate
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /* validation */
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        (portMode >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    /* In MLG mode only first port number in the GOP has 4 active lanes,
        all other three ports have 0 active lane */
    if ((curPortParams.numOfActLanes == 0) &&
        (portMode != _100GBase_MLG)
#ifndef MV_HWS_REDUCED_BUILD
        && (mvHwsUsxModeCheck(devNum, phyPortNum, portMode) == GT_FALSE)
#endif
       )
    {
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsPortLoopbackValidate function
* @endinternal
*
* @brief   Validate loopback port input parameters.
*         In MMPCS mode: the ref_clk comes from ExtPLL, thus the Serdes can be in power-down.
*         In all other PCS modes: there is no ExtPLL, thus the ref_clk comes is taken
*         from Serdes, so the Serdes should be in power-up.
* @param[in] portPcsType              - port pcs type
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] serdesInit               - if true, init port serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackValidate
(
    GT_U32  portPcsType,
    GT_BOOL lbPort,
    GT_BOOL *serdesInit
)
{
    if (lbPort == GT_TRUE)
    {
        /* in MMPCS PCS mode for Loopback mode, the ref_clk comes from ExtPLL,
           so the Serdes can be in power-down */
        if ((MV_HWS_PORT_PCS_TYPE)portPcsType == MMPCS)
        {
            *serdesInit = GT_FALSE;
        }
        else /* for all other PCS modes in Loopback mode, there is no ExtPLL,
                so the Serdes should be in power-up */
        {
            *serdesInit = GT_TRUE;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/


GT_STATUS mvHwsPortSerdesPowerUp
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE     refClockSource,
    GT_U32                      *curLanesList
)
{
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_SERDES_CONFIG_STC        serdesConfig;

    GT_U32                          numOfActLanes;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    numOfActLanes = curPortParams.numOfActLanes;

    serdesConfig.baudRate       = curPortParams.serdesSpeed;
    serdesConfig.media          = curPortParams.serdesMediaType;
    serdesConfig.busWidth       = curPortParams.serdes10BitStatus;
    serdesConfig.refClock       = refClock;
    serdesConfig.refClockSource = refClockSource;
    serdesConfig.serdesType     = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]));
    serdesConfig.opticalMode    = GT_FALSE;
    serdesConfig.encoding       = SERDES_ENCODING_NA;


    if ( PRV_CPSS_OPTICAL_PORT_MODE_MAC(portMode) )
    {
        serdesConfig.opticalMode = GT_TRUE;
    }

#ifndef ALDRIN_DEV_SUPPORT
    switch (serdesConfig.baudRate)
    {
        case _26_5625G_PAM4:      /* for NRZ mode at 26.5625G,  for PAM4 mode at 53.125G */
        case _28_125G_PAM4:     /* for NRZ mode at 28.125G, for PAM4 mode at 56.25G */
        case _27_1875_PAM4:
            serdesConfig.encoding = SERDES_ENCODING_PAM4;
            break;
        default:
            serdesConfig.encoding = SERDES_ENCODING_NA;
            break;
    }
#endif

    /* power up the serdes */
    if (PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(multiLaneConfig) == GT_TRUE)
    {
        CHECK_STATUS(mvHwsSerdesArrayPowerCtrl(devNum, portGroup, numOfActLanes,
                                              curLanesList, GT_TRUE, &serdesConfig));
    }
    else
    { /* on each related serdes */
        for (i = 0; i < numOfActLanes; i++)
        {
            serdesConfig.serdesType     = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)));
            CHECK_STATUS(mvHwsSerdesPowerCtrl(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), GT_TRUE, &serdesConfig));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortApSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes for 802.3AP protocol.
*         assumes: parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesPowerUp
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE refClockSource,
    GT_U32 *curLanesList
)
{
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }


    serdesConfig.baudRate = curPortParams.serdesSpeed;
    serdesConfig.media = curPortParams.serdesMediaType;
    serdesConfig.busWidth = curPortParams.serdes10BitStatus;
    serdesConfig.refClock       = refClock;
    serdesConfig.refClockSource = refClockSource;
    serdesConfig.encoding       = SERDES_ENCODING_NA;
    /*serdesConfig.serdesType     = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));*/

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))); /* board might have different SDs like AC family */
        CHECK_STATUS(mvHwsSerdesApPowerCtrl(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF), GT_TRUE, &serdesConfig));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortModeSquelchCfg function
* @endinternal
*
* @brief   reconfigure default squelch threshold value only for KR (CR) modes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] curLanesList             - active Serdes lanes list according to configuration
* @param[in] txAmp                    - Reference clock frequency
* @param[in] emph0                    - Reference clock source line
* @param[in] emph1                    - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeSquelchCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32  *curLanesList
)
{
    GT_U32 i;
    GT_U32 squelchVal;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT      txTuneValues;   /* TX configuration parameters */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))

    {
        return GT_NOT_SUPPORTED;
    }

    hwsOsMemSetFuncPtr(&txTuneValues, 0, sizeof(txTuneValues));

    switch (portMode)
    {
        case _10GBase_KR:
        case _20GBase_KR2:
        case _40GBase_KR4:
        case _40GBase_CR4:
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                mvHwsSerdesSqlchCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), 2);
            }
            break;
        case _100GBase_KR10:
        case _100GBase_SR10:
        case _12_1GBase_KR:
        case _12_5GBase_KR:
        case XLHGL_KR4:
        case RXAUI:
        case _12GBaseR:
        case _5_625GBaseR:
        case _48GBaseR4:
        case _12GBase_SR:
        case _48GBase_SR4:
        case _5GBaseR:
            /* on each related serdes */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                mvHwsSerdesSqlchCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), 2);
#ifndef ALDRIN_DEV_SUPPORT
                /* for Lion2 */
                txTuneValues.txComphyH.txAmp     = 26;
                txTuneValues.txComphyH.txAmpAdj  = GT_TRUE;
                txTuneValues.txComphyH.txAmpShft = GT_TRUE;
                txTuneValues.txComphyH.txEmph0   = 3;
                txTuneValues.txComphyH.txEmph1   = 3;
                txTuneValues.txComphyH.slewCtrlEn   = GT_FALSE;
                txTuneValues.txComphyH.slewRate  = 0;
                mvHwsSerdesManualTxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                                          HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &txTuneValues);
#endif
            }
            break;
        case _10GBase_SR_LR:
        case _20GBase_SR_LR2:
        case _40GBase_SR_LR4:

            /* on each related serdes */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                squelchVal = (HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)) <= COM_PHY_H_REV2) ? 6 : 2;
                mvHwsSerdesSqlchCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), squelchVal);
            }
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal mvHwsPortModeCfg function
* @endinternal
*
* @brief   configures MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeCfg
(
    IN GT_U8                               devNum,
    IN GT_U32                              portGroup,
    IN GT_U32                              phyPortNum,
    IN MV_HWS_PORT_STANDARD                portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsPortModeCfg ******\n");
    }
#endif

    /* set PCS mode */
    if ((curPortParams.portPcsType != GPCS) && (curPortParams.portPcsType != GPCS_NET)
             && (curPortParams.portPcsType != QSGMII_PCS))
    {
        CHECK_STATUS(mvHwsPcsModeCfg(devNum, portGroup, phyPortNum, portMode, attributesPtr));
    }

    /* set MAC mode */
    CHECK_STATUS(mvHwsMacModeCfg(devNum, portGroup, phyPortNum, portMode, attributesPtr));

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsPortModeCfg ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortStartCfg function
* @endinternal
*
* @brief   Unreset MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStartCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    portPcsType = curPortParams.portPcsType;
#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsPortStartCfg ******\n");
    }
#endif

    if ((portPcsType != GPCS) && (portPcsType != GPCS_NET) && (portPcsType != QSGMII_PCS))
    {
#ifndef ALDRIN_DEV_SUPPORT
        if((MMPCS == portPcsType) || (CGPCS == portPcsType))
        {
            if((HWS_25G_MODE_CHECK(curPortParams.portStandard)) && (curPortParams.portFecMode == RS_FEC))
            {
                portPcsType = CGPCS;
            }
            if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard) && (curPortParams.portFecMode == FC_FEC))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
#endif

        /* pcs unreset */
        CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, curPortParams.portStandard, portPcsType, UNRESET));
    }
    /* run RF (digital reset / unreset */
    if ((portPcsType == XPCS) || (portPcsType == MMPCS && curPortParams.numOfActLanes > 1))
    {
#ifndef RAVEN_DEV_SUPPORT
        CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 1));
#endif
    }

    /* In Sip6_0 MAC UNRESET is part of mvHwsPortModeCfg() */
    if(!HWS_DEV_SIP_6_CHECK_MAC(devNum))
    {
        /* mac unreset */
        CHECK_STATUS(mvHwsMacReset(devNum, portGroup,
                        curPortParams.portMacNumber, portMode,
                        curPortParams.portMacType, UNRESET));
    }

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsPortStartCfg ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsPortStopCfg function
* @endinternal
*
* @brief   Reset MAC and PCS components
*         Port power down on each related serdes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - port power down or reset
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*                                      reset pcs        - reset pcs option
*                                      reset mac        - reset mac option
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStopCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_ACTION action,
    GT_U32 *curLanesList,
    MV_HWS_RESET reset_pcs,
    MV_HWS_RESET reset_mac
)
{
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;
    GT_STATUS               rc;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    portPcsType = curPortParams.portPcsType;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    serdesConfig.baudRate = curPortParams.serdesSpeed;
    serdesConfig.media = curPortParams.serdesMediaType;
    serdesConfig.busWidth = curPortParams.serdes10BitStatus;
    serdesConfig.refClock       = _156dot25Mhz;
    serdesConfig.refClockSource = PRIMARY_LINE_SRC;
    serdesConfig.encoding       = SERDES_ENCODING_NA;
    /*serdesConfig.serdesType     = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));*/
#ifndef RAVEN_DEV_SUPPORT
    if (HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]) == AVAGO)
    {
        /* set serdesType , that needed by mvHwsSerdesArrayHaltDfeTune(...) */
        serdesConfig.serdesType     = HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]);
        /* peroform sbus reset in case of DFE tune is still running to enure safety serdes power off */
        CHECK_STATUS(mvHwsSerdesArrayHaltDfeTune(devNum, portGroup,  &(curLanesList[0]), curPortParams.numOfActLanes, &serdesConfig));
    }
#endif
    /* PORT_POWER_DOWN on each related serdes */
    for (i = 0; (PORT_POWER_DOWN == action) && (i < curPortParams.numOfActLanes); i++)
    {
        serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)));
        rc = mvHwsSerdesPowerCtrl(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                          GT_FALSE, &serdesConfig);
        if (rc != GT_OK)
        {
        }
        CHECK_STATUS(rc);
    }

#ifndef ALDRIN_DEV_SUPPORT
    if((MMPCS == portPcsType) || (CGPCS == portPcsType))
    {
        if ((curPortParams.portFecMode == RS_FEC) && (HWS_25G_MODE_CHECK(curPortParams.portStandard)))
        {
            portPcsType = CGPCS;
        }
        if((curPortParams.portFecMode == FC_FEC) && HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
#endif

    if ((reset_pcs == RESET) || (reset_pcs == POWER_DOWN)) /* In case of RESET or FULL_RESET - perform the MAC side first */
    {
        rc = mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, reset_mac);
        if (rc != GT_OK)
        {
        }
        CHECK_STATUS(rc);

        rc = mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, portPcsType, reset_pcs);
        if (rc != GT_OK)
        {
        }
        CHECK_STATUS(rc);
    }
    else /* In case of UNRESET perform the PCS side first */
    {
        CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, portPcsType, reset_pcs));
        CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, reset_mac));
    }


#ifndef RAVEN_DEV_SUPPORT
    if ((curPortParams.portMacType == GEMAC_SG) ||
        (curPortParams.portMacType == GEMAC_X))
    {
        CHECK_STATUS(mvHwsMacModeSelectDefaultCfg(devNum, portGroup,
                                    curPortParams.portMacNumber,GEMAC_X));
    }
#endif
    return GT_OK;
}

