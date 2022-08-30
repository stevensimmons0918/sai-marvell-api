// xpsLockArr.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsLockArr.h"
#include "xpsSal.h"
#include <stdarg.h>

xpsLockArray::xpsLockArray():
    lock()
{
    for (uint32_t i = 0u; i < XP_MAX_DEVICES; i++)
    {
        for (uint32_t j = 0u; j < XP_LOCKINDEX_MAX; j++)
        {
            rangeStatic[i][j] = NULL;
        }

        rangeDynamic[i].clear();
        dynId[i].clear();
    }

    this->createGenSystemLocks();
}

xpsLockArray::~xpsLockArray()
{
    this->destroyGenSystemLocks();

    for (uint32_t i = 0u; i < XP_MAX_DEVICES; i++)
    {
        for (uint32_t j = 0u; j < XP_LOCKINDEX_MAX; j++)
        {
            if (rangeStatic[i][j] != NULL)
            {
                delete rangeStatic[i][j];
            }
        }

        for (uint32_t j = 0u; j < rangeDynamic[i].size(); j++)
        {
            if (rangeDynamic[i][j] != NULL)
            {
                delete rangeDynamic[i][j];
            }
        }
        rangeDynamic[i].clear();
        dynId[i].clear();
    }
}

void xpsLockArray::createGenSystemLocks(void)
{
    // We must create SAL lock here, because there is not other corresponding place
    // to perform this action.
    this->createLock(0u, XP_LOCKINDEX_SAL);

    // We must create buffer manager lock here, because there is not other
    //corresponding place to perform this action.
    this->createLock(0u, XP_LOCKINDEX_BUFFER_MGR);

    // We must create Packet Driver Interface lock here,
    // because there is not other corresponding place
    // to perform this action.
    this->createLock(0u, XP_LOCKINDEX_PDI);

    // We must create xpSwPersistentAllocator lock here,
    // because there is not other corresponding place
    // to perform this action.
    this->createLock(0u, XP_LOCKINDEX_PERSIST_ALLOCATOR);

    // We must create log specific locks here,
    // because there is not other corresponding place
    // to perform this action.
    this->createLock(0u, XP_LOCKINDEX_FILE_LOG);
    this->createLock(0u, XP_LOCKINDEX_CONSOLE_LOG);
    this->createLock(0u, XP_LOCKINDEX_BUF_LOG);
    this->createLock(0u, XP_LOCKINDEX_SHARED_BUF_LOG);
    this->createLock(0u, XP_LOCKINDEX_PORT_CREATE);
    this->createLock(0u, XP_LOCKINDEX_DMA);
    this->createLock(0u, XP_LOCKINDEX_QUEUE_TX);
    this->createLock(0u, XP_LOCKINDEX_QUEUE_RX);
    this->createLock(0u, XP_LOCKINDEX_BUS);
    this->createLock(0u, XP_LOCKINDEX_IPC_UDP);
    this->createLock(0u, XP_LOCKINDEX_IPC_TCP);
    this->createLock(0u, XP_LOCKINDEX_IPC_NETLINK);
    this->createLock(0u, XP_LOCKINDEX_FL_ALLOCATOR_MGR);
}

void xpsLockArray::destroyGenSystemLocks(void)
{
    this->destroyLock(0u, XP_LOCKINDEX_SAL);
    this->destroyLock(0u, XP_LOCKINDEX_BUFFER_MGR);
    this->destroyLock(0u, XP_LOCKINDEX_PDI);
    this->destroyLock(0u, XP_LOCKINDEX_PERSIST_ALLOCATOR);
    this->destroyLock(0u, XP_LOCKINDEX_FILE_LOG);
    this->destroyLock(0u, XP_LOCKINDEX_CONSOLE_LOG);
    this->destroyLock(0u, XP_LOCKINDEX_BUF_LOG);
    this->destroyLock(0u, XP_LOCKINDEX_SHARED_BUF_LOG);
    this->destroyLock(0u, XP_LOCKINDEX_FILE_LOG);
    this->destroyLock(0u, XP_LOCKINDEX_DMA);
    this->destroyLock(0u, XP_LOCKINDEX_QUEUE_TX);
    this->destroyLock(0u, XP_LOCKINDEX_QUEUE_RX);
    this->destroyLock(0u, XP_LOCKINDEX_BUS);
    this->destroyLock(0u, XP_LOCKINDEX_IPC_UDP);
    this->destroyLock(0u, XP_LOCKINDEX_IPC_TCP);
    this->destroyLock(0u, XP_LOCKINDEX_IPC_NETLINK);
    this->destroyLock(0u, XP_LOCKINDEX_FL_ALLOCATOR_MGR);
}

xpsLockArray* xpsLockArray::instance()
{
    static xpsLockArray sInstance;

    return &sInstance;
}

int xpsLockArray::disableAllLockLogs()
{
    for (uint32_t i = 0u; i < XP_MAX_DEVICES; i++)
    {
        /* set disable flag for static range */
        for (uint32_t j = 0u; j < XP_LOCKINDEX_MAX; j++)
        {
            if (rangeStatic[i][j] != NULL)
            {
                rangeStatic[i][j]->setLogEnable(false);
            }
        }

        /* set disable flag for dynamic range */
        for (uint32_t j = 0u; j < rangeDynamic[i].size(); j++)
        {
            if (rangeDynamic[i][j] != NULL)
            {
                rangeDynamic[i][j]->setLogEnable(false);
            }
        }
    }

    return 0;
}

int xpsLockArray::enableAllLockLogs()
{
    for (uint32_t i = 0u; i < XP_MAX_DEVICES; i++)
    {
        /* set enable flag for static range */
        for (uint32_t j = 0u; j < XP_LOCKINDEX_MAX; j++)
        {
            if (rangeStatic[i][j] != NULL)
            {
                rangeStatic[i][j]->setLogEnable(true);
            }
        }

        /* set enable flag for dynamic range */
        for (uint32_t j = 0u; j < rangeDynamic[i].size(); j++)
        {
            if (rangeDynamic[i][j] != NULL)
            {
                rangeDynamic[i][j]->setLogEnable(true);
            }
        }
    }

    return 0;
}

void xpsLockArray::printLockInfo()
{
    for (uint32_t i = 0u; i < XP_MAX_DEVICES; i++)
    {
        /* set enable flag for static range */
        for (uint32_t j = 0u; j < XP_LOCKINDEX_MAX; j++)
        {
            if (rangeStatic[i][j] != NULL)
            {
                rangeStatic[i][j]->writeLog("\tdevId: %d\towner: %s", i,
                                            rangeStatic[i][j]->getOwner());
            }
        }

        for (uint32_t j = 0u; j < rangeDynamic[i].size(); j++)
        {
            if (rangeDynamic[i][j] != NULL)
            {
                rangeDynamic[i][j]->writeLog("\tdevId: %d\towner: %s", i,
                                             rangeDynamic[i][j]->getOwner());
            }
        }
    }
}

int xpsLockArray::allocateLockId(xpDevice_t devId, uint32_t &lockId)
{
    uint32_t i;

    if (devId >= XP_MAX_DEVICES)
    {
        return -1;
    }

    lock.getWriteLock();
    for (i = 0u; i < dynId[devId].size(); i++)
    {
        /* check if we have free Id in the list */
        if (dynId[devId][i] == XP_LOCKINDEX_INVALID)
        {
            break;
        }
    }
    /* not found. we need to add new one to the end */
    if (i == dynId[devId].size())
    {
        dynId[devId].push_back(i + XP_LOCKINDEX_MAX);
        rangeDynamic[devId].push_back(NULL);
    }
    lockId = dynId[devId][i];
    lock.putWriteLock();

    return 0;
}

int xpsLockArray::releaseLockId(xpDevice_t devId, uint32_t lockId)
{
    int ret = -1;

    if (devId >= XP_MAX_DEVICES)
    {
        return -1;
    }

    lock.getWriteLock();
    if ((lockId >= XP_LOCKINDEX_MAX) &&
        (lockId < (dynId[devId].size() + XP_LOCKINDEX_MAX)))
    {
        if ((dynId[devId][lockId - XP_LOCKINDEX_MAX] != XP_LOCKINDEX_INVALID) &&
            (rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] == NULL))
        {
            dynId[devId][lockId - XP_LOCKINDEX_MAX] = XP_LOCKINDEX_INVALID;
            ret = 0;
        }
    }
    lock.putWriteLock();

    return ret;
}

