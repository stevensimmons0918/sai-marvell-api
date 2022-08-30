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
* @file noKmDrvUtils.c
*
* @brief misc utils
*
* @version   1
********************************************************************************
*/

#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

/* must be before prvNoKmDrv.h - to prevent redefinitions */
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <gtExtDrv/drivers/gtDmaDrv.h>
#include "prvNoKmDrv.h"
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
/* definitions collide */
#undef _4K
#undef _64K
#undef _256K
#undef _512K
#undef _1M
#undef _2M
#undef _4M
#undef _8M
#undef _16M
#undef _64M
#ifdef CHX_FAMILY
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#endif /*CHX_FAMILY*/
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>


#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

/***************************************************/
/*   register read/write definitions               */
/***************************************************/
GT_U32 prvNoKm_reg_read(GT_UINTPTR regsBase, GT_U32 regAddr)
{
    GT_U32 val;
#ifdef ASIC_SIMULATION
    ASIC_SIMULATION_ReadMemory(regsBase,regAddr,1,&val,regsBase+regAddr);
#else /*!ASIC_SIMULATION*/
    val = *((volatile GT_U32*)(regsBase+regAddr));
#endif /*!ASIC_SIMULATION*/
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "RD base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,val);
#endif
    return CPSS_32BIT_LE(val);
}

void prvNoKm_reg_write(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 value)
{
#ifdef ASIC_SIMULATION
    ASIC_SIMULATION_WriteMemory(regsBase,regAddr,1,&value,regsBase+regAddr);
#else /*!ASIC_SIMULATION*/
    *((volatile GT_U32*)(regsBase+regAddr)) = htole32(value);
#endif /*!ASIC_SIMULATION*/
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "WR base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,value);
#endif
}
void prvNoKm_reg_write_field(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 mask, GT_U32 value)
{
    GT_U32 val;
    val = prvNoKm_reg_read(regsBase, regAddr);
    val &= ~mask;
    val |= (value & mask);
    prvNoKm_reg_write(regsBase, regAddr, val);
}

#define BLOCK_SIZE 0x10000

/*******************************************************************************
* prvNoKmDrv_configure_dma
*   Configure DMA for PP
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma(
    IN GT_UINTPTR   regsBase,
    IN int          targetIsMbus
)
{
    GT_U32     attr, target, dmaSize, win;
    GT_UINTPTR dmaBase;

    if (extDrvGetDmaBase(&dmaBase) != GT_OK || dmaBase == 0L)
    {
        printf("extDrvGetDmaBase() failed, dma not configured\n");
        return GT_FAIL;
    }

#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "prvExtDrvDmaPtr=%p\n", (void*)dmaBase);
#endif
    extDrvGetDmaSize(&dmaSize);

    if ((dmaBase & 0x000fffff) != 0 || (dmaSize & 0x000fffff) != 0)
    {
        fprintf(stderr, "DMA base address %p and size 0x%x must be at least 1M aligned\n", (void*)dmaBase, dmaSize);
        return GT_FAIL;
    }

    attr    = (targetIsMbus) ? 0x00003e00 : 0x00000e04;
    target  = (targetIsMbus) ? 0x000003e0 : 0x000000e4;

    /* Set Unit Default ID (UDID) Register (0x00000204)
     * DATTR(11:4) = 0x000000e0 == identical to base address reg attr
     * DIDR(3:0)   = 0x00000004 == Target Unit ID PCIe
     */
    prvNoKm_reg_write(regsBase, 0x00000204, target);

    for (win = 0; win < 6 && dmaSize; win++)
    {
        GT_U32 size_mask, size_next;
        /* Calculate the BLOCK_SIZE==2^n where
         * baseAddr is BLOCK_SIZE aligned
         * and
         * dmaSize <= BLOCK_SIZE
         */
        size_mask = 0x00000000;
        while ((dmaBase & size_mask) == 0)
        {
            size_next = (size_mask<<1) | BLOCK_SIZE;
            if ((dmaBase & size_next) || size_next > dmaSize)
                break;
            size_mask = size_next;
            if (size_mask == 0xffff0000)
                break;
        }
        /* Configure DMA base in Base Address n Register (0x0000020c+n*8)
         * Attr(15:8)  = 0x00000e00 == target specific attr
         * Target(3:0) = 0x00000004 == target resource == PCIe
         */
        prvNoKm_reg_write(regsBase, 0x0000020c+win*8, (((GT_U32)dmaBase) & 0xffff0000) | attr);
        /* write dmaBase[63:32] to register 0x23c+n*4 (high address remap n) */
        prvNoKm_reg_write(regsBase, 0x0000023c+win*4, ((GT_U32)((PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64)) >> 32)));

        /* Configure size n register (0x00000210+n*8)
         * Set max size
         */
        prvNoKm_reg_write(regsBase, 0x00000210+win*8, size_mask);

        /* Window Control register n (0x00000254+n*4)
         * WinApn(2:1)    = 0x00000006 == RW
         * BARenable(0:0) = 0x00000000 == Enable
         */
        prvNoKm_reg_write(regsBase, 0x00000254+win*4, 0x00000006);

        if (size_mask == 0xffff0000)
            break;
        dmaBase += (size_mask + BLOCK_SIZE); /* += BLOCK_SIZE */
        dmaSize -= (size_mask + BLOCK_SIZE); /* -= BLOCK_SIZE */
    }

    return GT_OK;
}

/* PCI ATU (Address Translation Unit) registers */
#define OATU_PCI_BASE_ADDR  0x80000000 /*2G*/
#define OATU_PCI_SIZE       0x80000000 /*2G*/

typedef enum {
    ATU_REGION_CTRL_1_REG_OFFSET_IN_TABLE_E,
    ATU_REGION_CTRL_2_REG_OFFSET_IN_TABLE_E,
    ATU_LOWER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_UPPER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_LIMIT_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_LOWER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_UPPER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E

} ATU_REGION_REG_OFFSET_IN_TABLE;

struct SIP6_ATU_FIELD
{
    GT_U32 offset;
    GT_U32 value;
} atu_table[7] = {
    {0x0, 0x00000000},         /* ATU_REGION_CTRL_1_REG        */
    {0x4, 0x80000000},         /* ATU_REGION_CTRL_2_REG        */
    {0x8, OATU_PCI_BASE_ADDR}, /* ATU_LOWER_BASE_ADDRESS_REG   */
    {0xC, 0x00000000},         /* ATU_UPPER_BASE_ADDRESS_REG   */
    {0x10, 0xffffffff},        /* ATU_LIMIT_ADDRESS_REG        */
    {0x14, 0xffffffff},        /* ATU_LOWER_TARGET_ADDRESS_REG */
    {0x18, 0xffffffff}         /* ATU_UPPER_TARGET_ADDRESS_REG */
};

#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200

#define oATU_REGISTER_ADDRESS(_register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_register))

/*******************************************************************************
* prvNoKmDrv_configure_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*
*   mgRevision    - the MG unit revision
*   mgDoAddrRemap - do we need MG to do add remap or not.
*                   NOTE : relevant to mgRevision >= 1
*   internalCpu  - indecation if the 'internal CPU' used (AC5/Phoenix) or external
*******************************************************************************/
static GT_STATUS internal_prvNoKmDrv_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   ambMgCnmNicId,
    IN GT_U32   mgRevision,     /* 0 = AC5          1 = Falcon/Hawk/Phoenix*/
    IN GT_U32   mgDoAddrRemap,  /* 0 = AC5/Phoenix  1 = Falcon/Hawk        */
    IN GT_BOOL  internalCpu
)
{
    GT_STATUS   rc;
    GT_U32     dmaSize, win, i;
    GT_UINTPTR dmaBase;
    GT_U32  mgOffset;
    GT_U32  defaultAttribute = 0xE;/* 0xE is 'default' attribute from register 0x00000204 */
    GT_U32  BaseAddressValue;
    GT_U32  UnitDefaultIDValue;
    GT_U32  WindowControlValue;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32  regValue;
    GT_U32  PCIExpressCommandAndStatusRegisterAddr = 0x4;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;
    if (extDrvGetDmaBase(&dmaBase) != GT_OK)
    {
        printf("extDrvGetDmaBase() failed, dma not configured\n");
        return GT_FAIL;
    }
    extDrvGetDmaSize(&dmaSize);
    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }


#ifdef NOKMDRV_DEBUG
    printf("prvNoKmDrv_configure_dma_per_devNum : dmaBase[0x%x]\n",(GT_U32)dmaBase);
    printf("prvNoKmDrv_configure_dma_per_devNum : dmaSize[0x%x]\n",dmaSize);
#endif
    if (dmaBase == 0L)
        return GT_FAIL;

    if ((dmaBase & 0x000fffff) != 0 || (dmaSize & 0x000fffff) != 0)
    {
        fprintf(stderr, "DMA memory base %p and size 0x%x must be at least 1M aligned\n", (void*)dmaBase, dmaSize);
        return GT_FAIL;
    }

    if(dmaSize > OATU_PCI_SIZE)
    {
        fprintf(stderr, "DMA allocated size 0x%x of more than '2GB' is not supported by the device.\n", dmaSize);
        return GT_FAIL;
    }

    if( internalCpu == GT_FALSE )
    {
        drv = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId];
        if( NULL == drv )
        {
            fprintf(stderr, "HW Driver not initialized\n");
            return GT_FAIL;
        }

        /* Configure oATU */
#ifdef NOKMDRV_DEBUG
	printf("oATU: ");
#endif
        atu_table[2].value = (dmaBase | (1 << 31)) & 0xfff00000;
        atu_table[4].value = atu_table[2].value + dmaSize;
        atu_table[5].value = dmaBase & 0xfff00000;
        atu_table[6].value = (GT_U32)((PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64)) >> 32);
        for (i = 0; i < sizeof(atu_table) / sizeof(struct SIP6_ATU_FIELD); i++)
        {
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
#ifdef NOKMDRV_DEBUG
		printf("0x%x\t", atu_table[i].value);
#endif
                rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                                oATU_REGISTER_ADDRESS(atu_table[i].offset),
                                &atu_table[i].value, 1, 0xffffffff);
                if( GT_OK != rc )
                    return rc;
            }
        }
#ifdef NOKMDRV_DEBUG
	printf("\n");
#endif

        /* Enable PCIe bus mastering.
         * Should be enabled for SDMA
         * PP will generate master transactions as End Point
         *
         * PCI Express Command and Status Register
         * 0x40004
         * Bit 2: Master Enable. This bit controls the ability of the device
         *        to act as a master on the PCI Express port.
         *        When set to 0, no memory or I/O read/write request packets
         *        are generated to PCI Express.
         */
        regValue = 0x4; /* <Master Enable> - bit 2 */
        if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            rc = drv->writeMask(drv,CPSS_HW_DRIVER_AS_PCI_CONFIG_E,
                                PCIExpressCommandAndStatusRegisterAddr,
                                &regValue, 1, 0x4);
            if(rc != GT_OK)
                return rc;
        }
    }

#ifdef CHX_FAMILY
    mgOffset = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_MG_E,NULL);
#else /*! CHX_FAMILY*/
    mgOffset = 0;
#endif/*! CHX_FAMILY*/

    BaseAddressValue    = defaultAttribute << 8/*Attr*/               | ambMgCnmNicId/*<Target >*/;
    UnitDefaultIDValue  = defaultAttribute << 4/*Default Attributes*/ | ambMgCnmNicId/*<Default ID >*/;

    /* Set Unit Default ID (UDID) Register (0x00000204)
     * DATTR(11:4) = 0x000000e0 == identical to base address reg attr
     * DIDR(3:0)   = 0x00000003 == Target Unit ID PCIe
     */
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x00000204, UnitDefaultIDValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (mgRevision == 0)
        mgDoAddrRemap = 0;
    /* Window Control register n (0x00000254+n*4)
     * WinApn(2:1)    = 0x00000006 == RW
     * BARenable(0:0) = 0x00000000 == Enable
        need to set bits 3 <Window Remap Enable> value 0x1
        and in bits 16..31 <Window Remap Value> value 0x8000
     */
    /* we need the remap because the CnM subsystem need the address to be in range of :
        0x80000000 - 0xFFFFFFFF

        the 'map back' is done in BAR0 in the OATU (outband ATU) windows
        (direction from the CnM to the Host CPU).
    */
    WindowControlValue =  (3 << 1); /*WinApn(2:1) - allow read/write access */
    if (mgDoAddrRemap != 0)     /* remap needed */
        WindowControlValue |= (1 << 3) |/*<Window Remap Enable>*/
                              (0x8000 << 16); /*<Window Remap Value>*//*OATU_PCI_BASE_ADDR*/

    for (win = 0; win < 6 && dmaSize; win++)
    {
        GT_U32 size_mask, size_next;
        /* Calculate the BLOCK_SIZE==2^n where
         * baseAddr is BLOCK_SIZE aligned
         * and
         * dmaSize <= BLOCK_SIZE
         */
        size_mask = 0x00000000;
        while ((dmaBase & size_mask) == 0)
        {
            size_next = (size_mask<<1) | BLOCK_SIZE;
            if ((dmaBase & size_next) || size_next > dmaSize)
                break;
            size_mask = size_next;
            if (size_mask == 0xffff0000)
                break;
        }

#ifdef NOKMDRV_DEBUG
        printf("prvNoKmDrv_configure_dma_per_devNum : size_mask[0x%x]\n",size_mask);
        printf("prvNoKmDrv_configure_dma_per_devNum : (GT_U32)(prvExtDrvDmaPhys64 >> 32)[0x%x]\n",(GT_U32)((PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64)) >> 32));
#endif
        /* Configure DMA base in Base Address n Register (0x0000020c+n*8)
         * Attr(15:8)  = 0x00000e00 == target specific attr
         * Target(3:0) = 0x00000003 == target resource == PCIe
         */
        if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x0000020c+win*8, (((GT_U32)dmaBase) & 0xffff0000) | BaseAddressValue);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Configure size n register (0x00000210+n*8)
             * Set max size
             */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x00000210+win*8, size_mask);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* Window Control register n (0x00000254+n*4)
         * WinApn(2:1)    = 0x00000006 == RW
         * BARenable(0:0) = 0x00000000 == Enable
            need to set bits 3 <Window Remap Enable> value 0x1
            and in bits 16..31 <Window Remap Value> value 0x8000 OATU_PCI_BASE_ADDR
         */

        if (mgDoAddrRemap != 0)     /* remap needed */
        {
            WindowControlValue &= (~0x7FFF0000); /* clear bits 16..30 */
            /* OR the bits 16..31 (remember that bit 31 is set at WindowControlValue) */
            WindowControlValue |= (((GT_U32)dmaBase) & 0xffff0000);
        }

#ifdef NOKMDRV_DEBUG
        printf("prvNoKmDrv_configure_dma_per_devNum : WindowControlValue[0x%x]\n",WindowControlValue);
#endif
        if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x00000254+win*4, WindowControlValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if (size_mask == 0xffff0000)
            break;
        dmaBase += (size_mask + BLOCK_SIZE);
        dmaSize -= (size_mask + BLOCK_SIZE);
    }

    /* the register doesn't exist in Falcon
        prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x2684, 0xaaba);
    */

    return GT_OK;
}

/*******************************************************************************
* prvNoKmDrv_configure_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    GT_U32  ambMgCnmNicId = 3;/* (from the CnM Subsystem figure) 'leg' #3 of the CnM NIC connected to <AMB MG> */

    return internal_prvNoKmDrv_configure_dma_per_devNum(devNum,portGroupId,ambMgCnmNicId,1/*mgRevision = 1*/,1/* need MG remap */,GT_FALSE/* not internal CPU*/);
}

/*
    The window '2' is relevant in AC5 for 'PCI' , that used for 'SDMA purposes' from the 'AMB3'
    We want to use 'remap' to be 'compatible' to 2G offset that the PCI have in Falcon , so configuring the OATU windows at synopsys is kept as is. (not project depended)
    So I suggest make CnM address decoder for client AM3 to recognize memory of :
    Window Size Low = 2G
    Window Remap Low = 2G
    If dmaBase >= 2G
        Window Base Low = 2G
    else
        Window Base Low = 0
    end
*/
static GT_STATUS internal_prvNoKmDrv_configure_CnM_window_ctrl(
    IN GT_U8    devNum,
    IN GT_U32   clientIndex,           /*0..8*/
    IN GT_U32   windowIndex,           /*0..15*/
    IN GT_UINTPTR  base,                  /* */
    IN GT_UINTPTR  size,                  /* */
    IN GT_UINTPTR  remap                  /* */
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 offset = 0x80400000 + 0x00010000 * clientIndex + 0x18 * windowIndex;

    if (base & 0xffff || size & 0xffff || remap & 0xffff)
    {
        printf("CnM_window_ctrl: base, size and remap must be 64KB aligned\n");
        return GT_FAIL;
    }

    /*{0x00000100 , "Window Attributes and Size High"}*/
    /* disable the window , as some fields require it to be disabled for change */
    rc = prvCpssDrvHwPpSetRegField(devNum, offset + 0x100,  0,1,0);

    /*{0x00000104 , "Window Size Low"}*/
    if(rc == GT_OK)
        rc = prvCpssDrvHwPpWriteRegister(devNum, offset + 0x104, (GT_U32)((size >> 16)-1));

    /*{0x0000010C , "Window Base Low"}*/
    if(rc == GT_OK)
        rc = prvCpssDrvHwPpWriteRegister(devNum, offset + 0x10c, (GT_U32)(base >> 16));

    /*{0x00000110 , "Window Remap Low"}*/
    if(rc == GT_OK)
        rc = prvCpssDrvHwPpWriteRegister(devNum, offset + 0x110, (GT_U32)(remap >> 16));

    /*{0x00000100 , "Window Attributes and Size High"}*/
    /* re-enable the window , as some fields require it to be disabled for change */
    if(rc == GT_OK)
        rc = prvCpssDrvHwPpSetRegField(devNum, offset + 0x100, 0,1,1);

    return rc;
}

/*/Cider/EBU/AC5/AC5 {Current}/<CNM_IP>CNM_IP/<CNM_IP> <ADDR_DEC> Address Decoder/Units %c*/
#define PHOENIX_CNM_ADDR_DECODER_BASE_ADDR 0x80400000
#define AC5_ON_CHIP_DDR_ADDR  0x200000000L
typedef enum{
CNM_WINDOW_INDEX_RESERVED_0         = 0 ,
CNM_WINDOW_INDEX_AMB3_0             = 1 ,
CNM_WINDOW_INDEX_PCIe               = 2 ,
CNM_WINDOW_INDEX_CPU_ACP            = 3 ,
CNM_WINDOW_INDEX_AMB2               = 4 ,
CNM_WINDOW_INDEX_MG0_CM3_SRAM       = 5 ,
CNM_WINDOW_INDEX_MG1_CM3_SRAM       = 6 ,
CNM_WINDOW_INDEX_MG2_CM3_SRAM       = 7 ,
CNM_WINDOW_INDEX_ROM                = 8 ,
CNM_WINDOW_INDEX_SRAM               = 9 ,
CNM_WINDOW_INDEX_RESERVED_2         = 10,
CNM_WINDOW_INDEX_RESERVED_3         = 11,
CNM_WINDOW_INDEX_DDR                = 12,
CNM_WINDOW_INDEX_iNIC               = 13,
CNM_WINDOW_INDEX_AMB3_1             = 14,
CNM_WINDOW_INDEX_RESERVED_4         = 15,
}CNM_WINDOW_INDEX_ENT;

