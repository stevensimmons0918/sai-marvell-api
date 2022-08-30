/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file smemSoho.c
*
* @brief This is API implementation for Soho memories.
*
* @version   32
********************************************************************************
*/
#include <asicSimulation/SKernel/smem/smemSoho.h>
#include <asicSimulation/SKernel/sohoCommon/sregSoho.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetSohoEgress.h>
#include <asicSimulation/SLog/simLog.h>

static void smemSohoInitFuncArray(
    INOUT SOHO_DEV_MEM_INFO  * devMemInfoPtr
);

static void smemSohoAllocSpecMemory(
    INOUT SOHO_DEV_MEM_INFO  * devMemInfoPtr,
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

static void smemOpalAddStuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 * vtuWordPtr
);

static GT_STATUS smemOpalGetNextStuEntry (
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 sid,
    OUT GT_U32 * stuWordPtr
);

static void smemSohoAddVtuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 * vtuWordPtr
);

static void smemSohoDeleteVtuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 vid
);

static GT_STATUS smemSohoGetNextVtuEntry (
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 vid,
    OUT GT_U32 * vtuWordPtr
);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemChtActiveWriteInterruptsMaskReg);

static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteStatsOp);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteVtuOp);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteAtuOp);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteTrunkMask);
static ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemSohoActiveReadTrunkMask);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteTrunkRout);
static ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemSohoActiveReadTrunkRout);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteTrgDevice);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteSwitchMac);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteFlowCtrlDelay);
static ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemSohoActiveReadGlobalStat);
static ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemSohoActiveReadPhyInterruptStat);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWritePhyInterruptMask);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteGlobal2InterruptMask);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteGobalStatus);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteForceLinkDown);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteForceLink);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteGlobal2SmiPhyCommnd);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteMultiChipAddressingControlReg);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteFlowControl);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteIeeePriorityMappingTable);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteIMPCommDebug);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteLedControl);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteTCAMCommon);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteQosWeight);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteEnergyMngmt);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteScratchMisc);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteGlobalControl);
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSohoActiveWriteHiddenRegister);

typedef enum {
    SOHO_SMI_SINGLE_ADDR_MODE = 0,
    SOHO_SMI_MULTI_ADDR_MODE
} SOHO_REG_ACCESS_E;

/* Statistic Unit Opcodes */
typedef enum {
    NO_OPERATION = 0,
    FLUSH_ALL_CNT_ALL_E,
    FLUSH_ALL_CNT_FOR_PORT_E,
    RESERVED,
    READ_CAPTURED_CNT_E,
    CAPTURE_ALL_CNT_FOR_PORT_E
} SOHO_STAT_OP_E;

/* VTU Opcodes */
typedef enum {
    VTU_NO_OPERATION_E = 0,
    VTU_FLUSH_ALL_VTU_ALL_E = 1,
    VTU_LOAD_PURGE_ENTRY_E = 3,
    VTU_GET_NEXT_E = 4,
    STU_LOAD_PURGE_E = 5,
    STU_GET_NEXT_E = 6,
    VTU_GET_CLEAR_VIOLATION_E = 7
} SOHO_VTU_OP_E;

/* Stats operation counter number */
#define SOHO_RMON_CNT_NUM                   (0x26)
/* the Opal use registers 0xA...0xF */
/* order of registers :
    0xB ,0xC,0xD,0xE,0xF
    0xA
*/
/* the Opal+ use also register 1 --> put register 0x1 to be after 0xA*/
/* order of registers :
    0xB ,0xC,0xD,0xE,0xF
    0xA , 0x1
*/
#define SMEM_SOHO_ATU_MSG_WORDS_NUM         (7)
#define SMEM_SOHO_LINK_FORCE_MSG_SIZE        (0x5)

#define SOHO_PER_PORT_MASK  0xFFF0FFFF
/* Active memory table */
static SMEM_ACTIVE_MEM_ENTRY_STC smemSohoActiveTable[] =
{
    /* Stats operation register */
    {GLB_STAT_OPER_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteStatsOp, 0},
    /* VTU operation register */
    {GLB_VTU_OPER_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteVtuOp, 0},
    /* ATU operation register */
    {GLB_ATU_OPER_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteAtuOp, 0},
    /* Switch Global Status Register */
    {GLB_STATUS_REG, SMEM_FULL_MASK_CNS, smemSohoActiveReadGlobalStat, 0, NULL, 0},
    /* PHY Interrupt Status Register */
    {PHY_INTERRUPT_STATUS_REG(0), SOHO_PER_PORT_MASK, smemSohoActiveReadPhyInterruptStat, 0, NULL, 0},
    /* PHY Interrupt MASK Register */
    {PHY_INTERRUPT_ENABLE_REG(0), SOHO_PER_PORT_MASK, NULL, 0, smemSohoActiveWritePhyInterruptMask , 0},
    /* Flow Control Delay Memory */
    {GLB2_FLOW_CTRL_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteFlowCtrlDelay, 0},
    /* Target device Memory */
    {GLB2_ROUT_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteTrgDevice, 0},
    /* Trunk Mask Memory */
    {GLB2_TRUNK_MASK_REG, SMEM_FULL_MASK_CNS, smemSohoActiveReadTrunkMask, 0 , smemSohoActiveWriteTrunkMask, 0},
    /* Trunk Route Memory */
    {GLB2_TRUNK_ROUT_REG, SMEM_FULL_MASK_CNS, smemSohoActiveReadTrunkRout, 0 , smemSohoActiveWriteTrunkRout, 0},
    /* Switch MAC / WoL / WoF Memory */
    {GLB2_SWITCH_MAC_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteSwitchMac, 0},
    /* Switch Global Control Register */
    {GLB_CTRL_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteGobalStatus, 0},
    /* PCS Port Control register */
    /* PORT_PCS_REG */
    {PER_PORT_LEGACY_BASE(0,0x1), SOHO_PER_PORT_MASK, NULL, 0 , smemSohoActiveWriteForceLinkDown, 0},
    /* must be last anyway */
    {SMAIN_NOT_VALID_CNS, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}
};

#define SMEM_ACTIVE_MEM_TABLE_SIZE \
    (sizeof(smemSohoActiveTable)/sizeof(smemSohoActiveTable[0]))

/* additional Peridot Active memory table */
static SMEM_ACTIVE_MEM_ENTRY_STC smemSohoActiveTable_peridot[] =
{
    /* PCS Port Control register */
    /* PORT_PCS_REG */
    {PER_PORT_PERIDOT_BASE(0,0x1), SOHO_PER_PORT_MASK, NULL, 0 , smemSohoActiveWriteForceLink, 0},
    /* Global2 : SMI PHY command for SMI Access */
    {GLB2_SMI_PHY_COMMND_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteGlobal2SmiPhyCommnd, 0},
    /* Global2 : PHY Interrupt MASK Register */
    {GLB2_INTERUPT_MASK     , SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteGlobal2InterruptMask, 0},
    /* Flow Control register */
    {PER_PORT_PERIDOT_BASE(0,0x2), SOHO_PER_PORT_MASK, NULL, 0 , smemSohoActiveWriteFlowControl, 0},
    /* CPU Port - Flow Control register */
    {CPU_PORT_FLOW_CONTROL_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteFlowControl, 0},
    /* Global2 : IMP Comm/Debug Register */
    {GLB2_IMP_COMM_DEBUG_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteIMPCommDebug, 0},
    /* LED Control register */
    {PER_PORT_PERIDOT_BASE(0,0x16), SOHO_PER_PORT_MASK, NULL, 0 , smemSohoActiveWriteLedControl, 0},
    /* IEEE Priority Mapping Table register */
    {PER_PORT_PERIDOT_BASE(0,0x18), SOHO_PER_PORT_MASK, NULL, 0 , smemSohoActiveWriteIeeePriorityMappingTable, 0},
    /* Global2 : Qos Weight Register */
    {GLB2_QOS_WEIGHT_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteQosWeight, 0},
    /* Global2 : Energy Management Register */
    {GLB2_ENERGY_MNGMT_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteEnergyMngmt, 0},
    /* Global2 : Scratch and Miscellaneous Register */
    {GLB2_SCRATCH_MISC_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteScratchMisc, 0},
    /* Queue Control register */
    {PER_PORT_PERIDOT_BASE(0,0x1c), SOHO_PER_PORT_MASK, NULL, 0 , smemSohoActiveWriteQosWeight, 0},
    /* CPU Port - Queue Control register */
    {CPU_PORT_Q_CTRL_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteQosWeight, 0},
    /* TCAM : Common Register */
    {TCAM_COMMON_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteTCAMCommon, 0},
    /* Global1 : Global Control Register */
    {GLB_CTRL_REG, SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteGlobalControl, 0},
    /* Hidden Register */
    {PER_PORT_PERIDOT_BASE(4,0x1a), SMEM_FULL_MASK_CNS, NULL, 0 , smemSohoActiveWriteHiddenRegister, 0},

    /* must be last anyway */
    {SMAIN_NOT_VALID_CNS, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}
};

/* additional multi-chip addressing Active memory table */
static SMEM_ACTIVE_MEM_ENTRY_STC smemSohoActiveTable_multiChipAddressingMode[] =
{
    /* Multi-Chip Addressing Control Register */
    {0x00000000, 0xFFFFFFFF, NULL, 0 , smemSohoActiveWriteMultiChipAddressingControlReg, 0},
    /* must be last anyway */
    {SMAIN_NOT_VALID_CNS, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}
};

#define SCAN_ACTIVE_MEM_TABLE(active_mem_tbl,_address,_activeMemPtrPtr) \
    {                                                                   \
        GT_U32  index;                                                  \
        for (index = 0; active_mem_tbl[index].address != SMAIN_NOT_VALID_CNS; index++) \
        {                                                               \
            /* check address */                                         \
            if ((_address & active_mem_tbl[index].mask)                 \
                 == active_mem_tbl[index].address)                      \
            {                                                           \
                *_activeMemPtrPtr = &active_mem_tbl[index];             \
                break;                                                  \
            }                                                           \
        }                                                               \
    }



/**
* @internal smemSohoInit function
* @endinternal
*
* @brief   Init memory module for a Twist device.
*/
void smemSohoInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr; /* device's memory pointer */
    /* string for parameter */
    char   param_str[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    GT_U32 param_val; /* parameter value */

    devObjPtr->sohoInMultiChipAddressingMode = devObjPtr->deviceHwId ? 1 : 0;
    devObjPtr->supportAnyAddress = 1;/* the register address is like in PHY 1
                    based jumps (not 0x4 like in DXCH) */

    devObjPtr->notSupportPciConfigMemory = 1;/* the device not support PCI/PEX configuration memory space */

    /* alloc SOHO_DEV_MEM_INFO */
    devMemInfoPtr = (SOHO_DEV_MEM_INFO *)calloc(1, sizeof(SOHO_DEV_MEM_INFO));
    if (devMemInfoPtr == 0)
    {
            skernelFatalError("smemSohoInit: allocation error\n");
    }

    if (!SIM_OS_MAC(simOsGetCnfValue)("system",  "access_mode",
                             SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        /* Default access mode single-chip addressing mode */
        devMemInfoPtr->accessMode = SOHO_SMI_SINGLE_ADDR_MODE;
    }
    else
    {
        sscanf(param_str, "%u", &param_val);
        devMemInfoPtr->accessMode = param_val;
    }

    /* init specific functions array */
    smemSohoInitFuncArray(devMemInfoPtr);

    /* allocate address type specific memories */
    smemSohoAllocSpecMemory(devMemInfoPtr,devObjPtr);

    devObjPtr->devFindMemFunPtr = (void *)smemSohoFindMem;
    devObjPtr->deviceMemory = devMemInfoPtr;

    if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        GT_U32  ii;
        /* init the internal table with default values */
        for(ii = 0 ; ii < 8;ii++)
        {
            devMemInfoPtr->trunkMaskMem.trunkTblMem[ii] = 0x7FF;
        }
    }

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
    {
        devObjPtr->soho_ports_base_addr = PER_PORT_PERIDOT_BASE(0,0);
    }
    else
    {
        devObjPtr->soho_ports_base_addr = PER_PORT_LEGACY_BASE(0,0);
    }

    /* set the SMI address as the <deviceNumber> */
    smemRegFldSet(devObjPtr,GLB_CTRL_2_REG,0,5,devObjPtr->deviceHwId ? 1 : 0);

}
/*******************************************************************************
*   smemSohoFindMem
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*
* OUTPUTS:
*     activeMemPtrPtr - pointer to the active memory entry or NULL if not exist.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
void * smemSohoFindMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr
)
{
    void              * memPtr;         /* registers's memory pointer */
    SOHO_DEV_MEM_INFO * devMemInfoPtr;  /* device's memory pointer */
    GT_U32              index;          /* register's memory offset */
    GT_U32              param;          /* additional parameter */
    GT_BIT              sohoInMultiChipAddressingMode;

    memPtr = 0;
    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    if((!IS_SKERNEL_OPERATION_MAC(accessType)) &&
       simulationInitReady == 1 &&
       devObjPtr->sohoInMultiChipAddressingMode &&
       devMemInfoPtr->devRegs.multiChipScibMode ==
        SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E)
    {
        sohoInMultiChipAddressingMode = 1;
    }
    else
    {
        sohoInMultiChipAddressingMode = 0;
    }

    if(sohoInMultiChipAddressingMode)
    {
        /* The initialization from 'REG file' is over. (simulation of EEprom)
           And the device is in 'Multi Chip Addressing' Mode*/

        /* there are only 2 registers that the device apply to :
            address     : 0
            and address : 1
        */

        if(address != 0 /* control register */ &&
           address != 1 /* data register */)
        {
            skernelFatalError("smemSohoFindMem: 'Multi Chip Addressing' Mode , register address [0x%x] is not valid (only addresses [0,1] are valid) \n",
                address);
        }
    }

    index = (address >> 28) & 0xf;
    /* Call register spec function to obtain pointer to register memory */
    param   = devMemInfoPtr->specFunTbl[index].specParam;
    memPtr  = devMemInfoPtr->specFunTbl[index].specFun(devObjPtr,
                                                       accessType,
                                                       address,
                                                       memSize,
                                                       param);

    /* find active memory entry */
    if (activeMemPtrPtr != NULL)
    {
        *activeMemPtrPtr = NULL;

        if(sohoInMultiChipAddressingMode)
        {
            SCAN_ACTIVE_MEM_TABLE(smemSohoActiveTable_multiChipAddressingMode,
                address,activeMemPtrPtr);
        }
        else
        {
            if(*activeMemPtrPtr == NULL &&
               SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
            {
                SCAN_ACTIVE_MEM_TABLE(smemSohoActiveTable_peridot,
                    address,activeMemPtrPtr);
            }

            if(*activeMemPtrPtr == NULL)
            {
                SCAN_ACTIVE_MEM_TABLE(smemSohoActiveTable,
                    address,activeMemPtrPtr);
            }
        }
    }

    return memPtr;
}

/*******************************************************************************
*   smemSohoGlobalReg
*
* DESCRIPTION:
*       Global configuration Registers.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoGlobalReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr; /* device's memory pointer */
    GT_U32               index;         /* register's memory offset */
    SMEM_REGISTER      * globalRegsPtr; /* global register's pointer */

    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    if((!IS_SKERNEL_OPERATION_MAC(accessType)) &&
       simulationInitReady == 1 &&
       devObjPtr->sohoInMultiChipAddressingMode &&
       devMemInfoPtr->devRegs.multiChipScibMode ==
        SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E)
    {
        index = address;/* 0 or 1 */
        CHECK_MEM_BOUNDS(devMemInfoPtr->devRegs.multiChipModeRegisters,
                         2,
                         index, memSize);
        return &devMemInfoPtr->devRegs.multiChipModeRegisters[index];
    }

    globalRegsPtr = NULL;

    switch(address & 0x001F0000)
    {
        case GLOBAL1_BASE_ADDR_REG(0):
            globalRegsPtr = devMemInfoPtr->devRegs.globalRegs;
            break;
        case GLOBAL2_BASE_ADDR_REG(0):
            globalRegsPtr = devMemInfoPtr->devRegs.global2Regs;
            break;
        case GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0):
            globalRegsPtr = devMemInfoPtr->devRegs.globalInternalCpuRegs;
            break;
        case GLOBAL_TCAM_BASE_ADDR_REG(0):
            globalRegsPtr = devMemInfoPtr->devRegs.globalTcamRegs;
            break;
        default:
            break;
    }

    if(globalRegsPtr == NULL)
    {
        /* fatal error by the caller */
        return NULL;
    }


    /* Global registers */
    index = (address & 0xffff) >> 4;
    CHECK_MEM_BOUNDS(globalRegsPtr,
                     SOHO_REG_NUMBER,
                     index, memSize);

    return &globalRegsPtr[index];
}

/*******************************************************************************
*   smemSohoPhyReg
*
* DESCRIPTION:
*       Device PHY's registers access.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoPhyReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr;     /* device's memory pointer */
    GT_U32               index, smiPort;    /* device and register offsets */
    GT_U32              * regValPtr;        /* pointer to register's value */

    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    regValPtr = 0;

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType) &&
        ! IS_SKERNEL_OPERATION_MAC(accessType) &&
        devMemInfoPtr->devRegs.multiChipScibMode ==
            SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E)
    {
        /* this memory space is not accessible directly from the CPU */
        skernelFatalError("smemSohoPhyReg: this memory space is not accessible directly from the CPU \n");
    }

    /* Retrieve port address */
    smiPort = (address >> 16) & 0xff;

    /* force Phy Port Summary register to be common for all ports */
    if ((address & 0xFF00FFFF) == PHY_INTERRUPT_PORT_SUM_REG(0))
    {
        smiPort = 0 ;
    }

    if (smiPort >= (SOHO_PHY_REGS_START_ADDR + SOHO_PORTS_NUMBER))
    {
        skernelFatalError("smemSohoPhyReg: index or memory size is out of range\n");
    }

    index = (address & 0xffff) >> 4;
    CHECK_MEM_BOUNDS(devMemInfoPtr->devRegs.phyRegs[smiPort],
                     devMemInfoPtr->devRegs.phyRegsNum,
                     index, memSize);

    regValPtr = &devMemInfoPtr->devRegs.phyRegs[smiPort][index];

    return regValPtr;
}


/*******************************************************************************
*   smemSohoPortsReg
*
* DESCRIPTION:
*       Device ports registers access.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoPortsReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr;     /* device's memory pointer */
    GT_U32               index, port;    /* device and register offsets */
    GT_U32              * regValPtr;        /* pointer to register's value */

    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    regValPtr = 0;

    /* Retrieve port address */
    port = (address >> 16) & 0xff;

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
    {
        if (port >= SOHO_PORTS_NUMBER)
        {
            skernelFatalError("smemSohoPortsReg: index or memory size is out of range\n");
        }
    }
    else
    {
        /* Align SMI device address to zero based index */
        if (port >= (SOHO_PORT_REGS_START_ADDR + SOHO_PORTS_NUMBER))
        {
            skernelFatalError("smemSohoPortsReg: index or memory size is out of range\n");
        }
    }
    port %= SOHO_PORTS_NUMBER;
    index = (address & 0xffff) >> 4;
    CHECK_MEM_BOUNDS(devMemInfoPtr->devRegs.portRegs[port],
                     devMemInfoPtr->devRegs.portRegsNum,
                     index, memSize);

    regValPtr = &devMemInfoPtr->devRegs.portRegs[port][index];

    return regValPtr;
}


/*******************************************************************************
*   smemSohoMacTableReg
*
* DESCRIPTION:
*       MAC table memory access
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoMacTableReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO   * devMemInfoPtr;    /* device's memory pointer */
    GT_U32              * regValPtr;        /* pointer to register's value */
    GT_U32              index;/* index of word in memory */

    regValPtr = 0;
    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    index =  (address>>2) & 0xffff;
    index += ((address>>1)& 0x1)?1:0;

    if (index >= devMemInfoPtr->macDbMem.macTblMemSize)
    {
        skernelFatalError("Wrong MAC table address %X, exceed maximal size %X",
                          address,
                          devMemInfoPtr->macDbMem.macTblMemSize);
    }
    regValPtr = &devMemInfoPtr->macDbMem.macTblMem[index];

    return regValPtr;
}

/*******************************************************************************
*   smemSohoStatsCountReg
*
* DESCRIPTION:
*       Stats counters memory access
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoStatsCountReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO   * devMemInfoPtr;    /* device's memory pointer */
    GT_U32              * regValPtr;        /* pointer to register's value */
    GT_U32               index, smiPort;    /* device and register offsets */
    GT_U32              StatsBank;

    regValPtr = 0;
    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Retrieve port address */
    smiPort   = ((address >> 16) & 0x0f);
    StatsBank = ((address >> 20) & 0x0f) - 1;
    /* Allign SMI device address to zero based index */
    if (smiPort >= (SOHO_PORTS_NUMBER))
    {
        skernelFatalError("smemSohoStatsCountReg: index or memory size is out of range\n");
    }

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType) && StatsBank > 1)
    {
        skernelFatalError("smemSohoStatsCountReg: StatsBank[%d] is not supported in the device \n",
            StatsBank);
    }
    else
    if(StatsBank != 0)
    {
        skernelFatalError("smemSohoStatsCountReg: StatsBank[%d] is not supported in the device \n",
            StatsBank);
    }

    index = (address & 0xffff) >> 4;

    if (index >= devMemInfoPtr->statsCntMem.cntStatsTblSize)
    {
        skernelFatalError("Wrong Stats Count table address %X, exceed maximal size %X",
                          address,
                          devMemInfoPtr->statsCntMem.cntStatsTblSize);
    }

    if(StatsBank == 0 || devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[0] == NULL)
    {
        regValPtr = &devMemInfoPtr->statsCntMem.cntStatsTblMem[smiPort][index];
    }
    else
    {
        regValPtr = &devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[smiPort][index];
    }


    return regValPtr;
}


/*******************************************************************************
*   smemSohoVlanTableReg
*
* DESCRIPTION:
*       VLAN memory table accsess
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoVlanTableReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr; /* device's memory pointer */
    GT_U32             * regValPtr;     /* pointer to register's value */
    GT_U32 index;                       /* VLAN table memory offset */
    GT_U32 vid;                         /* VLAN id */
    GT_U32 word;                        /* word number 0 or 1 */

    regValPtr = 0;
    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    vid = (address >> 4) & 0xfff;
    word = (address & 0xf) / 4;

    index = (vid * SOHO_VLAN_ENTRY_WORDS) + word;
    if (index >= devMemInfoPtr->vlanDbMem.vlanTblMemSize)
    {
        skernelFatalError("Wrong VLAN table address %X, exceed maximal size %X",
                          address,
                          devMemInfoPtr->vlanDbMem.vlanTblMemSize);
    }
    regValPtr = &devMemInfoPtr->vlanDbMem.vlanTblMem[index];
    /* Vlan Table Registers */
    return regValPtr;
}

/*******************************************************************************
*   smemSohoStuTableReg
*
* DESCRIPTION:
*       STU memory table accsess
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoStuTableReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr; /* device's memory pointer */
    GT_U32             * regValPtr;     /* pointer to register's value */
    GT_U32 index;                       /* SIDtable memory offset */
    GT_U32 sid;                         /* VSID id */
    GT_U32 word;                        /* word number 0 or 1 */

    regValPtr = 0;
    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    sid = (address >> 4) & 0x3f;
    word = (address & 0xf) / 4;

    index = (sid * SOHO_VLAN_ENTRY_WORDS) + word;
    if (index >= devMemInfoPtr->stuDbMem.stuTblMemSize)
    {
        skernelFatalError("Wrong STU table address %X, exceed maximal size %X",
                          address,
                          devMemInfoPtr->stuDbMem.stuTblMemSize);
    }
    regValPtr = &devMemInfoPtr->stuDbMem.stuTblMem[index];
    /* Vlan Table Registers */
    return regValPtr;
}

/*******************************************************************************
*   smemSohoPvtTableReg
*
* DESCRIPTION:
*       PVT memory table accsess
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoPvtTableReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SOHO_DEV_MEM_INFO  * devMemInfoPtr; /* device's memory pointer */
    GT_U32             * regValPtr;     /* pointer to register's value */
    GT_U32 index;                       /* pvt table memory offset */
    GT_U32 pid;                         /* VSID id */
    GT_U32 word;                        /* word number 0 or 1 */

    regValPtr = 0;
    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    pid = (address >> 4) & 0x3f;
    word = (address & 0xf) / 4;

    index = (pid * SOHO_VLAN_ENTRY_WORDS) + word;
    if (index >= devMemInfoPtr->pvtDbMem.pvtTblMemSize)
    {
        skernelFatalError("Wrong PVT table address %X, exceed maximal size %X",
                          address,
                          devMemInfoPtr->pvtDbMem.pvtTblMemSize);
    }
    regValPtr = &devMemInfoPtr->pvtDbMem.pvtTblMem[index];
    /* Pvt Table Registers */
    return regValPtr;
}

/*******************************************************************************
*   smemSohoFatalError
*
* DESCRIPTION:
*       Memory access error funktion
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemSohoFatalError(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    skernelFatalError("smemSohoFatalError: illegal function pointer, device %d,  address 0x%x", devObjPtr, address);

    return 0;
}
/**
* @internal smemSohoInitFuncArray function
* @endinternal
*
* @brief   Init specific functions array.
*
* @param[in,out] devMemInfoPtr            - pointer to device memory object.
*/
static void smemSohoInitFuncArray(
    INOUT SOHO_DEV_MEM_INFO  * devMemInfoPtr
)
{
    GT_U32              i;

    for (i = 0; i < 16; i++) {
        devMemInfoPtr->specFunTbl[i].specFun    = smemSohoFatalError;
    }

    devMemInfoPtr->specFunTbl[0].specFun        = smemSohoGlobalReg;
    devMemInfoPtr->specFunTbl[1].specFun        = smemSohoPortsReg;
    devMemInfoPtr->specFunTbl[2].specFun        = smemSohoPhyReg;
    devMemInfoPtr->specFunTbl[3].specFun        = smemSohoVlanTableReg;
    devMemInfoPtr->specFunTbl[4].specFun        = smemSohoMacTableReg;
    devMemInfoPtr->specFunTbl[5].specFun        = smemSohoStatsCountReg;
    devMemInfoPtr->specFunTbl[6].specFun        = smemSohoStuTableReg;
    devMemInfoPtr->specFunTbl[7].specFun        = smemSohoPvtTableReg;
}

