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
* @file simOsBindOwn.h
*
* @brief allow bind of the simOs functions to the simulation
*
*
* @version   1
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsBindOwnh
#define __simOsBindOwnh

/************* Includes *******************************************************/

#include <os/simTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @internal simOsFuncBindOwnSimOs function
* @endinternal
*
* @brief   the functions of simOs will be bound to the simulation
*/
void simOsFuncBindOwnSimOs
(
    void
);

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
);

/**
* @internal simOsTaskOwnTaskPurposeGet function
* @endinternal
*
* @brief   Gets type of the thread.
*
* @param[out] type                     - task type
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - fail, should never happen
*/
GT_STATUS simOsTaskOwnTaskPurposeGet
(
    OUT SIM_OS_TASK_PURPOSE_TYPE_ENT   *type
);


#ifdef __cplusplus
}
#endif

#endif  /* __simOsBindOwnh */


