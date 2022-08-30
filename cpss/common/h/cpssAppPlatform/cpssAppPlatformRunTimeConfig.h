/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformRunTimeConfig.h
*
* @brief CPSS Application platform - run time configuration functions
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_RUNTIME_CONFIG_H
#define __CPSS_APP_PLATFORM_RUNTIME_CONFIG_H

#include <profiles/cpssAppPlatformProfile.h>

/**
 * @struct APP_PLATFORM_UNI_EVENT_COUNTER_STC
 *
 * @brief Used to store the event counter's info during event counter increment.
 */
typedef struct APP_PLATFORM_UNI_EVENT_COUNTER_STCT
{
    /** @brief event counter value */
    GT_U32 counterValue;

    /** @brief event extended data */
    GT_U32 extData;

    /** @brief pointer to the next counter entry */
    struct APP_PLATFORM_UNI_EVENT_COUNTER_STCT *nextEntryPtr;

}APP_PLATFORM_UNI_EVENT_COUNTER_STC;

/* CPSS Applicaiton Platform - PP Run Time configuration APIs */

/*
* @internal cpssAppPlatformUserEventsInit function
* @endinternal
*
* @brief   Initialize user event handlers.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventsInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileListPtr
);

/**
* @internal cpssAppPlatformEventHandlerReset function
* @endinternal
*
* @brief   This routine deletes event handlers when last PP
*          device is removed. It also deletes event counters.
*
* @param[in] devNum           - device number
*
* @retval GT_OK               - on success,
* @retval GT_FAIL             - otherwise.
*/
GT_STATUS cpssAppPlatformEventHandlerReset
(
    IN GT_U8 devNum
);

/*
* @internal cpssAppPlatformPortManagerInit function
* @endinternal
*
* @brief   initialize port manager.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*
*/
GT_STATUS cpssAppPlatformPortManagerInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr
);

/*
* @internal cpssAppPlatformTrafficEnable function
* @endinternal
*
* @brief   Enable traffic for given ports.
*
* @param [in] *trafficEnablePtr - traffic enable handle ptr
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformTrafficEnable
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
);

#endif /* __CPSS_APP_PLATFORM_RUNTIME_CONFIG_H */
