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
* @file simOsIntr.h
*
* @brief Operating System wrapper. Interrupt facility.
*
* @version   2
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsIntrh
#define __simOsIntrh

/************* Includes *******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/************ Defines  ********************************************************/

/************* Functions ******************************************************/

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
);
/**
* @internal simOsInitInterrupt function
* @endinternal
*
*/
void simOsInitInterrupt
(
    void
);

#ifdef __cplusplus
}
#endif

#endif  /* __simOsIntrh */