/**
* @internal smemSohoAllocSpecMemory function
* @endinternal
*
* @brief   Allocate address type specific memories.
*
* @param[in,out] devMemInfoPtr            - pointer to device memory object.
*/
static void smemSohoAllocSpecMemory(
    INOUT SOHO_DEV_MEM_INFO  * devMemInfoPtr,
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 port;        /* current device port */

    /* Global registers memory allocation */
    devMemInfoPtr->devRegs.globalRegsNum = SOHO_REG_NUMBER;
    devMemInfoPtr->devRegs.globalRegs =
                        calloc(SOHO_REG_NUMBER, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->devRegs.globalRegs == 0)
    {
        skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
    }
    /* Global 2 registers memory allocation */
    devMemInfoPtr->devRegs.global2Regs =
                        calloc(SOHO_REG_NUMBER, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->devRegs.global2Regs == 0)
    {
        skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
    }
    /* ports registers memory allocation */
    devMemInfoPtr->devRegs.portRegsNum = SOHO_REG_NUMBER;
    for (port = 0; port < SOHO_PORTS_NUMBER; port++)
    {
        devMemInfoPtr->devRegs.portRegs[port] =
                        calloc(SOHO_REG_NUMBER, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->devRegs.portRegs[port] == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }
    }

    /* Phy registers memory allocation */
    devMemInfoPtr->devRegs.phyRegsNum = SOHO_REG_NUMBER;
    for (port = 0; port < SOHO_PORTS_NUMBER; port++)
    {
        devMemInfoPtr->devRegs.phyRegs[port] =
                        calloc(SOHO_REG_NUMBER, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->devRegs.phyRegs[port] == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }
    }

    /* Mac table memory allocation */
    devMemInfoPtr->macDbMem.macTblMemSize = SOHO_MAC_TABLE_8K_SIZE;
    devMemInfoPtr->macDbMem.macTblMem =
        calloc(devMemInfoPtr->macDbMem.macTblMemSize, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->macDbMem.macTblMem == 0)
    {
        skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
    }

    /* Vlan table memory allocation */
    devMemInfoPtr->vlanDbMem.vlanTblMemSize = SOHO_VLAN_TABLE_SIZE;
    devMemInfoPtr->vlanDbMem.vlanTblMem =
        calloc(SOHO_VLAN_TABLE_SIZE, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->vlanDbMem.vlanTblMem == 0)
    {
        skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
    }
    devMemInfoPtr->vlanDbMem.violation = SOHO_NONE_E;

    if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        /* Stu table memory allocation */
        devMemInfoPtr->stuDbMem.stuTblMemSize = OPAL_STU_TABLE_SIZE;
        devMemInfoPtr->stuDbMem.stuTblMem =
            calloc(OPAL_STU_TABLE_SIZE, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->stuDbMem.stuTblMem == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }
    }

    if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        /* PVT table memory allocation */
        devMemInfoPtr->pvtDbMem.pvtTblMemSize = OPAL_PVT_TABLE_SIZE;
        devMemInfoPtr->pvtDbMem.pvtTblMem =
            calloc(OPAL_PVT_TABLE_SIZE, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->pvtDbMem.pvtTblMem == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }
    }

    /* RMON statistic counters allocation */
    devMemInfoPtr->statsCntMem.cntStatsTblSize =
        SOHO_RMON_CNT_NUM;
    for (port = 0; port < SOHO_PORTS_NUMBER; port++)
    {
        devMemInfoPtr->statsCntMem.cntStatsTblMem[port] =
            calloc(SOHO_RMON_CNT_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->statsCntMem.cntStatsTblMem[port] == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }
    }
    /* RMON captured counters allocation */
    devMemInfoPtr->statsCntMem.cntStatsCaptureMem =
        calloc(SOHO_RMON_CNT_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->statsCntMem.cntStatsCaptureMem == 0)
    {
        skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
    }

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
    {
        /* RMON statistic counters allocation */
        for (port = 0; port < SOHO_PORTS_NUMBER; port++)
        {
            devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port] =
                calloc(SOHO_RMON_CNT_NUM, sizeof(SMEM_REGISTER));
            if (devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port] == 0)
            {
                skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
            }
        }

        devMemInfoPtr->statsCntMem.cntStatsCaptureMem_bank1 =
            calloc(SOHO_RMON_CNT_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->statsCntMem.cntStatsCaptureMem_bank1 == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }

        devMemInfoPtr->devRegs.globalInternalCpuRegs =
            calloc(SOHO_REG_NUMBER, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->devRegs.globalInternalCpuRegs == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }

        devMemInfoPtr->devRegs.globalTcamRegs =
            calloc(SOHO_REG_NUMBER, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->devRegs.globalTcamRegs == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }

        /* Flow Control memory (per port) */
        devMemInfoPtr->flowCtrlMem.len = 0x20; /* 0x20 to 0x7F is unused*/
        for (port = 0; port < SOHO_PORTS_NUMBER; port++)
        {
            devMemInfoPtr->flowCtrlMem.mem[port] = calloc(0x20, sizeof(GT_U8));
            if ( devMemInfoPtr->flowCtrlMem.mem[port] == 0)
            {
                skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
            }
        }
        port = 0x1E;    /* CPU port */
        devMemInfoPtr->flowCtrlMem.mem[port] = calloc(0x20, sizeof(GT_U8));
        if ( devMemInfoPtr->flowCtrlMem.mem[port] == 0)
        {
            skernelFatalError("smemSohoAllocSpecMemory: allocation error\n");
        }
    }


}
/**
* @internal smemSohoActiveWriteStatsOp function
* @endinternal
*
* @brief   Provides CPU interface for the operations on the stat counters
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteStatsOp (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 fldValue;        /* register's and register' field value */
    SOHO_STAT_OP_E statOp;          /* statistic unit opcode */
    GT_U32 port;                    /* SMI device's port */
    SOHO_DEV_MEM_INFO * devMemInfoPtr; /* device's memory pointer */
    SMEM_REGISTER * statsMemPtr;    /* pointer to statistic register */
    GT_U32 counter, counterValue;   /* counter's index and counter's value */
    GT_U32 setIntr = 0;             /* interrupt's flag */
    GT_U32  StatsPtr;
    GT_U32  StatsPort;
    GT_U32  StatsBank;

    *memPtr = *inMemPtr;

    /* Get pointer to the device memory */
    devMemInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    /* Statistic unit Opcode */
    statOp = SMEM_U32_GET_FIELD(memPtr[0], 12, 3);

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
    {
        StatsPtr  = SMEM_U32_GET_FIELD(memPtr[0], 0, 5);
        StatsPort = SMEM_U32_GET_FIELD(memPtr[0], 5, 5);
        StatsBank = SMEM_U32_GET_FIELD(memPtr[0], 10, 1);
    }
    else
    {
        StatsPtr = SMEM_U32_GET_FIELD(memPtr[0], 0, 5);
        StatsPort = SMEM_U32_GET_FIELD(memPtr[0], 5, 4);
        StatsBank = 0;
    }

    setIntr = 1;

    if (statOp == FLUSH_ALL_CNT_ALL_E)
    {
        for (port = 0; port < SOHO_PORTS_NUMBER; port++)
        {
            statsMemPtr = devMemInfoPtr->statsCntMem.cntStatsTblMem[port];
            for (counter = 0; counter < SOHO_RMON_CNT_NUM; counter++)
            {
                statsMemPtr[counter] = 0;
            }

            if(devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port])
            {
                statsMemPtr = devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port];
                for (counter = 0; counter < SOHO_RMON_CNT_NUM; counter++)
                {
                    statsMemPtr[counter] = 0;
                }
            }

        }
    }
    else
    if (statOp == FLUSH_ALL_CNT_FOR_PORT_E)
    {
        if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
        {
            port = (StatsPort - 1);
        }
        else
        {
            port = StatsPtr;
        }

        statsMemPtr = devMemInfoPtr->statsCntMem.cntStatsTblMem[port];
        for (counter = 0; counter < SOHO_RMON_CNT_NUM; counter++)
        {
            statsMemPtr[counter] = 0;
        }

        if(devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port])
        {
            statsMemPtr = devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port];
            for (counter = 0; counter < SOHO_RMON_CNT_NUM; counter++)
            {
                statsMemPtr[counter] = 0;
            }
        }
    }
    else
    if (statOp == READ_CAPTURED_CNT_E)
    {
        counter = StatsPtr;

        if(StatsPort == 0)
        {
            /* read from 'captured' memory */
            /* Set these bits = 0x0 to access the captured counters. */

            if(StatsBank == 0 || devMemInfoPtr->statsCntMem.cntStatsCaptureMem_bank1 == NULL)
            {
                counterValue = devMemInfoPtr->statsCntMem.cntStatsCaptureMem[counter];
            }
            else
            {
                counterValue = devMemInfoPtr->statsCntMem.cntStatsCaptureMem_bank1[counter];
            }
        }
        else
        {
            /* read from actual memory */
            port = (StatsPort - 1);
            /* Set these bits = 0x1 to access the counters for Port 0.
               Set these bits = 0x2 to access the counters for Port 1, etc */

            if(StatsBank == 0 || devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port] == NULL)
            {
                counterValue = devMemInfoPtr->statsCntMem.cntStatsTblMem[port][counter];
            }
            else
            {
                counterValue = devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port][counter];
            }
        }

        fldValue = (counterValue >> 16);
        smemRegFldSet(devObjPtr, GLB_STATS_CNT3_2_REG, 0, 16, fldValue);

        fldValue = (counterValue & 0xffff);
        smemRegFldSet(devObjPtr, GLB_STATS_CNT1_0_REG, 0, 16, fldValue);
    }
    else
    if (statOp == CAPTURE_ALL_CNT_FOR_PORT_E)
    {
        if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
        {
            port = (StatsPort - 1);
        }
        else
        {
            port = StatsPtr;
        }
        statsMemPtr = devMemInfoPtr->statsCntMem.cntStatsTblMem[port];
        for (counter = 0; counter < SOHO_RMON_CNT_NUM; counter++)
        {
            devMemInfoPtr->statsCntMem.cntStatsCaptureMem[counter] =
                statsMemPtr[counter];
        }

        if(devMemInfoPtr->statsCntMem.cntStatsCaptureMem_bank1 &&
           devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port])
        {
            statsMemPtr = devMemInfoPtr->statsCntMem.cntStatsTblMem_bank1[port];
            for (counter = 0; counter < SOHO_RMON_CNT_NUM; counter++)
            {
                devMemInfoPtr->statsCntMem.cntStatsCaptureMem_bank1[counter] =
                    statsMemPtr[counter];
            }
        }
    }
    else
    {
        setIntr = 0;
    }

    /* Clear Statistic Unit Busy bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);
    if (setIntr == 1)
    {
        snetSohoDoInterrupt(devObjPtr,
            GLB_STATUS_REG,/*causeRegAddr*/
            GLB_CTRL_REG,/*causeMaskRegAddr*/
            1<<6/*bit 6 in global 1*/,
            1<<6/*bit 6 in global 1*/);
    }
}
/**
* @internal smemSohoActiveWriteVtuOp function
* @endinternal
*
* @brief   Provides CPU interface for the operations on the VTU
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void  smemSohoActiveWriteVtuOp (
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    GT_U32 fldValue;                        /* register's and register' field value */
    SOHO_VTU_OP_E vtuOp;                    /* VTU opcode */
    SOHO_DEV_MEM_INFO * memInfoPtr;         /* device's memory pointer */
    GT_U32 vtuWord[SOHO_VLAN_ENTRY_WORDS];  /* VTU words buffer */
    GT_U32 stuWord[OPAL_STU_ENTRY_WORDS];  /* VTU words buffer */
    GT_U32 vid, valid,sid;                  /* VID and valid bit */
    GT_STATUS status;                       /* return status value */
    GT_U16 * wordDataPtr = NULL;            /* violation data pointers */
    GT_U32 * opalMemPtr = NULL;             /* Pointer to the register's memory */

    *memPtr = *inMemPtr;

    if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {/* VLan entry information is taken 3 bytes before thr byte with the operatipn  */
        opalMemPtr = memPtr - 0x3;
    }

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    vtuOp = SMEM_U32_GET_FIELD(memPtr[0], 12, 3);

    if (vtuOp == VTU_FLUSH_ALL_VTU_ALL_E)
    {
        memset(memInfoPtr->vlanDbMem.vlanTblMem, 0,
                            memInfoPtr->vlanDbMem.vlanTblMemSize);
        if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            memset(memInfoPtr->stuDbMem.stuTblMem, 0,
                   memInfoPtr->stuDbMem.stuTblMemSize);
    }
    else
    if (vtuOp == VTU_LOAD_PURGE_ENTRY_E)
    {

        valid = SMEM_U32_GET_FIELD(memPtr[1], 12, 1);

        memset(vtuWord, 0, sizeof(vtuWord));
        /* Valid */
        if (valid == 1)
        {
            if(!SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {
                /* DBNum/SPID[3:0] */
                vtuWord[0] =  SMEM_U32_GET_FIELD(memPtr[0], 0, 4);
                /* VID */
                vtuWord[0] |= SMEM_U32_GET_FIELD(memPtr[1], 0, 12) << 4;
                /* Ports 0-3 */
                vtuWord[0] |= SMEM_U32_GET_FIELD(memPtr[2], 0, 16) << 16;
                /* Ports 4-7 */
                vtuWord[1] = SMEM_U32_GET_FIELD(memPtr[3], 0, 16);
                /* Ports 8-9 and Vid PRI */
                vtuWord[1] |= SMEM_U32_GET_FIELD(memPtr[4], 0, 16) << 16;
                /* Valid */
                vtuWord[2] = valid;
                /* DBNum[7:4] */
                vtuWord[2] |= SMEM_U32_GET_FIELD(memPtr[0], 8, 4) << 1;
            }
            else
            {
                /* DBNum/SPID[3:0] , VID policy --- 13 bits */
                vtuWord[0] =  SMEM_U32_GET_FIELD(opalMemPtr[0], 0, 13);  /* register 0x02 */
                /* SID */
                vtuWord[0] |= SMEM_U32_GET_FIELD(opalMemPtr[1], 0, 6) << 13; /* register 0x03 */
                /* VID */
                vtuWord[0] |= SMEM_U32_GET_FIELD(opalMemPtr[4], 0, 12) << 19; /* register 0x06 */
                /* Valid */
                vtuWord[0] |= valid << 31;
                /* Ports 0-3 */
                vtuWord[1] |= SMEM_U32_GET_FIELD(opalMemPtr[5], 0, 16) ; /* register 0x07 */
                /* Ports 4-7 */
                vtuWord[1] |= SMEM_U32_GET_FIELD(opalMemPtr[6], 0, 16) << 16; /* register 0x08 */
                /* Ports 8-9 and Vid PRI */
                vtuWord[2] |= SMEM_U32_GET_FIELD(opalMemPtr[7], 0, 16) ; /* register 0x09 */

            }

            /* Add new VTU entry to SRAM */
            smemSohoAddVtuEntry(devObjPtr, vtuWord);
        }
        else
        {
            if (!(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
            {
                vid = SMEM_U32_GET_FIELD(memPtr[1], 0, 12);
            }
            else
            {
                vid = SMEM_U32_GET_FIELD(opalMemPtr[3], 0, 12);
            }
            /* Delete VTU entry from SRAM */
            smemSohoDeleteVtuEntry(devObjPtr, vid);
        }
    }
    else
    if (vtuOp == VTU_GET_NEXT_E)
    {

        vid = SMEM_U32_GET_FIELD(memPtr[1], 0, 12);
         /* Find the next higher VID currently in the VTU's database */
        status = smemSohoGetNextVtuEntry(devObjPtr, vid, vtuWord);
        if (status == GT_OK)
        {
            if(!SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {
                 /* DBNum/SPID[3:0] */
                fldValue = vtuWord[0] & 0xf;
                SMEM_U32_SET_FIELD(memPtr[0], 0, 4, fldValue);
                /* VID */
                fldValue = (vtuWord[0] >> 4) & 0xfff;
                SMEM_U32_SET_FIELD(memPtr[1], 0, 12, fldValue);
                /* Ports 0-3 */
                fldValue = (vtuWord[0] >> 16) & 0xffff;
                SMEM_U32_SET_FIELD(memPtr[2], 0, 16, fldValue);
                /* Ports 4-7 */
                fldValue = vtuWord[1] & 0xffff;
                SMEM_U32_SET_FIELD(memPtr[3], 0, 16, fldValue);
                /* Ports 8-9 and Vid PRI */
                fldValue = (vtuWord[1] >> 16) & 0xffff;
                SMEM_U32_SET_FIELD(memPtr[4], 0, 16, fldValue);
                /* Valid */
                fldValue = vtuWord[2] & 0x1;
                SMEM_U32_SET_FIELD(memPtr[1], 12, 1, fldValue);
                /* DBNum[7:4] */
                fldValue = (vtuWord[2] >> 1) & 0xf;
                SMEM_U32_SET_FIELD(memPtr[0], 8, 4, fldValue);
            }
            else
            {
                 /* DBNum/SPID[11:0] ,  VID policy ---> 13 bits*/
                fldValue = vtuWord[0] & 0x1fff;
                SMEM_U32_SET_FIELD(opalMemPtr[0], 0, 13, fldValue); /* register 0x02 */

                /* SID */
                fldValue = (vtuWord[0] >> 13) & 0x3f;
                SMEM_U32_SET_FIELD(opalMemPtr[1], 0, 6, fldValue);  /* register 0x03 */

                /* VID , valid -- 13 bits */
                fldValue = (vtuWord[0] >> 19) & 0x1fff;
                SMEM_U32_SET_FIELD(opalMemPtr[4], 0, 13, fldValue);  /* register 0x06 */

                /* Ports 0-3 */
                fldValue = vtuWord[1] & 0xffff;
                SMEM_U32_SET_FIELD(opalMemPtr[5], 0, 16, fldValue);  /* register 0x07 */
                /* Ports 4-7 */
                fldValue = (vtuWord[1] >> 16) & 0xffff;
                SMEM_U32_SET_FIELD(opalMemPtr[6], 0, 16, fldValue);  /* register 0x08 */
                /* Ports 8-9 and Vid PRI */
                fldValue = vtuWord[2] & 0xffff;
                SMEM_U32_SET_FIELD(opalMemPtr[7], 0, 16, fldValue);  /* register 0x09 */
            }

        }
        else
        {

                /* Next VID was not found */
                SMEM_U32_SET_FIELD(memPtr[1], 12, 1, 0);
                SMEM_U32_SET_FIELD(memPtr[1], 0, 12, 0xfff);

        }
    }
    else
    if (vtuOp == VTU_GET_CLEAR_VIOLATION_E)
    {
        wordDataPtr = memInfoPtr->vlanDbMem.violationData;
        /* Copy violation data */
        memcpy(memPtr, wordDataPtr, 5 * sizeof(GT_U16));

        if (memInfoPtr->vlanDbMem.violation == SOHO_MISS_VTU_VID_E)
        {
            /* Set VTU miss violation */
            SMEM_U32_SET_FIELD(memPtr[0], 5, 1, 1);
        }
        else
        if (memInfoPtr->vlanDbMem.violation == SOHO_SRC_VTU_PORT_E)
        {
            /* Set Source Port member violation */
            SMEM_U32_SET_FIELD(memPtr[0], 6, 1, 1);
        }
    }
    else if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        if (vtuOp == STU_LOAD_PURGE_E)
        {
                    memset(stuWord, 0, sizeof(stuWord));
                    /* SID*/
                    stuWord[0] |= SMEM_U32_GET_FIELD(opalMemPtr[1], 0, 6);
                    /* Valid */
                    stuWord[0] |= SMEM_U32_GET_FIELD(opalMemPtr[4], 12, 1) << 6;
                    /* Ports 0-3 */
                    stuWord[1] = SMEM_U32_GET_FIELD(opalMemPtr[5], 0, 16) ;
                    /* Ports 4-7 */
                    stuWord[1] = SMEM_U32_GET_FIELD(opalMemPtr[6], 0, 16) ;
                    /* Ports 8-10  */
                    stuWord[2] |= SMEM_U32_GET_FIELD(opalMemPtr[7], 0, 12) ;
                    /* Add new STU entry to SRAM */
                    smemOpalAddStuEntry(devObjPtr, stuWord);


        }
        else
        if (vtuOp == STU_GET_NEXT_E)
        {
            sid = SMEM_U32_GET_FIELD(opalMemPtr[1], 0, 6);
            /* Find the next higher sid currently in the STU's database */
            status = smemOpalGetNextStuEntry(devObjPtr, sid, stuWord);
            if (status == GT_OK)
            {
                    /* Ports 0-3 */
                    stuWord[1] = SMEM_U32_GET_FIELD(opalMemPtr[5], 0, 16) << 16;
                    /* Ports 4-7 */
                    stuWord[1] |= SMEM_U32_GET_FIELD(opalMemPtr[6], 0, 16);
                    /* Ports 8-9 and Vid PRI */
                    stuWord[2] = SMEM_U32_GET_FIELD(opalMemPtr[7], 0, 16) << 16;

                    smemRegFldSet(devObjPtr, GLB_VTU_SID_REG, 0, 6, sid + 1);
            }
        }
    }

    /* Clear VTU busy */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);

    /* check that interrupt enabled */
    smemRegFldGet(devObjPtr, GLB_CTRL_REG, 4, 1, &fldValue);
    if (fldValue)
    {
        /* VTU done interrupt */
        snetSohoDoInterrupt(devObjPtr,
            GLB_STATUS_REG,/*causeRegAddr*/
            GLB_CTRL_REG,/*causeMaskRegAddr*/
            1<<4/*bit 4 in global 1*/,
            1<<4/*bit 4 in global 1*/);
    }
}

/**
* @internal smemSohoAddVtuEntry function
* @endinternal
*
* @brief   Add VTU entry to vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
*                                      vtuWord     - pointer to VTU words
*/
static void smemSohoAddVtuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 * vtuWordPtr
)
{
    GT_U32 vid;                         /* VLAN id */
    GT_U32 address = 0;                 /* VLAN table memory address */

    if (!(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
    {
        vid = (vtuWordPtr[0] >> 4) & 0xfff;
    }
    else
    {
        vid = (vtuWordPtr[0] >> 19) & 0xfff;
    }
    /* Make 32 bit word address */
    address = (3 << 28) | (vid << 4);
    smemMemSet(devObjPtr, address, &vtuWordPtr[0], 1);
    address += 0x4;
    smemMemSet(devObjPtr, address, &vtuWordPtr[1], 1);
    address += 0x4;
    smemMemSet(devObjPtr, address, &vtuWordPtr[2], 1);
}


/**
* @internal smemOpalAddStuEntry function
* @endinternal
*
* @brief   Add VTU entry to vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
*                                      stuWord     - pointer to STU words
*/
static void smemOpalAddStuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 * stuWordPtr
)
{
    GT_U32 address = 0;                 /* stu table memory address */
    GT_U32 sid;                         /* Vbit id */

    sid = stuWordPtr[0] & 0x3f;
    /* Make 32 bit word address */
    address = (6 << 28) | (sid << 4 );

    smemMemSet(devObjPtr, address, &stuWordPtr[0], 1);
    address += 0x4;
    smemMemSet(devObjPtr, address, &stuWordPtr[1], 1);
    address += 0x4;
    smemMemSet(devObjPtr, address, &stuWordPtr[2], 1);
}

/**
* @internal smemSohoDeleteVtuEntry function
* @endinternal
*
* @brief   Delete VTU entry from vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
*                                      vtuEntryPtr - pointer to VTU structure
*/
static            void smemSohoDeleteVtuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 vid
)
{
    GT_U32 address = 0;         /* VLAN table memory address */
    GT_U32 regVal = 0;          /* register's value to write */

    /* Make 32 bit word address */
    if (!(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
    {
        address = (3 << 28) | (vid << 4);
    }
    else
    {
        address = (3 << 28) | (vid << 19);
    }
    smemMemSet(devObjPtr, address, &regVal, 1);
    address += 0x4;
    smemMemSet(devObjPtr, address, &regVal, 1);
    address += 0x4;
    smemMemSet(devObjPtr, address, &regVal, 1);
}

/**
* @internal smemSohoGetNextVtuEntry function
* @endinternal
*
* @brief   Get next valid VTU entry from vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
*                                      vtuEntryPtr - pointer to VTU structure
*/
static GT_STATUS smemSohoGetNextVtuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 vid,
    OUT GT_U32 * vtuWordPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr;         /* device's memory pointer */
    GT_U32 vtuOffset = vid + 1, vtuSize;    /* VLAN table memory offset */
    SMEM_REGISTER * vlanTblMemPtr;          /* VLAN table memory pointer */
    GT_32 index;
    GT_U32 validBit;

    /* mask the vtu offset */
    vtuOffset = vtuOffset & 0xFFF;
    if (vid == 0xfff)
    {
        vtuOffset = 0 ;
    }

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);
    vlanTblMemPtr = memInfoPtr->vlanDbMem.vlanTblMem;

    vtuSize = memInfoPtr->vlanDbMem.vlanTblMemSize / SOHO_VLAN_ENTRY_BYTES;
    if (vtuOffset >= vtuSize)
    {
        skernelFatalError("Wrong VLAN table address %X, exceed maximal address %X",
                          vtuOffset,
                          memInfoPtr->vlanDbMem.vlanTblMemSize);
    }

    while (vtuOffset <  vtuSize)
    {
        index = vtuOffset * SOHO_VLAN_ENTRY_WORDS;

        if (!(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
        {
            validBit = SMEM_U32_GET_FIELD(vlanTblMemPtr[index + 2], 0, 1);
        }
        else
        {
            validBit = SMEM_U32_GET_FIELD(vlanTblMemPtr[index], 31, 1);
        }
        if ( validBit )
        {
            memcpy(vtuWordPtr, &vlanTblMemPtr[index],
                    SOHO_VLAN_ENTRY_WORDS * sizeof(GT_U32));

            return GT_OK;
        }

        vtuOffset++;
    }

    /* End of the VLAN table was reached with no new valid entries */
    return GT_NOT_FOUND;
}


/**
* @internal smemOpalGetNextStuEntry function
* @endinternal
*
* @brief   Get next valid VTU entry from vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
*                                      vtuEntryPtr - pointer to VTU structure
*/
static GT_STATUS smemOpalGetNextStuEntry (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 sid,
    OUT GT_U32 * stuWordPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr;         /* device's memory pointer */
    GT_U32 stuOffset = sid + 1, stuSize;    /* STU table memory offset */
    SMEM_REGISTER * stuTblMemPtr;          /* STU table memory pointer */
    GT_32 index;
    GT_U32 validBit;

    /* mask the vtu offset */
    stuOffset = stuOffset & 0xFFF;
    if (sid == 0xfff)
    {
        stuOffset = 0 ;
    }

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);
    stuTblMemPtr = memInfoPtr->stuDbMem.stuTblMem;

    stuSize = memInfoPtr->stuDbMem.stuTblMemSize / OPAL_STU_ENTRY_BYTES;
    if (stuOffset >= stuSize)
    {
        skernelFatalError("Wrong STU table address %X, exceed maximal address %X",
                          stuOffset,
                          memInfoPtr->stuDbMem.stuTblMemSize);
    }

    while (stuOffset <  stuSize)
    {
        index = stuOffset * OPAL_STU_ENTRY_WORDS;

        validBit = SMEM_U32_GET_FIELD(stuTblMemPtr[index], 6, 1);
        if ( validBit )
        {
            memcpy(stuWordPtr, &stuTblMemPtr[index],
                    OPAL_STU_ENTRY_WORDS * sizeof(GT_U32));

            return GT_OK;
        }
        if (stuOffset != 0xfff)
        {
            stuOffset++;
        }
    }

    /* End of the VLAN table was reached with no new valid entries */
    return GT_NOT_FOUND;
}

/**
* @internal smemSohoActiveReadGlobalStat function
* @endinternal
*
* @brief   Provides read from the global status register
*
* @param[in] deviceObjPtr             - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter -
*                                      global interrupt bit number.
*/
static void smemSohoActiveReadGlobalStat (
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{

    *outMemPtr = *memPtr;

    /* Clear all ROC fields */
    *memPtr &= (~(1 << 0));
    *memPtr &= (~(1 << 2));
    *memPtr &= (~(1 << 4));
    *memPtr &= (~(1 << 6));
}

/**
* @internal checkAndSendLinkChange function
* @endinternal
*
* @brief   check if port changed link , and send message (to smain task) if needed
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] oldLinkState             - the 'old' link state of the port (0-down , 1-up)
* @param[in] forceLinkDown            - the register force link down (0-no , 1 -yes)
* @param[in] forceLinkUp              - the register force link up   (0-no , 1 -yes)
*/
static void checkAndSendLinkChange (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   port,
    IN         GT_U32   oldLinkState,
    IN         GT_U32   forceLinkDown,
    IN         GT_U32   forceLinkUp
)
{
    SBUF_BUF_ID bufferId;       /* buffer */
    GT_U32  newLinkValue;       /* new link value */
    GT_U8  * dataPtr;         /* pointer to the data in the buffer */
    GT_U32 dataSize;          /* data size */
    GT_U32  linkStateWhenNoForce; /* state of port when no forcing of link UP , or down*/

    devObjPtr->portsArr[port].isForcedLinkUp   = forceLinkUp;
    devObjPtr->portsArr[port].isForcedLinkDown = forceLinkDown;

    if(forceLinkDown)
    {
        /* check if there was a change in the link state */
        if (!oldLinkState)
        {
            /* port already "down" */
            return;
        }
    }
    else if(forceLinkUp)
    {
        /* force link UP */
        /* check if there was a change in the link state */
        if(oldLinkState)
        {
            /* port already "up" */
            return;
        }
    }
    else
    {
        /* no forcing of link UP , or down */
        linkStateWhenNoForce = (devObjPtr->portsArr[port].linkStateWhenNoForce ==
                                SKERNEL_PORT_NATIVE_LINK_UP_E) ? 1 : 0;

        if(linkStateWhenNoForce == oldLinkState)
        {
            /* port already in the needed state */
            return;
        }
    }

    /* the link status must change */
    newLinkValue = !oldLinkState;

    /* Get buffer      */
    bufferId = sbufAlloc(devObjPtr->bufPool, SMEM_SOHO_LINK_FORCE_MSG_SIZE);
    if (bufferId == NULL)
    {
        simWarningPrintf(" checkAndSendLinkChange : no buffers for link change \n");
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);
    /* copy port number and link status to buffer  */
    memcpy(dataPtr, &port , sizeof(GT_U32) );
    dataPtr+=sizeof(GT_U32);
    memcpy(dataPtr, &newLinkValue , sizeof(GT_U32) );

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_LINK_CHG_MSG_E;

    /* put buffer to queue                          */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));

}