/*******************************************************************************
* prvNoKmDrv_configure_dma_over_the_pci
*   External CPU that the PP send SDMA to external DDR over the pci (not onChip DDR)
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma_over_the_pci(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32 ambMgCnmNicId = 3;/* (from the CnM Subsystem figure) 'leg' #3 of the CnM NIC connected to <AMB MG> */
    GT_U32 mgRevision = 0;
    GT_UINTPTR dmaBase;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            mgRevision = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            mgRevision = 1;
            break;
        default:
            return GT_OK;
    }

    rc =  internal_prvNoKmDrv_configure_dma_per_devNum(devNum, 0, ambMgCnmNicId, mgRevision, 0 /* don't care */, GT_FALSE/* not internal CPU*/);
    /*
        Configure PCI  window ('2') in AC5 for 'PCI' for 'SDMA purposes' from the 'AMB3'
        Remap higher 2G to be compatible with Falcon, thus have same PCIe OATU settings.
        So I suggest make CnM address decoder for client AM3 to recognize memory of :
        Window Size Low = 2G
        Window Remap Low = 2G
        If dmaBase >= 2G
            Window Base Low = 2G
        else
            Window Base Low = 0
        end
    */
    if (rc == GT_OK)
    {
        if (extDrvGetDmaBase(&dmaBase) != GT_OK)
        {
            printf("extDrvGetDmaBase() failed, dma not configured\n");
            return GT_FAIL;
        }
        /* dma base already checked in internal_prvNoKmDrv_configure_dma_per_devNum() */
        rc = internal_prvNoKmDrv_configure_CnM_window_ctrl(
                devNum,
                ambMgCnmNicId,          /* AMB3 */
                CNM_WINDOW_INDEX_PCIe,  /* window[2] */
                (dmaBase & OATU_PCI_BASE_ADDR) ? OATU_PCI_BASE_ADDR : 0,  /* base address */
                OATU_PCI_SIZE,          /* size */
                OATU_PCI_BASE_ADDR      /* remap */);
    }

    return rc;
}

/*******************************************************************************
*
*   With internal CPU that the PP send SDMA to internal DDR (onChip DDR)
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma_internal_cpu(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_U32  ambMgCnmNicId = 2;/* (from the CnM Subsystem figure) 'leg' #2 of the CnM NIC connected to <AMB MG> */
    GT_U32 mgRevision = 0;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            mgRevision = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            mgRevision = 1;
            break;
        default:
            return GT_OK;
    }
    rc = internal_prvNoKmDrv_configure_dma_per_devNum(devNum, 0, ambMgCnmNicId, mgRevision, 0 /* don't care */, GT_TRUE/*internal CPU*/);

    return rc;
}

/***************************************************************************************
 * Debug functions
 ***************************************************************************************/


GT_STATUS prvNoKmDrv_debug_print_CnM_Address_Decoder(
    IN GT_U8    devNum,
    IN GT_U32   clientIndex /* 0.. 8*//*0x80400000 + c*0x10000: where c (0-8) represents Address Decoder instance*/
)
{
    GT_U32  regValue,ii,addr,regId, baseAddr = PHOENIX_CNM_ADDR_DECODER_BASE_ADDR;
    struct{
        GT_U32  offset;
        char*   name;
    }infoReg[6] =    {
         {0x00000100 , "Window Attributes and Size High"}
        ,{0x00000104 , "Window Size Low"}
        ,{0x00000108 , "Window Base and Remap High"}
        ,{0x0000010C , "Window Base Low"}
        ,{0x00000110 , "Window Remap Low"}
        ,{0x00000114 , "AXI Attributes"}
    };

    char* clientNames[9] = {
      /*0*/   "CPU - 0x8040_0000"
      /*1*/  ,"CoreSight Trace - 0x8041_0000"
      /*2*/  ,"AMB2 - 0x8042_0000"
      /*3*/  ,"AMB3 - 0x8043_0000"
      /*4*/  ,"iNIC - 0x8044_0000"
      /*5*/  ,"XOR0 - 0x8045_0000"
      /*6*/  ,"XOR1 - 0x8046_0000"
      /*7*/  ,"PCIe - 0x8047_0000"
      /*8*/  ,"GIC  - 0x8048_0000"};

    if(clientIndex >= 9 )
    {
        printf("index[%d] must be less than 9 \n",clientIndex);
        return GT_BAD_PARAM;
    }

    baseAddr &= 0xffff0000;
    baseAddr += 0x00010000 * clientIndex;

    printf("client[%s] \n",clientNames[clientIndex]);

    for(ii = 0 ; ii < 16 ; ii++)
    {
        printf("start window[%d] \n",ii);

        for(regId = 0 ; regId < 6 ; regId++)
        {
            addr = baseAddr + infoReg[regId].offset + ii*0x18;
            regValue = 0;
            prvCpssDrvHwPpReadRegister(devNum,addr, &regValue);

            printf("addr[0x%8.8x] : value [0x%8.8x] --> [%s][%d]  \n",
                addr , regValue , infoReg[regId].name,regId);
        }

        printf("end window[%d] \n\n\n",ii);
    }

    return GT_OK;
}

