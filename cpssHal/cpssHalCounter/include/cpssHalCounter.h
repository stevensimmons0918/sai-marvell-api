/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalConter.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for CNC counters.
*
* @version   01
********************************************************************************
*/
#ifndef __cpssHalCounterh
#define __cpssHalCounterh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "xpsCommon.h"
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>

/**
* @internal cpssHalCncBlockClientEnableAndBindSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncBlockClientEnableAndBindSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT            client,
    IN GT_BOOL                              enable,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    format
);

/**
* @internal cpssHalCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of device.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  devId                   - device number
* @param[out] enablePtr               - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncCounterClearByReadEnableGet
(
    IN  GT_U8                               devId,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssHalCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                    - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] counterIdx               - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] pktCnt                  - (pointer to) packet counter value.
* @param[out] counterPtr              - (pointer to) byte counter value.
* @param[out] maxVal                  - (pointer to) maximum counter value.

* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncCounterGet
(
    IN  GT_U32                               devId,
    IN  GT_U32                              blockNum,
    IN  GT_U32                              counterIdx,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    format,
    OUT uint64_t                            *pktCnt,
    OUT uint64_t                            *byteCnt,
    OUT uint64_t                            *maxVal
);

GT_STATUS cpssHalCncVlanClientIndexModeSet
(
    IN  GT_U32                             devId,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT indexMode
);

GT_STATUS cpssHalCncClientByteCountModeSet
(
    IN  GT_U32                             devId,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT countMode
);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __cpssHalCoounterh */

