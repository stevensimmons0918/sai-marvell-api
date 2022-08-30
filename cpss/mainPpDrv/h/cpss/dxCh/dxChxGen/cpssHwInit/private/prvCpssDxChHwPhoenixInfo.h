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
* @file prvCpssDxChHwPhoenixInfo.h
*
* @brief Private definition for the AC5X (a.k.a Phoenix) devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwPhoenixInfoh
#define __prvCpssDxChHwPhoenixInfoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvDxChHwRegAddrPhoenixDbInit function
* @endinternal
*
* @brief   init the base address manager of the Phoenix device.
*         prvDxChPhoenixUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrPhoenixDbInit(GT_VOID);

/**
* @internal prvCpssPhoenixInitParamsSet function
* @endinternal
*
* @brief   Phoenix : init the very first settings in the DB of the device.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssPhoenixInitParamsSet
(
    IN  GT_U8   devNum
);
GT_VOID prvCpssPhoenixNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwPhoenixInfoh */

