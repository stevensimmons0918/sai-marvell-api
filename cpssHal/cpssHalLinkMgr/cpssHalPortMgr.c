/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// file cpssHalPort.c
//


#include "xpsInit.h"
#include "xpsLock.h"
#include "cpssHalUtil.h"
#include "gtEnvDep.h"
#include "cpssDxChPortCtrl.h"
#include "cpssDxChPortManager.h"
#include "cpssHalPort.h"
#include "cpss/common/cpssTypes.h"
#include "cpssHalDevice.h"
#include "xpsMac.h"
#include "xpsLink.h"
#include "xpsUmac.h"
#include "openXpsEnums.h"
#include "cpssHalInit.h"
#include "cpssHalLed.h"
#include "cpssHalPhyConfig.h"
#include "cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h"
#include "cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

XP_STATUS cpssHalMacMgrGetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t *status)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PORT_MANAGER_STATUS_STC portStagePtr;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    GT_BOOL linkUp;

    cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerStatusGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }

    cpssRet = cpssDxChPortLinkStatusGet(cpssDevId, cpssPortNum, &linkUp);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", devId, portNum, cpssRet);
        return ret;
    }

    if (!cpssHalPortMacAnEnableGet(devId, portNum))
    {
        if ((linkUp == GT_TRUE) ||
            (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E))
        {
            *status = 1;
            cpssRet = cpssDxChBrgEgrFltPortLinkEnableSet(cpssDevId,
                                                         (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgEgrFltPortLinkEnableSet up dev %d port %d failed(%d)", cpssDevId,
                      cpssPortNum, cpssRet);
            }
        }
        else
        {
            *status = 0;
        }
    }
    else
    {
        if ((linkUp == GT_TRUE) &&
            (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E))
        {
            *status = 1;
            cpssRet = cpssDxChBrgEgrFltPortLinkEnableSet(cpssDevId,
                                                         (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgEgrFltPortLinkEnableSet up dev %d port %d failed(%d)", cpssDevId,
                      cpssPortNum, cpssRet);
                return ret;
            }
        }
        else
        {
            cpssRet = cpssDxChBrgEgrFltPortLinkEnableSet(cpssDevId,
                                                         (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgEgrFltPortLinkEnableSet down dev %d port %d failed(%d)", cpssDevId,
                      cpssPortNum, cpssRet);
                return ret;
            }
            *status = 0;
        }
    }

    // 10G LED update
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_SPEED_ENT cpssSpeed;

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet == GT_OK)
    {
        cpssSpeed = cpssPortParams.portParamsType.regPort.speed;
        cpssHalLedPortForcedStatus(cpssDevId, cpssPortNum, cpssSpeed, *status);
    }

    return XP_NO_ERR;

}

XP_STATUS cpssHalMacMgrGetPortSpeed(xpsDevice_t devId, uint32_t portNum,
                                    xpSpeed *speed)
{
    XPS_LOCK(xpsMacGetPortSpeed);
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_SPEED_ENT cpssSpeed;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }

    if (!cpssHalPortMacAnEnableGet(devId, portNum))
    {
        cpssSpeed = cpssPortParams.portParamsType.regPort.speed;
    }
    else
    {
        cpssSpeed = cpssPortParams.portParamsType.apPort.modesArr[0].speed;
    }


    switch (cpssSpeed)
    {
        case CPSS_PORT_SPEED_10_E:
            *speed = SPEED_10MB;
            break;
        case CPSS_PORT_SPEED_100_E:
            *speed = SPEED_100MB;
            break;
        case CPSS_PORT_SPEED_1000_E:
            *speed = SPEED_1GB;
            break;
        case CPSS_PORT_SPEED_2500_E:
            *speed = SPEED_2_5GB;
            break;
        case CPSS_PORT_SPEED_10000_E:
            *speed = SPEED_10GB;
            break;
        case CPSS_PORT_SPEED_40000_E:
            *speed = SPEED_40GB;
            break;
        case CPSS_PORT_SPEED_100G_E:
            *speed = SPEED_100GB;
            break;
        case CPSS_PORT_SPEED_25000_E:
            *speed = SPEED_25GB;
            break;
        case CPSS_PORT_SPEED_50000_E:
            *speed = SPEED_50GB;
            break;
        case CPSS_PORT_SPEED_200G_E:
            *speed = SPEED_200GB;
            break;
        case CPSS_PORT_SPEED_400G_E:
            *speed = SPEED_400GB;
            break;
        case CPSS_PORT_SPEED_NA_E:
            *speed = SPEED_MAX_VAL;
            break;
        default:
            break;

    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacMgrPortEnable(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t enable)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PORT_MANAGER_STC portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStagePtr;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    XP_DEV_TYPE_T devType;
    //GT_BOOL signalDetect = GT_FALSE;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssHalGetDeviceType(devId, &devType);

    cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerStatusGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }

    if (cpssHalPortMacAnEnableGet(devId, portNum))
    {
        if (!enable)
        {
            if ((portStagePtr.portState == CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E) ||
                (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E))
            {
                cpssOsPrintf("Link state for dev %d port %d is already down, state %d\n",
                             devId, cpssPortNum, portStagePtr.portState);
            }
            // Link down the port
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E;
            //    forceDown = GT_TRUE;
            cpssOsPrintf("AUTONEG CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E dev %d port %d\n",
                         devId, cpssPortNum);
            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &portEventStc);
            if (cpssRet == GT_TIMEOUT)
            {
                cpssOsPrintf("disable timeout\n");
                return cpssRet;
            }
#if 0
            cpssRet = cpssDxChBrgEgrFltPortLinkEnableSet(cpssDevId, cpssPortNum,
                                                         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgEgrFltPortLinkEnableSet dev %d port %d failed(%d)", devId, portNum,
                      cpssRet);
            }
#endif
        }
        else
        {
            if (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
            {
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
                cpssOsPrintf("AUTONEG CPSS_PORT_MANAGER_EVENT_CREATE_E enabling dev %d port %d\n",
                             devId, cpssPortNum);
            }
            else
            {
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E;
                cpssOsPrintf("AUTONEG CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E dev %d port %d\n",
                             devId, cpssPortNum);
            }

            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &portEventStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                      cpssRet);
                return ret;

            }
#if 0
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &portEventStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                      cpssRet);
                return ret;

            }
#endif
            cpssRet = cpssDxChBrgEgrFltPortLinkEnableSet(cpssDevId,
                                                         (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgEgrFltPortLinkEnableSet down dev %d port %d failed(%d)", cpssDevId,
                      cpssPortNum, cpssRet);
                return ret;
            }
        }
