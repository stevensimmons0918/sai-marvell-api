/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include "../linuxNoKernelModule/prvNoKmDrv.h"
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>

#include <asicSimulation/SCIB/scib.h>

#if defined(_LINUX) && defined(SHARED_MEMORY)
#   include <gtOs/gtOsSharedPp.h>
#endif
#include <string.h>

/*******************************************************************************
* internal definitions
*******************************************************************************/
#ifdef PRESTERA_DEBUG
#define PCI_DRV_DEBUG
#endif

#ifdef PCI_DRV_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif


GT_U32 internalBaseArray[256];


/**
* @internal extDrvPciConfigWriteReg function
* @endinternal
*
* @brief   This routine write register to the PCI configuration space.
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regAddr                  - Register offset in the configuration space.
* @param[in] data                     -  to write.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciConfigWriteReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    scibPciRegWrite(devSel,regAddr,1, &data);
    return GT_OK;
}



/**
* @internal extDrvPciConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regAddr                  - Register offset in the configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciConfigReadReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    scibPciRegRead(devSel,regAddr,1, data);

    return GT_OK;
}


/**
* @internal extDrvPciMap function
* @endinternal
*
* @brief   This routine maps PP registers and PCI registers into userspace
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regsSize                 - registers size (64M for PCI)
*
* @param[out] mapPtr                   - The pointer to all mappings
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciMap
(
    IN  GT_U32                  busNo,
    IN  GT_U32                  devSel,
    IN  GT_U32                  funcNo,
    IN  GT_UINTPTR              regsSize,
    OUT GT_EXT_DRV_PCI_MAP_STC  *mapPtr
)
{
    GT_U32 baseAddr;
    GT_U32 internalBase;
    GT_STATUS ret;

    if (mapPtr == NULL)
        return GT_BAD_PARAM;
    memset(mapPtr,0,sizeof(*mapPtr));

    if(cpssHwDriverGenWmInPexModeGet()) /*from extDrvPciMap(...) in noKmDrvPciDrv.c */
    {
        GT_STATUS rc;
        CPSS_HW_INFO_STC hwInfo;

        rc = extDrvPexConfigure(busNo, devSel, funcNo,
            (regsSize == _4M) ? MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E : 0,
            &hwInfo);
        if (rc != GT_OK)
            return rc;

        mapPtr->config.base = hwInfo.resource.cnm.start;
        mapPtr->config.size = hwInfo.resource.cnm.size;
        mapPtr->regs.base   = hwInfo.resource.switching.start;
        mapPtr->regs.size   = hwInfo.resource.switching.size;
        mapPtr->dfx.base    = hwInfo.resource.resetAndInitController.start;
        mapPtr->dfx.size    = hwInfo.resource.resetAndInitController.size;
        mapPtr->sram.base   = hwInfo.resource.sram.start;
        mapPtr->sram.size   = hwInfo.resource.sram.size;
        return GT_OK;
    }

    /* get device BAR 2 */
    ret = extDrvPciConfigReadReg(busNo,devSel,funcNo,0x18,&baseAddr);
    if(ret != GT_OK)
    {
        return GT_FAIL;
    }

    baseAddr &= 0xFFFFFFF0;/* lower 4 bits are flags */

    /* Get internal base address    */
    internalBase = (GT_UINTPTR)&internalBaseArray[devSel];

    mapPtr->regs.size = _8M;
    osPhy2Virt((GT_UINTPTR)baseAddr, &(mapPtr->regs.base));
    mapPtr->config.size = _1M;
    osPhy2Virt((GT_UINTPTR)internalBase, &(mapPtr->config.base));

#if defined(_LINUX) && defined(SHARED_MEMORY)
    /* We should note mapped section to restore mapping for all non-first clients */
    ret = shrMemRegisterPpMmapSection
    (
        (GT_VOID *)(mapPtr->regs.base),
        mapPtr->regs.size,
        0
    );
    if (GT_OK != ret) return ret;
    /* We should note mapped section to restore mapping for all non-first clients */
    ret = shrMemRegisterPpMmapSection
    (
        (GT_VOID *)(mapPtr->config.base),
        mapPtr->config.size,
        0
    );
    if (GT_OK != ret) return ret;
