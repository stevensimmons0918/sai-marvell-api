/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file cpssDxChIpfixManager.c
*
* @brief CPSS APIs implementation for Ipfix Manager.
*
* @version   1
*********************************************************************************
**/

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>

#define PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(_dev)       \
    if(!PRV_CPSS_DXCH_FALCON_CHECK_MAC(_dev))                      \
    {                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,   \
            "IPFIX Manager not supported for dev: %d",_dev);      \
    }

#define PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(_dbPtr)                 \
    if(_dbPtr == NULL)                                                       \
    {                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG); \
    }

/**
 * @internal internal_cpssDxChIpfixManagerCreate function
 * @endinternal
 *
 * @brief Create IPFIX Manager and IPFIX Assistant FW entities
 *        per device, and configure the running parameters
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device number
 * @param[in] attributesPtr     - (pointer to) ipfix manager attributes
 *
 * @retVal GT_OK                - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  *attributesPtr
)
{
    GT_STATUS                           rc;
    GT_UINTPTR                          fwChannel;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC  *ipfixMgrDbPtr = NULL;
    CPSS_DXCH_NET_DSA_PARAMS_STC        dsaInfo;
    GT_HW_DEV_NUM                       hwDevNum;
    GT_U32                              portGroupId;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(attributesPtr);

    /* check whether service cpu number is valid */
    if ((attributesPtr->serviceCpuNum < PRV_CPSS_DXCH_SERVICE_CPU_NUM_MIN_CNS) ||
        (attributesPtr->serviceCpuNum > PRV_CPSS_DXCH_SERVICE_CPU_NUM_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "serviceCpuNum should be in rage {%d, %d}",
                                      PRV_CPSS_DXCH_SERVICE_CPU_NUM_MIN_CNS,
                                      PRV_CPSS_DXCH_SERVICE_CPU_NUM_MAX_CNS);
    }

    /* Get handle for ipfix manager Db */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    if (ipfixMgrDbPtr != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CREATE_ERROR, LOG_ERROR_NO_MSG);
    }

    /* Reset the specified service CPU and load IPFIX Assistant FW image into its SRAM */
    rc = prvCpssIpfixManagerToSrvCpuLoad(devNum, attributesPtr->serviceCpuNum, &fwChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* Allocate IPFIX manager database memory */
    PRV_CPSS_DXCH_PP_MAC(devNum)->ipfixMgrDbPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC));
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    cpssOsMemSet(ipfixMgrDbPtr, 0, sizeof(PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC));

    ipfixMgrDbPtr->ipc.fwChannel                 = fwChannel;
    ipfixMgrDbPtr->attributes.serviceCpuNum      = attributesPtr->serviceCpuNum;

    /* Set default global config parameters */
    ipfixMgrDbPtr->globalConfig.policerStage        = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    ipfixMgrDbPtr->globalConfig.ipfixEntriesPerFlow = CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E;
    ipfixMgrDbPtr->globalConfig.ipfixDataQueueNum   = PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_DATA_QUEUE_NUM_CNS;
    ipfixMgrDbPtr->globalConfig.dataPktMtu          = PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_DATA_PKT_MTU_CNS;
    ipfixMgrDbPtr->globalConfig.idleTimeout         = PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_IDLE_TIMEOUT_CNS;
    ipfixMgrDbPtr->globalConfig.activeTimeout       = PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_ACTIVE_TIMEOUT_CNS;
    ipfixMgrDbPtr->globalConfig.agingOffload        = GT_FALSE;
    ipfixMgrDbPtr->globalConfig.monitoringOffload   = GT_FALSE;
    ipfixMgrDbPtr->globalConfig.deltaMode           = GT_FALSE;

    for (portGroupId = 0; portGroupId < CPSS_DXCH_MAX_PORT_GROUPS_CNS; portGroupId+=2)
    {
        ipfixMgrDbPtr->portGroupConfig[portGroupId].maxIpfixIndex =
                      PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_PORT_GROUP_CNS;
    }

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Build Extended TO_CPU DSA tag; 4 words */
    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));
    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
    dsaInfo.dsaInfo.toCpu.hwDevNum = (hwDevNum + 1) % 32;
    dsaInfo.dsaInfo.toCpu.cpuCode = CPSS_DXCH_IPFIX_MANAGER_DATA_PKTS_CPU_CODE_DEFAULT;

    /* build DSA Tag buffer */
    rc = cpssDxChNetIfDsaTagBuild(devNum,
        &dsaInfo,
        &ipfixMgrDbPtr->globalConfig.txDsaTag[0]);

    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
 * @internal cpssDxChIpfixManagerCreate function
 * @endinternal
 *
 * @brief Create IPFIX Manager and IPFIX Assistant FW entities
 *        per device, and configure the running parameters
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device number
 * @param[in] attributesPtr     - (pointer to) ipfix manager attributes
 *
 * @retVal GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  *attributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, attributesPtr));

    rc = internal_cpssDxChIpfixManagerCreate(devNum, attributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, attributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerEnableSet function
 * @endinternal
 *
 * @brief Enable/Disable IPFIX Manager
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum       - device number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retVal GT_OK                - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ipfixEnable
)
{
    GT_STATUS   rc = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC      *ipfixMgrDbPtr = NULL;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;
    GT_U32                                   i;
    GT_U32                                   netIfNum = 0;
    GT_U32                                   mgNum = 0;
    GT_U32                                   localQueueNum = 0;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    ipfixMgrDbPtr->ipfixEnable = ipfixEnable;

    /* create IPC message for configSet */
    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.configSet.opcode              = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_SET_E;
    ipc_msg.configSet.ipfixEnable         = ipfixEnable;
    ipc_msg.configSet.policerStage        = ipfixMgrDbPtr->globalConfig.policerStage;
    ipc_msg.configSet.ipfixEntriesPerFlow = ipfixMgrDbPtr->globalConfig.ipfixEntriesPerFlow;
    ipc_msg.configSet.dataPktMtu          = ipfixMgrDbPtr->globalConfig.dataPktMtu;
    ipc_msg.configSet.idleTimeout         = ipfixMgrDbPtr->globalConfig.idleTimeout;
    ipc_msg.configSet.activeTimeout       = ipfixMgrDbPtr->globalConfig.activeTimeout;
    ipc_msg.configSet.agingOffload        = ipfixMgrDbPtr->globalConfig.agingOffload;
    ipc_msg.configSet.monitoringOffload   = ipfixMgrDbPtr->globalConfig.monitoringOffload;
    ipc_msg.configSet.deltaMode           = ipfixMgrDbPtr->globalConfig.deltaMode;

    cpssOsMemCpy(ipc_msg.configSet.txDsaTag, ipfixMgrDbPtr->globalConfig.txDsaTag,
                 sizeof(ipc_msg.configSet.txDsaTag));

    netIfNum      = ipfixMgrDbPtr->globalConfig.ipfixDataQueueNum >> 3;
    localQueueNum = ipfixMgrDbPtr->globalConfig.ipfixDataQueueNum & 0x7;
    rc = prvCpssDxChNetifCheckNetifNumAndConvertToMgUnitId(devNum, netIfNum, &mgNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    ipc_msg.configSet.mgNum               = mgNum;
    ipc_msg.configSet.localQueueNum       = localQueueNum;

    for (i = 0; i < CPSS_DXCH_MAX_PORT_GROUPS_CNS; i+=2)
    {
        ipc_msg.configSet.maxIpfixIndex[i] = ipfixMgrDbPtr->portGroupConfig[i].maxIpfixIndex;
    }

    rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Disabling the below events generated by trapped packets to FW as Queue#6 is meant for
     * message packet and is not meant for application usage.
     */
    rc = cpssEventDeviceMaskSet(devNum,
                                CPSS_PP_RX_BUFFER_QUEUE0_E + localQueueNum,
                                CPSS_EVENT_MASK_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
                     __FUNCNAME__, __LINE__, rc, devNum, CPSS_PP_RX_BUFFER_QUEUE6_E);
        return rc;
    }

    rc = cpssEventDeviceMaskSet(devNum,
                                CPSS_PP_RX_ERR_QUEUE0_E + localQueueNum,
                                CPSS_EVENT_MASK_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
                     __FUNCNAME__, __LINE__, rc, devNum, CPSS_PP_RX_ERR_QUEUE6_E);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal cpssDxChIpfixManagerEnableSet function
 * @endinternal
 *
 * @brief Enable/Disable IPFIX Manager
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum       - device number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retVal GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ipfixEnable
)
{
    GT_STATUS   rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ipfixEnable));

    rc = internal_cpssDxChIpfixManagerEnableSet(devNum, ipfixEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ipfixEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerDelete function
 * @endinternal
 *
 * @brief Delete IPFIX Manager
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device number
 *
 * @retVal GT_OK                - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerDelete
(
    IN  GT_U8                                   devNum
)
{
    GT_STATUS                           rc             = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC  *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    if (ipfixMgrDbPtr->ipfixEnable == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                      "IPFIX Manager must be disabled first");
    }

    /* Free service cpu */
    rc = prvCpssGenericSrvCpuRemove(devNum, ipfixMgrDbPtr->attributes.serviceCpuNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    if (ipfixMgrDbPtr != NULL)
    {
        /* delete ipfix manager */
        cpssOsFree(ipfixMgrDbPtr);

        PRV_CPSS_DXCH_PP_MAC(devNum)->ipfixMgrDbPtr = NULL;
    }

    return GT_OK;
}

/**
 * @internal cpssDxChIpfixManagerDelete function
 * @endinternal
 *
 * @brief Delete IPFIX Manager
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device number
 * @param[in] attributesPtr     - (pointer to) ipfix manager attributes
 *
 * @retVal GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerDelete
(
    IN  GT_U8                                   devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChIpfixManagerDelete(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerGlobalConfigSet function
 * @endinternal
 *
 * @brief  Configure IPFIX Manager Global configuration and its IPFIX Assistant
 *         FW on a specific device
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device Number
 * @param[in] globalCfgPtr      - (pointer to) structure with ipfix manager
 *                                global configuration.
 *
 * @retval  GT_OK               - on success.
 */
static GT_STATUS internal_cpssDxChIpfixManagerGlobalConfigSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC  *globalCfgPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC      *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    /* Check whether globalCfgPtr is null */
    CPSS_NULL_PTR_CHECK_MAC(globalCfgPtr);

    /* check ipfixDataQueueNum is valid */
    if ((globalCfgPtr->ipfixDataQueueNum >= PRV_CPSS_DXCH_SDMA_QUEUE_MAX_CNS) ||
        (globalCfgPtr->idleTimeout >
         PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_IDLE_TIMEOUT_MAX_CNS) ||
        (globalCfgPtr->activeTimeout >
         PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_ACTIVE_TIMEOUT_MAX_CNS) ||
        (globalCfgPtr->dataPktMtu >
         PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_DATA_PKT_MTU_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if ((globalCfgPtr->agingOffload == GT_TRUE) ||
        (globalCfgPtr->monitoringOffload == GT_TRUE) ||
        (globalCfgPtr->ipfixEntriesPerFlow !=
         CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E) ||
        (globalCfgPtr->deltaMode == GT_TRUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                      "agingOffload/monitoringOffload not supported");
    }

    /* copy the global config to ipfix manager database */
    cpssOsMemCpy(&ipfixMgrDbPtr->globalConfig, globalCfgPtr,
                 sizeof(CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC));

    return rc;
}

/**
 * @internal cpssDxChIpfixManagerGlobalConfigSet function
 * @endinternal
 *
 * @brief  Configure IPFIX Manager Global configuration and its IPFIX Assistant
 *         FW on a specific device
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device Number
 * @param[in] globalCfgPtr      - (pointer to) structure with ipfix manager
 *                                global configuration.
 *
 * @retval  GT_OK               - on success.
 */
GT_STATUS cpssDxChIpfixManagerGlobalConfigSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC  *globalCfgPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerGlobalConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalCfgPtr));

    rc = internal_cpssDxChIpfixManagerGlobalConfigSet(devNum, globalCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalCfgPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerPortGroupConfigSet function
 * @endinternal
 *
 * @brief  Configure IPFIX Manager port group configuration and its IPFIX Assistant
 *         FW on a specific device
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device Number
 * @param[in] portGroupsBmp     - port groups bitmap
 * @param[in] portGroupCfgPtr    - (pointer to) structure with ipfix manager
 *                                port group configuration.
 *
 * @retval  GT_OK               - on success.
 */
static GT_STATUS internal_cpssDxChIpfixManagerPortGroupConfigSet
(
    IN  GT_U8                                          devNum,
    IN  GT_PORT_GROUPS_BMP                             portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC  *portGroupCfgPtr
)
{
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC      *ipfixMgrDbPtr = NULL;
    GT_U32                                   i;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(portGroupCfgPtr);

    /* check whether maxIpfixIndex is valid */
    if (portGroupCfgPtr->maxIpfixIndex > CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    for (i = 0; i < CPSS_DXCH_MAX_PORT_GROUPS_CNS; i+=2)
    {
        if (1 == ((portGroupsBmp >> i) & 1))
        {
            ipfixMgrDbPtr->portGroupConfig[i].maxIpfixIndex = portGroupCfgPtr->maxIpfixIndex;
        }
    }

    return GT_OK;
}

/**
 * @internal cpssDxChIpfixManagerPortGroupConfigSet function
 * @endinternal
 *
 * @brief  Configure IPFIX Manager port group configuration and its IPFIX Assistant
 *         FW on a specific device
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device Number
 * @param[in] portGroupsBmp     - port groups bitmap
 * @param[in] portGroupCfgPtr    - (pointer to) structure with ipfix manager
 *                                port group configuration.
 *
 * @retval  GT_OK               - on success.
 */
GT_STATUS cpssDxChIpfixManagerPortGroupConfigSet
(
    IN  GT_U8                                          devNum,
    IN  GT_PORT_GROUPS_BMP                             portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC  *portGroupCfgPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerPortGroupConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, portGroupCfgPtr));

    rc = internal_cpssDxChIpfixManagerPortGroupConfigSet(devNum, portGroupsBmp, portGroupCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, portGroupCfgPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerConfigGet function
 * @endinternal
 *
 * @brief   Request the settings configured by cpssDxChIpfixManagerConfigSet
 *          to the FW
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum           - device Number
 *
 * @retval GT_OK                - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerConfigGet
(
    IN  GT_U8                               devNum
)
{
    GT_STATUS           rc  = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.configGet.opcode = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_GET_E;

    rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixManagerConfigGet function
 * @endinternal
 *
 * @brief   Request the settings configured by cpssDxChIpfixManagerConfigSet
 *          to the FW
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum           - device Number
 *
 * @retval GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerConfigGet
(
    IN  GT_U8                               devNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChIpfixManagerConfigGet(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internalcpssDxChIpfixManagerEntryAdd function
 * @endinternal
 *
 * @brief   Add a new IPFIX flow to a port group, specifying its First TS
 *          extracted from the first packet mirrored to the App
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum         - device number
 * @param[in]  portGroupsBmp   - port groups bitmap
 * @param[in]  entryParamsPtr - (pointer to) IPFIX entry input params
 * @param[out] flowIdPtr      - (pointer to) flow identifier. In the range
 *                              between 1 to tileMaxIpfixIndex (included).
 *                              Returns 0 when no free entry is available
 *
 * @retval  GT_OK               - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerEntryAdd
(
    IN  GT_U8                                    devNum,
    IN  GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC *entryParamsPtr,
    OUT GT_U32                                   *flowIdPtr
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;
    GT_U32                                   portGroupId;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC       *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    CPSS_NULL_PTR_CHECK_MAC(entryParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(flowIdPtr);

    if (ipfixMgrDbPtr->ipfixEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, "IPFIX Manager must be enabled first");
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp, portGroupId);

    for (portGroupId = 0; portGroupId < CPSS_DXCH_MAX_PORT_GROUPS_CNS; portGroupId++)
    {
        if (1 == ((portGroupsBmp >> portGroupId) & 1))
        {
            rc = prvCpssDxChIpfixManagerFreeFlowIdGet(devNum, portGroupId, flowIdPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (*flowIdPtr >= CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "FlowId exceeds the limit");
            }

            if(entryParamsPtr->firstTs >= CPSS_DXCH_IPFIX_MANAGER_FIRST_TS_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
            ipc_msg.entryAdd.opcode  = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_ADD_E;
            ipc_msg.entryAdd.flowId  = ((portGroupId & 0x7) << 12) | ((*flowIdPtr) & 0xFFF);
            if(ipfixMgrDbPtr->globalConfig.ipfixEntriesPerFlow ==
                    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E)
            {
                /* Set Bit 12 to 0 */
                ipc_msg.entryAdd.flowId &= (~(1<<12));
            }
            ipc_msg.entryAdd.firstTs      = entryParamsPtr->firstTs;
            ipc_msg.entryAdd.firstTsValid = entryParamsPtr->firstTsValid;

            rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
    }
    return rc;
}

/**
 * @internal cpssDxChIpfixManagerEntryAdd function
 * @endinternal
 *
 * @brief   Add a new IPFIX flow to a port group, specifying its First TS
 *          extracted from the first packet mirrored to the App
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum       - device number
 * @param[in]  portGroupsBmp - port groups bitmap
 * @param[in]  entryParamsPtr  - IPFIX entry input params
 * @param[out] flowIdPtr    - (pointer to) flow identifier. In the range
 *                            between 1 to tileMaxIpfixIndex (included).
 *                            Returns 0 when no free entry is available
 *
 * @retval  GT_OK               - on success
 */
GT_STATUS cpssDxChIpfixManagerEntryAdd
(
    IN  GT_U8                                    devNum,
    IN  GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC *entryParamsPtr,
    OUT GT_U32                                   *flowIdPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerEntryAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, entryParamsPtr, flowIdPtr));

    rc = internal_cpssDxChIpfixManagerEntryAdd(devNum, portGroupsBmp, entryParamsPtr, flowIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp,entryParamsPtr, flowIdPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerEntryDelete function
 * @endinternal
 *
 * @brief   Release the FlowId of an active entry in a Tile
 *          and terminate its IPFIX monitoring
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum        - device number
 * @param[in]  portGroupsBmp  - port groups bitmap
 * @param[in]  flowId        - flow identifier
 *
 * @retval GT_OK       - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerEntryDelete
(
    IN  GT_U8                                  devNum,
    IN  GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN  GT_U32                                 flowId
)
{
    GT_STATUS                               rc = GT_OK;
    GT_BOOL                                 isFlowActive = GT_FALSE;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;
    GT_U32                                  portGroupId;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    if (ipfixMgrDbPtr->ipfixEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, "IPFIX Manager must be enabled first");
    }

    if (flowId >= CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp, portGroupId);

    for (portGroupId = 0; portGroupId < CPSS_DXCH_MAX_PORT_GROUPS_CNS; portGroupId++)
    {
        if (1 == ((portGroupsBmp >> portGroupId) & 1))
        {
            /* Check the entry is active, otherwise complete the API with an error status */
            rc = prvCpssDxChIpfixManagerIsFlowIdActive(devNum, portGroupId, flowId, &isFlowActive);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (isFlowActive == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }

            /* Push back the flowId in the free pool of the Tile */
            rc = prvCpssDxChIpfixManagerFlowIdRelease(devNum, portGroupId, flowId);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
            ipc_msg.entryDelete.opcode = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_DELETE_E;
            ipc_msg.entryDelete.flowId = ((portGroupId & 0x7) << 12) | (flowId & 0xFFF);
            if(ipfixMgrDbPtr->globalConfig.ipfixEntriesPerFlow ==
                    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E)
            {
                /* Set Bit 12 to 0 */
                ipc_msg.entryDelete.flowId &= (~(1<<12));
            }

            rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);
        }
    }
    return rc;
}

/**
 * @internal cpssDxChIpfixManagerEntryDelete function
 * @endinternal
 *
 * @brief   Release the FlowID of an active entry in a Tile
 *          and terminate its IPFIX monitoring
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum        - device number
 * @param[in]  portGroupsBmp - port groups bitmap
 * @param[in]  flowId        - flow identifier
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerEntryDelete
(
    IN  GT_U8                                  devNum,
    IN  GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN  GT_U32                                 flowId
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerEntryDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, flowId));

    rc = internal_cpssDxChIpfixManagerEntryDelete(devNum, portGroupsBmp, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, flowId));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerEntryDeleteAll function
 * @endinternal
 *
 * @brief   Release all FlowIDs of all active entries in
 *          the device and terminate all IPFIX monitoring.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum          - device number
 * @param[out[ numDeletedPtr   - number of active entries deleted
 *
 * @retval GT_OK       - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerEntryDeleteAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numDeletedPtr
)
{
    GT_STATUS  rc               = GT_OK;
    GT_U32     portGroupId     = 0;
    GT_U32     flowId           = 0;
    GT_BOOL    isFlowActive     = GT_FALSE;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    CPSS_NULL_PTR_CHECK_MAC(numDeletedPtr);

    *numDeletedPtr = 0;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        for (flowId = 0;
              flowId < ipfixMgrDbPtr->portGroupConfig[(portGroupId/2)*2].maxIpfixIndex;
              flowId++)
        {
            rc = prvCpssDxChIpfixManagerIsFlowIdActive(devNum, portGroupId, flowId, &isFlowActive);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (isFlowActive == GT_TRUE)
            {
                /* Push back the flowId in the free pool of the Tile */
                rc = prvCpssDxChIpfixManagerFlowIdRelease(devNum, portGroupId, flowId);
                if (rc != GT_OK)
                {
                    return rc;
                }

                (*numDeletedPtr) ++;
            }
        }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    /* Issue an EntryDeleteAll IPC message to FW for being sure both entities are in sync */
    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.entryDeleteAll.opcode = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E;

    rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);
    return rc;
}

/**
 * @internal cpssDxChIpfixManagerEntryDeleteAll function
 * @endinternal
 *
 * @brief   Release all FlowIDs of all active entries in
 *          the device and terminate all IPFIX monitoring.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum          - device number
 * @param[out[ numDeletedPtr   - number of active entries deleted
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerEntryDeleteAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numDeletedPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerEntryDeleteAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numDeletedPtr));

    rc = internal_cpssDxChIpfixManagerEntryDeleteAll(devNum, numDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numDeletedPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerDataGet function
 * @endinternal
 *
 * @brief   Pull the IPFIX data for a single entry in a Tile.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum        - device number
 * @param[in]  portGroupsBmp  - port group bitmap
 * @param[in]  flowId        - flow identifier
 *
 * @retval GT_OK       - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerIpfixDataGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN  GT_U32                                 flowId
)
{
    GT_STATUS   rc              = GT_OK;
    GT_BOOL     isFlowActive    = GT_FALSE;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;
    GT_U32                                       portGroupId;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    if (ipfixMgrDbPtr->ipfixEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, "IPFIX Manager must be enabled first");
    }

    if (flowId >= CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp, portGroupId);

    for (portGroupId = 0; portGroupId < CPSS_DXCH_MAX_PORT_GROUPS_CNS; portGroupId++)
    {
        if (1 == ((portGroupsBmp >> portGroupId) & 1))
        {
            /* Check the entry is active, otherwise return error status */
            rc = prvCpssDxChIpfixManagerIsFlowIdActive(devNum, portGroupId, flowId, &isFlowActive);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (isFlowActive == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "Flow is not active");
            }

            /* Issue a DataGet IPC message to IPFIX Assistant FW with TileNum and flowId as input */
            cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
            ipc_msg.dataGet.opcode  = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_DATA_GET_E;
            ipc_msg.dataGet.flowId  = ((portGroupId & 0x7) << 12) | (flowId & 0xFFF);
            if(ipfixMgrDbPtr->globalConfig.ipfixEntriesPerFlow ==
                    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E)
            {
                /* Set Bit 12 to 0 */
                ipc_msg.dataGet.flowId &= (~(1<<12));
            }

            rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return rc;
}

