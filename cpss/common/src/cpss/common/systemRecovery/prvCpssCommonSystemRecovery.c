/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssCommonSystemRecovery.c
*
* @brief Generic System Recovery ApIs
*
* @version   1
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#endif /*CHX_FAMILY*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



extern void exit(int);/* should be OS function */
extern GT_STATUS appDemoHaEmulateSwCrash
(
    GT_VOID
);

#ifndef ASIC_SIMULATION
#define PRV_SYSTEM_RECOVERY_ERROR_INJECTION_TRIGGER_FILE_NAME "/usr/bin/system_recovery_error_trigger.dbg"
#else
#define PRV_SYSTEM_RECOVERY_ERROR_INJECTION_TRIGGER_FILE_NAME "system_recovery_error_trigger.dbg"
#endif

/* this function check if errorInjectionIndex is set in errorBmp DB */
static GT_BOOL checkerrorInjectionBtm
(
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_ENT errorInjectionIndex
)
{
    GT_U32 bmpOffset = 0 ,bitOffset = 0 ;

    bmpOffset = errorInjectionIndex / 32 ;
    bitOffset = errorInjectionIndex % 32;

    if((PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[bmpOffset] & (1 << bitOffset)) != 0)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}

/**
* @internal prvCpssSystemRecoveryInProgress function
* @endinternal
*
* @brief   This function inform if any system recovery process
*          take place
*
* @retval GT_TRUE       - system recovery process is going.
* @retval GT_FALSE      - otherwise.
*
*/
GT_BOOL  prvCpssSystemRecoveryInProgress
(
    GT_VOID
)
{
    CPSS_SYSTEM_RECOVERY_INFO_STC systemRecoveryInfo;
    cpssSystemRecoveryStateGet(&systemRecoveryInfo);

    return systemRecoveryInfo.systemRecoveryProcess!=CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
}

/**
* @internal prvCpssSystemRecoveryErrorInjectionCheck function
* @endinternal
*
* @brief   This function get error index and check if it match system recovery errorBmp DB
*          if match trigger SW crash (OS only)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - in case errorIndex not valid
* @retval GT_NOT_SUPPORTED         - in case of simulation ,doesn't crash SW only return error
*
*/
GT_STATUS prvCpssSystemRecoveryErrorInjectionCheck
(
    IN  PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_ENT errorInjectionIndex
)
{
    CPSS_OS_FILE_TYPE_STC file;

    /*2 step check : 1)check if error bit is set
                     2)check if trigger file is found */
    if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorInjectionEnable == GT_TRUE)
    {
        if (errorInjectionIndex > PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_LAST_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("\nerrorInjectionEnable bit is set ");

        file.type = CPSS_OS_FILE_REGULAR;
        file.fd = cpssOsFopen(PRV_SYSTEM_RECOVERY_ERROR_INJECTION_TRIGGER_FILE_NAME, "r",&file);

        if (file.fd!=CPSS_OS_FILE_INVALID )
        {
            CPSS_LOG_INFORMATION_MAC("\nFile:%s found \n",PRV_SYSTEM_RECOVERY_ERROR_INJECTION_TRIGGER_FILE_NAME);
            cpssOsFclose(&file);

            CPSS_LOG_INFORMATION_MAC("\ncheck error index [%d] ,DB error bitmap [0x%x][0x%x][0x%x] ",
                             errorInjectionIndex,
                             PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[0],
                             PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[1],
                             PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[2]);

            if (checkerrorInjectionBtm(errorInjectionIndex))
            {
                CPSS_LOG_INFORMATION_MAC("\nerror Injection activated ,\nerror index [%d] ,DB error bitmap [0x%x][0x%x][0x%x] ",
                                         errorInjectionIndex,
                                         PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[0],
                                         PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[1],
                                         PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection.errorBmp[2]);

#ifndef ASIC_SIMULATION
                exit(0);
#else
                CPSS_LOG_INFORMATION_MAC("\nSW crash is not supported in simulation ,return GT_NOT_SUPPORTED");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#endif
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssSystemRecoveryErrorInjectionSet function
* @endinternal
*
* @brief   This function set error in system recovery error DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
*
*/
GT_STATUS  prvCpssSystemRecoveryErrorInjectionSet
(
    IN CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC errorBmp
)
{

    GT_BOOL errorFound = GT_FALSE;
    GT_U32 ii;

    /*check params */
    for (ii=0;ii<MAX_ERROR_INJECTION_BMP_SIZE;ii++)
    {
        if (errorBmp.errorBmp[ii]!=0)
        {
            errorFound=GT_TRUE;
            break;
        }
    }
    if (errorBmp.errorInjectionEnable != errorFound )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemCpy(&(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection), &errorBmp, sizeof(CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC));
    return GT_OK;
}

/**
* @internal prvCpssSystemRecoveryErrorInjectionGet function
* @endinternal
*
* @brief   This function Get errorBmp from system recovery error DB
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS prvCpssSystemRecoveryErrorInjectionGet
(
    INOUT CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC   *errorBmpPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(errorBmpPtr);

    *errorBmpPtr = PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.errorInjection;
    return GT_OK;
}

/**
* @internal prvCpssSystemRecoveryDebugModeSkipVersionCheck function
* @endinternal
*
* @brief   This function enable to skip hs Version Check (HS recovery process)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssSystemRecoveryDebugModeSkipVersionCheck
(
    IN GT_BOOL   enable
)
{
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG.hsSkipVersionCheck = enable;
    return GT_OK;
}

/**
* @internal prvCpssSystemRecoveryDebugModeSkipMiReset function
* @endinternal
*
* @brief   This function enable to skip hs Mi Reset (HS recovery process)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssSystemRecoveryDebugModeSkipMiReset
(
    IN GT_BOOL   enable
)
{
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG.hsSkipMiReset = enable;
    return GT_OK;
}

/**
* @internal prvCpssSystemRecoveryDebugModeSkipDfxInitStateCheck function
* @endinternal
*
* @brief   This function enable to skip DFX init state check (HS recovery process)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssSystemRecoveryDebugModeSkipDfxInitStateCheck
(
    IN GT_BOOL   enable
)
{
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG.hsSkipDfxInitStateCheck = enable;
    return GT_OK;
}
