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
* @file cpssTmNodesCreate.c
*
* @brief TM nodes creation APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <tm_nodes_create.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/***************************************************************************
 * Help Function
 ***************************************************************************/

/**
* @internal prvCpssTmPortParamsCpy function
* @endinternal
*
* @brief   Copy CPSS Port structure to TM Port structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] paramsPtr                - CPSS Port parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmPortParamsCpy
(
    IN  CPSS_TM_PORT_PARAMS_STC     *paramsPtr,
    OUT struct tm_port_params       *prmsPtr
)
{
    int i;

    prmsPtr->cir_bw = paramsPtr->cirBw;
    prmsPtr->eir_bw = paramsPtr->eirBw;
    prmsPtr->cbs = paramsPtr->cbs;
    prmsPtr->ebs = paramsPtr->ebs;
    for(i=0; i<8; i++) {
        prmsPtr->quantum[i] = (uint16_t)paramsPtr->quantumArr[i];
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }
    prmsPtr->wred_profile_ref = (uint8_t)paramsPtr->dropProfileInd;
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssTmPortDropPerCosParamsCpy function
* @endinternal
*
* @brief   Copy CPSS Port Drop per Cos structure to TM Port Drop per Cos structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] paramsPtr                - CPSS Port Drop per Cos parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmPortDropPerCosParamsCpy
(
    IN  CPSS_TM_PORT_DROP_PER_COS_STC     *paramsPtr,
    OUT struct tm_port_drop_per_cos       *prmsPtr
)
{
    int i;

    for(i=0; i<8; i++)
        prmsPtr->wred_profile_ref[i] = (uint8_t)paramsPtr->dropProfileIndArr[i];
    prmsPtr->wred_cos = (uint8_t)paramsPtr->dropCosMap;
}


/**
* @internal prvCpssTmCNodeParamsCpy function
* @endinternal
*
* @brief   Copy CPSS C-Node structure to TM C-Node structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] paramsPtr                - CPSS C-Node parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmCNodeParamsCpy
(
    IN  CPSS_TM_C_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_c_node_params        *prmsPtr
)
{
    int i;

    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    for(i=0; i<8; i++)
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    prmsPtr->wred_cos = (uint8_t)paramsPtr->dropCosMap;
    for(i=0; i<8; i++)
        prmsPtr->wred_profile_ref[i] = (uint8_t)paramsPtr->dropProfileIndArr[i];
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssTmBNodeParamsCpy function
* @endinternal
*
* @brief   Copy CPSS B-Node structure to TM B-Node structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] paramsPtr                - CPSS B-Node parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmBNodeParamsCpy
(
    IN  CPSS_TM_B_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_b_node_params        *prmsPtr
)
{
    int i;

    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    for(i=0; i<8; i++)
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    prmsPtr->wred_profile_ref = (uint16_t)paramsPtr->dropProfileInd;
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssTmANodeParamsCpy function
* @endinternal
*
* @brief   Copy CPSS A-Node structure to TM A-Node structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] paramsPtr                - CPSS A-Node parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmANodeParamsCpy
(
    IN  CPSS_TM_A_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_a_node_params        *prmsPtr
)
{
    int i;

    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    for(i=0; i<8; i++)
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    prmsPtr->wred_profile_ref = (uint16_t)paramsPtr->dropProfileInd;
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssTmQueueParamsCpy function
* @endinternal
*
* @brief   Copy CPSS Queue structure to TM Queue structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] paramsPtr                - CPSS Queue parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmQueueParamsCpy
(
    IN  CPSS_TM_QUEUE_PARAMS_STC      *paramsPtr,
    OUT struct tm_queue_params        *prmsPtr
)
{
    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    prmsPtr->wred_profile_ref = (uint16_t)paramsPtr->dropProfileInd;
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
}


/***************************************************************************
 * Port Creation
 ***************************************************************************/

