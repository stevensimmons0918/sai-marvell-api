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

/*******************************************************************************
* simOsWin32Task.c
*
* DESCRIPTION:
*       Operating System wrapper. Ini file facility.
*
* DEPENDENCIES:
*       Win32, CPU independed.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#include <os/simTypes.h>
#include <asicSimulation/SInit/sinit.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsConsole.h>

/************* Defines ***********************************************/

/************ Public Functions ************************************************/
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
*        A pointer to the command line
*
* COMMENTS:
*       None
*
*******************************************************************************/
char *simOsGetCommandLine
(
    void
)
{
    FUNCTION_NOT_SUPPORTED(simOsGetCommandLine);

    return "";
}
/**
* @internal simOsAllocConsole function
* @endinternal
*
* @brief   Sets the config file name.
*
* @retval GT_OK                    - if succeeds
* @retval GT_FAIL                  - if fails
*/
GT_STATUS simOsAllocConsole
(
    void
)
{
    return GT_NOT_SUPPORTED;
}
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
)
{
    return GT_NOT_SUPPORTED;
}


