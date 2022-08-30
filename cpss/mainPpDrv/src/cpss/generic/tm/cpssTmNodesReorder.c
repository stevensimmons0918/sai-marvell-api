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
* @file cpssTmNodesReorder.c
*
* @brief TM nodes reshuffling APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmNodesReorder.h>
#include <tm_nodes_reorder.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal internal_cpssTmNodesSwitch function
* @endinternal
*
* @brief   Switch children between two nodes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Node level.
* @param[in] nodeAIndex               - Node A index in switch.
* @param[in] nodeBIndex               - Node B index in switch.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmNodesSwitch
(
    IN GT_U8                        devNum,
    IN CPSS_TM_LEVEL_ENT            level,
    IN GT_U32                       nodeAIndex,
    IN GT_U32                       nodeBIndex
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                      CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    ret = tm_nodes_switch(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, level,
                         (uint16_t)nodeAIndex,
                         (uint16_t)nodeBIndex);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmNodesSwitch function
* @endinternal
*
* @brief   Switch children between two nodes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Node level.
* @param[in] nodeAIndex               - Node A index in switch.
* @param[in] nodeBIndex               - Node B index in switch.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesSwitch
(
    IN GT_U8                        devNum,
    IN CPSS_TM_LEVEL_ENT            level,
    IN GT_U32                       nodeAIndex,
    IN GT_U32                       nodeBIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNodesSwitch);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, nodeAIndex, nodeBIndex));

    rc = internal_cpssTmNodesSwitch(devNum, level, nodeAIndex, nodeBIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, nodeAIndex, nodeBIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