xpsGenLock *xpsLockArray::createLock(xpDevice_t devId, uint32_t lockId,
                                     uint32_t mutexNum)
{
    xpsGenLock *pGenLock = NULL;

    if (devId >= XP_MAX_DEVICES)
    {
        return NULL;
    }

    lock.getWriteLock();
    if (lockId < XP_LOCKINDEX_MAX)
    {
        if (rangeStatic[devId][lockId] == NULL)
        {
            pGenLock = rangeStatic[devId][lockId] = new xpsGenLock(lockId, mutexNum);
        }
    }
    else
    {
        if ((lockId < (dynId[devId].size() + XP_LOCKINDEX_MAX)) &&
            (dynId[devId][lockId - XP_LOCKINDEX_MAX] != XP_LOCKINDEX_INVALID))
        {
            if (rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] == NULL)
            {
                pGenLock = rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] = new xpsGenLock(
                    lockId, mutexNum);
            }
        }
    }
    lock.putWriteLock();

    return pGenLock;
}

void xpsLockArray::destroyLock(xpDevice_t devId, uint32_t lockId)
{
    if (devId >= XP_MAX_DEVICES)
    {
        return;
    }

    lock.getWriteLock();
    if (lockId < XP_LOCKINDEX_MAX)
    {
        if (rangeStatic[devId][lockId] != NULL)
        {
            delete rangeStatic[devId][lockId];
            rangeStatic[devId][lockId] = NULL;
        }
    }
    else
    {
        if ((lockId < (rangeDynamic[devId].size() + XP_LOCKINDEX_MAX)) &&
            (dynId[devId][lockId - XP_LOCKINDEX_MAX] != XP_LOCKINDEX_INVALID))
        {
            if (rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] != NULL)
            {
                delete rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX];
                rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] = NULL;
            }
        }
    }
    lock.putWriteLock();
}

xpsGenLock *xpsLockArray::getLock(xpDevice_t devId, uint32_t lockId)
{
    if (devId >= XP_MAX_DEVICES)
    {
        return NULL;
    }

    if (lockId < XP_LOCKINDEX_MAX)
    {
        return rangeStatic[devId][lockId];
    }
    else
    {
        if ((lockId < (rangeDynamic[devId].size() + XP_LOCKINDEX_MAX)) &&
            (dynId[devId][lockId - XP_LOCKINDEX_MAX] != XP_LOCKINDEX_INVALID))
        {
            return rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX];
        }
    }

    return NULL;
}

int xpsLockArray::enableLock(xpDevice_t devId, uint32_t lockId)
{
    int ret = -1;

    if (devId >= XP_MAX_DEVICES)
    {
        return -1;
    }

    lock.getWriteLock();
    if (lockId < XP_LOCKINDEX_MAX)
    {
        ret = rangeStatic[devId][lockId]->restore();
    }
    else
    {
        if ((lockId < (dynId[devId].size() + XP_LOCKINDEX_MAX) &&
             (dynId[devId][lockId - XP_LOCKINDEX_MAX] != XP_LOCKINDEX_INVALID)))
        {
            if (rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] != NULL)
            {
                ret = rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX]->restore();
            }
        }
    }
    lock.putWriteLock();

    return ret;
}

int xpsLockArray::disableLock(xpDevice_t devId, uint32_t lockId)
{
    int ret = -1;

    if (devId >= XP_MAX_DEVICES)
    {
        return -1;
    }

    lock.getWriteLock();
    if (lockId < XP_LOCKINDEX_MAX)
    {
        ret = rangeStatic[devId][lockId]->redirect();
    }
    else
    {
        if ((lockId < (dynId[devId].size() + XP_LOCKINDEX_MAX) &&
             (dynId[devId][lockId - XP_LOCKINDEX_MAX] != XP_LOCKINDEX_INVALID)))
        {
            if (rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX] != NULL)
            {
                ret = rangeDynamic[devId][lockId - XP_LOCKINDEX_MAX]->redirect();
            }
        }
    }
    lock.putWriteLock();

    return ret;
}


xpsLockRdAuto::xpsLockRdAuto(const char *lockOwner, xpsGenLock* pLock,
                             uint32_t mutexId) :
    lock(pLock),
    range(mutexId)
{
    if (lock)
    {
        lock->getReadLock(range, lockOwner);
    }
}

