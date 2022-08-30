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
* @file mvHwsMtiCpuMacRev2If.h
*
* @brief MTI100 MAC interface
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMtiCpuMacRev2If_H
#define __mvHwsMtiCpuMacRev2If_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
/**
* @internal hwsMtiCpuMacRev2IfInit function
* @endinternal
*
* @brief   Init CG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiCpuMacRev2IfInit
(
    MV_HWS_MAC_FUNC_PTRS **funcPtrArray
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsMtiCpuMacRev2If_H */