#if 0
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
        cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &portEventStc);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                  cpssRet);
            return ret;

        }
#endif
    }
    else
    {
        // Disable the port
        //regular port enable flow
        if (!enable)
        {
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E;
            cpssOsPrintf("FIXED CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E dev %d port %d\n",
                         devId, cpssPortNum);

            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &portEventStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                      cpssRet);
                return ret;
            }
#if defined(INCLUDE_MPD)
            if (IS_DEVICE_AC5X(devType))
            {
                if (cpssPortNum >= 0  && cpssPortNum <= 47)
                {
                    cpssHalMpdPortDisable(cpssDevId, cpssPortNum, TRUE);
                }
            }
#endif
        }
        if (enable)
        {
#if defined(INCLUDE_MPD)
            if (IS_DEVICE_AC5X(devType))
            {
                if (cpssPortNum >= 0  && cpssPortNum <= 47)
                {
                    cpssHalMpdPortDisable(cpssDevId, cpssPortNum, FALSE);
                }
            }
#endif
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E;
            cpssOsPrintf("FIXED CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E dev %d port %d\n",
                         devId, cpssPortNum);

            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &portEventStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                      cpssRet);
                return ret;
            }
            /*
            cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                                   (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerStatusGet dev %d port %d failed(%d)", devId, portNum,
                      cpssRet);
                return ret;
            }

            if ((portStagePtr.portState == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E) &&
                (portStagePtr.portUnderOperDisable == GT_FALSE))
            {
                cpssOsPrintf("inside CPSS_PORT_MANAGER_STATE_LINK_DOWN_E \n");
                cpssRet = cpssDxChPortSerdesSignalDetectGet(cpssDevId,
                                                            (GT_PHYSICAL_PORT_NUM)cpssPortNum, &signalDetect);
                if (cpssRet != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPortSerdesSignalGet dev %d port %d failed(%d)", devId, portNum,
                          cpssRet);
                    return ret;
                }
                cpssOsPrintf("signal detect %d \n", signalDetect);
                if (signalDetect)
                {
                    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
                    cpssRet = cpssDxChPortManagerEventSet(cpssDevId,
                                                          (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portEventStc);
                    if (cpssRet != GT_OK)
                    {
                        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                              cpssRet);
                        return ret;
                    }
                }
                cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                                       (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
                if (cpssRet != GT_OK)
                {
                    ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPortManagerStatusGet dev %d port %d failed(%d)", devId, portNum,
                          cpssRet);
                    return ret;
                }

                // printf("Current State = %d", portStagePtr.portState);
                while (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
                {
                    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;
                    cpssRet = cpssDxChPortManagerEventSet(cpssDevId,
                                                          (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portEventStc);
                    if (cpssRet != GT_OK)
                    {
                        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                              cpssRet);
                        return ret;
                    }
                    cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
                    if (cpssRet != GT_OK)
                    {
                        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "cpssDxChPortManagerStatusGet dev %d port %d failed(%d)", devId, portNum,
                              cpssRet);
                        return ret;
                    }

                }

            }
                */

            /* JIRA LARCH-16
             * We need to enable Bypass for 2.5G CPU port
             * on RAMAN and MCS boards
             */
            cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId, cpssPortNum,
                                                       &cpssPortParamsStcPtr);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", cpssDevId,
                      cpssPortNum, cpssRet);
                // return ret;
            }
            if (cpssPortParamsStcPtr.portParamsType.regPort.speed == CPSS_PORT_SPEED_2500_E && (devType == AC3XRAMAN || devType == AC3XMCS))
            {

                cpssRet = cpssDxChPortInBandAutoNegBypassEnableSet(cpssDevId, portNum,
                                                              GT_TRUE);
                if (cpssRet != GT_OK)
                {
                    ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPortInBandAutoNegBypassEnableSet dev %d port %d failed(%d)", devId, portNum,
                          cpssRet);
                    return ret;
                }

            }
        }
    }
    return ret;

}

XP_STATUS cpssHalMacMgrFecModeSet(int devId, uint32_t portNum,
                                  xpFecMode fecMode)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    CPSS_PORT_MANAGER_STATUS_STC portStagePtr;
    GT_BOOL portAdminDisable;

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsStructInit dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        return ret;
    }
    cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerStatusGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }

    portAdminDisable = portStagePtr.portUnderOperDisable;


#if 0
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        return ret;
    }
#else
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif
    if (!cpssHalPortMacAnEnableGet(devId, portNum))
    {
        switch (fecMode)
        {
            case FC_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
                break;
            case RS_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                break;
            case RS_544_514_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case MAX_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_FEC_MODE_LAST_E;
        }
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);
    }
    else
    {
        switch (fecMode)
        {
            case FC_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
                break;
            case RS_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                break;
            case RS_544_514_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                    CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                    CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case MAX_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
        }
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);
    }
    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        return ret;
    }


    cpssPortEventsStc.portEvent = portAdminDisable?
                                  CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E: CPSS_PORT_MANAGER_EVENT_CREATE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }


    return XP_NO_ERR;

}

XP_STATUS cpssHalMacMgrFecModeGet(xpDevice_t devId, uint32_t portNum,
                                  xpFecMode *fecMode)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_DXCH_PORT_FEC_MODE_ENT cpssFecMode;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (!cpssHalPortMacAnEnableGet(devId, portNum))
    {
        cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                                   (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", devId, portNum,
                  cpssRet);
            return ret;
        }
        cpssFecMode = cpssPortParams.portParamsType.regPort.portAttributes.fecMode;
    }
    else
    {
        cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                                   (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", devId, portNum,
                  cpssRet);
            return ret;
        }
        cpssFecMode = cpssPortParams.portParamsType.apPort.modesArr[0].fecSupported;
    }

    switch (cpssFecMode)
    {
        case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
            *fecMode = FC_FEC_MODE;
            break;
        case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:
            *fecMode = MAX_FEC_MODE;
            break;
        case CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E:
            *fecMode = RS_544_514_FEC_MODE;
            break;
        case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
            *fecMode = RS_FEC_MODE;
            break;
        default:
            break;
    }

    return XP_NO_ERR;

}

