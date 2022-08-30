/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file simOsBindOwn.c
*
* @brief allow bind of the simOs functions to the simulation
*
* @version   3
********************************************************************************
*/

#include <os/simTypesBind.h>

/* define next -- must by before any include of next os H files */
#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#endif

#include <os/simOsBindOwn.h>
#include <os/simOsTask.h>
#include <os/simOsIniFile.h>
#include <os/simOsIntr.h>
#include <os/simOsSlan.h>
#include <os/simOsSync.h>
#include <os/simOsConsole.h>
#include <os/simOsProcess.h>

#define BIND_LEVEL_FUNC(level,funcName)     \
        simOsBindInfo.level.funcName = funcName

/* max tasks num. This value is aligned with hash function below. */
#define SIM_OS_TASKS_MAX_NUM 1024

/* task table is hash one. This MACRO defines hash function:
  - use LSBs of task ID. */
#define SIM_OS_TASKS_ID_HASH_MAC(_tid) (_tid & 0x3FF)

/* get next entry MACRO */
#define SIM_OS_TASKS_NEXT_ENTRY_GET_MAC(_currentIdx) ((_currentIdx + 1) & 0x3FF)


extern GT_STATUS osTaskGetSelf(OUT GT_U32 *tid);
extern GT_STATUS osShellExecute(IN  char*   command);

/**
* @internal osShellExecute function
* @endinternal
*
* @brief   execute command through OS shell
*
* @param[in] command                  - null terminated  string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on timeout
*/
GT_STATUS   simOsShellExecute
(
    IN  char*   command
)
{
    return osShellExecute(command);
}

/**
* @struct SIM_OS_TASKS_INFO_STC
 *
 * @brief Describe a tasks info(like task type)
*/
typedef struct{

    /** task Id */
    GT_U32 taskId;

    /** task type */
    SIM_OS_TASK_PURPOSE_TYPE_ENT taskType;

    /** @brief cookie pointer
     *  Comments:
     */
    GT_PTR cookiePtr;

} SIM_OS_TASKS_INFO_STC;

/* running tasks table */
static SIM_OS_TASKS_INFO_STC simOsTasksInfoArr[SIM_OS_TASKS_MAX_NUM] = {{0}};

