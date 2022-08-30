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
* @file prvCpssDxChTrunkHa.c
*
* @brief Trunk manager Ha support
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTrunkHah
#define __prvCpssDxChTrunkHah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>

/* macro to get trunkId status in skipListTrunkIdArry
   GT_TRUE  - trunk is in skip list
   GT_FALSE - trunk is not in skip list*/
#define PRV_CPSS_TRUNK_HA_TRUNK_SKIP_LIST_GET_MAC(trunkId)        \
        PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->skipListTrunkIdArry[trunkId]

/**
* @internal prvCpssDxChTrunkSyncSwHwForHa function
* @endinternal
*
* @brief  this function goes over all the devices and trunk groups defined in
*         the system, and for each one, check if SW DB matches HW DB,
*         in case SW doesnt match HW ,the trunk group gets overide with data from SW
*         in case the mismatch is only in inner order of group members the SW get overide
*         with data from HW
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,AC5P; AC5X; Harrier; Ironman;
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_BAD_BARAM             - wrong parameters.
* @retval GT_FAIL                  - on other error.
*
*/
GT_STATUS prvCpssDxChTrunkSyncSwHwForHa
(
    GT_VOID
);

/**
* @internal prvCpssDxChTrunkHaSkipListCheckAndSet function
* @endinternal
*
* @brief  this function set trunkId into skip list for high availability
*         trunkId is added only for sip6 and if the trunk set from low level api
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,AC5P; AC5X; Harrier; Ironman;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
*
*/
GT_STATUS prvCpssDxChTrunkHaSkipListCheckAndSet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChTrunkHah */


