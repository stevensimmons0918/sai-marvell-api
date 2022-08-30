/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file noKmDrvPciDrvSip6Mbus.c
*
* @brief MBUS driver for sip6 device (AC5(sip4),AC5X)
*
* @version   1
********************************************************************************
*/
#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#include <cpss/common/cpssHwInit/cpssHwInit.h>

#include "prvNoKmDrv.h"

#include "drivers/mvResources.h"
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>



#define RES_OFFSET(_nm) offsetof(CPSS_HW_INFO_STC,resource._nm)

#ifndef _1G
#define _1G     0x40000000
#endif

#ifndef _2G
#define _2G     0x80000000
#endif

/*
    0x0000 0000 --> 2G
    0x8000 0000

    0x8000 0000 -> 5M
    0x8050 0000

    0x8400 0000 -> 1M
    0x8410 0000

*/
struct mbusResources_STC{
    int cpss_resource_offset;
    int mbus_resource;
    int maxSize;
};

static struct mbusResources_STC mbusResources_ac5[] = {
    { RES_OFFSET(switching),              MV_RESOURCE_MBUS_SWITCH, _2G-4    },
    { RES_OFFSET(cnm),                    MV_RESOURCE_MBUS_RUNIT,  _4M+_1M  },
    { RES_OFFSET(resetAndInitController), MV_RESOURCE_MBUS_DFX,    _1M      },
    { -1, -1, 0}
};

static struct mbusResources_STC mbusResources_ac5x[] = {
    { RES_OFFSET(switching),              MV_RESOURCE_MBUS_SWITCH, _1G      },
    { RES_OFFSET(cnm),                    MV_RESOURCE_MBUS_RUNIT,  _16M     },
    { RES_OFFSET(resetAndInitController), MV_RESOURCE_MBUS_DFX,    _1M      },
    { -1, -1, 0}
};

static int mvMbusDrvFd = -1;

static int mvMbusDrvOpen(void)
{
    if (mvMbusDrvFd < 0)
        mvMbusDrvFd = open("/dev/mvMbusDrv", O_RDWR);
    return mvMbusDrvFd < 0 ? 0 : 1;
}

#define ERROR_PRINT(info) \
    printf("ERROR : in [%s] \n",info)

#define LINUX_MBUS_BASE       0x70000000


static GT_STATUS sip6_mbus_map_resource(
    IN  GT_U32      flags,
    IN  GT_U32      mbusResources_index,
    OUT CPSS_HW_INFO_RESOURCE_MAPPING_STC *mappingPtr
)
{
    void *vaddr = MAP_FAILED;

    struct mbusResources_STC *currentPtr;
#ifdef SHARED_MEMORY
    FILE *f;
    GT_U32 offset =0 ,i;
#endif
    if((flags >> MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS) == MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5_ID_CNS)
    {
        currentPtr = mbusResources_ac5;
    }
    else if((flags >> MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS) == MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5X_ID_CNS)
    {
        currentPtr = mbusResources_ac5x;
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

#ifdef SHARED_MEMORY
     for(i=0;i<mbusResources_index;i++)
     {
        offset+=currentPtr[i].maxSize;
     }
     offset+=LINUX_MBUS_BASE;
#endif

    if(mvMbusDrvFd > 0) {
        vaddr = mmap(
#ifdef SHARED_MEMORY
               (void*)((GT_UINTPTR)(offset)),
#else
                NULL,
#endif
                currentPtr[mbusResources_index].maxSize, /* limited to 2G-4 since type is 'integer' */
                PROT_READ | PROT_WRITE,
               MAP_SHARED,
                mvMbusDrvFd,
                (off_t)(currentPtr[mbusResources_index].mbus_resource << SYSTEM_PAGE_SHIFT));
    }
    if (MAP_FAILED == vaddr)
    {
        ERROR_PRINT("mmap failed");
        return GT_FAIL;
    }

    mappingPtr->start = (GT_UINTPTR)vaddr;
    mappingPtr->size  = (GT_UINTPTR)currentPtr[mbusResources_index].maxSize;
    mappingPtr->phys  = (GT_PHYSICAL_ADDR)0;

#ifdef SHARED_MEMORY

    f = fopen(SHMEM_PP_MAPPINGS_FILENAME, "a");
    if (f == NULL)
        return GT_FAIL;
    osGlobalDbNonSharedDbPpMappingStageDone();
    fprintf(f, "MBUS_SIP6 %p %p %p\n",
            vaddr,
            (void*)(mappingPtr->size),
            (off_t)(currentPtr[mbusResources_index].mbus_resource << SYSTEM_PAGE_SHIFT));
    fclose(f);
#endif
    return GT_OK;
}

/**
* @internal prvExtDrvSip6MbusConfigure function
* @endinternal
*
* @brief   This routine maps all PP resources to userspace and detects IRQ
*          for MBUS sip6 device (AC5(sip4) , AC5X)
*
* @param[out] hwInfoPtr            - pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvSip6MbusConfigure
(
    IN  GT_U32              flags,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_U32 i;
    struct mbusResources_STC *currentPtr;

    if((flags >> MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS) == MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5_ID_CNS)
    {
        currentPtr = mbusResources_ac5;
    }
    else if((flags >> MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS) == MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5X_ID_CNS)
    {
        currentPtr = mbusResources_ac5x;
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    if(0 > mvMbusDrvOpen())
    {
        ERROR_PRINT("mvMbusDrvOpen failed");
        return GT_NOT_FOUND;
    }

    /* TODO: get devVend for quirks */

    memset(hwInfoPtr, 0, sizeof(*hwInfoPtr));
    hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_MBUS_E;
    hwInfoPtr->hwAddr.busNo = 0xffff;/*to match IS_SIP6_MBUS_ADDR(...) */
    hwInfoPtr->hwAddr.devSel = 0xff; /*to match IS_SIP6_MBUS_ADDR(...) */
    hwInfoPtr->hwAddr.funcNo = 0xff; /*to match IS_SIP6_MBUS_ADDR(...) */

    for (i = 0; currentPtr[i].cpss_resource_offset >= 0; i++)
    {
        sip6_mbus_map_resource(flags, i,
                (CPSS_HW_INFO_RESOURCE_MAPPING_STC*)
                    (((GT_UINTPTR)hwInfoPtr)+currentPtr[i].cpss_resource_offset));
    }

#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    if (prvExtDrvMvIntDrvConnected() == GT_FALSE)
    {
        hwInfoPtr->irq.switching = 0x11ab0000 | (noKmMappingsNum & 0xffff);
    }
    else
#endif
    {
        if (prvExtDrvMbusConfigureIrq(hwInfoPtr) == GT_FAIL)
        {
            ERROR_PRINT("Fail to retrieve IRQ # from Mbus, defaulting to polling mode");
            hwInfoPtr->irq.switching = CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS;
        }
    }

    hwInfoPtr->intMask.switching = (GT_UINTPTR)hwInfoPtr->irq.switching;


    noKmMappingsList[noKmMappingsNum] = *hwInfoPtr;
    noKmMappingsNum++;

    return GT_OK;
}

#ifdef SHARED_MEMORY

GT_STATUS sip6_mbus_remap
(
    void *vaddr,
    int    maxSize,
    GT_U32 offset
)
{
    if(0 > mvMbusDrvOpen())
    {
        ERROR_PRINT("mvMbusDrvOpen failed");
        return GT_NOT_FOUND;
    }

    vaddr = mmap(vaddr,
                maxSize, /* limited to 2G-4 since type is 'integer' */
                PROT_READ | PROT_WRITE,
                MAP_SHARED|MAP_FIXED,
                mvMbusDrvFd,
                offset);
    if (MAP_FAILED == vaddr)
    {
        perror("mmap failed");
        return GT_FAIL;
    }

    osGlobalDbNonSharedDbPpMappingStageDone();

    return GT_OK;
}

#endif

