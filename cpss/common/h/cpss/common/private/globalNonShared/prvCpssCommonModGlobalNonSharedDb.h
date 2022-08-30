/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file prvCpssCommonModGlobalNonSharedDb.h
*
* @brief This file define common module non shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssCommonModGlobalNonSharedDb
#define __prvCpssCommonModGlobalNonSharedDb

#ifdef SHARED_MEMORY
#include <sys/types.h>
#include <unistd.h>
#endif
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>

/**
* @struct PRV_CPSS_DRV_COMMON_MOD_INT_GENERIC_DIR_NON_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are NOT meant to be shared
* between processes (when CPSS is linked as shared object)
* in driver module, interrupt generic directory
*/
typedef struct
{
    PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC eventMaskDeviceFuncArray[PRV_CPSS_DEVICE_TYPE_LAST_E];
    PRV_CPSS_EVENT_MASK_DEVICE_GET_FUNC eventMaskGetDeviceFuncArray[PRV_CPSS_DEVICE_TYPE_LAST_E];
    PRV_CPSS_EVENT_GENERATE_DEVICE_SET_FUNC eventGenerateDeviceFuncArray[PRV_CPSS_DEVICE_TYPE_LAST_E];
    PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC eventGppIsrConnectFunc;
    PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC prvCpssEventExtDataConvertFunc;
    PRV_CPSS_EVENT_MAC_EXT_DATA_CONVERT_FUNC prvCpssMacEventExtDataConvertFunc;
} PRV_CPSS_DRV_COMMON_MOD_INT_GENERIC_DIR_NON_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_DRV_COMMON_MOD_INT_DIR_NON_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are NOT meant to be shared
* between processes (when CPSS is linked as shared object)
* in driver module, interrupt directory
*/
typedef struct
{
    PRV_CPSS_DRV_COMMON_MOD_INT_GENERIC_DIR_NON_SHARED_GLOBAL_DB genericInterruptsSrc;
} PRV_CPSS_DRV_COMMON_MOD_INT_DIR_NON_SHARED_GLOBAL_DB;



/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_NON_SHARED_SILICON_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain function pointers  that are not  meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvHwsSiliconIf.c
*/
typedef struct
{
    MV_HWS_REG_ADDR_FIELD_STC       *mvHwsRegDb[LAST_UNIT];
    MV_OS_TIME_WK_AFTER_FUNC        hwsOsTimerWkFuncGlobalPtr;
    MV_OS_MEM_SET_FUNC              hwsOsMemSetFuncGlobalPtr;
    MV_OS_FREE_FUNC                 hwsOsFreeFuncGlobalPtr;
    MV_OS_MALLOC_FUNC               hwsOsMallocFuncGlobalPtr;
    MV_OS_MEM_COPY_FUNC             hwsOsMemCopyFuncGlobalPtr;
    MV_OS_STR_CAT_FUNC              hwsOsStrCatFuncGlobalPtr;
    MV_TIMER_GET                    hwsTimerGetFuncGlobalPtr;
    MV_REG_ACCESS_SET               hwsRegisterSetFuncGlobalPtr;
    MV_REG_ACCESS_GET               hwsRegisterGetFuncGlobalPtr;
    MV_SERDES_REG_ACCESS_SET        hwsSerdesRegSetFuncGlobalPtr;
    MV_SERDES_REG_ACCESS_GET        hwsSerdesRegGetFuncGlobalPtr;
    MV_HWS_DEV_FUNC_PTRS            hwsDevFunc[LAST_SIL_TYPE];/* device specific functions pointers */
    GT_BOOL                         devFuncInitDone;/*used  in hwsDeviceSpecGetFuncPtr*/

} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_NON_SHARED_SILICON_IF_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_PCS_DIR_NON_SHARED_PCS_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,serdes directory, src file :
 *   mvHwsPcsIf.c
*/
typedef struct
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr[HWS_MAX_DEVICE_NUM][LAST_PCS];
} PRV_CPSS_COMMON_MOD_PCS_DIR_NON_SHARED_PCS_IF_SRC_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_PCS_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,pcs directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_PCS_DIR_NON_SHARED_PCS_IF_SRC_GLOBAL_DB mvHwsPcsIfSrc;
} PRV_CPSS_COMMON_MOD_PCS_DIR_NON_SHARED_GLOBAL_DB;

typedef struct
{
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_NON_SHARED_SILICON_IF_SRC_GLOBAL_DB mvHwsSiliconIfSrc;
} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB;

#ifdef CPSS_LOG_ENABLE