GT_STATUS prvNoKmDrv_debug_print_MG_Address_Decoder(
    IN GT_U8    devNum,
    IN GT_U32   mgIndex
)
{
    GT_U32  regValue,ii,addr,regId,mgOffset;
    struct{
        GT_U32  offset;
        GT_U32  step;
        char*   name;
    }infoReg[4] =    {
         {0x0000020C , 0x8,"Base Address"}
        ,{0x00000210 , 0x8,"Size (S)"}
        ,{0x0000023C , 0x4,"High Address Remap (HA)"}
        ,{0x00000254 , 0x4,"Window Control Register"}
    };

#ifdef CHX_FAMILY
    {
        GT_BOOL isError;
        PRV_CPSS_DXCH_UNIT_ENT   unitId;
        unitId =  mgIndex == 0 ?
                PRV_CPSS_DXCH_UNIT_MG_E :
                PRV_CPSS_DXCH_UNIT_MG_0_1_E + mgIndex - 1;
        mgOffset = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,&isError);
        if(isError)
        {
            printf("MG[%d] not supported \n",mgIndex);
            return GT_BAD_PARAM;
        }
    }
#else /*! CHX_FAMILY*/
    mgOffset = 0;
    GT_UNUSED_PARAM(mgIndex);
#endif/*! CHX_FAMILY*/

    for(ii = 0 ; ii < 6 ; ii++)
    {
        printf("start window[%d] \n",ii);

        for(regId = 0 ; regId < 4 ; regId++)
        {
            addr = mgOffset + infoReg[regId].offset + ii*infoReg[regId].step;
            regValue = 0;
            prvCpssDrvHwPpReadRegister(devNum,addr, &regValue);

            printf("addr[0x%8.8x] : value [0x%8.8x] --> [%s][%d]  \n",
                addr , regValue , infoReg[regId].name,regId);
        }

        printf("end window[%d] \n\n\n",ii);
    }

    return GT_OK;
}


/* debug function to print the oATU registers of the device */
GT_STATUS prvNoKmDrv_debug_print_oAtu_window(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_U32 data;
    GT_U32 regAddr;

    printf("DUMP oATU settings device[%d] \n",devNum);

#define PRINT_oATU_REG_VALUE(_regAddr,value,_regName)          \
    printf("oATU addr[0x%8.8x] : value [0x%8.8x] --> [%s] \n", \
           _regAddr , value , #_regName)

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_LOWER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }

    PRINT_oATU_REG_VALUE(regAddr,data,ATU_LOWER_BASE_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_UPPER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_UPPER_BASE_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_LIMIT_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_LIMIT_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_LOWER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_LOWER_TARGET_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_UPPER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_UPPER_TARGET_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_REGION_CTRL_1_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_REGION_CTRL_1_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_REGION_CTRL_2_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_REGION_CTRL_2_REG);

    return GT_OK;
}

/* debug function to print a register value in Bar0/oATU memory space of the device */
GT_STATUS prvNoKmDrv_debug_print_oAtu_register(
    IN GT_U8    devNum,
    IN GT_U32   regOffset
)
{
    GT_STATUS rc;
    GT_U32  data;

    rc = prvCpssDrvHwPpBar0ReadReg(devNum,regOffset,&data);
    if(rc != GT_OK)
    {
        return rc;
    }

    printf("oAtu : devNum[%d] Register[0x%8.8x] Read value[0x%8.8x] \n",
    devNum,regOffset,data);

    return GT_OK;
}
