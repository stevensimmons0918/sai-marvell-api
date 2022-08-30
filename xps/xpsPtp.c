// xpsPtp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsPtp.h"
#include "xpsInterface.h"
#include "xpsPort.h"
#include "xpsInit.h"
#include "xpsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsPtpInit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpDeInit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpFetchTimeStamp(xpsDevice_t devId, uint64_t *timeStamp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpLoadCurrentTime(xpsDevice_t devId, int64_t currentTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetCurrentTime(xpsDevice_t devId, uint64_t *currentTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpAdjustCurrentTime(xpsDevice_t devId, int64_t deltaTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetAdjustCurrentTime(xpsDevice_t devId, uint64_t *deltaTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpLoadClockInc(xpsDevice_t devId, uint32_t nsInc, uint32_t fnsInc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetClockInc(xpsDevice_t devId, uint32_t *nsInc,
                            uint32_t* fnsInc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpSetOutputClock(xpsDevice_t devId, uint32_t outputNum,
                               uint64_t startTime, int pulseWidth, uint32_t  nsInc, uint32_t fsnInc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetOutputClock(xpsDevice_t devId, uint64_t *startTime,
                               uint32_t* ns, uint32_t* fsn)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpSetSyncDir(xpsDevice_t devId, uint32_t pinNum,
                           uint32_t direction)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetSyncDir(xpsDevice_t devId, uint32_t pinNum,
                           uint32_t *direction)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpSetupPulseAction(xpsDevice_t devId, uint32_t inputNum,
                                 uint32_t action)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetPulseAction(xpsDevice_t devId, uint32_t inputNum,
                               uint32_t *action)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpSetCaptureTime(xpsDevice_t devId, uint64_t nsTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetPulseTime(xpsDevice_t devId, uint32_t inputNum,
                             uint64_t *arrivalTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPtpGetEgressTimeStamp(xpsDevice_t devId, uint32_t portNum,
                                   uint64_t *egressTs)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
