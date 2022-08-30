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
* @file cpssExtServices.c
*
* @brief External Driver wrapper. definitions for bind OS , external driver
* dependent services and trace services to CPSS .
*
*
* @version   24
********************************************************************************
*/

/************* Includes *******************************************************/

#include <cpss/extServices/cpssExtServices.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/os/gtOs/cpssOsTask.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static void extServiceDbReset(void *s, int c, GT_U32 n)
{
    unsigned char* p=s;
    while(n--)
        *p++ = (unsigned char)c;
}

#define FUNC_DB_RESET(_db)  extServiceDbReset(&cpssNonSharedGlobalVars.nonVolatileDb._db,0,sizeof(cpssNonSharedGlobalVars.nonVolatileDb._db));

#define CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG

/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST     (void *)

#ifndef CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG
/* define a STUB function that "do nothing" but return "not implemented" */
/* the function parameters do not matter , since the function is "forced to
   do casting"
*/

#define STR_NOT_IMPLEMENTED_CNS " extServiceFuncNotImplementedCalled \n"
static GT_STATUS extServiceFuncNotImplementedCalled
(
    void
)
{
    if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)extServiceFuncNotImplementedCalled)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}
/* STUB function that "do nothing" but return 0.
 * Applicable for malloc, realloc, etc
 */
static GT_UINTPTR extServiceFuncNotImplementedCalled0
(
    void
)
{
    if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)extServiceFuncNotImplementedCalled)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS);
    }

    return 0;
}
#define DECLARE_FUNC(_name,_type) \
    _type _name = (_type)extServiceFuncNotImplementedCalled;
#define DECLARE_FUNC0(_name,_type) \
    _type _name = (_type)extServiceFuncNotImplementedCalled0;

#else /* defined CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG */
#define STR_NOT_IMPLEMENTED_CNS " extServiceFuncNotImplementedCalled: %s\n"
static  GT_STATUS extServiceFuncNotImplementedCalled_cpssOsPrintf(void);
static void extServiceFuncNotImplementedCalledPrint(const char *name)
{
    if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)extServiceFuncNotImplementedCalled_cpssOsPrintf)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS, name);
    }
}

static  GT_STATUS extServiceFuncNotImplementedCalled_cpssOsPrintf(void)
{
    extServiceFuncNotImplementedCalledPrint("cpssOsPrintf");
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}
#define DECLARE_FUNC(_name,_type) \
    static GT_STATUS extServiceFuncNotImplementedCalled_ ## _name(void) \
    { \
        extServiceFuncNotImplementedCalledPrint(#_name); \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG); \
    } \
    _type _name = (_type)extServiceFuncNotImplementedCalled_ ## _name;
#define DECLARE_FUNC0(_name,_type) \
    static GT_UINTPTR extServiceFuncNotImplementedCalled_ ## _name(void) \
    { \
        extServiceFuncNotImplementedCalledPrint(#_name); \
        return 0; \
    } \
    _type _name = (_type)extServiceFuncNotImplementedCalled_ ## _name;


#endif
/* macro to replace the content of pointer only if replaced with non-NULL pointer */
#define REPLACE_IF_NOT_NULL_MAC(_target,_source)   \
    _target = _source ? _source : _target

/* macro that checks if mandatory functions are not replaced with NULL pointer */
#define MANDATORY_FUNCTION(_target,_source)   \
    if (_source == NULL) \
    { \
        cpssOsPrintf("cpssExtServicesBind: the mandatory " #_source " is not defined \n"); \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG); \
    } \
    _target = _source

/************* Prototypes *****************************************************/

/* need to fill stub function */



GT_STATUS   cpssExtServicesBind(
    IN CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    IN CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    IN CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
)
{

    if(extDrvFuncBindInfoPtr)
    {
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvMgmtCacheFlush      , extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheFlush);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvMgmtCacheInvalidate , extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheInvalidate);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvDmaWrite            , extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaWriteDriverFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvDmaRead             , extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaReadFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPortRxInit       , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxInitFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthRawSocketModeSet       , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthRawSocketModeGet       , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvLinuxModeSet       , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvLinuxModeGet       , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHsuMemBaseAddrGet         , extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuMemBaseAddrGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHsuWarmRestart           , extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuWarmRestartFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvInboundSdmaEnable        , extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaEnableFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvInboundSdmaDisable       , extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaDisableFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvInboundSdmaStateGet      , extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaStateGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPortTxInit            , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxInitFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPortEnable       , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortEnableFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPortDisable      , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortDisableFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPortTx           , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthInputHookAdd     , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortInputHookAddFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthRawSocketRxHookAdd     , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketRxHookAddFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthTxCompleteHookAdd, extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxCompleteHookAddFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthRxPacketFree     , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxPacketFreeFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPortTxModeSet    , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxModeSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthCpuCodeToQueue   , extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthCpuCodeToQueueFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEthPrePendTwoBytesHeaderSet, extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPrePendTwoBytesHeaderSetFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvIntConnect          , extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntConnectFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvIntDisconnect       ,extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisconnectFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvIntEnable           , extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntEnableFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvIntDisable          , extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisableFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvPciConfigWriteReg   , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigWriteRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvPciConfigReadReg    , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigReadRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvPciFindDev          , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDevFindFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvGetPciIntVec        , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntVecFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvGetIntMask          , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntMaskFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvEnableCombinedPciAccess  , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciCombinedAccessEnableFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvPciDoubleWrite      , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleWriteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvPciDoubleRead       , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleReadFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvPciSetDevId         , extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciSetDevIdFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiInitDriver        , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiInitDriverFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiWriteReg          , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiWriteRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiReadReg           , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiReadRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTskRegRamRead     , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamReadFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTskRegRamWrite    , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamWriteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTskRegVecRead     , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecReadFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTskRegVecWrite    , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecWriteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTaskWriteReg      , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTaskReadReg       , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskReadRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiInterruptWriteReg , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntWriteRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiInterruptReadReg  , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntReadRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvSmiDevVendorIdGet        , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiDevVendorIdGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfSmiTaskWriteRegField , extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteFieldFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfTwsiInitDriver , extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiInitDriverFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfTwsiWriteReg   , extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiWriteRegFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvHwIfTwsiReadReg    , extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiReadRegFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvI2cMgmtMasterInit    , extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvI2cMgmtMasterInitFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvMgmtReadRegister     , extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtReadRegisterFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvMgmtWriteRegister    , extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtWriteRegisterFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvMgmtIsrReadRegister  , extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtIsrReadRegisterFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvMgmtIsrWriteRegister , extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtIsrWriteRegisterFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssExtDrvDragoniteShMemBaseAddrGet , extDrvFuncBindInfoPtr->extDrvDragoniteInfo.extDrvDragoniteShMemBaseAddrGetFunc);
    }
    else
    {
        FUNC_DB_RESET(extDrvFuncDb);
    }


    if(osFuncBindPtr)
    {
        REPLACE_IF_NOT_NULL_MAC(cpssOsNtohl       , osFuncBindPtr->osInetBindInfo.osInetNtohlFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsHtonl       , osFuncBindPtr->osInetBindInfo.osInetHtonlFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsNtohs       , osFuncBindPtr->osInetBindInfo.osInetNtohsFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsHtons       , osFuncBindPtr->osInetBindInfo.osInetHtonsFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsInetNtoa    , osFuncBindPtr->osInetBindInfo.osInetNtoaFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsBindStdOut , osFuncBindPtr->osIoBindInfo.osIoBindStdOutFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsPrintf     , osFuncBindPtr->osIoBindInfo.osIoPrintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsVprintf    , osFuncBindPtr->osIoBindInfo.osIoVprintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSprintf    , osFuncBindPtr->osIoBindInfo.osIoSprintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsVsprintf   , osFuncBindPtr->osIoBindInfo.osIoVsprintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSnprintf   , osFuncBindPtr->osIoBindInfo.osIoSnprintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsVsnprintf  , osFuncBindPtr->osIoBindInfo.osIoVsnprintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsPrintSync  , osFuncBindPtr->osIoBindInfo.osIoPrintSynchFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsGets       , osFuncBindPtr->osIoBindInfo.osIoGetsFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsFlastErrorStr , osFuncBindPtr->osIoBindInfo.osIoFlastErrorStrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFopen         , osFuncBindPtr->osIoBindInfo.osIoFopenFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFclose        , osFuncBindPtr->osIoBindInfo.osIoFcloseFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsRewind        , osFuncBindPtr->osIoBindInfo.osIoRewindFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFprintf       , osFuncBindPtr->osIoBindInfo.osIoFprintfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFgets         , osFuncBindPtr->osIoBindInfo.osIoFgets);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFwrite        , osFuncBindPtr->osIoBindInfo.osIoFwriteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFread         , osFuncBindPtr->osIoBindInfo.osIoFreadFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsFgetLength    , osFuncBindPtr->osIoBindInfo.osIoFgetLengthFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsFatalError    , osFuncBindPtr->osIoBindInfo.osIoFatalErrorFunc);

        MANDATORY_FUNCTION(cpssOsBzero              , osFuncBindPtr->osMemBindInfo.osMemBzeroFunc);
        MANDATORY_FUNCTION(cpssOsMemSet             , osFuncBindPtr->osMemBindInfo.osMemSetFunc);
        MANDATORY_FUNCTION(cpssOsMemCpy             , osFuncBindPtr->osMemBindInfo.osMemCpyFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMemMove       , osFuncBindPtr->osMemBindInfo.osMemMoveFunc);
        MANDATORY_FUNCTION(cpssOsMemCmp             , osFuncBindPtr->osMemBindInfo.osMemCmpFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStaticMalloc  , osFuncBindPtr->osMemBindInfo.osMemStaticMallocFunc);

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
        MANDATORY_FUNCTION(cpssOsMalloc_MemoryLeakageDbg        , osFuncBindPtr->osMemBindInfo.osMemMallocFunc);
        MANDATORY_FUNCTION(cpssOsRealloc_MemoryLeakageDbg       , osFuncBindPtr->osMemBindInfo.osMemReallocFunc);
        MANDATORY_FUNCTION(cpssOsFree_MemoryLeakageDbg          , osFuncBindPtr->osMemBindInfo.osMemFreeFunc);
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
        MANDATORY_FUNCTION(cpssOsMalloc             , osFuncBindPtr->osMemBindInfo.osMemMallocFunc);
        MANDATORY_FUNCTION(cpssOsRealloc            , osFuncBindPtr->osMemBindInfo.osMemReallocFunc);
        MANDATORY_FUNCTION(cpssOsFree               , osFuncBindPtr->osMemBindInfo.osMemFreeFunc);
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
        REPLACE_IF_NOT_NULL_MAC(cpssOsCacheDmaMalloc, osFuncBindPtr->osMemBindInfo.osMemCacheDmaMallocFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsCacheDmaFree  , osFuncBindPtr->osMemBindInfo.osMemCacheDmaFreeFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsPhy2Virt      , osFuncBindPtr->osMemBindInfo.osMemPhyToVirtFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsVirt2Phy      , osFuncBindPtr->osMemBindInfo.osMemVirtToPhyFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsGlobalDbShmemInit      , osFuncBindPtr->osMemBindInfo.osMemGlobalDbShmemInit);
        REPLACE_IF_NOT_NULL_MAC(cpssOsGlobalDbShmemUnlink, osFuncBindPtr->osMemBindInfo.osMemGlobalDbShmemUnlink);

        MANDATORY_FUNCTION(cpssOsRand               , osFuncBindPtr->osRandBindInfo.osRandFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSrand         , osFuncBindPtr->osRandBindInfo.osSrandFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsMutexCreate     , osFuncBindPtr->osSemBindInfo.osMutexCreateFunc);
     #if defined  CPSS_USE_MUTEX_PROFILER
        REPLACE_IF_NOT_NULL_MAC(cpssOsMutexSetGlAttributes, osFuncBindPtr->osSemBindInfo.osMutexSetGlAttributesFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsCheckUnprotectedPerDeviceDbAccess, osFuncBindPtr->osSemBindInfo.osCheckUnprotectedPerDeviceDbAccess);
      #endif
        REPLACE_IF_NOT_NULL_MAC(cpssOsMutexDelete     , osFuncBindPtr->osSemBindInfo.osMutexDeleteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMutexLock       , osFuncBindPtr->osSemBindInfo.osMutexLockFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMutexUnlock     , osFuncBindPtr->osSemBindInfo.osMutexUnlockFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsSigSemBinCreate , osFuncBindPtr->osSemBindInfo.osSigSemBinCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSigSemMCreate   , osFuncBindPtr->osSemBindInfo.osSigSemMCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSigSemCCreate   , osFuncBindPtr->osSemBindInfo.osSigSemCCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSigSemDelete    , osFuncBindPtr->osSemBindInfo.osSigSemDeleteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSigSemWait      , osFuncBindPtr->osSemBindInfo.osSigSemWaitFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSigSemSignal    , osFuncBindPtr->osSemBindInfo.osSigSemSignalFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsNamedSemOpen    , osFuncBindPtr->osSemBindInfo.osNamedSemOpen);
        REPLACE_IF_NOT_NULL_MAC(cpssOsNamedSemWait    , osFuncBindPtr->osSemBindInfo.osNamedSemWait);
        REPLACE_IF_NOT_NULL_MAC(cpssOsNamedSemPost    , osFuncBindPtr->osSemBindInfo.osNamedSemPost);
        REPLACE_IF_NOT_NULL_MAC(cpssOsNamedSemClose   , osFuncBindPtr->osSemBindInfo.osNamedSemClose);
        REPLACE_IF_NOT_NULL_MAC(cpssOsNamedSemUnlink  , osFuncBindPtr->osSemBindInfo.osNamedSemUnlink);


        REPLACE_IF_NOT_NULL_MAC(cpssOsStrlen   , osFuncBindPtr->osStrBindInfo.osStrlenFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrCpy   , osFuncBindPtr->osStrBindInfo.osStrCpyFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrNCpy  , osFuncBindPtr->osStrBindInfo.osStrNCpyFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrChr   , osFuncBindPtr->osStrBindInfo.osStrChrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrStr   , osFuncBindPtr->osStrBindInfo.osStrStrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrRChr  , osFuncBindPtr->osStrBindInfo.osStrRevChrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrCmp   , osFuncBindPtr->osStrBindInfo.osStrCmpFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrNCmp  , osFuncBindPtr->osStrBindInfo.osStrNCmpFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrCat   , osFuncBindPtr->osStrBindInfo.osStrCatFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrNCat  , osFuncBindPtr->osStrBindInfo.osStrStrNCatFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsToUpper  , osFuncBindPtr->osStrBindInfo.osStrChrToUpperFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrTo32  , osFuncBindPtr->osStrBindInfo.osStrTo32Func);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrToU32 , osFuncBindPtr->osStrBindInfo.osStrToU32Func);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrTol   , osFuncBindPtr->osStrBindInfo.osStrTolFunc);

        MANDATORY_FUNCTION(cpssOsTimerWkAfter     , osFuncBindPtr->osTimeBindInfo.osTimeWkAfterFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTickGet     , osFuncBindPtr->osTimeBindInfo.osTimeTickGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTime        , osFuncBindPtr->osTimeBindInfo.osTimeGetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTimeRT      , osFuncBindPtr->osTimeBindInfo.osTimeRTFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsGetSysClockRate , osFuncBindPtr->osTimeBindInfo.osGetSysClockRateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsDelay       , osFuncBindPtr->osTimeBindInfo.osDelayFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsStrfTime    , osFuncBindPtr->osTimeBindInfo.osStrftimeFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsTaskCreate  , osFuncBindPtr->osTaskBindInfo.osTaskCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTaskDelete  , osFuncBindPtr->osTaskBindInfo.osTaskDeleteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTaskGetSelf , osFuncBindPtr->osTaskBindInfo.osTaskGetSelfFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTaskGetPid      , osFuncBindPtr->osTaskBindInfo.osTaskGetPid);
        REPLACE_IF_NOT_NULL_MAC(cpssOsTaskAtProcExit  , osFuncBindPtr->osTaskBindInfo.osTaskAtProcExit);

        REPLACE_IF_NOT_NULL_MAC(cpssOsQsort       , osFuncBindPtr->osStdLibBindInfo.osQsortFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsBsearch     , osFuncBindPtr->osStdLibBindInfo.osBsearchFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsMsgQCreate  , osFuncBindPtr->osMsgQBindInfo.osMsgQCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMsgQDelete  , osFuncBindPtr->osMsgQBindInfo.osMsgQDeleteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMsgQSend    , osFuncBindPtr->osMsgQBindInfo.osMsgQSendFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMsgQRecv    , osFuncBindPtr->osMsgQBindInfo.osMsgQRecvFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsMsgQNumMsgs , osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsLog         , osFuncBindPtr->osLogBindInfo.osLogFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketLastError, osFuncBindPtr->osSocketInfo.osSocketLastErrorFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketLastErrorStr, osFuncBindPtr->osSocketInfo.osSocketLastErrorStrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketTcpCreate, osFuncBindPtr->osSocketInfo.osSocketTcpCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketUdpCreate, osFuncBindPtr->osSocketInfo.osSocketUdpCreateFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketTcpDestroy, osFuncBindPtr->osSocketInfo.osSocketTcpDestroyFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketUdpDestroy, osFuncBindPtr->osSocketInfo.osSocketUdpDestroyFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketCreateAddr, osFuncBindPtr->osSocketInfo.osSocketCreateAddrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketDestroyAddr, osFuncBindPtr->osSocketInfo.osSocketDestroyAddrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketBind, osFuncBindPtr->osSocketInfo.osSocketBindFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketListen, osFuncBindPtr->osSocketInfo.osSocketListenFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketAccept, osFuncBindPtr->osSocketInfo.osSocketAcceptFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketConnect, osFuncBindPtr->osSocketInfo.osSocketConnectFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketSetNonBlock, osFuncBindPtr->osSocketInfo.osSocketSetNonBlockFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketSetBlock, osFuncBindPtr->osSocketInfo.osSocketSetBlockFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketSend, osFuncBindPtr->osSocketInfo.osSocketSendFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketSendTo, osFuncBindPtr->osSocketInfo.osSocketSendToFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketRecv, osFuncBindPtr->osSocketInfo.osSocketRecvFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketRecvFrom, osFuncBindPtr->osSocketInfo.osSocketRecvFromFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketSetSocketNoLinger, osFuncBindPtr->osSocketInfo.osSocketSetSocketNoLingerFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketExtractIpAddrFromSocketAddr, osFuncBindPtr->osSocketInfo.osSocketExtractIpAddrFromSocketAddrFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketGetSocketAddrSize, osFuncBindPtr->osSocketInfo.osSocketGetSocketAddrSizeFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketShutDown, osFuncBindPtr->osSocketInfo.osSocketShutDownFunc);

        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectCreateSet, osFuncBindPtr->osSocketSelectInfo.osSelectCreateSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectEraseSet, osFuncBindPtr->osSocketSelectInfo.osSelectEraseSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectZeroSet, osFuncBindPtr->osSocketSelectInfo.osSelectZeroSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectAddFdToSet, osFuncBindPtr->osSocketSelectInfo.osSelectAddFdToSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectClearFdFromSet, osFuncBindPtr->osSocketSelectInfo.osSelectClearFdFromSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectIsFdSet, osFuncBindPtr->osSocketSelectInfo.osSelectIsFdSetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelectCopySet, osFuncBindPtr->osSocketSelectInfo.osSelectCopySetFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSelect, osFuncBindPtr->osSocketSelectInfo.osSelectFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssOsSocketGetSocketFdSetSize, osFuncBindPtr->osSocketSelectInfo.osSocketGetSocketFdSetSizeFunc);
    }
    else
    {
        cpssOsPrintf("cpssExtServicesBind: the mandatory pointer is NULL \n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if(traceFuncBindPtr)
    {
        REPLACE_IF_NOT_NULL_MAC(cpssTraceHwAccessWrite  , traceFuncBindPtr->traceHwBindInfo.traceHwAccessWriteFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssTraceHwAccessRead   , traceFuncBindPtr->traceHwBindInfo.traceHwAccessReadFunc);
        REPLACE_IF_NOT_NULL_MAC(cpssTraceHwAccessDelay   , traceFuncBindPtr->traceHwBindInfo.traceHwAccessDelayFunc);
    }
    else
    {
        REPLACE_IF_NOT_NULL_MAC(cpssTraceHwAccessWrite  , NULL);
        REPLACE_IF_NOT_NULL_MAC(cpssTraceHwAccessRead   , NULL);
        REPLACE_IF_NOT_NULL_MAC(cpssTraceHwAccessDelay   , NULL);
    }
#if defined(CPSS_LOG_ENABLE)
{
    GT_STATUS rc;
    rc = prvCpssLogInit();
    if(rc != GT_OK)
    {
        return rc;
    }
}
#endif
    return GT_OK;
}

