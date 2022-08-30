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
* @file simOsLinuxIntr.c
*
* @brief Linux Operating System Simulation. Task facility implementation.
*
* @version   5
********************************************************************************
*/

#include <os/simTypes.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsIntr.h>
#include <gtOs/linuxStubs.h>

/************* Externals *********************************************/
extern UINT_32  SHOSTG_set_interrupt (UINT_32 intr);
extern void     SHOSTC_init_intr_table (void);
extern void     exit (int __status);
extern void     simOsTaskLibInit (void);

/************* Defines ***********************************************/

/************ Public Functions ************************************************/

/**
* @internal simOsInterruptSet function
* @endinternal
*
* @brief   Create OS Task/Thread and start it.
*
* @param[in] intr                     - interrupt
*
* @retval GT_OK                    - Succeed
* @retval GT_FAIL                  - failed
*/
GT_STATUS simOsInterruptSet
(
    IN  GT_U32 intr
)
{

    if(SHOSTG_set_interrupt(intr))
        return GT_OK;
    else
        return GT_FALSE;
}

/**
* @internal simOsInitInterrupt function
* @endinternal
*
*/
void simOsInitInterrupt
(
    void
)
{
    SHOSTC_init_intr_table();
}


