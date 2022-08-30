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
* @file newdrv_pexMbusCntl.c
*
* @brief Driver for PEX/PEXMBUS connected PP
*       Supports both legacy and 8-region address completion
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
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define PEX_ACCESS_TRACE
#ifdef PEX_ACCESS_TRACE

/* the NO_KM module is polling the interrupt summary register of the device */
#define GENERIC_POLLING_TASK_REG_ADD   (0x30)

extern GT_BOOL pex_access_trace_enable;
extern GT_U32 pex_access_trace_delay;


#define REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,...) \
    if( GT_TRUE == pex_access_trace_enable) \
    {                                       \
        if((regAddr) != GENERIC_POLLING_TASK_REG_ADD)\
        {                                   \
            cpssOsPrintf(__VA_ARGS__);      \
        }                                   \
    }
#define REG_PEX_REG_ACCESS_TRACE_DELAY \
    if( 0 != pex_access_trace_delay)   \
    {                                                \
        if((regAddr) != GENERIC_POLLING_TASK_REG_ADD)\
        {                                            \
            cpssOsTimerWkAfter(pex_access_trace_delay);  \
        }                                            \
    }
#else
#define REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,...)
#define REG_PEX_REG_ACCESS_TRACE_DELAY
#endif

#ifdef PEX_ACCESS_TRACE
void pexAccessTraceEnable(GT_BOOL enable);
void pexAccessTraceDelaySet(GT_U32 delay);
#endif

/*#define PEX_DELAY*/
#ifdef PEX_DELAY
#define PEX_DELAY_MS cpssOsTimerWkAfter(5)
#else
#define PEX_DELAY_MS
#endif

typedef struct CPSS_HW_DRIVER_PEX_ALL_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_HW_DRIVER_STC* as[16];
} CPSS_HW_DRIVER_PEX_ALL_STC;

static GT_STATUS cpssHwDriverPexAllRead(
    IN  CPSS_HW_DRIVER_PEX_ALL_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_STATUS rc;

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"PEX Read: regAddr 0x%8.8x\n", regAddr);
    REG_PEX_REG_ACCESS_TRACE_DELAY;

    if (addrSpace >= 16)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space greater than 16 [%d] \n", addrSpace);
    if (drv->as[addrSpace] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space [%d] is not initialized \n", addrSpace);

    rc = drv->as[addrSpace]->read(drv->as[addrSpace], 0, regAddr, dataPtr, count);

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr-4,"PEX Read: rc %d, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr-4, *(dataPtr-1));

    return rc;
}

static GT_STATUS cpssHwDriverPexAllWriteMask(
    IN  CPSS_HW_DRIVER_PEX_ALL_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_STATUS rc;

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"PEX Write: regAddr 0x%8.8x, data 0x%8.8x, mask 0x%8.8x\n", regAddr, *dataPtr, mask);
    REG_PEX_REG_ACCESS_TRACE_DELAY;

    if (addrSpace >= 16)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space greater than 16 [%d] \n", addrSpace);
    if (drv->as[addrSpace] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space [%d] is not initialized \n", addrSpace);

    rc = drv->as[addrSpace]->writeMask(drv->as[addrSpace], 0, regAddr, dataPtr, count, mask);

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr-4,"PEX Write: rc %d\n", rc);

    return rc;
}

static GT_STATUS cpssHwDriverPexAllDestroy(
    IN  CPSS_HW_DRIVER_PEX_ALL_STC *drv
)
{
    GT_U32 i;
    for (i = 0; i < 16; i++)
    {
        if (drv->as[i] != 0)
            cpssHwDriverDestroy(drv->as[i]);
    }
    cpssOsFree(drv);
    return GT_OK;
}

GT_VOID cpssHwDriverPexDrvReload
(
    CPSS_HW_DRIVER_STC *drv
)
{
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverPexAllRead;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverPexAllWriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverPexAllDestroy;
}

/**
* @internal cpssHwDriverPexCreateDrv function
* @endinternal
*
* @brief Create driver instance for PEX connected PP
*        (legacy 4-region address completion)
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverPexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_HW_DRIVER_PEX_ALL_STC *drv;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverPexAllRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverPexAllWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverPexAllDestroy;
    drv->common.type = CPSS_HW_DRIVER_TYPE_PEX_E;

    if (hwInfo->resource.cnm.start)
        drv->as[CPSS_HW_DRIVER_AS_CNM_E] = cpssHwDriverGenMmapCreateDrv(
            hwInfo->resource.cnm.start,
            hwInfo->resource.cnm.size);

    if (hwInfo->resource.switching.start)
    {
        GT_U32 data = 0x00010000;
        CPSS_HW_DRIVER_STC *d = cpssHwDriverGenMmapAc4CreateDrv(
                        hwInfo->resource.switching.start);
        if (d == NULL)
            return NULL;
        drv->as[CPSS_HW_DRIVER_AS_SWITCHING_E] = d;
        d->writeMask(d, 0, 0x140, &data, 1, 0x00010000);
    }

    if (hwInfo->resource.resetAndInitController.start)
        drv->as[CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E] =
                cpssHwDriverGenMmapCreateDrv(
                        hwInfo->resource.resetAndInitController.start,
                        hwInfo->resource.resetAndInitController.size);

    return (CPSS_HW_DRIVER_STC*)drv;
}


/**
* @internal cpssHwDriverPexMbusCreateDrv function
* @endinternal
*
* @brief Create driver instance for eArch PP
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
* @param[in] compRegionMask     - the bitmap of address completion regions to use
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverPexMbusCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  GT_U32      compRegionMask
)
{
    CPSS_HW_DRIVER_PEX_ALL_STC *drv;

    if ((compRegionMask & 0xfe) == 0)
        return NULL;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverPexAllRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverPexAllWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverPexAllDestroy;
    drv->common.type = CPSS_HW_DRIVER_TYPE_PEX_E;

    if (hwInfo->resource.cnm.start)
        drv->as[CPSS_HW_DRIVER_AS_CNM_E] = cpssHwDriverGenMmapCreateDrv(
            hwInfo->resource.cnm.start,
            hwInfo->resource.cnm.size);

    if (hwInfo->resource.switching.start)
    {
        GT_U32 data = 0;
        CPSS_HW_DRIVER_STC *d;
#if __WORDSIZE == 64
        if (hwInfo->resource.switching.size >= 0x100000000L)
        {
            d = cpssHwDriverGenMmapCreateDrv(
                        hwInfo->resource.switching.start,
                        0x100000000L);
            if (d == NULL)
            {
                cpssOsFree(drv);
                return NULL;
            }

            /* Address Completion Control:
             * set bit 16 to 0, bit 4 to 1 */
            data = 0x00000010;
            d->writeMask(d, 0, 0x140, &data, 1, 0x00010010);
        }
        else
#endif
        {
            d = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.switching.start,
                        hwInfo->resource.switching.size,
                        compRegionMask);
            if (d == NULL)
            {
                cpssOsFree(drv);
                return NULL;
            }
            d->writeMask(d, 0, 0x140, &data, 1, 0x00010000);
        }
        drv->as[CPSS_HW_DRIVER_AS_SWITCHING_E] = d;
    }

    if (hwInfo->resource.resetAndInitController.start)
        drv->as[CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E] =
                cpssHwDriverGenMmapCreateDrv(
                        hwInfo->resource.resetAndInitController.start,
                        hwInfo->resource.resetAndInitController.size);

    if (hwInfo->resource.mg1.start)
        drv->as[CPSS_HW_DRIVER_AS_MG1_E] = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.mg1.start,
                        hwInfo->resource.mg1.size,
                        0x3);
    if (hwInfo->resource.mg2.start)
        drv->as[CPSS_HW_DRIVER_AS_MG2_E] = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.mg2.start,
                        hwInfo->resource.mg2.size,
                        0x1);
    if (hwInfo->resource.mg3.start)
        drv->as[CPSS_HW_DRIVER_AS_MG3_E] = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.mg3.start,
                        hwInfo->resource.mg3.size,
                        0x1);


    return (CPSS_HW_DRIVER_STC*)drv;
}



