// xpsXpImports.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsXpImports.h"
#include "xpsInterface.h"
#include "cpssHalDevice.h"
#include <cpssHalExt.h>
#include "cpssCommon/cpssPresteraDefs.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


XP_STATUS xpLinkManagerGetSerdesIdsForPort(xpDevice_t devId, uint32_t portNum,
                                           xpMacConfigMode macConfigMode, int16_t *portSerdesArrIds)
{

    return XP_NO_ERR;
}

XP_STATUS xpLinkManagerPlatformGetSerdesLaneSwapInfo(xpDevice_t devId,
                                                     uint32_t portNum, uint8_t channelNum, uint8_t *channelSwapNum)
{

    *channelSwapNum = channelNum;
    return XP_NO_ERR;
}

xpSalType_t xpGetSalType()
{

    return XP_SAL_HW_TYPE;
}

XP_STATUS xpSalGetDeviceType(xpDevice_t devId, XP_DEV_TYPE_T *devType)
{

    cpssHalGetDeviceType(devId, devType);
    return XP_NO_ERR;
}

void xpSetSalType(xpsSalType_e salType)
{
    return;
}

int xpSalInit(XP_DEV_TYPE_T devType)
{
    return XP_NO_ERR ;
}


XP_STATUS xpSalThreadCreateName(void *thread, const char *name,
                                const void *attr, void *(*start_routine)(void *), void *arg)
{
    GT_STATUS rc;
    CPSS_TASK*  tid= (CPSS_TASK *)thread;
    //void* (*)(void*)' to 'unsigned int (*)(void*)
    unsigned(*start_addr)(void*) = (unsigned(*)(void*))start_routine;
    if (NULL == start_routine)
    {
        return XP_ERR_NULL_POINTER;
    }

    rc = cpssOsTaskCreate(name,                 /* Task Name      */
                          200,                                /* Task Priority  */
                          _32K,                               /* Stack Size     */
                          start_addr,                         /* Starting Point */
                          arg,                                /* Arguments list */
                          tid);                               /* task ID        */

    return (XP_STATUS)rc;
}
XP_STATUS xpSalThreadDelete(void *thread)
{
    GT_STATUS rc;
    CPSS_TASK  tid = *((CPSS_TASK *)thread);
    rc = cpssOsTaskDelete(tid);
    return (XP_STATUS)rc;
}

XP_STATUS xpIpcRecvDebugMessage(xpDevice_t devId, char *data, uint16_t *size)
{
    return XP_NO_ERR;
}

XP_STATUS xpGlobalSwitchControlGetMcpuPortNumber(xpDevice_t devId,
                                                 uint32_t* mcpuPortNum)
{
    return XP_NO_ERR;
}
XP_STATUS xpLinkManagerCounterStatsDirectGet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat)
{
    return XP_NO_ERR;
}
