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
* @file prvCpssGlobalNonSharedOsFuncInitVars.c
*
* @brief This file handle global non shared OS function pointers initialization
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG


#ifndef CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG

#define STR_NOT_IMPLEMENTED_CNS " osServiceFuncNotImplementedCalled \n"


#define PRV_CPSS_INIT_OS_FUNC(_name,_type) \
         _name = (_type)osServiceFuncNotImplementedCalled;

static GT_STATUS osServiceFuncNotImplementedCalled
(
    void
)
{
    if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)osServiceFuncNotImplementedCalled)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}

static GT_UINTPTR osServiceFuncNotImplementedCalled0
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




#else
static void osServiceFuncNotImplementedCalledPrint(const char *name);
static GT_STATUS osServiceFuncNotImplementedCalled_cpssOsPrintf(void);


#define STR_NOT_IMPLEMENTED_CNS " osServiceFuncNotImplementedCalled %s\n"

    static void osServiceFuncNotImplementedCalledPrint(const char *name)
    {
        if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)osServiceFuncNotImplementedCalled_cpssOsPrintf)
        {
            /* we already have "printf" from the application
              but current pointer of a function was not initialized yet */
            cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS, name);
        }
    }


#define PRV_CPSS_DECLARE_DBG_OS_FUNC(_name)\
    \
    static GT_STATUS osServiceFuncNotImplementedCalled_ ## _name(void) \
    { \
        osServiceFuncNotImplementedCalledPrint(#_name); \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG); \
    } \

#define PRV_CPSS_DECLARE_DBG_OS_FUNC0(_name)\
    \
    static GT_STATUS osServiceFuncNotImplementedCalled_ ## _name(void) \
    { \
        osServiceFuncNotImplementedCalledPrint(#_name); \
        return 0 ;\
    } \


#define PRV_CPSS_INIT_OS_FUNC(_name,_type) \
        _name = (NULL== _name)?(_type)osServiceFuncNotImplementedCalled_ ## _name:_name;

   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFlastErrorStr)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFopen)
   PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsFclose)
   PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsRewind)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFprintf)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFgets)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFwrite)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFread)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsFgetLength)
   PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsFatalError)

   PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsBzero)
   PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMemSet)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsMemCpy)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsMemMove)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsMemCmp)
   PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStaticMalloc)
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
       PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsMalloc_MemoryLeakageDbg)
       PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsRealloc_MemoryLeakageDbg)
       PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsFree_MemoryLeakageDbg)
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
       PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMalloc)
       PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsRealloc)
       PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsFree)
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/

    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsCacheDmaMalloc)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsCacheDmaFree)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsPhy2Virt)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsVirt2Phy)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsGlobalDbShmemInit)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsGlobalDbShmemUnlink)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsRand)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSrand)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMutexCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMutexDelete)

#if defined  CPSS_USE_MUTEX_PROFILER
     PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMutexSetGlAttributes)
     PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsCheckUnprotectedPerDeviceDbAccess)
