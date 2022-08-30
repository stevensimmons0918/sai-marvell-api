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
* @file prvCpssDxChHwInitTypes.h
*
* @brief Includes Core level basic Hw initialization data structures.
* 
*
* @version   41
********************************************************************************
*/
#ifndef __prvCpssDxChHwInitTypesh
#define __prvCpssDxChHwInitTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ERRATA PRV_CPSS_DXCH_BOBK_TXQ_TAIL_DROP_TABLES_R_W_PENDS_BUS_WA_E treatment */

/* all array sizes in structures are minimal that were needed for Bobcat3      */

/**
* @struct ERRATA_BOBK_TXQ_TAIL_DROP_UNIT_ADDR_STC
 *
 * @brief A structure to hold TXQ_QUEUE unit address range.
*/
typedef struct{

    /** unit base address. */
    GT_U32 unitBase;

    /** unit address range excluded upper bound. */
    GT_U32 unitUpperBound;

} ERRATA_BOBK_TXQ_TAIL_DROP_UNIT_ADDR_STC;

/**
* @struct ERRATA_BOBK_TXQ_TAIL_DROP_ADDR_RANGE_STC
 *
 * @brief A structure to hold table address range.
*/
typedef struct{

    /** table base address. */
    GT_U32 lowBound;

    /** table address range excluded upper bound. */
    GT_U32 highBound;

    /** size of table line (entry alignment) in bytes */
    GT_U32 lineSizeInBytes;

    /** size of table entry in bytes */
    GT_U32 entrySizeInBytes;

} ERRATA_BOBK_TXQ_TAIL_DROP_ADDR_RANGE_STC;

/**
* @struct ERRATA_BOBK_TXQ_TAIL_DROP_READ_AFTER_WRITE_STC
 *
 * @brief A structure to hold info for write WA - "read after wtrite".
*/
typedef struct{

    /** table CPSS Id to read from it entry number 62. */
    CPSS_DXCH_TABLE_ENT tableId;

} ERRATA_BOBK_TXQ_TAIL_DROP_READ_AFTER_WRITE_STC;

/**
* @struct ERRATA_BOBK_TXQ_TAIL_DROP_CONFUIGURE_REG_STC
 *
 * @brief A structure to hold info for read WA - instanse per each configured/restored register.
*/
typedef struct{

    /** register address. */
    GT_U32 regAddr;

    /** bitmask for update. */
    GT_U32 updateMask;

    /** data for update. */
    GT_U32 updateData;

} ERRATA_BOBK_TXQ_TAIL_DROP_CONFUIGURE_REG_STC;

/**
* @struct ERRATA_BOBK_TXQ_TAIL_DROP_DATA_STC
 *
 * @brief A structure to hold info WA per device family.
*/
typedef struct{

    /** GT_TRUE */
    GT_BOOL initialized;

    /** bounds of TXQ_QUEUE unit address space. */
    ERRATA_BOBK_TXQ_TAIL_DROP_UNIT_ADDR_STC unitAddr;

    ERRATA_BOBK_TXQ_TAIL_DROP_ADDR_RANGE_STC addrRangeArr[6];

    GT_U32 numOfRanges;

    /** read after write WA table info. */
    ERRATA_BOBK_TXQ_TAIL_DROP_READ_AFTER_WRITE_STC readAfterWrite;

    ERRATA_BOBK_TXQ_TAIL_DROP_CONFUIGURE_REG_STC updatedRegArr[2];

    /** number of updated registers. */
    GT_U32 numOfUpdatedRegs;

} ERRATA_BOBK_TXQ_TAIL_DROP_DATA_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #define __prvCpssDxChHwInitTypesh  */


