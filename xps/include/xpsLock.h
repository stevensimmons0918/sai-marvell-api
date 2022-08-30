// xpsLock.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

#ifndef _INCLUDE_XPS_LOCK_
#define _INCLUDE_XPS_LOCK_

//#include "xpLockArr.h"
#include "xpEnums.h"
#include "xpTypes.h"
#include "xpsCommon.h"
#include "xpsSal.h"
#include "prvCpssBindFunc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * On compilation, python script genXpsToFlRelations.py is called to generate
 * xpsFlGroups.h file, that analyses XPS code and creates groups of locks.
 * Script analyses XPS code and deploy API calls chain to gather in groups all
 * used resources.
 * As a result xpOrderedWrAutoLock() functionality is generated.
 *
 */

#if defined(XP_MT_ENABLED) || defined(XP_XPS_LOCK_ENABLED)
#define XPS_LOCK(x)                      do {} while(0)   //TO DO: implement function specific locks
#define xpsLockTake(index)               xpsOsMutexTake(index)
#define xpsLockRelease(index)            xpsOsMutexRelease(index)
#define xpsLockCreate(index, num)       xpsOsMutexCreate(#index, index)
#define xpsLockDestroy(index)            xpsOsMutexDestroy(index)
#else
#define XPS_LOCK(x)               do {} while(0)
#define xpsLockTake(index)        do {} while(0)
#define xpsLockRelease(index)     do {} while(0)
#define xpsLockCreate(index, num) do {} while(0)
#define xpsLockDestroy(index)     do {} while(0)
#endif

/**
 * \brief Take the mutex at a given index
 *
 * \param [in] index Mutex Index
 *
 * \return
 */
void xpsOsMutexTake(uint32_t index);

/**
 * \brief Release the mutex at a given index
 *
 * \param [in] index Mutex index
 *
 * \return
 */
void xpsOsMutexRelease(uint32_t index);

/**
 * \brief Create mutex
 *
 * \param [in] name : Mutex name
 * \param [in] index : Mutex index
 *
 * \return
 */
void xpsOsMutexCreate(const char *name, uint32_t index);

/**
 * \brief Destroy a given mutex
 *
 * \param [in] index : Mutex index
 *
 * \return
 */
void xpsOsMutexDestroy(uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* _INCLUDE_XPS_LOCK_ */
