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
* @file cpssAppPlatformBoardConfig.h
*
* @brief CPSS Application platform - Board initialization function
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_BOARDCONFIG_H
#define __CPSS_APP_PLATFORM_BOARDCONFIG_H

#include <profiles/cpssAppPlatformProfile.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* CPSS Application Platform - Board configuration APIs */
/*
* @internal cpssAppPlatformBoardInit function
* @endinternal
*
* @brief   initialize the board OS and External Services.
*
* @param[in] inputProfileList    - Profile list containing board profile(s).
* @param[in] systemRecovery      - system recovery mode.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if board profile is not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformBoardInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileList,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC  *systemRecovery
);

/*
* @internal cpssAppPlatformOsCbGet function
* @endinternal
*
* @brief   Get OS callbacks.
*
* @param[in] osCallType     - OS call type static or shared.
*
* @param[OUT] osFuncBindPtr - OS callback functions.
*
* @retval GT_OK             - on success.
* @retval GT_FAIL           - otherwise.
*/
GT_STATUS cpssAppPlatformOsCbGet
(
    IN  CPSS_APP_PLATFORM_OS_CALL_TYPE_ENT  osCallType,
    OUT CPSS_OS_FUNC_BIND_STC              *osFuncBindPtr
);

/*
* @internal cpssAppPlatformExtDrvCbGet function
* @endinternal
*
* @brief   Get External services callbacks.
*
* @param[in] drvCallType   - external driver type static or shared.
*
* @param[OUT] extDrvFuncBindInfoPtr - external driver callback functions.
*
* @retval GT_OK                     - on success.
* @retval GT_FAIL                   - otherwise.
*/
GT_STATUS cpssAppPlatformExtDrvCbGet
(
    IN  CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_ENT  drvCallType,
    OUT CPSS_EXT_DRV_FUNC_BIND_STC              *extDrvFuncBindInfoPtr
);

#endif /* __CPSS_APP_PLATFORM_BOARDCONFIG_H */
