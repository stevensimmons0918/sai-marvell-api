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
* @file mvHwsMacIf.c
*
* @brief MAC specific HW Services API
*
* @version   15
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

#ifdef HWS_DEBUG
extern GT_FILEP fdHws;
#endif

#ifdef MV_HWS_REDUCED_BUILD
static MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr[HWS_MAX_DEVICE_NUM][LAST_MAC] = {{NULL}};
#define PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(_var) _var
#else
#define PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.macDir.mvHwsMacIfSrc._var)
#endif

#define MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(_devNum, _macType, _funcPtrName) \
        if((NULL == PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[_devNum][_macType]) || (NULL == PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[_devNum][_macType]->_funcPtrName))   return GT_NOT_IMPLEMENTED;


extern GT_STATUS mvHwsXlgMacModeSelectDefaultCfg
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                macNum
);

/**
* @internal mvHwsMacReset function
* @endinternal
*
* @brief   Set the MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    IN MV_HWS_RESET            action
)
{
    GT_STATUS rc;
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT)
    GT_U8 chipIdx = 0;
#endif
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macRestFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Mac Reset: macType %s, action %d.(0 - for reset)\n",
      PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macTypeGetFunc(), action);
    osFclose(fdHws);
#endif

    /*protect mac clock enable*/
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT)
    if ( mvHwsMtipIsReducedPort(devNum, portMacNum) )
    {
         chipIdx = (GT_U8)(portMacNum - (hwsDeviceSpecInfo[devNum].portsNum - hwsDeviceSpecInfo[devNum].numOfTiles*4));
    }
    else
    {
         chipIdx = portMacNum/16;
    }

#endif
    mvHwsHWAccessLock(devNum,
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT)
                      chipIdx,
#endif
                      MV_SEMA_PM_CM3);
    rc = PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macRestFunc(devNum, portGroup, portMacNum, portMode, action);
    /*protect mac clock enable*/

    mvHwsHWAccessUnlock(devNum,
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT)
                      chipIdx,
#endif
                      MV_SEMA_PM_CM3);
    return rc;
}

/**
* @internal mvHwsMacModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacModeCfg
(
    IN GT_U8                               devNum,
    IN GT_U32                              portGroup,
    IN GT_U32                              portMacNum,
    IN MV_HWS_PORT_STANDARD                portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portMacType == MAC_NA) || (curPortParams.portMacType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, curPortParams.portMacType, macModeCfgFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Mac Mode Cfg: macType %s, number of lanes %d.\n",
      PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][curPortParams.portMacType]->macTypeGetFunc(), curPortParams.numOfActLanes);
    osFclose(fdHws);
#endif

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][curPortParams.portMacType]->macModeCfgFunc(devNum, portGroup, portMacNum, portMode, attributesPtr);
}


/**
* @internal mvHwsMacLinkStatusGet function
* @endinternal
*
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[out] linkStatus               - link up, if true
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacLinkStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    OUT GT_BOOL                 *linkStatus
)
{
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macLinkGetFunc)

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macLinkGetFunc(devNum, portGroup, macNum, portMode, linkStatus);
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsMacLoopbackCfg function
* @endinternal
*
* @brief   Configure MAC loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[in] lbType                   - loopback type (no loopback, rx2tx or tx2rx)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacLoopbackCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    IN MV_HWS_PORT_LB_TYPE     lbType
)
{
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macLbCfgFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Mac Loopback Cfg: macType %s, type %d.\n",
      PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macTypeGetFunc(), lbType);
    osFclose(fdHws);
#endif

    PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macLbCfgFunc(devNum, portGroup, macNum, portMode, lbType);

    return GT_OK;
}

/**
* @internal mvHwsMacLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacLoopbackStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macLbStatusGetFunc)

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macLbStatusGetFunc(devNum, portGroup, macNum, portMode, lbType);
}
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsMacModeSelectDefaultCfg function
* @endinternal
*
* @brief   MAC mode select default value configuration
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacModeSelectDefaultCfg
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                macNum,
    IN MV_HWS_PORT_MAC_TYPE  macType
)
{
    macType = macType;

    switch (HWS_DEV_SILICON_TYPE(devNum))
    {
        case Bobcat3:
            /* although CPU ports of BC3 have XLG MAC, by default they are GE */
            if ((72 == macNum) || (73 == macNum))
                return GT_OK;
            break;
        case Aldrin2:/*?*/
            if (72 == macNum)
                return GT_OK;
            break;
        case Aldrin:
            /* not relevant for GIG port 32 in Aldrin */
            if (32 == macNum)
                return GT_OK;
            break;
        case BobK:
            /* not relevant for CPU port in BobK */
            if (62 == macNum)
                return GT_OK;
            break;
        default:
            break;
    }

    return mvHwsXlgMacModeSelectDefaultCfg(devNum,portGroup,macNum);
}

