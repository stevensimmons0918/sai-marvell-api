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
* @file newdrv_falconZCntl.c
*
* @brief Driver for FalconZ to emulate PEX
*       Supports full mamory mapped address space
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
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
#include <asicSimulation/SCIB/scib.h>
#endif

#include <cpss/common/cpssTypes.h>


#define NUM_OF_TILES       4
#define TILE_OFFSET_OFFSET 0x20000000

#define NUM_OF_RAVENS_PER_TILE  4
#define RAVEN_MEMORY_SPACE_SIZE 0x01000000

#define RAVENS_MEMORY_SPACE_PER_TILE (NUM_OF_RAVENS_PER_TILE*RAVEN_MEMORY_SPACE_SIZE)
#define NUM_OF_RAVENS (NUM_OF_TILES * NUM_OF_TILES)


typedef struct CPSS_HW_DRIVER_FALCON_Z_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_UINTPTR          base;

    CPSS_HW_DRIVER_STC* bar0;

} CPSS_HW_DRIVER_FALCON_Z_STC;

typedef struct FALCON_Z_DEFAULT_REG_STCT {
    GT_U32 regAddr;
    GT_U32 regData;

} FALCON_Z_DEFAULT_REG_STC;

/* List of registers outside of Ravens address space range that must be emulated */
/* by non-zero values. This is required for cpssInitSystem to pass successfully. */
/* pizza size fixed to 0x21 i.e. max-slice==0x20                                 */
FALCON_Z_DEFAULT_REG_STC falconZdefaultRegs[] = {
    { 0x180bff70, 0x00000015},
    { 0x1d00004c, 0x00086000},
    { 0x1d000050, 0x000011ab},
    { 0x1bef8010, 0x00000030},

    { 0x0d560050, 0x00000020},
    { 0x0d570050, 0x00000020},
    { 0x0d580050, 0x00000020},
    { 0x0d590050, 0x00000020},
    { 0x0d5a0050, 0x00000020},
    { 0x0d5b0050, 0x00000020},
    { 0x0d5c0050, 0x00000020},
    { 0x0d5d0050, 0x00000020},
    { 0x15560050, 0x00000020},
    { 0x15570050, 0x00000020},
    { 0x15580050, 0x00000020},
    { 0x15590050, 0x00000020},
    { 0x155a0050, 0x00000020},
    { 0x155b0050, 0x00000020},
    { 0x155c0050, 0x00000020},
    { 0x155d0050, 0x00000020},
    { 0x2d560050, 0x00000020},
    { 0x2d570050, 0x00000020},
    { 0x2d580050, 0x00000020},
    { 0x2d590050, 0x00000020},
    { 0x2d5a0050, 0x00000020},
    { 0x2d5b0050, 0x00000020},
    { 0x2d5c0050, 0x00000020},
    { 0x2d5d0050, 0x00000020},
    { 0x35560050, 0x00000020},
    { 0x35570050, 0x00000020},
    { 0x35580050, 0x00000020},
    { 0x35590050, 0x00000020},
    { 0x355a0050, 0x00000020},
    { 0x355b0050, 0x00000020},
    { 0x355c0050, 0x00000020},
    { 0x355d0050, 0x00000020},
    { 0x4d560050, 0x00000020},
    { 0x4d570050, 0x00000020},
    { 0x4d580050, 0x00000020},
    { 0x4d590050, 0x00000020},
    { 0x4d5a0050, 0x00000020},
    { 0x4d5b0050, 0x00000020},
    { 0x4d5c0050, 0x00000020},
    { 0x4d5d0050, 0x00000020},
    { 0x55560050, 0x00000020},
    { 0x55570050, 0x00000020},
    { 0x55580050, 0x00000020},
    { 0x55590050, 0x00000020},
    { 0x555a0050, 0x00000020},
    { 0x555b0050, 0x00000020},
    { 0x555c0050, 0x00000020},
    { 0x555d0050, 0x00000020},
    { 0x6d560050, 0x00000020},
    { 0x6d570050, 0x00000020},
    { 0x6d580050, 0x00000020},
    { 0x6d590050, 0x00000020},
    { 0x6d5a0050, 0x00000020},
    { 0x6d5b0050, 0x00000020},
    { 0x6d5c0050, 0x00000020},
    { 0x6d5d0050, 0x00000020},
    { 0x75560050, 0x00000020},
    { 0x75570050, 0x00000020},
    { 0x75580050, 0x00000020},
    { 0x75590050, 0x00000020},
    { 0x755a0050, 0x00000020},
    { 0x755b0050, 0x00000020},
    { 0x755c0050, 0x00000020},
    { 0x755d0050, 0x00000020},

    { 0x0d7f7058, 0x00060000},
    { 0x0d807058, 0x00060000},
    { 0x0d817058, 0x00060000},
    { 0x0d827058, 0x00060000},
    { 0x157f7058, 0x00060000},
    { 0x15807058, 0x00060000},
    { 0x15817058, 0x00060000},
    { 0x15827058, 0x00060000},
    { 0x357f7058, 0x00060000},
    { 0x35807058, 0x00060000},
    { 0x35817058, 0x00060000},
    { 0x35827058, 0x00060000},
    { 0x2d7f7058, 0x00060000},
    { 0x2d807058, 0x00060000},
    { 0x2d817058, 0x00060000},
    { 0x2d827058, 0x00060000},
    { 0x4d7f7058, 0x00060000},
    { 0x4d807058, 0x00060000},
    { 0x4d817058, 0x00060000},
    { 0x4d827058, 0x00060000},
    { 0x557f7058, 0x00060000},
    { 0x55807058, 0x00060000},
    { 0x55817058, 0x00060000},
    { 0x55827058, 0x00060000},
    { 0x757f7058, 0x00060000},
    { 0x75807058, 0x00060000},
    { 0x75817058, 0x00060000},
    { 0x75827058, 0x00060000},
    { 0x6d7f7058, 0x00060000},
    { 0x6d807058, 0x00060000},
    { 0x6d817058, 0x00060000},
    { 0x6d827058, 0x00060000},

};


