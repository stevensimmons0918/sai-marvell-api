// xpsAllocator.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAllocator.h"
#include "xpsCommon.h"
#include "xpsAllocatorMgr.h"
#include "xpsSal.h"
#ifdef XP_HEADERS
#include "xpAllocatorMgr.h"
#include "xpLogModXps.h"
#include "xpSal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsAllocatorInitIdAllocator(xpsScope_t scopeId, uint16_t allocatorId,
                                      uint32_t maxIds, uint32_t rangeStart)
{
    XP_STATUS status;

    XPS_FUNC_ENTRY_LOG();

    status = (xpsAllocatorMgr::instance(scopeId)->initAllocator(allocatorId, maxIds,
                                                                rangeStart));

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsAllocatorAllocateId(xpsScope_t scopeId, uint16_t allocatorId,
                                 uint32_t* id)
{
    XP_STATUS status = XP_NO_ERR;

    XPS_FUNC_ENTRY_LOG();

    if (id == NULL)
    {
        status = XP_ERR_NULL_POINTER;
    }
    else
    {
        status = xpsAllocatorMgr::instance(scopeId)->allocateId(allocatorId, *id);
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsAllocatorAllocateWithId(xpsScope_t scopeId, uint16_t allocatorId,
                                     uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;

    XPS_FUNC_ENTRY_LOG();

    status = xpsAllocatorMgr::instance(scopeId)->allocateWithId(allocatorId, id);

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsAllocatorAllocateIdGroup(xpsScope_t scopeId, uint16_t allocatorId,
                                      uint32_t size,
                                      uint32_t* id)
{
    XP_STATUS status = XP_NO_ERR;

    XPS_FUNC_ENTRY_LOG();

    if (id == NULL)
    {
        status = XP_ERR_NULL_POINTER;
    }
    else
    {
        status = xpsAllocatorMgr::instance(scopeId)->allocateIdGroup(allocatorId, size,
                                                                     *id);
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsAllocatorReleaseId(xpsScope_t scopeId, uint16_t allocatorId,
                                uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;

    XPS_FUNC_ENTRY_LOG();

    status = (xpsAllocatorMgr::instance(scopeId)->releaseId(allocatorId, id));

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsAllocatorReleaseIdGroup(xpsScope_t scopeId, uint16_t allocatorId,
                                     uint32_t size,
                                     uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;

    XPS_FUNC_ENTRY_LOG();

    status = (xpsAllocatorMgr::instance(scopeId)->releaseIdGroup(allocatorId, size,
                                                                 id));

    XPS_FUNC_EXIT_LOG();

    return status;
}
void* xpsAllocatorIdBasedPersistentMalloc(xpsScope_t scopeId, uint16_t poolId,
                                          size_t size)
{
    uint64_t uniqueObjId = XP_ALLOCATOR_OID(scopeId, (uint64_t)poolId);
    void * ptr = NULL;

    XPS_FUNC_ENTRY_LOG();

    if (poolId < XP_USR_DEFINED_MEM_POOL_OID_END)
    {
        ptr = XP_ID_BASED_PERSISTENT_MALLOC(uniqueObjId, size);
    }

    XPS_FUNC_EXIT_LOG();

    return ptr;

}

XP_STATUS xpsAllocatorIdBasedPersistentFree(void* ptr)
{
    XP_STATUS status;

    XPS_FUNC_ENTRY_LOG();

    status = XP_ID_BASED_PERSISTENT_FREE(ptr);

    XPS_FUNC_EXIT_LOG();

    return status;
}

#ifdef __cplusplus
}
#endif
