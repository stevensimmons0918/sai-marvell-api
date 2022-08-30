/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// file cpssHalPortNoMgr.c
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
#include "cpssHalProfile.h"
#include "cpssHalPhy.h"
#include "cpssHalLed.h"
#include "cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


XP_STATUS cpssHalMacNoMgrGetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                                       uint8_t *status)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    GT_BOOL cpssStatus;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortLinkStatusGet(cpssDevNum,
                                        (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssStatus);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", devId, portNum, cpssRet);
        return ret;
    }

    cpssRet = cpssDxChPortLinkStatusGet(cpssDevNum,
                                        (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssStatus);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", devId, portNum, cpssRet);
        return ret;
    }

    if (cpssStatus == GT_TRUE)
    {
        *status = 1;
    }

    else if (cpssStatus == GT_FALSE)
    {
        *status = 0;
    }

    // 10G LED update
    CPSS_PM_PORT_PARAMS_STC cpssPortParams;
    CPSS_PORT_SPEED_ENT cpssSpeed;

    cpssRet = cpssDxChPortManagerPortParamsGet(cpssDevNum,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, &cpssPortParams);
    if (cpssRet == GT_OK)
    {
        cpssSpeed = cpssPortParams.portParamsType.regPort.speed;
        cpssHalLedPortForcedStatus(cpssDevNum, cpssPortNum, cpssSpeed, *status);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacNoMgrPortEnable(xpsDevice_t devId, uint32_t portNum,
                                    uint8_t enable)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    GT_BOOL cpssEnable, forceDown;
    CPSS_PORT_INTERFACE_MODE_ENT cpssMacConfigMode;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (enable == 0)
    {
        cpssEnable = GT_FALSE;
        forceDown = GT_TRUE;
    }
    else
    {
        cpssEnable = GT_TRUE;
        forceDown = GT_FALSE;
    }

    cpssRet = cpssDxChPortInterfaceModeGet(cpssDevNum,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssMacConfigMode);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortInterfaceModeGet dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
    }

    cpssRet = cpssDxChPortEnableSet(cpssDevNum, (GT_PHYSICAL_PORT_NUM)cpssPortNum,
                                    cpssEnable);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortEnableSet dev %d port %d failed(%d)", devId, portNum, cpssRet);
    }
    //if(cpssEnable)
    {
#ifndef ASIC_SIMULATION
        GT_U16 data;
        switch (cpssMacConfigMode)
        {
            case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            case CPSS_PORT_INTERFACE_MODE_QSGMII_E:

                cpssRet = cpssDxChPhyPortSmiRegisterRead(cpssDevNum,
                                                         (GT_PHYSICAL_PORT_NUM)cpssPortNum, 16,
                                                         &data); /*Read reg 16 - Copper Specific Control reg 1*/
                if (cpssRet !=GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhyPortSmiRegisterRead dev %d port %d failed(%d)", devId, portNum,
                          cpssRet);
                }

                if (cpssEnable)
                {
                    data=data & 0xFFFFFFF7;
                }
                else
                {
                    data=data | 0x8 ; /*Set bit 3 Copper transmitter disable */
                }
                cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum,
                                                          (GT_PHYSICAL_PORT_NUM)cpssPortNum, 16, data);
                if (cpssRet !=GT_OK)
                {
                    cpssOsPrintf("\n SmiRegisterWrite on port %d Error %d\n", portNum, cpssRet);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", devId, portNum,
                          cpssRet);
                }

                break;

            default:
                /*Make Link partner down*/
                cpssRet = cpssDxChPortSerdesTxEnableSet(cpssDevNum,
                                                        (GT_PHYSICAL_PORT_NUM)cpssPortNum, cpssEnable);
                if (cpssRet != GT_OK && cpssRet != GT_NOT_SUPPORTED)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPortSerdesTxEnableSet dev %d port %d failed(%d)", devId, portNum,
                          cpssRet);
                }

                break;
        }

#endif

        cpssRet = cpssDxChPortForceLinkDownEnableSet(cpssDevNum,
                                                     (GT_PHYSICAL_PORT_NUM)cpssPortNum, forceDown);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortForceLinkDownEnableSet dev %d port %d failed(%d)", cpssDevNum,
                  cpssPortNum, cpssRet);
        }
    } // enable == true

    return ret;

}

XP_STATUS cpssHalMacNoMgrFecModeSet(int devId, uint32_t portNum,
                                    xpFecMode fecMode)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    CPSS_DXCH_PORT_FEC_MODE_ENT cpssFecMode;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    switch (fecMode)
    {
        case FC_FEC_MODE:
            cpssFecMode = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
            break;
        case RS_FEC_MODE:
            cpssFecMode = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
            break;
        case MAX_FEC_MODE:
            cpssFecMode = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            break;
        default:
            cpssFecMode = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            break;
    }

    cpssRet = cpssDxChPortFecModeSet(cpssDevNum, (GT_PHYSICAL_PORT_NUM)cpssPortNum,
                                     cpssFecMode);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortFecModeSet dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        return ret;
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalMacNoMgrFecModeGet(int devId, uint32_t portNum,
                                    xpFecMode *fecMode)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    CPSS_DXCH_PORT_FEC_MODE_ENT cpssFecMode;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortFecModeGet(cpssDevNum, (GT_PHYSICAL_PORT_NUM)cpssPortNum,
                                     &cpssFecMode);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortFecModeGet dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        return ret;
    }

    switch (cpssFecMode)
    {
        case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
            *fecMode = FC_FEC_MODE;
            break;
        case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:
            *fecMode = MAX_FEC_MODE;
            break;
        case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
            *fecMode = RS_FEC_MODE;
            break;
        default:
            break;
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalPortNoMgrPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                        uint32_t portNum, uint32_t* rxSerdesLaneArr, uint32_t* txSerdesLaneArr)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    GT_U32 rxArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U8 i;

    GT_U32* cpssRxSerdesLaneArr = rxArr;
    GT_U32* cpssTxSerdesLaneArr = txArr;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortXgPscLanesSwapGet(cpssDevNum,
                                            (GT_PHYSICAL_PORT_NUM) cpssPortNum, cpssRxSerdesLaneArr, cpssTxSerdesLaneArr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortXgPscLanesSwapGet dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
        return ret;
    }

    for (i=0; i<CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS; i++)
    {
        *(rxSerdesLaneArr++) = (uint32_t)(*cpssRxSerdesLaneArr++);
        *(txSerdesLaneArr++) = (uint32_t)(*cpssTxSerdesLaneArr++);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacNoMgrConfigModeGet(int devId, uint32_t portNum,
                                       xpMacConfigMode* macConfigMode)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    CPSS_PORT_INTERFACE_MODE_ENT cpssMacConfigMode;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    cpssRet = cpssDxChPortInterfaceModeGet(cpssDevNum,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssMacConfigMode);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortInterfaceModeGet dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
        return ret;
    }

    switch (cpssMacConfigMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            *macConfigMode = MAC_MODE_1GB;
            break;


        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_HX_E:
            *macConfigMode = MAC_MODE_MIX_SGMII;
            break;

        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            *macConfigMode = MAC_MODE_1X10GB;
            break;

        case CPSS_PORT_INTERFACE_MODE_KR2_E:
            *macConfigMode = MAC_MODE_2X50GB;    //As (2x10 20G/25G) modes are not available
            break;

        case CPSS_PORT_INTERFACE_MODE_NA_E:
            *macConfigMode = MAC_MODE_MAX_VAL;
            break;

        default:
            *macConfigMode = MAC_MODE_MIX_4_CHANNEL_10G;
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacNoMgrGetPortStatus(xpsDevice_t devId, uint32_t portNum,
                                       bool *linkStatus, bool *faultStatus, bool *serdesStatus)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    GT_BOOL cpssPortStatus;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    *linkStatus = 0;
    *faultStatus = 0;
    *serdesStatus = 0;

    cpssRet = cpssDxChPortLinkStatusGet(cpssDevNum,
                                        (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortStatus);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortLinkStatusGet dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        return ret;
    }

    if (cpssPortStatus == GT_TRUE)
    {
        *linkStatus = 1;
        *faultStatus = 0;
        *serdesStatus = 1;
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacNoMgrPortInitWithLinkStatusControl(int devId,
                                                       uint32_t portNum,
                                                       xpMacConfigMode macConfigMode, CPSS_PORT_INTERFACE_MODE_ENT cpssIntfType,
                                                       bool initSerdes, bool prbsTestMode,
                                                       bool firmwareUpload, xpFecMode fecMode, uint8_t enableFEC, uint8_t keepPortDown)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT cpssPortType;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    CPSS_PORT_INTERFACE_MODE_ENT cpssMacConfigMode;

    cpssRet = cpssDxChPortInterfaceModeGet(cpssDevNum,
                                           (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssMacConfigMode);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
    }

    cpssRet = cpssDxChPortManagerEventSet(cpssDevNum, cpssPortNum,
                                          &cpssPortEventsStc);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "PortManager dev %d port %d Event Delete (rc %d)\n", cpssDevNum, cpssPortNum,
          cpssRet);
    cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                      &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsStructInit dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
        return ret;
    }

    if (enableFEC != 0)
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
            case MAX_FEC_MODE:
            default:
                cpssPortParamsStcPtr.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_FEC_MODE_LAST_E;
        }
        CPSS_PM_SET_VALID_ATTR(&cpssPortParamsStcPtr, CPSS_PM_ATTR_FEC_MODE_E);
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "PortManager dev %d port %d macMode  %d\n", cpssDevNum, cpssPortNum,
          macConfigMode);
    switch (macConfigMode)
    {
        case MAC_MODE_10M:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_QSGMII_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_10_E;
            // TODO: 10M 100M doesnt work properly
            // Fall back to 1G
            cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 22, 0x0000);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 0, 0x0100);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }

            cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 0, 0x8100);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            break;
        case MAC_MODE_100M:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_QSGMII_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_100_E;
            cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 22, 0x0000);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 0, 0x2100);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }

            cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 0, 0xA100);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            break;

        case MAC_MODE_1GB:
            if (cpssMacConfigMode ==
                CPSS_PORT_INTERFACE_MODE_SR_LR_E) //For a 10G port trying to change its speed to 1G
            {
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_1000_E;
            }
            else
            {
                cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                    CPSS_PORT_INTERFACE_MODE_QSGMII_E;
                cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_1000_E;
                cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 22, 0x0000);
                if (cpssRet != GT_OK)
                {
                    ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                          cpssPortNum, cpssRet);
                    return ret;
                }
                cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 0, 0x1140);
                if (cpssRet != GT_OK)
                {
                    ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                          cpssPortNum, cpssRet);
                    return ret;
                }

                cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevNum, cpssPortNum, 0, 0x9140);
                if (cpssRet != GT_OK)
                {
                    ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhyPortSmiRegisterWrite dev %d port %d failed(%d)", cpssDevNum,
                          cpssPortNum, cpssRet);
                    return ret;
                }


            }
            break;
        case MAC_MODE_1X10GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_10000_E;
            break;
        case MAC_MODE_2X50GB:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_KR2_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case MAC_MODE_MAX_VAL:
            cpssPortParamsStcPtr.portParamsType.regPort.ifMode =
                CPSS_PORT_INTERFACE_MODE_NA_E;
            cpssPortParamsStcPtr.portParamsType.regPort.speed = CPSS_PORT_SPEED_NA_E;
        default:
            break;
    }
    if (cpssIntfType != CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        cpssPortParamsStcPtr.portParamsType.regPort.ifMode = cpssIntfType;
    }
    cpssRet = cpssDxChPortManagerPortParamsSet(cpssDevNum,
                                               (GT_PHYSICAL_PORT_NUM) cpssPortNum, &cpssPortParamsStcPtr);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerPortParamsSet dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
        return ret;
    }


    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    cpssRet = cpssDxChPortManagerEventSet(cpssDevNum, cpssPortNum,
                                          &cpssPortEventsStc);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortManagerEventSet dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
        return ret;
    }

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
              "cpssHalMacPortEnable dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        return ret;
    }
    /*
    if(macConfigMode == MAC_MODE_10M || macConfigMode == MAC_MODE_100M)
    {
        cpssRet = cpssDxChPortForceLinkPassEnableSet(cpssDevNum, cpssPortNum, GT_TRUE);
    }
    if(cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_ERROR,"cpssDxChPortForceLinkPassEnableSet dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, cpssRet);
        return ret;
    }
    */
    return XP_NO_ERR;

}


XP_STATUS cpssHalMacNoMgrLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                     xpMacLoopbackLevel loopback,  uint8_t *status)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_BOOL cpssEnable;
    GT_U32 cpssPortNum;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    switch (loopback)
    {
        case MAC_LEVEL_LOOPBACK:
            cpssRet = cpssDxChPortInternalLoopbackEnableGet(cpssDevNum, cpssPortNum,
                                                            &cpssEnable);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortInternalLoopbackEnableGet dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            else
            {
                *status= (uint8_t)cpssEnable;
            }
            break;
        case MAC_PCS_LEVEL_LOOPBACK:
            cpssRet =cpssDxChPortPcsLoopbackModeGet(cpssDevNum, cpssPortNum, &mode);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortPcsLoopbackModeGet dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            else
            {
                if (mode==CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E)
                {
                    *status=0;
                }
                else
                {
                    *status=1;
                }
            }
            break;
        case TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_FIFO_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        case MAC_FIFO_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_FIFO_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        case RXTOTX_GMII_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RXTOTX_GMII_INTERFACE_LOOPBACK  is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        case MAC_MAX_LOOPBACK_MODES:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_MAX_LOOPBACK_MODES  is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        default:
            break;
    }
    return ret;
}

XP_STATUS cpssHalMacNoMgrLoopback(xpsDevice_t devId, uint32_t portNum,
                                  xpMacLoopbackLevel loopback, uint8_t enable)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    GT_BOOL cpssEnable;
    if (enable==1)
    {
        cpssEnable = GT_TRUE;
    }
    else
    {
        cpssEnable = GT_FALSE;
    }

    cpssRet = cpssDxChPortForceLinkPassEnableSet(cpssDevNum,
                                                 (GT_PHYSICAL_PORT_NUM)cpssPortNum, cpssEnable);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortForceLinkDownEnableSet dev %d port %d failed(%d)", cpssDevNum,
              cpssPortNum, cpssRet);
    }
    cpssRet = cpssDxChPortEnableSet(cpssDevNum, (GT_PHYSICAL_PORT_NUM)cpssPortNum,
                                    cpssEnable);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortEnableSet dev %d port %d failed(%d)", devId, portNum, cpssRet);
    }

    switch (loopback)
    {
        case MAC_LEVEL_LOOPBACK:
            cpssRet = cpssDxChPortInternalLoopbackEnableSet(cpssDevNum, cpssPortNum,
                                                            cpssEnable?GT_TRUE:GT_FALSE);
            if (cpssRet != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortInternalLoopbackEnableSet  dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }
            break;
        case MAC_PCS_LEVEL_LOOPBACK:
            cpssRet = cpssDxChPortPcsLoopbackModeSet(cpssDevNum, portNum,
                                                     cpssEnable?CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E:
                                                     CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E);
            if (cpssRet != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortPcsLoopbackModeSet dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                return ret;
            }
            break;
        case TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK is not supported dev %d port %d ",
                  cpssDevNum, cpssPortNum);
            break;
        case MAC_FIFO_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_FIFO_INTERFACE_LOOPBACK is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        case RXTOTX_GMII_INTERFACE_LOOPBACK:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RXTOTX_GMII_INTERFACE_LOOPBACK is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        case MAC_MAX_LOOPBACK_MODES:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "MAC_MAX_LOOPBACK_MODES is not supported dev %d port %d ", cpssDevNum,
                  cpssPortNum);
            break;
        default:
            break;
    }
    return XP_NO_ERR;
}


XP_STATUS cpssHalPortNoMgrGetMru(xpsDevice_t devId, uint32_t portNum,
                                 uint16_t *frameSize)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;

    GT_U32 cpssFrameSize;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    if ((cpssRet = cpssDxChPortMruGet(cpssDevNum, cpssPortNum,
                                      &cpssFrameSize)!= GT_OK))
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortMruGet  dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        return ret;
    }

    *frameSize = (uint16_t)cpssFrameSize;
    return XP_NO_ERR;
}

XP_STATUS cpssHalPortNoMgrSetMru(xpsDevice_t devId, uint32_t portNum,
                                 uint16_t frameSize)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;

    GT_U32 cpssFrameSize;
    cpssFrameSize=(uint32_t)frameSize;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPortMruSet(cpssDevNum, cpssPortNum, cpssFrameSize);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortMruSet dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        return ret;
    }
    return ret;

}

XP_STATUS cpssHalMacNoMgrPortSerdesTune(int devId, xpsPort_t *portList,
                                        uint32_t numOfPort, xpSerdesDfeTuneMode_t tuneMode, uint8_t force)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT cpssSerdesTuneMode;
    uint32_t portIndex = 0;
    uint32_t portNum;

    switch (tuneMode)
    {
        case XP_DFE_ICAL:
            cpssSerdesTuneMode = CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
            break;
        case XP_DFE_PCAL:
            cpssSerdesTuneMode=
                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E;
            break;
        case XP_DFE_START_ADAPTIVE:
            cpssSerdesTuneMode =
                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E;
            break;

        case XP_DFE_STOP_ADAPTIVE:
            cpssSerdesTuneMode =
                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E;
            break;

        default:
            cpssSerdesTuneMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
            break;
    }

    for (portIndex = 0; portIndex < numOfPort; portIndex++)
    {
        portNum = portList[portIndex];
        cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        if (xpsIsPortInited(devId, portNum) == XP_NO_ERR)
        {
            cpssRet=cpssDxChPortSerdesAutoTune(cpssDevNum,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum, cpssSerdesTuneMode);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortSerdesAutoTune dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
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



XP_STATUS cpssHalMacNoMgrPortSerdesTxTune(int devId, xpsPort_t *portList,
                                          uint32_t numOfPort, uint8_t force)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet = GT_OK;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    uint32_t portIndex = 0;
    uint32_t portNum;

    for (portIndex = 0; portIndex < numOfPort;
         portIndex++) // followed directions given for using cpssDxChPortSerdesAutoTune(cpssDxChPortCtrl.h) for Tx training.
    {
        portNum = portList[portIndex];
        cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        if (xpsIsPortInited(devId, portNum) == XP_NO_ERR)
        {
            cpssRet=cpssDxChPortSerdesAutoTune(cpssDevNum,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum,
                                               CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortSerdesAutoTune dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
                      cpssRet);
                return ret;
            }

            cpssRet=cpssDxChPortSerdesAutoTune(cpssDevNum,
                                               (GT_PHYSICAL_PORT_NUM)cpssPortNum,
                                               CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortSerdesAutoTune dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
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

XP_STATUS cpssHalNoMgrSerdesLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t *status)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U8 i;
    GT_U32 cpssPortNum;
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT modePtr;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    xpMacConfigMode macConfigMode;
    cpssRet = cpssHalMacConfigModeGet(devId, portNum, &macConfigMode);

    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalMacConfigModeGet  dev %d port %d failed(%d)", cpssDevNum, cpssPortNum,
              cpssRet);
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    switch (macConfigMode)
    {
        case MAC_MODE_1GB:
        case MAC_MODE_100M:
        case MAC_MODE_10M:
        case MAC_MODE_1X10GB:
        case MAC_MODE_4X1GB:
        case MAC_MODE_4X10GB:
        case MAC_MODE_2X50GB:


            cpssRet = cpssDxChPortSerdesLoopbackModeGet(cpssDevNum, cpssPortNum,
                                                        0 /* Lane0 */, &modePtr);
            if (cpssRet != GT_OK)
            {
                ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPortSerdesLoopbackModeGet dev %d port %d failed(%d)", cpssDevNum,
                      cpssPortNum, cpssRet);
                return ret;
            }
            else
            {
                if (modePtr == CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E)
                {
                    *status = 0;
                }
                else if (modePtr == CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)
                {
                    *status = 1;
                }
            }
            break;
        default:
            *status = 0;
            for (i=0; i<CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS; i++)
            {
                cpssRet = cpssDxChPortSerdesLoopbackModeGet(cpssDevNum, cpssPortNum, i,
                                                            &modePtr);
                if (cpssRet != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPortSerdesLoopbackModeGet dev %d port %d failed(%d)", cpssDevNum,
                          cpssPortNum, cpssRet);
                    ret = xpsConvertCpssStatusToXPStatus(cpssRet);
                    return ret;
                }
                if (modePtr == CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E)
                {
                    return ret;
                }
            }
            *status = 1;
    }
    return ret;
}

XP_STATUS cpssHalNoMgrSerdesLoopback(xpsDevice_t devId, uint32_t portNum,
                                     uint8_t enable)
{

    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevNum;
    GT_U32 cpssPortNum;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (enable==1)
    {
        cpssRet = cpssDxChPortSerdesLoopbackModeSet(cpssDevNum, cpssPortNum,
                                                    0 /* Lane0 */, CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortSerdesLoopbackModeSet  dev %d port %d failed(%d)", cpssDevNum,
                  cpssPortNum, cpssRet);
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            return ret;
        }
    }
    else
    {
        cpssRet = cpssDxChPortSerdesLoopbackModeSet(cpssDevNum, cpssPortNum,
                                                    0 /* Lane0 */, CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortSerdesLoopbackModeSet dev %d port %d failed(%d)", cpssDevNum,
                  cpssPortNum, cpssRet);
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            return ret;
        }
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacNoMgrGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                          uint16_t *portAdvertFdAbility)
{
    return XP_ERR_NOT_SUPPORTED;
}

XP_STATUS cpssHalMacNoMgrGetPortSpeed(xpsDevice_t devId, uint32_t portNum,
                                      xpSpeed *speed)
{
    return XP_ERR_NOT_SUPPORTED;
}

XP_STATUS cpssHalMacNoMgrPortDeInit(xpsDevice_t devId, uint32_t portNum)
{
    return XP_ERR_NOT_SUPPORTED;
}

