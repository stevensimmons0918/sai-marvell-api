// xpsMutexDummy.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsMutexDummy.h"

long int xpMutexDummy::getRdCount(uint32_t unused)
{
    return XP_LOCK_MAXTHREADS;
}
int xpMutexDummy::setRdCount(uint32_t unused0, long int unused1)
{
    return 0;
}
long int xpMutexDummy::getWrCount(uint32_t unused)
{
    return XP_LOCK_MAXTHREADS;
}
int xpMutexDummy::setWrCount(uint32_t unused0, long int unused1)
{
    return 0;
}
int xpMutexDummy::getReadLock(uint32_t unused)
{
    return 0;
}
int xpMutexDummy::putReadLock(uint32_t unused)
{
    return 0;
}
int xpMutexDummy::getWriteLock(uint32_t unused)
{
    return 0;
}
int xpMutexDummy::putWriteLock(uint32_t unused)
{
    return 0;
}
uint64_t xpMutexDummy::getThreadId()
{
    return (uint64_t)0u;
}
