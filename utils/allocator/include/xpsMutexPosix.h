// xpsMutexPosix.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _INCLUDE_XPSAL_XPMUTEXPOSIX_
#define _INCLUDE_XPSAL_XPMUTEXPOSIX_

#ifdef __cplusplus
#include <iostream>
#include <exception>
#include "xpsMutexBase.h"


#include "xpsSal.h"

#include "xpsBitVector.h"
#include "xpEnums.h"

#endif

#include <pthread.h>

class xpsMutexPthread: public xpMutexBase
{
private:
    static pthread_key_t tlsKey;
    static pthread_once_t keyOnce;

    uint32_t maxNum;
    /* We don't use rwlock because currently there is no way to upgrade
     * holding a read lock to holding a write lock */
    pthread_rwlock_t *rwlock;

    static void initTls();
    static void clenaupTls(void *data);
public:
    xpsMutexPthread(uint32_t mutexCnt = 1);
    long int getRdCount(uint32_t = 0);
    int setRdCount(uint32_t = 0, long int = 0);
    long int getWrCount(uint32_t = 0);
    int setWrCount(uint32_t = 0, long int = 0);
    int getReadLock(uint32_t = 0);
    int putReadLock(uint32_t = 0);
    int getWriteLock(uint32_t = 0);
    int putWriteLock(uint32_t = 0);
    uint64_t getThreadId();
    ~xpsMutexPthread();
};

#endif
