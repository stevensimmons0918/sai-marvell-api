// xpsAllocatorMgr.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAllocatorMgr.h"
#include "xpsSal.h"

static xpsGenLock *localLock;
xpsAllocatorMgr* xpsAllocatorMgr::sInstance[XP_MAX_SCOPES] = {NULL};

// Singleton instance method
xpsAllocatorMgr* xpsAllocatorMgr::instance(xpScope_t scopeId)
{

    if (!sInstance[scopeId])
    {
        XP_RW_FL_LOCK_WRITE_PROTECT_ORDERED(1, localLock);
        if (!sInstance[scopeId])
        {
            sInstance[scopeId] = new xpsAllocatorMgr(scopeId);
        }
    }

    return sInstance[scopeId];
}

void xpsAllocatorMgr::releaseScopeInstance(xpScope_t scopeId)
{
    if (sInstance[scopeId])
    {
        XP_RW_FL_LOCK_WRITE_PROTECT_ORDERED(1, localLock);
        if (sInstance[scopeId])
        {
            delete sInstance[scopeId];
            sInstance[scopeId] = NULL;
        }
    }

    return;
}





xpsAllocatorMgr::xpsAllocatorMgr(xpScope_t scopeId)
{
    int j;
    for (j = 0; j < XPS_ALLOCATOR_ENDS; j++)
    {
        xpsAllocatorIdVec.push_back(new xpsIdAllocator());
    }

    localLock = XP_SYS_LOCK_GET(0, XP_LOCKINDEX_FL_ALLOCATOR_MGR);



}


XP_STATUS xpsAllocatorMgr::initAllocator(uint16_t allocatorId, uint32_t maxIds,
                                         uint32_t rangeStart)
{
    XP_STATUS status = XP_NO_ERR;

    if (allocatorId > XPS_ALLOCATOR_ENDS)
    {
        return XP_ERR_INVALID_ID;
    }

    // Check if the allocator is already initialized
    if (xpsAllocatorIdVec[allocatorId]->getContext()->getMaxIds() != 0)
    {
        return XP_ERR_INIT_FAILED;
    }

    status = xpsAllocatorIdVec[allocatorId]->init(maxIds, 1);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    status = xpsAllocatorIdVec[allocatorId]->configureRange(this->instanceId, 0,
                                                            maxIds, rangeStart, allocatorId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}


XP_STATUS xpsAllocatorMgr::allocateId(uint16_t allocatorId, uint32_t& id)
{
    XP_STATUS status = XP_NO_ERR;

    if (allocatorId > XPS_ALLOCATOR_ENDS)
    {
        return XP_ERR_INVALID_ID;
    }

    // Check if the allocator is already initialized
    if (xpsAllocatorIdVec[allocatorId]->getContext()->getMaxIds() == 0)
    {
        return XP_ERR_NOT_INITED;
    }

    status = xpsAllocatorIdVec[allocatorId]->allocateId(id);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAllocatorMgr::allocateWithId(uint16_t allocatorId, uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;

    if (allocatorId > XPS_ALLOCATOR_ENDS)
    {
        return XP_ERR_INVALID_ID;
    }

    // Check if the allocator is already initialized
    if (xpsAllocatorIdVec[allocatorId]->getContext()->getMaxIds() == 0)
    {
        return XP_ERR_NOT_INITED;
    }

    status = xpsAllocatorIdVec[allocatorId]->allocateWithId(id);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAllocatorMgr::allocateIdGroup(uint16_t allocatorId, uint32_t size,
                                           uint32_t& id)
{
    XP_STATUS status = XP_NO_ERR;

    if (allocatorId > XPS_ALLOCATOR_ENDS)
    {
        return XP_ERR_INVALID_ID;
    }

    // Check if the allocator is already initialized
    if (xpsAllocatorIdVec[allocatorId]->getContext()->getMaxIds() == 0)
    {
        return XP_ERR_NOT_INITED;
    }

    status = xpsAllocatorIdVec[allocatorId]->allocateIdGroupFromRange(0, size, id);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAllocatorMgr::releaseId(uint16_t allocatorId, uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;

    if (allocatorId > XPS_ALLOCATOR_ENDS)
    {
        return XP_ERR_INVALID_ID;
    }

    // Check if the allocator is already initialized
    if (xpsAllocatorIdVec[allocatorId]->getContext()->getMaxIds() == 0)
    {
        return XP_ERR_NOT_INITED;
    }

    status = xpsAllocatorIdVec[allocatorId]->releaseId(id);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAllocatorMgr::releaseIdGroup(uint16_t allocatorId, uint32_t size,
                                          uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;

    if (allocatorId > XPS_ALLOCATOR_ENDS)
    {
        return XP_ERR_INVALID_ID;
    }

    // Check if the allocator is already initialized
    if (xpsAllocatorIdVec[allocatorId]->getContext()->getMaxIds() == 0)
    {
        return XP_ERR_NOT_INITED;
    }

    status = xpsAllocatorIdVec[allocatorId]->releaseIdGroupFromRange(0, size, id);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}
