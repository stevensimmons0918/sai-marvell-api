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
* @file prvCpssGlobalDbInterface.h
*
* @brief This file provide interface to  global variables data base.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssGlobalDbInterface
#define __prvCpssGlobalDbInterface

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*macro for setting and getting shared global variables*/
#define PRV_SHARED_GLOBAL_VAR_GET(_var) cpssSharedGlobalVarsPtr->_var
#define PRV_SHARED_GLOBAL_VAR_SET(_var,_value) cpssSharedGlobalVarsPtr->_var = _value

/*macro for setting and getting non-shared global variables*/
#define PRV_NON_SHARED_GLOBAL_VAR_GET(_var) cpssNonSharedGlobalVarsPtr->_var
#define PRV_NON_SHARED_GLOBAL_VAR_SET(_var,_value) cpssNonSharedGlobalVarsPtr->_var = _value

#define PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(_var) cpssNonSharedGlobalVars._var
#define PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(_var,_value) cpssNonSharedGlobalVars._var = _value



#define PRV_NON_SHARED_GLOBAL_VAR_LOG_GET() \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.cpssLogDb.prvCpssLogEnabled)

#define PRV_NON_SHARED_GLOBAL_VAR_LOG_SET(_value) \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(nonVolatileDb.cpssLogDb.prvCpssLogEnabled,_value)

#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.systemRecoveryDebugDb)

#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.systemRecoveryInfo)


#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.systemRecoveryManagersDb)


#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc)

#define PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_SET(_var,_value)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(nonVolatileDb._var,_value)

#define PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonVolatileDb._var)


#define PRV_SHARED_GLOBAL_VAR_DB_CHECK() \
    do \
    { \
        if(NULL==cpssSharedGlobalVarsPtr) \
        { \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, \
                                    "cpssSharedGlobalVarsPtr ==NULL\n");\
        } \
    } \
    while(0)

#define PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG \
        PRV_SHARED_GLOBAL_VAR_GET(commonMod.configDir.dDrvPpConGenInitSrc.prvCpssDrvPpConfig)


#define PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG_CHECK(_index) \
        do \
        { \
            PRV_SHARED_GLOBAL_VAR_DB_CHECK(); \
            if(NULL == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[_index])\
            { \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, \
                                    "prvCpssDrvPpConfig[%d]==NULL\n", _index);\
            } \
        } \
        while(0)

#define PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG \
                PRV_SHARED_GLOBAL_VAR_GET(commonMod.configDir.dDrvPpConGenInitSrc.prvCpssDrvPpObjConfig)


#define PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG_CHECK(_index) \
        do \
        { \
            PRV_SHARED_GLOBAL_VAR_DB_CHECK(); \
            if(NULL == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[_index])\
            { \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, \
                                    "prvCpssDrvPpObjConfig[%d]==NULL\n", _index);\
            } \
        } \
        while(0)
#ifdef  WIN32
     #define  globalDbFuncName __FUNCTION__
#else
     #define  globalDbFuncName __func__
#endif


#define PRV_CPSS_SHM_PRINT(fmt, ...) \
        do \
        { \
            if(cpssNonSharedGlobalVars.nonVolatileDb.generalVars.verboseMode)\
                cpssOsPrintf("[CPSS_SHM_DBG] %s[%d] " fmt,  globalDbFuncName, __LINE__, ## __VA_ARGS__);\
        }while(0)




/** globals defines to replace full path of global variables */

/* shared vaiables */
#define hwsDeviceSpecInfo (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceSpecInfoGlobal))
#define hwsDevicesPortsElementsArray (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.commonSrc.hwsDevicesPortsElementsArrayGlobal))
#define hwsOsExactDelayPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsExactDelayGlobalPtr))
#define hwsOsMicroDelayPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMicroDelayGlobalPtr))

#define hwsServerRegSetFuncPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegSetFuncGlobalPtr))
#define hwsServerRegGetFuncPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegGetFuncGlobalPtr))
#define hwsServerRegFieldSetFuncPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegFieldSetFuncGlobalPtr))
#define hwsServerRegFieldGetFuncPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegFieldGetFuncGlobalPtr))

#define hwsIlknRegDbGetFuncPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsIlknRegDbGetFuncGlobalPtr))
#define hwsPpHwTraceEnablePtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsPpHwTraceEnableGlobalPtr))

/*non shared*/
#define hwsSerdesRegSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegSetFuncGlobalPtr))
#define hwsSerdesRegGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegGetFuncGlobalPtr))
#define hwsOsMemCopyFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemCopyFuncGlobalPtr))
#define hwsOsTimerWkFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsTimerWkFuncGlobalPtr))
#define hwsOsMemSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemSetFuncGlobalPtr))
#define hwsOsFreeFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsFreeFuncGlobalPtr))
#define hwsOsMallocFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMallocFuncGlobalPtr))
#define hwsOsStrCatFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsStrCatFuncGlobalPtr))
#define hwsTimerGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsTimerGetFuncGlobalPtr))
#define hwsRegisterSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterSetFuncGlobalPtr))
#define hwsRegisterGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterGetFuncGlobalPtr))



#define DUPLICATED_ADDR_GETFUNC(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->duplicatedAddrGetFunc)
#define DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(_devNum) \
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->prvCpssDmaGlobalNumToLocalNumInDpConvertFunc)

#define DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(_devNum) \
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->prvCpssDmaLocalNumInDpToGlobalNumConvertFunc)


#define PORT_OBJ_FUNC(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->portObjFunc)

#define PRV_TABLE_FORMAT_INFO(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->tableFormatInfo)

#define PRV_PORTS_PARAMS_SUP_MODE_MAP(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->portsParamsSupModesMap)

#define PRV_INTERRUPT_FUNC_GET(_devNum,_func)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->_func)

#define PRV_INTERRUPT_CTRL_GET(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]->intCtrlNonShared)


GT_U32  prvCpssBmpIsZero
(
    GT_VOID * portsBmpPtr
);

/*NOTE: PRV_CPSS_PORTS_BMP_PORT_SET_ALL_MAC and PRV_CPSS_PORTS_BMP_IS_ZERO_MAC  \
        should be NOT used by  application. This is for CPSS internal use use only*/


#define PRV_CPSS_PORTS_BMP_PORT_SET_ALL_MAC(portsBmpPtr)     \
    cpssOsMemSet(portsBmpPtr,0xFF,sizeof(CPSS_PORTS_BMP_STC))


#define PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsBmpPtr)     \
    cpssOsMemSet(portsBmpPtr,0,sizeof(CPSS_PORTS_BMP_STC))

/* all ii 0.. max --> portsBmpTargetPtr[ii] == 0 */
#define PRV_CPSS_PORTS_BMP_IS_ZERO_MAC(portsBmpPtr)     \
    prvCpssBmpIsZero(portsBmpPtr)

/* check if ports bmp in portsBmpPtr1 == portsBmpPtr2
    return 1 - equal
    return 0 - NOT equal
*/
#define PRV_CPSS_PORTS_BMP_ARE_EQUAL_MAC(portsBmpPtr1,portsBmpPtr2)     \
    (cpssOsMemCmp(portsBmpPtr1,portsBmpPtr2,sizeof(CPSS_PORTS_BMP_STC)) ? 0 : 1)

/* non-shared variables */


/**
* @internal cpssGlobalDbInit function
* @endinternal
*
* @brief   Initialize CPSS global variables data base.
*          Global variables data base is allocated.
*          In case useSharedMem equal GT_TRUE then also shared memory is allocated for shared variables.
*          After memory is allocated ,the global variables are being initialized to init values.
* @param[in] aslrSupport - whether shared memory should be used for shared variables.
*                           in case equal GT_FALSE dynamic memory allocation is used for shared variables,
*                           otherwise shared memory is used s used for shared variables,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbInit
(
    IN GT_BOOL aslrSupport,
    IN GT_BOOL forceInit
);


/**
* @internal cpssGlobalDbDestroy function
* @endinternal
*
* @brief   UnInitialize CPSS global variables data base.
*          Global variables data base is deallocated.
*          In case  shared memory is used then shared memory is unlinked here.
*
* @param[in] unlink   - whether shared memory should be unlinked.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbDestroy
(
    GT_BOOL unlink,
    GT_BOOL *unlinkDonePtr
);


/**
* @internal prvCpssGlobalDbExistGet function
* @endinternal
*
* @brief   Check if  gllobal variables data base was allocated.
*
* @retval GT_TRUE                    global DB exist
* @retval GT_FAIL                  - global DB does not exist
*/
GT_BOOL prvCpssGlobalDbExistGet
(
    GT_VOID
);

/**
 * @internal cpssGlobalNonSharedDbExtDrvFuncInit function
 * @endinternal
 *
 * @brief  Initialize global non shared function pointers
 *
 */
GT_VOID cpssGlobalNonSharedDbExtDrvFuncInit
(
    GT_VOID
);

/**
 * @internal cpssGlobalNonSharedDbOsFuncInit function
 * @endinternal
 *
 * @brief  Initialize global non shared OS function pointers
 *
 */
GT_VOID cpssGlobalNonSharedDbOsFuncInit
(
    GT_VOID
);

/**
* @internal osGlobalSharedDbAddProcess function
* @endinternal
*
* @brief add process to pid list(debug)
*
*/
GT_VOID osGlobalSharedDbAddProcess
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGlobalMutexh */

