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
* @file simOsTask.h
*
* @brief Operating System wrapper. Task facility.
*
* @version   4
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsTaskh
#define __simOsTaskh

/************* Includes *******************************************************/
#include <os/simTypes.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
* @internal simOsTaskCreate function
* @endinternal
*
* @brief   Create OS Task/Thread and start it.
*
* @param[in] prio                     - task priority 255 - 0 => HIGH- 0 => HIGH
*                                      start_addr - task Function to execute
*                                      arglist    - pointer to list of parameters for task function
*
* @retval GT_TASK_HAND             - handler to the task
*/
GT_TASK_HANDLE simOsTaskCreate
(
    IN  GT_TASK_PRIORITY_ENT prio,
    IN  unsigned (__TASKCONV *startaddrPtr)(void*),
    IN  void    *arglistPtr
);


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
);

/**
* @internal simOsSleep function
* @endinternal
*
* @brief   Puts current task to sleep for specified number of millisecond.
*
* @param[in] mils                     - time to sleep in milliseconds
*                                       None
*/
void simOsSleep
(
    IN GT_U32 mils
);

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
);

/**
* @internal simOsAbort function
* @endinternal
*
* @brief   Perform Warm start up on operational mode and halt in debug mode.
*/
void simOsAbort
(
    void
);

/**
* @internal simOsLaunchApplication function
* @endinternal
*
* @brief   launch application
*/
void simOsLaunchApplication
(
    char * fileName
);

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
);

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
);


#ifdef __cplusplus
}
#endif

#endif  /* __simOsTaskh */