/**
* @internal smemSohoActiveWriteForceLinkDown function
* @endinternal
*
* @brief   Write Message to the main task - Link change on port.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function invoked when the linkdown bit (0) on
*       port<n> Auto-Negotiation has been changed.
*
*/
static void smemSohoActiveWriteForceLinkDown (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 currLinkState;          /* field value bit '0' in port Auto-Negotiation*/
    GT_U32 port_no;             /* port number */
    GT_U32 addressOfLinkStatus; /* current status of link */
    GT_BIT  newFroceLinkState;
    GT_BIT  newFroceLinkValue;

    *memPtr = *inMemPtr ;

    newFroceLinkState = ((*memPtr) & (1 << 4)) ? 1 : 0;
    newFroceLinkValue = ((*memPtr) & (1 << 5)) ? 1 : 0;

    /* find the port number */
    port_no =   ( (address >> 0x10 ) & 0xF );
    /* find the current state of the link */
    addressOfLinkStatus = SWITCH_PORT0_STATUS_REG(devObjPtr,port_no);
    smemRegFldGet(devObjPtr, addressOfLinkStatus, 11, 1, &currLinkState);

    checkAndSendLinkChange(devObjPtr,0/*not used*/,port_no,
        currLinkState,
        newFroceLinkState ? (1-newFroceLinkValue) : 0,
        newFroceLinkState ? newFroceLinkValue     : 0);
}

/**
* @internal smemSohoActiveWriteForceLink function
* @endinternal
*
* @brief   Handle write to Switch Port register #1 - PCS control
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function handles side-effects of forcing link up/down.
*       No interrupt is generated.
*
*/
static void smemSohoActiveWriteForceLink
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 linkState;          /* current status of link */
    GT_U32 port_no;             /* port number */
    GT_U32 addressOfLinkStatus,swPortRegAddr; /* Status register adresses */
    GT_BIT  newForceLinkState;
    GT_BIT  newForceLinkValue;

    *memPtr = *inMemPtr ;

    newForceLinkState = ((*memPtr) & (1 << 4)) ? 1 : 0;
    newForceLinkValue = ((*memPtr) & (1 << 5)) ? 1 : 0;

    /* find the port number */
    port_no =   ( (address >> 0x10 ) & 0xF );

    if (newForceLinkState)
    {
        linkState = newForceLinkValue;
    }
    else
    {
        /* Not forced, reflect PHY state */
        /* PHY port specific status */
        swPortRegAddr = PHY_PORT_STATUS_REG(port_no);
        /* Link */
        smemRegFldGet(devObjPtr, swPortRegAddr, 10, 1, &linkState);
    }

    /* Update the current state of the link */
    addressOfLinkStatus = SWITCH_PORT0_STATUS_REG(devObjPtr,port_no);
    smemRegFldSet(devObjPtr, addressOfLinkStatus, 11, 1, linkState);
}

/**
* @internal smemSohoActiveReadPhyInterruptStat function
* @endinternal
*
* @brief   Provides read from the Phy Interrupt status register
*
* @param[in] deviceObjPtr             - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter -
*                                      global interrupt bit number.
*/
static void smemSohoActiveReadPhyInterruptStat (
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U8      phyNum ;
    GT_U32     *dataPtr ;

    *outMemPtr = *memPtr;

    /* Clear all bits of Phy Interupt status */
    *memPtr = 0;

    phyNum = (GT_U8)((address >> 16) & 0xFF) ;

    /* clear Phy interrupt port summary port-bit */
    dataPtr = (GT_U32*)smemMemGet(deviceObjPtr, PHY_INTERRUPT_PORT_SUM_REG(phyNum)) ;
    *dataPtr &= (~ (1 << phyNum)) ;

    /* clear global interrupt status cause bit1 */
    dataPtr = (GT_U32*)smemMemGet(deviceObjPtr, GLB_STATUS_REG) ;
    *dataPtr &= (~ (1 << 1)) ;

}

/**
* @internal smemSohoActiveWriteAtuOp function
* @endinternal
*
* @brief   Provides CPU interface for the operations on the ATU
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteAtuOp (
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID             bufferId;   /* buffer */
    GT_U8                *  dataPtr;    /* pointer to the data in the buffer */
    GT_U32                  dataSize;   /* data size */
    GT_U32               * tmpAtuControlPtr ;

    *memPtr = *inMemPtr;

    /* send message to SKernel task to process it. */
    /* get buffer */
    bufferId = sbufAlloc(devObjPtr->bufPool,
                         SMEM_SOHO_ATU_MSG_WORDS_NUM * sizeof(GT_U32));

    if (bufferId == NULL)
    {
        printf(" smemSohoActiveWriteAtuOp: "\
                "no buffers to update MAC table\n");
        return;
    }

    /* get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    tmpAtuControlPtr = (GT_U32*)dataPtr + (5);
    /* copy ATU update message to buffer */
    memcpy(dataPtr, (GT_U8 *)memPtr,
           (SMEM_SOHO_ATU_MSG_WORDS_NUM -1) * sizeof(GT_U32));

    /* set the word 5 from the info of the ATU control register ,
      Offset 0x0A
      this is needed for the Opal,Jade that support dbNum up to 255
      */
    smemRegGet(devObjPtr,GLB_ATU_CTRL_REG ,tmpAtuControlPtr);

    tmpAtuControlPtr = (GT_U32*)dataPtr + (6);
    /* set the word 6 from the info of the ATU FID register ,
      Offset 0x01
      this is needed for the Opal+
      */
    smemRegGet(devObjPtr,GLB_ATU_FID_REG ,tmpAtuControlPtr);

    /* set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_FDB_UPDATE_E;

    /* Set ATU Busy */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 1);

    /* put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));

    /* make minimal sleep to give CPU time for
    the Simulation task for message processing*/
    SIM_OS_MAC(simOsSleep)(1);
}

/**
* @internal smemSohoActiveWriteTrunkMask function
* @endinternal
*
* @brief   Provides CPU interface for the Trunk Mask register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteTrunkMask(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr; /* device's memory pointer */
    GT_U32 entryIdx;                /* trunk mask index */
    SMEM_REGISTER   * trunkMaskPtr; /* trunk mask pointer */
    GT_BOOL         update;

    *memPtr = *inMemPtr;

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    update = SMEM_U32_GET_FIELD(memPtr[0], 15, 1);

    entryIdx = SMEM_U32_GET_FIELD(memPtr[0], 12, 3);
    trunkMaskPtr = &memInfoPtr->trunkMaskMem.trunkTblMem[entryIdx];

    if(update == 1)
    {
        *trunkMaskPtr = SMEM_U32_GET_FIELD(memPtr[0], 0, 11);
    }
    else
    {
        /* set the value needed for read */
        memInfoPtr->trunkMaskMem.readRegVal = *trunkMaskPtr;
    }

    /* Clear update bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);
}

/**
* @internal smemSohoActiveReadTrunkMask function
* @endinternal
*
* @brief   Provides CPU interface for the Trunk mask memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
*                                      inMemPtr    - Pointer to the memory to get register's content.
*/
static void smemSohoActiveReadTrunkMask(
    IN   SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN   GT_U32   address,
    IN   GT_U32   memSize,
    IN   GT_U32 * memPtr,
    IN   GT_UINTPTR   param,
    OUT  GT_U32 * outMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr; /* device's memory pointer */

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    /* get the value */
    *outMemPtr = memInfoPtr->trunkMaskMem.readRegVal;
}


/**
* @internal smemSohoActiveWriteTrunkRout function
* @endinternal
*
* @brief   Provides CPU interface for the Trunk routing memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteTrunkRout(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr; /* device's memory pointer */
    GT_U32 trunkIdx;                /* trunk rout index */
    SMEM_REGISTER   * trunkRoutPtr; /* trunk rout pointer */
    GT_BOOL         update;

    *memPtr = *inMemPtr;

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    update = SMEM_U32_GET_FIELD(memPtr[0], 15, 1);
    trunkIdx = SMEM_U32_GET_FIELD(memPtr[0], 11, 4);
    trunkRoutPtr = &memInfoPtr->trunkRouteMem.trouteTblMem[trunkIdx];

    if(update == 1)
    {
        *trunkRoutPtr = (SMEM_U32_GET_FIELD(memPtr[0], 0, 11));
    }
    else
    {
        /* set the value needed for read */
        memInfoPtr->trunkRouteMem.readRegVal = *trunkRoutPtr;
    }

    /* Clear update bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);
}

/**
* @internal smemSohoActiveReadTrunkRout function
* @endinternal
*
* @brief   Provides CPU interface for the Trunk routing memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
*                                      inMemPtr    - Pointer to the memory to get register's content.
*/
static void smemSohoActiveReadTrunkRout(
    IN   SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN   GT_U32   address,
    IN   GT_U32   memSize,
    IN   GT_U32 * memPtr,
    IN   GT_UINTPTR   param,
    OUT  GT_U32 * outMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr; /* device's memory pointer */

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    /* get the value */
    *outMemPtr = memInfoPtr->trunkRouteMem.readRegVal;
}

/**
* @internal smemSohoActiveWriteFlowCtrlDelay function
* @endinternal
*
* @brief   Provides CPU interface for the Flow Control Delay Memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteFlowCtrlDelay(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr;     /* device's memory pointer */
    SOHO_PORT_SPEED_MODE_E spd;         /* speed Number */
    GT_U32 delay;                       /* delay time */

    *memPtr = *inMemPtr;

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    spd = SMEM_U32_GET_FIELD(memPtr[0], 13, 2);
    delay = SMEM_U32_GET_FIELD(memPtr[0], 0, 13);

    /* Set flow control port delay */
    memInfoPtr->flowCtrlDelayMem.fcDelayMem[spd] = delay;

    /* Clear update bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);

}

/**
* @internal smemSohoActiveWriteTrgDevice function
* @endinternal
*
* @brief   Provides CPU interface for the Target Device memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteTrgDevice(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr; /* device's memory pointer */
    GT_U32 deviceIdx;               /* target device index */
    SMEM_REGISTER   * devMemPtr;    /* target device memory pointer */
    GT_BIT update;

    *memPtr = *inMemPtr;

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    update = SMEM_U32_GET_FIELD(memPtr[0], 15, 1);

    deviceIdx = SMEM_U32_GET_FIELD(memPtr[0], 8, 5);
    devMemPtr = &memInfoPtr->trgDevMem.deviceTblMem[deviceIdx];
    if (update)
    {
        *devMemPtr = (SMEM_U32_GET_FIELD(memPtr[0], 8, 5) << 8) |
                     (SMEM_U32_GET_FIELD(memPtr[0], 0, 4));
    }
    else
    {
        *memPtr = *devMemPtr;
    }

    /* Clear update bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);
}

/**
* @internal smemSohoActiveWriteSwitchMac function
* @endinternal
*
* @brief   Provides CPU interface for the Switch MAX /WoL / WoF memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteSwitchMac(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    SOHO_DEV_MEM_INFO * memInfoPtr; /* device's memory pointer */
    GT_U32 deviceIdx;               /* target device index */
    SMEM_REGISTER   * devMemPtr;    /* target device memory pointer */
    GT_BIT update;

    *memPtr = *inMemPtr;

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    update = SMEM_U32_GET_FIELD(memPtr[0], 15, 1);

    deviceIdx = SMEM_U32_GET_FIELD(memPtr[0], 8, 6);
    devMemPtr = &memInfoPtr->switchMacMem.switchMacWolWofMem[deviceIdx];
    if (update)
    {
        *devMemPtr = (SMEM_U32_GET_FIELD(memPtr[0], 8, 6) << 8) |
                     (SMEM_U32_GET_FIELD(memPtr[0], 0, 8));
    }
    else
    {
        *memPtr = *devMemPtr;
    }

    /* Clear update bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);
}

/**
* @internal smemSohoActiveWriteGobalStatus function
* @endinternal
*
* @brief   Provides CPU interface for the Target Device memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteGobalStatus(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    GT_U32 ppuEn;                       /* PHY Polling Unit Enabled */

    *memPtr = *inMemPtr;

    /* Get PPU state */
    ppuEn = SMEM_U32_GET_FIELD(memPtr[0], 14, 1);

    if (ppuEn)
    {
        /* PPU is Active detecting and initializing external PHYs */
        smemRegFldSet(devObjPtr, GLB_STATUS_REG, 14, 2, 1);
    }
    else
    {
        /* PPU Disabled after Initialization */
        smemRegFldSet(devObjPtr, GLB_STATUS_REG, 14, 2, 2);
    }

    /* Clear update bit */
    SMEM_U32_SET_FIELD(memPtr[0], 15, 1, 0);
}

/**
* @internal snetSohoDoInterrupt function
* @endinternal
*
* @brief   Set SOHO interrupt
*/
GT_VOID snetSohoDoInterrupt
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN GT_U32 causeMaskRegAddr,
    IN GT_U32 causeBitBmp,
    IN GT_U32 globalBitBmp
)
{
    DECLARE_FUNC_NAME(snetSohoDoInterrupt);

    GT_U32 causeRegVal;             /* Cause register value */
    GT_U32 causeRegMask;            /* Cause register mask */
    GT_BOOL setIntr = GT_FALSE;     /* Set interrupt */
    GT_BOOL setGlbIntr = GT_FALSE;  /* Set interrupt in global register */
    GT_U32  setInterrupt = 1;
    GT_U32  phyInterrupt;
    GT_U32  value;

    if(causeRegAddr >= PER_PHY_BASE(0,0))
    {
        phyInterrupt = 1;
    }
    else
    {
        phyInterrupt = 0;
    }

    if(causeRegAddr == GLB_STATUS_REG)
    {
        setIntr = GT_TRUE;
        goto globalCauseReg_lbl;
    }

    /* read interrupt cause data */
    smemRegGet(devObjPtr, causeRegAddr, &causeRegVal);

    /* read interrupt mask data */
    smemRegGet(devObjPtr, causeMaskRegAddr, &causeRegMask);

    /* set cause bit in the data for interrupt cause */
    causeRegVal = causeRegVal | causeBitBmp;

    /* if mask is set for bitmap, set summary bit in data for interrupt cause */
    if (causeBitBmp & causeRegMask)
    {
        SMEM_U32_SET_FIELD(causeRegVal, 0, 1, 1);
        setIntr = GT_TRUE;
    }

    if(phyInterrupt && setIntr == GT_TRUE)
    {
        GT_U32 port = (causeRegAddr >>  16) & 0xF;

        if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
        {
            /* the summary is in the global2 */
            /* Global interrupt cause */
            smemRegFldSet(devObjPtr, GLB2_INTERUPT_SOURCE, port, 1, 1);

            smemRegFldGet(devObjPtr, GLB2_INTERUPT_MASK, port, 1, &value);
            if(value == 0)
            {
                setIntr = GT_FALSE;
            }
        }
        else
        {
            /* PHY port interrupt summary */
            /* port summary the same for all ports in HW, in SW used for port0 */
            smemRegFldSet(devObjPtr, PHY_INTERRUPT_PORT_SUM_REG(0), port, 1, 1);
        }
    }

    smemRegSet(devObjPtr, causeRegAddr, causeRegVal);

globalCauseReg_lbl:
    if (setIntr == GT_TRUE)
    {
        /* Global Interrupt Cause Register */
        smemRegFldGet(devObjPtr, GLB_STATUS_REG , 0,9, &causeRegVal);

        /* Global Interrupt Summary Mask */
        smemRegFldGet(devObjPtr, GLB_CTRL_REG , 0,9, &causeRegMask);

        /* set cause bit in the data for interrupt cause */
        causeRegVal = causeRegVal | globalBitBmp;

        /* if mask is set for bitmap, set summary bit in data for interrupt cause */
        if (globalBitBmp & causeRegMask)
        {
            /* IntSum */
            setGlbIntr = GT_TRUE;
        }

        /* only bits 0..8 are interrupts related */
        smemRegFldSet(devObjPtr, GLB_STATUS_REG, 0,9,causeRegVal);
    }

    if (setGlbIntr && setInterrupt)
    {
        snetChtPerformScibSetInterrupt(devObjPtr->deviceId);
    }

    __LOG(("Global Interrupt [%s generated] , causeRegAddr[0x%8.8x] causeMaskRegAddr[0x%8.8x] causeBitBmp[0x%8.8x]\n"
                  ,(setGlbIntr && setInterrupt) ? "" : "not"
                  ,causeRegAddr
                  ,causeMaskRegAddr
                  ,causeBitBmp
                  ));
}

/**
* @internal snetSohoInterruptsMaskChanged function
* @endinternal
*
* @brief   handle interrupt mask registers
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - cause register address
* @param[in] maskRegAddr              - mask  register address
* @param[in] intRegBit                - interrupt bit in the global cause register
* @param[in] currentCauseRegVal       - current cause register values
* @param[in] lastMaskRegVal           - last mask register values
* @param[in] newMaskRegVal            - new mask register values
*/
extern void snetSohoInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
)
{
    GT_U32  diffCause;
    GT_U32  mask;

    if(causeRegAddr == GLB_STATUS_REG)
    {
        mask = 0x1ff;/*0..8*/
    }
    else
    {
        mask = 0xFFFF;
    }

    diffCause = ((newMaskRegVal & mask) & ~lastMaskRegVal) & currentCauseRegVal;

    /* check if there is a reason to do interrupt */
    if(diffCause)
    {
        snetSohoDoInterrupt(devObjPtr,causeRegAddr,
                              maskRegAddr,diffCause,intRegBit);
    }

    return;
}


/**
* @internal smemSohoActiveWritePhyInterruptMask function
* @endinternal
*
* @brief   CPU changes the PHY interrupts mask register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWritePhyInterruptMask(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    GT_U32 port;

    port = (address >> 16) & 0xf;

    /* Address of interrupt cause register */
    dataArray[0] = PHY_INTERRUPT_STATUS_REG(port);
    /* Address of interrupt mask register */
    dataArray[1] = PHY_INTERRUPT_ENABLE_REG(port);
    /* bmp of bit in Global 1 summary bit */
    dataArray[2] = 1<<7;

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemSohoActiveWriteGlobal2InterruptMask function
* @endinternal
*
* @brief   CPU changes the Global2 interrupts mask register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteGlobal2InterruptMask(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */

    /* Address of interrupt cause register */
    dataArray[0] = GLB2_INTERUPT_SOURCE;
    /* Address of interrupt mask register */
    dataArray[1] = GLB2_INTERUPT_MASK;
    /* bmp of bit in Global 1 summary bit */
    dataArray[2] = 1<<7;

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemSohoVtuEntryGet function
* @endinternal
*
* @brief   Get VTU entry from vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] vid                      - vlan id being searched for
*/
GT_STATUS smemSohoVtuEntryGet (
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 vid,
    OUT SNET_SOHO_VTU_STC * vtuEntry
)
{
    GT_STATUS status;               /* return status value */
    GT_U32 vtuWord[SOHO_VLAN_ENTRY_WORDS];/* VTU database entry and pointer */
    GT_U32 portMapBits;

    ASSERT_PTR(vtuEntry);

    if (vid == 0)
    {
        return GT_NOT_FOUND;
    }

    portMapBits = SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType) ?  4 :  2;

    /* Perform search with value one less than the one being searched for  */
    status = smemSohoGetNextVtuEntry(devObjPtr, vid - 1, vtuWord);
    if (status == GT_OK)
    {


        if (!(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
        {
            vtuEntry->dbNum = SMEM_U32_GET_FIELD(vtuWord[0], 0, 4);
            vtuEntry->vid = SMEM_U32_GET_FIELD(vtuWord[0], 4, 12);
            vtuEntry->pri = SMEM_U32_GET_FIELD(vtuWord[0], 12, 4);
            /* Get ports bitmap */
            vtuEntry->portsMap[0] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[0], 16, portMapBits);
            vtuEntry->portsMap[1] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[0], 20, portMapBits);
            vtuEntry->portsMap[2] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[0], 24, portMapBits);
            vtuEntry->portsMap[3] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[0], 28, portMapBits);
            vtuEntry->portsMap[4] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1],  0, portMapBits);
            vtuEntry->portsMap[5] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1],  4, portMapBits);
            vtuEntry->portsMap[6] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1],  8, portMapBits);
            vtuEntry->portsMap[7] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 12, portMapBits);
            vtuEntry->portsMap[8] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 16, portMapBits);
            vtuEntry->portsMap[9] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 20, portMapBits);
            /* Ruby */
            if ((devObjPtr->deviceType == SKERNEL_RUBY) ||
                (devObjPtr->deviceType == SKERNEL_OPAL))
            {
                vtuEntry->portsMap[10] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 24, portMapBits);

            }
        }
        else
        {
            vtuEntry->dbNum = SMEM_U32_GET_FIELD(vtuWord[0], 0, 12);
            vtuEntry->vidPolicy = SMEM_U32_GET_FIELD(vtuWord[0], 12, 1);
            vtuEntry->sid = SMEM_U32_GET_FIELD(vtuWord[0],13, 6);
            vtuEntry->vid = SMEM_U32_GET_FIELD(vtuWord[0], 19, 12);
            vtuEntry->valid = SMEM_U32_GET_FIELD(vtuWord[0], 31, 1);

            /* Get ports bitmap */
            vtuEntry->portsMap[0] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 0, portMapBits);
            vtuEntry->portsMap[1] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 4, portMapBits);
            vtuEntry->portsMap[2] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 8, portMapBits);
            vtuEntry->portsMap[3] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 12, portMapBits);
            vtuEntry->portsMap[4] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1],  16, portMapBits);
            vtuEntry->portsMap[5] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1],  20, portMapBits);
            vtuEntry->portsMap[6] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1],  24, portMapBits);
            vtuEntry->portsMap[7] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[1], 28, portMapBits);
            vtuEntry->portsMap[8] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[2], 0, portMapBits);
            vtuEntry->portsMap[9] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[2], 4, portMapBits);
            vtuEntry->portsMap[10] =
                (GT_U8)SMEM_U32_GET_FIELD(vtuWord[2], 8, portMapBits);

            vtuEntry->pri = SMEM_U32_GET_FIELD(vtuWord[2], 12, 3);
            vtuEntry->usepri = SMEM_U32_GET_FIELD(vtuWord[2], 15, 1);
        }
    }

    return status;
}

/**
* @internal smemSohoActiveWriteGlobal2SmiPhyCommnd function
* @endinternal
*
* @brief   writing to the SMI control register to access the internal PHYs.
*         (the device is hiding the option to directly access the PHYs)
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteGlobal2SmiPhyCommnd(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    DECLARE_FUNC_NAME(smemSohoActiveWriteGlobal2SmiPhyCommnd);

    GT_U32  regValue;
    GT_U32  dataRegAddr;
    GT_U32  SMIFunc,SMIMode,SMIOp,DevAddr,RegAddr;
    GT_U32  phyNum;
    GT_U32  phyRegAddr;

    if(0 == (SMEM_U32_GET_FIELD(*inMemPtr,15,1)))
    {
        /* no operation needed */
        *memPtr = *inMemPtr;
        return;
    }

    *memPtr = *inMemPtr & (~0x8000);/* clear the bit */

    /* SMIBusy - when set to 1 --> start operation */
    SMIFunc = SMEM_U32_GET_FIELD((*inMemPtr),13,2);
    SMIMode = SMEM_U32_GET_FIELD((*inMemPtr),12,1);
    SMIOp   = SMEM_U32_GET_FIELD((*inMemPtr),10,2);
    DevAddr = SMEM_U32_GET_FIELD((*inMemPtr), 5,5);
    RegAddr = SMEM_U32_GET_FIELD((*inMemPtr), 0,5);

    if(SMIFunc != 0)
    {
        __LOG(("SMIFunc[%d] is not supported by simulation \n",
            SMIFunc));
        return;
    }
    if(SMIMode != 1/*clause 22 - SMI */)
    {
        __LOG(("SMIMode[%d] is not supported by simulation \n",
            SMIMode));
        return;
    }
    if(SMIOp != 1/*Write*/ && SMIOp != 2/*Read*/)
    {
        __LOG(("SMIOp[%d] is not supported by simulation \n",
            SMIOp));
        return;
    }

    /* SMIFunc == 0 meaning internal PHY access */
    phyNum = DevAddr;
    phyRegAddr  = PER_PHY_BASE(phyNum,RegAddr);
    dataRegAddr = GLB2_SMI_PHY_DATA_REG;

    if(SMIOp == 1/*Write*/)
    {
        /* get data from 'SMI PHY data' register */
        smemRegGet(devObjPtr,dataRegAddr,&regValue);
        /* write the data to phy */
        /* use SCIB to allow 'active memory' in the PHY memory */
        scibWriteMemory(devObjPtr->deviceId, phyRegAddr, 1, &regValue);
    }
    else /* Read */
    {
        /* read data from from phy */
        /* use SCIB to allow 'active memory' in the PHY memory */
        scibReadMemory(devObjPtr->deviceId, phyRegAddr, 1, &regValue);
        /* write data to 'SMI PHY data' register */
        smemRegSet(devObjPtr,dataRegAddr,regValue);
    }

}


/**
* @internal smemSohoActiveWriteMultiChipAddressingControlReg function
* @endinternal
*
* @brief   writing to the SMI control register to access any register of the device.
*         (the device is hiding the option to directly access any register but only
*         2 registers - control and data)
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteMultiChipAddressingControlReg(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32   address,
    IN  GT_U32   memSize,
    IN  GT_U32 * memPtr,
    IN  GT_UINTPTR   param,
    IN  GT_U32 * inMemPtr
)
{
    DECLARE_FUNC_NAME(smemSohoActiveWriteMultiChipAddressingControlReg);

    SOHO_DEV_MEM_INFO * devMemInfoPtr;  /* device's memory pointer */
    GT_U32  regValue;
    GT_U32  SMIMode,SMIOp,DevAddr,RegAddr;

    if(0 == (SMEM_U32_GET_FIELD(*inMemPtr,15,1)))
    {
        /* no operation needed */
        *memPtr = *inMemPtr;
        return;
    }

    *memPtr = *inMemPtr & (~0x8000);/* clear the bit */

    /* SMIBusy - when set to 1 --> start operation */
    SMIMode = SMEM_U32_GET_FIELD((*inMemPtr),12,1);
    SMIOp   = SMEM_U32_GET_FIELD((*inMemPtr),10,2);
    DevAddr = SMEM_U32_GET_FIELD((*inMemPtr), 5,5);
    RegAddr = SMEM_U32_GET_FIELD((*inMemPtr), 0,5);

    if(SMIMode != 1/*clause 22 - SMI */)
    {
        __LOG(("SMIMode[%d] is not supported by simulation \n",
            SMIMode));
        return;
    }
    if(SMIOp != 1/*Write*/ && SMIOp != 2/*Read*/)
    {
        __LOG(("SMIOp[%d] is not supported by simulation \n",
            SMIOp));
        return;
    }

    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    /* start the 'local' SCIB transaction */
    devMemInfoPtr->devRegs.multiChipScibMode =
        SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_LOCAL_E;

    /*****************************************************/
    /* NOTE: call SCIB to allow active memory operations */
    /*****************************************************/

    if(SMIOp == 1/*Write*/)
    {
        /* get data from 'data' register */
        regValue = memPtr[1];

        /* write the data to the device */
        scibSmiRegWrite(devObjPtr->deviceId,DevAddr,RegAddr,regValue);
    }
    else /* Read */
    {
        /* read data from from the device */
        scibSmiRegRead(devObjPtr->deviceId,DevAddr,RegAddr,&regValue);
        /* write data to 'data' register */
        memPtr[1] = regValue;
    }

    /* end  the 'local' SCIB transaction */
    devMemInfoPtr->devRegs.multiChipScibMode =
        SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E;

}

void smemSohoDirectAccess_wm_internal(
    IN  GT_U8    hostDeviceNumber,
    IN  GT_U8    deviceNumber,
    IN  GT_U32   DevAddr,
    IN  GT_U32   RegAddr,
    IN  GT_U32   data,/*for write*/
    IN  GT_BOOL   doRead/*GT_TRUE - read , GT_FALSE - write*/
)
{
    GT_U32  regValue;
    SOHO_DEV_MEM_INFO * devMemInfoPtr;  /* device's memory pointer */
    SKERNEL_DEVICE_OBJECT* devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceNumber);
    GT_U32  index,memAddr;

    if(devObjPtr->deviceFamily != SKERNEL_SOHO_FAMILY)
    {
        skernelFatalError("smemSohoDirectAccess: trying to access wrong device [%d]\n",deviceNumber);
    }

    devMemInfoPtr = (SOHO_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    /* start the 'local' SCIB transaction */
    devMemInfoPtr->devRegs.multiChipScibMode =
        SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_LOCAL_E;

    /*****************************************************/
    /* NOTE: call SCIB to allow active memory operations */
    /*****************************************************/

    /* patch for Peridot */
    index = (DevAddr >= SOHO_GLOBAL_REGS_START_ADDR) ? 0 /* global registers */:
            (DevAddr < SOHO_PORT_REGS_START_ADDR) ? 1 /* port registers */:
            2/* PHY registers */;

    /* write the data to the device */
    memAddr = (index << 28) | (DevAddr << 16) | (RegAddr << 4);

    if(doRead == GT_FALSE/*Write*/)
    {
        /* get data from 'data' register */
        regValue = data;
        /*scibSmiRegWrite(devObjPtr->deviceId,DevAddr,RegAddr,regValue);*/

        scibWriteMemory(devObjPtr->deviceId, memAddr, 1, &regValue);
    }
    else /* Read */
    {
        /* read data from from the device */
        /*scibSmiRegRead(devObjPtr->deviceId,DevAddr,RegAddr,&regValue);*/

        scibReadMemory(devObjPtr->deviceId, memAddr, 1, &regValue);

        /* answer to the remote caller */
        smemChtAnswerToIpcCpssToWm(hostDeviceNumber,regValue,0,0);
    }

    /* end  the 'local' SCIB transaction */
    devMemInfoPtr->devRegs.multiChipScibMode =
        SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E;

}




/**
* @internal smemSohoActiveWriteFlowControl function
* @endinternal
*
* @brief   Write internal Flow Control related registers according to
*         Flow Control Register content.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of Flow Control Register
*       updated.
*
*/
static void smemSohoActiveWriteFlowControl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SOHO_DEV_FC_MEM *fcMemPtr;
    GT_U32 port;
    GT_U32 pointer;
    GT_U32 data;

    fcMemPtr = &((SOHO_DEV_MEM_INFO *)devObjPtr->deviceMemory)->flowCtrlMem;

    /* find the port number */
    port = (address >> 0x10 ) & 0x1F;
    /* find the current state of the link */
    pointer   = (*inMemPtr >> 8) & 0x7F; /* bits 14:8 */
    data  = *inMemPtr & 0xFF;            /* bits  7:0 */

    if ((pointer >= fcMemPtr->len) ||
        (port > 0x1E) ||
        (fcMemPtr->mem[port] == NULL))
    {
        skernelFatalError("smemSohoActiveWriteFlowControl: a memory index %x is out of range\n",
                          pointer);
    }

    if ((*inMemPtr >> 15) & 0x1) /* writing */
    {
        fcMemPtr->mem[port][pointer] = data;
    }
    else                          /* reading */
    {
        data = fcMemPtr->mem[port][pointer];
    }

    *memPtr = (pointer & 0x7F) << 8 | (data & 0xFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteIeeePriorityMappingTable function
* @endinternal
*
* @brief   Write internal IEEE Priority Mapping Table related registers according to
*         IEEE Priority Mapping Table Register content.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of Flow Control Register
*       updated.
*
*/
static void smemSohoActiveWriteIeeePriorityMappingTable (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SOHO_IEEE_PRIO_MAP_TABLE_MEM *ieeePrioMapTableMemPtr;
    GT_U32 port;
    GT_U32 pointer;
    GT_U32 data;

    /* find the port number */
    port = (address >> 0x10 ) & 0x1F;
    /* find the current state of the link */
    pointer   = (*inMemPtr >> 9) & 0x3F; /* bits 14:9 */
    data  = *inMemPtr & 0x1FF;           /* bits  8:0 */

    if (port == 0x1E)
    {
        /* CPU port */
        port = 11;
    }
    else
    if (port > 10)
    {
        skernelFatalError("smemSohoActiveWriteIeeePriorityMappingTable: port %d is out of range\n",
                          port);
    }

    ieeePrioMapTableMemPtr = &((SOHO_DEV_MEM_INFO *)devObjPtr->deviceMemory)->ieeePrioMapTableMem[port];

    if ((*inMemPtr >> 15) & 0x1) /* writing */
    {
        ieeePrioMapTableMemPtr->ieeePrioMapTableMem[pointer] = data;
    }
    else                          /* reading */
    {
        data = ieeePrioMapTableMemPtr->ieeePrioMapTableMem[pointer];
    }

    *memPtr = (pointer & 0x3F) << 9 | (data & 0x1FF); /* bit # 15 (Update) = 0 */
}


/**
* @internal smemSohoActiveWriteIMPCommDebug function
* @endinternal
*
* @brief   Write internal IMP related registers according to
*         IMP Comm/Debug Register content.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of IMP Comm/Debug Register
*       updated.
*       Very limited functionality is actually implemented at this version of
*       the simulation - IMP is not simulated.
*
*/
static void smemSohoActiveWriteIMPCommDebug(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SOHO_IMP_COMM_DEBUG_MEM *impCommDebugMemPtr;
    GT_U32 pointer;
    GT_U32 data;

    impCommDebugMemPtr = &((SOHO_DEV_MEM_INFO *)devObjPtr->deviceMemory)->impCommDebugMem;

    /* find the current state of the link */
    pointer   = (*inMemPtr >> 8) & 0x7F; /* bits 14:8 */
    data  = *inMemPtr & 0xFF;            /* bits  7:0 */

    if ((*inMemPtr >> 15) & 0x1) /* writing */
    {
        impCommDebugMemPtr->impCommDebugMem[pointer] = data;
    }
    else                          /* reading */
    {
        data = impCommDebugMemPtr->impCommDebugMem[pointer];
    }

    *memPtr = (pointer & 0x7F) << 8 | (data & 0xFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteLedControl function
* @endinternal
*
* @brief   Write internal LED Control related registers according to
*         LED Control Register content.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of LED Control Register
*       is updated.
*
*/
static void smemSohoActiveWriteLedControl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SOHO_LED_CONTROL_MEM    *ledControlMemPtr;
    GT_U32 port;
    GT_U32 pointer;
    GT_U32 data;

    /* find the port number */
    port = (address >> 0x10 ) & 0x1F;
    /* find the current state of the link */
    pointer   = (*inMemPtr >> 12) & 0x07; /* bits 14:12 */
    data  = *inMemPtr & 0x7FF;           /* bits 10:0 */

    if (port == 0x1E)
    {
        /* CPU port */
        port = 11;
    }
    else
    if (port > 10)
    {
        skernelFatalError("smemSohoActiveWriteLedControl: port %d is out of range\n",
                          port);
    }

    ledControlMemPtr = &((SOHO_DEV_MEM_INFO *)devObjPtr->deviceMemory)->ledControlMem[port];

    if ((*inMemPtr >> 15) & 0x1) /* writing */
    {
        ledControlMemPtr->ledControlMem[pointer] = data;
    }
    else                          /* reading */
    {
        data = ledControlMemPtr->ledControlMem[pointer];
    }

    *memPtr = (pointer & 0x07) << 12 | (data & 0x7FF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteScratchMisc function
* @endinternal
*
* @brief   Write Scratch / Miscellaneous registers.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of Scratch / Miscellaneous Register
*       updated.
*
*/
static void smemSohoActiveWriteScratchMisc
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SOHO_SCRATCH_MISC_MEM  *scratchMiscMemPtr;
    GT_U32 pointer;
    GT_U32 data;

    scratchMiscMemPtr = &((SOHO_DEV_MEM_INFO *)devObjPtr->deviceMemory)->scratchMiscMem;

    /* find the current state of the link */
    pointer   = (*inMemPtr >> 8) & 0x7F; /* bits 14:8 */
    data  = *inMemPtr & 0xFF;            /* bits  7:0 */

    if ((*inMemPtr >> 15) & 0x1) /* writing */
    {
        scratchMiscMemPtr->scratchMiscMem[pointer] = data;
    }
    else                          /* reading */
    {
        data = scratchMiscMemPtr->scratchMiscMem[pointer];
    }

    *memPtr = (pointer & 0x7F) << 8 | (data & 0xFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteTCAMCommon function
* @endinternal
*
* @brief   Write internal TCAM Common register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of TCAM Common Register
*       updated.
*       Very limited functionality is actually implemented at this version of
*       the simulation - TCAM is not simulated, and there is not memory for
*       actual TCAM pages.
*
*/
static void smemSohoActiveWriteTCAMCommon(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    *memPtr = (*inMemPtr & 0x7FFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteQosWeight function
* @endinternal
*
* @brief   Write internal Qos Weight register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of Qos Weight Register
*       updated.
*       Very limited functionality is actually implemented at this version of
*       the simulation - WRR is not simulated, and there is no memory for
*       actual weights.
*
*/
static void smemSohoActiveWriteQosWeight(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    *memPtr = (*inMemPtr & 0x7FFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteGlobalControl function
* @endinternal
*
* @brief   Write internal GlobalControl register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteGlobalControl(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    *memPtr = (*inMemPtr & 0x7FFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteHiddenRegister function
* @endinternal
*
* @brief   Write internal Global Control register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSohoActiveWriteHiddenRegister(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    *memPtr = (*inMemPtr & 0x7FFF); /* bit # 15 (Update) = 0 */
}

/**
* @internal smemSohoActiveWriteEnergyMngmt function
* @endinternal
*
* @brief   Write internal Energy Management register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers's specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function is invoked when content of Qos Weight Register
*       updated.
*       Very limited functionality is actually implemented at this version of
*       the simulation - WRR is not simulated, and there is no memory for
*       actual weights.
*
*/
static void smemSohoActiveWriteEnergyMngmt
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    *memPtr = (*inMemPtr & 0x7FFF); /* bit # 15 (Update) = 0 */
}


