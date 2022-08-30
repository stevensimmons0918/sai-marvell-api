/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxVersion.c
*
* @brief Implements software CPSS PX version information.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/version/cpssPxVersion.h>
#include <cpss/px/version/private/prvCpssPxVersionLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal internal_cpssPxVersionGet function
* @endinternal
*
* @brief   This function returns CPSS PX version.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] versionPtr               - (pointer to) CPSS PX version info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one parameter is NULL pointer
* @retval GT_BAD_SIZE              - the version name is too long
*/
static GT_STATUS internal_cpssPxVersionGet
(
    OUT GT_VERSION  *versionPtr
)
{
    GT_U32  versionLen = cpssOsStrlen(CPSS_PX_VERSION_CNS);

    if (versionPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if ((versionLen + 1) > VERSION_MAX_LEN)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
    }

    cpssOsMemCpy(versionPtr->version, CPSS_PX_VERSION_CNS, versionLen + 1);

    return GT_OK;
}

/**
* @internal cpssPxVersionGet function
* @endinternal
*
* @brief   This function returns CPSS PX version.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] versionPtr               - (pointer to) CPSS PX version info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one parameter is NULL pointer
* @retval GT_BAD_SIZE              - the version name is too long
*/
GT_STATUS cpssPxVersionGet
(
    OUT GT_VERSION  *versionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxVersionGet);

    /*This API must use zero level protection*/
    CPSS_ZERO_LEVEL_API_LOCK_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId, versionPtr));

    rc = internal_cpssPxVersionGet(versionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, versionPtr));
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return rc;
}

