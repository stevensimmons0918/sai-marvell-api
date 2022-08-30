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
* @file smemLion2.h
*
* @brief Lion2 memory mapping implementation: the pipe and the shared memory
*
* @version   23
********************************************************************************
*/
#ifndef __smemLion2h
#define __smemLion2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemXCat2.h>

/*
    get the register address according to offset from the base address of the
    OAM unit

    cycle 0 - ioam 0
    cycle 1 - ioam 1
    cycle 2 - eoam


    offset - register offset from start of the unit
*/
#define SMEM_LION3_OAM_OFFSET_MAC(dev, cycle , offset)      \
    (SMEM_LION3_OAM_BASE_ADDR_MAC(dev, cycle) + (offset))

/* step between addresses of 2 consecutive ports for MIB counter */
#define XG_COUNTER_STEP_PER_PORT_CNS    0x20000
/* MAC Counters address mask */
#define     SMEM_LION2_XG_MIB_COUNT_MSK_CNS       (GT_U32)((~(XG_COUNTER_STEP_PER_PORT_CNS * 0x3F)) & (~(0xFF)))

/**
* @internal smemLion2RegsInfoSet function
* @endinternal
*
* @brief   Init memory module for Lion2 and above devices.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemLion2RegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemLion2Init function
* @endinternal
*
* @brief   Init memory module for a Lion2 device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemLion2Init
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemLion2Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemLion2Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/*******************************************************************************
*   smemLion2SubUnitMemoryGet
*
* DESCRIPTION:
*       Get the port-group object for specific sub-unit.

* INPUTS:
*       devObjPtr   - pointer to device object.
*       accessType  - Memory access type PCI/Regular
*       address     - address of memory(register or table).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to object for specific subunit
*
* COMMENTS:
*       TxQ unit contains several sub-units, that can be classed into three categories:
*           - Sub-units that are duplicated per core each sub-unit is configured
*             through the management interface bound to its core.
*           - Sub-units that serve a pair of cores, and thus have two instances.
*             Each of the instances is configured through a different management interface.
*           - Sub-units that have a single instance, and serve the entire device.
*
*******************************************************************************/
SKERNEL_DEVICE_OBJECT * smemLion2SubUnitMemoryGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address
);

/**
* @internal smemLion2AllocSpecMemory function
* @endinternal
*
* @brief   Allocate address type specific memories.
*
* @param[in,out] commonDevMemInfoPtr      - pointer to common device memory object.
*/
void smemLion2AllocSpecMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr
);

/**
* @internal smemLion2TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemLion2TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemLion2IsDeviceMemoryOwner function
* @endinternal
*
* @brief   Return indication that the device is the owner of the memory.
*         relevant to multi port groups where there is 'shared memory' between port groups.
*
* @retval GT_TRUE                  - the device is   the owner of the memory.
* @retval GT_FALSE                 - the device is NOT the owner of the memory.
*/
GT_BOOL smemLion2IsDeviceMemoryOwner
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address
);

/*******************************************************************************
*   smemLion2FindMem
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       accessType  - Read/Write operation
*       address     - address of memory(register or table).
*       memsize     - size of memory
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
void * smemLion2FindMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr
);

/**
* @internal smemLion2InitFuncArray function
* @endinternal
*
* @brief   Init specific functions array.
*
* @param[in,out] commonDevMemInfoPtr      - pointer to common device memory object.
*/
void smemLion2InitFuncArray
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr
);

/**
* @internal smemLion2UnitMemoryBindToChunk function
* @endinternal
*
* @brief   bind the units to the specific unit chunk with the generic function
*         build addresses of the units according to base address
* @param[in] devObjPtr                - pointer to device object.
*/
void smemLion2UnitMemoryBindToChunk
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemLion2TableInfoSetPart2 function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*         AFTER the bound of memories (after calling smemBindTablesToMemories)
* @param[in] devObjPtr                - device object PTR.
*/
void smemLion2TableInfoSetPart2
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemLion2UnitPex function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PEX unit
*/
void smemLion2UnitPex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN GT_U32 pexBaseAddr
);
/**
* @internal smemLion2GopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Lion2 and above devices.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemLion2GopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);
/**
* @internal smemLion2MgRegDbInit function
* @endinternal
*
* @brief   Init MG regDB registers for Lion2 and above devices.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemLion2MgRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);



ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteFDBGlobalCfgReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteEqDualDeviceIdReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteTtiInternalConfReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteHaGlobalConfReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteHierarchicalPolicerControl);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteKVCOCalibrationControlReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteTgControl0Reg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLion2ActiveWriteDfxResetCtlReg);

ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemLion2ActiveReadMsmMibCounters);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemLion2h */


