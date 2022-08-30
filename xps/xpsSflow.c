// xpsSflow.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsSflow.h"
#include "xpsLock.h"
#include "xpsScope.h"
#include "xpsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsSflowInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsSflowInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowSetPortSamplingConfig(xpsInterfaceId_t portIntfId,
                                        xpAcmClient_e client,
                                        uint32_t nSample, uint32_t mBase, uint32_t mExpo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowSetPortSamplingConfigScope(xpsScope_t scopeId,
                                             xpsInterfaceId_t portIntfId, xpAcmClient_e client,
                                             uint32_t nSample, uint32_t mBase, uint32_t mExpo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowGetPortSamplingConfig(xpsInterfaceId_t portIntfId,
                                        xpAcmClient_e client, uint32_t *nSample, uint32_t *mBase, uint32_t *mExpo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowGetPortSamplingConfigScope(xpsScope_t scopeId,
                                             xpsInterfaceId_t portIntfId, xpAcmClient_e client, uint32_t *nSample,
                                             uint32_t *mBase, uint32_t *mExpo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowEnablePortSampling(xpsInterfaceId_t portIntfId,
                                     uint16_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowEnablePortSamplingScope(xpsScope_t scopeId,
                                          xpsInterfaceId_t portIntfId, uint16_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowSetEnable(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowSetPktCmd(xpsDevice_t devId, xpPktCmd_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowGetPktCmd(xpsDevice_t devId, xpPktCmd_e *pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowSetIntfId(xpsDevice_t devId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowGetIntfId(xpsDevice_t devId, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowCreate(uint32_t* xpsSflowId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowCreateScope(xpsScope_t scopeId, uint32_t* xpsSflowId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowDestroy(uint32_t xpsSflowId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSflowDestroyScope(xpsScope_t scopeId, uint32_t xpsSflowId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


#ifdef __cplusplus
}
#endif

