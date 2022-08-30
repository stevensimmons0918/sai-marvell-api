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
* @file cpssAppPlatformLogLib.c
*
* @brief CPSS Application Platform Logging Facility Implementation
*
* @version   1
********************************************************************************
*/

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformLogLib.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


CPSS_APP_PLATFORM_LOG_LEVEL_ENT capLogDb [CPSS_APP_PLATFORM_LOG_MODULE_LAST_E] = {
                  CPSS_APP_PLATFORM_LOG_LEVEL_ERR_E | CPSS_APP_PLATFORM_LOG_LEVEL_INFO_E | CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E
                 ,CPSS_APP_PLATFORM_LOG_LEVEL_ERR_E | CPSS_APP_PLATFORM_LOG_LEVEL_INFO_E | CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E
                 ,CPSS_APP_PLATFORM_LOG_LEVEL_ERR_E | CPSS_APP_PLATFORM_LOG_LEVEL_INFO_E | CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E
                };

GT_STATUS cpssAppPlatformLogSet
(
    IN  CPSS_APP_PLATFORM_LOG_MODULE_ENT module,
    IN  CPSS_APP_PLATFORM_LOG_LEVEL_ENT trace_mask
)
{
       if (module < CPSS_APP_PLATFORM_LOG_MODULE_LAST_E)
           capLogDb[module] = trace_mask | CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E;
       else
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

       return GT_OK;
}

