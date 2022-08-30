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
* @file newdrv_sip6MbusCntl.c
*
* @brief Driver for sip6 MBUS connected PP (AC5(sip4) / AC5X)
*
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
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PEX_ACCESS_TRACE
#ifdef PEX_ACCESS_TRACE

/* the NO_KM module is polling the interrupt summary register of the device */
#define GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS   0x30

/* as the polling is done 5 or 50 times a second , we not want the 'trace' to be with this register access indication */
#define AC5X_POLLING_TASK_REG_ADD   (PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS+ GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS)
#define AC5_POLLING_TASK_REG_ADD    (0                                    + GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS)

#define IS_MG0_INTERRUPT_CAUSE_REG_MAC(regAddr)         \
    /* first line check is for 'fast check' if register address is 'potential' for the global cause register */ \
    ((((regAddr) & 0x000FFFFF/*1M*/) != GLOBAL_INTERRUPT_CAUSE_REG_ADDR_IN_MG_CNS) ? 0 : \
    /* next lines check if it belongs to MG of known device */ \
     (((regAddr) == AC5_POLLING_TASK_REG_ADD         ||  \
       (regAddr) == AC5X_POLLING_TASK_REG_ADD) ? 1 : 0))

#define NEED_TO_IGNORE_REGISTER_TRACE_MAC(regAddr)  \
    /* as the polling is done 5 or 50 times a second , we not want the 'trace' to be with this register access indication */ \
    IS_MG0_INTERRUPT_CAUSE_REG_MAC(regAddr)

extern GT_BOOL pex_access_trace_enable;
extern GT_U32 pex_access_trace_delay;


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
#define REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,...)
#define REG_PEX_REG_ACCESS_TRACE_DELAY
#endif

/*#define PEX_DELAY*/
#ifdef PEX_DELAY
#define PEX_DELAY_MS cpssOsTimerWkAfter(5)
#else
#define PEX_DELAY_MS
#endif

/* need to support CPSS_HW_DRIVER_AS_ATU_E */
#define MAX_ADDR_SPACE_SUPPORTED    (1+CPSS_HW_DRIVER_AS_ATU_E)

typedef struct ADDR_SPACE_RANGE_STCT{
    CPSS_HW_DRIVER_STC          *addrSpace_common;
    struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT  splitInfo;

}ADDR_SPACE_RANGE_STC;

typedef struct {
    CPSS_HW_DRIVER_STC      common;
    ADDR_SPACE_RANGE_STC    addressSpaceArr[MAX_ADDR_SPACE_SUPPORTED];
} CPSS_HW_DRIVER_SIP6_MBUS_STC;

#ifdef SHARED_MEMORY

GT_VOID prvCpssDrvHwDrvReload
(
    IN CPSS_HW_DRIVER_STC          *drv

);
#endif

