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
* @file simOsLinuxTask.c
*
* @brief Linux Operating System Simulation. Task facility implementation.
*
* @version   15
********************************************************************************
*/

#include <time.h>
#include <assert.h>
#include <os/simTypes.h>
#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsTask.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#ifdef  LINUX
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <os/simTypes.h>
#include <os/simOsTask.h>

#ifdef SHARED_MEMORY
    /* call LIBC free() to free memory allocated by backtrace_symbols() */
    void __libc_free(void *ptr);
#define free   __libc_free
#endif

extern GT_STATUS osTaskCreate
(
    IN  char    *name,
    IN  GT_U32  prio,
    IN  GT_U32  stack,
    IN  unsigned (__TASKCONV *start_addr)(void*),
    IN  void    *arglist,
    OUT GT_TASK_ID *tid
);

extern GT_STATUS osTaskDelete
(
    IN GT_TASK_ID tid
);

extern GT_STATUS osTimerWkAfter
(
        IN GT_U32 mils
);

extern GT_U32 osTickGet(void);

extern GT_STATUS osTimeRT
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
);

/************* Defines ***********************************************/

/************ Public Functions ************************************************/
#define MAX_TASKS_CNS   256
#define INVALID_HANDLE_CNS (GT_TASK_HANDLE)0xFFFFFFFF
static GT_U32         numOfTasks=0;
static GT_TASK_HANDLE tasksIdArr[MAX_TASKS_CNS];

/**
* @internal simOsTaskCreate function
* @endinternal
*
* @brief   Create OS Task and start it.
*
* @param[in] prio                     - task priority 255 - 0 => HIGH- 0 => HIGH
*                                      start_addr - task Function to execute
*                                      arglist    - pointer to list of parameters for task function
*
* @retval GT_TASK_HANDLE           - the handle of the thread
*/
GT_TASK_HANDLE simOsTaskCreate
(
    IN  GT_TASK_PRIORITY_ENT prio,
    IN  unsigned (__TASKCONV *startaddrPtr)(void*),
    IN  void    *arglistPtr
)
{
    HANDLE                      retHThread = 0;
    GT_32                       thredPriority;
    GT_CHAR  taskName[16];
    GT_U32   ii;

    if(numOfTasks >= MAX_TASKS_CNS)
    {
        printf("simOsTaskCreate : ERROR : task table size [%d] not enough ... need more \n",
            MAX_TASKS_CNS);
        return 0;
    }

    sprintf(taskName, "task%d", numOfTasks);

    /* setting the priority of the thread*/
    switch(prio)
    {
        case   GT_TASK_PRIORITY_ABOVE_NORMAL:
        thredPriority = 160;
                break;
        case    GT_TASK_PRIORITY_BELOW_NORMAL:
                thredPriority = 180;
                break;
        case    GT_TASK_PRIORITY_HIGHEST:
                thredPriority = 150;
                break;
        case    GT_TASK_PRIORITY_IDLE:
                thredPriority = 200;
                break;
        case    GT_TASK_PRIORITY_LOWEST:
                thredPriority = 190;
                break;
        case    GT_TASK_PRIORITY_NORMAL:
                thredPriority = 170;
                break;
        case     GT_TASK_PRIORITY_TIME_CRITICAL:
                thredPriority = 140;
                break;
        default:
                thredPriority = 170;
                break;

    }

    if (osTaskCreate(taskName, thredPriority, 0x40000,
                        startaddrPtr, arglistPtr,
                        (GT_TASK_ID*)(void*)&retHThread) != GT_OK)
    {
        return 0;
    }

    /* save the task ID , to allow 'kill all' */
    /* increment task counter */
    for (ii = 0; (ii < numOfTasks); ii++)
    {
        if (tasksIdArr[ii] == INVALID_HANDLE_CNS)
        {
            tasksIdArr[ii] = retHThread;
            return retHThread;
        }
    }
    /* free place in the array not found */
    /* increment task counter           */
    tasksIdArr[numOfTasks++] = retHThread;

    return retHThread;
}

