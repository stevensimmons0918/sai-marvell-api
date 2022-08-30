// xpsLock.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdarg.h>

#include "xpsLockArr.h"
#include "xpsMutexDummy.h"
#include "xpsMutexPosix.h"

#define XP_LOCK_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            XP_PRINT_STACK_TRACE(); \
            while(1) { \
                writeLog("Assertion occurred: %s, %d", \
                    __FILE__, __LINE__); \
                sleep(1); \
            } \
        } \
    } while(0)

#ifndef MAC
#define TIMESTAMP_FMT "Time: %02lu:%02lu:%02lu:%03lu:%03lu:%03lu"
#define TIMESTAMP_MJR_ARGS (ts.tv_sec / 3600LU), (ts.tv_sec / 60LU) % 60, (ts.tv_sec % 60)
#define TIMESTAMP_MNR_ARGS ts.tv_nsec / 1000000LU, (ts.tv_nsec / 1000LU) % 1000, ts.tv_nsec % 1000LU
#define LOGINFO "\ntid: %" PRIu64" | lockidx: %d | " TIMESTAMP_FMT " | %s\n"
#endif // MAC

/*
 * Semantics of the optional/mandatory lock is explained as comments
 * see. "Description of xpLock semantics" in xpLock.h
 */
const bool xpsGenLock::rw_map[2][2] = {{true, true},  // ReadLock operation
    {true, false}
}; // WriteLock operation

xpsGenLock::xpsGenLock(int32_t idx, uint32_t maxNum, char *log_file)
{
    int i;

    lock_idx = idx;
    owner = "N/A";
    atomic_count = 0;

    if (!log_file || !(log_output = fopen(log_file, "w+")))
    {
        log_output = NULL;
        log_output = stderr;

    }
    log_enable = XP_LOCK_DEBUG;    // by default log is disabled

    lock_arr[0] = new xpsMutexPthread(maxNum);    // mandatory lock
    if (lock_arr[0] == NULL)
    {
        // throw exception here
    }
    for (i = 1; i < XP_LOCK_NUM; i++)
    {
        lock_arr[i] = new xpMutexDummy();   // dummy locks
        if (lock_arr[i] == NULL)
        {
            // throw exception here
        }
    }
    lock = lock_arr[0];
    writeLog("Mgr %d initialized", lock_idx);
}

xpsGenLock::~xpsGenLock()
{
    int i;

    for (i = 0; i < XP_LOCK_NUM; i++)
    {
        delete lock_arr[i];
    }

    if (log_output != stderr)
    {
        fclose(log_output);
    }
}

uint32_t xpsGenLock::getLockIdx()
{
    return lock_idx;
}

const char *xpsGenLock::getOwner()
{
    return owner;
}

int xpsGenLock::getReadLock(uint32_t range, const char *lockOwner)
{
    if (this == (xpsGenLock *)XP_LOCK_DISABLE_VALUE)
    {
        return (XP_NO_ERR);
    }

    /* recursive read locking is alright by the same thread
     * Make sure that no write lock is taken
     */
    long int r = this->lock->getRdCount(range);
    long int w = this->lock->getWrCount(range);

    writeLog("%s | getReadLock", lockOwner);

#if XP_LOCK_DEBUG
    /* Apply debug only to non dummy lock */
    if (lock_arr[0] == this->lock)
    {
        XP_LOCK_ASSERT((r >= 0) && (w >= 0));
    }
#endif
    /* Validate recursive lock rules. "0" in "rw_map" first index states
     * for attempt of ReadLock operation */
    if (rw_map[0][w > 0] == false)
    {
        writeLog("getReadLock(): invalid recursive lock\n");
        return -1;
    }

    if (this->lock->setRdCount(range, r + 1) != 0)
    {
        writeLog("setRdCount(): failed\n");
        return -1;
    }

    /* check the previously read value */
    if ((w > 0) || (r > 0))
    {
        /* Already held, nothing to do now */
        writeLog("getReadLock() with lock  w(%ld), r(%ld)\n", w, r);
        return 0;
    }
    /* Please note that atomic_count is shared across all threads and
     * will be zero only if all r == 0 and w == 0
     */
    if (__sync_add_and_fetch(&atomic_count, 1) > 0)
    {
        int rc;
        rc = this->lock->getReadLock(range);
        owner = lockOwner;

        return rc;
    }
    __sync_sub_and_fetch(&atomic_count, 1);

    return 0;
}