XP_STATUS cpssHalPortMgrPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                      uint32_t portNum, uint32_t* rxSerdesLaneArr, uint32_t* txSerdesLaneArr)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_U8 i;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }

    for (i=0; i<CPSS_PM_MAX_LANES; i++)
    {
        *(rxSerdesLaneArr++) = (uint32_t)
                               cpssPortParams.portParamsType.regPort.laneParams[i].globalLaneNum;
        *(txSerdesLaneArr++) = (uint32_t)
                               cpssPortParams.portParamsType.regPort.laneParams[i].globalLaneNum;
    }

    return XP_NO_ERR;

}

XP_STATUS cpssHalMacMgrConfigModeGet(int devId, uint32_t portNum,
                                     xpMacConfigMode* macConfigMode)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_INTERFACE_MODE_ENT cpssMacConfigMode;
    CPSS_PORT_SPEED_ENT speed;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }
    if (cpssPortParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        cpssMacConfigMode = cpssPortParams.portParamsType.regPort.ifMode;
        speed = cpssPortParams.portParamsType.regPort.speed;
    }
    else
    {
        cpssMacConfigMode = cpssPortParams.portParamsType.apPort.modesArr[0].ifMode;
        speed = cpssPortParams.portParamsType.apPort.modesArr[0].speed;
    }
    switch (cpssMacConfigMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
            *macConfigMode = MAC_MODE_1GB;
            break;

        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            if (speed == CPSS_PORT_SPEED_1000_E)
            {
                *macConfigMode = MAC_MODE_1GB;
            }
            else
            {
                *macConfigMode = MAC_MODE_2_5GB;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_HX_E:
            *macConfigMode = MAC_MODE_MIX_SGMII;
            break;

        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            if (speed == CPSS_PORT_SPEED_10000_E)
            {
                *macConfigMode = MAC_MODE_1X10GB;
            }
            else if (speed == CPSS_PORT_SPEED_50000_E)
            {
                *macConfigMode = MAC_MODE_1X50GB;
            }
            else if (speed == CPSS_PORT_SPEED_25000_E)
            {
                *macConfigMode = MAC_MODE_1X25GB;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
            if (speed == CPSS_PORT_SPEED_10000_E)
            {
                *macConfigMode = MAC_MODE_1X10GB;
            }
            else if (speed == CPSS_PORT_SPEED_50000_E)
            {
                *macConfigMode = MAC_MODE_1X50GB;
            }
            else if (speed == CPSS_PORT_SPEED_25000_E)
            {
                *macConfigMode = MAC_MODE_1X25GB;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
            if (speed == CPSS_PORT_SPEED_100G_E)
            {
                *macConfigMode = MAC_MODE_2X100GB;
            }
            else if (speed == CPSS_PORT_SPEED_50000_E)
            {
                *macConfigMode = MAC_MODE_2X50GB;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            if (speed == CPSS_PORT_SPEED_200G_E)
            {
                *macConfigMode = MAC_MODE_4X200GB;
            }
            else if (speed == CPSS_PORT_SPEED_40000_E)
            {
                *macConfigMode = MAC_MODE_4X40GB;
            }
            else if (speed == CPSS_PORT_SPEED_100G_E)
            {
                *macConfigMode = MAC_MODE_4X100GB;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
        case CPSS_PORT_INTERFACE_MODE_KR8_E:
        case CPSS_PORT_INTERFACE_MODE_CR8_E:
            if (speed == CPSS_PORT_SPEED_400G_E)
            {
                *macConfigMode = MAC_MODE_8X400GB;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_NA_E:
            *macConfigMode = MAC_MODE_MAX_VAL;
            break;

        default:
            *macConfigMode = MAC_MODE_MIX_4_CHANNEL_10G;
    }

    return XP_NO_ERR;

}

XP_STATUS cpssHalMacMgrGetPortStatus(xpsDevice_t devId, uint32_t portNum,
                                     bool *linkStatus, bool *faultStatus, bool *serdesStatus)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_BOOL cpssPortStatus;
    CPSS_PORT_MANAGER_STATUS_STC portStagePtr;
    CPSS_PORT_MANAGER_STC portEventStc;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    *linkStatus = 0;
    *faultStatus = 0;
    *serdesStatus = 0;

    cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                           (GT_PHYSICAL_PORT_NUM)cpssPortNum, &portStagePtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", devId, portNum, cpssRet);
        return ret;
    }

    if (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
    {
        cpssPortStatus = GT_TRUE;
    }
    else
    {
        cpssPortStatus = GT_FALSE;
    }
    cpssRet = cpssDxChPortLinkStatusGet(cpssDevId,
                                        (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortStatus);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }

    if ((portStagePtr.portState == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E) &&
        (cpssPortStatus == GT_TRUE))
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
        cpssRet = cpssDxChPortManagerEventSet(cpssDevId,
                                              (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portEventStc);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", devId, portNum,
                  cpssRet);
            return ret;
        }
    }

    cpssRet = cpssDxChPortManagerStatusGet(cpssDevId,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &portStagePtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", devId, portNum, cpssRet);
        return ret;
    }

    if ((cpssPortStatus == GT_TRUE) ||
        (portStagePtr.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E))
    {
        *linkStatus = 1;


        // FALCON TODO GET from serdes - PENDING
        *faultStatus = 0;
        *serdesStatus = 1;
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacMgrPortInitWithLinkStatusControl(xpDevice_t devId,
                                                     uint32_t portNum, xpMacConfigMode macConfigMode,
                                                     CPSS_PORT_INTERFACE_MODE_ENT cpssIntfType, bool initSerdes,
                                                     bool prbsTestMode, bool firmwareUpload, xpFecMode fecMode, uint8_t enableFEC,
                                                     uint8_t keepPortDown, uint8_t initPort)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT cpssPortType;
    XP_DEV_TYPE_T devType;
    uint32_t cableLen;

    cpssOsPrintf("cpssHalMacMgrPortInitWithLinkStatusControl for port %d with macConfig %d  "
                 "and fecMode %d ifMode %d keepPortDown %d initPort %d\n",
                 portNum, macConfigMode, fecMode, cpssIntfType, keepPortDown, initPort);

    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssHalGetDeviceType(devId, &devType);

    if (cpssPortNum >= MAX_PORTNUM)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsGlobalPortToPortnum dev %d port %d failed(%d)", devId, portNum,
              cpssPortNum);
        return GT_ERROR;
    }

    cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    cableLen = cpssHalMacGetPortCableLen(cpssDevId, cpssPortNum);

    if (cpssHalPortMacAnEnableGet(devId, portNum))
    {
        cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
    }

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId, cpssPortNum,
                                               &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsGet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        //return ret;
    }
    else
    {
        if ((cpssIntfType == CPSS_PORT_INTERFACE_MODE_NA_E) &&
            (cpssPortParamsStcPtr.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E))
        {
            cpssIntfType = cpssPortParamsStcPtr.portParamsType.regPort.ifMode;
        }
    }

    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
    if (initPort)
    {
        //Init is need if switch from AP to regular mode
        cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                          &cpssPortParamsStcPtr);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortManagerPortParamsStructInit dev %d port %d failed(%d)", cpssDevId,
                  cpssPortNum, cpssRet);
            //return ret;
        }
    }

    if (IS_DEVICE_FUJITSU_LARGE(devType))
    {
        /* Fujitsu has two modes for 1G: 1000BaseX and SR_LR.
           This code does not catch this correctly
           via modifying portmanager port structure.
           So we need to re-init port if we have change from 1000BASEX to SR_LR
           with speed 1G.
        */
        if (cpssIntfType == CPSS_PORT_INTERFACE_MODE_SR_LR_E && macConfigMode == MAC_MODE_1GB
            && cpssPortParamsStcPtr.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
        {
            cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                              &cpssPortParamsStcPtr);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerPortParamsStructInit dev %d port %d failed(%d)", cpssDevId,
                      cpssPortNum, cpssRet);
                // return ret;
            }
        }
    }

    if (!cpssHalPortMacAnEnableGet(devId, portNum))
    {
        if (enableFEC == 0)
        {
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_FEC_MODE_DISABLED_E;
        }
        else
        {
            switch (fecMode)
            {
                case FC_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                    break;
                case RS_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    break;
                case RS_544_514_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                        CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    break;
                case MAX_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                        CPSS_PORT_FEC_MODE_LAST_E;
            }
        }
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);

        switch (macConfigMode)
        {
            case MAC_MODE_10M:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_SGMII_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_10_E;
                break;
            case MAC_MODE_100M:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_SGMII_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_100_E;
                break;
            case MAC_MODE_1GB:
            case MAC_MODE_4X1GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_QSGMII_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_1000_E;
                if (devType == ALDRIN2XL || devType == ALDRIN2XLFL || 
                    devType == ALDRIN2EVAL  || devType == ALDRIN2EB3)
                {
                    cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                        CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                }
                if (IS_DEVICE_AC5X(devType))
                {
                    cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                        CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E;
                }
                break;
            case MAC_MODE_2_5GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_2500_E;
                break;
            case MAC_MODE_1X10GB:
            case MAC_MODE_4X10GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_SR_LR_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_10000_E;
                break;
            case MAC_MODE_1X25GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_25000_E;
                break;
            case MAC_MODE_1X50GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_50000_E;
                break;
            case MAC_MODE_2X50GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR2_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_50000_E;
                break;
            case MAC_MODE_4X40GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_40000_E;
                break;
            case MAC_MODE_4X100GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_100G_E;
                break;
            case MAC_MODE_2X100GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR2_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_100G_E;
                break;
            case MAC_MODE_4X200GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_200G_E;
                break;
            case MAC_MODE_8X400GB:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR8_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_400G_E;
                break;
            case MAC_MODE_MAX_VAL:
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_NA_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_NA_E;
            default:
                break;
        }

        if (!IS_DEVICE_FUJITSU_LARGE(devType))
        {
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.trainMode =
                    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E;
        }

        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr,
                               CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E);

        if (cableLen == 0)
        {
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_1_E;

        }
        else if (cableLen == 1)
        {
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_DEFAULT_E;

        }
        else if (cableLen == 2)
        {
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_2_E;

        }
        if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevId))
        {
            if (cpssPortParamsStcPtr.portParamsType.regPort.ifMode ==
                CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
            {
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable
                    = GT_FALSE;
            }
        }
        if (cpssIntfType != CPSS_PORT_INTERFACE_MODE_NA_E && cpssPortParamsStcPtr.portParamsType.regPort.ifMode != CPSS_PORT_INTERFACE_MODE_SGMII_E)
        {
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode = cpssIntfType;
        }

    }
    else
    {

        if (enableFEC == 0)
        {
            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                CPSS_PORT_FEC_MODE_DISABLED_E;
            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                CPSS_PORT_FEC_MODE_DISABLED_E;
        }
        else
        {
            switch (fecMode)
            {
                case FC_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                    break;
                case RS_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    break;
                case RS_544_514_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                        CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                        CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    break;
                case MAX_FEC_MODE:
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
            }
            //CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);
            CPSS_PM_SET_VALID_AP_ATTR(&cpssPortParamsStcPtr,
                                      CPSS_PM_AP_PORT_ATTR_OPERATIONS_E);
        }

        switch (macConfigMode)
        {
            case MAC_MODE_1GB:
            case MAC_MODE_4X1GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_QSGMII_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_1000_E;
                cpssHalGetDeviceType(devId, &devType);
                if (devType == ALDRIN2XL || devType == ALDRIN2XLFL || 
                    devType == ALDRIN2EVAL  || devType == ALDRIN2EB3)
                {
                    cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                        CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                }
                if (IS_DEVICE_AC5X(devType))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                        CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E;
                }

                break;
            case MAC_MODE_2_5GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_2500_E;
                break;
            case MAC_MODE_1X10GB:
            case MAC_MODE_4X10GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_SR_LR_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_10000_E;
                break;
            case MAC_MODE_1X25GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_25000_E;
                break;
            case MAC_MODE_1X50GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_50000_E;
                break;
            case MAC_MODE_2X50GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR2_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_50000_E;
                break;
            case MAC_MODE_4X40GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_40000_E;
                break;
            case MAC_MODE_4X100GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_100G_E;
                break;
            case MAC_MODE_2X100GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR2_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_100G_E;
                break;
            case MAC_MODE_4X200GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_200G_E;
                break;
            case MAC_MODE_8X400GB:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR8_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_400G_E;
                break;
            case MAC_MODE_MAX_VAL:
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_NA_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].speed =
                    CPSS_PORT_SPEED_NA_E;
            default:
                break;
        }
        if (cpssIntfType != CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[0].ifMode = cpssIntfType;
        }
        //cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.trainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E;
    }
    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        return ret;
    }


    cpssPortEventsStc.portEvent = keepPortDown?
                                  CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E:
                                  CPSS_PORT_MANAGER_EVENT_CREATE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
    /* setting port managaer parameter cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable
    is not changing cpssDxChPortInBandAutoNegBypassEnableSet (could be bcz its AP parameter and valid attribute bit was not enabled
    )so setting the BypassEnable explicitly
    */
    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevId))
    {
        if (cpssPortParamsStcPtr.portParamsType.regPort.ifMode ==
            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
        {
            cpssRet =  cpssDxChPortInBandAutoNegBypassEnableSet(cpssDevId, cpssPortNum,
                                                                GT_FALSE);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
                      cpssRet);
                return ret;
            }
        }
    }