/**
* @internal mvHwsMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacActiveStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    OUT GT_U32                  *numOfLanes
)
{
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macActiveStatusFunc)

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macActiveStatusFunc(devNum, portGroup, macNum, portMode, numOfLanes);
}

/**
* @internal mvHwsMacClearChannelCfg function
* @endinternal
*
* @brief   Configures MAC advanced feature accordingly.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[in] txIpg                    - TX_IPG
* @param[in] txPreamble               - TX Preamble
* @param[in] rxPreamble               - RX Preamble
* @param[in] txCrc                    - TX CRC
* @param[in] rxCrc                    - RX CRC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacClearChannelCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    IN GT_U32                  txIpg,
    IN GT_U32                  txPreamble,
    IN GT_U32                  rxPreamble,
    IN MV_HWS_PORT_CRC_MODE    txCrc,
    IN MV_HWS_PORT_CRC_MODE    rxCrc
)
{
    GT_STATUS rc;

    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macClearChannelCfgFunc)

    rc = PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macClearChannelCfgFunc(devNum, portGroup, macNum, portMode, txIpg, txPreamble, rxPreamble, txCrc, rxCrc);
    return rc;
}
#endif

/**
* @internal mvHwsMacFcStateCfg function
* @endinternal
*
* @brief   Configure Flow Control state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacFcStateCfg
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          macNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_MAC_TYPE            macType,
    IN MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
)
{
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macFcStateCfgFunc)

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macFcStateCfgFunc(devNum, portGroup,
                                                                macNum, portMode, fcState);
}

/**
* @internal hwsMacGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsMacGetFuncPtr
(
   GT_U8                     devNum,
   MV_HWS_MAC_FUNC_PTRS    ***hwsFuncsPtr
)
{
   if (devNum >= HWS_MAX_DEVICE_NUM)
   {
       return GT_NOT_SUPPORTED;
   }

   *hwsFuncsPtr = PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum];

   return GT_OK;
}

/**
* @internal hwsMacIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported MAC types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsMacIfClose
(
    IN GT_U8                   devNum
)
{
    MV_HWS_PORT_MAC_TYPE macType;
    for (macType = (MV_HWS_PORT_MAC_TYPE)0 ; macType < LAST_MAC ; macType++)
    {
        if(PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType])
        {
            hwsOsFreeFuncPtr(PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]);
            PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType] = NULL;
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsMacPortEnable function
* @endinternal
*
* @brief   Perform port disable on the port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[in] enable                   - GT_TRUE for port enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacPortEnable
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    IN GT_BOOL                 enable
)
{
    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
        return GT_BAD_PARAM;
    }
    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macPortEnableFunc)

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macPortEnableFunc(devNum, portGroup, macNum, portMode, enable);
}

#ifndef BOBK_DEV_SUPPORT

/**
* @internal mvHwsMacPortEnableGet function
* @endinternal
*
* @brief   Get port enable status on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - Port Mode
* @param[in] macType                  - MAC type according to port configuration
* @param[out] enablePtr                   - GT_TRUE for port
*       enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacPortEnableGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAC_TYPE    macType,
    OUT GT_BOOL                *enablePtr
)
{

    if ((macType == MAC_NA) || (macType >= LAST_MAC))
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_MAC_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, macType, macPortEnableGetFunc);

    return PRV_NON_SHARED_MAC_DIR_MAC_IF_SRC_GLOBAL_VAR(hwsMacFuncsPtr)[devNum][macType]->macPortEnableGetFunc(devNum, portGroup, macNum, portMode, enablePtr);
}
#endif

