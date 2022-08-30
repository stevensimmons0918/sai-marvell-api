// xpsMutexPosix.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <map>
#include "xpsMutexPosix.h"

class tls_rw_cnt_t
{
public:
    long int read;
    long int write;
    tls_rw_cnt_t()
    {
        read = write = 0;
    }
};

pthread_key_t xpsMutexPthread::tlsKey;
pthread_once_t xpsMutexPthread::keyOnce = PTHREAD_ONCE_INIT;

void xpsMutexPthread::initTls()
{
    (void) pthread_key_create(&xpsMutexPthread::tlsKey, &clenaupTls);
}

void xpsMutexPthread::clenaupTls(void *data)
{
    if (data)
    {
        delete (std::map<pthread_rwlock_t*, tls_rw_cnt_t> *)data;
    }
}

xpsMutexPthread::xpsMutexPthread(uint32_t mutexCnt)
{
    uint32_t i;

    if (mutexCnt == 0)
    {
        mutexCnt = 1;
    }

    maxNum = mutexCnt;
    rwlock = new pthread_rwlock_t[maxNum];
    if (rwlock == NULL)
    {
        // throw exception here
        // ToDo: Fix with XDK-1358 Jira defect
    }
    (void) pthread_once(&keyOnce, &initTls);
    for (i = 0; i < maxNum; i++)
    {
        (void) pthread_rwlock_init(&rwlock[i], NULL);
    }
}

uint64_t xpsMutexPthread::getThreadId()
{
    return (uint64_t)xpThreadSelf();
}

long int xpsMutexPthread::getRdCount(uint32_t idx)
{
    if (idx >= maxNum)
    {
        return -1;
    }

    std::map<pthread_rwlock_t*, tls_rw_cnt_t> *tls =
        (std::map<pthread_rwlock_t*, tls_rw_cnt_t> *)pthread_getspecific(
            xpsMutexPthread::tlsKey);
    if (tls == NULL)
    {
        tls = new std::map<pthread_rwlock_t*, tls_rw_cnt_t>;
        pthread_setspecific(xpsMutexPthread::tlsKey, (void *)tls);
    }

    return (*tls)[&rwlock[idx]].read;
}

int xpsMutexPthread::setRdCount(uint32_t idx, long int count)
{
    if (idx >= maxNum)
    {
        return -1;
    }

    std::map<pthread_rwlock_t*, tls_rw_cnt_t> *tls =
        (std::map<pthread_rwlock_t*, tls_rw_cnt_t> *)pthread_getspecific(
            xpsMutexPthread::tlsKey);
    if (tls == NULL)
    {
        tls = new std::map<pthread_rwlock_t*, tls_rw_cnt_t>;
        pthread_setspecific(xpsMutexPthread::tlsKey, (void *)tls);
    }

    (*tls)[&rwlock[idx]].read = count;

    return 0;
}

long int xpsMutexPthread::getWrCount(uint32_t idx)
{
    if (idx >= maxNum)
    {
        return -1;
    }
    std::map<pthread_rwlock_t*, tls_rw_cnt_t> *tls =
        (std::map<pthread_rwlock_t*, tls_rw_cnt_t> *)pthread_getspecific(
            xpsMutexPthread::tlsKey);
    if (tls == NULL)
    {
        tls = new std::map<pthread_rwlock_t*, tls_rw_cnt_t>;
        pthread_setspecific(xpsMutexPthread::tlsKey, (void *)tls);
    }

    return (*tls)[&rwlock[idx]].write;
}

int xpsMutexPthread::setWrCount(uint32_t idx, long int count)
{
    if (idx >= maxNum)
    {
        return -1;
    }

    std::map<pthread_rwlock_t*, tls_rw_cnt_t> *tls =
        (std::map<pthread_rwlock_t*, tls_rw_cnt_t> *)pthread_getspecific(
            xpsMutexPthread::tlsKey);
    if (tls == NULL)
    {
        tls = new std::map<pthread_rwlock_t*, tls_rw_cnt_t>;
        pthread_setspecific(xpsMutexPthread::tlsKey, (void *)tls);
    }

    (*tls)[&rwlock[idx]].write = count;

    return 0;
}

int xpsMutexPthread::putReadLock(uint32_t idx)
{
    if (idx >= maxNum)
    {
        return -1;
    }
    return pthread_rwlock_unlock(&rwlock[idx]);
}

int xpsMutexPthread::putWriteLock(uint32_t idx)
{
    if (idx >= maxNum)
    {
        return -1;
    }
    return pthread_rwlock_unlock(&rwlock[idx]);
}

int xpsMutexPthread::getReadLock(uint32_t idx)
{
    if (idx >= maxNum)
    {
        return -1;
    }
    return pthread_rwlock_rdlock(&rwlock[idx]);
}

int xpsMutexPthread::getWriteLock(uint32_t idx)
{
    if (idx >= maxNum)
    {
        return -1;
    }
    return pthread_rwlock_wrlock(&rwlock[idx]);
}

xpsMutexPthread::~xpsMutexPthread()
{
    uint32_t i;
    for (i = 0; i < maxNum; i++)
    {
        pthread_rwlock_destroy(&rwlock[i]);
    }
    // not destroying the rwlockattr_t
    if (rwlock)
    {
        delete [] rwlock;
        rwlock = NULL;
    }
}