#if 0
    if (keepPortDown ==  1)
    {
        cpssRet = cpssHalMacPortEnable(devId, portNum, 0);
    }

    else
    {
        cpssRet = cpssHalMacPortEnable(devId, portNum, 1);
    }
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalMacPortEnable dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
#endif
    return XP_NO_ERR;

}

XP_STATUS cpssHalMacMgrLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                   xpMacLoopbackLevel loopback,  uint8_t *status)
{
    XPS_LOCK(xpsMacGetPortSpeed);
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    switch (loopback)
    {
        case MAC_LEVEL_LOOPBACK:
            *status =(uint8_t)
                     cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable;
            break;
        case MAC_PCS_LEVEL_LOOPBACK:
            if (cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.pcsLoopbackMode!=
                CPSS_PORT_PCS_LOOPBACK_DISABLE_E)
            {
                *status=1;
            }
            break;
        case TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK is not supported dev %d port %d ",
                  cpssDevId, cpssPortNum);
            break;
        case MAC_FIFO_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_FIFO_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevId,
                  cpssPortNum);
            break;
        case RXTOTX_GMII_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RXTOTX_GMII_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevId,
                  cpssPortNum);
            break;
        case MAC_MAX_LOOPBACK_MODES:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_MAX_LOOPBACK_MODES  is not supported dev %d port %d ", cpssDevId,
                  cpssPortNum);
            break;
        default:
            break;

    }
    return ret;
}

XP_STATUS cpssHalMacMgrLoopback(xpsDevice_t devId, uint32_t portNum,
                                xpMacLoopbackLevel loopback, uint8_t enable)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);

    //set the loopback to MAC
    cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackType=
        CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E;
    //set the loopback mode.
    switch (loopback)
    {
        case MAC_LEVEL_LOOPBACK:
            if (enable==1)
            {
                cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable
                    =GT_TRUE;
            }
            else
            {
                cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable
                    =GT_FALSE;
            }
            break;
        case MAC_PCS_LEVEL_LOOPBACK:
            if (enable==1)
            {
                cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.pcsLoopbackMode
                    =CPSS_PORT_PCS_LOOPBACK_TX2RX_E;
            }
            else
            {
                cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.pcsLoopbackMode
                    =CPSS_PORT_PCS_LOOPBACK_DISABLE_E;
            }
            break;
        case TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK  is not supported dev %d port %d ",
                  cpssDevId, cpssPortNum);
            break;
        case MAC_FIFO_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_FIFO_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevId,
                  cpssPortNum);
            break;
        case RXTOTX_GMII_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RXTOTX_GMII_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevId,
                  cpssPortNum);
            break;
        case MAC_MAX_LOOPBACK_MODES:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_MAX_LOOPBACK_MODES  is not supported dev %d port %d ", cpssDevId,
                  cpssPortNum);
            break;
        default:
            break;
    }
    CPSS_PM_SET_VALID_ATTR(&cpssPortParams, CPSS_PM_ATTR_LOOPBACK_E);
    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParams);

    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum,
              cpssRet);
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    cpssPortEventsStc.portEvent = enable ?
                                  CPSS_PORT_MANAGER_EVENT_CREATE_E:
                                  CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
    return XP_NO_ERR;
}


XP_STATUS cpssHalMacMgrPortSerdesTune(int devId, xpsPort_t *portList,
                                      uint32_t numOfPort, xpSerdesDfeTuneMode_t tuneMode, uint8_t force)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT cpssSerdesTuneMode;
    uint32_t portIndex = 0;
    uint32_t portNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;

    switch (tuneMode)
    {
        case XP_DFE_ICAL:
            cpssSerdesTuneMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
            break;
        case XP_DFE_PCAL:
            cpssSerdesTuneMode=
                CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E;
            break;
        case XP_DFE_START_ADAPTIVE:
            cpssSerdesTuneMode =
                CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E;
            break;

        case XP_DFE_STOP_ADAPTIVE:
            cpssSerdesTuneMode =
                CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E;
            break;

        default:
            cpssSerdesTuneMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
            break;

    }

    for (portIndex = 0; portIndex < numOfPort; portIndex++)
    {
        portNum = portList[portIndex];
        cpssDevId = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        if (xpsIsPortInited(devId, portNum) == XP_NO_ERR)
        {
            cpssDxChPortManagerPortParamsGet(cpssDevId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                             &cpssPortParams);
#if 0
            cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
            cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &cpssPortEventsStc);
#else
            cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif

            cpssPortParams.portParamsType.regPort.portAttributes.trainMode =
                cpssSerdesTuneMode;
            CPSS_PM_SET_VALID_ATTR(&cpssPortParams, CPSS_PM_ATTR_TRAIN_MODE_E);
            cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                                       (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParams);

            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }

            cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                                  &cpssPortEventsStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
                      cpssRet);
                return ret;
            }

        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Port: %d not initialized. Can't perform requested operation on this port\n",
                  portNum);
        }
    }
    return XP_NO_ERR;

}

XP_STATUS cpssHalMacMgrPortSerdesTxTune(int devId, xpsPort_t *portList,
                                        uint32_t numOfPort, uint8_t force)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    uint32_t portIndex = 0;
    uint32_t portNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT cpssPortType;

    for (portIndex = 0; portIndex < numOfPort;
         portIndex++) // followed directions given for using cpssDxChPortSerdesAutoTune(cpssDxChPortCtrl.h) for Tx training.
    {
        portNum = portList[portIndex];
        cpssDevId = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
        cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;

        if (xpsIsPortInited(devId, portNum) == XP_NO_ERR)
        {
#if 0
            cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
            cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &cpssPortEventsStc);
#else
            cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif
            cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                              &cpssPortParamsStcPtr);

            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_50000_E;
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.trainMode =
                CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E;
            CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_TRAIN_MODE_E);

            cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                                       (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);

            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }

            cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                                  &cpssPortEventsStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
                      cpssRet);
                return ret;
            }