/**
* @internal simOsFuncBindOwnSimOs function
* @endinternal
*
* @brief   the functions of simOs will be bound to the simulation
*/
void simOsFuncBindOwnSimOs
(
    void
)
{
    SIM_OS_FUNC_BIND_STC simOsBindInfo;

    /* reset all fields of simOsBindInfo */
    memset(&simOsBindInfo,0,sizeof(simOsBindInfo));

    BIND_LEVEL_FUNC(sockets,simOsSocketSetSocketNoDelay);

    BIND_LEVEL_FUNC(tasks,simOsTaskCreate);
    BIND_LEVEL_FUNC(tasks,simOsTaskDelete);
    BIND_LEVEL_FUNC(tasks,simOsTaskOwnTaskPurposeSet);
    BIND_LEVEL_FUNC(tasks,simOsTaskOwnTaskPurposeGet);
    BIND_LEVEL_FUNC(tasks,simOsSleep);
    BIND_LEVEL_FUNC(tasks,simOsTickGet);
    BIND_LEVEL_FUNC(tasks,simOsAbort);
#ifndef APPLICATION_SIDE_ONLY
    BIND_LEVEL_FUNC(tasks,simOsLaunchApplication);  /* needed only on devices side */
#endif /*!APPLICATION_SIDE_ONLY*/
    BIND_LEVEL_FUNC(tasks,simOsBacktrace);

    BIND_LEVEL_FUNC(sync,simOsSemCreate);
    BIND_LEVEL_FUNC(sync,simOsSemDelete);
    BIND_LEVEL_FUNC(sync,simOsSemWait);
    BIND_LEVEL_FUNC(sync,simOsSemSignal);
    BIND_LEVEL_FUNC(sync,simOsMutexCreate);
    BIND_LEVEL_FUNC(sync,simOsMutexDelete);
    BIND_LEVEL_FUNC(sync,simOsMutexUnlock);
    BIND_LEVEL_FUNC(sync,simOsMutexLock);
    BIND_LEVEL_FUNC(sync,simOsEventCreate);
    BIND_LEVEL_FUNC(sync,simOsEventSet);
    BIND_LEVEL_FUNC(sync,simOsEventWait);
#ifndef APPLICATION_SIDE_ONLY
    BIND_LEVEL_FUNC(sync,simOsSendDataToVisualAsic); /* needed only on devices side */
    BIND_LEVEL_FUNC(sync,simOsTime);                 /* needed only on devices side */

    BIND_LEVEL_FUNC(slan,simOsSlanBind);             /* needed only on devices side */
    BIND_LEVEL_FUNC(slan,simOsSlanTransmit);         /* needed only on devices side */
    BIND_LEVEL_FUNC(slan,simOsSlanUnbind);           /* needed only on devices side */
    BIND_LEVEL_FUNC(slan,simOsSlanInit);             /* needed only on devices side */
    BIND_LEVEL_FUNC(slan,simOsSlanClose);             /* needed only on devices side */
    BIND_LEVEL_FUNC(slan,simOsSlanStart);            /* needed only on devices side */
    BIND_LEVEL_FUNC(slan,simOsChangeLinkStatus);     /* needed only on devices side */
#endif /*!APPLICATION_SIDE_ONLY*/

#ifndef DEVICES_SIDE_ONLY
    BIND_LEVEL_FUNC(interrupts,simOsInterruptSet);  /* needed only on application side */
    BIND_LEVEL_FUNC(interrupts,simOsInitInterrupt); /* needed only on application side */
#endif /*!DEVICES_SIDE_ONLY*/

    BIND_LEVEL_FUNC(iniFile,simOsGetCnfValue);
    BIND_LEVEL_FUNC(iniFile,simOsSetCnfFile);

    BIND_LEVEL_FUNC(console,simOsGetCommandLine);
    BIND_LEVEL_FUNC(console,simOsAllocConsole);
    BIND_LEVEL_FUNC(console,simOsSetConsoleTitle);
    BIND_LEVEL_FUNC(console,simOsShellExecute);

#ifndef DEVICES_SIDE_ONLY
    BIND_LEVEL_FUNC(processes,simOsSharedMemGet);     /* needed only on application side */
    BIND_LEVEL_FUNC(processes,simOsSharedMemAttach);  /* needed only on application side */
    BIND_LEVEL_FUNC(processes,simOsProcessIdGet);     /* needed only on application side */
    BIND_LEVEL_FUNC(processes,simOsProcessNotify);    /* needed only on application side */
    BIND_LEVEL_FUNC(processes,simOsProcessHandler);   /* needed only on application side */
#endif /*!DEVICES_SIDE_ONLY*/

    /* this needed for binding the OS of simulation with our OS functions */
    simOsFuncBind(&simOsBindInfo);
}

/**
* @internal simOsTaskOwnTaskPurposeSet function
* @endinternal
*
* @brief   Sets type of the thread.
*
* @param[in] type                     - task type
*                                      to allow update of only the cookie :
*                                      if SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL____LAST__E for
*                                      already bound thread --> ignored
* @param[in] cookiePtr                - cookie pointer
*                                      to allow update of only the task type :
*                                      if NULL for already bound thread --> ignored
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - fail, should never happen
*/
GT_STATUS simOsTaskOwnTaskPurposeSet
(
    IN SIM_OS_TASK_PURPOSE_TYPE_ENT      type,
    IN SIM_OS_TASK_COOKIE_INFO_STC*      cookiePtr
)
{
    GT_U32  currThread = 0;
    GT_U32  i, entryIdx;

    osTaskGetSelf(&currThread);

    if(0 == currThread)
    {
        return GT_FAIL;
    }


    entryIdx = SIM_OS_TASKS_ID_HASH_MAC(currThread);

    /* search current thread in the table */
    for(i = 0; i < SIM_OS_TASKS_MAX_NUM; i++)
    {
        if(currThread == simOsTasksInfoArr[entryIdx].taskId)
        {
            if(type != SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL____LAST__E)
            {
                simOsTasksInfoArr[entryIdx].taskType = type;
            }
            else
            {
                /* allow the caller to modify only the cookie ... */
            }

            if(cookiePtr)
            {
                simOsTasksInfoArr[entryIdx].cookiePtr = cookiePtr;
            }
            else
            {
                /* allow the caller to modify only the type of task ... */
            }

            return GT_OK;
        }

        entryIdx = SIM_OS_TASKS_NEXT_ENTRY_GET_MAC(entryIdx);
    }

    entryIdx = SIM_OS_TASKS_ID_HASH_MAC(currThread);

    /* add current thread info to the table */
    for(i = 0; i < SIM_OS_TASKS_MAX_NUM; i++)
    {
        if(!simOsTasksInfoArr[entryIdx].taskId)
        {
            simOsTasksInfoArr[entryIdx].taskId   = currThread;
            simOsTasksInfoArr[entryIdx].taskType = type;
            if(cookiePtr)
            {
                simOsTasksInfoArr[entryIdx].cookiePtr = cookiePtr;
                cookiePtr->additionalInfo = GT_TRUE;
            }
            else
            {
                cookiePtr =
                    calloc(1,sizeof(SIM_OS_TASK_COOKIE_INFO_STC));
                simOsTasksInfoArr[entryIdx].cookiePtr = cookiePtr;
                cookiePtr->additionalInfo = GT_FALSE;
            }
            return GT_OK;
        }

        entryIdx = SIM_OS_TASKS_NEXT_ENTRY_GET_MAC(entryIdx);
    }

    /* no more space left in the task table */
    return GT_FAIL;
}

/**
* @internal simOsTaskOwnTaskPurposeGet function
* @endinternal
*
* @brief   Gets type of the thread.
*
* @param[out] type                     - task type
*
* @retval GT_OK                    - success
* @retval GT_BAD_PTR               - wrong pointer
* @retval GT_NOT_FOUND             - task info not found
*/
GT_STATUS simOsTaskOwnTaskPurposeGet
(
    OUT SIM_OS_TASK_PURPOSE_TYPE_ENT   *type
)
{
    GT_U32 currThread = 0;
    GT_U32  i, entryIdx;

    if(NULL == type)
    {
        return GT_BAD_PTR;
    }

    osTaskGetSelf(&currThread);

    if(0 == currThread)
    {
        return GT_FAIL;
    }

    entryIdx = SIM_OS_TASKS_ID_HASH_MAC(currThread);

    for(i = 0; i < SIM_OS_TASKS_MAX_NUM; i++)
    {
        if(currThread == simOsTasksInfoArr[entryIdx].taskId)
        {
            *type = simOsTasksInfoArr[entryIdx].taskType;
            return GT_OK;
        }

        entryIdx = SIM_OS_TASKS_NEXT_ENTRY_GET_MAC(entryIdx);
    }

    *type = SIM_OS_TASK_PURPOSE_TYPE_CPU_APPLICATION_E;
    return GT_OK;
}

/**
* @internal simOsTaskOwnTaskCookieGet function
* @endinternal
*
* @brief   Gets 'cookiePtr' of the thread.
*/
GT_PTR simOsTaskOwnTaskCookieGet
(
    GT_VOID
)
{
    GT_U32 currThread = 0;
    GT_U32  i, entryIdx;

    osTaskGetSelf(&currThread);

    if(0 == currThread)
    {
        return NULL;
    }

    entryIdx = SIM_OS_TASKS_ID_HASH_MAC(currThread);

    for(i = 0; i < SIM_OS_TASKS_MAX_NUM; i++)
    {
        if(currThread == simOsTasksInfoArr[entryIdx].taskId)
        {
            return simOsTasksInfoArr[entryIdx].cookiePtr;
        }

        entryIdx = SIM_OS_TASKS_NEXT_ENTRY_GET_MAC(entryIdx);
    }

    /* not found the task in the registered list */
    return NULL;
}



