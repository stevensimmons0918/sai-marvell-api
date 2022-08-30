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
* @file prvCpssDxChHitlessStartup.h
*
* @brief CPSS DxCh HitlessStartup facility API.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHitlessStartuph
#define __prvCpssDxChHitlessStartuph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

/**
* @enum PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT
 *
 * @brief Init state cpss enums.
 * Used to monitor init state useing MI
*/
typedef enum {
    /** configure by micro init when starts */
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_BOOT_INIT_STATE_IN_PROGRESS_E = 1,
    /** configure by mocro init when done */
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_BOOT_INIT_STATE_DONE_E,
    /** configure by hitless startup when system recovery starts */
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_IN_PROGRESS_E,
    /** configure by hitless startup when system recovery is done */
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E,
} PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT;

/**
* @internal prvCpssDxChHitlessStartupStateCheckAndSet function
* @endinternal
*
* @brief   This function checks expected Init State register value and set new value
*/
GT_STATUS prvCpssDxChHitlessStartupStateCheckAndSet
(
    GT_U8 devNum,
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT expValue,
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT newValue
);

/**
* @internal prvCpssDxChHitlessStartupPortConfiguredCheck function
* @endinternal
*
* @brief   This function checks if port configured by MI
*/
GT_STATUS prvCpssDxChHitlessStartupPortConfiguredCheck
(
    GT_U8                                 devNum,
    GT_PHYSICAL_PORT_NUM                  portNum,
    GT_BOOL                               *isConfigure
);

/**
* @internal prvHitlessStartupMiClose function
* @endinternal
*
* @brief   This function close MI process
*/
GT_STATUS prvHitlessStartupMiClose
(
    GT_U8 devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHitlessStartup */

