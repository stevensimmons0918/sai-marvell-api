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
* @file simOsConsole.h
*
* @brief Operating System wrapper. Console facility.
*
* @version   1
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsConsoleh
#define __simOsConsoleh

/************* Includes *******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/************ Defines  ********************************************************/

/************ Functions *******************************************************/
/*******************************************************************************
* simOsGetCommandLine
*
* DESCRIPTION:
*       Gets a command line.
*
* INPUTS:
*       None
*
* OUTPUTS:
*
* RETURNS:
*       A pointer to the command line
*
* COMMENTS:
*       None
*
*******************************************************************************/
char *simOsGetCommandLine
(
    void
);
/**
* @internal simOsAllocConsole function
* @endinternal
*
* @brief   Allocates a new console for the calling process.
*
* @retval GT_OK                    - if succeeds
* @retval GT_FAIL                  - if fails
*/
GT_STATUS simOsAllocConsole
(
    void
);
/**
* @internal simOsSetConsoleTitle function
* @endinternal
*
* @brief   Displays the string in the title bar of the console window.
*
* @param[in] consoleTitlePtr          - pointer to the title string
*
* @retval GT_OK                    - if succeeds
* @retval GT_FAIL                  - if fails
*/
GT_STATUS simOsSetConsoleTitle
(
    IN char *consoleTitlePtr
);


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
);


#ifdef __cplusplus
}
#endif

#endif  /* __simOsConsoleh */


