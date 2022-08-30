// xpsLink.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include "xpsLink.h"
#include "xpsPort.h"
#include "xpsMac.h"
#include "xpsCommon.h"
#include "xpsInternal.h"
#include "cpssHalPort.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

XP_STATUS xpsLinkInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return xpsLinkInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsLinkInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS ret = XP_NO_ERR;
    ret = xpsPortDbInitScope(scopeId);

    XPS_FUNC_EXIT_LOG();
    return ret;
}

XP_STATUS xpsLinkDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return xpsLinkDeInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsLinkDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS ret = XP_NO_ERR;
    ret = xpsPortDbDeInitScope(scopeId);

    XPS_FUNC_EXIT_LOG();
    return ret;
}

//TODO: Remove linkUpIntrRec and linkDownIntrRec Apis after testing of interrupts
void linkUpIntrRec(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("**********************************\n");
    cpssOsPrintf("Link up interrupt received on port: %d, devId: %d\n", portNum,
                 devId);
    cpssOsPrintf("**********************************\n");

}

void linkDownIntrRec(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("**********************************\n");
    cpssOsPrintf("Link down interrupt received on port: %d, devId: %d\n", portNum,
                 devId);
    cpssOsPrintf("**********************************\n");

}

void faultIntrRec(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("**********************************\n");
    cpssOsPrintf("Link is Faulty. Interrupt received on port: %d, devId: %d\n",
                 portNum,
                 devId);
    cpssOsPrintf("**********************************\n");

}

void faultFreeIntrRec(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("**********************************\n");
    cpssOsPrintf("Link is Fault free. Interrupt received on port: %d, devId: %d\n",
                 portNum, devId);
    cpssOsPrintf("**********************************\n");

}

void serdesSigIntrRec(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("**********************************\n");
    cpssOsPrintf("serdes signal is OK. Interrupt received on port: %d, devId: %d\n",
                 portNum, devId);
    cpssOsPrintf("**********************************\n");
}

void serdesSigLostIntr(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("**********************************\n");
    cpssOsPrintf("serdes signal is not present on Link. Interrupt received on port: %d, devId: %d\n",
                 portNum, devId);
    cpssOsPrintf("**********************************\n");
}

void rxLocalFaultIntr(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("\n**********************************\n");
    cpssOsPrintf("Rx Local Fault Interrupt received on port: %d, devId: %d\n",
                 portNum, devId);
    cpssOsPrintf("**********************************\n");
}

void rxRemoteFaultIntr(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("\n**********************************\n");
    cpssOsPrintf("Rx Remote Fault Interrupt received on port: %d, devId: %d\n",
                 portNum, devId);
    cpssOsPrintf("**********************************\n");
}

void pcsDecoderTrapIntr(xpDevice_t devId, uint32_t portNum)
{
    return;
}

void timestampAvailIntr(xpDevice_t devId, uint32_t portNum)
{
    cpssOsPrintf("\n**********************************\n");
    cpssOsPrintf("TimeStamp Available Interrupt Reported on Port: %d, devId: %d\n",
                 portNum, devId);
    cpssOsPrintf("**********************************\n");
}

XP_STATUS xpsLinkAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    xpsPortCreateDbForDevice(devId);
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkGetFaultLiveStatus(xpDevice_t devId, uint32_t portNum,
                                    xpMacRxFault *fault)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortANOrAutoTunePerform(xpDevice_t devId,
                                                xpsPortList_t *portsList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortANLtEnable(xpDevice_t devId, uint32_t port,
                                       uint32_t portANAbility, uint16_t portANCtrl, uint8_t ieeeMode25G,
                                       uint8_t keepPortDown)
{
    cpssOsPrintf("inside xpsLinkManagerPortANLtEnable portANAbility %x \n ",
                 portANAbility);
    return cpssHalPortAutoNegEnable(devId, port, portANAbility, portANCtrl,
                                    keepPortDown);
}

XP_STATUS xpsLinkManagerPortANLtDisable(xpDevice_t devId, uint32_t port)
{
    return cpssHalPortAutoNegDisable(devId, port);
}

XP_STATUS xpsLinkManagerPortANStatusCheck(xpDevice_t devId, uint32_t port)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortANNextPageStatusCheck(xpDevice_t devId,
                                                  uint32_t port)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortLtStatusCheck(xpDevice_t devId, uint32_t port)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortLinkStatusCheck(xpDevice_t devId, uint32_t port)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerCheckStatusAndTunePort(xpDevice_t devId,
                                               uint32_t portNum, uint8_t *portTuneStatus)
{
    XP_STATUS ret;
    if (portTuneStatus == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "recieved a null pointer");
        return XP_ERR_NULL_POINTER;
    }

    xpsPort_t portList[1];
    portList[0] = portNum;

    ret = xpsMacPortSerdesTune(devId, portList, 1, XP_DFE_ICAL, 0);
    if (ret==XP_NO_ERR)
    {
        *portTuneStatus=1;
    }
    return ret;
}

XP_STATUS xpsLinkManagerPortANLtStateGet(xpDevice_t devId, uint32_t portNum,
                                         xpPortANLtState *state)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortANLtRemoteAbilityGet(xpDevice_t devId,
                                                 uint32_t port,
                                                 uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl)
{

    return xpsMacPortANLtRemoteAbilityGet(devId, port, portRemoteANAbility,
                                          portRemoteANCtrl);

}

XP_STATUS xpsLinkManagerPortANLtStateMachineFunc(xpDevice_t devId,
                                                 uint32_t port)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortAutoTuneEnable(xpDevice_t devId, uint32_t portNum,
                                           uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerPortAutoTuneStatusGet(xpDevice_t devId,
                                              uint32_t portNum,
                                              uint8_t *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLinkManagerIsPortNumValid(xpDevice_t devId, uint32_t portNum)
{
    return cpssHalPortIsPortNumValid(devId, portNum);
}

#ifdef __cplusplus
}
#endif
