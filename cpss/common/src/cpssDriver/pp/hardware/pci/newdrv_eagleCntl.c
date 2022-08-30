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
* @file newdrv_eagleCntl.c
*
* @brief Driver for EAGLE PEX
*       Supports full memory mapped address space
*       (Naive windowing support included)
* Resources are mapped to user-space
*
* @version   1
********************************************************************************
*/
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif
/* cpss allowed only 4MB (4 windows of 1MB), Other 4MB goes to external utilities (vUART(s)) , kernel */
#define CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM 4
/* Current window size 1M */
#define WINDOW_SIZE          _1M

#define REGISTER_OFFSET_BITS_MASK (WINDOW_SIZE - 1)
#define REGISTER_WINDOW_BITS_MASK ~(WINDOW_SIZE - 1)

typedef struct CPSS_HW_DRIVER_GEN_AMAP_WIN64_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_UINTPTR          base;
    GT_UINTPTR          size;
    struct {
        GT_UINTPTR      reg;        /* Register offset in BAR0 to configure the "mapAddress" value */
        GT_U32          mapAddress; /* Translated Base Address */
        GT_UINTPTR      winBase;    /* Base Address in BAR = BAR base + offset */
    } window[CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM]; /* address mapping windows 0..63 */
    GT_U32 lastMatchedIndex;/* cache of last matched (for consecutive access) */
    GT_U32 numOfWindows;
    GT_U32 nextWindowIndex;
    CPSS_HW_DRIVER_STC* bar0;
    struct {
        GT_U32   busNo;
        GT_U32   devSel;
        GT_U32   funcNo;
    } hwAddr;

} CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC;

/* PEX ATU (Address Translation Unit) registers */
#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200
#define ATU_REGION_CTRL_1_REG         0x100
#define ATU_REGION_CTRL_2_REG         0x104
#define ATU_LOWER_BASE_ADDRESS_REG    0x108
#define ATU_UPPER_BASE_ADDRESS_REG    0x10C
#define ATU_LIMIT_ADDRESS_REG         0x110
#define ATU_LOWER_TARGET_ADDRESS_REG  0x114
#define ATU_UPPER_TARGET_ADDRESS_REG  0x118

#define iATU_REGISTER_ADDRESS(_iWindow, _register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_iWindow)*0x200 + (_register))

/*#define PEX_DELAY*/
#ifdef PEX_DELAY
#define PEX_DELAY_MS cpssOsTimerWkAfter(5)
#else
#define PEX_DELAY_MS
#endif

#define PEX_ACCESS_TRACE
#ifdef PEX_ACCESS_TRACE

CPSS_TBD_BOOKMARK_AC5P /* PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS not proper for hawk*/

#define GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS   0x30

/* the NO_KM module is polling the interrupt summary register of the device */
/* as the polling is done 5 or 50 times a second , we not want the 'trace' to be with this register access indication */
#define FALCON_POLLING_TASK_REG_ADD    (PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS + GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS)
#define AC5P_POLLING_TASK_REG_ADD      (PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS   + GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS)
#define PHOENIX_POLLING_TASK_REG_ADD   (PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS+ GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS)
#define AC5_POLLING_TASK_REG_ADD       (0                                    + GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS)

#define IS_MG0_INTERRUPT_CAUSE_REG_MAC(regAddr)         \
    /* first line check is for 'fast check' if register address is 'potential' for the global cause register */ \
    ((((regAddr) & 0x000FFFFF/*1M*/) != GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS) ? 0 : \
    /* next lines check if it belongs to MG of known device */ \
     (((regAddr) == FALCON_POLLING_TASK_REG_ADD      ||  \
      (regAddr) == AC5P_POLLING_TASK_REG_ADD        ||  \
      (regAddr) == AC5_POLLING_TASK_REG_ADD         ||  \
      (regAddr) == PHOENIX_POLLING_TASK_REG_ADD) ? 1 : 0))

#define NEED_TO_IGNORE_REGISTER_TRACE_MAC(regAddr)  \
    /* as the polling is done 5 or 50 times a second , we not want the 'trace' to be with this register access indication */ \
    IS_MG0_INTERRUPT_CAUSE_REG_MAC(regAddr)


GT_BOOL pex_access_trace_enable = GT_FALSE;
GT_U32 pex_access_trace_delay = 0;
void (*pex_access_trace_pre_write_callback)(GT_U32 addr, GT_U32 data) = NULL;

