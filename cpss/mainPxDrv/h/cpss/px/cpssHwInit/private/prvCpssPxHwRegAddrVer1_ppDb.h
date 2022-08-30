/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxHwRegAddrVer1_ppDb.h
*
* DESCRIPTION:
*       This file includes definitions of all different 'eArch' registers
*       addresses to be used by all functions accessing PP registers.
*       a variable of type PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC should be allocated
*       for each PP type.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvCpssPxHwRegAddrVer1_ppDb_h
#define __prvCpssPxHwRegAddrVer1_ppDb_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>

#ifndef NON_VALID_BASE_ADDR_CNS /* defrined also in prvCpssDxChHwRegAddrVer1_ppDb.h */
#define NON_VALID_BASE_ADDR_CNS         0xFE000001
#endif
#define END_OF_TABLE_INDICATOR_CNS      0xFFFFFFFE

extern GT_U32 prvPxUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];  /* unit base address array */

/**
* @internal prvCpssPxUnitBaseTableGet function
* @endinternal
*
* @brief   get based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @param[out] prvPxUnitsBaseAddrPtrPtr - ptr to point to table
*                                       GT_OK on success
*/
GT_STATUS prvCpssPxUnitBaseTableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32 **prvPxUnitsBaseAddrPtrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*__prvCpssPxHwRegAddrVer1_ppDb_h*/