xpsLockRdAuto::xpsLockRdAuto(const char *lockOwner, xpDevice_t devId,
                             uint32_t lockId, uint32_t mutexId) :
    range(mutexId)
{
    lock = xpsLockArray::instance()->getLock(devId, lockId);
    if (lock)
    {
        lock->getReadLock(range, lockOwner);
    }
}

xpsLockRdAuto::~xpsLockRdAuto()
{
    if (lock)
    {
        lock->putReadLock(range);
    }
}

xpsLockWrAuto::xpsLockWrAuto(const char *lockOwner, xpsGenLock* pLock,
                             uint32_t mutexId) :
    lock(pLock),
    range(mutexId)
{
    if (lock)
    {
        lock->getWriteLock(range, lockOwner);
    }
}

xpsLockWrAuto::xpsLockWrAuto(const char *lockOwner, xpDevice_t devId,
                             uint32_t lockId, uint32_t mutexId) :
    range(mutexId)
{
    lock = xpsLockArray::instance()->getLock(devId, lockId);
    if (lock)
    {
        lock->getWriteLock(range, lockOwner);
    }
}

xpsLockWrAuto::~xpsLockWrAuto()
{
    if (lock)
    {
        lock->putWriteLock(range);
    }
}

class xpLockSortCmp
{
public:
    bool operator()(xpsGenLock *i, xpsGenLock *j)
    {
        if ((i == (xpsGenLock *)XP_LOCK_DISABLE_VALUE) ||
            (j == (xpsGenLock *)XP_LOCK_DISABLE_VALUE))
        {
            return 0;
        }
        return (i->getLockIdx() < j->getLockIdx());
    }
};

xpsLockOrderedWrAuto::xpsLockOrderedWrAuto(const char *lockOwner, int lockNum,
                                           ...)
{
    va_list ap;

    lock_lst.clear();

    va_start(ap, lockNum);
    while (lockNum-- > 0)
    {
        xpsGenLock *lock = va_arg(ap, xpsGenLock *);
        if (lock)
        {
            lock_lst.push_back(lock);
        }
    }
    va_end(ap);

    std::sort(lock_lst.begin(), lock_lst.end(), xpLockSortCmp());

    for (std::vector<xpsGenLock *>::iterator it = lock_lst.begin();
         it != lock_lst.end(); ++it)
    {
        (*it)->getWriteLock(0u, lockOwner);
    }
}

xpsLockOrderedWrAuto::~xpsLockOrderedWrAuto()
{
    for (std::vector<xpsGenLock *>::iterator it = lock_lst.begin();
         it != lock_lst.end(); ++it)
    {
        (*it)->putWriteLock(0u);
    }
}

xpsLockOrderedRdAuto::xpsLockOrderedRdAuto(const char *lockOwner, int lockNum,
                                           ...)
{
    va_list ap;

    lock_lst.clear();

    va_start(ap, lockNum);
    while (lockNum-- > 0)
    {
        xpsGenLock *lock = va_arg(ap, xpsGenLock *);
        if (lock)
        {
            lock_lst.push_back(lock);
        }
    }
    va_end(ap);

    std::sort(lock_lst.begin(), lock_lst.end(), xpLockSortCmp());

    for (std::vector<xpsGenLock *>::iterator it = lock_lst.begin();
         it != lock_lst.end(); ++it)
    {
        (*it)->getReadLock(0u, lockOwner);
    }
}

xpsLockOrderedRdAuto::~xpsLockOrderedRdAuto()
{
    for (std::vector<xpsGenLock *>::iterator it = lock_lst.begin();
         it != lock_lst.end(); ++it)
    {
        (*it)->putReadLock(0u);
    }
}

xpsLockMgr::xpsLockMgr()
{
    memset(xpMgrLock, 0, XP_MAX_DEVICES * sizeof(xpsGenLock *));
}

xpsLockMgr::~xpsLockMgr()
{
}

xpsGenLock* xpsLockMgr::getLock(xpDevice_t devId)
{
    if (devId >= XP_MAX_DEVICES)
    {
        return NULL;
    }

    return xpMgrLock[devId];
}
