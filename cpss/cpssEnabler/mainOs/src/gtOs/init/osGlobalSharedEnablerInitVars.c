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
* @file osGlobalSharedEnablerInitVars.c
*
* @brief This file  Initialize global shared variables used in  module:cpssEnabler
*
* @version   1
********************************************************************************
*/

#ifdef LINUX
#include <pthread.h>
#endif


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#ifdef LINUX
#include <gtOs/linuxLmsgqLib.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/linuxKeLsemLib.h>
#include <gtOs/linuxLtaskLib.h>
#include <gtOs/gtOsSemDefs.h>
#endif


/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsMemLibSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtOsMemLibSrc._var)

#ifdef LINUX
#define PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsLinuxPthrLsmggLibSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsKeLsemLibSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsLinuxMsgQSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsLtaskLibSrc._var,_value)
#endif

/************* Defines ********************************************************/

#define SEM_ALLOC_CHUNK 32


/**
 * @internal osGlobalSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *          src:osLinuxMem
 *
 */
static GT_VOID osGlobalSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_INIT(firstInitialization,GT_TRUE);
}

/**
 * @internal osGlobalSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *          src:gtOsMemLib
 *
 */
static GT_VOID osGlobalSharedDbEnablerModMainOsDirGtOsMemLibSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(firstMemInit,GT_TRUE);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(globalDmaAllocationsFirstPtr,NULL);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(globalDmaAllocationsLastPtr,NULL);
#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
     PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(globalDmaAllocationsFirstPtr,NULL);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(globalDmaAllocationsLastPtr,NULL);
#endif /* INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG */
    PRV_SHARED_MAIN_OS_DIR_GT_OS_MEM_LIB_SRC_GLOBAL_VAR_INIT(globalMemoryLeakageState,OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E);
}

#ifdef LINUX
/**
 * @internal osGlobalSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *          src:gtOsMemLib
 *
 */
static GT_VOID osGlobalSharedDbEnablerModMainOsDirGtOsLinuxPthrLsmggLibSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_VAR_INIT(mqList.list.allocated, 1);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_VAR_INIT(mqList.list.allocChunk, 64);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_VAR_INIT(mqList.list.objSize, sizeof(OS_MSGQ_STC));
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_VAR_INIT(mqList.list.allocFuncPtr, V2L_ts_malloc);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_VAR_INIT(mqList.list.freeFuncPtr, V2L_ts_free);
}

static GT_VOID osGlobalSharedDbEnablerModMainOsDirGtOsKeLsemLibSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_VAR_INIT(userspaceMutexeList.list.allocated, 0);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_VAR_INIT(userspaceMutexeList.list.allocChunk, 128);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_VAR_INIT(userspaceMutexeList.list.objSize, sizeof(GT_LINUX_MUTEX));
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_VAR_INIT(userspaceMutexeList.list.allocFuncPtr, SHARED_MALLOC_MAC);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_VAR_INIT(userspaceMutexeList.list.freeFuncPtr, SHARED_FREE_MAC);
}

static GT_VOID osGlobalSharedDbEnablerModMainOsDirGtOsLinuxMsgQSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.allocated, 0);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.allocChunk, 64);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.objSize, sizeof(OS_MSGQ_STC));
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.allocFuncPtr, (OS_OBJ_ALLOC_FUNC_TYPE)myOsMalloc osMalloc_MemoryLeakageDbg);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.freeFuncPtr, osFree_MemoryLeakageDbg);
#else
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.allocFuncPtr, (OS_OBJ_ALLOC_FUNC_TYPE)osMalloc);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_VAR_INIT(mqList.list.freeFuncPtr, osFree);
#endif
}

static GT_VOID osGlobalSharedDbEnablerModMainOsDirGtOsLtaskLibSrcDataSectionInit
(
    GT_VOID
)
{
    pthread_mutex_init(&osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLtaskLibSrc.tasks_mtx, NULL);
    pthread_mutex_init(&osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLtaskLibSrc.taskLock_mtx, NULL);
    pthread_cond_init(&osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLtaskLibSrc.taskLock_cond, 0);

    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_VAR_INIT(tasks.allocated, 0);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_VAR_INIT(tasks.allocChunk, 32);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_VAR_INIT(tasks.objSize, sizeof(_V2L_taskSTC));
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_VAR_INIT(tasks.allocFuncPtr, V2L_ts_malloc);
    PRV_SHARED_MAIN_OS_DIR_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_VAR_INIT(tasks.freeFuncPtr, V2L_ts_free);
}


static GT_VOID osGlobalSharedDbEnablerModMainOsDirGtOsLsemLibSrcDataSectionInit
(
    GT_VOID
)
{

    OS_OBJECT_LIST_STC semaphores  = {
        NULL,                   /* list */
        0,                      /* allocated */
        SEM_ALLOC_CHUNK,        /* allocChunk */
        sizeof(_V2L_semSTC),    /* objSize */
        V2L_ts_malloc,          /* allocFuncPtr */
        V2L_ts_free             /* freeFuncPtr */
    };

    osMemCpy(&osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLinuxPthrLsemSrc.semaphores,&semaphores,sizeof(OS_OBJECT_LIST_STC));
}

#endif

/**
 * @internal cpssGlobalSharedDbMainPpDrvModHwInitDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *
 */
static GT_VOID osHelperGlobalSharedDbEnablerModMainOsDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    osGlobalSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit();
    osGlobalSharedDbEnablerModMainOsDirGtOsMemLibSrcDataSectionInit();
#ifdef LINUX
    osGlobalSharedDbEnablerModMainOsDirGtOsLinuxPthrLsmggLibSrcDataSectionInit();
    osGlobalSharedDbEnablerModMainOsDirGtOsKeLsemLibSrcDataSectionInit();
    osGlobalSharedDbEnablerModMainOsDirGtOsLinuxMsgQSrcDataSectionInit();
    osGlobalSharedDbEnablerModMainOsDirGtOsLtaskLibSrcDataSectionInit();
    osGlobalSharedDbEnablerModMainOsDirGtOsLsemLibSrcDataSectionInit();
#endif

}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:cpssEnabler
 *
 */
GT_VOID osHelperGlobalSharedDbEnablerModDataSectionInit
(
    GT_VOID
)
{
    /*add here directory  initializers*/
    osHelperGlobalSharedDbEnablerModMainOsDirDataSectionInit();
}