#if 0
            cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
            cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum, &cpssPortEventsStc);
#else
            cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif
            cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                              &cpssPortParamsStcPtr);

            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }
            cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.trainMode =
                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E;
            CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_TRAIN_MODE_E);

            cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                                       (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);

            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }

            cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
            cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                                  &cpssPortEventsStc);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
                      cpssRet);
                return ret;
            }


        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Port: %d not initialized. Can't perform requested operation on this port\n",
                  portNum);
        }

    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalMgrSerdesLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t *status)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT modePtr;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    else
    {
        if (cpssPortParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
        {
            *status = 0;

        }
        else
        {
            modePtr = cpssPortParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode;
            if (modePtr == CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E)
            {
                *status = 0;
            }
            else if (modePtr == CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)
            {
                *status = 1;
            }
            else
            {
                *status = 1;
            }
        }

    }
    return ret;
}

XP_STATUS cpssHalMgrSerdesLoopback(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t enable)
{

    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParamsStcPtr);
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);

    if (enable==1)
    {
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);

        //set the loopback to Serdes
        cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.loopback.loopbackType
            = CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E;
        cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode
            = CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E;
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_LOOPBACK_E);
    }
    else
    {
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);

        cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode
            = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_LOOPBACK_E);

    }

    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    cpssPortEventsStc.portEvent = enable ?
                                  CPSS_PORT_MANAGER_EVENT_CREATE_E:
                                  CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }


    return XP_NO_ERR;
}

XP_STATUS cpssHalMacMgrPortDeInit(xpsDevice_t devId, uint32_t portNum)
{
    //XP_STATUS ret = XP_NO_ERR;
    //GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    //CPSS_PORT_MANAGER_STC cpssPortEventsStc;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
#if 0
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK && cpssRet!=GT_BAD_PARAM)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
#else
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif
    return XP_NO_ERR;
}

XP_STATUS cpssHalMacMgrPortInit(xpsDevice_t devId, uint32_t portNum,
                                xpMacConfigMode macConfigMode, xpFecMode fecMode, uint8_t enableFEC)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT cpssPortType;
    XP_DEV_TYPE_T devType;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssOsPrintf("inside cpssHalMacMgrPortInit fecMode %d\n", fecMode);

#if 0
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK && cpssRet!= GT_BAD_PARAM)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
#else
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif

    cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                      &cpssPortParamsStcPtr);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    if (enableFEC == 0)
    {
        cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
            CPSS_PORT_FEC_MODE_DISABLED_E;
    }
    else
    {
        switch (fecMode)
        {
            case FC_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
                break;
            case RS_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                break;
            case RS_544_514_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case MAX_FEC_MODE:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_FEC_MODE_LAST_E;
        }
    }

    CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);

    switch (macConfigMode)
    {
        case MAC_MODE_1GB:
        case MAC_MODE_4X1GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_QSGMII_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_1000_E;
            cpssHalGetDeviceType(devId, &devType);
            if (IS_DEVICE_AC5X(devType))
            {
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E;
            }


            break;
        case MAC_MODE_2_5GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_2500_E;
            break;
        case MAC_MODE_1X10GB:
        case MAC_MODE_4X10GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_10000_E;
            break;
        case MAC_MODE_1X25GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case MAC_MODE_1X50GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_50000_E;
            break;
        case MAC_MODE_2X50GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR2_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_50000_E;
            break;
        case MAC_MODE_4X40GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR4_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_40000_E;
            break;
        case MAC_MODE_4X100GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR4_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_100G_E;
            break;
        case MAC_MODE_2X100GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR2_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_100G_E;
            break;
        case MAC_MODE_4X200GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR4_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_200G_E;
            break;
        case MAC_MODE_8X400GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR8_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_400G_E;
            break;
        case MAC_MODE_MAX_VAL:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_NA_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_NA_E;
        default:
            break;
    }

    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
    return XP_NO_ERR;

}

XP_STATUS cpssHalPortMgrAutoNegEnable(xpsDevice_t devId, uint32_t portNum,
                                      uint32_t portANAbility, uint16_t portANCtrl, uint8_t keepPortDown)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, cableLen;
    GT_U32 currMode = 0;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_SPEED_ENT         prevSpeed;
    CPSS_PORT_INTERFACE_MODE_ENT prevInterfaceMode;

    CPSS_PORT_MANAGER_PORT_TYPE_ENT cpssPortType;
    GT_BOOL skipLowSpeedConfig = GT_FALSE;

    cpssHalSetDeviceSwitchId(devId);

    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(devId, &devType);

    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (cpssPortNum >= MAX_PORTNUM)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsGlobalPortToPortnum dev %d port %d failed(%d)", devId, portNum,
              cpssPortNum);
        return GT_ERROR;
    }

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerportParamsGet dev %d port %d failed(%d)", devId, portNum,
              cpssRet);
        return ret;
    }

    if (cpssPortParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        prevSpeed = cpssPortParams.portParamsType.apPort.modesArr[0].speed;
        prevInterfaceMode = cpssPortParams.portParamsType.apPort.modesArr[0].ifMode;
    }
    else
    {
        prevSpeed =  cpssPortParams.portParamsType.regPort.speed ;
        prevInterfaceMode =  cpssPortParams.portParamsType.regPort.ifMode;
    }

    cpssOsPrintf("cpssHalPortMgrAutoNegEnable : Enabling AN for port %d with AN ability 0x%x and portANCtrl 0x%x keepPortDown %d\n",
                 portNum, portANAbility, portANCtrl, keepPortDown);
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);

    cableLen = cpssHalMacGetPortCableLen(cpssDevId, cpssPortNum);
    cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
    cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                      &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    //    if (prevSpeed == CPSS_PORT_SPEED_100G_E &&(
    //            prevInterfaceMode ==  CPSS_PORT_INTERFACE_MODE_CR4_E||
    //            prevInterfaceMode ==  CPSS_PORT_INTERFACE_MODE_KR4_E))

    if (devType == FC64x100GR4IXIA || devType == FC32x100GR4IXIA ||
        devType == F2T80x25GIXIA)
    {
        //cpssOsPrintf("cpssHalPortMgrAutoNegEnable: CPSS_PORT_SPEED_100G_E for KR4");
        if (portANAbility & IEEE_TECH_ABILITY_100GBASE_KR4_BIT)
        {

            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                CPSS_PORT_INTERFACE_MODE_CR4_E;
            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                CPSS_PORT_SPEED_100G_E;
            if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                               UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS))
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            }
            else if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                    UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_FC))
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
            }
            else
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
            }

            currMode++;
        }
        if (portANAbility & IEEE_TECH_ABILITY_50GBASE_KR2_BIT)
        {

            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                CPSS_PORT_INTERFACE_MODE_CR2_C_E;
            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                CPSS_PORT_SPEED_50000_E;
            if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                              UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            }
            else
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
            }
            currMode++;
        }
        if (portANAbility &
            (IEEE_TECH_ABILITY_25GBASE_KR1_BIT|IEEE_TECH_ABILITY_25GBASE_CR1_BIT))
        {

            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                CPSS_PORT_INTERFACE_MODE_CR_E;
            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                CPSS_PORT_SPEED_25000_E;
            if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                               UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS))
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            }
            else if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                    UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_FC))
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_FEC_MODE_ENABLED_E;
            }
            else
            {
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                    CPSS_PORT_FEC_MODE_DISABLED_E;
            }
            currMode++;
        }
    }
    else

    {
        if (devType == FALCONEBOF)
        {
            if (portANAbility & IEEE_TECH_ABILITY_200GBASE_KR4_CR4_BIT)
            {
                skipLowSpeedConfig = GT_TRUE;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_CR4_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_200G_E;
                if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                  UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }
                currMode++;
            }
            if ((portANAbility & IEEE_TECH_ABILITY_100GBASE_KR4_BIT) ||
                (portANAbility & IEEE_TECH_ABILITY_100GBASE_CR4_BIT))
            {
                skipLowSpeedConfig = GT_TRUE;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_CR4_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_100G_E;
                if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                   UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                }
                else if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                        UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_FC))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }

                currMode++;
            }
        }
        else
        {
            if (portANAbility & IEEE_TECH_ABILITY_400GBASE_KR8_BIT)
            {

                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR8_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_400G_E;
                if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                  UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }
                currMode++;
            }
            if (portANAbility & IEEE_TECH_ABILITY_200GBASE_KR4_CR4_BIT)
            {

                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR4_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_200G_E;
                if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                  UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }
                currMode++;
            }
            if (portANAbility & IEEE_TECH_ABILITY_100GBASE_KR2_CR2_BIT)
            {

                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR2_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_100G_E;
                if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                  UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }
                currMode++;
            }
            if (portANAbility & IEEE_TECH_ABILITY_100GBASE_KR4_BIT)
            {
                if (prevInterfaceMode != CPSS_PORT_INTERFACE_MODE_KR2_E)
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                        CPSS_PORT_INTERFACE_MODE_KR4_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                        CPSS_PORT_SPEED_100G_E;
                    if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                       UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS))
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    }
                    else if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                            UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_FC))
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                    }
                    else
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                    }
                    currMode++;
                }
            }
        }

        if (skipLowSpeedConfig == GT_FALSE)
        {
            if (portANAbility & IEEE_TECH_ABILITY_50GBASE_KR2_BIT)
            {

                if (prevSpeed == CPSS_PORT_SPEED_100G_E &&(
                        prevInterfaceMode ==  CPSS_PORT_INTERFACE_MODE_CR4_E||
                        prevInterfaceMode ==  CPSS_PORT_INTERFACE_MODE_KR4_E))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                        CPSS_PORT_INTERFACE_MODE_KR2_C_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                        CPSS_PORT_SPEED_50000_E;
                    if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                      UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    }
                    else
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                    }
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                        CPSS_PORT_INTERFACE_MODE_KR_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                        CPSS_PORT_SPEED_50000_E;
                    if (portANCtrl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                      UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                    }
                    else
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                    }
                }
                currMode++;
            }
            if (portANAbility&
                (IEEE_TECH_ABILITY_25GBASE_KR1_BIT|IEEE_TECH_ABILITY_25GBASE_CR1_BIT))
            {

                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_25000_E;
                if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                   UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                }
                else if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                        UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_FC))
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_ENABLED_E;
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }
                currMode++;
            }
            if (portANAbility&
                (IEEE_TECH_ABILITY_10GBASE_KR_BIT|IEEE_TECH_ABILITY_10GBASE_KX4_BIT))
            {

                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                    CPSS_PORT_SPEED_10000_E;
                if ((prevSpeed == CPSS_PORT_SPEED_10000_E) ||
                    (prevSpeed  ==
                     CPSS_PORT_SPEED_25000_E)) //work around for higher speeds as adding 10G with fec enabled doesnt work
                {
                    if (portANCtrl == (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                                       UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_FC))
                    {
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_FEC_MODE_ENABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_FEC_MODE_ENABLED_E;
                    }
                    else
                    {
                        cpssOsPrintf("inside CPSS_PORT_SPEED_10000_E fec disable\n");
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                            CPSS_PORT_FEC_MODE_DISABLED_E;
                    }
                }
                else
                {
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecSupported =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                    cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].fecRequested =
                        CPSS_PORT_FEC_MODE_DISABLED_E;
                }
                currMode++;
                /*            cpssOsPrintf("adding 1G for 10G");
                            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
                                CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                            cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
                                CPSS_PORT_SPEED_1000_E;
                            currMode++;
                */

            }
        }

    }

    cpssPortParamsStcPtr.portParamsType.apPort.numOfModes=currMode;
    if (cableLen == 1)
    {
        cpssPortParamsStcPtr.portParamsType.apPort.apAttrs.interconnectProfile =
            CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_DEFAULT_E;
        CPSS_PM_SET_VALID_AP_ATTR(&cpssPortParamsStcPtr,
                                  CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);
        cpssOsPrintf("Setting cable len CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_DEFAULT_E for AP port %d\n",
                     portNum);
    }
    else if (cableLen == 0)
    {
        cpssPortParamsStcPtr.portParamsType.apPort.apAttrs.interconnectProfile =
            CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_1_E;
        CPSS_PM_SET_VALID_AP_ATTR(&cpssPortParamsStcPtr,
                                  CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);
        cpssOsPrintf("Setting cable len CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_1_E for AP port %d\n",
                     portNum);
    }

    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevId,
              portNum, cpssRet);
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    cpssPortEventsStc.portEvent = keepPortDown?
                                  CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E:
                                  CPSS_PORT_MANAGER_EVENT_CREATE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d) for CPSS_PORT_MANAGER_EVENT_CREATE_E",
              cpssDevId, portNum, cpssRet);
        return ret;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalPortMgrAutoNegDisable(xpsDevice_t devId, uint32_t portNum)
{
    XP_STATUS ret = XP_NO_ERR;
    //GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    //CPSS_PORT_MANAGER_STC cpssPortEventsStc;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

#if 0
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;

    cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK && cpssRet!= GT_BAD_PARAM)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