#define PEX_ACCESS_TRACE_PRINTF(...) \
    if( GT_TRUE == pex_access_trace_enable) \
    {                                       \
        cpssOsPrintf(__VA_ARGS__);          \
    }
#define REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,...) \
    if( GT_TRUE == pex_access_trace_enable) \
    {                                       \
        if(0 == NEED_TO_IGNORE_REGISTER_TRACE_MAC(regAddr))\
        {                                   \
            cpssOsPrintf(__VA_ARGS__);      \
        }                                   \
    }
#define REG_PEX_REG_ACCESS_TRACE_DELAY \
    if( 0 != pex_access_trace_delay)   \
    {                                                \
        if(0 == NEED_TO_IGNORE_REGISTER_TRACE_MAC(regAddr))\
        {                                            \
            cpssOsTimerWkAfter(pex_access_trace_delay);  \
        }                                            \
    }
#else
#define PEX_ACCESS_TRACE_PRINTF(...)
#define REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,...)
#define REG_PEX_REG_ACCESS_TRACE_DELAY
#endif

#define ATU_DEBUG_STAT
#ifdef ATU_DEBUG_STAT
CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *debugDrv = NULL;
GT_U32 numOfAccess = 0;
GT_U32 numOfHits = 0;
GT_U32 windowHit[CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM];
GT_U32 windowReplace[CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM];
#endif

#ifdef SHARED_MEMORY

GT_VOID prvCpssDrvHwDrvReload
(
    IN CPSS_HW_DRIVER_STC          *drv

);
#endif

/* resolve address mapping window */
GT_INLINE GT_STATUS prvGetWindow(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv,
    IN  GT_U32      regAddr,
    OUT GT_UINTPTR  *winBase,
    OUT GT_U32      *addrInWin,
    INOUT GT_BOOL   *locked
)
{
    GT_U32 ii;
    GT_U32 data;
    GT_STATUS   rc;
    CPSS_HW_DRIVER_METHOD_READ       readFunc;
    CPSS_HW_DRIVER_METHOD_WRITE_MASK writeMaskFunc;
#ifdef SHARED_MEMORY
    /*function pointers that are relevant to the process ,
            we can't use from shared  drv since they may be not maped to the processs.*/
    CPSS_HW_DRIVER_STC              localDrv;
#endif

    if (*locked == GT_FALSE)
    {
        cpssOsMutexLock(drv->mtx);
        *locked = GT_TRUE;
    }

#ifdef ATU_DEBUG_STAT
    numOfAccess++;
#endif

    data = regAddr & REGISTER_WINDOW_BITS_MASK;

    /* check the cache of last matched (for consecutive access) */
    {
        ii = drv->lastMatchedIndex;
        if(drv->window[ii].mapAddress == data)
        {
            *winBase = drv->window[ii].winBase;
            *addrInWin = (regAddr & REGISTER_OFFSET_BITS_MASK);
            drv->lastMatchedIndex = ii;
#ifdef ATU_DEBUG_STAT
            numOfHits++;
            windowHit[ii]++;
#endif
            return GT_OK;
        }
    }

    for (ii = 0; ii < drv->numOfWindows; ii++)
    {
        /* compare window mapping (1M size) to reg address */
        if (drv->window[ii].mapAddress == data)
        {
            *winBase = drv->window[ii].winBase;
            *addrInWin = (regAddr & REGISTER_OFFSET_BITS_MASK);
            drv->lastMatchedIndex = ii;
#ifdef ATU_DEBUG_STAT
            numOfHits++;
            windowHit[ii]++;
#endif

            return GT_OK;
        }
    }

    ii = drv->nextWindowIndex;
    drv->nextWindowIndex++;
    if (drv->nextWindowIndex >= drv->numOfWindows)
        drv->nextWindowIndex = 0;

#ifdef ATU_DEBUG_STAT
    windowReplace[ii]++;
#endif

#ifdef SHARED_MEMORY
     localDrv.type =drv->bar0->type;
     /*need to reload function pointers due to ASLR*/
     prvCpssDrvHwDrvReload(&localDrv);
     readFunc = localDrv.read;
     writeMaskFunc = localDrv.writeMask;
#else
    readFunc = drv->bar0->read;
    writeMaskFunc =  drv->bar0->writeMask;
#endif

    /* access to BAR0 to configure mapped address*/
    rc = writeMaskFunc(drv->bar0, 0,
            drv->window[ii].reg, &data, 1, 0xFFFFFFFF);
    PEX_ACCESS_TRACE_PRINTF("PEX BAR0 write: rc %d, regAddr 0x%8.8x, data 0x%8.8x\n", rc, drv->window[ii].reg, data);
    if (rc != GT_OK)
        return rc;

    GT_SYNC; /* to avoid from write combining */

/* Patch due to PEX timing\race problem */
    {
       GT_U32 tempData;
       rc = readFunc(drv->bar0, 0, drv->window[ii].reg, &tempData, 1);
       if (rc != GT_OK)
           return rc;
    }

    PEX_DELAY_MS;

    drv->window[ii].mapAddress = data;
    *winBase = drv->window[ii].winBase;
    *addrInWin = (regAddr & REGISTER_OFFSET_BITS_MASK);
    drv->lastMatchedIndex = ii;

    return GT_OK;
}

static GT_STATUS cpssHwDriverEagleAmapWin64Read(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_UINTPTR  winBase = 0;
    GT_U32      addrInWin = WINDOW_SIZE;
    GT_U32      data;
    GT_BOOL     locked = GT_FALSE;
    GT_STATUS rc = GT_BAD_PARAM;

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"PEX Read: regAddr 0x%8.8x\n", regAddr);
    REG_PEX_REG_ACCESS_TRACE_DELAY;

    if(addrSpace != CPSS_HW_DRIVER_AS_SWITCHING_E)
    {
        /* CPSS_HW_DRIVER_AS_CNM_E is used to enable access to the configuration */
        /* space for debug purpose with Internal PCI APIs. */
        if( addrSpace == CPSS_HW_DRIVER_AS_PCI_CONFIG_E ||
            addrSpace == CPSS_HW_DRIVER_AS_CNM_E)
        {
            /* Configuration Space access */
            return cpssExtDrvPciConfigReadReg(drv->hwAddr.busNo,
                                              drv->hwAddr.devSel,
                                              drv->hwAddr.funcNo,
                                              regAddr, dataPtr);
        }
        else if ( addrSpace == CPSS_HW_DRIVER_AS_ATU_E )
        {
            return(drv->bar0->read(drv->bar0, 0, regAddr, dataPtr, count));
        }
        else if ( addrSpace == CPSS_HW_DRIVER_AS_DFX_E )
        {
            return /* This is no real error, only used to indicate Falcon */ GT_NO_RESOURCE;
        }
    }

    for (; count; count--,dataPtr++)
    {
        if (addrInWin >= WINDOW_SIZE)
        {
            rc = prvGetWindow(drv, regAddr, &winBase, &addrInWin, &locked);
            if (rc != GT_OK)
                break;
        }
#ifdef ASIC_SIMULATION
        ASIC_SIMULATION_ReadMemory(drv->base, regAddr, 1, &data , winBase + addrInWin);
#else
        data = *((volatile GT_U32*)(winBase + addrInWin));
#endif
        GT_SYNC; /* to avoid read combining */
        PEX_DELAY_MS;
        *dataPtr = CPSS_32BIT_LE(data);
        addrInWin += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr-4,"PEX Read: rc %d, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr-4, *(dataPtr-1));

    return rc;
}

static GT_STATUS cpssHwDriverEagleAmapWin64WriteMask(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_UINTPTR  winBase = 0;
    GT_U32      addrInWin = WINDOW_SIZE;
    GT_U32      data = 0;
    GT_BOOL     locked = GT_FALSE;
    GT_STATUS rc = GT_BAD_PARAM;

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"PEX Write: regAddr 0x%8.8x, data 0x%8.8x, mask 0x%8.8x\n", regAddr, *dataPtr, mask);
    REG_PEX_REG_ACCESS_TRACE_DELAY;

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    /* CPSS_HW_DRIVER_AS_CNM_E is used to enable access to the configuration */
    /* space for debug purpose with Internal PCI APIs. */
    if( addrSpace == CPSS_HW_DRIVER_AS_PCI_CONFIG_E ||
        addrSpace == CPSS_HW_DRIVER_AS_CNM_E)
    {
        /* Configuration Space access */
        if (mask != 0xffffffff)
        {
            rc = cpssExtDrvPciConfigReadReg(drv->hwAddr.busNo,
                                           drv->hwAddr.devSel,
                                           drv->hwAddr.funcNo,
                                           regAddr, &data);
            if (rc != GT_OK)
                return rc;

            data &= ~mask;
        }

        data |= ((*dataPtr) & mask);
        return cpssExtDrvPciConfigWriteReg(drv->hwAddr.busNo,
                                           drv->hwAddr.devSel,
                                           drv->hwAddr.funcNo,
                                           regAddr, data);
    }
    else if ( addrSpace == CPSS_HW_DRIVER_AS_ATU_E )
    {
        return(drv->bar0->writeMask(drv->bar0, 0, regAddr, dataPtr, count, mask));
    }

    for (; count; count--,dataPtr++)
    {
        if (addrInWin >= WINDOW_SIZE)
        {
            rc = prvGetWindow(drv, regAddr, &winBase, &addrInWin, &locked);
            if (rc != GT_OK)
                break;
        }

        if (mask != 0xffffffff)
        {
#ifdef ASIC_SIMULATION
            ASIC_SIMULATION_ReadMemory(drv->base, regAddr, 1, &data , winBase + addrInWin);
#else
            data = CPSS_32BIT_LE(*((volatile GT_U32*)(winBase + addrInWin)));
            GT_SYNC; /* to avoid read combining */
            PEX_DELAY_MS;
#endif
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
        if (pex_access_trace_pre_write_callback)
        {
            pex_access_trace_pre_write_callback(regAddr, data);
        }
#ifdef ASIC_SIMULATION
        ASIC_SIMULATION_WriteMemory(drv->base, regAddr, 1, &data , winBase + addrInWin);
#else
        *((volatile GT_U32*)(winBase + addrInWin)) = CPSS_32BIT_LE(data);
#endif
        GT_SYNC; /* to avoid from write combining */
        PEX_DELAY_MS;
        addrInWin += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr-4,"PEX Write: rc %d\n", rc);

    return rc;
}

static GT_STATUS cpssHwDriverEagleAmapWin64DestroyDrv(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

GT_VOID cpssHwDriverEagleAmapWin64ReloadDrv
(
    CPSS_HW_DRIVER_STC *drv
)
{
   drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverEagleAmapWin64Read;
   drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverEagleAmapWin64WriteMask;
   drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverEagleAmapWin64DestroyDrv;
}



/**
* @internal cpssHwDriverEagleAmapWin64CreateDrv function
* @endinternal
*
* @brief Create driver instance for Address Mapping based on
*        available 64 windows
*
* @param[in] base           - resource virtual address
* @param[in] size           - mapped resource size
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size currently expected to be up to 64M (the default
*        4M)
*/
CPSS_HW_DRIVER_STC *cpssHwDriverEagleAmapWin64CreateDrv(
    IN  CPSS_HW_DRIVER_STC *bar0,
    IN  CPSS_HW_INFO_STC *hwInfo
)
{
    CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv;
    char        buf[64];
    GT_U32      ii;
    GT_U32      data;
    GT_STATUS   rc;

    if (!hwInfo->resource.switching.start)
        return NULL;
    if (hwInfo->resource.switching.size < _1M)
        return NULL;

    /* Currently for the naive flow (=NFLOW) we are expecting up to 64M size */
    /* the default is 4M size */
    if (hwInfo->resource.switching.size > _64M)
        return NULL;


    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverEagleAmapWin64Read;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverEagleAmapWin64WriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverEagleAmapWin64DestroyDrv;
    drv->common.type = CPSS_HW_DRIVER_TYPE_EAGLE_E;

    drv->bar0 = bar0;
    drv->base = hwInfo->resource.switching.start;
    drv->size = hwInfo->resource.switching.size;

    drv->hwAddr.busNo  = hwInfo->hwAddr.busNo;
    drv->hwAddr.devSel = hwInfo->hwAddr.devSel;
    drv->hwAddr.funcNo = hwInfo->hwAddr.funcNo;

    drv->lastMatchedIndex = 0;

    for (ii = 0; ii < CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM; ii++)
    {
        /* Currently, for the NFLOW we are using 1M windows offset in bar */
        if ( (ii+1)*WINDOW_SIZE > hwInfo->resource.switching.size)
            break; /* region don't fit to mapped area */

        /* region base address - bar1 base + region*1M */
        data = hwInfo->resource.switching.phys + ii*WINDOW_SIZE;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_LOWER_BASE_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        /* region limit address - bar1 base + region*1M + (1M -1) */
        data += (WINDOW_SIZE - 1);
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_LIMIT_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        data = (hwInfo->resource.switching.phys >> 32);
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_UPPER_BASE_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        data = 0;
        /* explicitly reset the register that may hold value from previous run
           that did 'soft reset' with 'skip reset of PCIe'

           in harrier the DFX base addr of the unit is '0x0' , and need to match
           drv->window[ii].mapAddress
        */
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_LOWER_TARGET_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        data = 0x0;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_UPPER_TARGET_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        /* type of region to be mem */
        data = 0x0;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_REGION_CTRL_1_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        /* enable the region */
        data = 0x80000000;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_REGION_CTRL_2_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        drv->window[ii].reg = iATU_REGISTER_ADDRESS(ii, ATU_LOWER_TARGET_ADDRESS_REG);
        drv->window[ii].winBase = hwInfo->resource.switching.start + ii*WINDOW_SIZE;
        drv->numOfWindows++;
    }

    if (drv->numOfWindows)
    {
        cpssOsSprintf(buf, "eagleAmap%p", (void*)hwInfo->resource.switching.start);
        cpssOsMutexCreate(buf, &(drv->mtx));
    }

#ifdef ATU_DEBUG_STAT
    debugDrv = drv;
    for (ii = 0 ; ii < debugDrv->numOfWindows ; ii++ )
    {
        windowHit[ii] = 0;
        windowReplace[ii] = 0;
    }
#endif

    return (CPSS_HW_DRIVER_STC*)drv;
}

#ifdef ASIC_SIMULATION
static GT_STATUS cpssHwDriverSimulationEaglePexBar0Read(
    IN  CPSS_HW_DRIVER_STC *drv GT_UNUSED,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr GT_UNUSED,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
#ifdef _VISUALC
    (void)drv;
    (void)addrSpace;
    (void)regAddr;
#endif
    for (; count; count--,dataPtr++)
    {
        *dataPtr = 0;
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverSimulationEaglePexBar0WriteMask(
    IN  CPSS_HW_DRIVER_STC *drv GT_UNUSED,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr GT_UNUSED,
    IN  GT_U32 *dataPtr GT_UNUSED,
    IN  GT_U32  count GT_UNUSED,
    IN  GT_U32  mask GT_UNUSED
)
{
#ifdef _VISUALC
    (void)drv;
    (void)addrSpace;
    (void)regAddr;
    (void)dataPtr;
    (void)count;
    (void)mask;
#endif
    return GT_OK;
}

static GT_STATUS cpssHwDriverSimulationEaglePexBar0Destroy(
    IN  CPSS_HW_DRIVER_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

#ifdef SHARED_MEMORY

GT_VOID cpssHwDriverSimulationEagleDrvReload
(
    CPSS_HW_DRIVER_STC *drv
)
{
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverSimulationEaglePexBar0Read;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverSimulationEaglePexBar0WriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverSimulationEaglePexBar0Destroy;
}
#endif

/**
* @internal cpssHwDriverSimulationEaglePexBar0CreateDrv function
* @endinternal
*
* @brief Create driver instance for simulated BAR0 access -
*        actually a NOP driver
*
* @param[in] hwInfo         - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSimulationEaglePexBar0CreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_HW_DRIVER_STC *drv;

    if(GT_TRUE == cpssHwDriverGenWmInPexModeGet())
    {
        /* work with the same driver as used for HW */
        return cpssHwDriverGenMmapCreateDrv(
                hwInfo->resource.cnm.start,
                hwInfo->resource.cnm.size);
    }

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverSimulationEaglePexBar0Read;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverSimulationEaglePexBar0WriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverSimulationEaglePexBar0Destroy;
    drv->type = CPSS_HW_DRIVER_TYPE_SIMULATION_EAGLE_E;

    return drv;
}
#endif

/**(CPSS_HW_DRIVER_STC*)
* @internal cpssHwDriverEaglePexCreateDrv function
* @endinternal
*
* @brief Create driver instance for Eagle PEX - PP Resources are
*        mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverEaglePexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_HW_DRIVER_STC *drv, *bar0;

    if (hwInfo->resource.switching.start == 0)
        return NULL;
#ifdef ASIC_SIMULATION
    bar0 = cpssHwDriverSimulationEaglePexBar0CreateDrv(hwInfo);
#else
    bar0 = cpssHwDriverGenMmapCreateDrv(
                hwInfo->resource.cnm.start,
                hwInfo->resource.cnm.size);
#endif

#if __WORDSIZE == 64
    if (hwInfo->resource.switching.start &&
        hwInfo->resource.switching.size >= 0x80000000L)
    {
CPSS_TBD_BOOKMARK_FALCON_EMULATOR
/* The direct usage of Generic driver in case of 2G window is no longer valid */
/* Since we need access to BAR0 registers for outbound ATU and also to the    */
/* Configuration Space */
        return cpssHwDriverGenMmapCreateDrv(
                            hwInfo->resource.switching.start,
                            hwInfo->resource.switching.size
                            );
    }
#endif

    if (hwInfo->resource.cnm.start)
    {
        drv = cpssHwDriverEagleAmapWin64CreateDrv(bar0, hwInfo);
        return drv;
    }

    return NULL;
}


