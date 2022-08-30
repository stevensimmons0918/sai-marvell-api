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
* @file smemSoho.h
*
* @brief Data definitions for Soho memories.
*
* @version   13
********************************************************************************
*/
#ifndef __smemSohoh
#define __smemSohoh

#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/suserframes/snetSoho.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Maximal devices number */
#define SOHO_SMI_DEV_TOTAL          (32)
/* Maximum registers number */
#define SOHO_REG_NUMBER             (32)
/* Maximum device ports number */
#define SOHO_PORTS_NUMBER           (11)

/* MAC table memory size in words */
#define SOHO_MAC_TABLE_8K_SIZE         (1024 * 8 * 3)
/* MAC table memory size in words */
#define SOHO_MAC_TABLE_4K_SIZE         (1024 * 4 * 3)

/* VLAN table entry size in words */
#define SOHO_VLAN_ENTRY_WORDS       3

/* VLAN table entry size in bytes */
#define SOHO_VLAN_ENTRY_BYTES       (SOHO_VLAN_ENTRY_WORDS )

/* VLAN table memory size */
#define SOHO_VLAN_TABLE_SIZE        (4 * 1024 * SOHO_VLAN_ENTRY_BYTES)

/* STU table entry size in words */
#define OPAL_STU_ENTRY_WORDS       3

/* STU table entry size in bytes */
#define OPAL_STU_ENTRY_BYTES       (OPAL_STU_ENTRY_WORDS * 4)

/* STU table memory size */
#define OPAL_STU_TABLE_SIZE        (64 * 22 * OPAL_STU_ENTRY_WORDS)

/* PVT table entry size  */
#define OPAL_PVT_ENTRY_BYTES       (0x200) /* 512 */

/* PVT table memory size */
#define OPAL_PVT_TABLE_SIZE        (512 * SOHO_PORTS_NUMBER)

/* Start address of phy related register.  */
#define SOHO_PHY_REGS_START_ADDR    (0x0)

/* Start address of ports related register.*/
#define SOHO_PORT_REGS_START_ADDR   (0x10)

/* Start address of global register.       */
#define SOHO_GLOBAL_REGS_START_ADDR (0x1b)

typedef enum {
    SOHO_NONE_E,
    SOHO_SRC_VTU_PORT_E,
    SOHO_MISS_VTU_VID_E,
    SOHO_FULL_VTU_E
} SOHO_VTU_VIOLATION_E;

typedef enum {
    SOHO_SRC_ATU_PORT_E,
    SOHO_MISS_ATU_E,
    SOHO_FULL_ATU_E
} SOHO_ATU_VIOLATION_E;

typedef enum
{
    SOHO_PORT_SPEED_10_MBPS_E,
    SOHO_PORT_SPEED_100_MBPS_E,
    SOHO_PORT_SPEED_1000_MBPS_E
} SOHO_PORT_SPEED_MODE_E;

/**
* @enum SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ENT
 *
 * @brief this is SCIB access mode to the device.
 * NOTE: relevant only to multi-chip addressing mode.
 * values:
 * SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E - the SCIB of the device
 * is accessed from 'remote' --> not from inside the simulation but from
 * other device or CPU.
 * at this mode the device hold only 2 registers of multiChipModeRegister[2]
 * (address 0x0 and 0x1)
 * and hold active memory only of the 'control' register
 * SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_LOCAL_E - the SCIB of the device
 * is accessed from 'local' --> from inside the simulation.
 * at this mode the device hold all it's registers
 * and hold 'regular' active memory
*/
typedef enum{

    SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_REMOTE_E,

    SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ACCESS_FROM_LOCAL_E,

} SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ENT;


/**
* @struct SOHO_DEVICE_MEM
 *
 * @brief Describe a Soho registers memory object.
*/
typedef struct{

    SMEM_REGISTER multiChipModeRegisters[2];

    /** current SCIB access mode to the device. */
    SMEM_SOHO_SCIB_MULTI_CHIP_MODE_ENT multiChipScibMode;

    /** @brief number of global registers
     *  globalRegs   - pointer to global registers memory
     *  global2RegsNum - number of global 2 registers
     *  global2Regs   - pointer to global 2 registers memory
     *  globalRegsNum = 32 0x10
     *  phyRegs     - registers with address offset 0x0
     */
    GT_U32 globalRegsNum;

    SMEM_REGISTER * globalRegs;

    SMEM_REGISTER * global2Regs;

    /** @brief number of PHY registers
     *  phyRegs     - pointer to Phy registers memory
     *  phyRegsNum = 32
     *  'per PHY' registers
     *  NOTE: NO 'pages' support !!!
     *  NOTE: in Peridot the PHYs accessed only via Global2 SMI indirect registers
     *  portRegs     - registers with address offset 0x10
     */
    GT_U32 phyRegsNum;

    SMEM_REGISTER * phyRegs [SOHO_PORTS_NUMBER];

    /** @brief number of 'per port' registers
     *  portRegs    - pointer to 'per port' registers memory
     *  Comments:
     *  globalRegs   - registers with address offset 0x1B
     *  portRegsNum = 32
     *  'per PORT' registers
     *  NOTE: in Peridot the PORTs addresses start at 0x00 (not 0x10)
     *  globalInternalCpuRegs - (peridot) Internal CPU memory.
     *  globalTcamRegs    - (peridot) TCAM memory.
     */
    GT_U32 portRegsNum;

    SMEM_REGISTER * portRegs [SOHO_PORTS_NUMBER];

    SMEM_REGISTER * globalInternalCpuRegs;

    SMEM_REGISTER * globalTcamRegs;

} SOHO_DEVICE_MEM;

