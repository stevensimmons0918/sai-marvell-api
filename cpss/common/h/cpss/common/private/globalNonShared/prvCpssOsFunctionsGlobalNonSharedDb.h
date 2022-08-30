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
* @file prvCpssOsFunctionsGlobalNonSharedDb.h.h
*
* @brief This file os function pointers  non shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssOsFunctionsGlobalNonSharedDb
#define __prvCpssOsFunctionsGlobalNonSharedDb

#include <cpss/extServices/os/gtOs/cpssOsStr.h>


#define PRV_CPSS_OS_DECLARE_FUNC(_name,_type) _type _name

/**
* @struct PRV_CPSS_OS_FUNC_PTR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain OS function pointers  that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module
*/
typedef struct
{

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFlastErrorStr, CPSS_OS_IO_F_LAST_ERROR_STR_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFopen, CPSS_OS_IO_FOPEN_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFclose, CPSS_OS_IO_FCLOSE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsRewind, CPSS_OS_IO_REWIND_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFprintf, CPSS_OS_IO_FPRINTF_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFgets, CPSS_OS_IO_FGETS_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFwrite, CPSS_OS_IO_FWRITE_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFread, CPSS_OS_IO_FREAD_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFgetLength, CPSS_OS_IO_FGET_LENGTH_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsFatalError, CPSS_OS_FATAL_ERROR_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsBzero, CPSS_OS_BZERO_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMemSet, CPSS_OS_MEM_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMemCpy, CPSS_OS_MEM_CPY_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMemMove, CPSS_OS_MEM_MOVE_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMemCmp, CPSS_OS_MEM_CMP_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStaticMalloc, CPSS_OS_STATIC_MALLOC_FUNC);/*0*/
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
       PRV_CPSS_OS_DECLARE_FUNC(cpssOsMalloc_MemoryLeakageDbg, CPSS_OS_MALLOC_FUNC);/*0*/
       PRV_CPSS_OS_DECLARE_FUNC(cpssOsRealloc_MemoryLeakageDbg, CPSS_OS_REALLOC_FUNC);/*0*/
       PRV_CPSS_OS_DECLARE_FUNC(cpssOsFree_MemoryLeakageDbg, CPSS_OS_FREE_FUNC);
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
       PRV_CPSS_OS_DECLARE_FUNC(cpssOsMalloc, CPSS_OS_MALLOC_FUNC);/*0*/
       PRV_CPSS_OS_DECLARE_FUNC(cpssOsRealloc, CPSS_OS_REALLOC_FUNC);/*0*/
       PRV_CPSS_OS_DECLARE_FUNC(cpssOsFree, CPSS_OS_FREE_FUNC);
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsCacheDmaMalloc, CPSS_OS_CACHE_DMA_MALLOC_FUNC);/*0*/
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsCacheDmaFree, CPSS_OS_CACHE_DMA_FREE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsPhy2Virt, CPSS_OS_PHY_TO_VIRT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsVirt2Phy, CPSS_OS_VIRT_TO_PHY_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsGlobalDbShmemInit, CPSS_OS_MEM_GLOBAL_DB_SHMEM_INIT);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsGlobalDbShmemUnlink, CPSS_OS_MEM_GLOBAL_DB_SHMEM_UNLINK);



   PRV_CPSS_OS_DECLARE_FUNC(cpssOsRand, CPSS_OS_RAND_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSrand, CPSS_OS_SRAND_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMutexCreate, CPSS_OS_MUTEX_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMutexDelete, CPSS_OS_MUTEX_DELETE_FUNC);

#if defined  CPSS_USE_MUTEX_PROFILER
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMutexSetGlAttributes, CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsCheckUnprotectedPerDeviceDbAccess, CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS);
#endif

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrlen, CPSS_OS_STR_LEN_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrCpy, CPSS_OS_STR_CPY_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrNCpy, CPSS_OS_STR_N_CPY_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrChr, CPSS_OS_STR_CHR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrStr, CPSS_OS_STR_STR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrRChr, CPSS_OS_STR_REV_CHR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrCmp, CPSS_OS_STR_CMP_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrNCmp, CPSS_OS_STR_N_CMP_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrCat, CPSS_OS_STR_CAT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrNCat, CPSS_OS_STR_N_CAT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsToUpper, CPSS_OS_TO_UPPER_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrTo32, CPSS_OS_STR_TO_32_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrToU32, CPSS_OS_STR_TO_U32_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrTol, CPSS_OS_STR_TOL_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketLastError, CPSS_SOCKET_LAST_ERROR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketLastErrorStr, CPSS_SOCKET_LAST_ERROR_STR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketTcpCreate, CPSS_SOCKET_TCP_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketUdpCreate, CPSS_SOCKET_UDP_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketTcpDestroy, CPSS_SOCKET_TCP_DESTROY_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketUdpDestroy, CPSS_SOCKET_UDP_DESTROY_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketCreateAddr, CPSS_SOCKET_CREATE_ADDR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketDestroyAddr, CPSS_SOCKET_DESTROY_ADDR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketBind, CPSS_SOCKET_BIND_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketListen, CPSS_SOCKET_LISTEN_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketAccept, CPSS_SOCKET_ACCEPT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketConnect, CPSS_SOCKET_CONNECT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketSetNonBlock, CPSS_SOCKET_SET_NONBLOCK_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketSetBlock, CPSS_SOCKET_SET_BLOCK_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketSend, CPSS_SOCKET_SEND_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketSendTo, CPSS_SOCKET_SENDTO_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketRecv, CPSS_SOCKET_RECV_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketRecvFrom, CPSS_SOCKET_RECVFROM_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketSetSocketNoLinger, CPSS_SOCKET_SET_SOCKET_NOLINGER_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketExtractIpAddrFromSocketAddr, CPSS_SOCKET_EXTRACT_IPADDR_FROM_SOCKET_ADDR_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketGetSocketAddrSize, CPSS_SOCKET_GET_ADDR_SIZE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketShutDown, CPSS_SOCKET_SHUTDOWN_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectCreateSet, CPSS_SOCKET_SELECT_CREATE_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectEraseSet, CPSS_SOCKET_SELECT_ERASE_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectZeroSet, CPSS_SOCKET_SELECT_ZERO_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectAddFdToSet, CPSS_SOCKET_SELECT_ADD_FD_TO_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectClearFdFromSet, CPSS_SOCKET_SELECT_CLEAR_FD_FROM_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectIsFdSet, CPSS_SOCKET_SELECT_IS_FD_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelectCopySet, CPSS_SOCKET_SELECT_COPY_SET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSelect, CPSS_SOCKET_SELECT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSocketGetSocketFdSetSize, CPSS_SOCKET_SELECT_GET_SOCKET_FD_SIZE_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNtohl, CPSS_OS_INET_NTOHL_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsHtonl, CPSS_OS_INET_HTONL_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNtohs, CPSS_OS_INET_NTOHS_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsHtons, CPSS_OS_INET_HTONS_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsInetNtoa, CPSS_OS_INET_NTOA_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsBindStdOut, CPSS_OS_IO_BIND_STDOUT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsPrintf, CPSS_OS_IO_PRINTF_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsVprintf, CPSS_OS_IO_VPRINTF_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSprintf, CPSS_OS_IO_SPRINTF_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsVsprintf, CPSS_OS_IO_VSPRINTF_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSnprintf, CPSS_OS_IO_SNPRINTF_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsVsnprintf, CPSS_OS_IO_VSNPRINTF_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsPrintSync, CPSS_OS_IO_PRINT_SYNC_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsGets, CPSS_OS_IO_GETS_FUNC);/*0*/

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMutexLock, CPSS_OS_MUTEX_LOCK_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMutexUnlock, CPSS_OS_MUTEX_UNLOCK_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSigSemBinCreate, CPSS_OS_SIG_SEM_BIN_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSigSemMCreate, CPSS_OS_SIG_SEM_M_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSigSemCCreate, CPSS_OS_SIG_SEM_C_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSigSemDelete, CPSS_OS_SIG_SEM_DELETE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSigSemWait, CPSS_OS_SIG_SEM_WAIT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsSigSemSignal, CPSS_OS_SIG_SEM_SIGNAL_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNamedSemOpen, CPSS_OS_NAMED_SEM_OPEN_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNamedSemWait, CPSS_OS_NAMED_SEM_WAIT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNamedSemPost, CPSS_OS_NAMED_SEM_POST_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNamedSemClose, CPSS_OS_NAMED_SEM_CLOSE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsNamedSemUnlink, CPSS_OS_NAMED_SEM_UNLINK_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTimerWkAfter, CPSS_OS_TIME_WK_AFTER_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTickGet, CPSS_OS_TIME_TICK_GET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTime, CPSS_OS_TIME_GET_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTimeRT, CPSS_OS_TIME_RT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsGetSysClockRate, CPSS_OS_GET_SYS_CLOCK_RATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsDelay, CPSS_OS_DELAY_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsStrfTime, CPSS_OS_STRF_TIME_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssTraceHwAccessWrite, CPSS_TRACE_HW_ACCESS_WRITE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssTraceHwAccessRead, CPSS_TRACE_HW_ACCESS_READ_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssTraceHwAccessDelay, CPSS_TRACE_HW_ACCESS_DELAY_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTaskGetPid, CPSS_OS_TASK_GET_PID_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTaskAtProcExit, CPSS_OS_TASK_AT_PROC_EXIT_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTaskCreate, CPSS_OS_TASK_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTaskDelete, CPSS_OS_TASK_DELETE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsTaskGetSelf, CPSS_OS_TASK_GET_SELF_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsQsort, CPSS_OS_STDLIB_QSORT_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsBsearch, CPSS_OS_STDLIB_BSEARCH_FUNC);

   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMsgQCreate, CPSS_OS_MSGQ_CREATE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMsgQDelete, CPSS_OS_MSGQ_DELETE_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMsgQSend, CPSS_OS_MSGQ_SEND_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMsgQRecv, CPSS_OS_MSGQ_RECV_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsMsgQNumMsgs, CPSS_OS_MSGQ_NUM_MSGS_FUNC);
   PRV_CPSS_OS_DECLARE_FUNC(cpssOsLog, CPSS_OS_LOG_FUNC);

} PRV_CPSS_OS_FUNC_PTR_NON_SHARED_GLOBAL_DB;


#define cpssOsFlastErrorStr PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFlastErrorStr)
#define cpssOsFopen PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFopen)
#define cpssOsFclose PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFclose)
#define cpssOsRewind PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsRewind)
#define cpssOsFprintf PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFprintf)
#define cpssOsFgets PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFgets)
#define cpssOsFwrite PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFwrite)
#define cpssOsFread PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFread)
#define cpssOsFgetLength PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFgetLength)
#define cpssOsFatalError PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFatalError)

#define cpssOsBzero PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsBzero)
#define cpssOsMemSet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMemSet)
#define cpssOsMemCpy PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMemCpy)
#define cpssOsMemMove PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMemMove)
#define cpssOsMemCmp PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMemCmp)
#define cpssOsStaticMalloc PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStaticMalloc)
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
  #define cpssOsMalloc_MemoryLeakageDbg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMalloc_MemoryLeakageDbg)
  #define cpssOsRealloc_MemoryLeakageDbg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsRealloc_MemoryLeakageDbg)
  #define cpssOsFree_MemoryLeakageDbg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFree_MemoryLeakageDbg)
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
  #define cpssOsMalloc PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMalloc)
  #define cpssOsRealloc PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsRealloc)
  #define cpssOsFree PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsFree)
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/

#define cpssOsCacheDmaMalloc PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsCacheDmaMalloc)
#define cpssOsCacheDmaFree PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsCacheDmaFree)
#define cpssOsPhy2Virt PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsPhy2Virt)
#define cpssOsVirt2Phy PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsVirt2Phy)
#define cpssOsGlobalDbShmemInit PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsGlobalDbShmemInit)
#define cpssOsGlobalDbShmemUnlink PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsGlobalDbShmemUnlink)

#define cpssOsRand PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsRand)
#define cpssOsSrand PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSrand)

#define cpssOsMutexCreate PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMutexCreate)
#define cpssOsMutexDelete PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMutexDelete)

#if defined  CPSS_USE_MUTEX_PROFILER
  #define cpssOsMutexSetGlAttributes PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMutexSetGlAttributes)
  #define cpssOsCheckUnprotectedPerDeviceDbAccess PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsCheckUnprotectedPerDeviceDbAccess)
#endif

#define cpssOsStrlen   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrlen)
#define cpssOsStrCpy   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrCpy)
#define cpssOsStrNCpy  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrNCpy)
#define cpssOsStrChr   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrChr)
#define cpssOsStrStr   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrStr)
#define cpssOsStrRChr  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrRChr)
#define cpssOsStrCmp   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrCmp)
#define cpssOsStrNCmp  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrNCmp)
#define cpssOsStrCat   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrCat)
#define cpssOsStrNCat  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrNCat)
#define cpssOsToUpper  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsToUpper)
#define cpssOsStrTo32  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrTo32)
#define cpssOsStrToU32 PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrToU32)
#define cpssOsStrTol   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrTol)


