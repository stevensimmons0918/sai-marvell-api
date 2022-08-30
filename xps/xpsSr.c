// xpsSr.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsSr.h"
#include "xpsLock.h"
#include "xpsScope.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief State manager Database Handle
 *
 */


#define XPS_INVALID_INSPTR 0xFFFFFFFF

/**
 * \brief State structure maintained by XPS SR
 *
 * This state contains the relationship between a SR interface
 * and the segments  and NH-ID that are contained by it.
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpsSrDbEntry_t
{
    uint32_t            numOfSegments;
    ipv6Addr_t          segment[XP_MAX_SEGMENTS];
    uint32_t            ins0;
    uint32_t            ins1;
    uint32_t            ins2;
    uint32_t            ins3;
    int32_t             nhId;
    xpVif_t             srVifId;
} xpsSrDbEntry_t;

/**
 * \brief SR State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */





XP_STATUS xpsSrGetData(xpsDevice_t devId, xpsInterfaceId_t srIntfId,
                       xpsSrhData_t *srhData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsSrInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrAddDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrCreateInterface(xpsDevice_t devId, xpsInterfaceId_t *srIntfId,
                               xpsSrhData_t srhData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrInterfaceDelete(xpsDevice_t devId, xpsInterfaceId_t srIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrOriginationEntry(xpsDevice_t devId, xpsInterfaceId_t srIntId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrTerminationEntry(xpsDevice_t devId, xpsInterfaceId_t srIntId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrSetNextHopData(xpsDevice_t devId, xpsInterfaceId_t srIntId,
                              uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSrSetLocalSid(xpsDevice_t devId, xpsSidEntry sidEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