/**
* @internal simOsTaskDelete function
* @endinternal
*
* @brief   Deletes existing task.
*
* @param[in] hThread                  - the handle of the thread
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simOsTaskDelete
(
    IN GT_TASK_HANDLE hThread
)
{
    GT_U32  ii;

    if(hThread == (GT_TASK_HANDLE)(0xdeadbeef))/* kill all the WM tasks */
    {
        printf("Start : Killing ALL WM registered tasks : \n");
        for(ii = 0 ; ii < numOfTasks; ii++)
        {
            if (tasksIdArr[ii] == INVALID_HANDLE_CNS)
            {
                continue;
            }
            printf("Kill TaskId[%d] \n", ii);
            osTaskDelete((GT_TASK_ID)tasksIdArr[ii]);
            simOsSleep(200);
        }
        printf("Done : Killing ALL WM registered tasks \n");

        return GT_OK;
    }

    for (ii = 0; (ii < numOfTasks); ii++)
    {
        if (tasksIdArr[ii] == hThread)
        {
            tasksIdArr[ii] = INVALID_HANDLE_CNS;
        }
    }

    return osTaskDelete((GT_TASK_ID)hThread);
}

/**
* @internal simOsSleep function
* @endinternal
*
* @brief   Puts current task to sleep for specified number of milisecond.
*
* @param[in] timeOut                  - time to sleep in milliseconds
*                                       None
*/
void simOsSleep
(
    IN GT_U32 timeOut
)
{
    osTimerWkAfter(timeOut);

}

/**
* @internal simOsTickGet function
* @endinternal
*
* @brief Gets time in milliseconds. (from start of process or start of OS depend on the OS)
*       need to be used only as 'diff between time A and time B' and not as absolute time.
*/
GT_U32 simOsTickGet
(
    void
)
{
    GT_U32 milliSeconds; /* tick counter in milliseconds */
    GT_U32 seconds;      /* tick counter in seconds */
    GT_U32 nanoSeconds;  /* tick counter in nanoseconds */

    /* the Linux tick is 10 millisecond.
      Use nanosecond counter to get exact number of milliseconds.  */
    osTimeRT(&seconds, &nanoSeconds);

    milliSeconds = (seconds * 1000) + (nanoSeconds / 1000000);

    return milliSeconds;
}

/**
* @internal simOsAbort function
* @endinternal
*
* @brief   Perform Warm start up on operational mode and halt in debug mode.
*/
void simOsAbort
(
    void
)
{
    abort();
}
/**
* @internal simOsLaunchApplication function
* @endinternal
*
* @brief   launch application
*/
void simOsLaunchApplication
(
    char * fileName
)
{
    if (system(fileName) == 0)
    {
        exit(1);
    }
}

/**
* @internal simOsTime function
* @endinternal
*
* @brief   Gets number of seconds passed since system boot
*/
GT_U32 simOsTime(void)
{
    time_t t ;
    return (GT_U32) time(&t);
}

/**
* @internal simOsSocketSetSocketNoDelay function
* @endinternal
*
* @brief   Set the socket option to be no delay.
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS simOsSocketSetSocketNoDelay
(
    IN  GT_SOCKET_FD     socketFd
)
{
    int        noDelay=1;
    int        ret;
    int        level = IPPROTO_TCP;

    ret = setsockopt(socketFd, level, TCP_NODELAY, (char*)&noDelay,
                     sizeof (noDelay));
    if(ret != 0)
    {
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal simOsBacktrace function
* @endinternal
*
* @brief   return buffer with backtrace
*
* @param[in] maxFramesPrint           - max number of frames to print
* @param[in] bufferLen                - length of buffer
*
* @param[out] buffer                   -  with backtrace info
*
* @retval buflen                   - size of returned data in buffer
*/
GT_U32 simOsBacktrace
(
    IN  GT_U32   maxFramesPrint,
    OUT GT_CHAR *buffer,
    IN  GT_U32   bufferLen
)
{
#ifdef  LINUX
    void    *arr[100];
    int     nptrs, j, len;
    char    **strings;
    GT_U32  ret = 0;

    nptrs = backtrace(arr, (maxFramesPrint < 100) ? maxFramesPrint+1 : 100);
    strings = backtrace_symbols(arr, nptrs);
    for (j = 1; j < nptrs; j++)
    {
        len = strlen(strings[j]);
        if (len + 1 >= bufferLen - ret)
            len = bufferLen - ret - 1;
        memcpy(buffer + ret, strings[j], len);
        ret += len;
        buffer[ret++] = '\n';
    }
    free(strings);
    if (ret < bufferLen)
        buffer[ret] = 0;
    return ret;

#else    /* !defined(LINUX) */
    return 0;
#endif  /* !defined(LINUX) */
}

