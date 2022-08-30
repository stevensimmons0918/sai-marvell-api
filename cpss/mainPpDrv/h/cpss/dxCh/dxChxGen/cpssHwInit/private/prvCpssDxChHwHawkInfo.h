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
* @file prvCpssDxChHwHawkInfo.h
*
* @brief Private definition for the Hawk devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwHawkInfoh
#define __prvCpssDxChHwHawkInfoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvDxChHwRegAddrHawkDbInit function
* @endinternal
*
* @brief   init the base address manager of the Hawk device.
*         prvDxChHawkUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrHawkDbInit(GT_VOID);

/**
* @internal prvCpssHawkInitParamsSet function
* @endinternal
*
* @brief   Hawk : init the very first settings in the DB of the device.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssHawkInitParamsSet
(
    IN  GT_U8   devNum
);

GT_VOID prvCpssHawkNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwHawkInfoh */

