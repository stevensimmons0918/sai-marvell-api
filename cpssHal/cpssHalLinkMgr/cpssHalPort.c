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
#include "cpssHalPortMgr.h"
#include "cpssHalPortNoMgr.h"
#include "cpss/common/cpssTypes.h"
#include "cpssHalDevice.h"
#include "xpsMac.h"
#include "xpsLink.h"
#include "xpsUmac.h"
#include "openXpsEnums.h"
#include "cpssHalInit.h"
#include "cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h"
#include "cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h"


static int cpssHaMacPortANInfo[MAX_PORTNUM];

typedef struct
{
    int                                     is_valid;
    CPSS_PORT_FEC_MODE_ENT                  fecMode;
} CPSSHAL_PORT_REG_PORT_TX_PARAMS_STC;
static CPSSHAL_PORT_REG_PORT_TX_PARAMS_STC
cpssHalPortRegTxParamInfo[MAX_PORTNUM];

static int cpssHaMacPortCableInfo[MAX_PORTNUM] = {1};

XP_STATUS cpssHalGlobalSwitchControlGetCpuPhysicalPortNum(int deviceId,
                                                          uint32_t* portNum)
{

    GT_U8 cpssDevId = deviceId;

    switch (cpssHalDevPPFamilyGet(cpssDevId))
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            *portNum = 264;
            break;

        case CPSS_PP_FAMILY_DXCH_AC3X_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            *portNum = CPSS_CPU_PORT_NUM_CNS;
            break;

        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Using default CPU Port %d\n",
                  CPSS_CPU_PORT_NUM_CNS);
            *portNum = CPSS_CPU_PORT_NUM_CNS;
            break;
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacGetLinkStatus(xpsDevice_t devId, uint32_t portNum,
                                  uint8_t *status)
{
    GT_U8 cpssDevId;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrGetLinkStatus(devId, portNum, status);
    }
    else
    {
        return cpssHalMacNoMgrGetLinkStatus(devId, portNum, status);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacGetPortSpeed(xpsDevice_t devId, uint32_t portNum,
                                 xpSpeed *speed)
{

    return cpssHalMacMgrGetPortSpeed(devId, portNum, speed);
#if 0
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrGetPortSpeed(devId, portNum, speed);
    }
    else
    {
        return cpssHalMacNoMgrGetPortSpeed(devId, portNum, speed);
    }
#endif

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPortEnable(xpsDevice_t devId, uint32_t portNum,
                               uint8_t enable)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrPortEnable(devId, portNum, enable);
    }
    else
    {
        return cpssHalMacNoMgrPortEnable(devId, portNum, enable);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacFecModeSet(int devId, uint32_t portNum, xpFecMode fecMode)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrFecModeSet(devId, portNum, fecMode);
    }
    else
    {
        return cpssHalMacNoMgrFecModeSet(devId, portNum, fecMode);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacFecModeGet(xpDevice_t devId, uint32_t portNum,
                               xpFecMode *fecMode)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrFecModeGet(devId, portNum, fecMode);
    }
    else
    {
        return cpssHalMacNoMgrFecModeGet(devId, portNum, fecMode);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalPortPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                   uint32_t portNum, uint32_t* rxSerdesLaneArr, uint32_t* txSerdesLaneArr)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalPortMgrPlatformGetSerdesLaneSwapInfo(devId, portNum,
                                                           rxSerdesLaneArr, txSerdesLaneArr);
    }
    else
    {
        return cpssHalPortNoMgrPlatformGetSerdesLaneSwapInfo(devId, portNum,
                                                             rxSerdesLaneArr, txSerdesLaneArr);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacConfigModeGet(int devId, uint32_t portNum,
                                  xpMacConfigMode* macConfigMode)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrConfigModeGet(devId, portNum, macConfigMode);
    }
    else
    {
        return cpssHalMacNoMgrConfigModeGet(devId, portNum, macConfigMode);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacGetPortStatus(xpsDevice_t devId, uint32_t portNum,
                                  bool *linkStatus, bool *faultStatus, bool *serdesStatus)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrGetPortStatus(devId, portNum, linkStatus, faultStatus,
                                          serdesStatus);
    }
    else
    {
        return cpssHalMacNoMgrGetPortStatus(devId, portNum, linkStatus, faultStatus,
                                            serdesStatus);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPortInitWithLinkStatusControl(xpDevice_t devId,
                                                  uint32_t portNum, xpMacConfigMode macConfigMode,
                                                  CPSS_PORT_INTERFACE_MODE_ENT cpssIntfType,
                                                  bool initSerdes, bool prbsTestMode,
                                                  bool firmwareUpload, xpFecMode fecMode,
                                                  uint8_t enableFEC, uint8_t keepPortDown,
                                                  uint8_t initPort)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrPortInitWithLinkStatusControl(devId, portNum, macConfigMode,
                                                          cpssIntfType,
                                                          initSerdes, prbsTestMode,
                                                          firmwareUpload, fecMode,
                                                          enableFEC, keepPortDown, initPort);
    }
    else
    {
        return cpssHalMacNoMgrPortInitWithLinkStatusControl(devId, portNum,
                                                            macConfigMode, cpssIntfType, initSerdes,
                                                            prbsTestMode, firmwareUpload,
                                                            fecMode, enableFEC,
                                                            keepPortDown);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                xpMacLoopbackLevel loopback,  uint8_t *status)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrLoopbackGet(devId, portNum, loopback, status);
    }
    else
    {
        return cpssHalMacNoMgrLoopbackGet(devId, portNum, loopback, status);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacLoopback(xpsDevice_t devId, uint32_t portNum,
                             xpMacLoopbackLevel loopback, uint8_t enable)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrLoopback(devId, portNum, loopback, enable);
    }
    else
    {
        return cpssHalMacNoMgrLoopback(devId, portNum, loopback, enable);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalPortGetMru(xpsDevice_t devId, uint32_t portNum,
                            uint16_t *frameSize)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;

    GT_U32 cpssFrameSize;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    if ((cpssRet = cpssDxChPortMruGet(cpssDevId, cpssPortNum,
                                      &cpssFrameSize)!= GT_OK))
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortMruGet  dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }

    *frameSize = (uint16_t)cpssFrameSize;
    return XP_NO_ERR;

}

XP_STATUS cpssHalPortSetMru(xpsDevice_t devId, uint32_t portNum,
                            uint16_t frameSize)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;

    GT_U32 cpssFrameSize;
    cpssFrameSize=(uint32_t)frameSize;
    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPortMruSet(cpssDevId, cpssPortNum, cpssFrameSize);

    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortMruSet dev %d port %d failed(%d)", cpssDevId, cpssPortNum,
              cpssRet);
        return ret;
    }
    return ret;

}

XP_STATUS cpssHalPortIsPortNumValid(int devId, uint32_t portNum)
{
    int devNum;
    int lPort;

    return cpssHalG2LPortNum(portNum, &devNum, &lPort);

}

XP_STATUS cpssHalPortGlobalSwitchControlGetMacMaxChannelNumber(int devId,
                                                               uint8_t* macMaxChanNum)
{
    XP_DEV_TYPE_T devType;

    cpssHalGetDeviceType((xpsDevice_t)devId, &devType);

    if (IS_DEVICE_AC3X(devType) || devType == ALDRIN2XL || IS_DEVICE_AC5X(devType) \
            || IS_DEVICE_FUJITSU_LARGE(devType) || IS_DEVICE_FUJITSU_SMALL(devType))
    {
        *macMaxChanNum = 1;
    }
    else if (IS_DEVICE_FALCON_12_8(devType))
    {
        *macMaxChanNum = 8;
    }
    else if (IS_DEVICE_FALCON_6_4(devType))
    {
        *macMaxChanNum = 8;
    }
    else if (IS_DEVICE_FALCON_3_2(devType))
    {
        *macMaxChanNum = 8;
    }
    else if (IS_DEVICE_FALCON_2(devType))
    {
        *macMaxChanNum = 8;
    }
    else if (IS_DEVICE_GEMINI(devType))
    {
        *macMaxChanNum = 8;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPortSerdesTune(int devId, xpsPort_t *portList,
                                   uint32_t numOfPort, xpSerdesDfeTuneMode_t tuneMode, uint8_t force)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portList[0]);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrPortSerdesTune(devId, portList, numOfPort, tuneMode, force);
    }
    else
    {
        return cpssHalMacNoMgrPortSerdesTune(devId, portList, numOfPort, tuneMode,
                                             force);
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalMacPortSerdesIsDfeRunning(xpsDevice_t devId, xpsPort_t portNum,
                                           uint8_t *isDfeRunning)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    // CPSS_DXCH_PORT_SERDES_TUNE_STC tuneValuesPtr;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatusPtr;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatusPtr;

    cpssRet = cpssDxChPortSerdesAutoTuneStatusGet(cpssDevId, cpssPortNum,
                                                  &rxTuneStatusPtr, &txTuneStatusPtr);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortSerdesAutoTuneStatusGet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        return ret;
    }

    if (rxTuneStatusPtr==CPSS_DXCH_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
    {
        *isDfeRunning=1;
    }
    else
    {
        *isDfeRunning=0;
    }
    return ret;
}

XP_STATUS cpssHalMacPortSerdesIsDfeRunningDebug(xpsDevice_t devId,
                                                xpsPort_t portNum, uint8_t *rxTuneStatusPtr, uint8_t *txTuneStatusPtr)
{

    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    // CPSS_DXCH_PORT_SERDES_TUNE_STC tuneValuesPtr;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatusPtr_1;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatusPtr_1;

    cpssRet = cpssDxChPortSerdesAutoTuneStatusGet(cpssDevId, cpssPortNum,
                                                  &rxTuneStatusPtr_1, &txTuneStatusPtr_1);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortSerdesAutoTuneStatusGet dev %d port %d failed(%d)", cpssDevId,
              cpssPortNum, cpssRet);
        return ret;
    }
    *rxTuneStatusPtr=(uint8_t)rxTuneStatusPtr_1;
    *txTuneStatusPtr=(uint8_t)txTuneStatusPtr_1;

    return ret;

}


XP_STATUS cpssHalMacPortSerdesTxTune(int devId, xpsPort_t *portList,
                                     uint32_t numOfPort, uint8_t force)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portList[0]);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrPortSerdesTxTune(devId, portList, numOfPort, force);
    }
    else
    {
        return cpssHalMacNoMgrPortSerdesTxTune(devId, portList, numOfPort, force);
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalSerdesLoopbackGet(xpsDevice_t devId, uint32_t portNum,
                                   uint8_t *status)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMgrSerdesLoopbackGet(devId, portNum, status);
    }
    else
    {
        return cpssHalNoMgrSerdesLoopbackGet(devId, portNum, status);
    }
}

XP_STATUS cpssHalSerdesLoopback(xpsDevice_t devId, uint32_t portNum,
                                uint8_t enable)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMgrSerdesLoopback(devId, portNum, enable);
    }
    else
    {
        return cpssHalNoMgrSerdesLoopback(devId, portNum, enable);
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPortDeInit(xpsDevice_t devId, uint32_t portNum)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrPortDeInit(devId, portNum);
    }
    else
    {
        return cpssHalMacNoMgrPortDeInit(devId, portNum);
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPortInit(xpsDevice_t devId, uint32_t portNum,
                             xpMacConfigMode macConfigMode, xpFecMode fecMode, uint8_t enableFEC)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrPortInit(devId, portNum, macConfigMode, fecMode, enableFEC);
    }
    else
    {
        return cpssHalMacNoMgrPortInit(devId, portNum, macConfigMode, fecMode,
                                       enableFEC);
    }
    return XP_NO_ERR;

}

