/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssDxChFlowManager.c
*
* @brief CPSS APIs for Flow Manager.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/flowManager/private/prvCpssDxChFlowManagerLog.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_FLOW_MANAGER_DB_VAR_SET(_var,_value)\
      PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.flowMgrDir.flowMgrSrc._var,_value)

#define PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.flowMgrDir.flowMgrSrc._var)

/* the longest DSA tag - eDSA - consists of  four 32-bit words */
#define MAC_ADDR_LEN_IN_BYTES_CNS                    12
#define FLOW_KEY_ENTRY_LEN_IN_BYTES_CNS              16
#define FLOW_KEY_DISABLED                            16
#define FLOW_MANAGER_FW_SRAM_OFFSET                  0x80000

/**
* @internal internal_cpssDxChFlowManagerCreate function
* @endinternal
*
* @brief   Create Flow Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId               - Flow Manager Id
*                                     (APPLICABLE RANGES: 0..127)
* @param[in] flowMngInfoPtr          - ptr to flow manager related structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_ALREADY_EXIST         - if already exist Flow Manager with given Id
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on no memory available at host CPU.
*/
GT_STATUS internal_cpssDxChFlowManagerCreate
(
    IN  GT_U32                                flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_STC            *flowMngInfoPtr
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC          *flowMngDbPtr;
    GT_U32                                  iter = 0;

    CPSS_NULL_PTR_CHECK_MAC(flowMngInfoPtr);
    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (flowMngInfoPtr->flowDbSize > PRV_CPSS_DXCH_FLOW_DB_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) != NULL)
    {
        /* can't redefine an existing Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* allocate memory for the flow manager object */
    PRV_SHARED_FLOW_MANAGER_DB_VAR_SET(flowMngDB[flowMngId],(PRV_CPSS_DXCH_FLOW_MANAGER_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_FLOW_MANAGER_STC)));

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    cpssOsMemSet(flowMngDbPtr, 0, sizeof(PRV_CPSS_DXCH_FLOW_MANAGER_STC));

    flowMngDbPtr->flowMngId = flowMngId;
    flowMngDbPtr->flowDbSize = flowMngInfoPtr->flowDbSize;

    /* alloate memory for flow state which tracks flow id exists in FW or not */
    flowMngDbPtr->flowState = (GT_BOOL*)cpssOsMalloc(sizeof(GT_BOOL)*(flowMngInfoPtr->flowDbSize));
    if(flowMngDbPtr->flowState == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* initializing all the flows state in cpss db to be inactive or not
     * present in fw db.
     */
    for(iter = 0; iter < flowMngInfoPtr->flowDbSize /* flow hash size */; iter++)
    {
        flowMngDbPtr->flowState[iter] = GT_FALSE;
    }

    /* allocate memory for flow data db */
    flowMngDbPtr->flowDataDb = (CPSS_DXCH_FLOW_MANAGER_FLOW_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_STC)*(flowMngInfoPtr->flowDbSize));
    if(flowMngDbPtr->flowDataDb == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(flowMngDbPtr->flowDataDb, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_STC)*(flowMngInfoPtr->flowDbSize));

    /* allocate memory for flow counter db */
    flowMngDbPtr->flowCounterDb = (CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC)*(flowMngInfoPtr->flowDbSize));
    if(flowMngDbPtr->flowCounterDb == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(flowMngDbPtr->flowCounterDb, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC)*(flowMngInfoPtr->flowDbSize));

    /* allocate memory for flow counter db for removed flows */
    flowMngDbPtr->removedFlowCounterDb = (PRV_CPSS_DXCH_FLOW_MANAGER_REMOVED_FLOW_ENTRY_STC*)cpssOsMalloc
                                         (sizeof(PRV_CPSS_DXCH_FLOW_MANAGER_REMOVED_FLOW_ENTRY_STC)*(flowMngInfoPtr->flowDbSize));
    if(flowMngDbPtr->removedFlowCounterDb == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(flowMngDbPtr->removedFlowCounterDb, 0, sizeof(PRV_CPSS_DXCH_FLOW_MANAGER_REMOVED_FLOW_ENTRY_STC)*(flowMngInfoPtr->flowDbSize));

    /* allocate memory for flow hash db for flows management */
    flowMngDbPtr->flowHashDb = (PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC)*(flowMngInfoPtr->flowDbSize));
    if(flowMngDbPtr->flowHashDb == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(flowMngDbPtr->flowHashDb, 0, sizeof(PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC)*(flowMngInfoPtr->flowDbSize));

    /* loop from second entry till last - 1 */
    for(iter = 1; iter < (flowMngInfoPtr->flowDbSize - 1) /* flow hash size */; iter++)
    {
        flowMngDbPtr->flowHashDb[iter].cpssFlowIndex = iter;
        flowMngDbPtr->flowHashDb[iter].next = &flowMngDbPtr->flowHashDb[iter+1];
        flowMngDbPtr->flowHashDb[iter].prev = &flowMngDbPtr->flowHashDb[iter-1];
        flowMngDbPtr->flowHashDb[iter].nextInHashList = NULL;
        flowMngDbPtr->flowHashDb[iter].prevInHashList = NULL;
    }

    /* last entry - iter already points to last entry */
    flowMngDbPtr->flowHashDb[iter].cpssFlowIndex = iter;
    flowMngDbPtr->flowHashDb[iter].next = NULL;
    flowMngDbPtr->flowHashDb[iter].prev = &flowMngDbPtr->flowHashDb[iter-1];
    flowMngDbPtr->flowHashDb[iter].nextInHashList = NULL;
    flowMngDbPtr->flowHashDb[iter].prevInHashList = NULL;

    /* first entry */
    iter = 0;
    flowMngDbPtr->flowHashDb[iter].cpssFlowIndex = iter;
    flowMngDbPtr->flowHashDb[iter].next = &flowMngDbPtr->flowHashDb[iter+1];
    flowMngDbPtr->flowHashDb[iter].prev = NULL;
    flowMngDbPtr->flowHashDb[iter].nextInHashList = NULL;
    flowMngDbPtr->flowHashDb[iter].prevInHashList = NULL;

    flowMngDbPtr->cpssFlowFreeListHeader = &flowMngDbPtr->flowHashDb[0];
    flowMngDbPtr->cpssFlowUsedListHeader = NULL;
    flowMngDbPtr->cpssFlowUsedListTail = NULL;

    /* allocate memory for flow hash db for flows management */
    for(iter = 0; iter < PRV_NUM_OF_HASH_LISTS_CNS; iter++)
    {
        flowMngDbPtr->hashList[iter] = (PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC));
        if(flowMngDbPtr->hashList[iter] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(flowMngDbPtr->hashList[iter], 0, sizeof(PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC));
    }

    for(iter = 0; iter < PRV_NUM_OF_HASH_LISTS_CNS; iter++)
    {
        flowMngDbPtr->hashList[iter]->nextInHashList = NULL;
    }

    flowMngDbPtr->fwToCpssFlowDb = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*(flowMngInfoPtr->cpssFwCfg.flowDBSize));
    if(flowMngDbPtr->fwToCpssFlowDb == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    for(iter = 0; iter < flowMngInfoPtr->cpssFwCfg.flowDBSize; iter++)
    {
        /* Init Flow index mapping from FW DB to CPSS DB with an invalid flow
         * index.
         */
        flowMngDbPtr->fwToCpssFlowDb[iter] = PRV_CPSS_FLOW_MANAGER_INVALID_FLOW_INDEX_CNS;
    }

    /* allocate 10K buffers, max supported by device to store complete received packet received from FW */
    flowMngDbPtr->reassemblyBufferPtr = (GT_U8*)cpssOsMalloc(sizeof(GT_U8)*(_10K));
    if(flowMngDbPtr->reassemblyBufferPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(flowMngDbPtr->reassemblyBufferPtr, 0, sizeof(GT_U8)*(_10K));

    /* assign the FW configuration params sent from application to our local structure */
    flowMngDbPtr->cpssFwCfg.rxSdmaQ = flowMngInfoPtr->cpssFwCfg.rxSdmaQ;
    flowMngDbPtr->cpssFwCfg.txSdmaQ = flowMngInfoPtr->cpssFwCfg.txSdmaQ;
    flowMngDbPtr->cpssFwCfg.messagePacketCpuCode = flowMngInfoPtr->cpssFwCfg.messagePacketCpuCode;
    flowMngDbPtr->cpssFwCfg.cpuCodeStart = flowMngInfoPtr->cpssFwCfg.cpuCodeStart;
    flowMngDbPtr->cpssFwCfg.tcamStartIndex = flowMngInfoPtr->cpssFwCfg.tcamStartIndex;
    flowMngDbPtr->cpssFwCfg.ipfixStartIndex = flowMngInfoPtr->cpssFwCfg.ipfixStartIndex;
    flowMngDbPtr->cpssFwCfg.tcamNumOfShortRules = flowMngInfoPtr->cpssFwCfg.tcamNumOfShortRules;
    flowMngDbPtr->cpssFwCfg.tcamShortRuleSize = flowMngInfoPtr->cpssFwCfg.tcamShortRuleSize;
    flowMngDbPtr->cpssFwCfg.flowDBSize = flowMngInfoPtr->cpssFwCfg.flowDBSize;
    flowMngDbPtr->cpssFwCfg.flowShortDBSize = flowMngInfoPtr->cpssFwCfg.flowShortDBSize;
    flowMngDbPtr->cpssFwCfg.reducedRuleSize = flowMngInfoPtr->cpssFwCfg.reducedRuleSize;

    flowMngDbPtr->tcamShortRuleSize = flowMngInfoPtr->cpssFwCfg.tcamShortRuleSize;
    flowMngDbPtr->flowDbSize = flowMngInfoPtr->flowDbSize;
    flowMngDbPtr->appCbFlowNotificaion = flowMngInfoPtr->appCbFlowNotificaion;
    flowMngDbPtr->appCbOldFlowRemovalNotificaion = flowMngInfoPtr->appCbOldFlowRemovalNotificaion;

    /* FW by design always has service enabled after device add which loads FW and pushes init configurations
     * and so making the flag to be true to be inline with the design.
     */
    flowMngDbPtr->flowMngEnable = GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChFlowManagerCreate function
* @endinternal
*
* @brief   Create Flow Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                    - Flow Manager Id
*                                          (APPLICABLE RANGES: 0..127)
* @param[in] flowMngInfoPtr               - ptr to flow manager configuration structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_ALREADY_EXIST         - if already exist Flow Manager with given Id
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation fail
*/
GT_STATUS cpssDxChFlowManagerCreate
(
    IN  GT_U32                                      flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_STC                  *flowMngInfoPtr
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, flowMngInfoPtr));

    rc = internal_cpssDxChFlowManagerCreate(flowMngId, flowMngInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, flowMngInfoPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv function
* @endinternal
*
* @brief   Send and Receive IPC message from service cpu.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum              - device number id.
* @param[in] flowMngDbPtr        - (pointer to)Flow Manager structure
* @param[in] ipc_msg_send        - (pointer to)ipc message send structure
* @param[out]ipc_msg_recv        - (pointer to)ipc message receive structure
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
* @retval GT_NO_MORE               - no messages ready
*/
#ifndef ASIC_SIMULATION
static GT_STATUS prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv
(
    IN  GT_U8                               devNum,
    IN  PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr,
    IN  PRV_CPSS_DXCH_IPC_MSG_STC           *ipc_msg_send,
    OUT PRV_CPSS_DXCH_IPC_RECV_MSG_STC      *ipc_msg_recv
)
{
    GT_STATUS                            rc = GT_OK;
    GT_UINTPTR                           fwChannel;
    GT_U32                               replyTimeout = 500;

    fwChannel = flowMngDbPtr->devInfo[devNum]->fwChannel;
    /* Send IPC message to FW passing the configuration parameters */
    rc = prvCpssGenericSrvCpuIpcMessageSend(fwChannel, 1, (GT_U8*)ipc_msg_send, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Wait for message from Service CPU */
    do
    {
        rc = prvCpssGenericSrvCpuIpcMessageRecv(fwChannel, 1, (GT_U8*)ipc_msg_recv, NULL);
        if (rc != GT_NO_MORE)
        {
            break;
        }
        else
        {
            cpssOsTimerWkAfter(1);
            replyTimeout--;
        }
    } while (replyTimeout > 0);
    if (rc != GT_OK)
    {
        return (rc == GT_NO_MORE) ? GT_TIMEOUT : rc;
    }

    return rc;
}
#endif

/**
* @internal deviceIteratorGetNext function
* @endinternal
*
* @brief   The function returns the first/next devNum that is bound to the flow manager.
*
* @param[in] flowMngDbPtr        - (pointer to)Flow Manager structure
* @param[in] isFirst             - is First or next
* @param[in,out] devNumPtr       - (pointer to) hold 'current' devNum . (relevant for 'next')
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_NO_MORE               - no devices attached to the vTcamMngId
*/
static GT_STATUS deviceIteratorGetNext
(
    IN    PRV_CPSS_DXCH_FLOW_MANAGER_STC  *flowMngDbPtr,
    IN    GT_BOOL                         isFirst,
    INOUT GT_U8                           *devNumPtr
)
{
    GT_U32  ii;/* must not be GT_U8 */

    if(isFirst == GT_TRUE)
    {
        ii = 0;
        *devNumPtr = 0;
    }
    else
    {
        ii = *devNumPtr;

        ii++;
    }

    for(/* continue */; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        if(PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, ii))
        {
            *devNumPtr = (GT_U8)ii;
            return GT_OK;
        }
    }

    return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
}

/**
* @internal cpssDxChFlowMgrDbDeviceIteratorGetFirst function
* @endinternal
*
* @brief   The function returns the first devNum that is bound to the ipfix flow manager.
*
* @param[in] flowMngDbPtr             - (pointer to)Flow Manager structure
* @param[out]devNumPtr                - (pointer to) hold 'first' devNum .
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_NO_MORE               - no devices attached to the flowMngId
*/
static GT_STATUS cpssDxChFlowMgrDbDeviceIteratorGetFirst
(
    IN  PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr,
    OUT GT_U8                               *devNumPtr
)
{
    return deviceIteratorGetNext(flowMngDbPtr, GT_TRUE, devNumPtr);
}

/**
* @internal cpssDxChFlowManagerConfigurationValidityCheck function
* @endinternal
*
* @brief   This function does validity check for FW configuration parameters.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number id.
* @param[in] flowMngDbPtr          - (pointer to) flow manager db structure.
* @param[out]hwCpuCodeForFw        - hw dsa code returned for the sw cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the device is not found in DB
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS cpssDxChFlowManagerConfigurationValidityCheck
(
    IN  GT_U8                  devNum,
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    OUT GT_U32                 *hwCpuCodeForFw
)
{
    GT_U32                               tcamRangeIndex;
    GT_U32                               ipfixRangeIndex;
    GT_U32                               ruleSize;
    GT_STATUS                            rc;

    /* Validity check for FW configuration parameters */
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum, flowMngDbPtr->cpssFwCfg.rxSdmaQ);
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum, flowMngDbPtr->cpssFwCfg.txSdmaQ);
    rc = prvCpssDxChNetIfCpuToDsaCode(flowMngDbPtr->cpssFwCfg.cpuCodeStart, hwCpuCodeForFw);
    if (rc != GT_OK)
    {
        return rc;
    }
    switch (flowMngDbPtr->cpssFwCfg.tcamShortRuleSize)
    {
        /* Acceptable ranges of pcl key size  and reduced rule allowed
         * ranges as per FW rule key design. First 2 bytes are reserved for
         * PCL-ID and hence the size is 2 less than the maximum key size*/
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E:
        if(flowMngDbPtr->cpssFwCfg.reducedRuleSize > 18)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        ruleSize = 2;
        break;
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E:
        if(flowMngDbPtr->cpssFwCfg.reducedRuleSize > 28)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        ruleSize = 3;
        break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    tcamRangeIndex = flowMngDbPtr->cpssFwCfg.tcamStartIndex + (flowMngDbPtr->cpssFwCfg.tcamNumOfShortRules *
                                                               ruleSize);
    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum, tcamRangeIndex);
    ipfixRangeIndex = flowMngDbPtr->cpssFwCfg.ipfixStartIndex + (flowMngDbPtr->cpssFwCfg.tcamNumOfShortRules);
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, ipfixRangeIndex);
    if(flowMngDbPtr->cpssFwCfg.flowDBSize > MAX_NUM_OF_FW_FLOW_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(flowMngDbPtr->cpssFwCfg.flowShortDBSize > flowMngDbPtr->cpssFwCfg.flowDBSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChFlowManagerDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing Flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] devListArr[]             - the array of device ids to add to the Flow Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*/
GT_STATUS internal_cpssDxChFlowManagerDevListAdd
(
    IN GT_U32   flowMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_U8                                i;
    GT_U8                                devNum;
    GT_STATUS                            rc = GT_OK;
    CPSS_DXCH_FLOW_MNG_PTR               flowMngDbPtr;
    GT_U8                                representativeDevNum;
    GT_BOOL                              noDevicesBeforAdding;
    PRV_CPSS_DXCH_IPC_MSG_STC            ipc_msg_send;
    PRV_CPSS_DXCH_IPC_RECV_MSG_STC       ipc_msg_recv;
    GT_HW_DEV_NUM                        hwDevNum;      /* HW device number */
    GT_U32                               index;
    CPSS_DXCH_NET_DSA_PARAMS_STC         dsaInfo;
    PRV_CPSS_DXCH_PP_CONFIG_STC          *pDev;
    GT_U32                               fwMgcamChannel = 1;/* FW uses MGCAM index 1 and CPSS uses index 0 */
    GT_U32                               fwWorkingMode = 1;
    GT_U32                               fwTcamSizeForHwMode = 0;
    GT_U32                               fwBackpressureThreshold = 0x800; /* BTL limit */
    GT_U32                               tcamNumOfShortRules;
#ifndef ASIC_SIMULATION
    GT_U32                               sCpuId = 0;
    GT_UINTPTR                           fwChannel;
    GT_U32                               configEnum;
    GT_U32                               lastReadIsfMsgSeqPtr;
#endif
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  hwCpuCodeForFw;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(numOfDevs >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(numOfDevs)
    {
        CPSS_NULL_PTR_CHECK_MAC(devListArr);
    }

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));

    /* check if the DB already hold existing device */
    rc = cpssDxChFlowMgrDbDeviceIteratorGetFirst(flowMngDbPtr, &representativeDevNum);
    if(rc == GT_OK)
    {
        /* there is at least one device in the Flow manager prior to calling this function */
        /* representativeDevNum is valid device */
        noDevicesBeforAdding = GT_FALSE;
    }
    else
    {
        /* there are no devices in the Flow Manager prior to calling this function */
        /* representativeDevNum is NOT valid device */
        noDevicesBeforAdding = GT_TRUE;
    }

    for (i = 0; i < numOfDevs; i++)
    {
        devNum = devListArr[i];

        /* Current design permits only one device per flow manager */
        if(noDevicesBeforAdding == GT_FALSE)
        {
            /* Current design permits only one device per flow manager */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        /*Lock the access to per device data base in order to avoid corruption*/
        /* check that device exist */
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        rc = cpssDxChFlowManagerConfigurationValidityCheck(devNum, flowMngDbPtr, &hwCpuCodeForFw);
        if (rc != GT_OK)
        {
            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            return rc;
        }

        pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

        /* Support only SIP5 devices for now Aldrin2 */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
        {
            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }

        /* Check device family */
        if (noDevicesBeforAdding == GT_TRUE)
        {
            flowMngDbPtr->devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;
        }

        /* check that the device wasn't added already */
        if (PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, devNum))
        {
             /*Unlock the access to per device data base*/
             CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
             /* the device exists */
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        if(flowMngDbPtr->numOfDevices >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: can't increment numOfDevices as already with maximal value[%d]",
                flowMngDbPtr->numOfDevices);
        }


        /* update devices counter */
        flowMngDbPtr->numOfDevices++;

        /* Add device to bitmap */
        PRV_CPSS_FLOW_MNG_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, devNum);

        /* alloate memory for device info pointer which tracks DB of source port per device type*/
        flowMngDbPtr->devInfo[devNum] = (PRV_CPSS_DXCH_FLOW_MANAGER_SOURCE_PORT_TO_DEV_NUM_MAPPING_STC *)
                                cpssOsMalloc(sizeof(PRV_CPSS_DXCH_FLOW_MANAGER_SOURCE_PORT_TO_DEV_NUM_MAPPING_STC));
        if(flowMngDbPtr->devInfo[devNum] == NULL)
        {
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        /* Assign Flow Manager Id for every device instance */
        pDev->flowMgrInfo.flowManagerId = flowMngId;

        /* unlock after finish using of pDev */
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* Prepare to configure the FW parameters. All the parameters follow the format
         * of <1 byte configuration parameters type, 4 byte of parameters value> pair
         * except DSA tag which is 4 words = 16bytes, ended with 1 byte no more type.
         * Service CPU is LE so the message needs to be converted from BE to LE before
         * sending it to the FW.
         */
        ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_CONFIG_E;

        index = 0;

        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_RX_SDMA_QUEUE_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.rxSdmaQ = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.rxSdmaQ);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.rxSdmaQ, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TX_SDMA_QUEUE_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.txSdmaQ = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.txSdmaQ);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.txSdmaQ, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_START_INDEX_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.tcamStartIndex = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.tcamStartIndex);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.tcamStartIndex, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_IPFIX_START_INDEX_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.ipfixStartIndex = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.ipfixStartIndex);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.ipfixStartIndex, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_SIZE_E;
        index += sizeof(GT_U8);
        if(flowMngDbPtr->cpssFwCfg.tcamShortRuleSize == CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E)
        {
            tcamNumOfShortRules = flowMngDbPtr->cpssFwCfg.tcamNumOfShortRules * 2;
        }
        else if(flowMngDbPtr->cpssFwCfg.tcamShortRuleSize == CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E)
        {
            tcamNumOfShortRules = flowMngDbPtr->cpssFwCfg.tcamNumOfShortRules * 3;
        }
        tcamNumOfShortRules = CPSS_32BIT_LE(tcamNumOfShortRules);
        cpssOsMemCpy(&ipc_msg_send.data[index], &tcamNumOfShortRules, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_SHORT_RULE_SIZE_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.tcamShortRuleSize = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.tcamShortRuleSize);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.tcamShortRuleSize, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_REDUCED_RULE_SIZE_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.reducedRuleSize = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.reducedRuleSize);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.reducedRuleSize, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_FW_FLOW_DB_SIZE_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.flowDBSize = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.flowDBSize);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.flowDBSize, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_FW_SHORT_DB_SIZE_E;
        index += sizeof(GT_U8);
        flowMngDbPtr->cpssFwCfg.flowShortDBSize  = CPSS_32BIT_LE(flowMngDbPtr->cpssFwCfg.flowShortDBSize);
        cpssOsMemCpy(&ipc_msg_send.data[index], &flowMngDbPtr->cpssFwCfg.flowShortDBSize, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_RX_CPU_CODE_START_E;
        index += sizeof(GT_U8);
        hwCpuCodeForFw = CPSS_32BIT_LE(hwCpuCodeForFw);
        cpssOsMemCpy(&ipc_msg_send.data[index], &hwCpuCodeForFw, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TX_DSA_TAG_E;
        index += sizeof(GT_U8);
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
        dsaInfo.dsaInfo.toCpu.cpuCode = flowMngDbPtr->cpssFwCfg.messagePacketCpuCode;

        /* build DSA Tag buffer */
        rc = cpssDxChNetIfDsaTagBuild(devNum,
            &dsaInfo,
            &ipc_msg_send.data[index]);
        if (rc != GT_OK)
        {
            return rc;
        }

        index += 16;

        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_MGCAM_CHANNEL_E;
        index += sizeof(GT_U8);
        fwMgcamChannel = CPSS_32BIT_LE(fwMgcamChannel);
        cpssOsMemCpy(&ipc_msg_send.data[index], &fwMgcamChannel, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_MODE_E;
        index += sizeof(GT_U8);
        fwWorkingMode = CPSS_32BIT_LE(fwWorkingMode);
        cpssOsMemCpy(&ipc_msg_send.data[index], &fwWorkingMode, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_SIZE_FOR_HW_MODE_E;
        index += sizeof(GT_U8);
        fwTcamSizeForHwMode = CPSS_32BIT_LE(fwTcamSizeForHwMode);
        cpssOsMemCpy(&ipc_msg_send.data[index], &fwTcamSizeForHwMode, sizeof(GT_U32));

        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_BACKPRESSURE_THRESHOLD_E;
        index += sizeof(GT_U8);
        fwBackpressureThreshold = CPSS_32BIT_LE(fwBackpressureThreshold);
        cpssOsMemCpy(&ipc_msg_send.data[index], &fwBackpressureThreshold, sizeof(GT_U32));



        /* end of FW configuration parameters */
        index += sizeof(GT_U32);
        ipc_msg_send.data[index] = PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_NO_MORE_E;

#ifndef ASIC_SIMULATION
            /* Flow Manager FW is loaded to Service CPU */
            rc = serviceCpuFlowManagerFwInit(devNum, sCpuId, &fwChannel);
            if (rc != GT_OK)
            {
                return rc;
            }
            flowMngDbPtr->devInfo[devNum]->fwChannel = fwChannel;
            rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(devNum, flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(ipc_msg_recv.data[1] != GT_OK)
            {
#define CONFIG_STR(_configEnum)  _configEnum ? #_configEnum :

                configEnum = ipc_msg_recv.data[2];

                cpssOsPrintf("Configuration failed due to wrong config parameter %s\n",
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_RX_SDMA_QUEUE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TX_SDMA_QUEUE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_START_INDEX_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_IPFIX_START_INDEX_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_SIZE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_SHORT_RULE_SIZE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_REDUCED_RULE_SIZE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_FW_FLOW_DB_SIZE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_FW_SHORT_DB_SIZE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_RX_CPU_CODE_START_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TX_DSA_TAG_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_MODE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_SIZE_FOR_HW_MODE_E)
                configEnum == CONFIG_STR(PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_BACKPRESSURE_THRESHOLD_E)
                "BAD PARAMETER");
                return rc;
            }
            else
            {
                cpssOsMemCpy(&lastReadIsfMsgSeqPtr, &(ipc_msg_recv.data[2]), sizeof(GT_U32));
                lastReadIsfMsgSeqPtr += FLOW_MANAGER_FW_SRAM_OFFSET;
                flowMngDbPtr->lastReadIsfMsgSeqPtr = lastReadIsfMsgSeqPtr;
            }
#endif
    }
    return rc;
}

/**
* @internal cpssDxChFlowManagerDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing Flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] devListArr[]             - the array of device ids to add to the Flow Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*/
GT_STATUS cpssDxChFlowManagerDevListAdd
(
    IN GT_U32   flowMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerDevListAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, devListArr, numOfDevs));

    rc = internal_cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, numOfDevs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, devListArr, numOfDevs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerDevListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing Flow manager.
*         Removing last device caused cleaning up all low Manager configurations.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] devListArr[]             - the array of device ids to remove from the
*                                       Flow Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
* @retval GT_BAD_STATE             - on no devices in DB to remove
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS internal_cpssDxChFlowManagerDevListRemove
(
    IN GT_U32   flowMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_U8                        i;
    GT_U8                        devNum;
    CPSS_DXCH_FLOW_MNG_PTR       flowMngDbPtr;
    GT_U32                       removedDevices = 0; /* Number of removed devices */
    PRV_CPSS_DXCH_PP_CONFIG_STC  *pDev;

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(numOfDevs >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(numOfDevs)
    {
        CPSS_NULL_PTR_CHECK_MAC(devListArr);
    }

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the FlowMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    for (i = 0; i < numOfDevs; i++)
    {
        devNum = devListArr[i];

        /*Lock the access to per device data base in order to avoid corruption*/
        /* check that device exist */
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* Support only SIP5_25 devices */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
        {
            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }

        pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
        pDev->flowMgrInfo.flowManagerId = 0; /* clearing the allocated flow mgr id */

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* check that the device was added to bitmap */
        if (PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, devNum))
        {
            /* Remove device from bitmap */
            PRV_CPSS_FLOW_MNG_DEV_BMP_CLEAR_MAC(flowMngDbPtr->devsBitmap, devNum);

            /* free memory for device info which is DB for source port per device type */
            if(flowMngDbPtr->devInfo[devNum] !=  NULL)
            {
              cpssOsFree(flowMngDbPtr->devInfo[devNum]);
            }
            flowMngDbPtr->devInfo[devNum] = NULL;
            removedDevices++;

            if(flowMngDbPtr->numOfDevices == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: can't decrement numOfDevices with value 0");
            }

            /* update devices counter */
            flowMngDbPtr->numOfDevices--;

        }
    }

    /* There are no devices in DB to remove */
    if (removedDevices == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChFlowManagerDevListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing Flow manager.
*         Removing last device caused cleaning up all Flow Manager configurations.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] devListArr[]             - the array of device ids to remove from the
*                                       Flow Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
* @retval GT_BAD_STATE             - on no devices in DB to remove
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChFlowManagerDevListRemove
(
    IN GT_U32   flowMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerDevListRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, devListArr, numOfDevs));

    rc = internal_cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, devListArr, numOfDevs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerDelete function
