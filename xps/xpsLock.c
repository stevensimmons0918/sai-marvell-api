// xpsLock.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsLock.c
 * \brief
 *
 */
#include "xpsLock.h"
#include "cpssHalUtil.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static unsigned long mutexId[XP_LOCKINDEX_MAX];

void xpsOsMutexCreate(const char *name, uint32_t index)
{
    XP_STATUS ret;
    GT_STATUS rc;
    CPSS_OS_MUTEX *mtx;

    if (index >= XP_LOCKINDEX_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " %s is not a valid Id\n",
              name);
        return;
    }
    mtx = (CPSS_OS_MUTEX *) &mutexId[index];
    rc = cpssOsMutexCreate(name, mtx);
    ret= xpsConvertCpssStatusToXPStatus(rc);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Lock Creation Failed\n");
        return;
    }
}


void xpsOsMutexTake(uint32_t index)
{
    XP_STATUS ret;
    if (index >= XP_LOCKINDEX_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Index is not a valid Id\n");
        return;
    }
    ret= xpsConvertCpssStatusToXPStatus(cpssOsMutexLock(mutexId[index]));
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Locking Failed\n");
        return;
    }


}

void xpsOsMutexRelease(uint32_t index)
{
    XP_STATUS ret;
    if (index >= XP_LOCKINDEX_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Index is not a valid Id\n");
        return;
    }
    ret= xpsConvertCpssStatusToXPStatus(cpssOsMutexUnlock(mutexId[index]));
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Lock release failed\n");
        return;
    }

}

void xpsOsMutexDestroy(uint32_t index)
{
    XP_STATUS ret;
    if (index >= XP_LOCKINDEX_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Index is not a valid Id\n");
        return;
    }
    ret= xpsConvertCpssStatusToXPStatus(cpssOsMutexDelete(mutexId[index]));
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Lock destroy failed\n");
        return;
    }

}

#ifdef __cplusplus
}
#endif

