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
* @file smemBobcat3.h
*
* @brief Bobcat3 memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemBobcat3h
#define __smemBobcat3h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemBobcat2.h>

ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemBobcat3ActiveReadPlrTables);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemBobcat3ActiveRead100GMacMibCounters);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobcat3ActiveReadHaArpTsTable);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemChtActiveWrite100GMacControl0);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemChtActiveWrite100GMacResets);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemChtActiveWrite100GMacMibCtrlReg);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobcat3ActiveWritePolicerManagementCounters);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobcat3ActiveWriteHaArpTsTable);

/* sip5.20 set metering and counting shared between all units */
#define ACTIVE_MEM_POLICER_COMMON_1_MAC(policerId)   \
    /*SMEM_LION3_POLICER_METERING_BASE_ADDR_REG*/                                                                    \
    /*Policer Metering Base Address*/                                                                                \
    {0x00000028, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerMeteringBaseAddr,  policerId},              \
    /*SMEM_LION3_POLICER_COUNTING_BASE_ADDR_REG*/                                                                    \
    /*Policer Counting Base Address*/                                                                                \
    {0x00000024, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerCountingBaseAddr,  policerId}

/* management counters policer active memory */
#define ACTIVE_MEM_POLICER_BC3_MANAGEMENT_COUNTERS_MAC(policerId)   \
    /* in BC3 those counters are not accessible to the CPU !!! */   \
    /* do not allow to write to it , do not let read from it !!! */ \
    {POLICER_MANAGEMENT_COUNTER_ADDR_CNS, POLICER_MANAGEMENT_COUNTER_MASK_CNS, \
        smemBobcatActiveReadPolicerManagementCounters, policerId, \
        smemBobcatActiveWritePolicerManagementCounters,policerId}


/* bc3 common policer active memory */
#define ACTIVE_MEM_POLICER_BC3_COMMON_MAC(policerId) \
    ACTIVE_MEM_POLICER_COMMON_MAC(policerId),        \
    ACTIVE_MEM_POLICER_COMMON_1_MAC(policerId)

#define ACTIVE_MEM_CNC_BC3_COMMON_MAC                                                                       \
    /* CNC Fast Dump Trigger Register Register */                                                           \
    {0x00000030,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncFastDumpTrigger, 0},                    \
                                                                                                            \
    /* CPU direct read from the counters */                                                                 \
    {0x00010000,0xFFFF0000, smemCht3ActiveCncBlockRead, 0x00010000/*base address of block 0*/, smemChtActiveWriteToReadOnlyReg, 0},    \
    {0x00020000,0xFFFF0000, smemCht3ActiveCncBlockRead, 0x00010000/*base address of block 0*/, smemChtActiveWriteToReadOnlyReg, 0},    \
                                                                                                            \
    /* CNC Block Wraparound Status Register */                                                              \
    {0x00001400,0xFFFFFE00, smemCht3ActiveCncWrapAroundStatusRead, 0, NULL, 0},                             \
                                                                                                            \
    /* read interrupts cause registers CNC -- ROC register */                                               \
    {0x00000100,SMEM_FULL_MASK_CNS,smemChtActiveReadIntrCauseReg, 3, smemChtActiveWriteIntrCauseReg, 0},    \
                                                                                                            \
    /* Write Interrupt Mask CNC Register */                                                                 \
    {0x00000104,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncInterruptsMaskReg, 0}

/**
* @internal smemBobcat3Init function
* @endinternal
*
* @brief   Init memory module for a Bobcat3 device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemBobcat3Init
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemBobcat3Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobcat3Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemBobcat3SpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemBobcat3SpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);
/**
* @internal smemBobcat3SpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemBobcat3SpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemBobcat3h */