#else
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);
#endif

    ret = cpssHalMacPortInit(devId, portNum, MAC_MODE_1X50GB, RS_544_514_FEC_MODE,
                             1);

    return ret;
}

XP_STATUS cpssHalPortMgrMirrorEnable(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t analyzerId, bool ingress, bool enable)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (ingress)
    {
        cpssRet = cpssDxChMirrorRxPortSet(cpssDevId, cpssPortNum, true, enable,
                                          analyzerId);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set ingress mirror on port %d with error %d", cpssPortNum, cpssRet);
            return ret;
        }
    }
    else
    {
        cpssRet = cpssDxChMirrorTxPortSet(cpssDevId, cpssPortNum, true, enable,
                                          analyzerId);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set egress mirror on port %d with error %d", cpssPortNum, cpssRet);
            return ret;
        }
    }

    return ret;
}


XP_STATUS cpssHalMacMgrGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                        uint16_t *portAdvertFdAbility)
{
    GT_U32  cpssFdmodes;
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_U32 iter;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    *portAdvertFdAbility =  CPSSHAL_PORT_ADVERT_SPEED_NONE;

    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevId,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
    cpssFdmodes = cpssPortParamsStcPtr.portParamsType.apPort.numOfModes;
    for (iter = 0; iter<cpssFdmodes; iter++)
    {
        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_10_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |
                                   CPSSHAL_PORT_ADVERT_SPEED_10MB;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_100_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |
                                   CPSSHAL_PORT_ADVERT_SPEED_100MB;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_1000_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |
                                   CPSSHAL_PORT_ADVERT_SPEED_1000MB;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_10000_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |  CPSSHAL_PORT_ADVERT_SPEED_10G;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_25000_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |  CPSSHAL_PORT_ADVERT_SPEED_25G;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_40000_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |  CPSSHAL_PORT_ADVERT_SPEED_40G;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_50000_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility)|  CPSSHAL_PORT_ADVERT_SPEED_50G;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_100G_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |
                                   CPSSHAL_PORT_ADVERT_SPEED_100G;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_200G_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |
                                   CPSSHAL_PORT_ADVERT_SPEED_200G;
        }

        if (cpssPortParamsStcPtr.portParamsType.apPort.modesArr[iter].speed ==
            CPSS_PORT_SPEED_400G_E)
        {
            *portAdvertFdAbility = (*portAdvertFdAbility) |
                                   CPSSHAL_PORT_ADVERT_SPEED_400G;
        }

    }
    return ret;

}

void cpssHalDeletPort(GT_U8 cpssDevId, uint32_t cpssPortNum, int retries)
{
    GT_STATUS cpssRet= GT_ERROR;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    int retryCount = 0;
    while (retries>0 && cpssRet != GT_OK)
    {

        cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;

        cpssRet = cpssDxChPortManagerEventSet(cpssDevId, cpssPortNum,
                                              &cpssPortEventsStc);
        if (cpssRet != GT_OK)
        {
            cpssOsTimerWkAfter(100);
        }

        retries --;
        retryCount++;
    }
    if (retries == 0 && cpssRet != GT_OK)
    {
        cpssOsPrintf("Delete failed for portNum %d with return value  %d\n",
                     cpssPortNum,
                     cpssRet);
    }
    else
    {
        cpssOsPrintf("Delete passed after %d retries  for  portNum %d \n", retryCount,
                     cpssPortNum);
    }
    return;

}
GT_STATUS cpssHalLinkstatuschangeEvent(GT_U8 devNum, uint32_t portNum)

{
    GT_STATUS                    rc = GT_NOT_APPLICABLE_DEVICE;
    CPSS_PORT_MANAGER_STC        portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStage;
    CPSS_PM_PORT_PARAMS_STC      portParams;
    GT_BOOL                      linkUp        = GT_FALSE;
    GT_BOOL                      linkStatusPM  = GT_FALSE;
    memset(&portEventStc, 0, sizeof(CPSS_PORT_MANAGER_STC));
    memset(&portStage, 0, sizeof(CPSS_PORT_MANAGER_STATUS_STC));
    memset(&portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));

/* Suppressing unnecessary print. Flooding consumed console.
 *  cpssOsPrintf("EVENT cpssHalLinkstatuschangeEvent dev=%d, portNum=%d\n", devNum,
 *               portNum);
 */

    rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStage);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Error: PortManagerStatusGet returned=%d, portNum=%d", rc,
                     portNum);
        return rc;
    }

    if (portStage.portUnderOperDisable == GT_TRUE)
    {
        cpssOsPrintf("EVENT: port is in disable so abort mac_level_change notification\n");
        //return GT_OK;
    }

    if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E)
    {
        cpssOsPrintf("EVENT: port is in HW FAILURE state so abort mac_level_change notification\n");
    }

    if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
    {
        cpssOsPrintf("EVENT: port is in RESET state so abort mac_level_change notification");
        //return GT_OK;
    }

    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("EVENT: cpssDxChPortManagerPortParamsGet returned=%d, portNum=%d",
                         rc,
                         portNum);
        }
    }

    if (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
        if (rc != GT_OK)
        {
            cpssOsPrintf("EVENT: cpssDxChPortLinkStatusGet returned=%d, portNum=%d", rc,
                         portNum);
        }

        if (portStage.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
        {
            linkStatusPM = GT_TRUE;
        }

        if (linkStatusPM == linkUp)
        {
            /* there is no change in the signal status, no need to handle status change anymore.
               this was made to ignore interrupts that has no effect on the link */
            cpssOsPrintf("EVENT: link status changed for port %d\n", portNum);
            return GT_OK;
        }
    }

    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT brgLinkState;
    brgLinkState = (linkUp) ?
                   CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                   CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, portNum, brgLinkState);
    if (rc != GT_OK)
    {
        cpssOsPrintf("ERROR: cpssDxChBrgEgrFltPortLinkEnableSet Failed port :%d state :%d \n",
                     portNum, brgLinkState);
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;

    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
    if (rc != GT_OK)
    {
        cpssOsPrintf("Error: PortManagerEventSet event %d returned=%d, portNum=%d",
                     portEventStc.portEvent, rc, portNum);
        return rc;
    }
    return GT_OK;
}
