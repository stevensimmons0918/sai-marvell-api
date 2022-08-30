// xpsMutexBase.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _INCLUDE_XP_MUTEX_BASE_H_
#define _INCLUDE_XP_MUTEX_BASE_H_

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
#include <iostream>
#include <exception>
#include "xpsBitVector.h"
#include "openXpsTypes.h"

#define XP_LOCK_MAXTHREADS      (4096)

class xpMutexBase
{
public:
    xpMutexBase() {}
    /**
     * \public
     * \brief Get recursive read counter
     *
     * Method that returns the current value of recursive read counter value
     * for current thread.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return The return values range is “0” or positive value of counter.
     * “-1” must be returned in case of error.
     *
     */
    virtual long int getRdCount(uint32_t = 0) = 0;
    /**
     * \public
     * \brief Set recursive read counter
     *
     * Method that updates the current value of recursive read counter
     * value for current thread.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return “0” in case of success and “-1” in case of error.
     *
     */
    virtual int setRdCount(uint32_t = 0, long int = 0) = 0;
    /**
     * \public
     * \brief Get recursive write counter
     *
     * Method that returns the current value of recursive write counter value
     * for current thread.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return The return values range is “0” or positive value of counter.
     * “-1” must be returned in case of error.
     *
     */
    virtual long int getWrCount(uint32_t = 0) = 0;
    /**
     * \public
     * \brief Set recursive write counter
     *
     * Method that updates the current value of recursive write counter
     * value for current thread.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return “0” in case of success and “-1” in case of error.
     *
     */
    virtual int setWrCount(uint32_t = 0, long int = 0) = 0;
    /**
     * \public
     * \brief Obtain read lock
     *
     * Method that implements OS specific attempting to acquire read/write
     * lock with read permission. If the lock can’t be obtained the method
     * must suspend calling thread until condition for lock acquisition will
     * be met.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return “0” in case of success and “-1” in case of error.
     *
     */
    virtual int getReadLock(uint32_t = 0) = 0;
    /**
     * \public
     * \brief Release read lock
     *
     * Method that implements OS specific attempting to release previously
     * obtained read/write lock.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return “0” in case of success and “-1” in case of error.
     *
     */
    virtual int putReadLock(uint32_t = 0) = 0;
    /**
     * \public
     * \brief Obtain write lock
     *
     * Method that implements OS specific attempting to acquire read/write
     * lock with write permission. If the lock can’t be obtained the method
     * must suspend calling thread until condition for lock acquisition will
     * be met.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return “0” in case of success and “-1” in case of error.
     *
     */
    virtual int getWriteLock(uint32_t = 0) = 0;
    /**
     * \public
     * \brief Release write lock
     *
     * Method that implements OS specific attempting to release previously
     * obtained read/write lock.
     *
     * /param [in] -- specifies the index of XDK mutex instance.
     *
     * /return “0” in case of success and “-1” in case of error.
     *
     */
    virtual int putWriteLock(uint32_t = 0) = 0;
    virtual uint64_t getThreadId() = 0;
    virtual ~xpMutexBase() {};
};

#endif /* __cplusplus */
#endif