/**
* @struct SOHO_ATU_DB_MEM
 *
 * @brief Describe a Soho MAC address table memory.
*/
typedef struct{

    /** @brief MAC address memory size
     *  macTblMem   - pointer to MAC address memory
     *  Comments:
     *  macTblMemSize - 4 1024 2 (4K entry, 2 bytes in the entry)
     */
    GT_U32 macTblMemSize;

    SMEM_REGISTER * macTblMem;

    SOHO_ATU_VIOLATION_E violation;

    GT_U16 violationData[5];

} SOHO_ATU_DB_MEM;

/**
* @struct OPAL_STU_DB_MEM
 *
 * @brief Describe a Soho STU table memory.
*/
typedef struct{

    /** @brief stu table memory size
     *  stuTblMem   - pointer to STU table memory
     *  Comments:
     *  STUTblMemSize - 4 1024 8 (4K entry, 22 bits in the entry)
     */
    GT_U32 stuTblMemSize;

    SMEM_REGISTER * stuTblMem;

} OPAL_STU_DB_MEM;

/**
* @struct OPAL_PVT_DB_MEM
 *
 * @brief Describe a OPAL PVT table memory.
*/
typedef struct{

    /** @brief pvt table memory size
     *  pvtTblMem   - pointer to PVT table memory
     *  Comments:
     *  PVTTblMemSize - 11 512 (1/2 K entry, 11 bits in the entry)
     */
    GT_U32 pvtTblMemSize;

    SMEM_REGISTER * pvtTblMem;

} OPAL_PVT_DB_MEM;

/**
* @struct SOHO_VLAN_DB_MEM
 *
 * @brief Describe a Soho VLAN table memory.
*/
typedef struct{

    /** @brief VLAN table memory size
     *  vlanTblMem   - pointer to VLAN table memory
     *  Comments:
     *  vlanTblMemSize - 4 1024 8 (4K entry, 64 bits in the entry)
     */
    GT_U32 vlanTblMemSize;

    SMEM_REGISTER * vlanTblMem;

    SOHO_VTU_VIOLATION_E violation;

    GT_U16 violationData[5];

} SOHO_VLAN_DB_MEM;

/**
* @struct SOHO_TRUNK_DB_MEM
 *
 * @brief Describe a Soho TRUNK table memory.
*/
typedef struct{

    SMEM_REGISTER trunkTblMem[8];

    /** @brief value of register 0x07 on global registers 2
     *  Comments:
     */
    GT_U32 readRegVal;

} SOHO_TRUNK_DB_MEM;

/**
* @struct SOHO_TROUT_DB_MEM
 *
 * @brief Describe a Soho Trunk route table memory.
*/
typedef struct{

    SMEM_REGISTER trouteTblMem[16];

    /** @brief value of register 0x08 on global registers 2
     *  Comments:
     */
    GT_U32 readRegVal;

} SOHO_TROUT_DB_MEM;


/**
* @struct SOHO_DEV_ROUT_MEM
 *
 * @brief Describe a Soho target device table memory.
*/
typedef struct{

    SMEM_REGISTER deviceTblMem[32];

} SOHO_DEV_ROUT_MEM;

/**
* @struct SOHO_SWITCH_MAC_WOL_WOF_MEM
 *
 * @brief Describe a Soho Switch MAC / WoL / WoF memory.
*/
typedef struct{

    SMEM_REGISTER switchMacWolWofMem[64];

} SOHO_SWITCH_MAC_WOL_WOF_MEM;

/**
* @struct SOHO_CNT_MEM
 *
 * @brief Describe a Soho RMON statistic counters.
*/
typedef struct{

    /** @brief RMON counters table size
     *  cntStatTblMem      - RMON counters table memory
     *  cntStatsTblMem_bank1  - RMON 'bank 1' counters table memory (Peridot)
     *  cntStatsCaptureMem_bank1 - RMON 'bank 1' captured counters table memory (Peridot)
     *  Comments:
     */
    GT_U32 cntStatsTblSize;

    SMEM_REGISTER * cntStatsTblMem[SOHO_PORTS_NUMBER];

    SMEM_REGISTER * cntStatsCaptureMem;

    SMEM_REGISTER * cntStatsTblMem_bank1[SOHO_PORTS_NUMBER];

    SMEM_REGISTER * cntStatsCaptureMem_bank1;

} SOHO_CNT_MEM;


/**
* @struct SOHO_DEV_FC_DELAY_MEM
 *
 * @brief Soho flow control delay memory
*/
typedef struct{

    SMEM_REGISTER fcDelayMem[3];

} SOHO_DEV_FC_DELAY_MEM;

/**
* @struct SOHO_DEV_FC_MEM
 *
 * @brief Soho Flow Control memory (per port)
*/
typedef struct{

    /** @brief size of mem.
     *  mem    - Flow Control memory octets:
     *  0x00 = Port Limit In
     *  0x01 = Port Limit Out
     *  0x02 to 0x0F = Reserved for future use
     *  0x10 = Port Flow Control Mode
     *  0x11 to 0x17 = Reserved for future use
     *  0x18 to 0x1B = PFC Priority to Tx Queue Mapping
     *  0x1C to 0x1F = Rx Queue to PFC Priority Mapping
     *  0x20 to 0x7F = Reserved for future use
     *  Comments:
     */
    GT_U32 len;

    GT_U8 *mem[(0x1E)+1];

} SOHO_DEV_FC_MEM;

/**
* @struct SOHO_IEEE_PRIO_MAP_TABLE_MEM
 *
 * @brief Soho IEEE Priority Mapping Table memory (per port)
*/
typedef struct{

    SMEM_REGISTER ieeePrioMapTableMem[64];

} SOHO_IEEE_PRIO_MAP_TABLE_MEM;

/**
* @struct SOHO_IMP_COMM_DEBUG_MEM
 *
 * @brief Describe IMP Comm/Debug memory.
*/
typedef struct{

    SMEM_REGISTER impCommDebugMem[128];

} SOHO_IMP_COMM_DEBUG_MEM;

/**
* @struct SOHO_LED_CONTROL_MEM
 *
 * @brief Soho LED Control memory (per port)
*/
typedef struct{

    SMEM_REGISTER ledControlMem[8];

} SOHO_LED_CONTROL_MEM;

/**
* @struct SOHO_SCRATCH_MISC_MEM
 *
 * @brief Describe Scratch / Miscellaneous memory.
*/
typedef struct{

    SMEM_REGISTER scratchMiscMem[128];

} SOHO_SCRATCH_MISC_MEM;

/**
* @struct SOHO_DEV_MEM_INFO
 *
 * @brief Describe a device's memory object in the simulation.
*/
typedef struct {
    SMEM_SPEC_FIND_FUN_ENTRY_STC    specFunTbl[16];/* using only 4 MSBits of address */
    GT_U8                           accessMode;
    SOHO_DEVICE_MEM                 devRegs;
    SOHO_ATU_DB_MEM                 macDbMem;
    SOHO_VLAN_DB_MEM                vlanDbMem;
    SOHO_CNT_MEM                    statsCntMem;
    SOHO_TRUNK_DB_MEM               trunkMaskMem;
    SOHO_DEV_ROUT_MEM               trgDevMem;
    SOHO_TROUT_DB_MEM               trunkRouteMem;
    SOHO_DEV_FC_DELAY_MEM           flowCtrlDelayMem;
    OPAL_STU_DB_MEM                 stuDbMem;
    OPAL_PVT_DB_MEM                 pvtDbMem;
    SOHO_DEV_FC_MEM                 flowCtrlMem;
    SOHO_SWITCH_MAC_WOL_WOF_MEM     switchMacMem;
    SOHO_IEEE_PRIO_MAP_TABLE_MEM    ieeePrioMapTableMem[12];
    SOHO_IMP_COMM_DEBUG_MEM         impCommDebugMem;
    SOHO_LED_CONTROL_MEM            ledControlMem[12];
    SOHO_SCRATCH_MISC_MEM           scratchMiscMem;
}SOHO_DEV_MEM_INFO;

/**
* @internal smemSohoInit function
* @endinternal
*
* @brief   Init memory module for a Soho device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemSohoInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

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
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr
);

/**
* @internal smemSohoVtuEntryGet function
* @endinternal
*
* @brief   Get VTU entry from vlan table SRAM
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] vid                      - VLAN ID
* @param[in] vtuEntry                 - pointer to VTU entry in SRAM
*/
GT_STATUS smemSohoVtuEntryGet (
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 vid,
    OUT SNET_SOHO_VTU_STC * vtuEntry
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemSohoh */

