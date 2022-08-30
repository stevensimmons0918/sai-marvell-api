/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* simOsWin32Task.c
*
* DESCRIPTION:
*       Win32 Operating System Simulation. Task facility implementation.
*
* DEPENDENCIES:
*       Win32, CPU independed.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <os/simTypesBind.h>
#include <GEN/INTR/EXP/INTR.H>
/* calling this function will create the DPmutex , that needed for correct use
    inside the SHOST code */
extern void   SHOSTG_psos_pre_init(void);
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
        return GT_FAIL;
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
    /* calling this function will create the DPmutex , that needed for correct use
       inside the SHOST code */
    SHOSTG_psos_pre_init();

    /* calling this function initialize the interrupts table (in the SHOST) */
    SHOSTC_init_intr_table();
}