#define cpssOsSocketLastError    PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketLastError)
#define cpssOsSocketLastErrorStr PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketLastErrorStr)
#define cpssOsSocketTcpCreate    PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketTcpCreate)
#define cpssOsSocketUdpCreate    PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketUdpCreate)
#define cpssOsSocketTcpDestroy   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketTcpDestroy)
#define cpssOsSocketUdpDestroy   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketUdpDestroy)
#define cpssOsSocketCreateAddr   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketCreateAddr)
#define cpssOsSocketDestroyAddr  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketDestroyAddr)
#define cpssOsSocketBind         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketBind)
#define cpssOsSocketListen       PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketListen)
#define cpssOsSocketAccept       PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketAccept)
#define cpssOsSocketConnect      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketConnect)
#define cpssOsSocketSetNonBlock  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketSetNonBlock)
#define cpssOsSocketSetBlock     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketSetBlock)
#define cpssOsSocketSend         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketSend)
#define cpssOsSocketSendTo       PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketSendTo)
#define cpssOsSocketRecv         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketRecv)
#define cpssOsSocketRecvFrom     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketRecvFrom)
#define cpssOsSocketSetSocketNoLinger PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketSetSocketNoLinger)
#define cpssOsSocketExtractIpAddrFromSocketAddr PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketExtractIpAddrFromSocketAddr)
#define cpssOsSocketGetSocketAddrSize PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketGetSocketAddrSize)
#define cpssOsSocketShutDown     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketShutDown)

#define cpssOsSelectCreateSet    PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectCreateSet)
#define cpssOsSelectEraseSet     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectEraseSet)
#define cpssOsSelectZeroSet      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectZeroSet)
#define cpssOsSelectAddFdToSet   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectAddFdToSet)
#define cpssOsSelectClearFdFromSet  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectClearFdFromSet)
#define cpssOsSelectIsFdSet      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectIsFdSet)
#define cpssOsSelectCopySet      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelectCopySet)
#define cpssOsSelect             PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSelect)
#define cpssOsSocketGetSocketFdSetSize  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSocketGetSocketFdSetSize)

#define cpssOsNtohl              PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNtohl)
#define cpssOsHtonl              PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsHtonl)
#define cpssOsNtohs              PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNtohs)
#define cpssOsHtons              PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsHtons)
#define cpssOsInetNtoa           PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsInetNtoa)

#define cpssOsBindStdOut         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsBindStdOut)
#define cpssOsPrintf             PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsPrintf)
#define cpssOsVprintf            PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsVprintf)
#define cpssOsSprintf            PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSprintf)
#define cpssOsVsprintf           PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsVsprintf)
#define cpssOsSnprintf           PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSnprintf)
#define cpssOsVsnprintf          PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsVsnprintf)

#define cpssOsPrintSync          PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsPrintSync)
#define cpssOsGets               PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsGets)

#define cpssOsMutexLock         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMutexLock)
#define cpssOsMutexUnlock       PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMutexUnlock)

#define cpssOsSigSemBinCreate   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSigSemBinCreate)
#define cpssOsSigSemMCreate     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSigSemMCreate)
#define cpssOsSigSemCCreate     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSigSemCCreate)
#define cpssOsSigSemDelete      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSigSemDelete)
#define cpssOsSigSemWait        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSigSemWait)
#define cpssOsSigSemSignal      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsSigSemSignal)
#define cpssOsNamedSemOpen      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNamedSemOpen)
#define cpssOsNamedSemWait      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNamedSemWait)
#define cpssOsNamedSemPost      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNamedSemPost)
#define cpssOsNamedSemClose     PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNamedSemClose)
#define cpssOsNamedSemUnlink    PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsNamedSemUnlink)

#define cpssOsTimerWkAfter      PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTimerWkAfter)
#define cpssOsTickGet           PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTickGet)
#define cpssOsTime              PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTime)
#define cpssOsTimeRT            PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTimeRT)
#define cpssOsGetSysClockRate   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsGetSysClockRate)
#define cpssOsDelay             PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsDelay)
#define cpssOsStrfTime          PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsStrfTime)

#define cpssTraceHwAccessWrite  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssTraceHwAccessWrite)
#define cpssTraceHwAccessRead   PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssTraceHwAccessRead)
#define cpssTraceHwAccessDelay  PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssTraceHwAccessDelay)

#define cpssOsTaskGetPid        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTaskGetPid)
#define cpssOsTaskAtProcExit    PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTaskAtProcExit)

#define cpssOsTaskCreate        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTaskCreate)
#define cpssOsTaskDelete        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTaskDelete)
#define cpssOsTaskGetSelf       PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsTaskGetSelf)

#define cpssOsQsort             PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsQsort)
#define cpssOsBsearch           PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsBsearch)

#define cpssOsMsgQCreate        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMsgQCreate)
#define cpssOsMsgQDelete        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMsgQDelete)
#define cpssOsMsgQSend          PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMsgQSend)
#define cpssOsMsgQRecv          PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMsgQRecv)
#define cpssOsMsgQNumMsgs       PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsMsgQNumMsgs)
#define cpssOsLog               PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.osFuncDb.cpssOsLog)

#endif /* __prvCpssOsFunctionsGlobalNonSharedDb */