#endif
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrlen)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrCpy)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrNCpy)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrChr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrStr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrRChr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrCmp)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrNCmp)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrCat)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrNCat)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsToUpper)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrTo32)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrToU32)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrTol)


    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsSocketLastError)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketLastErrorStr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketTcpCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketUdpCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketTcpDestroy)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketUdpDestroy)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketCreateAddr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketDestroyAddr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketBind)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketListen)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketAccept)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketConnect)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketSetNonBlock)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketSetBlock)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketSend)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketSendTo)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketRecv)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketRecvFrom)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketSetSocketNoLinger)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketExtractIpAddrFromSocketAddr)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketGetSocketAddrSize)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketShutDown)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectCreateSet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectEraseSet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectZeroSet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectAddFdToSet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectClearFdFromSet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectIsFdSet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelectCopySet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSelect)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSocketGetSocketFdSetSize)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNtohl)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsHtonl)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNtohs)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsHtons)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsInetNtoa)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsBindStdOut)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsPrintf)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsVprintf)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSprintf)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsVsprintf)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSnprintf)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsVsnprintf)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsPrintSync)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsGets)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMutexLock)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMutexUnlock)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSigSemBinCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSigSemMCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSigSemCCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSigSemDelete)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSigSemWait)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsSigSemSignal)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNamedSemOpen)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNamedSemWait)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNamedSemPost)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNamedSemClose)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsNamedSemUnlink)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTimerWkAfter)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsTickGet)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsTime)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTimeRT)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsGetSysClockRate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsDelay)
    PRV_CPSS_DECLARE_DBG_OS_FUNC0(cpssOsStrfTime)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssTraceHwAccessWrite)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssTraceHwAccessRead)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssTraceHwAccessDelay)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTaskGetPid)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTaskAtProcExit)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTaskCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTaskDelete)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsTaskGetSelf)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsQsort)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsBsearch)

    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMsgQCreate)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMsgQDelete)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMsgQSend)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMsgQRecv)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsMsgQNumMsgs)
    PRV_CPSS_DECLARE_DBG_OS_FUNC(cpssOsLog)

#endif

    /**
     * @internal cpssGlobalNonSharedDbExtDrvFuncInit function
     * @endinternal
     *
     * @brief  Initialize global non shared function pointers
     *
     */
    GT_VOID cpssGlobalNonSharedDbOsFuncInit
    (
        GT_VOID
    )
    {

       PRV_CPSS_INIT_OS_FUNC(cpssOsFlastErrorStr, CPSS_OS_IO_F_LAST_ERROR_STR_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFopen, CPSS_OS_IO_FOPEN_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFclose, CPSS_OS_IO_FCLOSE_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsRewind, CPSS_OS_IO_REWIND_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFprintf, CPSS_OS_IO_FPRINTF_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFgets, CPSS_OS_IO_FGETS_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFwrite, CPSS_OS_IO_FWRITE_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFread, CPSS_OS_IO_FREAD_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFgetLength, CPSS_OS_IO_FGET_LENGTH_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsFatalError, CPSS_OS_FATAL_ERROR_FUNC);

       PRV_CPSS_INIT_OS_FUNC(cpssOsBzero, CPSS_OS_BZERO_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsMemSet, CPSS_OS_MEM_SET_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsMemCpy, CPSS_OS_MEM_CPY_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsMemMove, CPSS_OS_MEM_MOVE_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsMemCmp, CPSS_OS_MEM_CMP_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsStaticMalloc, CPSS_OS_STATIC_MALLOC_FUNC);
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
           PRV_CPSS_INIT_OS_FUNC(cpssOsMalloc_MemoryLeakageDbg, CPSS_OS_MALLOC_FUNC);
           PRV_CPSS_INIT_OS_FUNC(cpssOsRealloc_MemoryLeakageDbg, CPSS_OS_REALLOC_FUNC);
           PRV_CPSS_INIT_OS_FUNC(cpssOsFree_MemoryLeakageDbg, CPSS_OS_FREE_FUNC);
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
           PRV_CPSS_INIT_OS_FUNC(cpssOsMalloc, CPSS_OS_MALLOC_FUNC);
           PRV_CPSS_INIT_OS_FUNC(cpssOsRealloc, CPSS_OS_REALLOC_FUNC);
           PRV_CPSS_INIT_OS_FUNC(cpssOsFree, CPSS_OS_FREE_FUNC);
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/

       PRV_CPSS_INIT_OS_FUNC(cpssOsCacheDmaMalloc, CPSS_OS_CACHE_DMA_MALLOC_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsCacheDmaFree, CPSS_OS_CACHE_DMA_FREE_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsPhy2Virt, CPSS_OS_PHY_TO_VIRT_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsVirt2Phy, CPSS_OS_VIRT_TO_PHY_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsGlobalDbShmemInit, CPSS_OS_MEM_GLOBAL_DB_SHMEM_INIT);
       PRV_CPSS_INIT_OS_FUNC(cpssOsGlobalDbShmemUnlink, CPSS_OS_MEM_GLOBAL_DB_SHMEM_UNLINK);



       PRV_CPSS_INIT_OS_FUNC(cpssOsRand, CPSS_OS_RAND_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsSrand, CPSS_OS_SRAND_FUNC);

       PRV_CPSS_INIT_OS_FUNC(cpssOsMutexCreate, CPSS_OS_MUTEX_CREATE_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsMutexDelete, CPSS_OS_MUTEX_DELETE_FUNC);

#if defined  CPSS_USE_MUTEX_PROFILER
       PRV_CPSS_INIT_OS_FUNC(cpssOsMutexSetGlAttributes, CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC);
       PRV_CPSS_INIT_OS_FUNC(cpssOsCheckUnprotectedPerDeviceDbAccess, CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS);
#endif
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrlen, CPSS_OS_STR_LEN_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrCpy, CPSS_OS_STR_CPY_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrNCpy, CPSS_OS_STR_N_CPY_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrChr, CPSS_OS_STR_CHR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrStr, CPSS_OS_STR_STR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrRChr, CPSS_OS_STR_REV_CHR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrCmp, CPSS_OS_STR_CMP_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrNCmp, CPSS_OS_STR_N_CMP_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrCat, CPSS_OS_STR_CAT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrNCat, CPSS_OS_STR_N_CAT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsToUpper, CPSS_OS_TO_UPPER_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrTo32, CPSS_OS_STR_TO_32_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrToU32, CPSS_OS_STR_TO_U32_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrTol, CPSS_OS_STR_TOL_FUNC);


        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketLastError, CPSS_SOCKET_LAST_ERROR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketLastErrorStr, CPSS_SOCKET_LAST_ERROR_STR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketTcpCreate, CPSS_SOCKET_TCP_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketUdpCreate, CPSS_SOCKET_UDP_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketTcpDestroy, CPSS_SOCKET_TCP_DESTROY_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketUdpDestroy, CPSS_SOCKET_UDP_DESTROY_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketCreateAddr, CPSS_SOCKET_CREATE_ADDR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketDestroyAddr, CPSS_SOCKET_DESTROY_ADDR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketBind, CPSS_SOCKET_BIND_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketListen, CPSS_SOCKET_LISTEN_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketAccept, CPSS_SOCKET_ACCEPT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketConnect, CPSS_SOCKET_CONNECT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketSetNonBlock, CPSS_SOCKET_SET_NONBLOCK_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketSetBlock, CPSS_SOCKET_SET_BLOCK_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketSend, CPSS_SOCKET_SEND_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketSendTo, CPSS_SOCKET_SENDTO_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketRecv, CPSS_SOCKET_RECV_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketRecvFrom, CPSS_SOCKET_RECVFROM_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketSetSocketNoLinger, CPSS_SOCKET_SET_SOCKET_NOLINGER_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketExtractIpAddrFromSocketAddr, CPSS_SOCKET_EXTRACT_IPADDR_FROM_SOCKET_ADDR_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketGetSocketAddrSize, CPSS_SOCKET_GET_ADDR_SIZE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketShutDown, CPSS_SOCKET_SHUTDOWN_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectCreateSet, CPSS_SOCKET_SELECT_CREATE_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectEraseSet, CPSS_SOCKET_SELECT_ERASE_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectZeroSet, CPSS_SOCKET_SELECT_ZERO_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectAddFdToSet, CPSS_SOCKET_SELECT_ADD_FD_TO_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectClearFdFromSet, CPSS_SOCKET_SELECT_CLEAR_FD_FROM_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectIsFdSet, CPSS_SOCKET_SELECT_IS_FD_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelectCopySet, CPSS_SOCKET_SELECT_COPY_SET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSelect, CPSS_SOCKET_SELECT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSocketGetSocketFdSetSize, CPSS_SOCKET_SELECT_GET_SOCKET_FD_SIZE_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsNtohl, CPSS_OS_INET_NTOHL_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsHtonl, CPSS_OS_INET_HTONL_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsNtohs, CPSS_OS_INET_NTOHS_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsHtons, CPSS_OS_INET_HTONS_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsInetNtoa, CPSS_OS_INET_NTOA_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsBindStdOut, CPSS_OS_IO_BIND_STDOUT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsPrintf, CPSS_OS_IO_PRINTF_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsVprintf, CPSS_OS_IO_VPRINTF_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSprintf, CPSS_OS_IO_SPRINTF_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsVsprintf, CPSS_OS_IO_VSPRINTF_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSnprintf, CPSS_OS_IO_SNPRINTF_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsVsnprintf, CPSS_OS_IO_VSNPRINTF_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsPrintSync, CPSS_OS_IO_PRINT_SYNC_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsGets, CPSS_OS_IO_GETS_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsMutexLock, CPSS_OS_MUTEX_LOCK_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsMutexUnlock, CPSS_OS_MUTEX_UNLOCK_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsSigSemBinCreate, CPSS_OS_SIG_SEM_BIN_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSigSemMCreate, CPSS_OS_SIG_SEM_M_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSigSemCCreate, CPSS_OS_SIG_SEM_C_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSigSemDelete, CPSS_OS_SIG_SEM_DELETE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSigSemWait, CPSS_OS_SIG_SEM_WAIT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsSigSemSignal, CPSS_OS_SIG_SEM_SIGNAL_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsNamedSemOpen, CPSS_OS_NAMED_SEM_OPEN_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsNamedSemWait, CPSS_OS_NAMED_SEM_WAIT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsNamedSemPost, CPSS_OS_NAMED_SEM_POST_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsNamedSemClose, CPSS_OS_NAMED_SEM_CLOSE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsNamedSemUnlink, CPSS_OS_NAMED_SEM_UNLINK_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsTimerWkAfter, CPSS_OS_TIME_WK_AFTER_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsTickGet, CPSS_OS_TIME_TICK_GET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsTime, CPSS_OS_TIME_GET_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsTimeRT, CPSS_OS_TIME_RT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsGetSysClockRate, CPSS_OS_GET_SYS_CLOCK_RATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsDelay, CPSS_OS_DELAY_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsStrfTime, CPSS_OS_STRF_TIME_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssTraceHwAccessWrite, CPSS_TRACE_HW_ACCESS_WRITE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssTraceHwAccessRead, CPSS_TRACE_HW_ACCESS_READ_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssTraceHwAccessDelay, CPSS_TRACE_HW_ACCESS_DELAY_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsTaskGetPid, CPSS_OS_TASK_GET_PID_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsTaskAtProcExit, CPSS_OS_TASK_AT_PROC_EXIT_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsTaskCreate, CPSS_OS_TASK_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsTaskDelete, CPSS_OS_TASK_DELETE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsTaskGetSelf, CPSS_OS_TASK_GET_SELF_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsQsort, CPSS_OS_STDLIB_QSORT_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsBsearch, CPSS_OS_STDLIB_BSEARCH_FUNC);

        PRV_CPSS_INIT_OS_FUNC(cpssOsMsgQCreate, CPSS_OS_MSGQ_CREATE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsMsgQDelete, CPSS_OS_MSGQ_DELETE_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsMsgQSend, CPSS_OS_MSGQ_SEND_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsMsgQRecv, CPSS_OS_MSGQ_RECV_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsMsgQNumMsgs, CPSS_OS_MSGQ_NUM_MSGS_FUNC);
        PRV_CPSS_INIT_OS_FUNC(cpssOsLog, CPSS_OS_LOG_FUNC);

    }