#ifdef PEX_ACCESS_TRACE
void pexAccessTraceEnable(GT_BOOL enable)
{
    pex_access_trace_enable = enable;
}
void pexAccessTraceDelaySet(GT_U32 delay)
{
    pex_access_trace_delay = delay;
}
/**
* @internal pexAccessTracePreWriteCallbackBind function
* @endinternal
*
* @brief Bind callback for tracing write accesses.
*
* @param[in] pCb - address of callcack function getting register address and written data
*                  called before access to register
*
* @retval        - none
*/
void pexAccessTracePreWriteCallbackBind(void (*pCb)(GT_U32, GT_U32))
{
    pex_access_trace_pre_write_callback = pCb;
}
#endif

#ifdef ATU_DEBUG_STAT
void printAtuStat(void)
{
    GT_U32 ii;

    if( NULL == debugDrv )
    {
        cpssOsPrintf("No driver created.\n");
        return;
    }

    cpssOsPrintf("Num of access: %d\nNum of hits: %d\n", numOfAccess, numOfHits);
    numOfAccess = 0;
    numOfHits = 0;

    for (ii = 0 ; ii < debugDrv->numOfWindows ; ii++ )
    {
        cpssOsPrintf("Window %2d: Hits - %d, Replaces - %d\n", ii, windowHit[ii], windowReplace[ii]);
        windowHit[ii] = 0;
        windowReplace[ii] = 0;
    }
    return;
}

void printAtuMap(void)
{
    GT_U32 ii;

    if( NULL == debugDrv )
    {
        cpssOsPrintf("No driver created.\n");
        return;
    }

    for (ii = 0 ; ii < debugDrv->numOfWindows ; ii++ )
    {
        cpssOsPrintf("Window %2d: map 0x%08x\n", ii, debugDrv->window[ii].mapAddress);
    }
    return;
}
#endif
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
GT_STATUS eagleDrvPerformanceTest(IN GT_U32 regAddr,IN GT_U32  numOfTimes , IN GT_U32 mode)
{
    CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drvPtr;
    GT_U32  devNum=0,portGroupId=0;
    GT_U32  data=0;
    GT_U32  ii;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_UINTPTR      winBase;

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId] == NULL)
    {
        cpssOsPrintf("ERROR : devNum 0 , portGroupId 0 was not created or not have 'driver' \n");

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    drvPtr = (CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC*)(void*)PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId];

    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

    if(mode == 0)
    {
        for(ii = 0 ; ii < numOfTimes ; ii++)
        {
            cpssHwDriverEagleAmapWin64Read(drvPtr,
            CPSS_HW_DRIVER_AS_SWITCHING_E,
                regAddr,&data,1);
        }
    }
    else
    {
        /* do single call to driver , to set the 'winBase' */
        cpssHwDriverEagleAmapWin64Read(drvPtr,
        CPSS_HW_DRIVER_AS_SWITCHING_E,
            regAddr,&data,1);

        winBase = drvPtr->window[drvPtr->lastMatchedIndex].winBase;

        winBase += regAddr & (WINDOW_SIZE-1);

        for(ii = 0 ; ii < numOfTimes ; ii++)
        {
            data = CPSS_32BIT_LE(*((volatile GT_U32*)winBase));
        }
    }

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }

    cpssOsPrintf("Performance of read register [0x%8.8x] for [%d] times in [%d] ms (regValue=[0x%8.8x]) \n",
        regAddr,numOfTimes,(seconds*1000+nanoSec/1000000),data);

    return GT_OK;
}