#endif

    return GT_OK;
}
/**
* @internal prvExtDrvPciSysfsConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigReadReg
(
    DBDF_DECL,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    pciDomain = pciDomain;
    return extDrvPciConfigReadReg(pciBus, pciDev, pciFunc ,regAddr,data);
}


#if __WORDSIZE == 64
    #define WM_BASE_HIGH_BAR0 0x12345678
    #define WM_BASE_HIGH_BAR2 0x87654321
#else
    #define WM_BASE_HIGH_BAR0 0x0
    #define WM_BASE_HIGH_BAR2 0x0
#endif

typedef struct{
    char*   resname;
    GT_U32  baseHigh;
    GT_U32  baseLow;
    GT_U32  addrInPciConfigSpace;
}RESOURCE_INFO;
static RESOURCE_INFO resourceInfoArr[] = {
     {"resource0", WM_BASE_HIGH_BAR0,0xf6800000,0x10}
    ,{"resource2", WM_BASE_HIGH_BAR2,0xf6000000,0x18}
    /*must be last*/
    ,{NULL,0,0}
};
/* in Linux HW , the Linux already query the BAR0 and BAR2 about their
   sizes and assigned them with 'base' : BAR
   but in WM ... the OS not do it for us ... so we do it now.
*/
static GT_STATUS wm_emulate_linux_set_pex_bar_addresses(
    DBDF_DECL
)
{
    GT_U32  ii,regValue;
    GT_U32  perDevOffset;
    /* in Linux HW , the Linux already query the BAR0 and BAR2 about their
       sizes and assigned them with 'base' : BAR
       but in WM ... the OS not do it for us ... so we do it now.
    */
    for(ii = 0; resourceInfoArr[ii].resname != NULL; ii++)
    {
        perDevOffset = 0x01000000 * pciDev;
        /* set low 32 bits address of the BAR */
        regValue = resourceInfoArr[ii].baseLow + perDevOffset;
        extDrvPciConfigWriteReg(pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace ,  regValue);
        /* set high 32 bits address of the BAR */
        regValue = resourceInfoArr[ii].baseHigh;
        extDrvPciConfigWriteReg(pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace+4 ,  regValue);
    }
    return GT_OK;
}
/***************************************************/
/*   read PEX addresses of PCI BAR0, BAR1, BAR2    */
/***************************************************/
static GT_STATUS sysfs_read_pex_bar_addresses(
    DBDF_DECL,
    OUT unsigned long long *res
)
{
    int i;
    GT_U32 word;

    for (i = 0; i < 3; i++)
    {
        /* read PEX BARx internal register */
        prvExtDrvPciSysfsConfigReadReg(DBDF, 0x10 + i*8, &word);
        if (word == 0 || word & 1)
            continue;
        res[i] = (unsigned long long)(word & 0xffff0000);
        if ((word & 0x0000006) == 4) /* type == 2 == 64bit address */
        {
            /* read PEX BARx internal (high) register */
            prvExtDrvPciSysfsConfigReadReg(DBDF, 0x10 + i*8 + 4, &word);
            res[i] |= ((unsigned long long)word) << 32;
        }
    }
    return GT_OK;
}
struct prestera_config {
    const char *name;
    unsigned    devId;
    unsigned    devIdMask;
} prestera_sysmap[] = {
    { "falcon",    0x8400, 0xfc00},
    { "hawk",      0x9400, 0xfc00},
    { "phoenix",   0x9800, 0xff00},
    { "ac5",       0xB400, 0xfc00},/*same as phoenix*/
    { "Aldrin3M",  0x9000, 0xFF00},
    { "harrier",   0x2100, 0xFF00},
    { "ironman",   0xa000, 0xFA00},/*bits 11..15 = 0x14 , bit 9 = 0 */


    /* must be last */
    { NULL,0,0}
};

