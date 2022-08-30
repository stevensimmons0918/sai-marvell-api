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
* @file cpssTmNodesRead.c
*
* @brief TM nodes configuration reading APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <tm_nodes_read.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/***************************************************************************
 * Read Configuration
 ***************************************************************************/

/**
* @internal internal_cpssTmQueueConfigurationRead function
* @endinternal
*
* @brief   Read queue software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
*
* @param[out] paramsPtr                - (pointer to) Queue parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmQueueConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_QUEUE_PARAMS_STC        *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_queue_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    ret = tm_read_queue_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    return rc;
}

/**
* @internal cpssTmQueueConfigurationRead function
* @endinternal
*
* @brief   Read queue software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
*
* @param[out] paramsPtr                - (pointer to) Queue parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmQueueConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_QUEUE_PARAMS_STC        *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmQueueConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmAnodeConfigurationRead function
* @endinternal
*
* @brief   Read A-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) A-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmAnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_A_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_a_node_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    ret = tm_read_a_node_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;
    for(i=0; i<8; i++)
        paramsPtr->schdModeArr[i] = prms.dwrr_priority[i];
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;
    return rc;
}

/**
* @internal cpssTmAnodeConfigurationRead function
* @endinternal
*
* @brief   Read A-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) A-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmAnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_A_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmAnodeConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmBnodeConfigurationRead function
* @endinternal
*
* @brief   Read B-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) B-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmBnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_B_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_b_node_params prms;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    ret = tm_read_b_node_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;
    for(i=0; i<8; i++)
        paramsPtr->schdModeArr[i] = prms.dwrr_priority[i];
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;
    return rc;
}

/**
* @internal cpssTmBnodeConfigurationRead function
* @endinternal
*
* @brief   Read B-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) B-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmBnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_B_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmBnodeConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmBnodeConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmCnodeConfigurationRead function
* @endinternal
*
* @brief   Read C-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) C-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmCnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_C_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_c_node_params prms;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    ret = tm_read_c_node_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->dropCosMap = prms.wred_cos;
    for(i=0; i<8; i++)
    {
        paramsPtr->dropProfileIndArr[i] = prms.wred_profile_ref[i];
        paramsPtr->schdModeArr[i]= prms.dwrr_priority[i];
    }
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;
    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;

    return rc;
}

/**
* @internal cpssTmCnodeConfigurationRead function
* @endinternal
*
* @brief   Read C-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) C-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_C_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmCnodeConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmCnodeConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmPortConfigurationRead function
* @endinternal
*
* @brief   Read port software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @param[out] paramsPtr                - (pointer to) Port parameters structure.
* @param[out] cosParamsPtr             - (pointer to) Port Drop per Cos structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmPortConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_PORT_PARAMS_STC         *paramsPtr,
    OUT CPSS_TM_PORT_DROP_PER_COS_STC   *cosParamsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_port_params prms;
    struct tm_port_drop_per_cos cos;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    ret = tm_read_port_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms, &cos);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->cirBw = prms.cir_bw;
    paramsPtr->eirBw = prms.eir_bw;
    paramsPtr->cbs = prms.cbs;
    paramsPtr->ebs = prms.ebs;
    for(i=0; i<8; i++) {
        paramsPtr->quantumArr[i] = prms.quantum[i];
        paramsPtr->schdModeArr[i] = prms.dwrr_priority[i];
    }
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;

    cosParamsPtr->dropCosMap = cos.wred_cos;
    for(i=0; i<8; i++)
        cosParamsPtr->dropProfileIndArr[i] = cos.wred_profile_ref[i];

    return rc;
}

/**
* @internal cpssTmPortConfigurationRead function
* @endinternal
*
* @brief   Read port software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @param[out] paramsPtr                - (pointer to) Port parameters structure.
* @param[out] cosParamsPtr             - (pointer to) Port Drop per Cos structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_PORT_PARAMS_STC         *paramsPtr,
    OUT CPSS_TM_PORT_DROP_PER_COS_STC   *cosParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmPortConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr, cosParamsPtr));

    rc = internal_cpssTmPortConfigurationRead(devNum, index, paramsPtr, cosParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr, cosParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

