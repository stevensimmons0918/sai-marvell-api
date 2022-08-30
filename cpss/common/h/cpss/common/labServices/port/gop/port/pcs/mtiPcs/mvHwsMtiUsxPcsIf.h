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
* @file __mvHwsMtiUsxPcsIf_H.h
*
* @brief MTI USX PCS interface API
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMtiUsxPcsIf_H
#define __mvHwsMtiUsxPcsIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsMtiUsxLowSpeedPcsRev2IfInit function
* @endinternal
*
* @brief   Init usx LSPCS configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiUsxLowSpeedPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray);

/**
* @internal mvHwsMtiUsxPcsRev2IfInit function
* @endinternal
*
* @brief   Init MtiUsxPcsLowSpeed configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiUsxPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsMtiUsxPcsIf_H */