/**
 * @internal cpssDxChIpfixManagerDataGet function
 * @endinternal
 *
 * @brief   Pull the IPFIX data for a single entry in a Tile.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum        - device number
 * @param[in]  portGroupsBmp  - port groups bitmap
 * @param[in]  flowId        - flow identifier
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpfixDataGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN  GT_U32                                 flowId
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerIpfixDataGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, flowId));

    rc = internal_cpssDxChIpfixManagerIpfixDataGet(devNum, portGroupsBmp, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, flowId));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerDataGetAll function
 * @endinternal
 *
 * @brief   Pull the IPFIX data for all active entries of the device.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum           - device number
 *
 * @retval GT_OK       - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerIpfixDataGetAll
(
    IN  GT_U8                                  devNum
)
{
    GT_STATUS       rc = GT_OK;
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT ipc_msg;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    if (ipfixMgrDbPtr->ipfixEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, "IPFIX Manager must be enabled first");
    }

    /* Issue a DataGetAll IPC message to IPFIX Assistant FW */
    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.dataGetAll.opcode = CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_DATA_GET_ALL_E;

    rc = prvCpssDxChIpfixManagerIpcMessageSend(devNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixManagerDataGetAll function
 * @endinternal
 *
 * @brief   Pull the IPFIX data for all active entries of the device.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum           - device number
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpfixDataGetAll
(
    IN  GT_U8                                  devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerIpfixDataGetAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChIpfixManagerIpfixDataGetAll(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerIpcMsgFetch function
 * @endinternal
 *
 * @brief  Trigger IPFIX Manager to fetch the next pending
 *         IPC message
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum              - device number
 * @param[out] numOfMsgFetchedPtr  - (pointer to) 0 or 1, depending if the Tx
 *                                   IPC memory was empty or not
 *
 * @retval GT_OK       - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerIpcMsgFetch
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numMsgToFetch = 1;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    CPSS_NULL_PTR_CHECK_MAC(numOfMsgFetchedPtr);

    rc = prvCpssDxChIpfixManagerIpcMsgFetchAndHandleAll(devNum, numMsgToFetch,
                                                        numOfMsgFetchedPtr);

    return rc;
}

/**
 * @internal cpssDxChIpfixManagerIpcMsgFetch function
 * @endinternal
 *
 * @brief  Trigger IPFIX Manager to fetch the next pending
 *         IPC message
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum              - device number
 * @param[out] numOfMsgFetchedPtr  - (pointer to) 0 or 1, depending if the Tx
 *                                   IPC memory was empty or not
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpcMsgFetch
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerIpcMsgFetch);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfMsgFetchedPtr));

    rc = internal_cpssDxChIpfixManagerIpcMsgFetch(devNum, numOfMsgFetchedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfMsgFetchedPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerIpcMsgFetchAll function
 * @endinternal
 *
 * @brief  Fetch all the pending IPC message (up to 1K max)
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum                 - device number
 * @param[out] numOfMsgFetchedPtr        - (pointer to) number of fetched messages.
 *                                         Range: 0 to 1K.
 *
 * @retval GT_OK       - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerIpcMsgFetchAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numMsgToFetch = PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_MAX_CNS;
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC          *ipfixMgrDbPtr = NULL;

    /* check dev number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_IPFIX_MANAGER_SUPPORT_CHECK_MAC(devNum);

    /* Get ipfix manager database pointer check whether initialized */
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_CHECK_MAC(ipfixMgrDbPtr);

    CPSS_NULL_PTR_CHECK_MAC(numOfMsgFetchedPtr);

    rc = prvCpssDxChIpfixManagerIpcMsgFetchAndHandleAll(devNum, numMsgToFetch,
                                                        numOfMsgFetchedPtr);

    return rc;
}

/**
 * @internal cpssDxChIpfixManagerIpcMsgFetchAll function
 * @endinternal
 *
 * @brief  Trigger IPFIX Manager to fetch all the pending IPC
 *         message (up to 1K max)
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum                 - device number
 * @param[out] numOfMsgFetchedPtr        - (pointer to) number of fetched messages.
 *                                         Range: 0 to 1K.
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpcMsgFetchAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerIpcMsgFetchAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfMsgFetchedPtr));

    rc = internal_cpssDxChIpfixManagerIpcMsgFetchAll(devNum, numOfMsgFetchedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfMsgFetchedPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixManagerEventNotifyBind function
 * @endinternal
 *
 * @brief  Register call back function for even notification
 *
 * @param[in] devNum           - device number
 * @param[in[ notifyFuncPtr    - (pointer to)event notification function callback
 *
 * @retval GT_OK     - on success
 */
static GT_STATUS internal_cpssDxChIpfixManagerEventNotifyBind
(
    IN   GT_U8                                 devNum,
    IN   CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC    notifyFunc
)
{
    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC  *ipfixMgrDbPtr = NULL;

    PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(devNum, ipfixMgrDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipfixMgrDbPtr);

    ipfixMgrDbPtr->eventNotifyFunc = notifyFunc;

    return GT_OK;
}


/**
 * @internal cpssDxChIpfixManagerEventNotifyBind function
 * @endinternal
 *
 * @brief  Register call back function for even notification
 *
 * @param[in] devNum           - device number
 * @param[in[ notifyFunc       - (pointer to)event notification function callback
 *
 * @retval GT_OK     - on success
 */
GT_STATUS cpssDxChIpfixManagerEventNotifyBind
(
    IN   GT_U8                                 devNum,
    IN   CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC    notifyFunc
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixManagerEventNotifyBind);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, notifyFunc));

    rc = internal_cpssDxChIpfixManagerEventNotifyBind(devNum, notifyFunc);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, notifyFunc));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}