* @endinternal
*
* @brief   Delete Flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - Flow Manager ID
*                                   (APPLICABLE RANGES: 0..127)
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on devices still attached to Flow manager
* @retval GT_NOT_FOUND             - on Flow manager not found
*/
static GT_STATUS internal_cpssDxChFlowManagerDelete
(
    IN GT_U32   flowMngId
)
{
    GT_STATUS                       rc; /* return code */
    PRV_CPSS_DXCH_FLOW_MANAGER_STC  *flowMngDbPtr;
    GT_U8                           devNum;
    GT_U32                          iter;
    GT_U32                          sCpuId = 0;

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    if (flowMngDbPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChFlowMgrDbDeviceIteratorGetFirst(
        flowMngDbPtr, &devNum);
    if (rc == GT_OK)
    {
        /* has yet not removed devices */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* free memory for flow state which tracks flow id exists in FW or not */
    if(flowMngDbPtr->flowState !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->flowState);
    }

    /* allocate memory for flow data db */
    if(flowMngDbPtr->flowDataDb !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->flowDataDb);
    }

    /* free memory for flow counter db */
    if(flowMngDbPtr->flowCounterDb !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->flowCounterDb);
    }

    /* free memory for removed flow counter db */
    if(flowMngDbPtr->removedFlowCounterDb !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->removedFlowCounterDb);
    }
    flowMngDbPtr->removedFlowCounterDb = NULL;

    /* free memory for flow hash db for flows management */
    if(flowMngDbPtr->flowHashDb !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->flowHashDb);
    }

    for(iter = 0; iter < PRV_NUM_OF_HASH_LISTS_CNS; iter++)
    {
        if(flowMngDbPtr->hashList[iter] != NULL)
        {
            cpssOsFree(flowMngDbPtr->hashList[iter]);
        }
    }

    /* free memory for fw to cpss flow index storage db */
    if(flowMngDbPtr->fwToCpssFlowDb !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->fwToCpssFlowDb);
    }

    /* free 10K buffers, max supported by device to store complete received packet received from FW */
    if(flowMngDbPtr->reassemblyBufferPtr !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->reassemblyBufferPtr);
    }

    /* free memory for device info which is DB for source port per device type */
    if(flowMngDbPtr->devInfo[devNum] !=  NULL)
    {
        cpssOsFree(flowMngDbPtr->devInfo[devNum]);
    }

    cpssOsFree(PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]));

    PRV_SHARED_FLOW_MANAGER_DB_VAR_SET(flowMngDB[flowMngId], NULL);
    prvCpssGenericSrvCpuRemove(devNum, sCpuId);

    return GT_OK;
}

/**
* @internal cpssDxChFlowManagerDelete function
* @endinternal
*
* @brief   Delete Flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - Flow Manager Id
*                                   (APPLICABLE RANGES: 0..127)
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on devices still attached to Flow manager
* @retval GT_NOT_FOUND             - on Flow manager not found
*/
GT_STATUS cpssDxChFlowManagerDelete
(
    IN  GT_U32                              flowMngId
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId));

    rc = internal_cpssDxChFlowManagerDelete(flowMngId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChFlowManagerEnableSet function
* @endinternal
*
* @brief   Enabling Flow Manager feature
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] enable                   - GT_TRUE  - enable Flow Manager feature
*                                       GT_FALSE - disable Flow Manager feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_VALUE             - on illegal value returned by FW
*/
GT_STATUS internal_cpssDxChFlowManagerEnableSet
(
    IN  GT_U32   flowMngId,
    IN  GT_BOOL  enable
)
{
    GT_U32                          ii;
    PRV_CPSS_DXCH_IPC_MSG_STC       ipc_msg_send;
    PRV_CPSS_DXCH_IPC_RECV_MSG_STC  ipc_msg_recv;
    CPSS_DXCH_FLOW_MNG_PTR          flowMngDbPtr;
    GT_STATUS                       rc = GT_OK;
#ifndef ASIC_SIMULATION
    GT_U32                          index;
#endif

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));
    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    flowMngDbPtr->flowMngEnable = enable;

    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
#ifndef ASIC_SIMULATION
        if(PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, ii))
        {
            /* Send IPC message to FW passing the init params */
            index = 0;
            if(enable == GT_TRUE)
            {
                ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_START_E;
            }
            else
            {
                ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_STOP_E;
            }
            ipc_msg_send.data[index] = CPSS_32BIT_LE(BOOL2BIT_MAC(enable));
            rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(ii, flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            if((ipc_msg_recv.data[1]) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }
        }
#else
        /* To pass compilation for WM ipc_msg_send needs to be used */
        if(ipc_msg_send.opcode == PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_START_E)
        {
        }
#endif
    }

    return rc;
}

/**
* @internal cpssDxChFlowManagerEnableSet function
* @endinternal
*
* @brief   Enabling Flow Manager feature
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] enable                   - GT_TRUE  - enable Flow Manager feature
*                                       GT_FALSE - disable Flow Manager feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_VALUE             - on illegal value returned by FW
*/
GT_STATUS cpssDxChFlowManagerEnableSet
(
    IN  GT_U32   flowMngId,
    IN  GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, enable));

    rc = internal_cpssDxChFlowManagerEnableSet(flowMngId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerEnableGet function
* @endinternal
*
* @brief   Indicates if Flow Manager feature is enabled
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - Flow Manager Id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] devNum                   - physical device number.
* @param[out] enablePtr                - (pointer to) GT_TRUE  - Flow Manager feature enabled
*                                        GT_FALSE - no Flow Manager feature support (or disabled)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on Flow manager not found
*/
GT_STATUS internal_cpssDxChFlowManagerEnableGet
(
    IN  GT_U32   flowMngId,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS                  rc = GT_OK;
    CPSS_DXCH_FLOW_MNG_PTR     flowMngDbPtr;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        CPSS_LOG_INFORMATION_MAC("flowMngId %d value not in allowed range\n",flowMngId);
    }

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    *enablePtr = flowMngDbPtr->flowMngEnable;

    return rc;
}

/**
* @internal cpssDxChFlowManagerEnableGet function
* @endinternal
*
* @brief   Indicates if DBA feature is enabled
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                 - Flow Manager Id
*                                       (APPLICABLE RANGES: 0..127)
* @param[out] enablePtr                - (pointer to) GT_TRUE  - Flow Manager feature enabled
*                                        GT_FALSE - no Flow Manager feature support (or disabled)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on Flow manager not found
*/
GT_STATUS cpssDxChFlowManagerEnableGet
(
    IN  GT_U32   flowMngId,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerEnableGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, enablePtr));

    rc = internal_cpssDxChFlowManagerEnableGet(flowMngId, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, enablePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerReSync function
* @endinternal
*
* @brief   Re-Sync CPSS DB with FW DB.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - flow manager id.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_VALUE             - on illegal value returned by FW
*/
GT_STATUS internal_cpssDxChFlowManagerReSync
(
    IN GT_U32 flowMngId
)
{
    PRV_CPSS_DXCH_IPC_MSG_STC                     ipc_msg_send;
    PRV_CPSS_DXCH_IPC_RECV_MSG_STC                ipc_msg_recv;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC                *flowMngDbPtr;
    GT_U32                                        rc = GT_OK;
    GT_U32                                        ii;

    cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));

    if (flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        CPSS_LOG_INFORMATION_MAC("flowMngId %d value not in allowed range\n",flowMngId);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    for(ii = 0; ii < (flowMngDbPtr->flowDbSize); ii++)
    {
        flowMngDbPtr->flowCounterDb[ii].flowId = ii;
        flowMngDbPtr->flowCounterDb[ii].byteCount = 0;
        flowMngDbPtr->flowCounterDb[ii].dropCount = 0;
        flowMngDbPtr->flowCounterDb[ii].packetCount = 0;
        flowMngDbPtr->flowCounterDb[ii].firstTimeStamp = 0;
        flowMngDbPtr->flowCounterDb[ii].lastTimeStamp = 0;

        flowMngDbPtr->removedFlowCounterDb[ii].byteCount = 0;
        flowMngDbPtr->removedFlowCounterDb[ii].dropCount = 0;
        flowMngDbPtr->removedFlowCounterDb[ii].packetCount = 0;

        flowMngDbPtr->flowHashDb[ii].cpssFlowIndex = ii;

        if (ii < (flowMngDbPtr->flowDbSize - 1))
        {
            flowMngDbPtr->flowHashDb[ii].next = &flowMngDbPtr->flowHashDb[ii+1];
        }

        if (ii)
        {
            flowMngDbPtr->flowHashDb[ii].prev = &flowMngDbPtr->flowHashDb[ii-1];
        }
        flowMngDbPtr->flowHashDb[ii].nextInHashList = NULL;
        flowMngDbPtr->flowHashDb[ii].prevInHashList = NULL;
    }
    flowMngDbPtr->flowHashDb[0].prev = NULL;
    flowMngDbPtr->flowHashDb[ii-1].next = NULL;
    flowMngDbPtr->cpssFlowFreeListHeader = &flowMngDbPtr->flowHashDb[0];
    flowMngDbPtr->cpssFlowUsedListHeader = NULL;
    flowMngDbPtr->cpssFlowUsedListTail = NULL;

    for(ii = 0; ii < PRV_NUM_OF_HASH_LISTS_CNS; ii++)
    {
        flowMngDbPtr->hashList[ii]->nextInHashList = NULL;
    }
    for(ii = 0; ii < flowMngDbPtr->cpssFwCfg.flowDBSize; ii++)
    {
        flowMngDbPtr->fwToCpssFlowDb[ii] = PRV_CPSS_FLOW_MANAGER_INVALID_FLOW_INDEX_CNS;
    }
    for(ii = 0; ii < flowMngDbPtr->flowDbSize; ii++)
    {
        flowMngDbPtr->flowState[ii] = GT_FALSE;
    }

    flowMngDbPtr->messageNewFlowCountAfterResync = 0;
    flowMngDbPtr->messageRemovedFlowCountAfterResync = 0;
    /* Directly calling the ReSync api misses the below debug counters
     * handled as part of message parse handling so moving it here.
     */
    flowMngDbPtr->seqNoCpss = 0;
    flowMngDbPtr->reSyncCount++;

    ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_MESSAGE_PACKET_SYNC_E;
    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
#ifndef ASIC_SIMULATION
        if(PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, ii))
        {
            /* Send IPC message to FW informing to re-sync the db */
            rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(ii, flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            if((ipc_msg_recv.data[1]) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }
        }
#else
        /* To pass compilation for WM ipc_msg_send needs to be used */
        if(ipc_msg_send.opcode == PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_MESSAGE_PACKET_SYNC_E)
        {
        }
#endif
    }
    return rc;
}

