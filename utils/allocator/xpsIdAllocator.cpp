// xpsIdAllocator.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsIdAllocator.h"

xpsIdAllocator::xpsIdAllocator() : context(NULL)
{
    this->lockId = 0;
    XP_RW_LOCK_ALLOCATE(0u, lockId);
    this->xpsIdAllocatorLock = XP_RW_LOCK_CREATE(0u, lockId, 1);
    context = createContext();
}

xpsIdAllocator::xpsIdAllocator(uint32_t maxIds,
                               uint32_t numRanges) : context(NULL)
{
    this->lockId = 0;
    XP_RW_LOCK_ALLOCATE(0u, lockId);
    this->xpsIdAllocatorLock = XP_RW_LOCK_CREATE(0u, lockId, 1);

    context = createContext(maxIds, numRanges);
}

xpsIdAllocator::~xpsIdAllocator()
{
    XP_RW_LOCK_DESTROY(0u, lockId);
    lockId = 0;
    this->xpsIdAllocatorLock = NULL;

    if (context)
    {
        delete context;
        context = NULL;
    }
}

XP_STATUS xpsIdAllocator::init()
{
    XP_STATUS status;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        if ((status = context->init()) != XP_NO_ERR)
        {
            return status;
        }

        for (uint32_t i = 0; i < context->getNumRanges(); i++)
        {
            this->idRanges.push_back(xpsPersistBitVector(1));
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::init(uint32_t maxIds, uint32_t numRanges)
{
    XP_STATUS status;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        if ((status = context->init(maxIds, numRanges)) != XP_NO_ERR)
        {
            return status;
        }

        for (uint32_t i = 0; i < context->getNumRanges(); i++)
        {
            this->idRanges.push_back(xpsPersistBitVector(1));
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::setAllocatorIdsInRange(uint32_t range)
{
    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        if (range >= context->getNumRanges())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        this->idRanges[range].set();
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::dumpAllocator()
{
    {
        XP_RW_LOCK_READ_PROTECT(this->xpsIdAllocatorLock, 0);

        for (uint32_t i = 0; i < context->getNumRanges(); i++)
        {
            cout << this->idRanges[i] << endl;
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::configureRange(uint32_t instanceId, uint32_t range,
                                         uint32_t depth, uint32_t start, uint64_t unique_OID)
{
    XP_STATUS status;

    XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

    if (range >= context->getNumRanges())
    {
        return XP_ERR_OUT_OF_RANGE;
    }

    if (depth)
    {
        // Resize the range vector to match depth
        this->idRanges[range].resize(depth);
        this->idRanges[range].set();
        uint64_t oid = XP_ALLOCATOR_OID(instanceId, unique_OID);
        status = this->idRanges[range].persist(oid);
        if (status != XP_NO_ERR)
        {
            return status;
        }
    }
    context->setRangeStartOffset(range, start);
    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::getMaxIdForAllocator(uint32_t& maxId)
{
    maxId = this->idRanges[0].size();
    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::allocateId(uint32_t& id)
{
    uint32_t first;
    std::vector<uint32_t> *rangeStarts;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();

        if (this->idRanges[0].size() == 0)
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // This API is used only when there is one range
        // it will only allocate from range 0
        first = this->idRanges[0].find_first();
        if (first >= this->idRanges[0].size())
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // Allocate the id (clear the bit)
        this->idRanges[0][first] = 0;

        // Offset the first set bit by the range start value
        id = first + (*rangeStarts)[0];
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::allocateWithId(uint32_t id)
{
    std::vector<uint32_t> *rangeStarts;
    uint32_t first;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();
        first = (*rangeStarts)[0];

        if (id >= this->idRanges[0].size() + first)
        {
            return XP_ERR_INVALID_ARG;
        }

        if (id < first)
        {
            return XP_ERR_INVALID_ARG;
        }

        id -= first;

        // This API is used only when there is one range
        // it will only allocate from range 0
        if (!(this->idRanges[0][id]))
        {
            return XP_ERR_ID_IS_PRE_ALLOCATED;
        }

        this->idRanges[0][id] = 0;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::allocateIdFromRange(uint32_t range, uint32_t& id)
{
    uint32_t first;
    std::vector<uint32_t> *rangeStarts;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();

        if (range >= context->getNumRanges())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        if (this->idRanges[range].size() == 0)
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // Use a linear search to find the first free id
        // Amortized time O(n), with spatial locality of vector in
        // a cache, is on average faster than any search other
        // than constant
        //
        // Using linear search here because we want constant time
        // lookup for robust is allocated checking
        //
        // xpsBitVector provides this search functionality
        first = this->idRanges[range].find_first();
        if (first >= this->idRanges[range].size())
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // Allocate the id (clear the bit)
        this->idRanges[range][first] = 0;
    }

    // Offset the first set bit by the range start value
    id = first + (*rangeStarts)[range];
    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::allocateIdFromRangeFromLast(uint32_t range,
                                                      uint32_t& id)
{
    uint32_t first;
    std::vector<uint32_t> *rangeStarts;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();

        if (range >= context->getNumRanges())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        if (this->idRanges[range].size() == 0)
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // Use a linear search to find the first free id
        // Amortized time O(n), with spatial locality of vector in
        // a cache, is on average faster than any search other
        // than constant
        //
        // Using linear search here because we want constant time
        // lookup for robust is allocated checking
        //
        // xpsBitVector provides this search functionality
        first = this->idRanges[range].find_last();
        if (first >= this->idRanges[range].size())
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // Allocate the id (clear the bit)
        this->idRanges[range][first] = 0;
    }

    // Offset the first set bit by the range start value
    id = first + (*rangeStarts)[range];
    return XP_NO_ERR;
}


XP_STATUS xpsIdAllocator::allocateWithIdFromRange(uint32_t range, uint32_t id)
{
    XP_STATUS status;
    uint32_t start = 0;
    uint32_t offset = 0;
    uint32_t testRange = 0;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        // Validate range
        if (range >= context->getNumRanges())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        if ((status = getRangeFromId(id, testRange)) != XP_NO_ERR)
        {
            return status;
        }

        if (range != testRange)
        {
            return XP_ERR_INVALID_ID_RANGE;
        }

        // Determine start offset
        if ((status = getRangeStartOffset(range, start)) != XP_NO_ERR)
        {
            return status;
        }

        if (id >= this->idRanges[range].size() + start)
        {
            return XP_ERR_INVALID_ARG;
        }

        // get the offset
        offset = id - start;

        // Check if ID is already allocated
        if (!(this->idRanges[range][offset]))
        {
            return XP_ERR_ID_IS_PRE_ALLOCATED;
        }

        // Allocate the id
        this->idRanges[range][offset] = 0;
    }

    return XP_NO_ERR;

}

XP_STATUS xpsIdAllocator::allocateIdGroupFromRange(uint32_t range,
                                                   uint32_t size, uint32_t& id)
{
    uint32_t first, index, freeSize;
    std::vector<uint32_t> *rangeStarts;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();

        if (range >= context->getNumRanges())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        if (this->idRanges[range].size() == 0)
        {
            return XP_ERR_OUT_OF_IDS;
        }

        // use a temp algorithm for now.
        first = this->idRanges[range].find_first();

        if ((first >= this->idRanges[range].size()) ||
            ((first + size - 1) >= this->idRanges[range].size()))
        {
            return XP_ERR_OUT_OF_IDS;
        }

        do
        {
            freeSize = 0;
            for (index = 0; index < size; index++)
            {
                if (this->idRanges[range][first + index] == 0)
                {
                    first = first + index + 1;
                    break;
                }
                freeSize++;
            }
        }
        while ((freeSize != size) && ((first + size) < this->idRanges[range].size()));

        if (freeSize != size)
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        // Allocate this range of IDs (clear the bits)
        for (index = 0; index < size; index++)
        {
            this->idRanges[range][first+index] = 0;
        }
    }

    // Offset the first set bit by the range start value
    id = first + (*rangeStarts)[range];
    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::reserveIdBlockFromRange(uint32_t range,
                                                  uint32_t first, uint32_t size)
{
    XP_STATUS status;
    uint32_t start = 0;
    uint32_t offset = 0;
    uint32_t testRange = 0;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        // Validate range
        if (range >= context->getNumRanges())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        if ((status = getRangeFromId(first, testRange)) != XP_NO_ERR)
        {
            return status;
        }

        if (range != testRange)
        {
            return XP_ERR_INVALID_ID_RANGE;
        }

        // Determine start offset
        if ((status = getRangeStartOffset(range, start)) != XP_NO_ERR)
        {
            return status;
        }

        // Ensure that the block fits within the range
        offset = first - start;
        if ((offset + size - 1) >= this->idRanges[range].size())
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        // Ensure that IDs within the block specified by the caller are available
        // for allocation
        for (uint32_t i = 0; i < size; i++)
        {
            if (!(this->idRanges[range][offset + i]))
            {
                return XP_ERR_ID_IS_PRE_ALLOCATED;
            }
        }

        // Clear bits in this range to indicate that they are allocated
        this->idRanges[range](offset, offset + size - 1) = 0;
    }

    return XP_NO_ERR;
}


XP_STATUS xpsIdAllocator::releaseId(uint32_t id)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t range = 0;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        // Determine range from id passed in
        if ((status = getRangeFromId(id, range)) != XP_NO_ERR)
        {
            // log message required here
            return status;
        }

        status = releaseIdFromRange(range, id);
    }

    return status;
}

XP_STATUS xpsIdAllocator::releaseIdFromRange(uint32_t range, uint32_t id)
{
    uint32_t idx;
    std::vector<uint32_t> *rangeStarts;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();

        if ((range >= context->getNumRanges()) ||
            (id < (*rangeStarts)[range]))
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        // Retreive offset into range vector
        idx = id - (*rangeStarts)[range];
        if (idx >= this->idRanges[range].size())
        {
            return XP_ERR_INVALID_ID;
        }

        // Ensure that we aren't freeing already freed id
        if (this->idRanges[range][idx] == 1)
        {
            return XP_ERR_FREEING_UNALLOCATED_ID;
        }

        // Release the id back into the pool
        this->idRanges[range][idx] = 1;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::releaseIdGroupFromRange(uint32_t range, uint32_t size,
                                                  uint32_t id)
{
    uint32_t idx, index;
    std::vector<uint32_t> *rangeStarts;

    {
        XP_RW_LOCK_WRITE_PROTECT(this->xpsIdAllocatorLock, 0);

        rangeStarts = context->getRangeStarts();

        if ((range >= context->getNumRanges()) ||
            ((id + size - 1) < (*rangeStarts)[range]))
        {
            return XP_ERR_OUT_OF_RANGE;
        }

        // Retrieve start offset into range vector
        idx = id - (*rangeStarts)[range];
        if (idx >= this->idRanges[range].size())
        {
            return XP_ERR_INVALID_ID;
        }

        // Ensure that we aren't freeing already freed id
        for (index = 0; index < size; index++)
        {
            if (this->idRanges[range][idx + index] == 1)
            {
                return XP_ERR_FREEING_UNALLOCATED_ID;
            }
        }

        // Release the block of IDs back into the pool
        for (index = 0; index < size; index++)
        {
            this->idRanges[range][idx + index] = 1;
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocator::getRangeFromId(uint32_t id, uint32_t& range)
{
    return xpsIdAllocator::getRangeFromId(*context, id, range);
}

XP_STATUS xpsIdAllocator::getRangeStartOffset(uint32_t range, uint32_t& start)
{
    return xpsIdAllocator::getRangeStartOffset(*context, range, start);
}

XP_STATUS xpsIdAllocator::copyContext(xpsIdAllocatorCtx& newCtx)
{
    // Use assignment operator of context
    newCtx = *context;

    return XP_NO_ERR;
}

/*
 * Static routines with xpsIdAllocator Class Scope
 */

XP_STATUS xpsIdAllocator::getRangeFromId(xpsIdAllocatorCtx& ctx, uint32_t id,
                                         uint32_t& range)
{
    std::vector<uint32_t> *rangeStarts = ctx.getRangeStarts();

    for (uint32_t i = 0; i < ctx.getNumRanges() - 1; i++)
    {
        if (id >= (*rangeStarts)[i] &&
            id < (*rangeStarts)[i + 1])
        {
            range = i;
            return XP_NO_ERR;
        }
    }

    // Perform the last check
    if ((id >= (*rangeStarts)[ctx.getNumRanges() - 1]) &&
        (id < ((*rangeStarts)[0] + ctx.getMaxIds())))
    {
        range = ctx.getNumRanges() - 1;
        return XP_NO_ERR;
    }
    else
    {
        range = ctx.getNumRanges();
    }

    return XP_ERR_INVALID_ID;
}

XP_STATUS xpsIdAllocator::getRangeStartOffset(xpsIdAllocatorCtx& ctx,
                                              uint32_t range, uint32_t& start)
{
    std::vector<uint32_t> *rangeStarts = ctx.getRangeStarts();

    if (range >= ctx.getNumRanges())
    {
        return XP_ERR_OUT_OF_RANGE;
    }

    start = (*rangeStarts)[range];

    return XP_NO_ERR;
}

uint8_t xpsIdAllocator::isRangeFree(uint32_t range)
{
    XP_RW_LOCK_READ_PROTECT(this->xpsIdAllocatorLock, 0);

    if (idRanges[range].count1s() == idRanges[range].size())
    {
        /* all are 1 - range is empty */
        return 1;
    }
    else
    {
        /* there is at least one allocated id */
        return 0;
    }
}

