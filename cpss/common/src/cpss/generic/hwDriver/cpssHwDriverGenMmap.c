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
* @file cpssHwDriverGenMmap.c
*
* @brief generic HW driver which use mmap'ed resource
*
* @version   1
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>
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

typedef struct CPSS_HW_DRIVER_GEN_MMAP_STCT {
    CPSS_HW_DRIVER_STC  common;
    GT_UINTPTR          base;
    GT_UINTPTR          size;
} CPSS_HW_DRIVER_GEN_MMAP_STC;

#ifdef ASIC_SIMULATION
/* indication that globally all devices work with memory access via BAR0,BAR2 */
extern GT_BOOL sinit_global_usePexLogic;

/* check if WM supports working in PEX mode (using BAR0,BAR2) */
GT_BOOL cpssHwDriverGenWmInPexModeGet(void)
{
    return sinit_global_usePexLogic;
}

/* set WM to supports working in PEX mode (using BAR0,BAR2) */
void cpssHwDriverGenWmInPexModeSet(IN GT_BOOL isPexMode)
{
    sinit_global_usePexLogic = isPexMode;
}

/**
* @internal ASIC_SIMULATION_ReadMemory function
* @endinternal
*
* @brief   Read memory from ASIC_SIMULATION device.
*
* @param[in] drvBaseAddr              - the base that the driver hold for the WM device.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] pexFinalAddr             - address on the PEX to access (used in 'pex mode')
*
* @param[out] dataPtr                  - pointer to copy read data.
*/
void ASIC_SIMULATION_ReadMemory
(
    IN  GT_U32        drvBaseAddr,  /* used by default mode */
    IN  GT_U32        memAddr,      /* used by default mode */
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr,

    IN GT_UINTPTR     pexFinalAddr
)
{
    if(sinit_global_usePexLogic == GT_FALSE)
    {
        GT_U32 deviceId = scibGetDeviceId(drvBaseAddr);
        scibReadMemory(deviceId,memAddr,length,dataPtr);
        return;
    }

#if __WORDSIZE == 64
    (void)wmMemPciRead(pexFinalAddr>>32,(GT_U32)pexFinalAddr,length,dataPtr);
#else
    (void)wmMemPciRead(0/*addr high*/,pexFinalAddr,length,dataPtr);
#endif

    return;
}

/**
* @internal ASIC_SIMULATION_WriteMemory function
* @endinternal
*
* @brief   Write to memory of a ASIC_SIMULATION device.
*
* @param[in] drvBaseAddr              - the base that the driver hold for the WM device.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] pexFinalAddr             - address on the PEX to access (used in 'pex mode')
* @param[in] dataPtr                  - pointer to copy read data.
*
*
*/
void ASIC_SIMULATION_WriteMemory
(
    IN  GT_U32        drvBaseAddr,  /* used by default mode */
    IN  GT_U32        memAddr,      /* used by default mode */
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr,

    IN GT_UINTPTR     pexFinalAddr
)
{
    if(sinit_global_usePexLogic == GT_FALSE)
    {
        GT_U32 deviceId = scibGetDeviceId(drvBaseAddr);
        scibWriteMemory(deviceId,memAddr,length,dataPtr);
        return;
    }
#if __WORDSIZE == 64
    (void)wmMemPciWrite(pexFinalAddr>>32,(GT_U32)pexFinalAddr,length,dataPtr);
#else
    (void)wmMemPciWrite(0/*addr high*/,pexFinalAddr,length,dataPtr);
#endif

    return;
}

#endif/*ASIC_SIMULATION*/

static GT_STATUS cpssHwDriverGenMmapRead(
    IN  CPSS_HW_DRIVER_GEN_MMAP_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32      data;

#ifdef _VISUALC
    (void)addrSpace;
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

static GT_STATUS cpssHwDriverGenMmapWriteMask(
    IN  CPSS_HW_DRIVER_GEN_MMAP_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32      data;

#ifdef _VISUALC
    (void)addrSpace;
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

static GT_STATUS cpssHwDriverGenMmapDestroyDrv(
    IN  CPSS_HW_DRIVER_GEN_MMAP_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

GT_VOID cpssHwDriverGenMmapReloadDrv
(
    CPSS_HW_DRIVER_STC *drv
)
{
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverGenMmapRead;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverGenMmapWriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverGenMmapDestroyDrv;
}


/**
* @internal cpssHwDriverGenMmapCreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*
* @param[in] base   - resource virtual address
* @param[in] size   - mapped resource size
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapCreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size
)
{
    CPSS_HW_DRIVER_GEN_MMAP_STC *drv;

    if (!base || size < 4)
        return NULL;
    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverGenMmapRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverGenMmapWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverGenMmapDestroyDrv;
    drv->common.type = CPSS_HW_DRIVER_TYPE_GEN_MMAP_E;

    drv->base = base;
    drv->size = size;
    return (CPSS_HW_DRIVER_STC*)drv;
}