/**
* @internal internal_cpssTmPortCreate function
* @endinternal
*
* @brief   Create Port and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
* @param[in] cNodesNum                - Number of C-nodes under port.
* @param[in] bNodesNum                - Number of B-nodes under port.
* @param[in] aNodesNum                - Number of A-nodes under port.
* @param[in] queuesNum                - Number of Queues under port.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*       2. Valid number of nodes per level must be multiple of number nodes
*       on upper level (parent nodes) in case of equal distribution. And
*       in case of unequal distribution all parent nodes except the last
*       one must have the same children range and the last parent node -
*       number of children less that the range. In case of not valid
*       number will be returned GT_BAD_PARAM error code.
*       3. If port will be used for transparent queues, set the c/b/aNodesNum = 1.
*
*/
static GT_STATUS internal_cpssTmPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_port_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    /* check that the port index is valid */
    if(portInd >71 && portInd<128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    prvCpssTmPortParamsCpy(paramsPtr, &prms);
    ret = tm_create_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                         (uint8_t)portInd,
                         &prms,
                         (uint16_t)cNodesNum,
                         (uint16_t)bNodesNum,
                         (uint16_t)aNodesNum,
                         queuesNum);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmPortCreate function
* @endinternal
*
* @brief   Create Port and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
* @param[in] cNodesNum                - Number of C-nodes under port.
* @param[in] bNodesNum                - Number of B-nodes under port.
* @param[in] aNodesNum                - Number of A-nodes under port.
* @param[in] queuesNum                - Number of Queues under port.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*       2. Valid number of nodes per level must be multiple of number nodes
*       on upper level (parent nodes) in case of equal distribution. And
*       in case of unequal distribution all parent nodes except the last
*       one must have the same children range and the last parent node -
*       number of children less that the range. In case of not valid
*       number will be returned GT_BAD_PARAM error code.
*       3. If port will be used for transparent queues, set the c/b/aNodesNum = 1.
*
*/
GT_STATUS cpssTmPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum));

    rc = internal_cpssTmPortCreate(devNum, portInd, paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmAsymPortCreate function
* @endinternal
*
* @brief   Create Port with assymetric sub-tree and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*
*/
static GT_STATUS internal_cpssTmAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_port_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    /* check that the port index is valid */
    if(portInd >71 && portInd<128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssTmPortParamsCpy(paramsPtr, &prms);
    ret = tm_create_asym_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                              (uint8_t)portInd,
                              &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmAsymPortCreate function
* @endinternal
*
* @brief   Create Port with assymetric sub-tree and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*
*/
GT_STATUS cpssTmAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAsymPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, paramsPtr));

    rc = internal_cpssTmAsymPortCreate(devNum, portInd, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmPortDropPerCosConfig function
* @endinternal
*
* @brief   Configure Port's Drop per Cos and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port Drop per Cos parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmPortDropPerCosConfig
(
    IN GT_U8                         devNum,
    IN GT_U32                        portInd,
    IN CPSS_TM_PORT_DROP_PER_COS_STC *paramsPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_port_drop_per_cos prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    /* check that the port index is valid */
    if(portInd >71 && portInd<128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssTmPortDropPerCosParamsCpy(paramsPtr, &prms);
    ret = tm_config_port_drop_per_cos(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                              (uint8_t)portInd,
                              &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmPortDropPerCosConfig function
* @endinternal
*
* @brief   Configure Port's Drop per Cos and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port Drop per Cos parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortDropPerCosConfig
(
    IN GT_U8                         devNum,
    IN GT_U32                        portInd,
    IN CPSS_TM_PORT_DROP_PER_COS_STC *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmPortDropPerCosConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, paramsPtr));

    rc = internal_cpssTmPortDropPerCosConfig(devNum, portInd, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/***************************************************************************
 * Queue Creation
 ***************************************************************************/

/**
* @internal internal_cpssTmQueueToPortCreate function
* @endinternal
*
* @brief   Create path from Queue to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmQueueToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;
    struct tm_a_node_params a_prms;
    struct tm_b_node_params b_prms;
    struct tm_c_node_params c_prms;
    uint32_t q_index;
    uint32_t a_index;
    uint32_t b_index;
    uint32_t c_index;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(cNodeIndPtr);

    prvCpssTmQueueParamsCpy(qParamsPtr, &q_prms);
    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    prvCpssTmBNodeParamsCpy(bParamsPtr, &b_prms);
    prvCpssTmCNodeParamsCpy(cParamsPtr, &c_prms);
    ret = tm_create_queue_to_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                  (uint8_t)portInd,
                                  &q_prms,
                                  &a_prms,
                                  &b_prms,
                                  &c_prms,
                                  &q_index,
                                  &a_index,
                                  &b_index,
                                  &c_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *queueIndPtr = q_index;
    *aNodeIndPtr = a_index;
    *bNodeIndPtr = b_index;
    *cNodeIndPtr = c_index;
    return rc;
}

/**
* @internal cpssTmQueueToPortCreate function
* @endinternal
*
* @brief   Create path from Queue to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueToPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, qParamsPtr, aParamsPtr, bParamsPtr, cParamsPtr, queueIndPtr, aNodeIndPtr, bNodeIndPtr, cNodeIndPtr));

    rc = internal_cpssTmQueueToPortCreate(devNum, portInd, qParamsPtr, aParamsPtr, bParamsPtr, cParamsPtr, queueIndPtr, aNodeIndPtr, bNodeIndPtr, cNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, qParamsPtr, aParamsPtr, bParamsPtr, cParamsPtr, queueIndPtr, aNodeIndPtr, bNodeIndPtr, cNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmTransQueueToPortCreate function
* @endinternal
*
* @brief   Create transparent path from Queue to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. This API introduces 'transparent' concept to TM nodes
*       structure that deals with Queues and Ports only. In this case no
*       configuration is needed for A,B,C level nodes, this nodes
*       are created automatically (one C-node, one B-node and one
*       A-node) and they are 'transparent' from the system point of
*       view. Transparent path can be created under symmetric port
*       only. To delete Queue from the structure - use
*       'cpssTmNodesCtlNodeDelete' API with level CPSS_TM_LEVEL_Q_E. To update queue
*       parameters - use 'cpssTmQueueUpdate' API. To delete Port from
*       the structure use 'cpssTmNodesCtlTransPortDelete' API. Applying any
*       other APIs on nodes underlying the port can cause unexpected
*       behavior of the system.
*       2. To indicate that no shaping is needed to the queue, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmTransQueueToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    OUT GT_U32                          *queueIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;
    uint32_t q_index;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);

    prvCpssTmQueueParamsCpy(qParamsPtr, &q_prms);
    ret = tm_create_trans_queue_to_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                        (uint8_t)portInd,
                                        &q_prms,
                                        &q_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *queueIndPtr = q_index;
    return rc;
}

/**
* @internal cpssTmTransQueueToPortCreate function
* @endinternal
*
* @brief   Create transparent path from Queue to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. This API introduces 'transparent' concept to TM nodes
*       structure that deals with Queues and Ports only. In this case no
*       configuration is needed for A,B,C level nodes, this nodes
*       are created automatically (one C-node, one B-node and one
*       A-node) and they are 'transparent' from the system point of
*       view. Transparent path can be created under symmetric port
*       only. To delete Queue from the structure - use
*       'cpssTmNodesCtlNodeDelete' API with level CPSS_TM_LEVEL_Q_E. To update queue
*       parameters - use 'cpssTmQueueUpdate' API. To delete Port from
*       the structure use 'cpssTmNodesCtlTransPortDelete' API. Applying any
*       other APIs on nodes underlying the port can cause unexpected
*       behavior of the system.
*       2. To indicate that no shaping is needed to the queue, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmTransQueueToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    OUT GT_U32                          *queueIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmTransQueueToPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, qParamsPtr, queueIndPtr));

    rc = internal_cpssTmTransQueueToPortCreate(devNum, portInd, qParamsPtr, queueIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, qParamsPtr, queueIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmQueueToCnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeIndex not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmQueueToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;
    struct tm_a_node_params a_prms;
    struct tm_b_node_params b_prms;
    uint32_t q_index;
    uint32_t a_index;
    uint32_t b_index;

    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    prvCpssTmQueueParamsCpy(qParamsPtr, &q_prms);
    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    prvCpssTmBNodeParamsCpy(bParamsPtr, &b_prms);
    ret = tm_create_queue_to_c_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    cNodeInd,
                                    &q_prms,
                                    &a_prms,
                                    &b_prms,
                                    &q_index,
                                    &a_index,
                                    &b_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *queueIndPtr = q_index;
    *aNodeIndPtr = a_index;
    *bNodeIndPtr = b_index;
    return rc;
}

/**
* @internal cpssTmQueueToCnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeIndex not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueToCnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cNodeInd, qParamsPtr, aParamsPtr, bParamsPtr, queueIndPtr, aNodeIndPtr, bNodeIndPtr));

    rc = internal_cpssTmQueueToCnodeCreate(devNum, cNodeInd, qParamsPtr, aParamsPtr, bParamsPtr, queueIndPtr, aNodeIndPtr, bNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cNodeInd, qParamsPtr, aParamsPtr, bParamsPtr, queueIndPtr, aNodeIndPtr, bNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmQueueToBnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to B-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmQueueToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;
    struct tm_a_node_params a_prms;
    uint32_t q_index;
    uint32_t a_index;

    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmQueueParamsCpy(qParamsPtr, &q_prms);
    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    ret = tm_create_queue_to_b_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    bNodeInd,
                                    &q_prms,
                                    &a_prms,
                                    &q_index,
                                    &a_index);

    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *queueIndPtr = q_index;
    *aNodeIndPtr = a_index;
    return rc;
}

/**
* @internal cpssTmQueueToBnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to B-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueToBnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bNodeInd, qParamsPtr, aParamsPtr, queueIndPtr, aNodeIndPtr));

    rc = internal_cpssTmQueueToBnodeCreate(devNum, bNodeInd, qParamsPtr, aParamsPtr, queueIndPtr, aNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bNodeInd, qParamsPtr, aParamsPtr, queueIndPtr, aNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmQueueToAnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on aNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmQueueToAnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    OUT GT_U32                          *queueIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;
    uint32_t q_index;

    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    prvCpssTmQueueParamsCpy(qParamsPtr, &q_prms);
    ret = tm_create_queue_to_a_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    aNodeInd,
                                    &q_prms,
                                    &q_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *queueIndPtr = q_index;
    return rc;
}

/**
* @internal cpssTmQueueToAnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on aNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToAnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    OUT GT_U32                          *queueIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueToAnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, aNodeInd, qParamsPtr, queueIndPtr));

    rc = internal_cpssTmQueueToAnodeCreate(devNum, aNodeInd, qParamsPtr, queueIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, aNodeInd, qParamsPtr, queueIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssTmQueueToAnodeByIndexCreate function
* @endinternal
*
* @brief   Create path from selected Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node parent index.
* @param[in] qParamsPtr               - pointer to Queue parameters structure.
* @param[in] queueInd                 - Index of Queue to init (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on aNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1.To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*       2. The selected queue must previously allocated to A-node parent
*
*/
static GT_STATUS internal_cpssTmQueueToAnodeByIndexCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  GT_U32                          queueInd
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;

    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    prvCpssTmQueueParamsCpy(qParamsPtr, &q_prms);
    ret = tm_init_selected_queue_of_a_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    aNodeInd,
                                    &q_prms,
                                    queueInd);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    return rc;
}

/**
* @internal cpssTmQueueToAnodeByIndexCreate function
* @endinternal
*
* @brief   Create path from selected Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node parent index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] queueInd                 - Index of Queue to init (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on aNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1.To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*       2. The selected queue must previously allocated to A-node parent
*
*/
GT_STATUS cpssTmQueueToAnodeByIndexCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  GT_U32                          queueInd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueToAnodeByIndexCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, aNodeInd, qParamsPtr, queueInd));

    rc = internal_cpssTmQueueToAnodeByIndexCreate(devNum, aNodeInd, qParamsPtr, queueInd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, aNodeInd, qParamsPtr, queueInd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/***************************************************************************
 * A-node Creation
 ***************************************************************************/

/**
* @internal internal_cpssTmAnodeToPortCreate function
* @endinternal
*
* @brief   Create path from A-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmAnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_a_node_params a_prms;
    struct tm_b_node_params b_prms;
    struct tm_c_node_params c_prms;
    uint32_t a_index;
    uint32_t b_index;
    uint32_t c_index;


    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(cNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    prvCpssTmBNodeParamsCpy(bParamsPtr, &b_prms);
    prvCpssTmCNodeParamsCpy(cParamsPtr, &c_prms);
    ret = tm_create_a_node_to_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                   (uint8_t)portInd,
                                   &a_prms,
                                   &b_prms,
                                   &c_prms,
                                   &a_index,
                                   &b_index,
                                   &c_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *aNodeIndPtr = a_index;
    *bNodeIndPtr = b_index;
    *cNodeIndPtr = c_index;
    return rc;
}

/**
* @internal cpssTmAnodeToPortCreate function
* @endinternal
*
* @brief   Create path from A-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeToPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, aParamsPtr, bParamsPtr, cParamsPtr, aNodeIndPtr, bNodeIndPtr, cNodeIndPtr));

    rc = internal_cpssTmAnodeToPortCreate(devNum, portInd, aParamsPtr, bParamsPtr, cParamsPtr, aNodeIndPtr, bNodeIndPtr, cNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, aParamsPtr, bParamsPtr, cParamsPtr, aNodeIndPtr, bNodeIndPtr, cNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmAnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmAnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_a_node_params a_prms;
    struct tm_b_node_params b_prms;
    uint32_t a_index;
    uint32_t b_index;

    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    prvCpssTmBNodeParamsCpy(bParamsPtr, &b_prms);
    ret = tm_create_a_node_to_c_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                     cNodeInd,
                                     &a_prms,
                                     &b_prms,
                                     &a_index,
                                     &b_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *aNodeIndPtr = a_index;
    *bNodeIndPtr = b_index;
    return rc;
}

/**
* @internal cpssTmAnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeToCnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cNodeInd, aParamsPtr, bParamsPtr, aNodeIndPtr, bNodeIndPtr));

    rc = internal_cpssTmAnodeToCnodeCreate(devNum, cNodeInd, aParamsPtr, bParamsPtr, aNodeIndPtr, bNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cNodeInd, aParamsPtr, bParamsPtr, aNodeIndPtr, bNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmAnodeToBnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmAnodeToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    OUT GT_U32                          *aNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_a_node_params a_prms;
    uint32_t a_index;

    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    ret = tm_create_a_node_to_b_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                     bNodeInd,
                                     &a_prms,
                                     &a_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *aNodeIndPtr = a_index;
    return rc;
}

/**
* @internal cpssTmAnodeToBnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    OUT GT_U32                          *aNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeToBnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bNodeInd, aParamsPtr, aNodeIndPtr));

    rc = internal_cpssTmAnodeToBnodeCreate(devNum, bNodeInd, aParamsPtr, aNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bNodeInd, aParamsPtr, aNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssTmAnodeWithQueueRangeToBnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node & allocates user defined queue children pool
*         Not applicable for symmetric port tree creation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node parent index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] firstQueueInRange        - index of first queue in required queue range
* @param[in] queueRangeSize           - queue range size
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on resource allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmAnodeWithQueueRangeToBnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  GT_U32                      firstQueueInRange,
    IN  GT_U32                      queueRangeSize,
    OUT GT_U32                      *aNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_a_node_params a_prms;
    uint32_t a_index;

    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmANodeParamsCpy(aParamsPtr, &a_prms);
    ret = tm_create_a_node_with_queue_range_to_b_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                                      bNodeInd,
                                                      &a_prms,
                                                      firstQueueInRange,
                                                      queueRangeSize,
                                                      &a_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *aNodeIndPtr = a_index;
    return rc;
}


/**
* @internal cpssTmAnodeToBnodeWithQueuePoolCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node & allocates user defined queue children pool
*         Not applicable for symmetric port tree creation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node parent index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] firstQueueInRange        - index of first queue in required queue range
* @param[in] queueRangeSize           - queue range size
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on resource allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToBnodeWithQueuePoolCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  GT_U32                      firstQueueInRange,
    IN  GT_U32                      queueRangeSize,
    OUT GT_U32                      *aNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeToBnodeWithQueuePoolCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bNodeInd, aParamsPtr, firstQueueInRange , queueRangeSize, aNodeIndPtr ));

    rc = internal_cpssTmAnodeWithQueueRangeToBnodeCreate(devNum, bNodeInd, aParamsPtr, firstQueueInRange, queueRangeSize, aNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bNodeInd, aParamsPtr,  firstQueueInRange , queueRangeSize, aNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/***************************************************************************
 * B-node Creation
 ***************************************************************************/

/**
* @internal internal_cpssTmBnodeToPortCreate function
* @endinternal
*
* @brief   Create path from B-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmBnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_b_node_params b_prms;
    struct tm_c_node_params c_prms;
    uint32_t b_index;
    uint32_t c_index;

    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmBNodeParamsCpy(bParamsPtr, &b_prms);
    prvCpssTmCNodeParamsCpy(cParamsPtr, &c_prms);
    ret = tm_create_b_node_to_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                   (uint8_t)portInd,
                                   &b_prms,
                                   &c_prms,
                                   &b_index,
                                   &c_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *bNodeIndPtr = b_index;
    *cNodeIndPtr = c_index;
    return rc;
}

/**
* @internal cpssTmBnodeToPortCreate function
* @endinternal
*
* @brief   Create path from B-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmBnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmBnodeToPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, bParamsPtr, cParamsPtr, bNodeIndPtr, cNodeIndPtr));

    rc = internal_cpssTmBnodeToPortCreate(devNum, portInd, bParamsPtr, cParamsPtr, bNodeIndPtr, cNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, bParamsPtr, cParamsPtr, bNodeIndPtr, cNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmBnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from B-node to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmBnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *bNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_b_node_params b_prms;
    uint32_t b_index;

    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmBNodeParamsCpy(bParamsPtr, &b_prms);
    ret = tm_create_b_node_to_c_node(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                     cNodeInd,
                                     &b_prms,
                                     &b_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *bNodeIndPtr = b_index;
    return rc;
}

/**
* @internal cpssTmBnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from B-node to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmBnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *bNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmBnodeToCnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cNodeInd, bParamsPtr, bNodeIndPtr));

    rc = internal_cpssTmBnodeToCnodeCreate(devNum, cNodeInd, bParamsPtr, bNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cNodeInd, bParamsPtr, bNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/***************************************************************************
 * C-node Creation
 ***************************************************************************/

/**
* @internal internal_cpssTmCnodeToPortCreate function
* @endinternal
*
* @brief   Create path from C-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
static GT_STATUS internal_cpssTmCnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_c_node_params c_prms;
    uint32_t c_index;

    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cNodeIndPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    prvCpssTmCNodeParamsCpy(cParamsPtr, &c_prms);
    ret = tm_create_c_node_to_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                   (uint8_t)portInd,
                                   &c_prms,
                                   &c_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *cNodeIndPtr = c_index;
    return rc;
}

/**
* @internal cpssTmCnodeToPortCreate function
* @endinternal
*
* @brief   Create path from C-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmCnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmCnodeToPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, cParamsPtr, cNodeIndPtr));

    rc = internal_cpssTmCnodeToPortCreate(devNum, portInd, cParamsPtr, cNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, cParamsPtr, cNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

