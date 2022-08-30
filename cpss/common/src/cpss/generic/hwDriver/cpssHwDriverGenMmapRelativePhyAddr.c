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
* @file cpssHwDriverGenMmapRelativePhyAddr.c
*
* @brief generic HW driver with use of relative address which use mmap'ed resource
*
* @version   1
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
#include <asicSimulation/SCIB/scib.h>
#include <asicSimulation/wmApi.h>
#endif

typedef struct CPSS_HW_DRIVER_GEN_MMAP_RELATIVE_ADDR_STCT{
    CPSS_HW_DRIVER_STC  common;
    GT_UINTPTR          base;
    GT_UINTPTR          size;

    GT_UINTPTR          relativeOffset;
    CPSS_HW_DRIVER_STC *nextSplitDrvPtr;

} CPSS_HW_DRIVER_GEN_MMAP_RELATIVE_ADDR_STC;

#ifdef SHARED_MEMORY

GT_VOID prvCpssDrvHwDrvReload
(
    IN CPSS_HW_DRIVER_STC          *drv

);
#endif


static GT_STATUS cpssHwDriverGenMmapRead_relativeOffset(
    IN  CPSS_HW_DRIVER_GEN_MMAP_RELATIVE_ADDR_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32      data;
    CPSS_HW_DRIVER_METHOD_READ       readFunc;

#ifdef SHARED_MEMORY
        /*function pointers that are relevant to the process ,
                we can't use from shared  drv since they may be not maped to the processs.*/
        CPSS_HW_DRIVER_STC              localDrv;
#endif

#ifdef _VISUALC
    (void)addrSpace;
#endif

    if(drv->nextSplitDrvPtr &&
        (regAddr - drv->relativeOffset) >= drv->size)
    {
#ifdef SHARED_MEMORY
    localDrv.type =drv->nextSplitDrvPtr->type;
    /*need to reload function pointers due to ASLR*/
    prvCpssDrvHwDrvReload(&localDrv);
    readFunc = localDrv.read;
#else
    readFunc = drv->nextSplitDrvPtr->read;
#endif
        /* let the split range check if it part of it , and handle it */
        CPSS_LOG_INFORMATION_MAC("'read' : check split support for regAddr[0x%x], relativeOffset[0x%x], drv_size[0x%x]", regAddr, drv->relativeOffset, drv->size);
        return readFunc(drv->nextSplitDrvPtr,addrSpace,regAddr,dataPtr,count);
    }

    #ifndef  ASIC_SIMULATION
        regAddr -= drv->relativeOffset;
    #endif

    for (;count;count--,regAddr+=4,dataPtr++)
    {
        if (regAddr+3 >= drv->size)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "regAddr[0x%x] is out of boundary",regAddr);
#ifdef  ASIC_SIMULATION
        ASIC_SIMULATION_ReadMemory(drv->base, regAddr, 1, &data , drv->base+regAddr);
#else
        data = *((volatile GT_U32*)(drv->base+regAddr));
#endif
        GT_SYNC; /* to avoid read combining */
        *dataPtr = CPSS_32BIT_LE(data);
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverGenMmapWriteMask_relativeOffset(
    IN  CPSS_HW_DRIVER_GEN_MMAP_RELATIVE_ADDR_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32      data;
    CPSS_HW_DRIVER_METHOD_WRITE_MASK writeMaskFunc;


#ifdef SHARED_MEMORY
            /*function pointers that are relevant to the process ,
                    we can't use from shared  drv since they may be not maped to the processs.*/
            CPSS_HW_DRIVER_STC              localDrv;
#endif

#ifdef _VISUALC
    (void)addrSpace;
#endif

    if(drv->nextSplitDrvPtr &&
        (regAddr - drv->relativeOffset) >= drv->size)
    {
#ifdef SHARED_MEMORY
    localDrv.type =drv->nextSplitDrvPtr->type;
    /*need to reload function pointers due to ASLR*/
    prvCpssDrvHwDrvReload(&localDrv);
    writeMaskFunc = localDrv.writeMask;
#else
    writeMaskFunc = drv->nextSplitDrvPtr->writeMask;
#endif
        /* let the split range check if it part of it , and handle it */
        CPSS_LOG_INFORMATION_MAC("'writeMask' : check split support for regAddr[0x%x]",regAddr);
        return writeMaskFunc(
            drv->nextSplitDrvPtr,addrSpace,regAddr,dataPtr,count,mask);
    }

    #ifndef  ASIC_SIMULATION
        regAddr -= drv->relativeOffset;
    #endif

    for (;count;count--,regAddr+=4,dataPtr++)
    {
        if (regAddr+3 >= drv->size)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "regAddr[0x%x] is out of boundary",regAddr);
        if (mask != 0xffffffff)
        {
#ifdef  ASIC_SIMULATION
            ASIC_SIMULATION_ReadMemory(drv->base, regAddr, 1, &data , drv->base+regAddr);
            data = CPSS_32BIT_LE(data);
#else
            data = CPSS_32BIT_LE(*((volatile GT_U32*)(drv->base+regAddr)));
#endif
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
#ifdef  ASIC_SIMULATION
        data = CPSS_32BIT_LE(data);
        ASIC_SIMULATION_WriteMemory(drv->base, regAddr, 1, &data , drv->base+regAddr);
#else
        *((volatile GT_U32*)(drv->base+regAddr)) = CPSS_32BIT_LE(data);
#endif
        GT_SYNC; /* to avoid from write combining */
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverGenMmapDestroyDrv_relativeOffset(
    IN  CPSS_HW_DRIVER_GEN_MMAP_RELATIVE_ADDR_STC *drv
)
{
    if(drv->nextSplitDrvPtr)
    {
        drv->nextSplitDrvPtr->destroy(drv->nextSplitDrvPtr);
    }

    cpssOsFree(drv);
    return GT_OK;
}

GT_VOID cpssHwDriverGenMmapRelativeAddrDrvReload
(
    CPSS_HW_DRIVER_STC *drv
)
{
   drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverGenMmapRead_relativeOffset;
   drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverGenMmapWriteMask_relativeOffset;
   drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverGenMmapDestroyDrv_relativeOffset;
}


/**
* @internal cpssHwDriverGenMmapCreateRelativeAddrDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*
* @param[in] base   - resource virtual address
* @param[in] size   - mapped resource size
* @param[in] relativeAddr   - the relative address that should be
*                   reduced from the 'regAddr' that used for read/write
* @param[in] nextSplitResourcePtr - allow to support split range or very large range
*            that it's actual size is more than 'size' parameter.
*
*   NOTE: compatible to cpssHwDriverGenMmapCreateDrv if using relativeAddr = 0
*         and nextSplitResourcePtr = NULL
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapCreateRelativeAddrDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size,
    IN  GT_UINTPTR  relativeAddr,
    IN  struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT*  nextSplitResourcePtr
)
{
    CPSS_HW_DRIVER_GEN_MMAP_RELATIVE_ADDR_STC *drv;

    if (!base || size < 4)
        return NULL;
    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));

    CPSS_LOG_INFORMATION_MAC("base[0x%lx],size[0x%x]relativeAddr[0x%x]\n",
        base,size,relativeAddr);

    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverGenMmapRead_relativeOffset;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverGenMmapWriteMask_relativeOffset;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverGenMmapDestroyDrv_relativeOffset;
    drv->common.type = CPSS_HW_DRIVER_TYPE_GEN_MMAP_RELOF_E;

    drv->base = base;
    drv->size = size;
    drv->relativeOffset = relativeAddr;
    if(nextSplitResourcePtr)
    {
        CPSS_LOG_INFORMATION_MAC("call to 'sun' [%p] : start[0x%lx],size[0x%x]relativeOffset[0x%x]nextSplitResourcePtr[%p]\n",
            nextSplitResourcePtr,
            nextSplitResourcePtr->start,
            nextSplitResourcePtr->size,
            nextSplitResourcePtr->relativeOffset,
            nextSplitResourcePtr->nextSplitResourcePtr);

        drv->nextSplitDrvPtr = cpssHwDriverGenMmapCreateRelativeAddrDrv(
            nextSplitResourcePtr->start,
            nextSplitResourcePtr->size,
            nextSplitResourcePtr->relativeOffset,
            nextSplitResourcePtr->nextSplitResourcePtr);
        if (drv->nextSplitDrvPtr == NULL)
        {
            cpssHwDriverGenMmapDestroyDrv_relativeOffset(drv);
            return NULL;
        }
    }

    return (CPSS_HW_DRIVER_STC*)drv;
}

