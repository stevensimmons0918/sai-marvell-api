// xpsLock.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _INCLUDE_XPSAL_XPLOCK_
#define _INCLUDE_XPSAL_XPLOCK_


#define XP_LOCK_DEBUG      false /* for debugging lock implementation */

/* Description of xpLock semantics
 *
 * xpLock implements read/write locks. Although posix read/write locks are
 * used, it can change provided the API remain unaltered
 *
 * Recursive locking is implemented because of code requirements of XDK
 * Different situations are described below
 *
 * 1. Read lock is taken, read lock is called again by the same thread
 * Expected behavior:
 *      function _should_ return success
 * Rationale:
 *      Read lock is already held by self. One more read lock should not block
 * Constraint:
 *      Number of times read lock is called must match the number of times
 *      read unlock function is called to get the lock back to unlocked state
 *
 * 2. Read lock is taken, write lock is called by the same thread
 * Expected behavior:
 *      Write lock SHOULD NOT return success. Must fail and notify the
 *      programmer to correct the code flow by taking a write lock instead of
 *      read lock.
 * Rationale:
 *      There can be at least one other thread executing the same sequence --
 *      trying to obtain a write lock with corresponding read lock held.
 *      One of the threads (that does NOT get the lock will have to go back
 *      and re-read the value because the thread that got the lock have modified
 *      the value, making the previous value read inaccurate.
 *
 *      In the XDK, this will lead to code changes. So, the implemented behavior
 *      will let the programmer to correct the code.
 *
 * 3. Write lock taken, read lock is called
 * Expected behavior:
 *      Write lock is held means there are no readers. The same thread can
 *      take the read lock without any problem. Thus code path must return
 *      success
 *
 * 4. Write lock taken, write lock is called
 * Expected behavior:
 *      Same as (3). Thus code path must return success
 */
#ifdef __cplusplus
#include <iostream>
#include <exception>
#include "xpsBitVector.h"
#include "xpEnums.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include "xpsMutexBase.h"

#define XP_LOCK_NUM     (2)
#define XP_LOCK_MAXTHREADS      (4096)

class xpsGenLock       /* XP Generic Lock Interface class */
{
private:
    uint32_t lock_idx; /* global index of the lock in the system */
    const char *owner;
    xpMutexBase *lock_arr[XP_LOCK_NUM];
    xpMutexBase * volatile lock;  /* will point to one element in lock_arr */
    FILE *log_output;
    bool log_enable;
    int atomic_count;   /* number of threads waiting, must be atomic */
    static const bool rw_map[2][2];

public:
    xpsGenLock(int32_t idx = XP_LOCKINDEX_INVALID, uint32_t maxNum = 1,
               char *log_file = NULL);
    ~xpsGenLock();
    /**
     * \public
     * \brief Convert a mandatory lock to optional one.
     *
     * \return int
     */
    int redirect();
    /**
     * \public
     * \brief Convert an optimal lcok to mandatory one.
     *
     * \return int
     */
    int restore();

    /**
     * \public
     * \brief return lock identifier
     *
     * \return uint32_t
     */
    uint32_t getLockIdx();
    /**
     * \public
     * \brief returns lock owner name
     *
     * \return const char *
     */
    const char *getOwner();
    /**
     * \public
     * \brief Recursive read locking implementation
     *
     * \param [in] range
     * \param [in] lockOwner
     *
     * \return int
     */
    int getReadLock(uint32_t = 0, const char *lockOwner = "N/A");
    /**
     * \public
     * \brief Recursive read lock release implementation
     *
     * \param [in] range
     *
     * \return int
     */
    int putReadLock(uint32_t = 0);
    /**
     * \public
     * \brief Recursive write locking implementation
     *
     * \param [in] range
     * \param [in] lockOwner
     *
     * \return int
     */
    int getWriteLock(uint32_t = 0, const char *lockOwner = "N/A");
    /**
     * \public
     * \brief Recursive write lock release implementation
     *
     * \param [in] range
     *
     * \return int
     */
    int putWriteLock(uint32_t = 0);
    /**
     * \public
     * \brief Logging enable/disable functionality
     *
     * \param [in] flag
     *
     * \return int
     */
    int setLogEnable(bool flag);
    /**
     * \public
     * \brief Log an event
     *
     * \param [in] fmt
     *
     * \return int
     */
    int writeLog(const char * fmt, ...);
};

#endif /* __cplusplus */
#endif