XP_STATUS cpssHalPortAutoNegEnable(xpsDevice_t devId, uint32_t portNum,
                                   uint32_t portANAbility, uint16_t portANCtrl, uint8_t keepPortDown)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalPortMgrAutoNegEnable(devId, portNum, portANAbility,
                                           portANCtrl, keepPortDown);
    }
    else
    {
        return cpssHalPortNoMgrAutoNegEnable(devId, portNum, portANAbility, portANCtrl);
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalPortAutoNegDisable(xpsDevice_t devId, uint32_t portNum)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalPortMgrAutoNegDisable(devId, portNum);
    }
    else
    {
        return cpssHalPortNoMgrAutoNegDisable(devId, portNum);
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalPortMirrorEnable(xpsDevice_t devId, uint32_t portNum,
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

XP_STATUS cpssHalPortMacSaBaseSet(int devId, GT_ETHERADDR *mac)
{
    GT_STATUS status = GT_OK;
    XP_STATUS ret = XP_NO_ERR;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChPortMacSaBaseSet(devNum, mac);
        if (status!= GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(status);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add MAC SA Base %d \n", status);
            return ret;
        }
    }
    return ret;
}

XP_STATUS cpssHalPortMacSaLsbSet(xpsDevice_t devId, uint32_t portNum,
                                 uint8_t macLsb)
{
    GT_STATUS status = GT_OK;
    XP_STATUS ret = XP_NO_ERR;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChPortMacSaLsbSet(cpssDevId, cpssPortNum, macLsb);
    if (status!= GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(status);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add MAC SA LSB %d \n", status);
        return ret;
    }
    return ret;
}

int cpssHalPortMacAnEnableGet(xpsDevice_t devId, uint32_t portNum)
{

    return cpssHaMacPortANInfo[portNum];

}

void cpssHalPortMacAnEnable(xpsDevice_t devId, uint32_t portNum, int enable)
{

    if (enable == 1)
    {
        cpssHaMacPortANInfo[portNum] = 1;
    }
    else
    {
        cpssHaMacPortANInfo[portNum] = 0;
    }

}

CPSS_PORT_FEC_MODE_ENT cpssHalPortFecParamGet(xpsDevice_t devId,
                                              uint32_t portNum)
{
    if (!cpssHalIsEbofDevice(devId))
    {
        return CPSS_PORT_FEC_MODE_LAST_E;
    }

    if (cpssHalPortRegTxParamInfo[portNum].is_valid)
    {
        return cpssHalPortRegTxParamInfo[portNum].fecMode;
    }
    else
    {
        return CPSS_PORT_FEC_MODE_LAST_E;
    }
}

void cpssHalPortFecParamSet(xpsDevice_t devId, uint32_t portNum,
                            xpFecMode fecMode, bool valid)
{
    CPSS_PORT_FEC_MODE_ENT cpssFec = CPSS_PORT_FEC_MODE_LAST_E;
    if (valid)
    {
        switch (fecMode)
        {
            case RS_FEC_MODE:
                cpssFec = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                break;
            case FC_FEC_MODE:
                cpssFec = CPSS_PORT_FEC_MODE_ENABLED_E;
                break;
            case RS_544_514_FEC_MODE:
                cpssFec = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case MAX_FEC_MODE:
                cpssFec = CPSS_PORT_FEC_MODE_DISABLED_E;
                break;
            default:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalPortFecParamSet dev %d port %d", devId, portNum);
        }
    }
    cpssHalPortRegTxParamInfo[portNum].fecMode = cpssFec;
    cpssHalPortRegTxParamInfo[portNum].is_valid = valid;
}

void cpssHalPortMacCableLenSet(xpsDevice_t devId, uint32_t portNum,
                               int cableType)
{
    if (cableType<=3 && cableType>=0)
    {
        cpssHaMacPortCableInfo[portNum] = cableType;
    }
    else
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set cable type info for  %d \n", portNum);

}

uint32_t cpssHalMacGetPortCableLen(xpsDevice_t devId, uint32_t portNum)
{
    return cpssHaMacPortCableInfo[portNum];
}

XP_STATUS cpssHalMacGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                     uint32_t *portAdvertFdAbility)
{
    GT_U8 cpssDevId;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);

    if (GT_OK == cpssHalPortManagerIsSupported(cpssDevId))
    {
        return cpssHalMacMgrGetPortFdAbility(devId, portNum, portAdvertFdAbility);
    }
    else
    {
        return cpssHalMacNoMgrGetPortFdAbility(devId, portNum, portAdvertFdAbility);
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalMacPortANLtRemoteAbilityGet(xpDevice_t devId, uint32_t port,
                                             uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl)
{
    XP_STATUS ret = XP_NO_ERR;

    *portRemoteANCtrl = UMAC_BPAN_AN_CTRL_FEC_MODE_RS;
    return ret;

}


XP_STATUS cpssHalPortMirrorGet(xpsDevice_t devId, uint32_t portNum,
                               bool ingress, uint32_t *analyzerId)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_BOOL enable;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    if (ingress)
    {
        cpssRet = cpssDxChMirrorRxPortGet(cpssDevId, cpssPortNum, true, &enable,
                                          analyzerId);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get ingress mirror on port %d with error %d", cpssPortNum, cpssRet);
            return ret;
        }
    }
    else
    {
        cpssRet = cpssDxChMirrorTxPortGet(cpssDevId, cpssPortNum, true, &enable,
                                          analyzerId);
        if (cpssRet != GT_OK)
        {
            ret = xpsConvertCpssStatusToXPStatus(cpssRet);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get egress mirror on port %d with error %d", cpssPortNum, cpssRet);
            return ret;
        }
    }

    return ret;
}


XP_STATUS cpssHalGetPortOperSpeed(xpsDevice_t devId, uint32_t portNum,
                                  xpSpeed *speed)
{
    XPS_LOCK(xpsMacGetPortOperSpeed);
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    CPSS_PORT_SPEED_ENT cpssSpeed = CPSS_PORT_SPEED_NA_E;
    GT_STATUS status = GT_OK;

    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChPortSpeedGet(cpssDevId,  cpssPortNum, &cpssSpeed);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortSpeedGet failed rc %d port %d\n", status,  cpssPortNum);
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
