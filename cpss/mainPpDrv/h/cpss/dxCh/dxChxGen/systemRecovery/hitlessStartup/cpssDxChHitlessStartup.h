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
* @file cpssDxChHitlessStartup.h
*
* @brief CPSS DxCh HitlessStartup facility API.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChHitlessStartuph
#define __cpssDxChHitlessStartuph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/generic/version/gtVersion.h>
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hitlessStartup/private/prvCpssDxChHitlessStartup.h>
#include <cpss/dxCh/dxChxGen/bootChannel/private/prvCpssDxChBootChannelHandler.h>

/**
* @internal cpssDxChHitlessStartupMiCompatibilityCheck function
* @endinternal
*
* @brief  This function gets micro init version and check if compatible with cpss version
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in]  devNum               - device number
* @param[in]  miVersionPtr         - (pointer to) MI version .
* @param[OUT] isCompatiblePtr      - (pointer to) if MI and cpss version are compatible .
*
* @retval GT_OK                      - on success
* @retval GT_FAIL                    - on case of error
* @retval GT_NOT_APPLICABLE_DEVICE   - not applicable device
* @retval GT_BAD_PARAM               - on wrong input
*
*/
GT_STATUS cpssDxChHitlessStartupMiCompatibilityCheck
(
    IN   GT_U8        devNum,
    IN   GT_CHAR     *miVersionPtr,
    OUT  GT_BOOL     *isCompatiblePtr
);

/**
* @internal cpssDxChHitlessStartupPortParamsGet function
* @endinternal
*
* @brief  This function gets link Status and port's params of the port configured by MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[out] portInfoPtr             - (pointer to) port info stc
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_SUPPORTED         - on not supported port mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHitlessStartupPortParamsGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT PRV_CPSS_BOOT_CH_PORT_STATUS_STC      *portInfoPtr
);

/**
* @internal cpssDxChHitlessStartupSyncPortManager function
* @endinternal
*
* @brief  sync port manager DB using data from MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - in case of error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChHitlessStartupSyncPortManager
(
    IN  GT_U8                                  devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChHitlessStartup */

