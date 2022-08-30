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
* @file simSip6Pha.h
*
* @brief Includes common PHA definitions for SIP 6 devices
*
* @version   1
********************************************************************************
*/
#ifndef __simSip6Pha_h
#define __simSip6Pha_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


void testEndianCasting(IN SKERNEL_DEVICE_OBJECT * devObjPtr);
void updateNeededRegisters(void);
uint32_t pha_fw_activeMem_write_toPpnMem(uint32_t addr, uint32_t *memPtr , uint32_t writeValue);
uint32_t fw_activeMem_read_DRAM_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue);
uint32_t fw_activeMem_write_DRAM_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue);
uint32_t pha_fw_activeMem_read_TOD(uint32_t addr, uint32_t *memPtr , uint32_t writeValue);
extern MEMORY_RANGE fw_memoryMap[];
extern ACTIVE_MEMORY_RANGE fw_activeMemoryMap[];
extern GT_U32  pha_fw_activeMem_read_TOD_always_ZERO;
void invalidFirmwareThread();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __simSip6Pha_h */


