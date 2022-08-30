// xpsMutexDummy.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _INCLUDE_XP_MUTEX_DUMMY_H_
#define _INCLUDE_XP_MUTEX_DUMMY_H_

#include "xpsMutexBase.h"

#ifdef __cplusplus
#include <iostream>
#include <exception>
#include "xpsBitVector.h"
#include "xpEnums.h"
#include "xpTypes.h"

class xpMutexDummy: public xpMutexBase
{
public:
    xpMutexDummy() {}
    long int getRdCount(uint32_t = 0);
    int setRdCount(uint32_t = 0, long int = 0);
    long int getWrCount(uint32_t = 0);
    int setWrCount(uint32_t = 0, long int = 0);
    int getReadLock(uint32_t = 0);
    int putReadLock(uint32_t = 0);
    int getWriteLock(uint32_t = 0);
    int putWriteLock(uint32_t = 0);
    uint64_t getThreadId();
    ~xpMutexDummy() {}
};

#endif /* __cplusplus */
#endif