int xpsGenLock::getWriteLock(uint32_t range, const char *lockOwner)
{
    if (this == (xpsGenLock *)XP_LOCK_DISABLE_VALUE)
    {
        return (XP_NO_ERR);
    }

    /* recursive read locking is alright by the same thread
     * Make sure that no write lock is taken
     */
    long int r = this->lock->getRdCount(range);
    long int w = this->lock->getWrCount(range);

    writeLog("%s | getWriteLock", lockOwner);
#if XP_LOCK_DEBUG
    /* Apply debug only to non dummy lock */
    if (lock_arr[0] == this->lock)
    {
        XP_LOCK_ASSERT((r >= 0) && (w >= 0));
    }
#endif

    /* Validate recursive lock rules. "1" in first index "rw_map" states
     * for attempt of WriteLock operation */
    if (rw_map[1][r > 0] == false)
    {
        /* Here is a little tricky part
         * At this point in time, write lock is not held by anyone because
         * we hold the read lock. This thread of execution wants to make sure
         * that read lock can be released and write lock acquired atomically
         * This poses a challenge to XDK because, even if we release read lock
         * and acquire write lock atomically, there can be another thread that
         * wants do the same sequence of operations. In that case one of the
         * threads will have to fail because the read value could have been
         * modified by the other thread
         */
        writeLog("getWriteLock(): invalid recursive lock\n");
        return -1;
    }

    if (w > 0)
    {
        /* Do not exit. According to the newly designed semantic, just increment
         * the count because write lock is already taken by this thread itself
         */
        if (this->lock->setWrCount(range, w + 1))
        {
            writeLog("setWrCount failed\n");
        }
        writeLog("getWriteLock(): wcount(%ld), rcount(%ld)\n", w, r);
        /* incremented wcount, return success now */
        return 0;
    }

    /* At this point, w == 0 and r == 0 */
    if (this->lock->setWrCount(range, w + 1))
    {
        writeLog("setWrCount failed\n");
    }

    if (__sync_add_and_fetch(&atomic_count, 1) > 0)
    {
        int rc;

        rc = this->lock->getWriteLock(range);
        owner = lockOwner;

        return rc;
    }
    /* w is stored in TLS, i.e., it is thread specific. Hence setspecifc */
    if (this->lock->setWrCount(range, w - 1))
    {
        writeLog("setWrCount failed to set = %ld\n", w-1);
    }
    __sync_sub_and_fetch(&atomic_count, 1);
    return 0;
}

int xpsGenLock::putReadLock(uint32_t range)
{
    if (this == (xpsGenLock *)XP_LOCK_DISABLE_VALUE)
    {
        return (XP_NO_ERR);
    }

    long int r = this->lock->getRdCount(range);
    long int w = this->lock->getWrCount(range);

    writeLog("%s | putReadLock", owner);

#if XP_LOCK_DEBUG
    /* Apply debug only to non dummy lock */
    if (lock_arr[0] == this->lock)
    {
        XP_LOCK_ASSERT((r > 0) && (w >= 0));
    }
#endif
    if (this->lock->setRdCount(range, r - 1))
    {
        writeLog("setRdCount failed\n");
    }
    if (r > 1)
    {
        /* locked multiple times. No further action required */
        return 0;
    }
    if (w == 0)
    {
        /* No one is holding the lock now, hence release the lock */
        this->lock->putReadLock(range);
        /* There existed only one read lock, subtract the count */
        __sync_sub_and_fetch(&atomic_count, 1);
    }
    return 0;
}

int xpsGenLock::putWriteLock(uint32_t range)
{
    if (this == (xpsGenLock *)XP_LOCK_DISABLE_VALUE)
    {
        return (XP_NO_ERR);
    }

    long int w = this->lock->getWrCount(range);

    writeLog("%s | putWriteLock", owner);

#if XP_LOCK_DEBUG
    /* Apply debug only to non dummy lock */
    if (lock_arr[0] == this->lock)
    {
        XP_LOCK_ASSERT((w > 0) && (this->lock->getRdCount(range) == 0));
    }
#endif

    if (this->lock->setWrCount(range, w - 1))
    {
        writeLog("setWrCount failed\n");
    }
    if (w > 1)
    {
        /* there are more references to unlock */
        return 0;
    }
    this->lock->putWriteLock(range);
    /* there existed only one (write) lock, subtract the count */
    __sync_sub_and_fetch(&atomic_count, 1);
    return 0;
}

int xpsGenLock::redirect()
{
    /* swap the pointers if and only if lock is NOT taken
     * Here we change the atomic variable (if it is zero -- none has acquired
     * the lock) such that from now onward, none can take the lock.
     *
     * The atomic swapping is effective only if the lock is not taken by
     * anyone (value == 0) */
    if (__sync_bool_compare_and_swap(&atomic_count, 0, -XP_LOCK_MAXTHREADS)
        == false)
    {
        return -EBUSY;
    }
    lock = lock_arr[1];
    /* In the meanwhile, someone could have tried to acquire the lock
     * making the the value of atomic_count != XP_LOCK_MAXTREHADS
     * In that case, the acquired thread will release it soon, so spin
     * on the variable */
    while (__sync_bool_compare_and_swap(&atomic_count, -XP_LOCK_MAXTHREADS, 0)
           != true);

    return 0;
}

int xpsGenLock::restore()
{
    /* swap the pointers if and only if lock is NOT taken
     * Here we change the atomic variable (if it is zero -- none has acquired
     * the lock) such that from now onward, none can take the lock.
     *
     * The atomic swapping is effective only if the lock is not taken by
     * anyone (value == 0) */
    if (__sync_bool_compare_and_swap(&atomic_count, 0, -XP_LOCK_MAXTHREADS)
        == false)
    {
        return -EBUSY;
    }
    lock = lock_arr[0];
    /* In the meanwhile, someone could have tried to acquire the lock
     * making the the value of atomic_count != XP_LOCK_MAXTREHADS
     * In that case, the acquired thread will release it soon, so spin
     * on the variable */
    while (__sync_bool_compare_and_swap(&atomic_count, -XP_LOCK_MAXTHREADS, 0)
           != true);

    return 0;
}

int xpsGenLock::writeLog(const char * fmt, ...)
{
#ifndef MAC
    struct timespec ts;
    char buf[256] = {};
    va_list args;

    if (!log_enable)
    {
        return 0;
    }

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, args);
    va_end(args);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);

    fprintf(log_output, LOGINFO, lock->getThreadId(), getLockIdx(),
            TIMESTAMP_MJR_ARGS, TIMESTAMP_MNR_ARGS, buf);
    fflush(log_output);
#endif // MAC

    return 1;
}

int xpsGenLock::setLogEnable(bool flag)
{
    log_enable = flag;

    return 0;
}