/**
* @internal cpssDxChFlowManagerReSync function
* @endinternal
*
* @brief   Re-Sync CPSS DB with FW DB.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - flow manager id.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_VALUE             - on illegal value returned by FW
*/
GT_STATUS cpssDxChFlowManagerReSync
(
    IN    GT_U32                                    flowMngId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerReSync);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId));

    rc = internal_cpssDxChFlowManagerReSync(flowMngId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerMessageParseAndCpssDbUpdate function
* @endinternal
*
* @brief   Parse the received frames and update the CPSS DB accordingly.
*          Also notify the application about the newly learnt flows
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] numOfBuff                - Num of buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_PTR               - on NULL input ptr.
*/
GT_STATUS internal_cpssDxChFlowManagerMessageParseAndCpssDbUpdate
(
    IN    GT_U8                                      devNum,
    IN    GT_U32                                     numOfBuff,
    IN    GT_U8                                      *packetBuffsArrPtr[],
    IN    GT_U32                                     buffLenArr[]
)
{
    GT_U32                                    rc = GT_OK;
    GT_U8                                     *buffPtr;   /* temporary buffer ptr */
    GT_U32                                    messageControl;
    GT_U8                                     *isfPktPtr;
    GT_U32                                    sequenceNumber;
    GT_U32                                    sequenceNumberTemp;
    GT_U32                                    sequenceNumberMessagePacket;
    GT_U8                                     opcode;
    GT_U32                                    flowId;
    GT_U32                                    cpssFlowId;
    GT_U32                                    flowMngId;
    CPSS_DXCH_FLOW_MNG_PTR                    flowMngDbPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC               *pDev;
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC           newFlow;
    GT_U32                                    keyNumberAndSize;
    GT_U32                                    tempBuff[6] = {0};
    GT_U32                                    buff;
    GT_U32                                    totalLen;
    GT_BOOL                                   seqNoCheck = GT_FALSE;
    GT_U32                                    iterCheck;
    GT_U8                                     keySize;
    GT_U8                                     keyId;
    GT_U8                                     *inKeyDataPtr;
    GT_U32                                    iter;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(packetBuffsArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(buffLenArr);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    flowMngId = pDev->flowMgrInfo.flowManagerId;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    if(numOfBuff == 1)
    {
        CPSS_NULL_PTR_CHECK_MAC(packetBuffsArrPtr[0]);
        buffPtr = packetBuffsArrPtr[0];
    }
    else
    {
        totalLen = 0;
        for(buff = 0; buff < numOfBuff; buff++)
        {
            /* retrieve total length of received packet */
            totalLen += buffLenArr[buff];
        }

        if(totalLen > _10K)
        {
            /* packet is too big, reassemblyBufferPtr has 10K only */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* copy entire content of received packet from packet buffer list to temporary buffer */
        buffPtr = flowMngDbPtr->reassemblyBufferPtr;
        for(buff = 0; buff < numOfBuff; buff++)
        {
            CPSS_NULL_PTR_CHECK_MAC(packetBuffsArrPtr[buff]);
            cpssOsMemCpy(buffPtr, packetBuffsArrPtr[buff], buffLenArr[buff]);
            buffPtr += buffLenArr[buff];
        }

        /* point buffer to the begging of reassembly one */
        buffPtr = flowMngDbPtr->reassemblyBufferPtr;
    }

    /* For debug purpose in order to dump the received packet */
    /* Message is in LE format which is native format for FW and so will have to
     * converted to BE format for CPSS to read and process further.
     */
    if(flowMngDbPtr->debug_dumpIsfPacket)
    {
        GT_U32 iter;
        GT_U32 offset = 0;
        GT_U32 jj;
        GT_U8* bufferPtr = &packetBuffsArrPtr[0][0];
        GT_U32 length = buffLenArr[0];
        GT_U32 totalLen = 0;

        /* calc totalLen */
        for(jj = 0; jj < numOfBuff; jj++)
        {
            totalLen += buffLenArr[jj];
        }

        for(jj = 0; jj < numOfBuff; jj++)
        {
            bufferPtr = &packetBuffsArrPtr[jj][0];
            length = buffLenArr[jj];

            for(iter = 0; iter < length; iter++)
            {
                if((offset & 0x0F) == 0)
                {
                    cpssOsPrintf("0x%4.4x :", offset);
                }
                cpssOsPrintf(" %2.2x", bufferPtr[iter]);

                if((offset & 0x0F) == 0x0F)
                {
                    cpssOsPrintf("\n");
                }

                offset++;
            }/*iter*/
        }
        cpssOsPrintf("\n");
    }

    /* Move the buffer pointer to the start of the packet aftet the dsa tag.
     * The received frame will be part of the message data and should start after
     * the dsa tag. Since FW is in LE format and host CPU is in BE we need to
     * convert the message and data into BE format before processing it.
     */
    /* The frame sent from the FW will be of 3 types i.e.
     * New Flow, Removed Flow, Existent Flow.
     * The received frame will be of the below format :
     * ----------------------------------------------------------
     * Message Control (1B) + Message Data(NB depending on Message Type)
     * Sequence Number (Bits 0:23)
     * Opcode          (Bit  24:30)
     * TLV             (Bit  31)
     * For Frame type CPSS_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_NEW_FLOW_E message data is
     * Word 0     : Flow ID
     * Word 1     : Flow Key (Bit0-15 : reserved, Bit16-23 : key size, Bit24-31 : key number)
     * Word 2 ... : Flow key content and so on
     * For Frame type CPSS_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_REMOVED_FLOW_E /
     * CPSS_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_FLOW_DATA_E message data is
     * Word 0  : Flow ID
     * Word 1  : Byte count
     * Word 2  : Packet count
     * Word 3  : Drop count
     * Word 4  : Start timestamp
     */
    /* MAC bytes traversal */
    isfPktPtr = buffPtr + MAC_ADDR_LEN_IN_BYTES_CNS;
    do
    {
        /* Assuming received frame size as 24B converting it from native FW LE format to CPSS BE format */
        tempBuff[0] = (GT_U32)(CPSS_32BIT_LE(*((GT_U32*)isfPktPtr)));

        isfPktPtr += sizeof(GT_U32);
        tempBuff[1] = (GT_U32)(CPSS_32BIT_LE(*((GT_U32*)isfPktPtr)));

        isfPktPtr += sizeof(GT_U32);

        messageControl = tempBuff[0];
        sequenceNumber =  messageControl & 0xFFFFFF;

        /* ISF frame buffer can contain several messages each having initial 4B of message control
         * where initial 24bits form the sequence number. The sequence number is populated
         * only for the first message within the frame buffer and rest of the messages have
         * this field valus as 0 which is don't care so updating the sequenceNumberMessagePacket
         * with received sequence number from FW message packet only once for the entire message
         * packet parsing and same for the local comparator variable sequenceNumberTemp.
         */
        if(seqNoCheck == GT_FALSE)
        {
            sequenceNumberTemp = (flowMngDbPtr->seqNoCpss)++;
            sequenceNumberMessagePacket = sequenceNumber;
            seqNoCheck = GT_TRUE;
            /* Updating lastReadIsfMsgSeqPtr with the latest parsed message
             * sequence number to be used by FW for backpressure threshold
             * limit check.
             */
            /* rc = prvCpssDrvHwPpSetRegField(devNum, flowMngDbPtr->lastReadIsfMsgSeqPtr, 0, 32, flowMngDbPtr->seqNoCpss); */
#ifndef ASIC_SIMULATION
            rc = prvCpssHwPpWriteRegister(devNum, flowMngDbPtr->lastReadIsfMsgSeqPtr, flowMngDbPtr->seqNoCpss);
            if(rc != GT_OK)
            {
              return rc;
            }
#endif
        }

        /* Assuming short rule key size as 30B below will be a sample config pushed to FW:
         *
         * Byte\s |  0-1 |    2-3    |   4-7     |    8-11   | 12-16   | 17-29
         * Anchor | FIXED|    L2     |    L3     |    L3     | NOT_USED| NOT_VALID
         * Value  |PCL-ID| Offsets to|Offsets to | Offsets to|    XX   |   XX
         *                 Vlan ID   |  SIP      |  DIP      |         |
         *
         * Anchor Type NOT_VALID is used to signify what bytes of key (30B) and not relevant to
         * reduced key from FW perspective (actually out of 28B as Bytes 0-1 are reserved to PCL-ID
         * This distinguishes itself from NOT_USED anchor types which are considered for reduced
         * key bytes from FW.
         *
         * The ISF message from FW (native FW CPU LE format) for every new flow will be of the below format:
         *
         * Byte   | 0 1 2 3|4 5 6 7| 8    9 | 10 | 11|12  13| 14 15 16 17| 18 19 20 21| 22 23 24 25 26|
         * Content| Message|Flow Id|Reserved|Key |Key|VlanID|     SIP    |    DIP     |  0  0  0  0  0|
         *        | control|                 Size|Id |
         *
         *
         * The ISF message from FW (native FW CPU LE format) for every removed/existent flow will be of the below format:
         *
         * Byte   | 0 1 2 3|4 5 6 7| 8,9,10,11 | 12,13,14,15 |16,17,18,19|20,21,22,23|
         * Content| Message|Flow Id| Byte Count| Packet Count| Drop Count|  Timestamp|
         *        | control|
         *
         */
        /* Sequence number is base of synchronization mechanism between CPSS and FW.
         * If received messages are lost, then CPSS DB is no longer valid and in sync with FW
         * and so re-sync needs to happen.
         */
        if(sequenceNumberMessagePacket == sequenceNumberTemp)
        {
            opcode = ((messageControl >> 24) & 0x7f);
            /* Word 0 of message data contains Flow Id of the flows */
            flowId = tempBuff[1];
            if(flowMngDbPtr->debug_dumpIsfPacket)
            {
                cpssOsPrintf("CP ISF Message Flow ID %d\n",flowId);
            }

            /* TBD prvCpssDxChFlowManagerParseMessagePacketAndAddRule()message complete size depends on the pcl
             * udb keys config done from CPSS to FW accordingly message sent from FW to CPSS will
             * have the pcl keys content which must be converted to host CPU BE format and then
             * processed further to do DB management.
             */
            switch(opcode)
            {
                case PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_NEW_FLOW_E:
                    /* Word 1 of message data for new flow contains key number and size with reserved bits */
                    tempBuff[2] = (GT_U32)(CPSS_32BIT_LE(*((GT_U32*)isfPktPtr)));

                    isfPktPtr += sizeof(GT_U32);

                    keyNumberAndSize = tempBuff[2];
                    keySize = (keyNumberAndSize & 0x00FF0000) >> 16;
                    keyId = (keyNumberAndSize & 0xFF000000) >> 24;
                    if(flowMngDbPtr->debug_dumpIsfPacket)
                    {
                        cpssOsPrintf("CP ISF Message keySize %d and keyId %d\n",
                                     keySize, keyId);
                    }
                    cpssOsMemSet(&newFlow, 0 ,sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_STC));
                    newFlow.keySize = keySize;
                    newFlow.keyId = keyId;

                    /* Store the isf flow key content for the new flow to be notified to
                     * the application. CPSS does not have idea about the pre-defined
                     * fields used by application for flow key configuration so CPSS
                     * will just store the reduced flow key content and pass to application
                     * which can then map it based on its provisioning.
                     */
                    /* Every isf frame corresponding to new flow will begin
                     * with 12B MAC + 12B message header = 24B followed by reduced rule
                     * flow key data and then subsequent messages will have the
                     * same format of 12B hdr and reduced rule flow key content
                     */
                    inKeyDataPtr = isfPktPtr;
                    for(iter = 0; iter < flowMngDbPtr->cpssFwCfg.reducedRuleSize; iter++)
                    {
                        newFlow.keyData[iter] = *(inKeyDataPtr + iter);
                    }

                    prvCpssDxChFlowManagerParseMessagePacketAndAddRule(devNum, flowMngDbPtr, isfPktPtr, opcode, flowId, &newFlow);

                    /* Store flow related data to local cpss db */
                    cpssFlowId = flowMngDbPtr->fwToCpssFlowDb[flowId];
                    flowMngDbPtr->flowDataDb[cpssFlowId].keySize = keySize;
                    flowMngDbPtr->flowDataDb[cpssFlowId].keyId = keyId;
                    flowMngDbPtr->flowDataDb[cpssFlowId].flowId = cpssFlowId;
                    newFlow.flowId = cpssFlowId;

                    isfPktPtr += flowMngDbPtr->cpssFwCfg.reducedRuleSize;
                    flowMngDbPtr->messageNewFlowCount++;
                    if(flowMngDbPtr->reSyncCount)
                    {
                        flowMngDbPtr->messageNewFlowCountAfterResync++;
                    }
                    break;

                case PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_REMOVED_FLOW_E:
                    prvCpssDxChFlowManagerParseMessagePacketAndAddRule(devNum, flowMngDbPtr, isfPktPtr, opcode, flowId, &newFlow);
                    isfPktPtr += FLOW_KEY_ENTRY_LEN_IN_BYTES_CNS;
                    flowMngDbPtr->messageRemovedFlowCount++;
                    if(flowMngDbPtr->reSyncCount)
                    {
                        flowMngDbPtr->messageRemovedFlowCountAfterResync++;
                    }
                    break;

                case PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_FLOW_DATA_E:
                    /* Avoid DB processing for invalid flow indexes */
                    if(flowMngDbPtr->fwToCpssFlowDb[flowId] != PRV_CPSS_FLOW_MANAGER_INVALID_FLOW_INDEX_CNS)
                    {
                        prvCpssDxChFlowManagerParseMessagePacketAndAddRule(devNum, flowMngDbPtr, isfPktPtr, opcode, flowId, &newFlow);
                    }
                    isfPktPtr += FLOW_KEY_ENTRY_LEN_IN_BYTES_CNS;
                    flowMngDbPtr->messageFlowDataGetCount++;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    break;
            }
        }
        else
        {
            /* There is a missed message from FW to CPSS and so we need to re-sync CPSS DB with FW DB by invoking re-sync api */
            rc = cpssDxChFlowManagerReSync(flowMngId);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("cpssDxChFlowManagerReSync returned not GT_OK\n");
                return rc;
            }
            flowMngDbPtr->messageSequenceNumberForResync = sequenceNumber;
            break;
        }

        /* ISF frame buffer marks the end of the messages within the frame by adding additional
         * opcode (end opcode of word format) with value as 0xFF signify the end of the messages.
         */
        tempBuff[3] = (GT_U32)(CPSS_32BIT_LE(*((GT_U32*)isfPktPtr)));
        iterCheck = tempBuff[3] >> 24;
    }while((iterCheck & PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_END_E) != 0xFF);

    return rc;
}


/**
* @internal cpssDxChFlowManagerMesageParseAndCpssDbUpdate function
* @endinternal
*
* @brief   Parse the received frames and update the CPSS DB accordingly.
*          Also notify the application about the newly learnt flows
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] numOfBuff                - Num of buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_PTR               - on NULL inout ptr.
*/
GT_STATUS cpssDxChFlowManagerMessageParseAndCpssDbUpdate
(
    IN    GT_U8                                      devNum,
    IN    GT_U32                                     numOfBuff,
    IN    GT_U8                                      *packetBuffsArrPtr[],
    IN    GT_U32                                     buffLenArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerMessageParseAndCpssDbUpdate);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfBuff, packetBuffsArrPtr, buffLenArr));

    rc = internal_cpssDxChFlowManagerMessageParseAndCpssDbUpdate(devNum, numOfBuff, packetBuffsArrPtr, buffLenArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfBuff, packetBuffsArrPtr, buffLenArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssDxChFlowManagerKeyEntryUserDefinedByteSet function
* @endinternal
*
* @brief Set the anchor type, offset and mask at given udb index for key table index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId               - flow manager id
* @param [in] keyTableIndex           - key table index
* @param [in] keySize                 - key size
* @param [in] udbIndex                - udb index
* @param [in] offsetType              - the type of offset
* @param [in] offset                  - the offset of UDB in the offset type
* @param [in] mask                    - mask to use to match the udb fields
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
* @note 1. Number of valid udb indexes depends on the tcam rule size so if tcam short rule size is 20B
*       then we can have max of 20 udb indexes and rest are invalid
*       2. Mask value 0 is a valid mask for all the anchor types
*       3. Offset value should always be in the range of 0-127 i.e. first 128 bytes of the packet
*
*/
GT_STATUS internal_cpssDxChFlowManagerKeyEntryUserDefinedByteSet
(
    IN GT_U32                                  flowMngId,
    IN GT_U32                                  keyTableIndex,
    IN CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT     keySize,
    IN GT_U32                                  udbIndex,
    IN CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT       offsetType,
    IN GT_U8                                   offsetOrPattern,
    IN GT_U8                                   mask
)
{
    GT_STATUS                            rc = GT_OK;
    PRV_CPSS_DXCH_IPC_MSG_STC            ipc_msg_send;
    PRV_CPSS_DXCH_IPC_RECV_MSG_STC       ipc_msg_recv;
    GT_U32                               ii;
    GT_U32                               index;
    CPSS_DXCH_FLOW_MNG_PTR               flowMngDbPtr;
#ifndef ASIC_SIMULATION
    GT_U32                               ipcRecvKeySize;
    GT_U32                               ipcRecvUdbIndex;
    GT_U32                               ipcRecvOffsetType;
    GT_U32                               ipcRecvKeyTableIndex;
#endif

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));

    /* As per Flow Manager design, allowed pcl key size is 20/30/60B.
     */
    switch (keySize)
    {
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E:
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E:
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_60B_E:
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* As per Flow Manager design, byte 0,1 of PCL keys are reserved for the pcl-id
     * which is of fixed anchor type so validating it.
     */
    if ((udbIndex == 0 || udbIndex == 1) &&
        offsetType != CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    switch (offsetType)
    {
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E:
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E:
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E:
        /* offsetOrPattern for anchor types L2/L3/L4 are applicable from only
         * first 128 bytes of the packet.
         */
        if (offsetOrPattern >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        break;
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E:
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E:
        if (offsetOrPattern >= BIT_1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        break;
        /* offsetOrPattern field is not significant for below anchor types
         * as they are not packet fields but are part of metadata so
         * handled differently.
         */
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E:
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E:
        case CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E:
        break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    }

    switch (flowMngDbPtr->cpssFwCfg.tcamShortRuleSize)
    {
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E:
        if(udbIndex >= 20)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E:
        if(udbIndex >= 30)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
        case CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_60B_E:
        /* TBD how to process this key size */
        break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Store the udb configuration for given key table index in local flow manager db */
    flowMngDbPtr->keyTableUdbCfg[keyTableIndex].keySize = keySize;
    flowMngDbPtr->keyTableUdbCfg[keyTableIndex].content[udbIndex].anchor = offsetType;
    flowMngDbPtr->keyTableUdbCfg[keyTableIndex].content[udbIndex].offsetOrPattern = offsetOrPattern;
    flowMngDbPtr->keyTableUdbCfg[keyTableIndex].content[udbIndex].mask = mask;
    ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_SET_UDB_KEY_E;

    index = 0;
    ipc_msg_send.data[index++] = (GT_U8)keyTableIndex;
    ipc_msg_send.data[index++] = (GT_U8)keySize;
    /* Temporariy to support the new keyType as Cache */
    ipc_msg_send.data[index++] = 1; /* place holder for keyType to be added to DB and implemented */
    ipc_msg_send.data[index++] = (GT_U8)udbIndex;
    ipc_msg_send.data[index++] = (GT_U8)offsetType;
    ipc_msg_send.data[index++] = (GT_U8)offsetOrPattern;
    ipc_msg_send.data[index++] = (GT_U8)mask;

    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        /* Send IPC message to FW passing the init params */
#ifndef ASIC_SIMULATION
        if(PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, ii))
        {
            rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(ii, flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(ipc_msg_recv.data[1] == GT_OUT_OF_RANGE)
            {
                ipcRecvKeyTableIndex = ipc_msg_recv.data[2];
                cpssOsPrintf("Configuration failed due to out of range key table index %d\n", ipcRecvKeyTableIndex);
                return rc;
            }
            else if(ipc_msg_recv.data[1] == GT_BAD_PARAM)
            {
                ipcRecvKeySize = ipc_msg_recv.data[2];
                ipcRecvUdbIndex = ipc_msg_recv.data[3];
                ipcRecvOffsetType = ipc_msg_recv.data[4];
                cpssOsPrintf("Configuration failed due to one of the incorrect parameters:\n"
                             "Key size:%d, UDB Index:%d, Anchor Type:%s\n",
                             ipcRecvKeySize, ipcRecvUdbIndex,
                             ipcRecvOffsetType == 0 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E" :
                             ipcRecvOffsetType == 1 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E" :
                             ipcRecvOffsetType == 2 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E" :
                             ipcRecvOffsetType == 3 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E" :
                             ipcRecvOffsetType == 4 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E" :
                             ipcRecvOffsetType == 5 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E" :
                             ipcRecvOffsetType == 6 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E" :
                             ipcRecvOffsetType == 7 ? "CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E" :
                             "BAD PARAMETER"
                );
                return rc;
            }
        }
#else
        /* To pass compilation for WM ipc_msg_send needs to be used */
        if(ipc_msg_send.opcode == PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_SET_UDB_KEY_E)
        {
        }
#endif
    }

    return rc;
}

/**
* @internal cpssDxChFlowManagerKeyEntryUserDefinedByteSet function
* @endinternal
*
* @brief Set the anchor type, offset and mask at given udb index for key table index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId               - flow manager id
* @param [in] keyTableIndex           - key table index
* @param [in] keySize                 - key size
* @param [in] udbIndex                - udb index
* @param [in] offsetType              - the type of offset
* @param [in] offset                  - the offset of UDB in the offset type
* @param [in] mask                    - mask to use to match the udb fields
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS cpssDxChFlowManagerKeyEntryUserDefinedByteSet
(
    IN GT_U32                                  flowMngId,
    IN GT_U32                                  keyTableIndex,
    IN CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT     keySize,
    IN GT_U32                                  udbIndex,
    IN CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT       offsetType,
    IN GT_U8                                   offsetOrPattern,
    IN GT_U8                                   mask
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerKeyEntryUserDefinedByteSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, keyTableIndex, keySize, udbIndex, offsetType, offsetOrPattern, mask));

    rc = internal_cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySize, udbIndex,
                                                                offsetType, offsetOrPattern, mask);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, keyTableIndex, keySize, udbIndex, offsetType, offsetOrPattern, mask));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerKeyEntryUserDefinedByteGet function
* @endinternal
*
* @brief   Get the UDB fields programmed to the FW for the given key and udb index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*                                  Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - flow manager id.
* @param[in] keyTableIndex            - key table index.
* @param[in] udbIndex                 - udb index for which information is needed, unused udb index FF for complete dump of key table index.
* @param[out]keySizePtr               - (pointer to) key size
* @param[out]offsetTypePtr            - (pointer to) The type of offset.
* @param[out]offsetPtr                - (pointer to) The offset of UDB, in
*                                       bytes, from the place in packet
*                                       indicated by offset type.
* @param[out]maskPtr                  - (pointer to) The mask type.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS internal_cpssDxChFlowManagerKeyEntryUserDefinedByteGet
(
    IN  GT_U32                                     flowMngId,
    IN  GT_U32                                     keyTableIndex,
    IN  GT_U32                                     udbIndex,
    OUT CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT        *keySizePtr,
    OUT CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT          *offsetTypePtr,
    OUT GT_U8                                      *offsetPtr,
    OUT GT_U8                                      *maskPtr
)
{
    CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr;

    CPSS_NULL_PTR_CHECK_MAC(keySizePtr);
    CPSS_NULL_PTR_CHECK_MAC(offsetTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    if(flowMngDbPtr->debug_dumpIsfPacket)
    {
        cpssOsPrintf("Flow Manager Key Table UDB Index %d Dump \n",udbIndex);
        cpssOsPrintf("Key Size\t Anchor Type\t OffsetOrPattern\t Mask\n");
    }

    *keySizePtr     = flowMngDbPtr->keyTableUdbCfg[keyTableIndex].keySize;
    *offsetTypePtr  = flowMngDbPtr->keyTableUdbCfg[keyTableIndex].content[udbIndex].anchor;
    *offsetPtr      = flowMngDbPtr->keyTableUdbCfg[keyTableIndex].content[udbIndex].offsetOrPattern;
    *maskPtr        = flowMngDbPtr->keyTableUdbCfg[keyTableIndex].content[udbIndex].mask;

    if(flowMngDbPtr->debug_dumpIsfPacket)
    {
        cpssOsPrintf("%d\t%d\t%d\t%d\n",
                     keySizePtr,
                     offsetTypePtr[udbIndex],
                     offsetPtr[udbIndex],
                     maskPtr[udbIndex]);
    }
    return GT_OK;
}

/**
* @internal cpssDxChFlowManagerKeyEntryUserDefinedByteGet function
* @endinternal
*
* @brief   Get the UDB fields programmed to the FW for the given key and udb index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - flow manager id.
* @param[in] keyTableIndex            - key table index.
* @param[in] udbIndex                 - udb index.
* @param[in] keySizePtr               - (pointer to) PCL key size.
* @param[out]offsetTypePtr            - (pointer to) The type of offset.
* @param[out]offsetPtr                - (pointer to) The offset of UDB, in
*                                       bytes, from the place in packet
*                                       indicated by offset type.
* @param[out]maskPtr                  - (pointer to) The mask type.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChFlowManagerKeyEntryUserDefinedByteGet
(
    IN  GT_U32                                     flowMngId,
    IN  GT_U32                                     keyTableIndex,
    IN  GT_U32                                     udbIndex,
    OUT CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT        *keySizePtr,
    OUT CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT          *offsetTypePtr,
    OUT GT_U8                                      *offsetPtr,
    OUT GT_U8                                      *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerKeyEntryUserDefinedByteGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, keyTableIndex, udbIndex, keySizePtr, offsetTypePtr, offsetPtr, maskPtr));

    rc = internal_cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, keySizePtr,
                                                                offsetTypePtr, offsetPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, keyTableIndex, udbIndex, keySizePtr, offsetTypePtr, offsetPtr, maskPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet function
* @endinternal
*
* @brief Set the source port, packet type mapping to the key index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number.
* @param [in] sourcePort            - source port number
* @param [in] keyIndexArr           - array of key index based on packet type, index to packet mapping
*                                     index    packet type
*                                       0       IPV4 TCP
*                                       1       IPV4 UDP
*                                       2       IPV6 TCP
*                                       3       IPV6 UDP
*                                       4        OTHER
*                                     APPLICABLE RANGES 0...15
* @param [in] enableBitmap          - enable/disable featur bitmap per packet type per port
*                                       BIT 0 - IPV4 TCP
*                                       BIT 1 - IPV4 UDP
*                                       BIT 2 - IPV6 TCP
*                                       BIT 3 - IPV6 UDP
*                                       BIT 4 - OTHER
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS internal_cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet
(
    IN GT_U8          devNum,
    IN GT_PORT_NUM    sourcePort,
    IN GT_U32         keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    IN GT_U32         enableBitmap
)
{
    GT_STATUS                            rc = GT_OK;
    PRV_CPSS_DXCH_IPC_MSG_STC            ipc_msg_send;
    PRV_CPSS_DXCH_IPC_RECV_MSG_STC       ipc_msg_recv;
    GT_U32                               index;
    CPSS_DXCH_FLOW_MNG_PTR               flowMngDbPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC          *pDev;
    GT_U32                               flowMngId;
#ifndef ASIC_SIMULATION
    GT_U32                               ipcRecvPortNumber;
    GT_U32                               ipcRecvKeyBind;
#endif

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, sourcePort);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if (enableBitmap >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(index = 0; index < CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E; index++)
    {
        if (keyIndexArr[index] > 16)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));
    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    flowMngId = pDev->flowMgrInfo.flowManagerId;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    index = 0;
    ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_SET_PORT_KEY_E;

    ipc_msg_send.data[index++] = (GT_U8)sourcePort;

    /* Key Index per port for flow key packet types are assigned values from 0-15
     * and value 16 is used to signify the for the given port and packet type,
     * flow classification will not happen.
     */
    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = (enableBitmap & 0x01) ?
               (GT_U8)(keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E]) : FLOW_KEY_DISABLED;
    ipc_msg_send.data[index++] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E];

    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = (enableBitmap & 0x02) ?
               (GT_U8)(keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E]) : FLOW_KEY_DISABLED;
    ipc_msg_send.data[index++] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E];

    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = (enableBitmap & 0x04) ?
               (GT_U8)(keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E]) : FLOW_KEY_DISABLED;
    ipc_msg_send.data[index++] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E];

    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = (enableBitmap & 0x8) ?
               (GT_U8)(keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E]) : FLOW_KEY_DISABLED;
    ipc_msg_send.data[index++] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E];

    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = (enableBitmap & 0x10) ?
               (GT_U8)(keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E]) : FLOW_KEY_DISABLED;
    ipc_msg_send.data[index++] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E];

    /* Store the key index to packet type mapping configuration in local flow manager db */
    flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[0] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E];
    flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[1] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E];
    flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[2] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E];
    flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[3] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E];
    flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[4] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E];
    flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].enableBitmap = enableBitmap;

    /* Send IPC message to FW passing key table index configuration */
#ifndef ASIC_SIMULATION
    rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(devNum, flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(ipc_msg_recv.data[1] == GT_OUT_OF_RANGE)
    {
        ipcRecvPortNumber = ipc_msg_recv.data[2];
        cpssOsPrintf("Configuration failed due to out of range port number %d\n", ipcRecvPortNumber);
        return rc;
    }
    else if(ipc_msg_recv.data[1] == GT_BAD_PARAM)
    {
        ipcRecvKeyBind = ipc_msg_recv.data[2];
        cpssOsPrintf("Configuration failed due to wrong key bind received:\n",
                     ipcRecvKeyBind);
        return rc;
    }
#else
    /* To pass compilation for WM ipc_msg_send needs to be used */
    if(ipc_msg_send.opcode == PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_SET_UDB_KEY_E)
    {
    }
#endif
    return rc;
}

/**
* @internal cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet function
* @endinternal
*
* @brief Set the source port, packet type mapping to the key index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number.
* @param [in] sourcePort            - source port number
* @param [in] keyIndexArr           - array of key index based on packet type, index to packet mapping
*                                     index    packet type
*                                       0       IPV4 TCP
*                                       1       IPV4 UDP
*                                       2       IPV6 TCP
*                                       3       IPV6 UDP
*                                       4        OTHER
*                                     APPLICABLE RANGES 0...15
* @param [in] enableBitmap          - enable/disable featur bitmap per packet type per port
*                                       BIT 0 - IPV4 TCP
*                                       BIT 1 - IPV4 UDP
*                                       BIT 2 - IPV6 TCP
*                                       BIT 3 - IPV6 UDP
*                                       BIT 4 - OTHER
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet
(
    IN GT_U8          devNum,
    IN GT_PORT_NUM    sourcePort,
    IN GT_U32         keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    IN GT_U32         enableBitmap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourcePort, keyIndexArr, enableBitmap));

    rc = internal_cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(devNum, sourcePort, keyIndexArr, enableBitmap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourcePort, keyIndexArr, enableBitmap));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet function
* @endinternal
*
* @brief Get the key index based on the source port.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]   devNum                - device number.
* @param [in]  sourcePort            - source port number
* @param [out] keyIndexArr           - key index array per packet type
* @param [out] enableBitmapPtr       - bitmap ptr for enable/disable per packet type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS internal_cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     sourcePort,
    OUT GT_U32          keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    OUT GT_U32          *enableBitmapPtr
)
{
    CPSS_DXCH_FLOW_MNG_PTR               flowMngDbPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC          *pDev;
    GT_U32                               flowMngId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, sourcePort);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enableBitmapPtr);
    CPSS_NULL_PTR_CHECK_MAC(keyIndexArr);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    flowMngId = pDev->flowMgrInfo.flowManagerId;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[0];
    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[1];
    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[2];
    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[3];
    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[4];
    *enableBitmapPtr = flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].enableBitmap;

    if(flowMngDbPtr->debug_dumpIsfPacket)
    {
        cpssOsPrintf("Flow Manager key index mapping configuration for port %dis\n",sourcePort);
        cpssOsPrintf("Key Index \tPacket Type\n");
        cpssOsPrintf("%d\t IPV4 TCP\n",keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E]);
        cpssOsPrintf("%d\t IPV4 UDP\n",keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E]);
        cpssOsPrintf("%d\t IPV6 TCP\n",keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E]);
        cpssOsPrintf("%d\t IPV6 UDP\n",keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E]);
        cpssOsPrintf("%d\t OTHER\n",keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E]);
        cpssOsPrintf("Enable Bitmap per packet type %d\n",*enableBitmapPtr);
    }

    return GT_OK;
}

/**
* @internal cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet function
* @endinternal
*
* @brief Get the key index based on the source port.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]   devNum                - device number.
* @param [in]  sourcePort            - source port number
* @param [out] keyIndexArr           - key index array per packet type
* @param [out] enableBitmapPtr       - bitmap ptr for enable/disable per packet type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     sourcePort,
    OUT GT_U32          keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    OUT GT_U32          *enableBitmapPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourcePort, keyIndexArr, enableBitmapPtr));

    rc = internal_cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet(devNum, sourcePort, keyIndexArr, enableBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourcePort, keyIndexArr, enableBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChFlowManagerSrcPortCheck function
* @endinternal
*
* @brief   Function to check source port from the received flows in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngDbPtr        - (pointer to)Flow Manager structure
* @param[in] keyIdFlow           - key id of the flow
* @param[in] index               - flow id as per cpss flow db
* @param[in] sourcePort          - source port of the device
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - none of the flow ids match the given source port
*/
static GT_STATUS prvCpssDxChFlowManagerSrcPortCheck
(
    IN  PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr,
    IN  GT_U32                              keyIdFlow,
    IN  GT_U32                              index,
    IN  GT_U32                              sourcePort
)
{
    GT_U32                               sourcePortUdbIndex;
    GT_U32                               sourcePortKeyData;
    GT_U32                               udbIndex;

    /* Parse through all the possible udb indexes for the given keyId to
     * derive the udbIndex location representing source port.
     */
    for(udbIndex = 0; udbIndex < flowMngDbPtr->keyTableUdbCfg[keyIdFlow].keySize; udbIndex++)
    {
        if(flowMngDbPtr->keyTableUdbCfg[keyIdFlow].content[udbIndex].anchor == CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E)
        {
            /* Per design, 1st 2B of PCL UDB key configuration denotes
             * PCL-ID which is not part of reduced key data sent from FW to
             * CPSS.
             */
            sourcePortUdbIndex = udbIndex - 2;
            sourcePortKeyData = flowMngDbPtr->flowDataDb[index].keyData[sourcePortUdbIndex];
            if(sourcePortKeyData == sourcePort)
            {
                return GT_OK;
            }
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
}


/**
* @internal internal_cpssDxChFlowManagerSrcPortFlowLearntGet function
* @endinternal
*
* @brief To get flows learnt per source port for statistics/monitoring purpose.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  devNum                - unique device number
* @param [in]  sourcePort            - source port number
* @param [out] flowsLearntPtr        - (pointer to) Number of learnt flows per packet type
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS internal_cpssDxChFlowManagerSrcPortFlowLearntGet
(
    IN  GT_U8                                                devNum,
    IN  GT_PORT_NUM                                          sourcePort,
    OUT CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC         *flowsLearntPtr
)
{
    CPSS_DXCH_FLOW_MNG_PTR               flowMngDbPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC          *pDev;
    GT_U32                               flowMngId;
    GT_U32                               keyIdFlow;
    GT_U32                               flowId;
    GT_STATUS                            rc = GT_OK;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC    *tempPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, sourcePort);
    CPSS_NULL_PTR_CHECK_MAC(flowsLearntPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    flowMngId = pDev->flowMgrInfo.flowManagerId;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    tempPtr = flowMngDbPtr->cpssFlowUsedListHeader;
    if (tempPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    while(tempPtr)
    {
        flowId = tempPtr->cpssFlowIndex;
        /* Extract the keyId for new flow stored in cpss db */
        keyIdFlow = flowMngDbPtr->flowDataDb[flowId].keyId;
        rc = prvCpssDxChFlowManagerSrcPortCheck(flowMngDbPtr, keyIdFlow, flowId, sourcePort);
        if(rc == GT_OK)
        {
            if(flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[0] == keyIdFlow)
            {
                flowsLearntPtr->ipv4TcpFlowsLearnt++;
            }
            else if(flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[1] == keyIdFlow)
            {
                flowsLearntPtr->ipv4UdpFlowsLearnt++;
            }
            else if(flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[2] == keyIdFlow)
            {
                flowsLearntPtr->ipv6TcpFlowsLearnt++;
            }
            else if(flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[3] == keyIdFlow)
            {
                flowsLearntPtr->ipv6UdpFlowsLearnt++;
            }
            else if(flowMngDbPtr->devInfo[devNum]->keyIndexMapCfg[sourcePort].keyIndexArr[4] == keyIdFlow)
            {
                flowsLearntPtr->otherTypeFlowsLearnt++;
            }
        }
        tempPtr = tempPtr->next;
    }

    if(flowMngDbPtr->debug_dumpIsfPacket)
    {
        cpssOsPrintf("Flows learnt on port %d\n",sourcePort);
        cpssOsPrintf("IPV4 TCP Flows: %d IPV4 UDP Flows: %d\n"
            "IPV6 TCP Flows: %d IPV6 UDP Flows: %d\n"
            "OTHER Flows:%d\n",
            flowsLearntPtr->ipv4TcpFlowsLearnt,
            flowsLearntPtr->ipv4UdpFlowsLearnt,
            flowsLearntPtr->ipv6TcpFlowsLearnt,
            flowsLearntPtr->ipv6UdpFlowsLearnt,
            flowsLearntPtr->otherTypeFlowsLearnt);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChFlowManagerSrcPortFlowLearntGet function
* @endinternal
*
* @brief To get flows learnt per source port for statistics/monitoring purpose.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  devNum                - unique device number
* @param [in]  sourcePort            - source port number
* @param [out] flowsLearntPtr        - (pointer to) Number of learnt flows per packet type
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerSrcPortFlowLearntGet
(
    IN  GT_U8                                                devNum,
    IN  GT_PORT_NUM                                          sourcePort,
    OUT CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC         *flowsLearntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerSrcPortFlowLearntGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourcePort, flowsLearntPtr));

    rc = internal_cpssDxChFlowManagerSrcPortFlowLearntGet(devNum, sourcePort, flowsLearntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourcePort, flowsLearntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFlowManagerFlowCountersToDbGetIpcSendRecv function
* @endinternal
*
* @brief   Send and Receive IPC message from/to FW inorder to get flow based
*          counters.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngDbPtr        - (pointer to)Flow Manager structure
* @param[in] ipc_msg_send        - (pointer to)ipc message send structure
* @param[out]ipc_msg_recv        - (pointer to)ipc message receive structure
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
* @retval GT_NO_MORE               - no messages ready
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
*/
static GT_STATUS prvCpssDxChFlowManagerFlowCountersToDbGetIpcSendRecv
(
    IN  PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr,
    IN  PRV_CPSS_DXCH_IPC_MSG_STC           *ipc_msg_send,
    OUT PRV_CPSS_DXCH_IPC_RECV_MSG_STC      *ipc_msg_recv
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS                            rc = GT_OK;
    GT_U32                               ii;
    GT_U32                               ipcRecvFlowDataRequest;
    GT_U32                               ipcRecvFlowId;

    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        /* Send IPC message to FW to get flow data */
        if(PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, ii))
        {
            rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(ii, flowMngDbPtr, ipc_msg_send, ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(ipc_msg_recv->data[1] == GT_OUT_OF_RANGE)
            {
                ipcRecvFlowId = ipc_msg_recv->data[3];
                ipcRecvFlowId = (ipcRecvFlowId << 8) | ipc_msg_recv->data[2];
                cpssOsPrintf("Out of range flow index %d\n", ipcRecvFlowId);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            else if(ipc_msg_recv->data[1] == GT_FAIL)
            {
                ipcRecvFlowDataRequest = ipc_msg_recv->data[2];
                cpssOsPrintf("Incorrect flow data request %d\n", ipcRecvFlowDataRequest);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
#else
    /* To pass compilation for WM simulation */
    if(flowMngDbPtr || ipc_msg_send || ipc_msg_recv)
    {
    }
#endif
    return GT_OK;
}

/**
* @internal internal_cpssDxChFlowManagerFlowCountersToDbGet function
* @endinternal
*
* @brief To send flow counters get request to FW to retrieve the IPFIX HW counters regularly and
*        store it in CPSS DB for further usage of application.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId             - unique flow manager id
* @param [in] flowIdCount           - number of flow ids for which counters is requested
* @param [in] flowIdArr             - array of flow ids for which counter is requested from FW
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
* @retval GT_NO_MORE               - no messages ready
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS internal_cpssDxChFlowManagerFlowCountersToDbGet
(
    IN  GT_U32   flowMngId,
    IN  GT_U32   flowIdCount,
    IN  GT_U32   flowIdArr[]
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                               flowId;
    GT_U32                               fwIndex;
    GT_U32                               index;
    PRV_CPSS_DXCH_IPC_MSG_STC            ipc_msg_send;
    PRV_CPSS_DXCH_IPC_RECV_MSG_STC       ipc_msg_recv;
    CPSS_DXCH_FLOW_MNG_PTR               flowMngDbPtr;
    GT_U32                               tempFlowIdCount;
    GT_U32                               flowIdModulo; /* modulo to send bulk ipc requests to FW with alignment with message size */
    GT_U32                               moduloIter; /* iterator to track every next bulk request formation */
    GT_U32                               flowIdRem; /* to track the remaining flow id count for which requests have to be sent */

    CPSS_NULL_PTR_CHECK_MAC(flowIdArr);
    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    if(flowIdCount > MAX_NUM_OF_FW_FLOW_INDEX_CNS || flowIdCount == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
    cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));
    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);

    index = 0;
    ipc_msg_send.data[index++] = 0;
    /* iterator to track the flow ids count for which the request has been made
     * to fetch flow counters to CPSS DB.
     */
    tempFlowIdCount = 0;
    flowIdRem = flowIdCount;

    /* PRV_CPSS_DXCH_IPC_MSG_STC message size is 191B which includes 190B
     * buffer and 1B message opcode so aligning the modulo with required
     * message buffer size
     */
    flowIdModulo = (sizeof(PRV_CPSS_DXCH_IPC_MSG_STC) - 1) / sizeof(GT_U32);
    if(flowIdCount <= flowIdModulo)
    {
        ipc_msg_send.data[index++] = flowIdCount;
    }
    else
    {
        ipc_msg_send.data[index++] = flowIdModulo;
    }
    moduloIter = 0;

    ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_GET_FLOW_DATA_E;
    /* Please note since this api is meant to retrieve flow counters from FW DB
     * to CPSS DB it is expected that the requested flow ids are present in the
     * FW DB when the request is made else this api would not retrieve anything
     * but the return code would be GT_OK since handling valid flow ids and
     * invalid flow ids in the same request cannot give different return code
     * for same request.
     */
    for (; 1; )
    {
        flowId = flowIdArr[tempFlowIdCount];
        /* Flow counters get request from cpss/application can be sent only for
         * valid flow ids existing in the FW. CPSS maintains local DB to track
         * the flow ids active in the FW by means of flag and also internally
         * maps it to local flow id so the conversion needs to be done before
         * making a request to the FW from CPSS. FW also uses 4B of flow ids so
         * aligning with that.
         */
        if(flowMngDbPtr->flowState[flowId] == GT_TRUE)
        {
            for(fwIndex = 0; fwIndex < flowMngDbPtr->cpssFwCfg.flowDBSize; fwIndex++)
            {
                if(flowMngDbPtr->fwToCpssFlowDb[fwIndex] == flowId)
                {
                    flowId = fwIndex;
                    flowId = CPSS_32BIT_LE(flowId);
                    cpssOsMemCpy(&ipc_msg_send.data[index], &flowId, sizeof(GT_U32));
                    index += sizeof(GT_U32);
                    moduloIter++;
                    break;
                }
            }
        }
        tempFlowIdCount++;
        /* When the requested flow count is reached, fetch the counters and
         * break out from the loop.
         */
        if(tempFlowIdCount == flowIdCount)
        {
            rc = prvCpssDxChFlowManagerFlowCountersToDbGetIpcSendRecv(flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;
        }

        /* Sending requests to fetch ipfix counters for every flowIdModulo flows in
         * alignment with the ipc message buffer size and reinit the ipc
         * message send data byte 1 and 2 for every new bulk request to be sent.
         */
        if((moduloIter == flowIdModulo))
        {
            rc = prvCpssDxChFlowManagerFlowCountersToDbGetIpcSendRecv(flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
            if(rc != GT_OK)
            {
                return rc;
            }
            moduloIter = 0;
            index = 0;
            ipc_msg_send.data[index++] = 0;
            flowIdRem -= flowIdModulo;
            if(flowIdRem <= flowIdModulo)
            {
                ipc_msg_send.data[index++] = flowIdRem;
            }
            else
            {
                ipc_msg_send.data[index++] = flowIdModulo;
            }
        }
    }

    return rc;
}


/**
* @internal cpssDxChFlowManagerFlowCountersToDbGet function
* @endinternal
*
* @brief To send flow counters get request to FW to retrieve the IPFIX HW counters regularly and
*        store it in CPSS DB for further usage of application.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId             - unique flow manager id
* @param [in] flowIdCount           - number of flow ids for which counters is requested
* @param [in] flowIdArr             - array of flow ids for which counter is requested from FW
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
* @retval GT_NO_MORE               - no messages ready
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerFlowCountersToDbGet
(
    IN  GT_U32   flowMngId,
    IN  GT_U32   flowIdCount,
    IN  GT_U32   flowIdArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerFlowCountersToDbGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, flowIdCount, flowIdArr));

    rc = internal_cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, flowIdArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, flowIdCount, flowIdArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerFlowIdGetNext function
* @endinternal
*
* @brief To get the flow id from the list of flow ids maintained in cpss db. The flow ids are maintained
*        in the list based on their insertion time order and the get request can be either from the
*        beginning of the list or can continue from where it last left.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngId             - unique flow manager id
* @param[in]   getFirst              - GT_TRUE - get first flow id
*                                      GT_FALSE - get subsequent flow ids
* @param [out] nextFlowIdPtr         - retrieve the existing flow id
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NO_MORE                 - no flow id exists in DB.
*/
GT_STATUS internal_cpssDxChFlowManagerFlowIdGetNext
(
    IN  GT_U32  flowMngId,
    IN  GT_BOOL getFirst,
    OUT GT_U32  *nextFlowIdPtr
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *tempUsedListHeadPtr = NULL;

    CPSS_NULL_PTR_CHECK_MAC(nextFlowIdPtr);
    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    /* CPSS maintains 2 kinds of linked list in order to track the free and
     * and used entries of the flow DB. All the entries of the cpss flow manager
     * db are initially part of free linked list and then based on every new
     * incoming flow one of the entris from the free list is assigned for this
     * flow and that becomes part of the used linked list. In used linked list
     * for efficient traversal purpose we have head and tail pointers so every
     * new flow is added to the head end and tail keeps track of the first added
     * entry also helpful for LRU algorithm.
     */
    if (getFirst == GT_TRUE)
    {
        tempUsedListHeadPtr = flowMngDbPtr->cpssFlowUsedListHeader;
    }
    else
    {
        tempUsedListHeadPtr = flowMngDbPtr->globalUsedListPtr;
    }

    if (tempUsedListHeadPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    /* Application may ask for the existing flow IDs for which data may be requested.
     * In case First existing id is requested then getFirst is set as TRUE. For other
     * case getFirst is FALSE and we traverse the used linked list from the place where
     * it last left.
     * Eg cases: Below H and T denote head and tail ptr.
     *              H/T
     * Used List -> NULL
     *             H/T
     * Used List -> 1 -> NULL
     *              H    T
     * Used List -> 2 -> 1 -> NULL
     */
    while(tempUsedListHeadPtr)
    {
        *nextFlowIdPtr = tempUsedListHeadPtr->cpssFlowIndex;
        tempUsedListHeadPtr = tempUsedListHeadPtr->next;
        flowMngDbPtr->globalUsedListPtr = tempUsedListHeadPtr;
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChFlowManagerFlowIdGetNext function
* @endinternal
*
* @brief To get the flow id from the list of flow ids maintained in cpss db. The flow ids are maintained
*        in the list based on their insertion time order and the get request can be either from the
*        beginning of the list or can continue from where it last left.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngId             - unique flow manager id
* @param[in]   getFirst              - GT_TRUE - get first flow id
*                                      GT_FALSE - get subsequent flow ids
* @param [out] nextFlowIdPtr         - retrieve the existing flow id
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND              - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NO_MORE                 - no flow id exists in DB.
*/
GT_STATUS cpssDxChFlowManagerFlowIdGetNext
(
    IN  GT_U32  flowMngId,
    IN  GT_BOOL getFirst,
    OUT GT_U32  *nextFlowIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerFlowIdGetNext);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, getFirst, nextFlowIdPtr));

    rc = internal_cpssDxChFlowManagerFlowIdGetNext(flowMngId, getFirst, nextFlowIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, getFirst, nextFlowIdPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFlowManagerFlowDataGet function
* @endinternal
*
* @brief To get flow data for an existing flow in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngId             - unique flow manager id
* @param [in]  flowId                - flow id for which data is needed
* @param [out] flowEntryPtr          - ptr to flow entry structure
* @param [out] flowDataPtr           - ptr to flow data structure
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_NO_MORE                 - no flow id exists in the used linked list.
* @retval GT_NO_SUCH                 - on no such flow id exists.
*/
GT_STATUS internal_cpssDxChFlowManagerFlowDataGet
(
    IN  GT_U32                                  flowMngId,
    IN  GT_U32                                  flowId,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC   *flowEntryPtr,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_STC         *flowDataPtr
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngDbPtr;
    GT_U32                            iter;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *tempPtr;

    CPSS_NULL_PTR_CHECK_MAC(flowEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(flowDataPtr);
    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS ||
       flowId > PRV_CPSS_DXCH_FLOW_DB_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    tempPtr = flowMngDbPtr->cpssFlowUsedListHeader;

    if(tempPtr == NULL)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }
    while(tempPtr)
    {
        if(tempPtr->cpssFlowIndex == flowId)
        {
            flowEntryPtr->flowId = flowId;
            flowEntryPtr->byteCount = (flowMngDbPtr->flowCounterDb[flowId].byteCount) +
                                      (flowMngDbPtr->removedFlowCounterDb[flowId].byteCount);
            flowEntryPtr->packetCount = (flowMngDbPtr->flowCounterDb[flowId].packetCount) +
                                        (flowMngDbPtr->removedFlowCounterDb[flowId].packetCount);
            flowEntryPtr->dropCount = (flowMngDbPtr->flowCounterDb[flowId].dropCount) +
                                      (flowMngDbPtr->removedFlowCounterDb[flowId].dropCount);
            flowEntryPtr->firstTimeStamp = flowMngDbPtr->flowCounterDb[flowId].firstTimeStamp;
            flowEntryPtr->lastTimeStamp = flowMngDbPtr->flowCounterDb[flowId].lastTimeStamp;
            flowDataPtr->flowId = flowId;
            flowDataPtr->keySize = flowMngDbPtr->flowDataDb[flowId].keySize;
            flowDataPtr->keyId = flowMngDbPtr->flowDataDb[flowId].keyId;

            for(iter = 0; iter < flowMngDbPtr->cpssFwCfg.reducedRuleSize; iter++)
            {
                flowDataPtr->keyData[iter] = flowMngDbPtr->flowDataDb[flowId].keyData[iter];
            }
            return GT_OK;
        }
        tempPtr = tempPtr->next;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChFlowManagerFlowDataGet function
* @endinternal
*
* @brief To get flow data for an existing flow in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngId             - unique flow manager id
* @param [in]  flowId                - flow id for which data is needed
* @param [out] flowEntryPtr          - ptr to flow entry structure
* @param [out] flowDataPtr           - ptr to flow data structure
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_NO_SUCH                 - on no such flow id exists.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerFlowDataGet
(
    IN  GT_U32                                  flowMngId,
    IN  GT_U32                                  flowId,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC   *flowEntryPtr,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_STC         *flowDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerFlowDataGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, flowId, flowEntryPtr, flowDataPtr));

    rc = internal_cpssDxChFlowManagerFlowDataGet(flowMngId, flowId, flowEntryPtr, flowDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, flowId, flowEntryPtr, flowDataPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* debug function to allow print (dump) of the received frame after DSA tag is removed */
/* TBD lua wrapper or invoked in appDemo */
GT_U32 cpssDxChFlowManagerMessageParseDebugDumpEnable
(
    IN GT_BOOL  enableDump,
    IN GT_U32   flowMngId
)
{
    PRV_CPSS_DXCH_FLOW_MANAGER_STC   *flowMngDbPtr;
    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    if(flowMngDbPtr == NULL)
    {
        /* Flow Mng DB does not exist */
        cpssOsPrintf("Flow Manager DB does not exist \n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG); /* Illegal Pointer Value */
    }

    flowMngDbPtr->debug_dumpIsfPacket = enableDump;
    return flowMngDbPtr->debug_dumpIsfPacket;
}

/**
* @internal prvCpssDxChFlowManagerDbFlowMngGet function
* @endinternal
*
* @brief To get DB info about Flow Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - unique flow manager id.
*/
PRV_CPSS_DXCH_FLOW_MANAGER_STC* prvCpssDxChFlowManagerDbFlowMngGet
(
    IN GT_U32  flowMngId
)
{
    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        /* flowMngId out of range */
        return NULL;
    }

    return PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
}

/**
* @internal internal_cpssDxChFlowManagerMessagePacketStatisticsGet function
* @endinternal
*
* @brief   Debug api to get all the message packet related statistics.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - flow manager id.
* @param[out]messagePacketStatsPtr - (pointer to) message packet statistics structure
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS internal_cpssDxChFlowManagerMessagePacketStatisticsGet
(
    IN  GT_U32                                                flowMngId,
    OUT CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC  *messagePacketStatsPtr
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC   *flowMngDbPtr;

    CPSS_NULL_PTR_CHECK_MAC(messagePacketStatsPtr);

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        /* flowMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    if(flowMngDbPtr == NULL)
    {
        /* Flow Mng DB does not exist */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    messagePacketStatsPtr->messageNewFlowCount = flowMngDbPtr->messageNewFlowCount;
    messagePacketStatsPtr->messageRemovedFlowCount = flowMngDbPtr->messageRemovedFlowCount;
    messagePacketStatsPtr->messageFlowDataGetCount = flowMngDbPtr->messageFlowDataGetCount;
    messagePacketStatsPtr->reSyncCount = flowMngDbPtr->reSyncCount;
    messagePacketStatsPtr->sequenceNumber = flowMngDbPtr->seqNoCpss;
    messagePacketStatsPtr->messageNewFlowCountAfterResync = flowMngDbPtr->messageNewFlowCountAfterResync;
    messagePacketStatsPtr->messageRemovedFlowCountAfterResync = flowMngDbPtr->messageRemovedFlowCountAfterResync;
    messagePacketStatsPtr->messageSequenceNumberForResync = flowMngDbPtr->messageSequenceNumberForResync;
    messagePacketStatsPtr->dbAddedFlowCount = flowMngDbPtr->dbAddedFlowCount;
    messagePacketStatsPtr->dbRemovedFlowCount = flowMngDbPtr->dbRemovedFlowCount;

    if(flowMngDbPtr->debug_dumpIsfPacket)
    {
        cpssOsPrintf("Message Packet Sequence Number: %d\n",flowMngDbPtr->seqNoCpss);
        cpssOsPrintf("Total New Flow DB: %d, Total Removed Flow DB: %d\n",flowMngDbPtr->dbAddedFlowCount, flowMngDbPtr->dbRemovedFlowCount);
        cpssOsPrintf("Message Packet New: %d, RM: %d, Data: %d, ReSync: %d\n",
            flowMngDbPtr->messageNewFlowCount, flowMngDbPtr->messageRemovedFlowCount,
            flowMngDbPtr->messageFlowDataGetCount, flowMngDbPtr->reSyncCount);
        cpssOsPrintf("Message Packet After Re-Sync New: %d, RM: %d, Sequence Number: %d\n",
            flowMngDbPtr->messageNewFlowCountAfterResync, flowMngDbPtr->messageRemovedFlowCountAfterResync,
            flowMngDbPtr->messageSequenceNumberForResync);
    }
    return rc;
}

/**
* @internal cpssDxChFlowManagerMessagePacketStatisticsGet function
* @endinternal
*
* @brief   Debug api to get all the message packet related statistics.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - flow manager id.
* @param[out]messagePacketStatsPtr - (pointer to) message packet statistics structure
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerMessagePacketStatisticsGet
(
    IN  GT_U32                                                flowMngId,
    OUT CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC  *messagePacketStatsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerMessagePacketStatisticsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, flowMngId, messagePacketStatsPtr));

    rc = internal_cpssDxChFlowManagerMessagePacketStatisticsGet(flowMngId, messagePacketStatsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, flowMngId, messagePacketStatsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* debug function to get FW Pointers */
GT_STATUS cpssDxChFlowManagerFwPtrsGet
(
    IN  GT_U32                            flowMngId
)
{
  GT_STATUS                              rc = GT_OK;
  PRV_CPSS_DXCH_IPC_MSG_STC              ipc_msg_send;
  PRV_CPSS_DXCH_IPC_RECV_MSG_STC         ipc_msg_recv;
  PRV_CPSS_DXCH_FLOW_MANAGER_STC         *flowMngDbPtr;
  GT_U32                                 ii;
#ifndef ASIC_SIMULATION
  PRV_CPSS_DXCH_IPC_FW_PTRS_RECV_MSG_STC fwPtr;
  PRV_CPSS_DXCH_FW_PTRS_ADDRESS_STC      fwPtrsList;
  GT_UINTPTR                             ptr;
  GT_U32                                 index;
#endif

  if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
  {
      /* flowMngId out of range */
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
  }

  flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
  if(flowMngDbPtr == NULL)
  {
      /* Flow Mng DB does not exist */
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
  }
  cpssOsMemSet(&ipc_msg_send, 0, sizeof(PRV_CPSS_DXCH_IPC_MSG_STC));
  cpssOsMemSet(&ipc_msg_recv, 0, sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));
  for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
  {
      /* Send IPC message to FW to get flow data */
#ifndef ASIC_SIMULATION
      if(PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(flowMngDbPtr->devsBitmap, ii))
      {
          /* Send IPC message to FW passing the configuration parameters */
          ipc_msg_send.opcode = PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_GET_FW_PTRS_E;
          rc = prvCpssDxChFlowManagerSrvCpuIpcMessageSendRecv(ii, flowMngDbPtr, &ipc_msg_send, &ipc_msg_recv);
          if(rc != GT_OK)
          {
              return rc;
          }
          cpssOsMemCpy(&fwPtr, &ipc_msg_recv.data[1], sizeof(PRV_CPSS_DXCH_IPC_RECV_MSG_STC));
          /* FW SRAM in board can be accessed from CPSS with offset 0x80000 and
           * so taking care of the below.
           */
          index = 0;
          cpssOsMemCpy(&fwPtrsList.fwInitCfgPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwInitCfgPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwFlowTuplePtr, &(fwPtr.data[index]), sizeof(GT_U32));
          cpssOsMemCpy(&ptr, &(fwPtr.data[index]), sizeof(GT_U32));
          ptr = ptr & 0x0000FFFFFFFF;
          fwPtrsList.fwFlowTuplePtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwFlowCntrPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwFlowCntrPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwHashListPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwHashListPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwFreeListPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwFreeListPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwUsedListPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwUsedListPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwPclKeyCfgPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwPclKeyCfgPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwPortBindKeyCfgPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwPortBindKeyCfgPtr += 0x80000;
          index += sizeof(GT_U32);
          cpssOsMemCpy(&fwPtrsList.fwTcamToFwDbMappingPtr, &(fwPtr.data[index]), sizeof(GT_U32));
          fwPtrsList.fwTcamToFwDbMappingPtr += 0x80000;

          cpssOsPrintf("FW pointers list is\n"
              "FW Configuration Ptr: %x Flow Tuple Ptr %x Flow Counters Ptr %x\n"
              "Hash List Ptr: %x Free List Ptr %x Used List Ptr %x\n"
              "PCL Keys Ptr: %x Port Bind Keys Ptr: %x TCAM to FW DB Mapping Ptr: %x\n",
              fwPtrsList.fwInitCfgPtr, fwPtrsList.fwFlowTuplePtr, fwPtrsList.fwFlowCntrPtr,
              fwPtrsList.fwHashListPtr, fwPtrsList.fwFreeListPtr, fwPtrsList.fwUsedListPtr,
              fwPtrsList.fwPclKeyCfgPtr, fwPtrsList.fwPortBindKeyCfgPtr, fwPtrsList.fwTcamToFwDbMappingPtr);
      }
#else
    /* To pass compilation for WM ipc_msg_send needs to be used */
      if(ipc_msg_send.opcode == PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_GET_FW_PTRS_E)
      {
      }
#endif
  }
  return rc;
}

/**
* @internal internal_cpssDxChFlowManagerSrcPortFlowIdGetNext function
* @endinternal
*
* @brief To get flows id learnt for the given source port from the list of flow ids maintained in cpss db.
*        The flow ids are maintained in form of list in insertion time based order and the request can be
*        either from the beginning of the list or can continue from where it last left.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                  - unique device number
* @param[in] sourcePort              - source port number
* @param[in] getFirst                - GT_TRUE - get first flow id
*                                      GT_FALSE - get subsequent flow ids
* @param[in] flowIdPtr               - (pointer to) flow ids learnt per source port
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NO_MORE                 - on no more flow ids present.
*/
GT_STATUS internal_cpssDxChFlowManagerSrcPortFlowIdGetNext
(
    IN    GT_U8                                   devNum,
    IN    GT_PORT_NUM                             sourcePort,
    IN    GT_BOOL                                 getFirst,
    OUT   GT_U32                                  *flowIdPtr
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC              *pDev;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC           *flowMngDbPtr;
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC        *tempPtr;
    GT_U32                                   keyIdFlow;
    GT_U32                                   flowMngId;
    GT_STATUS                                rc;

    CPSS_NULL_PTR_CHECK_MAC(flowIdPtr);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, sourcePort);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    flowMngId = pDev->flowMgrInfo.flowManagerId;

    if(flowMngId > PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]) == NULL)
    {
        /* can't find the Flow Manager DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    flowMngDbPtr = PRV_SHARED_FLOW_MANAGER_DB_VAR_GET(flowMngDB[flowMngId]);
    if (getFirst == GT_TRUE)
    {
        tempPtr = flowMngDbPtr->cpssFlowUsedListHeader;
    }
    else
    {
        tempPtr = flowMngDbPtr->usedListPtr;
    }

    if (tempPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    while(tempPtr)
    {
        *flowIdPtr = tempPtr->cpssFlowIndex;
        /* Extract the keyId for new flow stored in cpss db */
        keyIdFlow = flowMngDbPtr->flowDataDb[*flowIdPtr].keyId;
        rc = prvCpssDxChFlowManagerSrcPortCheck(flowMngDbPtr, keyIdFlow, *flowIdPtr, sourcePort);
        if(rc == GT_OK)
        {
            /* Once the flow id is returned to application, move the traversal pointers to point to next node from where the
             * next traversal should begin.
             */
            tempPtr = tempPtr->next;
            flowMngDbPtr->usedListPtr = tempPtr;
            return GT_OK;
        }
        tempPtr = tempPtr->next;
        flowMngDbPtr->usedListPtr = tempPtr;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChFlowManagerSrcPortFlowIdGetNext function
* @endinternal
*
* @brief To get flows id learnt for the given source port from the list of flow ids maintained in cpss db.
*        The flow ids are maintained in form of list in insertion time based order and the request can be
*        either from the beginning of the list or can continue from where it last left.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                  - unique device number
* @param[in] sourcePort              - source port number
* @param[in] getFirst                - GT_TRUE - get first flow id
*                                      GT_FALSE - get subsequent flow ids
* @param[in] flowIdPtr               - (pointer to) flow ids learnt per source port
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NO_MORE                 - on no more flow ids present.
*/
GT_STATUS cpssDxChFlowManagerSrcPortFlowIdGetNext
(
    IN    GT_U8                                   devNum,
    IN    GT_PORT_NUM                             sourcePort,
    IN    GT_BOOL                                 getFirst,
    IN    GT_U32                                  *flowIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFlowManagerSrcPortFlowIdGetNext);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourcePort, getFirst, flowIdPtr));

    rc = internal_cpssDxChFlowManagerSrcPortFlowIdGetNext(devNum, sourcePort, getFirst, flowIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourcePort, getFirst, flowIdPtr));

    return rc;
}