/**
* @struct PRV_CPSS_DRV_COMMON_MOD_LOG_NON_VOLATILE_GLOBAL_DB
*
* @brief  Structure contain global non volatile variables that are not meant to be shared
* between processes (when CPSS is linked as shared object)
* in common module, log  directory , source file cpssLog.c
*/
typedef struct
{
    /** Global state of CPSS Log */
    GT_BOOL                 prvCpssLogEnabled;

    /** Log format for API logging */
    CPSS_LOG_API_FORMAT_ENT prvCpssLogFormat;

    /** stack size of log */
    GT_U32 prvCpssLogStackSize;

    /** matrix of enablers for log functions */
    GT_BOOL prvCpssLogLibsLogsTypesTable[CPSS_LOG_LIB_ALL_E][CPSS_LOG_TYPE_ALL_E];

    /** indication that the 'ERROR' needed from all LIBs (registered LIBs and unregistered LIBs) */
    GT_BOOL prvCpssLogLibsLogs_errorAllEnable;

    /** CPSS Log TAB Index */
    GT_U32 prvCpssLogTabIndex;

    /** use a line number in some log messages */
    GT_BOOL prvCpssLogLineNumberEnabled;

    /** format of pointer values in the log */
    CPSS_LOG_POINTER_FORMAT_ENT prvCpssLogPointerFormat;

    /** Context entry data base pool ID */
    CPSS_BM_POOL_ID                 prvCpssLogDbPoolId;

    /** AVL tree pool ID */
    CPSS_BM_POOL_ID                 prvCpssLogAvlPoolId;

    /** AVL tree ID */
    PRV_CPSS_AVL_TREE_ID            prvCpssLogAvlTreeId;

    /** current log context entry */
    PRV_CPSS_LOG_CONTEXT_ENTRY_STC  * prvCpssLogContextEntryPtr;

    /** Current time format  */
    CPSS_LOG_TIME_FORMAT_ENT        timeFormat;

    /** @brief GT_TRUE - entry level function is not logged,
      GT_FALSE - output all types of log */
    GT_BOOL prvCpssLogSilent;

#ifdef SHARED_MEMORY
    /* Those variables are accesed before DB init(at prvCpssLogInit which is called from cpssExtServicesBind).
             So need to be a part of non volatile DB*/
    CPSS_OS_MUTEX            prvLogCbMtx;       /*TODO: init */
    CPSS_OS_SIG_SEM          prvLogCbHandleSem;
    CPSS_OS_SIG_SEM          prvLogCbHandleDoneSem;
    GT_U32                   prvLogCbProcessId; /*TODO: init when prvCpssLogInit() */
    CPSS_TASK                prvLogCbHandlerTaskId;
#endif
} PRV_CPSS_DRV_COMMON_MOD_LOG_NON_VOLATILE_GLOBAL_DB;

/** buffer size for strings */
#define PRV_LOG_STRING_BUFFER_SIZE_CNS      2048
#define PRV_LOG_STRING_NAME_BUFFER_SIZE_CNS 160

/**
* @struct PRV_CPSS_DRV_COMMON_MOD_INT_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are not meant to be shared
* between processes (when CPSS is linked as shared object)
* in common module, cpssLog directory
*/
typedef struct
{
    /** Common use buffers */
    char buffer[PRV_LOG_STRING_BUFFER_SIZE_CNS];
    char buffer1[PRV_LOG_STRING_NAME_BUFFER_SIZE_CNS];
    char buffer2[PRV_LOG_STRING_NAME_BUFFER_SIZE_CNS];
    char buffer3[PRV_LOG_STRING_BUFFER_SIZE_CNS];
    char buffer4[PRV_LOG_STRING_BUFFER_SIZE_CNS];
    char buffer5[PRV_LOG_STRING_BUFFER_SIZE_CNS];

    /** state variable for prvCpssLogContextEntryGet function */
    PRV_CPSS_LOG_CONTEXT_ENTRY_STC currentEntry;

    /** History log database */
    PRV_CPSS_LOG_API_HISTORY_ENTRY_STC firstHistoryDbEntry;

    /** Pointer to current history log database entry */
    PRV_CPSS_LOG_API_HISTORY_ENTRY_STC * currentHistoryEntryPtr;

    /** Current number of entries in history log database */
    GT_U32 currentHistoryEntryNum;

#ifdef SHARED_MEMORY
    /* Implement cpssLog callback issued from another application
     * (for example: luaCLI)
     */
    /* callback task parameters */

    CPSS_LOG_LIB_ENT         prvLogCbLib;
    CPSS_LOG_TYPE_ENT        prvLogCbType;
    void*                    prvLogCbDataPtr;
    PRV_LOG_CB_OPERATION_ENT prvLogCbOperation;
#endif

    /* log statistic */
    PRV_CPSS_LOG_STAT_STC    prvLogStat;
    PRV_CPSS_LOG_PHASE_ENT   lastState;

    /* Log history name */
    char prvCpssLogHistoryName[80];
} PRV_CPSS_COMMON_MOD_LOG_DIR_NON_SHARED_GLOBAL_DB;

#endif /* CPSS_LOG_ENABLE */

/**
* @struct PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SYS_RECOVERY_SRC_NON_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are not meant to be shared
* between processes (when CPSS is linked as shared object)
* in common module, systemRecovery  directory , source file cpssGenSystemRecovery.c
*/
typedef struct
{
    CPSS_SYSTEM_RECOVERY_INFO_STC                     systemRecoveryInfo;
    PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STC              systemRecoveryManagersDb;
    PRV_CPSS_SYSTEM_RECOVERY_DEBUG_STC                systemRecoveryDebugDb;
    PRV_CPSS_SYSTEM_RECOVERY_PARALLEL_COMPLETION_FUN  prvCpssSystemRecoveryParallelCompletionHandleFuncPtr;
    PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN     prvCpssSystemRecoveryMsgDisableModeHandleFuncPtr;
    PRV_CPSS_SYSTEM_RECOVERY_HA_FUN                   prvCpssSystemRecoveryCatchUpHandleFuncPtr;
    PRV_CPSS_SYSTEM_RECOVERY_HA_FUN                   prvCpssSystemRecoveryCompletionHandleFuncPtr ;
    PRV_CPSS_SYSTEM_RECOVERY_HA_FUN                   prvCpssSystemRecoveryCompletionGenHandleFuncPtr;
    PRV_CPSS_SYSTEM_RECOVERY_HA_FUN                   prvCpssSystemRecoveryHa2PhasesInitHandleFuncPtr;

} PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SYS_RECOVERY_SRC_NON_SHARED_GLOBAL_DB;


/**
* @struct PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_NON_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are not meant to be shared
* between processes (when CPSS is linked as shared object)
* in common module, systemRecovery directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SYS_RECOVERY_SRC_NON_SHARED_GLOBAL_DB genSystemRecoverySrc;
} PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_NON_SHARED_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_SERDES_DIR_NON_SHARED_SERDES_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are NOT meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,serdes directory, src file :
 *   mvHwsSerdesIf.c
*/
typedef struct
{
    MV_HWS_SERDES_FUNC_PTRS     *hwsSerdesFuncsPtr[HWS_MAX_DEVICE_NUM][SERDES_LAST];
} PRV_CPSS_COMMON_MOD_SERDES_DIR_NON_SHARED_SERDES_IF_SRC_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_SERDES_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are NOT meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,serdes directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_SERDES_DIR_NON_SHARED_SERDES_IF_SRC_GLOBAL_DB mvHwsSerdesIfSrc;
} PRV_CPSS_COMMON_MOD_SERDES_DIR_NON_SHARED_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_MAC_DIR_NON_SHARED_MAC_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are NOT meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,serdes directory, src file :
 *   mvHwsMacIf.c
*/
typedef struct
{
    MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr[HWS_MAX_DEVICE_NUM][LAST_MAC];
} PRV_CPSS_COMMON_MOD_MAC_DIR_NON_SHARED_MAC_IF_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_MAC_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are NOT meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,mac directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_MAC_DIR_NON_SHARED_MAC_IF_SRC_GLOBAL_DB mvHwsMacIfSrc;
} PRV_CPSS_COMMON_MOD_MAC_DIR_NON_SHARED_GLOBAL_DB;


/**
* @struct PRV_CPSS_COMMON_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module
*/
typedef struct
{
#ifdef CPSS_LOG_ENABLE
    PRV_CPSS_COMMON_MOD_LOG_DIR_NON_SHARED_GLOBAL_DB                 logDir;
#endif
    PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_NON_SHARED_GLOBAL_DB        systemRecoveryDir;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB        labServicesDir;
    PRV_CPSS_COMMON_MOD_PCS_DIR_NON_SHARED_GLOBAL_DB                 pcsDir;
    PRV_CPSS_COMMON_MOD_SERDES_DIR_NON_SHARED_GLOBAL_DB              serdesDir;
    PRV_CPSS_DRV_COMMON_MOD_INT_DIR_NON_SHARED_GLOBAL_DB             genericInterrupsDir;
    PRV_CPSS_COMMON_MOD_MAC_DIR_NON_SHARED_GLOBAL_DB                 macDir;

} PRV_CPSS_COMMON_MOD_NON_SHARED_GLOBAL_DB;

#endif /* __prvCpssCommonModGlobalNonSharedDb */