#ifndef ASIC_SIMULATION
/***************************/
/*   SMI Drivers Creation  */
/***************************/
#if (__ARM_ARCH == 8) && defined(LINUX)
extern void soc_init_a7k(void);
#define SOC_INIT_A7K() soc_init_a7k()
#else
#define SOC_INIT_A7K()
#endif
CPSS_HW_DRIVER_STC *falconSmi;
CPSS_HW_DRIVER_STC *falconZ[NUM_OF_RAVENS];

#define RAVEN_DFX_BASE_ADDR               0x700000
#define DFX_SERVER_STATUS_REG_ADDR_CNS    0xF8010
#define SMI_DEVICE_IDENTIFIER1_REG_ADDR_CNS   2
#define SMI_DEVICE_MUX_SMI_ADDR_CNS      0x11

#define XSMI_CONFIGURATION 0x33000C

GT_U8 falconZCurrentSmiMux = 0xFF;

static GT_STATUS falconZCreateSmiDrivers(void)
{
    GT_U32 smiDevIndex;
    GT_U32 data;
    GT_U32 counter;
    CPSS_HW_INFO_STC   hwInfo;
    int localPhy;
    GT_STATUS rc;


    if (cpssHwDriverLookup("/SoC") == NULL)
    {
        SOC_INIT_A7K();
    }

    if (cpssHwDriverLookup("/SoC/internal-regs/smi@0x12a200") != NULL)
    {
        CPSS_HW_DRIVER_STC *soc = cpssHwDriverLookup("/SoC/internal-regs");
        if (soc != NULL)
        {
            /* 0 - normal speed(/128), 1 - fast mode(/16), 2 - accel (/8) */

            /*
                PLEASE PAY ATTENTION:
                    bit_10 is disabled - "Enable Polling" disabled in order to solve congestion problem on SMI bus
            */

            /* current speed - fast(5.2Mhz)*/
            data = 0xA;
            soc->writeMask(soc, 0, 0x12a204, &data, 1, 0x40F);
        }
    }

    falconSmi = cpssHwDriverLookup("/smi0");
    if(falconSmi == NULL)
    {
        cpssOsPrintf("ERROR: SMI driver not found\n\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    for(smiDevIndex = 0; smiDevIndex < NUM_OF_RAVENS; smiDevIndex++)
    {
        /*************************************/
        /*     SMI Driver initialization     */
        /*************************************/

        localPhy = smiDevIndex % 8;

        /* The MUX selected MUX state executed through write to SMI Device Addr 0x11, Reg:0x1, Bit[0] */

        data = smiDevIndex / 8;

        falconSmi->writeMask(falconSmi, SMI_DEVICE_MUX_SMI_ADDR_CNS, 1, &data, 1, 0xFFFF);
        cpssOsTimerWkAfter(10);

        falconSmi->read(falconSmi, localPhy, SMI_DEVICE_IDENTIFIER1_REG_ADDR_CNS, &data, 1);
        if (data != 0x0141) /* not marvell slave smi device */
        {
            cpssOsPrintf("Not MARVELL slave SMI device found, SMI slave addr = 0x%x, id=0x%x\n", smiDevIndex, data);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }
        /*else
        {
            cpssOsPrintf("MARVELL slave SMI device found, SMI slave addr = 0x%x\n", smiDevIndex);
        }*/

        rc = cpssHwDriverSlaveSmiConfigure("/smi0", localPhy, &hwInfo);
        /* TEMP, CPSS will do it later */
        if (rc == GT_OK)
        {
            char buf[64];
            cpssOsSprintf(buf,"/raven%d", smiDevIndex);
            cpssHwDriverAddAlias(hwInfo.driver, buf);
            falconZ[smiDevIndex] = hwInfo.driver;
        }


        if (falconZ[smiDevIndex] == NULL)
        {
            cpssOsPrintf("cpssHwDriverSip5SlaveSMICreateDrv returned with NULL pointer for devNum = %d\n", smiDevIndex);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /*********************************/
        /*     Device initialization     */
        /*********************************/
        /*
            Init stage polling
            This polling is on the init_stage field (bits 5:4) of the Server
            Status register. Polling is continued until this field is equal
            2'b11 (Fully Functional), or until the timeout expires (100ms).
        */

        data = 0;
        counter = 0;

        while(data != 3)
        {
            falconZ[smiDevIndex]->read(falconZ[smiDevIndex], SSMI_FALCON_ADDRESS_SPACE, DFX_SERVER_STATUS_REG_ADDR_CNS + RAVEN_DFX_BASE_ADDR, &data, 1);
            data = (data >> 4) & 3;
            if(counter == 100)
            {
                cpssOsPrintf("ERROR: Device not ready, Slave Smi Addr = 0x%x\n", smiDevIndex+0x10);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            counter++;
            cpssOsTimerWkAfter(1);
        }

        /*cpssOsPrintf("SMI Device %d is ready\n", smiDevIndex);*/
    }

    return GT_OK;
}


GT_STATUS falconZSmiMuxConfig
(
    GT_U8 devNum
)
{
    GT_U32 data;

    data = devNum / 8;

    if(data != falconZCurrentSmiMux)
    {
        falconZCurrentSmiMux = data;
        falconSmi->writeMask(falconSmi, SMI_DEVICE_MUX_SMI_ADDR_CNS, 1, &data, 1, 0xFFFF);
        cpssOsTimerWkAfter(1);
    }

    return GT_OK;
}

GT_STATUS smiDriverIndexGet(GT_U32 regAddr, GT_U8 *drvIndex)
{
    GT_U8 tileNum;

    /* Mapping between Raven Address space to driver index: */
    /****************************************/
    /* Address Space             |   Index  */
    /****************************************/
    /* 0x00000000 - 0x00ffffff   |     0    */
    /* 0x01000000 - 0x01ffffff   |     1    */
    /* 0x02000000 - 0x02ffffff   |     2    */
    /* 0x03000000 - 0x03ffffff   |     3    */
    /* 0x20000000 - 0x20ffffff   |     7    */
    /* 0x21000000 - 0x21ffffff   |     6    */
    /* 0x22000000 - 0x22ffffff   |     5    */
    /* 0x23000000 - 0x23ffffff   |     4    */
    /* 0x40000000 - 0x40ffffff   |     8    */
    /* 0x41000000 - 0x41ffffff   |     9    */
    /* 0x42000000 - 0x42ffffff   |    10    */
    /* 0x43000000 - 0x43ffffff   |    11    */
    /* 0x60000000 - 0x60ffffff   |    15    */
    /* 0x61000000 - 0x61ffffff   |    14    */
    /* 0x62000000 - 0x62ffffff   |    13    */
    /* 0x63000000 - 0x63ffffff   |    12    */

    tileNum = (regAddr/TILE_OFFSET_OFFSET);

    if( (tileNum%2) == 0 )
    {
        *drvIndex = (regAddr%TILE_OFFSET_OFFSET)/RAVEN_MEMORY_SPACE_SIZE;
    } else {
        *drvIndex = (NUM_OF_RAVENS_PER_TILE - (regAddr%TILE_OFFSET_OFFSET)/RAVEN_MEMORY_SPACE_SIZE - 1);
    }

    *drvIndex += (tileNum * NUM_OF_RAVENS_PER_TILE);

    return GT_OK;
}

#endif /* ASIC_SIMULATION */
GT_STATUS smiDriverBaseAddressGet(GT_U8 drvIndex,GT_U32 *regAddr )
{
    /* Mapping between Raven Address space to driver index: */
    /****************************************/
    /* Address Space             |   Index  */
    /****************************************/
    /* 0x00000000 - 0x00ffffff   |     0    */
    /* 0x01000000 - 0x01ffffff   |     1    */
    /* 0x02000000 - 0x02ffffff   |     2    */
    /* 0x03000000 - 0x03ffffff   |     3    */
    /* 0x20000000 - 0x20ffffff   |     7    */
    /* 0x21000000 - 0x21ffffff   |     6    */
    /* 0x22000000 - 0x22ffffff   |     5    */
    /* 0x23000000 - 0x23ffffff   |     4    */
    /* 0x40000000 - 0x40ffffff   |     8    */
    /* 0x41000000 - 0x41ffffff   |     9    */
    /* 0x42000000 - 0x42ffffff   |    10    */
    /* 0x43000000 - 0x43ffffff   |    11    */
    /* 0x60000000 - 0x60ffffff   |    15    */
    /* 0x61000000 - 0x61ffffff   |    14    */
    /* 0x62000000 - 0x62ffffff   |    13    */
    /* 0x63000000 - 0x63ffffff   |    12    */

    *regAddr = (drvIndex/NUM_OF_RAVENS_PER_TILE) * TILE_OFFSET_OFFSET;

    if( (drvIndex % 8) < 4 )
    {
        *regAddr += (drvIndex % NUM_OF_RAVENS_PER_TILE) * RAVEN_MEMORY_SPACE_SIZE;
    } else {
        *regAddr += (NUM_OF_RAVENS_PER_TILE - (drvIndex % NUM_OF_RAVENS_PER_TILE) - 1) * RAVEN_MEMORY_SPACE_SIZE;
    }

    return GT_OK;
}


static GT_STATUS cpssHwDriverFalconZReadNoMtx(
    IN  CPSS_HW_DRIVER_FALCON_Z_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32      data;
    GT_U32      falconZ2IntRootTree = 0; /*this register is used for root interrupt tree of 16 raven,
                                            each raven is 1 bit using bits [1:16]*/
    GT_U8       smiDrvIndex = 0x1f;

    /* CPSS_HW_DRIVER_AS_CNM_E is used to enable access to the configuration */
    /* space for debug purpose with Internal PCI APIs. */
    if( addrSpace == CPSS_HW_DRIVER_AS_PCI_CONFIG_E ||
        addrSpace == CPSS_HW_DRIVER_AS_CNM_E)
    {
        /* Configuration Space access */
        *dataPtr = 0;
        return GT_OK;
    }
    else if ( addrSpace == CPSS_HW_DRIVER_AS_ATU_E )
    {
        return(drv->bar0->read(drv->bar0, 0, regAddr, dataPtr, count));
    }
    else if ( addrSpace == CPSS_HW_DRIVER_AS_DFX_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

#ifndef ASIC_SIMULATION
    if( (regAddr % TILE_OFFSET_OFFSET) < RAVENS_MEMORY_SPACE_PER_TILE )
    {
        /* One of the Ravens range */
        smiDriverIndexGet(regAddr, &smiDrvIndex);
        falconZSmiMuxConfig(smiDrvIndex);
    }
#endif

    for (; count; count--,dataPtr++)
    {
        if( (regAddr % TILE_OFFSET_OFFSET) >= RAVENS_MEMORY_SPACE_PER_TILE )
        {
            /* Not one of the Ravens range */
            GT_U32 ii;

            data = 0;

            /* patch address in order to read the interrupt tree in falconZ2*/
            if (regAddr == 0x1d000100)
            {
                /* need to check all raven and set bits in rootTree*/
                for(smiDrvIndex = 0; smiDrvIndex < NUM_OF_RAVENS; smiDrvIndex++)
                {
                    data = 0;

#ifdef ASIC_SIMULATION
                    smiDriverBaseAddressGet(smiDrvIndex,&regAddr);
                    regAddr += 0x300030;
                    scibReadMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
                    falconZSmiMuxConfig(smiDrvIndex);
                    falconZ[smiDrvIndex]->read(falconZ[smiDrvIndex], addrSpace, 0x300030, &data, 1);
#endif
                    if (data & 0x1)
                    {
                        falconZ2IntRootTree |= 1<<(1+smiDrvIndex);
                    }
                }
                data = falconZ2IntRootTree;
            }
            else
            {
                /* Check if is one of the out-of Ravens range registers with value different than 0 (zero) */
                for( ii = 0 ; ii < sizeof(falconZdefaultRegs)/sizeof(FALCON_Z_DEFAULT_REG_STC) ; ii++ )
                {
                    if( falconZdefaultRegs[ii].regAddr == regAddr )
                    {
                        data = CPSS_32BIT_LE(falconZdefaultRegs[ii].regData);
                        break;
                    }
                }
            }

        } else {
#ifdef ASIC_SIMULATION
            scibReadMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
            falconZ[smiDrvIndex]->read(falconZ[smiDrvIndex], SSMI_FALCON_ADDRESS_SPACE, regAddr%RAVEN_MEMORY_SPACE_SIZE, &data, 1);

#endif
        }
        GT_SYNC; /* to avoid read combining */
        *dataPtr = CPSS_32BIT_LE(data);
        regAddr += 4;

    }

    return GT_OK;
}

static GT_STATUS cpssHwDriverFalconZRead(
    IN  CPSS_HW_DRIVER_FALCON_Z_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_STATUS rc;

    cpssOsMutexLock(drv->mtx);
    rc = cpssHwDriverFalconZReadNoMtx(drv, addrSpace, regAddr, dataPtr, count);
    cpssOsMutexUnlock(drv->mtx);

    return rc;
}

static GT_STATUS cpssHwDriverFalconZWriteMaskNoMtx(
    IN  CPSS_HW_DRIVER_FALCON_Z_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32      data;
#ifndef ASIC_SIMULATION
    GT_U8 smiDrvIndex;
#endif

    /* CPSS_HW_DRIVER_AS_CNM_E is used to enable access to the configuration */
    /* space for debug purpose with Internal PCI APIs. */
    if( addrSpace == CPSS_HW_DRIVER_AS_PCI_CONFIG_E ||
        addrSpace == CPSS_HW_DRIVER_AS_CNM_E)
    {
        /* Configuration Space access */
        return GT_OK;
    }
    else if ( addrSpace == CPSS_HW_DRIVER_AS_ATU_E )
    {
        return(drv->bar0->writeMask(drv->bar0, 0, regAddr, dataPtr, count, mask));
    }

    if( (regAddr % TILE_OFFSET_OFFSET) >= RAVENS_MEMORY_SPACE_PER_TILE )
    {
        /* Not one of the Ravens range */
        return GT_OK;
    }

#ifndef ASIC_SIMULATION
    /* One of the Ravens range */
    smiDriverIndexGet(regAddr, &smiDrvIndex);
    falconZSmiMuxConfig(smiDrvIndex);
#endif

    for (; count; count--,dataPtr++)
    {
        if (0)
        {
#ifdef ASIC_SIMULATION
            scibReadMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
            falconZ[smiDrvIndex]->read(falconZ[smiDrvIndex], SSMI_FALCON_ADDRESS_SPACE, regAddr%RAVEN_MEMORY_SPACE_SIZE, &data, 1);

            GT_SYNC; /* to avoid read combining */
#endif
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
#ifdef ASIC_SIMULATION
        scibReadMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
        data &= ~mask;
        data |= ((*dataPtr) & mask);
        scibWriteMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
        falconZ[smiDrvIndex]->writeMask(falconZ[smiDrvIndex], SSMI_FALCON_ADDRESS_SPACE, regAddr%RAVEN_MEMORY_SPACE_SIZE, &data, 1, mask);
#endif
        GT_SYNC; /* to avoid from write combining */
        regAddr += 4;
    }

    return GT_OK;
}

static GT_STATUS cpssHwDriverFalconZWriteMask(
    IN  CPSS_HW_DRIVER_FALCON_Z_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_STATUS rc;

    cpssOsMutexLock(drv->mtx);
    rc = cpssHwDriverFalconZWriteMaskNoMtx(drv, addrSpace, regAddr, dataPtr, count, mask);
    cpssOsMutexUnlock(drv->mtx);

    return rc;
}

static GT_STATUS cpssHwDriverFalconZDestroyDrv(
    IN  CPSS_HW_DRIVER_FALCON_Z_STC *drv
)
{
    drv->bar0->destroy(drv->bar0);
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverFalconZCreateDrv function
* @endinternal
*
* @brief Create driver instance for falconZ
*
* @param[in] base           - resource virtual address
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note
*/
CPSS_HW_DRIVER_STC *cpssHwDriverFalconZCreateDrv(
    IN  CPSS_HW_DRIVER_STC *bar0,
    IN  CPSS_HW_INFO_STC *hwInfo
)
{
    CPSS_HW_DRIVER_FALCON_Z_STC *drv;
    char buf[64];

    if (!hwInfo->resource.switching.start)
        return NULL;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverFalconZRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverFalconZWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverFalconZDestroyDrv;
    drv->common.type = CPSS_HW_DRIVER_TYPE_FALCON_Z_E;

    drv->bar0 = bar0;
    drv->base = hwInfo->resource.switching.start;

    cpssOsSprintf(buf, "falconZ%p", (void*)hwInfo->resource.switching.start);
    cpssOsMutexCreate(buf, &(drv->mtx));

    return (CPSS_HW_DRIVER_STC*)drv;
}

static GT_STATUS cpssHwDriverFalconZPexBar0Read(
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

static GT_STATUS cpssHwDriverFalconZPexBar0WriteMask(
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

static GT_STATUS cpssHwDriverFalconZPexBar0Destroy(
    IN  CPSS_HW_DRIVER_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverFalconZPexBar0CreateDrv function
* @endinternal
*
* @brief Create driver instance for simulated BAR0 access -
*        actually a NOP driver
*
* @param[in] hwInfo         - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverFalconZPexBar0CreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo GT_UNUSED
)
{
    CPSS_HW_DRIVER_STC *drv;

#ifdef _VISUALC
    (void)hwInfo;
#endif

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverFalconZPexBar0Read;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverFalconZPexBar0WriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverFalconZPexBar0Destroy;

    return drv;
}

CPSS_HW_DRIVER_STC *drv4debug;

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
CPSS_HW_DRIVER_STC *cpssHwDriverFalconZPexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_HW_DRIVER_STC *bar0;

    bar0 = cpssHwDriverFalconZPexBar0CreateDrv(hwInfo);

#ifndef ASIC_SIMULATION
    if ( GT_OK != falconZCreateSmiDrivers() )
    {
        return NULL;
    }
#endif

    drv4debug = cpssHwDriverFalconZCreateDrv(bar0, hwInfo);
    return drv4debug;
}


/************************/
/*     Debug APIs       */
/************************/

/* Debug API to access FALCONZ driver directly for read opeartion. */
/* regAddr - address within the whole falcon range.                */
GT_STATUS falconZregread(GT_U8 devNum, GT_U32 regAddr)
{
    GT_U32 data;
    CPSS_HW_DRIVER_STC *drv;
    /*char drvName[32];*/
    GT_STATUS rc;

    devNum = devNum;

    /*cpssOsSprintf(drvName, "/pp%d", devNum);
    drv = cpssHwDriverLookup(drvName);*/
    drv = drv4debug;
    if( NULL == drv )
    {
        cpssOsPrintf("FALCONZ driver lookup returned NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = cpssHwDriverFalconZRead((CPSS_HW_DRIVER_FALCON_Z_STC*)drv, CPSS_HW_DRIVER_AS_SWITCHING_E, regAddr, &data, 1);
    if( GT_OK == rc )
    {
            cpssOsPrintf("Reg:0x%x, data=0x%x\n", regAddr, data);
    }

    return rc;
}

/* Debug API to access FALCONZ driver directly for write opeartion. */
/* regAddr - address within the whole falcon range.                 */
GT_STATUS falconZregwrite(GT_U8 devNum, GT_U32 regAddr, GT_U32 data)
{
    CPSS_HW_DRIVER_STC *drv;
    /*char drvName[32];*/
    GT_STATUS rc;

    devNum = devNum;

    /*cpssOsSprintf(drvName, "/pp%d", devNum);
    drv = cpssHwDriverLookup(drvName);*/
    drv = drv4debug;
    if( NULL == drv )
    {
        cpssOsPrintf("FALCONZ driver lookup returned NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = cpssHwDriverFalconZWriteMask((CPSS_HW_DRIVER_FALCON_Z_STC*)drv, CPSS_HW_DRIVER_AS_SWITCHING_E, regAddr, &data, 1, 0xffffffff);
    if( GT_OK == rc )
    {
            cpssOsPrintf("Reg:0x%x, data=0x%x\n", regAddr, data);
    }

    return rc;
}

#ifndef ASIC_SIMULATION
/* Debug API to access FALCONZ SMI driver directly for read opeartion. */
/* smiDrvIndex - SMI device index for SMI MUX configuration.           */
/* addrSpace   - used as the selected SMI XBAR number.                 */
/* regAddr     - address within the Raven SMI range.                   */
GT_STATUS directSmiregread(GT_U8 devNum, GT_U8 smiDrvIndex, GT_U32 addrSpace, GT_U32 regAddr)
{
    GT_U32 data;
    GT_STATUS rc;

    GT_UNUSED_PARAM(devNum);

    falconZSmiMuxConfig(smiDrvIndex);

    rc = falconZ[smiDrvIndex]->read(falconZ[smiDrvIndex], addrSpace/*SSMI_FALCON_ADDRESS_SPACE*/, regAddr, &data, 1);
    if( GT_OK == rc )
    {
            cpssOsPrintf("Reg:0x%x, data=0x%x\n", regAddr, data);
    }

    return rc;
}

/* Debug API to access FALCONZ SMI driver directly for write opeartion. */
/* smiDrvIndex - SMI device index for SMI MUX configuration.            */
/* addrSpace   - used as the selected SMI XBAR number.                  */
/* regAddr     - address within the Raven SMI range.                    */
GT_STATUS directSmiregwrite(GT_U8 devNum, GT_U8 smiDrvIndex, GT_U32 regAddr, GT_U32 data)
{
    GT_STATUS rc;

    GT_UNUSED_PARAM(devNum);

    falconZSmiMuxConfig(smiDrvIndex);

    rc = falconZ[smiDrvIndex]->writeMask(falconZ[smiDrvIndex], SSMI_FALCON_ADDRESS_SPACE, regAddr, &data, 1, 0xffffffff);
    if( GT_OK == rc )
    {
            cpssOsPrintf("Reg:0x%x, data=0x%x\n", regAddr, data);
    }

    return rc;
}
#endif
#define    XSMI_MANAGEMENT 0x330000
#define    XSMI_ADDRESS    0x330008
/**
* @internal falconXsmiRead function
* @endinternal
*
* @brief   read data from external phy's register using xsmi
* @param[in] devNum                - system device number
* @param[in] xsmiMasterRavenIndex  - xsmi bus master raven index
* @param[in] phyAddress            - phy's port number
* @param[in] devAddress            - phy's lane offset
* @param[in] regAddr               - phy's register address
* @param[out] dataPtr              - (ptr to) read value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS falconZXsmiReadNoMtx
(
    GT_U8  devNum,
    GT_U8  xsmiMasterRavenIndex,
    GT_U16 phyAddress,
    GT_U16 devAddress,
    GT_U16 regAddr,
    GT_U16 *dataPtr
)
{

    GT_U32 timeout = 1000;
    GT_U32 readValue, writeValue, reg_offset;
    GT_STATUS rc;

    GT_U32 baseAddr;
    devNum = devNum; /* avoid warning */
    rc = smiDriverBaseAddressGet(xsmiMasterRavenIndex,&baseAddr);
    if (GT_OK != rc)
    {
        return rc;
    }
    while(timeout > 0)
    {
        rc = cpssHwDriverFalconZRead((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_MANAGEMENT, &readValue, 1);
        if (GT_OK != rc)
        {
            return rc;
        }
        if((readValue&(1<<30)) == 0)
        {
            break;
        }
        cpssOsTimerWkAfter(1);
        timeout = timeout - 1;
    }
    if(timeout <= 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    reg_offset = (GT_U32)regAddr;
    writeValue = (0x7 << 26) | ((GT_U32)devAddress << 21) | ((GT_U32)phyAddress << 16) | 0;
    rc = cpssHwDriverFalconZWriteMask((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_ADDRESS, &reg_offset, 1, 0xffffffff);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = cpssHwDriverFalconZWriteMask((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_MANAGEMENT, &writeValue, 1, 0xffffffff);
    if (GT_OK != rc)
    {
        return rc;
    }

    timeout = 1000;
    while(timeout > 0)
    {
        rc = cpssHwDriverFalconZRead((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_MANAGEMENT, &readValue, 1);
        if((readValue&(1<<29)) != 0)
        {
            break;
        }
        cpssOsTimerWkAfter(1);
        timeout = timeout - 1;
    }
    if(timeout <= 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }
    rc = cpssHwDriverFalconZRead((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_MANAGEMENT, &readValue, 1);
    if (GT_OK != rc)
    {
        return rc;
    }
    *dataPtr = (GT_U16)(readValue & 0xffff);
    /*cpssOsPrintf("Read: %#06x, phyAddress:%#06x, devAddress:%#06x, regAddr:%#06x\n",*dataPtr,phyAddress,devAddress,regAddr);*/
    return GT_OK;
}

/**
* @internal falconXsmiWrite function
* @endinternal
*
* @brief   write data to external phy's register using xsmi
* @param[in] devNum                - system device number
* @param[in] xsmiMasterRavenIndex  - xsmi bus master raven index
* @param[in] phyAddress            - phy's port number
* @param[in] devAddress            - phy's lane offset
* @param[in] regAddr               - phy's register address
* @param[in] data                  - value to write
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS falconZXsmiWriteNoMtx
(
    GT_U8  devNum,
    GT_U8  xsmiMasterRavenIndex,
    GT_U16 phyAddress,
    GT_U16 devAddress,
    GT_U16 regAddr,
    GT_U16 data
)
{

    GT_U32 timeout = 1000;
    GT_U32 readValue;
    GT_U32 writeValue;
    GT_STATUS rc;

    GT_U32 baseAddr, address;
    devNum = devNum; /* avoid warning */

    rc = smiDriverBaseAddressGet(xsmiMasterRavenIndex,&baseAddr);
    if (GT_OK != rc)
    {
        return rc;
    }
    while(timeout > 0)
    {
        rc = cpssHwDriverFalconZRead((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_MANAGEMENT, &readValue, 1);
        if (GT_OK != rc)
        {
            return rc;
        }
        if((readValue&(1<<30)) == 0)
        {
            break;
        }
        cpssOsTimerWkAfter(1);
        timeout = timeout - 1;
    }
    if(timeout <= 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    address = regAddr;
    writeValue = (0x5 << 26) | ((GT_U32)devAddress << 21) | ((GT_U32)phyAddress << 16) | (GT_U32)data;
    rc = cpssHwDriverFalconZWriteMask((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_ADDRESS, &address, 1, 0xffffffff);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = cpssHwDriverFalconZWriteMask((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug, CPSS_HW_DRIVER_AS_SWITCHING_E, baseAddr + XSMI_MANAGEMENT, &writeValue, 1, 0xffffffff);
    if (GT_OK != rc)
    {
        return rc;
    }
    /*cpssOsPrintf("Write:%#06x, phyAddress:%#06x, devAddress:%#06x, regAddr:%#06x\n",data,phyAddress,devAddress,regAddr);*/
    return GT_OK;
}

GT_STATUS falconZXsmiRead
(
    GT_U8  devNum,
    GT_U8  xsmiMasterRavenIndex,
    GT_U16 phyAddress,
    GT_U16 devAddress,
    GT_U16 regAddr,
    GT_U16 *dataPtr
)
{
    GT_STATUS rc;
    cpssOsMutexLock(((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug)->mtx);
    rc = falconZXsmiReadNoMtx(devNum,xsmiMasterRavenIndex,phyAddress,devAddress,regAddr,dataPtr);
    cpssOsMutexUnlock(((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug)->mtx);

    return rc;
}

GT_STATUS falconZXsmiWrite
(
    GT_U8  devNum,
    GT_U8  xsmiMasterRavenIndex,
    GT_U16 phyAddress,
    GT_U16 devAddress,
    GT_U16 regAddr,
    GT_U16 data
)
{
    GT_STATUS rc;
    cpssOsMutexLock(((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug)->mtx);
    rc = falconZXsmiWriteNoMtx(devNum,xsmiMasterRavenIndex,phyAddress,devAddress,regAddr,data);
    cpssOsMutexUnlock(((CPSS_HW_DRIVER_FALCON_Z_STC*)drv4debug)->mtx);

    return rc;
}