XP_STATUS cpssHalMacNoMgrPortInit(xpsDevice_t devId, uint32_t portNum,
                                  xpMacConfigMode macConfigMode, xpFecMode fecMode, uint8_t enableFEC)
{
    return XP_ERR_NOT_SUPPORTED;
}

XP_STATUS cpssHalPortNoMgrAutoNegEnable(xpsDevice_t devId, uint32_t portNum,
                                        uint32_t portANAbility, uint16_t portANCtrl)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_U32 currMode = 0;
    CPSS_PM_PORT_PARAMS_STC cpssPortParamsStcPtr;
    CPSS_PORT_MANAGER_STC cpssPortEventsStc;

    CPSS_PORT_MANAGER_PORT_TYPE_ENT cpssPortType;
    CPSS_PORT_MANAGER_STATUS_STC portStagePtr;

    GT_BOOL portAdminDisable;

    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

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

    cpssOsPrintf("cpssHalPortMgrAutoNegEnable : Enabling AN for port %d with AN ability 0x%x and portANCtrl 0x%x\n",
                 portNum, portANAbility, portANCtrl);
    cpssHalDeletPort(cpssDevId, cpssPortNum, 3);

    cpssPortType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
    cpssRet = cpssDxChPortManagerPortParamsStructInit(cpssPortType,
                                                      &cpssPortParamsStcPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    if (portANAbility&
        (IEEE_TECH_ABILITY_10GBASE_KR_BIT|IEEE_TECH_ABILITY_10GBASE_KX4_BIT))
    {

        cpssOsPrintf("inside CPSS_PORT_SPEED_10000_E with prevSpeed %d \n");
        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].ifMode =
            CPSS_PORT_INTERFACE_MODE_KR_E;
        cpssPortParamsStcPtr.portParamsType.apPort.modesArr[currMode].speed =
            CPSS_PORT_SPEED_10000_E;
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

         */
    }
    cpssPortParamsStcPtr.portParamsType.apPort.numOfModes=currMode;
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
    cpssPortEventsStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
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
    if (portAdminDisable)
    {
        cpssRet = cpssHalMacPortEnable(devId, portNum, 1);
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
    return XP_NO_ERR;
}
XP_STATUS cpssHalPortNoMgrAutoNegDisable(xpsDevice_t devId, uint32_t portNum)
{
    return XP_ERR_NOT_SUPPORTED;
}
