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
* @file simOsWin32Task.c
*
* @brief Win32 Operating System Simulation. Task facility implementation.
*
* @version   5
********************************************************************************
*/

/* WA to avoid next warning :
   due to include to : #include <windows.h>
    c:\program files\microsoft visual studio\vc98\include\rpcasync.h(45) :
    warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
*/
struct _RPC_ASYNC_STATE;
#include <windows.h>
#include <process.h>
#include <time.h>
#include <winbase.h>
#include <assert.h>
#include <os/simTypesBind.h>
#include <common/SHOST/HOST_D/EXP/HOST_D.H>


/************* Defines ***********************************************/

/************* Externs ***********************************************/

int PrintCallStack(char *buffer, int bufferLen, int framesToSkip);

/************ Public Functions ************************************************/

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
    HANDLE                      retHThread;
    GT_32                       thredPriority;

    /* Create thread and save Handle */
    retHThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)startaddrPtr,arglistPtr,0,NULL);

    /* A handle to the new thread indicates success. NULL indicates failure. */
    if(retHThread != NULL)
    {

        /* setting the priority of the thread*/
        switch(prio) {
            case   GT_TASK_PRIORITY_ABOVE_NORMAL:
                thredPriority = THREAD_PRIORITY_ABOVE_NORMAL;
                break;
            case    GT_TASK_PRIORITY_BELOW_NORMAL:
                thredPriority = THREAD_PRIORITY_BELOW_NORMAL;
                break;
            case    GT_TASK_PRIORITY_HIGHEST:
                thredPriority = THREAD_PRIORITY_HIGHEST;
                break;
            case    GT_TASK_PRIORITY_IDLE:
                thredPriority = THREAD_PRIORITY_IDLE;
                break;
            case    GT_TASK_PRIORITY_LOWEST:
                thredPriority = THREAD_PRIORITY_LOWEST;
                break;
            case    GT_TASK_PRIORITY_NORMAL:
                thredPriority = THREAD_PRIORITY_NORMAL;
                break;
            case     GT_TASK_PRIORITY_TIME_CRITICAL:
                thredPriority = THREAD_PRIORITY_TIME_CRITICAL;
                break;
            default:
                thredPriority = THREAD_PRIORITY_NORMAL;
                break;
        }

        SetThreadPriority(retHThread,  thredPriority);
    }


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
    return (TerminateThread(hThread, 0) == 0) ? GT_FAIL : GT_OK;
}

/**
* @internal simOsSleep function
* @endinternal
*
* @brief   Puts current task to sleep for specified number of millisecond.
*
* @param[in] timeOut                  - time to sleep in milliseconds
*                                       None
*/
void simOsSleep
(
    IN GT_U32 timeOut
)
{
    Sleep(timeOut);

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
    clock_t clockT = clock();

    if(clockT==-1)
    {
        return 0;
    }

    return clockT;
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
    SHOSTG_abort();
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
    WinExec(fileName, 0);
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
    int        ret,err;

    ret = setsockopt (socketFd, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay,
                    sizeof (noDelay));
    if(ret != 0)
    {
        err = WSAGetLastError();
        return GT_FAIL;
    }

    return GT_OK ;
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
#ifdef _VISUALC
    GT_U32 buflen = 0;
#ifdef PRINT_CALL_STACK_USED
    buflen = PrintCallStack(buffer, bufferLen, maxFramesPrint);
#endif /* PRINT_CALL_STACK_USED */
    return buflen;
#else
    return 0;
#endif
}