typedef struct{
    char*   name;
    GT_U32  offset;
    GT_U32  startBit;
    GT_U32  numBits;
}FIELD_INFO;
static FIELD_INFO pciConfigSpaceFieldsArr[] = {
     {"vendor", 0x0, 0,16}
    ,{"device", 0x0,16,16}
    ,{"class" , 0x8, 8,24}
    ,{"irq"   ,0x3c, 0, 8}
    ,{"enable",0x38, 0, 1} /* don't know where is it should be and 0x38 is 'reserved'*/
    /*must be last*/
    ,{NULL,0,0,0}
};
static GT_STATUS sysfs_pci_readNum(
    DBDF_DECL,
    IN  const char *name,
    OUT unsigned *val
)
{
    GT_U32  ii;
    GT_U32  tmpValue;

    for(ii = 0 ; pciConfigSpaceFieldsArr[ii].name ; ii++)
    {
        if(osStrCmp(name,pciConfigSpaceFieldsArr[ii].name))
        {
            continue;
        }

        /*found it*/
        extDrvPciConfigReadReg(pciBus, pciDev, pciFunc ,pciConfigSpaceFieldsArr[ii].offset,&tmpValue);

        *val = U32_GET_FIELD_MAC(tmpValue,pciConfigSpaceFieldsArr[ii].startBit,pciConfigSpaceFieldsArr[ii].numBits);
        return GT_OK;
    }

    return GT_NOT_IMPLEMENTED;

}

static int sysfs_pci_detect_dev(
    DBDF_DECL,
    int print_found
)
{
    unsigned pciVendorId, pciDeviceId, pciClass;
    unsigned ii;

    if (sysfs_pci_readNum(DBDF, "vendor", &pciVendorId) != GT_OK)
        return -1;
    if (pciVendorId != 0x11ab)
        return -1;
    if (sysfs_pci_readNum(DBDF, "class", &pciClass) != GT_OK)
        return -1;
    if (pciClass != 0x020000 && pciClass != 0x058000)
        return -1;
    if (sysfs_pci_readNum(DBDF, "device", &pciDeviceId) != GT_OK)
        return -1;
#ifdef NOKMDRV_INFO
    if (print_found)
    {
        NOKMDRV_IPRINTF(("Found marvell device %04x:%04x @%04x:%02x:%02x.%d\n",
                pciVendorId, pciDeviceId,
                pciDomain, pciBus, pciDev, pciFunc));
    }
#else
    (void)print_found;
#endif

    for(ii = 0 ; prestera_sysmap[ii].name ; ii++)
    {
        if((pciDeviceId & prestera_sysmap[ii].devIdMask) ==
           (prestera_sysmap[ii].devId & prestera_sysmap[ii].devIdMask))
        {
            return ii;
        }
    }

    printf("sysfs_pci_detect_dev : not found [0x%4.4x] in prestera_sysmap[] \n",
        pciDeviceId);

    return -1;/* error indication */
}


static GT_STATUS sysfs_pci_map_resource(
    DBDF_DECL,
    IN  const char  *resname,
    IN  GT_UINTPTR   maxSize,
    IN  GT_UINTPTR   barOffset,
    OUT GT_UINTPTR  *mappedBase,
    OUT GT_UINTPTR  *mappedSize,
    OUT int         *fdPtr
)
{
    GT_U32  ii;
    GT_U32  regValue,orig_regValue;
    GT_U32  currentBarSize;

    for(ii = 0 ; resourceInfoArr[ii].resname ; ii++)
    {
        if(osStrCmp(resname,resourceInfoArr[ii].resname))
        {
            continue;
        }

        /* we need to 'calculate' the size of the BAR , since we not have Linux
            to give us this info
        */
        extDrvPciConfigReadReg (pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace , &orig_regValue);

        regValue = 0xFFFFFFFF;
        extDrvPciConfigWriteReg(pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace ,  regValue);
        extDrvPciConfigReadReg (pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace , &regValue);
        regValue &= 0xFFFFFFF0;/*ignore the lower 4 bits*/
        currentBarSize = (~regValue) + 1;

        if ((maxSize+barOffset) > currentBarSize || maxSize == 0)
        {
            maxSize = currentBarSize - barOffset;
        }

        *mappedBase =  orig_regValue & 0xFFFFFFF0;/*ignore the lower 4 bits*/
        *mappedSize =  maxSize;

        if(fdPtr) *fdPtr = 0;

        /* restore the base (BAR) */
        extDrvPciConfigWriteReg(pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace ,  orig_regValue);

        /* read the high address of the BAR */
        extDrvPciConfigReadReg (pciBus, pciDev ,pciFunc, resourceInfoArr[ii].addrInPciConfigSpace + 4 , &regValue);
#if __WORDSIZE == 64
        *mappedBase |= ((GT_UINTPTR)regValue) << 32;
#endif

        return GT_OK;
    }

    return GT_NOT_IMPLEMENTED;

}


static GT_STATUS sysfs_pci_configure_map(
    DBDF_DECL,
    IN  GT_U32              flags,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_UINTPTR bar0Base = 0;
    GT_UINTPTR bar0Size = 0;

    unsigned long long barPhys[3] = { 0, 0, 0 };
    int i;
    GT_UINTPTR   bar2MaxSize;

    if(0 == (flags & MV_EXT_DRV_CFG_FLAG_EAGLE_E))
    {
        return GT_NOT_SUPPORTED;
    }

    i = sysfs_pci_detect_dev(DBDF, 0);
    if (i < 0)
    {
        return GT_FAIL;
    }

    memset(hwInfoPtr, 0, sizeof(*hwInfoPtr));
    hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_PEX_E;
    hwInfoPtr->hwAddr.busNo = (pciDomain << 8) | pciBus;
    hwInfoPtr->hwAddr.devSel = pciDev;
    hwInfoPtr->hwAddr.funcNo = pciFunc;

    /* in Linux HW , the Linux already query the BAR0 and BAR2 about their
       sizes and assigned them with 'base' : BAR
       but in WM ... the OS not do it for us ... so we do it now.
    */
    wm_emulate_linux_set_pex_bar_addresses(DBDF);

#define R hwInfoPtr->resource

    /* config: BAR0*/
    sysfs_pci_map_resource(DBDF, "resource0", _1M, 0,
            &bar0Base, &bar0Size, NULL);

    sysfs_read_pex_bar_addresses(DBDF, barPhys);
    R.cnm.start = bar0Base;
    R.cnm.size  = bar0Size;
    R.cnm.phys  = (GT_PHYSICAL_ADDR)barPhys[0];

    bar2MaxSize = _64M;

    if (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E)
    {
        bar2MaxSize = _4M;
    }
    else if (flags & MV_EXT_DRV_CFG_FLAG_EAGLE_E)
    {
        /* Eyalo: currently no 2G support */
        bar2MaxSize = _8M;
        /*bar2MaxSize = _2G*/;
    }

    /* BAR2 */
    sysfs_pci_map_resource(DBDF, "resource2", bar2MaxSize, 0,
            &(R.switching.start), &(R.switching.size), NULL);
    R.switching.phys  = (GT_PHYSICAL_ADDR)barPhys[1];

    return GT_OK;
}
/**
* @internal prvExtDrvPciSysfsConfigure function
* @endinternal
*
* @brief   This routine maps all PP resources userspace and detects IRQ
*
* @param[out] hwInfoPtr                - pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigure
(
    DBDF_DECL,
    IN  GT_U32                  flags,
    OUT CPSS_HW_INFO_STC       *hwInfoPtr
)
{
    return sysfs_pci_configure_map(DBDF, flags, hwInfoPtr);
}
/**
* @internal extDrvPexConfigure function
* @endinternal
*
* @brief   This routine maps PP registers and PCI registers into userspace
*         then fill CPSS_HW_INFO_STC
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] flags                    - flags
*
* @param[out] hwInfoPtr                - The pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPexConfigure
(
    IN  GT_U32                  busNo,
    IN  GT_U32                  devSel,
    IN  GT_U32                  funcNo,
    IN  GT_U32                  flags,
    OUT CPSS_HW_INFO_STC        *hwInfoPtr
)
{
    GT_EXT_DRV_PCI_MAP_STC map;
    GT_STATUS rc;

    if(cpssHwDriverGenWmInPexModeGet())
    {
        rc = prvExtDrvPciSysfsConfigure(busNo>>8, busNo & 0xff, devSel, funcNo, flags, hwInfoPtr);
        if (rc != GT_OK)
        {
            osPrintf("Failed to prvExtDrvPciSysfsConfigure(): rc=%d\n", rc);
            return rc;
        }
    }
    else
    {

       memset(hwInfoPtr, 0, sizeof(*hwInfoPtr));
       hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_PEX_E;
       hwInfoPtr->hwAddr.busNo = busNo;
       hwInfoPtr->hwAddr.devSel = devSel;
       hwInfoPtr->hwAddr.funcNo = funcNo;

       rc = extDrvPciMap(busNo, devSel, funcNo,
               (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E) ? _4M :
               (flags & MV_EXT_DRV_CFG_FLAG_EAGLE_E)         ? _8M :
                                                               _64M,
               &map);
       if (rc != GT_OK)
       {
           osPrintf("Failed to extDrvPciMap(): rc=%d\n", rc);
           return rc;
       }
       hwInfoPtr->resource.cnm.start                    = map.config.base;
       hwInfoPtr->resource.cnm.size                     = map.config.size;
       hwInfoPtr->resource.switching.start              = map.regs.base;
       hwInfoPtr->resource.switching.size               = map.regs.size;
       hwInfoPtr->resource.resetAndInitController.start = map.dfx.base;
       hwInfoPtr->resource.resetAndInitController.size  = map.dfx.size;
       hwInfoPtr->resource.sram.start                   = map.sram.base;
       hwInfoPtr->resource.sram.size                    = map.sram.size;
    }

    rc = extDrvPcieGetInterruptNumber(busNo, devSel, funcNo,
        &(hwInfoPtr->irq.switching));
    if (rc != GT_OK)
    {
        osPrintf("Failed to extDrvPcieGetInterruptNumber(): rc=%d\n", rc);
    }
    hwInfoPtr->intMask.switching = (GT_UINTPTR)hwInfoPtr->irq.switching;
    return rc;
}

/**
* @internal extDrvPciFindDev function
* @endinternal
*
* @brief   This routine returns the next instance of the given device (defined by
*         vendorId & devId).
* @param[in] vendorId                 - The device vendor Id.
* @param[in] devId                    - The device Id.
* @param[in] instance                 - The requested device instance.
*
* @param[out] busNo                    - PCI bus number.
* @param[out] devSel                   - the device devSel.
* @param[out] funcNo                   - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvPciFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
)
{

    GT_U32  regData=0, asicDevId,FoundDev=0,SrcIns=instance;
    static GT_U32 BusDeviceInfo[255]={0};
    asicDevId = vendorId  |  (devId << 16);

    while ( ( instance < 128  ) && (FoundDev <= SrcIns) )
    {
        scibPciRegRead(instance, 0, 1, &regData);

        if (regData == asicDevId)
        {
            if (BusDeviceInfo[instance]==0)
            {
              BusDeviceInfo[instance]=1;
              *devSel = instance;
              return (GT_OK);
            }
            ++FoundDev;
        }
        ++instance;
    }

    return GT_FAIL;
}



/**
* @internal extDrvGetPciIntVec function
* @endinternal
*
* @brief   This routine return the PCI interrupt vector.
*
* @param[in] pciInt                   - PCI interrupt number.
*
* @param[out] intVec                   - PCI interrupt vector.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetPciIntVec
(
    IN  GT_PCI_INT  pciInt,
    OUT void        **intVec
)
{
    /* check parameters */
    if(intVec == NULL)
    {
        return GT_BAD_PARAM;
    }

    *intVec = (void *)pciInt;
    return GT_OK;
}

/**
* @internal extDrvGetIntMask function
* @endinternal
*
* @brief   This routine return the PCI interrupt vector.
*
* @param[in] pciInt                   - PCI interrupt number.
*
* @param[out] intMask                  - PCI interrupt mask.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*
* @note PCI interrupt mask should be used for interrupt disable/enable.
*
*/
GT_STATUS extDrvGetIntMask
(
    IN  GT_PCI_INT  pciInt,
    OUT GT_UINTPTR  *intMask
)
{

    return extDrvGetPciIntVec(pciInt,(void**)(intMask));
}

/**
* @internal extDrvPcieGetInterruptNumber function
* @endinternal
*
* @brief   This routine returns interrupt number for PCIe device
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
*
* @param[out] intNumPtr                - Interrupt number value
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPcieGetInterruptNumber
(
    IN  GT_U32      busNo,
    IN  GT_U32      devSel,
    IN  GT_U32      funcNo,
    OUT GT_U32     *intNumPtr
)
{
#ifdef GT_PCI
    GT_U32 data;

    if(intNumPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    scibPciRegRead(devSel,0x3c,1,&data);
    if ((data & 0x000000ff) == 0xff)
        return GT_FAIL;
    *intNumPtr = data & 0x000000ff;
    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal extDrvEnableCombinedPciAccess function
* @endinternal
*
* @brief   This function enables / disables the Pci writes / reads combining
*         feature.
*         Some system controllers support combining memory writes / reads. When a
*         long burst write / read is required and combining is enabled, the master
*         combines consecutive write / read transactions, if possible, and
*         performs one burst on the Pci instead of two. (see comments)
* @param[in] enWrCombine              - GT_TRUE enables write requests combining.
* @param[in] enRdCombine              - GT_TRUE enables read requests combining.
*
* @retval GT_OK                    - on sucess,
* @retval GT_NOT_SUPPORTED         - if the controller does not support this feature,
* @retval GT_FAIL                  - otherwise.
*
* @note 1. Example for combined write scenario:
*       The controller is required to write a 32-bit data to address 0x8000,
*       while this transaction is still in progress, a request for a write
*       operation to address 0x8004 arrives, in this case the two writes are
*       combined into a single burst of 8-bytes.
*
*/
GT_STATUS extDrvEnableCombinedPciAccess
(
    IN  GT_BOOL     enWrCombine,
    IN  GT_BOOL     enRdCombine
)
{
    return GT_OK;
}

/**
* @internal extDrvPciDoubleWrite function
* @endinternal
*
* @brief   This routine will write a 64-bit data to given address
*
* @param[in] address                  -  to write to
* @param[in] word1                    - the first half of double word to write (MSW)
* @param[in] word2                    - the second half of double word to write (LSW)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvPciDoubleWrite
(
    IN  GT_U32 address,
    IN  GT_U32 word1,
    IN  GT_U32 word2
)
{
        return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvPciDoubleRead function
* @endinternal
*
* @brief   This routine will read a 64-bit data from given address
*
* @param[in] address                  -  to read from
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvPciDoubleRead
(
    IN  GT_U32  address,
    OUT GT_U64  *dataPtr
)
{
        return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvPciConfigDev function
* @endinternal
*
* @brief   This routine enables PCI device
*
* @param[in] pciBus                - PCI bus number.
* @param[in] pciDev                - the device devSel.
* @param[in] pciFunc               - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciConfigDev
(
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc
)
{
    return GT_OK;
}

/**
* @internal extDrvPciGetDev function
* @endinternal
*
* @brief   This routine returns PCI vendor and device id
*          of the device identified by given BDF.
*
* @param[in] busNo        - PCI bus number.
* @param[in] devSel       - PCI device number.
* @param[in] funcNo       - function number.
*
* @param[out] vendorId    - device vendor Id.
* @param[out] devId       - device Id.
*
* @retval GT_OK           - on success,
* @retval GT_FAIL         - othersise.
*/
GT_STATUS extDrvPciGetDev
(
    IN  GT_U8  busNo,
    IN  GT_U8  devSel,
    IN  GT_U8  funcNo,
    OUT GT_U16 *vendorId,
    OUT GT_U16 *devId
)
{
    GT_U32 data;

    extDrvPciConfigReadReg(busNo, devSel, funcNo, 0x0/*address*/, &data);

    if(data == 0xFFFFFFFF)
        return GT_FAIL;

    *vendorId = data & 0xFFFF; /*0x11AB*/
    *devId    = data >> 16;

    return GT_OK;
}

/**
* @internal extDrvPciSetDevId function
* @endinternal
*
* @brief   This routine sets pci devId in driver shadow
* @param[in] devId                    - The device Id.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciSetDevId
(
    IN  GT_U16  devId
)
{
    return GT_OK;
}


