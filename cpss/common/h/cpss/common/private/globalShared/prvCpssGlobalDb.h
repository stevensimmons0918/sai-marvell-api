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
* @file prvCpssGlobalDb.h
*
* @brief This file provides  defenitions of global variables structure
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssGlobalDb
#define __prvCpssGlobalDb

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/*add here include files for modules (main directories)*/
#include <cpss/common/private/globalShared/prvCpssMainPpDrvModGlobalSharedDb.h>
#include <cpss/common/private/globalShared/prvCpssMainPpDrvModGlobalNonSharedDb.h>
#include <cpss/common/private/globalShared/prvCpssCommonModGlobalSharedDb.h>
#include <cpss/common/private/globalNonShared/prvCpssCommonModGlobalNonSharedDb.h>
#include <cpss/common/private/globalNonShared/prvCpssExtDrvFunctionsGlobalNonSharedDb.h>
#include <cpss/common/private/globalNonShared/prvCpssOsFunctionsGlobalNonSharedDb.h>



#define CPSS_MAX_PIDS_SIZE  256


/**
* @struct PRV_CPSS_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are ment to be shared
 * between processes (when CPSS is linked as shared object)
*/
typedef struct{

  /** @brief process id of the process that created shared memory used for this structure
      */
  GT_U32 initializerPid;

    /** @brief GT_TRUE if DB should be initialized,GT_FALSE otherwise
    */
  GT_BOOL dbInitialized;

  /** @brief unique number in order to identify shared variables data base.
    */
  GT_U32 magic;

  /** @brief number of processes that are using shared objest
  *          Note if not used as shared object then should be equal to 1
    */
  GT_U32 clientNum;
  /** @brief equal to GT_TRUE in case running as shared object ,GT_FALSE otherwise
    */
  GT_BOOL sharedMemoryUsed;

    /** @brief equal to GT_TRUE in case mutexes db has been initialized ,GT_FALSE otherwise.
    */
  GT_BOOL sysConfigPhase1Done;

    /** @brief : indication that running the CPSS without the 'ASIC_SIMULATION' flag.
        but the switch device is WM (and not HW).
        in ASIM system : the application expected to set this flag to 1
            see API : cpssPpWmDeviceInAsimEnv()
        in other systems : the flag not changed from the 0 that it is initialed with.
    */
  GT_U32 isWmDeviceInSimEnv;

    /** @brief process id of the processes  that using  shared memory
    */
  GT_U32 pids[CPSS_MAX_PIDS_SIZE];

  /** @brief Global shared variables used in mainPpDrv module
    */
  PRV_CPSS_MAIN_PP_DRV_MOD_SHARED_GLOBAL_DB mainPpDrvMod;

  /** @brief Global shared variables used in common module
    */
  PRV_CPSS_COMMON_MOD_SHARED_GLOBAL_DB  commonMod;

} PRV_CPSS_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_NON_SHARED_NON_VOLATILE_VARS
 *
 * @brief  Structure contain resources that were taken by the process.
 *  Should not be creared on initialization.
*/

typedef struct{

  /*running index of registration entry for cpssOsTaskAtProcExit*/
  GT_U32    atExitIndex;
  /* registration  of cpssGlobalSharedDbCleanUp*/
  GT_BOOL   cleanUpRegistered;
  GT_U32    cleanUpIndex;
   /* registration  of cpssGlobalSharedDbRemoveProcess*/
  GT_BOOL   removeProcRegistered;
  GT_U32    removeProcIndex;

} PRV_CPSS_NON_SHARED_AT_EXIT_VARS;


/**
* @struct PRV_CPSS_NON_SHARED_NON_VOLATILE_VARS
 *
 * @brief  Structure contain resources that were taken by the process.
 *  Should not be creared on initialization.
*/

typedef struct{


    /** @brief  GT_TRUE if process is the owner  CPSS_SHM_INIT_SEM.
        GT_FALSE otherwise
      */
  GT_BOOL initialization;

  /*mainPpDrv module ,port  directory , source: cpssDxChPortManagerSamples.c*/
  GT_BOOL debugLegacyCommandsMode;

  GT_BOOL   aslrSupport;

  PRV_CPSS_NON_SHARED_AT_EXIT_VARS exitVars;

  GT_BOOL   verboseMode;


} PRV_CPSS_NON_SHARED_NON_VOLATILE_VARS;

/**
 * @struct PRV_CPSS_NON_SHARED_NON_VOLATILE_DB
 *
 * @brief  Structure contain resources that are not erased during reset.
 *  The variables are initialized only once !
 * Once the variables are initialized the value is kept until process exit.
 *
 */

typedef struct{

     /** @brief External driver function pointers
     */
   PRV_CPSS_EXT_DRV_FUNC_PTR_NON_SHARED_GLOBAL_DB extDrvFuncDb;
     /** @brief OS driver function pointers
     */
   PRV_CPSS_OS_FUNC_PTR_NON_SHARED_GLOBAL_DB osFuncDb;

#ifdef CPSS_LOG_ENABLE

   /** @brief CPSS Log DB
    */
   PRV_CPSS_DRV_COMMON_MOD_LOG_NON_VOLATILE_GLOBAL_DB cpssLogDb;
#endif

   /** @brief  Resources taken by the process
     */
   PRV_CPSS_NON_SHARED_NON_VOLATILE_VARS         generalVars;


} PRV_CPSS_NON_SHARED_NON_VOLATILE_DB;


/**
* @struct PRV_CPSS_NON_SHARED_FUNCTION_PTRS
 *
 * @brief  Structure contain global function pointers  that are not ment to be shared
 * between processes (when CPSS is linked as shared object).
 *Each process will have an unique copy of the structure.
*/
typedef struct{
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_GET_FUNC   duplicatedAddrGetFunc;
#ifdef CHX_FAMILY
    PRV_CPSS_DXCH_PORT_OBJ_STC                  portObjFunc;
    PRV_CPSS_SIP6_DMA_GLOBAL_NUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC  prvCpssDmaGlobalNumToLocalNumInDpConvertFunc;
    PRV_CPSS_SIP6_DMA_LOCAL_NUM_IN_DP_TO_GLOBAL_NUM_CONVERT_FUNC  prvCpssDmaLocalNumInDpToGlobalNumConvertFunc;
    PRV_CPSS_SIP6_HWS_UNIT_BASE_ADDR_CALC_FUNC  prvCpssHwsUnitBaseAddrCalcFunc;
    PRV_CPSS_SIP6_REG_DB_INFO_GET_FUNC          prvCpssRegDbInfoGetFunc;
#endif
    PRV_CPSS_PORT_FUNC_PTRS_STC                 ppCommonPortFuncPtrsStc;
    PRV_CPSS_PORT_PM_FUNC_PTRS                  pmPortFuncPtrsStc;
#ifdef CHX_FAMILY
  PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC        tableFormatInfo[PRV_CPSS_DXCH_TABLE_LAST_FORMAT_E];
#endif
   const MV_HWS_PORT_INIT_PARAMS                      **portsParamsSupModesMap;
   PRV_CPSS_DRV_REMOTE_DEVICE_EVENT_MASK_DEVICE_SET_FUNC remoteDevice_drvEventMaskDeviceFunc;
   PRV_CPSS_DRV_UNIT_INTERRUPT_GENERATION_CHECK_FUNC unitIntGenerationCheckFunc;
   PRV_CPSS_DRV_EVENT_PORT_MAP_CONVERT_FUNC eventPortMapConvertFunc;
   PRV_CPSS_DRV_MAC_EVENT_PORT_MAP_CONVERT_FUNC macEventPortMapConvertFunc;
   PRV_CPSS_DRV_EVENT_GENERIC_CONVERT_FUNC eventGenericConvertFunc;
   PRV_CPSS_DRV_INT_CTRL_NON_SHARED_STC    intCtrlNonShared;

} PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA;


/**
* @struct PRV_CPSS_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object).
 *Each process will have an unique copy of the structure.
*/

typedef struct{
    /** @brief unique number in order to identify non-  shared variables data base.
      */

  GT_U32 magic;

    /** @brief Global non shared variables used in mainPpDrv module
    */
  PRV_CPSS_MAIN_PP_DRV_MOD_NON_SHARED_GLOBAL_DB mainPpDrvMod;

  /** @brief Global non shared variables used in common module
    */
  PRV_CPSS_COMMON_MOD_NON_SHARED_GLOBAL_DB commonMod;

  /* Structure contain resources that are not erased during reset.*/
  PRV_CPSS_NON_SHARED_NON_VOLATILE_DB     nonVolatileDb;

  PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA       * nonSharedDeviceSpecificDb[PRV_CPSS_MAX_PP_DEVICES_CNS];

} PRV_CPSS_NON_SHARED_GLOBAL_DB;




/*global variables that are ment to be shared  between processes in shared lib mode*/
extern PRV_CPSS_SHARED_GLOBAL_DB *cpssSharedGlobalVarsPtr;
/*global variables that are not ment to be shared  between processes in shared lib mode.
Unique per each process*/
extern PRV_CPSS_NON_SHARED_GLOBAL_DB *cpssNonSharedGlobalVarsPtr;
extern PRV_CPSS_NON_SHARED_GLOBAL_DB  cpssNonSharedGlobalVars;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGlobalMutexh */

