// xpsIdAllocatorCtx.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsIdAllocatorCtx.h"

xpsIdAllocatorCtx::xpsIdAllocatorCtx() : maxIds(0),
    numRanges(0)
{

}

xpsIdAllocatorCtx::xpsIdAllocatorCtx(uint32_t maxIds,
                                     uint32_t numRanges) : maxIds(maxIds),
    numRanges(numRanges)
{

}

xpsIdAllocatorCtx::xpsIdAllocatorCtx(const xpsIdAllocatorCtx& ctx)
{
    this->numRanges     = ctx.numRanges;
    this->maxIds        = ctx.maxIds;
    this->rangeStarts   = ctx.rangeStarts;
}

xpsIdAllocatorCtx::~xpsIdAllocatorCtx()
{
    std::vector<uint32_t>().swap(this->rangeStarts);
}

XP_STATUS xpsIdAllocatorCtx::init()
{
    if (!this->numRanges)
    {
        return XP_ERR_INVALID_DATA;
    }

    this->rangeStarts.resize(numRanges, 0);

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocatorCtx::init(uint32_t maxIds, uint32_t numRanges)
{
    this->maxIds    = maxIds;
    this->numRanges = numRanges;
    this->rangeStarts.resize(numRanges, 0);

    return XP_NO_ERR;
}

XP_STATUS xpsIdAllocatorCtx::setRangeStartOffset(uint32_t range, uint32_t start)
{
    if (range >= this->getNumRanges())
    {
        return XP_ERR_OUT_OF_RANGE;
    }

    this->rangeStarts[range] = start;

    return XP_NO_ERR;
}


xpsIdAllocatorCtx& xpsIdAllocatorCtx::operator=(xpsIdAllocatorCtx ctx)
{
    // Using Copy and Swap idiom
    std::swap(this->numRanges,   ctx.numRanges);
    std::swap(this->maxIds,      ctx.maxIds);
    std::swap(this->rangeStarts, ctx.rangeStarts);

    return *this;
}