static GT_STATUS cpssHwDriverSip6MbusRead(
    IN  CPSS_HW_DRIVER_SIP6_MBUS_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_STATUS rc;

    CPSS_HW_DRIVER_METHOD_READ       readFunc;
#ifdef SHARED_MEMORY
    /*function pointers that are relevant to the process ,
            we can't use from shared  drv since they may be not maped to the processs.*/
    CPSS_HW_DRIVER_STC              localDrv;
#endif

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"SIP6 MBUS Read: regAddr 0x%8.8x addrSpace [%d]\n", regAddr,addrSpace);
    REG_PEX_REG_ACCESS_TRACE_DELAY;

    if (addrSpace >= MAX_ADDR_SPACE_SUPPORTED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space >= than 18 [%d] \n", addrSpace);
    if (drv->addressSpaceArr[addrSpace].addrSpace_common == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space [%d] is not initialized \n", addrSpace);

#ifdef SHARED_MEMORY
    localDrv.type =drv->addressSpaceArr[addrSpace].addrSpace_common->type;
    /*need to reload function pointers due to ASLR*/
    prvCpssDrvHwDrvReload(&localDrv);
    readFunc = localDrv.read;
#else
    readFunc = drv->addressSpaceArr[addrSpace].addrSpace_common->read;
#endif

    rc = readFunc(drv->addressSpaceArr[addrSpace].addrSpace_common, 0, regAddr, dataPtr, count);

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"SIP6 MBUS Read: rc %d, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr, *dataPtr);

    return rc;
}

static GT_STATUS cpssHwDriverSip6MbusWriteMask(
    IN  CPSS_HW_DRIVER_SIP6_MBUS_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_METHOD_WRITE_MASK writeMaskFunc;
#ifdef SHARED_MEMORY
    /*function pointers that are relevant to the process ,
            we can't use from shared  drv since they may be not maped to the processs.*/
    CPSS_HW_DRIVER_STC              localDrv;
#endif

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"SIP6 MBUS Write: regAddr 0x%8.8x, data 0x%8.8x, mask 0x%8.8x addrSpace [%d]\n", regAddr, *dataPtr, mask,addrSpace);
    REG_PEX_REG_ACCESS_TRACE_DELAY;

    if (addrSpace >= MAX_ADDR_SPACE_SUPPORTED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space >= than 18 [%d] \n", addrSpace);
    if (drv->addressSpaceArr[addrSpace].addrSpace_common == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Address space [%d] is not initialized \n", addrSpace);
#ifdef SHARED_MEMORY
    localDrv.type =drv->addressSpaceArr[addrSpace].addrSpace_common->type;
    /*need to reload function pointers due to ASLR*/
    prvCpssDrvHwDrvReload(&localDrv);
    writeMaskFunc = localDrv.writeMask;
#else
    writeMaskFunc = drv->addressSpaceArr[addrSpace].addrSpace_common->writeMask;
#endif

    rc = writeMaskFunc(drv->addressSpaceArr[addrSpace].addrSpace_common, 0, regAddr, dataPtr, count, mask);

    REG_PEX_REG_ACCESS_TRACE_PRINTF(regAddr,"SIP6 MBUS Write: rc %d\n", rc);

    return rc;
}

static GT_STATUS cpssHwDriverSip6MbusDestroy(
    IN  CPSS_HW_DRIVER_SIP6_MBUS_STC *drv
)
{
    GT_U32 i;
    for (i = 0; i < MAX_ADDR_SPACE_SUPPORTED; i++)
    {
        if (drv->addressSpaceArr[i].addrSpace_common)
            cpssHwDriverDestroy(drv->addressSpaceArr[i].addrSpace_common);
    }
    cpssOsFree(drv);
    return GT_OK;
}

extern CPSS_PP_FAMILY_TYPE_ENT prvCpssDrvHwPpPrePhase1NextDevFamilyGet(GT_VOID);

GT_VOID cpssHwDriverSip6MbusDrvReload
(
    CPSS_HW_DRIVER_STC *drv
)
{
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverSip6MbusRead;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverSip6MbusWriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverSip6MbusDestroy;
}

/**
* @internal cpssHwDriverSip6MbusCreateDrv function
* @endinternal
*
* @brief Create driver instance for sip6 MBUS connected PP (AC5(sip4) / AC5X)
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSip6MbusCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_HW_DRIVER_SIP6_MBUS_STC *drv;
    struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT *currInfoPtr;
    struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT *splitInfoPtr;
    GT_U32 dfxBaseAddr;
    GT_U32 cnmBaseAddr;
    GT_U32 switchBaseAddr;

    devFamily = prvCpssDrvHwPpPrePhase1NextDevFamilyGet();
    switch(devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E :
            dfxBaseAddr = 0x84000000;
            cnmBaseAddr = 0x80000000;
            switchBaseAddr = 0x00000000;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            dfxBaseAddr = 0x94400000;
            cnmBaseAddr = 0x7F000000;
            switchBaseAddr = 0x80000000;
            break;
        default:
            CPSS_LOG_ERROR_MAC("unknown 'pre phase1' device family [%d]",devFamily);
            return NULL;
    }

    if(0 == hwInfo->resource.switching.start ||
       0 == hwInfo->resource.cnm.start ||
       0 == hwInfo->resource.resetAndInitController.start )
    {
        CPSS_LOG_ERROR_MAC("is switching.start == 0 [%d]",hwInfo->resource.switching.start==0);
        CPSS_LOG_ERROR_MAC("is cnm.start == 0 [%d]",hwInfo->resource.cnm.start==0);
        CPSS_LOG_ERROR_MAC("is resetAndInitController.start == 0 [%d]",hwInfo->resource.resetAndInitController.start==0);
        return NULL;
    }


    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverSip6MbusRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverSip6MbusWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverSip6MbusDestroy;
    drv->common.type = CPSS_HW_DRIVER_TYPE_MBUS_E;

    currInfoPtr = & drv->addressSpaceArr[CPSS_HW_DRIVER_AS_DFX_E].splitInfo;
    currInfoPtr->start                  = hwInfo->resource.resetAndInitController.start;
    currInfoPtr->size                   = hwInfo->resource.resetAndInitController.size;
    currInfoPtr->relativeOffset         = dfxBaseAddr;
    currInfoPtr->nextSplitResourcePtr   = NULL; /* point to NULL */

    drv->addressSpaceArr[CPSS_HW_DRIVER_AS_DFX_E].addrSpace_common =
        cpssHwDriverGenMmapCreateRelativeAddrDrv(
            currInfoPtr->start,
            currInfoPtr->size,
            currInfoPtr->relativeOffset,
            currInfoPtr->nextSplitResourcePtr);

    splitInfoPtr = & drv->addressSpaceArr[CPSS_HW_DRIVER_AS_DFX_E].splitInfo;

    currInfoPtr = & drv->addressSpaceArr[CPSS_HW_DRIVER_AS_CNM_E].splitInfo;
    currInfoPtr->start                  = hwInfo->resource.cnm.start;
    currInfoPtr->size                   = hwInfo->resource.cnm.size;
    currInfoPtr->relativeOffset         = cnmBaseAddr;
    currInfoPtr->nextSplitResourcePtr   = splitInfoPtr; /* point to DFX */

    drv->addressSpaceArr[CPSS_HW_DRIVER_AS_CNM_E].addrSpace_common =
        cpssHwDriverGenMmapCreateRelativeAddrDrv(
            currInfoPtr->start,
            currInfoPtr->size,
            currInfoPtr->relativeOffset,
            currInfoPtr->nextSplitResourcePtr);

    splitInfoPtr = & drv->addressSpaceArr[CPSS_HW_DRIVER_AS_CNM_E].splitInfo;

    currInfoPtr = & drv->addressSpaceArr[CPSS_HW_DRIVER_AS_SWITCHING_E].splitInfo;
    currInfoPtr->start                  = hwInfo->resource.switching.start;
    currInfoPtr->size                   = hwInfo->resource.switching.size;
    currInfoPtr->relativeOffset         = switchBaseAddr;
    currInfoPtr->nextSplitResourcePtr   = splitInfoPtr; /* point to CNM */

    drv->addressSpaceArr[CPSS_HW_DRIVER_AS_SWITCHING_E].addrSpace_common =
        cpssHwDriverGenMmapCreateRelativeAddrDrv(
            currInfoPtr->start,
            currInfoPtr->size,
            currInfoPtr->relativeOffset,
            currInfoPtr->nextSplitResourcePtr);


    /* the ATU access is '0x1200' base to start of 'PCIe MAC' */
    #define ATU_REGISTERS_OFFSET_IN_BAR0            0x1200
    #define ATU_REGISTERS_OFFSET_WORK_INTERNAL_CPU  0xc000      /* probably 'per device' ... TBD generic support */
    #define ATU_UNIT_RELATIVE_TO_CNM                0x000A0000  /* probably 'per device' ... TBD generic support */

    currInfoPtr = & drv->addressSpaceArr[CPSS_HW_DRIVER_AS_ATU_E].splitInfo;
    currInfoPtr->start                  = hwInfo->resource.cnm.start+ ATU_UNIT_RELATIVE_TO_CNM + ATU_REGISTERS_OFFSET_WORK_INTERNAL_CPU;
    currInfoPtr->size                   = 256;/* 256 is enough = 0x100 */
    currInfoPtr->relativeOffset         = ATU_REGISTERS_OFFSET_IN_BAR0;/*remove from address*/
    currInfoPtr->nextSplitResourcePtr   = NULL; /* point to NULL */

    drv->addressSpaceArr[CPSS_HW_DRIVER_AS_ATU_E].addrSpace_common =
        cpssHwDriverGenMmapCreateRelativeAddrDrv(
            currInfoPtr->start,
            currInfoPtr->size,
            currInfoPtr->relativeOffset,
            currInfoPtr->nextSplitResourcePtr);

    return (CPSS_HW_DRIVER_STC*)drv;
}

