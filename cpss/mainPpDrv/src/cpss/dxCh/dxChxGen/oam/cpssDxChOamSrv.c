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
* @file cpssDxChOamSrv.c
*
* @brief CPSS DxCh OAM service CPU API.
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/oam/private/prvCpssDxChOamLog.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOamSrv.h>
#include <cpss/common/srvCpu/private/prvCpssGenSrvCpuIpc.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU Loopback configuration message.
*/
typedef struct{

    /** The flow ID. */
    GT_U32 flowId;

    /** @brief When enabled, the packet�s sequence number field is
     *  compared to the previous packet sequence number field.
     *  GT_FALSE - disable out of sequence checking.
     *  GT_TRUE - enable out of sequence checking.
     */
    GT_BOOL outOfSequenceEnable;

    /** @brief When enabled, CRC32 is computed on the packet's
     *  data TLV and compared to tlvDataCrc32Val.
     *  GT_FALSE - disable invalid data TLV checking.
     *  GT_TRUE - enable invalid data TLV checking.
     */
    GT_BOOL invalidTlvEnable;

    /** The expected TLV data CRC32 value. */
    GT_U32 tlvDataCrc32Val;

} PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC;

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_LB_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Loopback statistical counters message.
*/
typedef struct{

    /** The flow ID. */
    GT_U32 flowId;

    /** The number of LBR packets that are out of sequence. */
    GT_U32 outOfSequenceCount;

    /** @brief The total number of LBR packets that were checked
     *  for invalid TLV data CRC32.
     */
    GT_U32 totalCheckedDataTlv;

    /** @brief The total number of LBR packets that their data
     *  TLV CRC32 value was wrong.
     */
    GT_U32 totalInvalidDataTlv;

} PRV_CPSS_DXCH_OAM_SRV_MSG_LB_COUNTERS_STC;

/**
* @enum PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT
 *
 * @brief Enumeration of OAM Service CPU Tx message types.
*/
typedef enum{

    /** OAM Firmware initialization message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_FW_INIT_E,

    /** OAM flow initialization message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_FLOW_INIT_E,

    /** Loopback configuration set message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CONFIG_SET_E,

    /** Loopback configuration get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CONFIG_GET_E,

    /** Loopback counter get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CNTR_GET_E,

    /** Loopback counter clear message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CNTR_CLR_E,

    /** Local mac address table entry Set message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_MAC_ADDR_SET_E,

    /** Local mac address table entry Get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_MAC_ADDRS_GET_E,

    /** Local interface Set message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_INTERFACE_MAC_ADDR_SET_E,

    /** Local interface Get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_INTERFACE_MAC_ADDR_GET_E,

    /** RX flow entry parameters Set message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_RX_FLOW_ENTRY_SET_E,

    /** RX flow entry parameters Get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_RX_FLOW_ENTRY_GET_E,

    /** LM flow parameters Set message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CONFIG_SET_E,

    /** LM flow parameters Get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CONFIG_GET_E,

    /** Get LM counters message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CNTR_GET_E,

    /** LM clear counters message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CNTR_CLR_E,

    /** DM flow parameters Set message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CONFIG_SET_E,

    /** DM flow parameters Get message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CONFIG_GET_E,

    /** Get DM counters message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CNTR_GET_E,

    /** DM clear counters message. */
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CNTR_CLR_E

} PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT
 *
 * @brief Enumeration of OAM Service CPU Rx message types.
*/
typedef enum{

    /** Loopback configuration message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LB_CONFIG_E,

    /** Loopback counters data message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LB_COUNTERS_E,

    /** Local mac address table entry message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LOCAL_MAC_ADDR_E,

    /** Local interface message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LOCAL_INTERFACE_MAC_ADDR_E,

    /** RX flow entry parameters message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_RX_FLOW_ENTRY_E,

    /** LM flow parameters message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LM_CONFIG_E,

    /** LM counters message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LM_COUNTERS_E,

    /** DM flow parameters message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_DM_CONFIG_E,

    /** DM counters message. */
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_DM_COUNTERS_E

} PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT;

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU Delay measurement configuration message.
*/
typedef struct{

    /** OAM MEP flow ID [1 */
    GT_U32 flowId;

    /** enable DM counters in service */
    GT_BOOL countersEnable;

} PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC;


/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_DM_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Delay Measurement basic counters.
*/
typedef struct{

    /** @brief flowId
     *  Single-Ended Delay Measurement Counters (DMR):
     *  1. twFd-A 32-bit Two-Way Frame Delay Measurement
     *  2. owFdFw-A 32-bit One-Way Frame Delay Measurement in the forward direction
     *  3. owFdBw-A 32-bit One-Way Frame Delay Measurement in the backward direction
     */
    GT_U32 flowId;

    GT_U32 twFd;

    GT_U32 owFdFw;

    GT_U32 owFdBw;

} PRV_CPSS_DXCH_OAM_SRV_MSG_DM_COUNTERS_STC;

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC
 *
 * @brief Structure of OAM Service CPU Loss measurement configuration message.
*/
typedef struct{

    /** OAM MEP flow ID [1 */
    GT_U32 flowId;

    /** enable DM counters in service */
    GT_BOOL countersEnable;

} PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC;


/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_LM_COUNTERS_STC
 *
 * @brief Structure of OAM Service CPU Loss Measurement basic counters.
*/
typedef struct{

    /** flowId */
    GT_U32 flowId;

    /** tx frame count in the forward direction */
    GT_U32 txFCf;

    /** rx frame count in the forward direction */
    GT_U32 rxFCf;

    /** tx frame count in the backward direction */
    GT_U32 txFCb;

    /** rx frame count in the backward direction */
    GT_U32 rxFCl;

    /** |TxFCf[tc] � TxFCf[tp]| � |RxFCf[tc] � RxFCf[tp]| */
    GT_U32 farEndFrameLoss;

    /** |TxFCb[tc] � TxFCb[tp]| � |RxFCl[tc] � RxFCl[tp]| */
    GT_U32 nearEndFrameLoss;

} PRV_CPSS_DXCH_OAM_SRV_MSG_LM_COUNTERS_STC;

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC
 *
 * @brief Structure of OAM Service CPU local-mac-address set message.
*/
typedef struct{

    /** @brief index to local
     *  (APPLICABLE RANGES:0..127)
     */
    GT_U32 localMacAddrIndex;

    GT_U8 macAddr[6];

} PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC;

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC
 *
 * @brief Structure of OAM Service CPU local-interface set message.
*/
typedef struct{

    /** @brief if interfaceType==port index can be 0 to 27,
     *  if interfaceType==eport index can be for 0 to 8K
     */
    GT_U32 interfaceIndex;

    /** 0 */
    GT_U8 interfaceType;

    /** @brief index to local
     *  (APPLICABLE RANGES:0..127)
     */
    GT_U8 localMacAddIndx;

} PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC;

/**
* @struct PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC
 *
 * @brief Structure of OAM Service CPU rx flow params set message.
*/
typedef struct{

    /** flowId */
    GT_U32 flowId;

    /** frame type */
    GT_U8 frame_type;

    /** mp type */
    GT_U8 mpType;

    /** cfm offset */
    GT_U8 cfmOffset;

    /** @brief dsa valid
     *  eDsaTag[16] - dsatag
     */
    GT_U8 eDsaTagValid;

    GT_U8 eDsaTag[16];

} PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC;

/**
* @internal internal_cpssDxChOamSrvLoopbackConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loopback configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] lbConfigPtr              - (pointer to) struct of LB configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLoopbackConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_LB_CONFIG_STC *lbConfigPtr
)
{
    GT_U8   msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC)];/*message buffer*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;/*message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC msgData;/*Loopback configuration message data*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(lbConfigPtr);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CONFIG_SET_E);
    msgData.flowId              = CPSS_32BIT_LE(flowId);
    msgData.outOfSequenceEnable = CPSS_32BIT_LE(lbConfigPtr->outOfSequenceEnable);
    msgData.invalidTlvEnable    = CPSS_32BIT_LE(lbConfigPtr->invalidTlvEnable);
    msgData.tlvDataCrc32Val     = CPSS_32BIT_LE(lbConfigPtr->tlvDataCrc32Val);

    cpssOsMemCpy(msgBuf, &msgType, 4);
    cpssOsMemCpy(&(msgBuf[4]), &msgData, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC));

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvLoopbackConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loopback configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] lbConfigPtr              - (pointer to) struct of LB configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_LB_CONFIG_STC *lbConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLoopbackConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, lbConfigPtr));

    rc = internal_cpssDxChOamSrvLoopbackConfigSet(devNum, flowId, lbConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, lbConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLoopbackConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loopback configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @param[out] lbConfigPtr              - (pointer to) struct of LB configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLoopbackConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_LB_CONFIG_STC *lbConfigPtr
)
{
    GT_STATUS   rc;/*function return code*/
    GT_U8       msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC)];/*message buffer*/
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC msgLbConfig;/*Loopback configuration message data*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT   rxMsgType;/*rx message type*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(lbConfigPtr);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CONFIG_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;
    /* receive reply message from Service CPU */
    rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
    if(rc != GT_OK)
        return rc;
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LB_CONFIG_E)
        {
            cpssOsMemSet(&msgLbConfig, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC));
            cpssOsMemCpy(&msgLbConfig, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_CONFIG_STC));
            if(CPSS_32BIT_LE(msgLbConfig.flowId) == flowId)
            {
                lbConfigPtr->outOfSequenceEnable = CPSS_32BIT_LE(msgLbConfig.outOfSequenceEnable);
                lbConfigPtr->invalidTlvEnable    = CPSS_32BIT_LE(msgLbConfig.invalidTlvEnable);
                lbConfigPtr->tlvDataCrc32Val     = CPSS_32BIT_LE(msgLbConfig.tlvDataCrc32Val);
                rc = GT_OK;
            }
       }
    }
    return rc;
}

/**
* @internal cpssDxChOamSrvLoopbackConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loopback configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @param[out] lbConfigPtr              - (pointer to) struct of LB configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_LB_CONFIG_STC *lbConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLoopbackConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, lbConfigPtr));

    rc = internal_cpssDxChOamSrvLoopbackConfigGet(devNum, flowId, lbConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, lbConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLoopbackStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM service CPU LB statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] clearOnRead              - perform clear on read.
*                                      GT_TRUE: The flows Loopback statistical
*                                      counters will be cleared.
*                                      GT_FALSE: The flows Loopback statistical
*                                      counters will not be cleared.
* @param[in] timeout                  - time to wait for the service CPU to response
*                                      0 - no wait.
*                                      1 to 999 - defined time in miliseconds.
*                                      0xFFFFFFFF - wait forever.
*
* @param[out] lbCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLoopbackStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnRead,
    IN  GT_U32                              timeout,
    OUT CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC   *lbCountersPtr
)
{
    GT_STATUS   rc;/*function return code*/
    GT_U8       msgBuf[32];/*message buffer*/
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT       txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT       rxMsgType;/*rx message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LB_COUNTERS_STC   msgLbCounters;/*Loopback counters message data*/

            /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(lbCountersPtr);
    if(timeout >= 1000 && timeout != 0xFFFFFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CNTR_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;

    /* wait after sending request for the response to arrive */
    if(timeout > 0 && timeout != 0xFFFFFFFF)
        cpssOsDelay(timeout*1000000);

    if(timeout == 0xFFFFFFFF)
    {
        do
        {
            rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
            if(rc != GT_OK)
                return rc;
        } while(size > 0);
    }
    else
    {
        /* receive reply message from Service CPU */
        rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
        if(rc != GT_OK)
            return rc;
    }
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LB_COUNTERS_E)
        {
            cpssOsMemSet(&msgLbCounters, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_COUNTERS_STC));
            cpssOsMemCpy(&msgLbCounters, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LB_COUNTERS_STC));
            if(CPSS_32BIT_LE(msgLbCounters.flowId) == flowId)
            {
                lbCountersPtr->outOfSequenceCount = CPSS_32BIT_LE(msgLbCounters.outOfSequenceCount);
                lbCountersPtr->totalCheckedDataTlv = CPSS_32BIT_LE(msgLbCounters.totalCheckedDataTlv);
                lbCountersPtr->totalInvalidDataTlv = CPSS_32BIT_LE(msgLbCounters.totalInvalidDataTlv);
                rc = GT_OK;
            }
       }
    }
    if(rc != GT_OK)
        return rc;

    if(clearOnRead)
        rc =  cpssDxChOamSrvLoopbackStatisticalCountersClear(devNum,flowId);

    return rc;
}

/**
* @internal cpssDxChOamSrvLoopbackStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM service CPU LB statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] clearOnRead              - perform clear on read.
*                                      GT_TRUE: The flows Loopback statistical
*                                      counters will be cleared.
*                                      GT_FALSE: The flows Loopback statistical
*                                      counters will not be cleared.
* @param[in] timeout                  - time to wait for the service CPU to response
*                                      0 - no wait.
*                                      1 to 999 - defined time in miliseconds.
*                                      0xFFFFFFFF - wait forever.
*
* @param[out] lbCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnRead,
    IN  GT_U32                              timeout,
    OUT CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC   *lbCountersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLoopbackStatisticalCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, clearOnRead, timeout, lbCountersPtr));

    rc = internal_cpssDxChOamSrvLoopbackStatisticalCountersGet(devNum, flowId, clearOnRead, timeout, lbCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, clearOnRead, timeout, lbCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLoopbackStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the LB statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLoopbackStatisticalCountersClear
(
    IN  GT_U8   devNum,
    IN  GT_U32  flowId
)
{
    GT_U8   msgBuf [8];/*message buffer*/
    GT_U32  swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;   /*message type*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LB_CNTR_CLR_E);
    cpssOsMemCpy(msgBuf, &msgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvLoopbackStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the LB statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLoopbackStatisticalCountersClear
(
    IN  GT_U8   devNum,
    IN  GT_U32  flowId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLoopbackStatisticalCountersClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId));

    rc = internal_cpssDxChOamSrvLoopbackStatisticalCountersClear(devNum, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvDmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Delay Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] dmConfigPtr              - (pointer to) struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvDmConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_DM_CONFIG_STC *dmConfigPtr
)
{
    GT_U8   msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC)];/*message buffer*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;/*message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC msgData;/*Loopback configuration message data*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(dmConfigPtr);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CONFIG_SET_E);
    msgData.flowId              = CPSS_32BIT_LE(flowId);
    msgData.countersEnable      = CPSS_32BIT_LE(dmConfigPtr->countersEnable);

    cpssOsMemCpy(msgBuf, &msgType, 4);
    cpssOsMemCpy(&(msgBuf[4]), &msgData, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC));

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvDmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Delay Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] dmConfigPtr              - (pointer to) struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_DM_CONFIG_STC *dmConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvDmConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, dmConfigPtr));

    rc = internal_cpssDxChOamSrvDmConfigSet(devNum, flowId, dmConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, dmConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvDmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Delay Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048).
*
* @param[out] dmConfigPtr              - pointer to struct of DM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvDmConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_DM_CONFIG_STC *dmConfigPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC)];
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC msgDmConfig;
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   txMsgType;
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT   rxMsgType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(dmConfigPtr);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CONFIG_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;
    /* receive reply message from Service CPU */
    rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
    if(rc != GT_OK)
        return rc;
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_DM_CONFIG_E)
        {
            cpssOsMemSet(&msgDmConfig, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC));
            cpssOsMemCpy(&msgDmConfig, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_CONFIG_STC));
            if(CPSS_32BIT_LE(msgDmConfig.flowId) == flowId)
            {
                dmConfigPtr->countersEnable = CPSS_32BIT_LE(msgDmConfig.countersEnable);
                rc = GT_OK;
            }
       }
    }
    return rc;
}

/**
* @internal cpssDxChOamSrvDmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Delay Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID.
*                                      (APPLICABLE RANGES: 1..2048).
*
* @param[out] dmConfigPtr              - pointer to struct of DM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_DM_CONFIG_STC *dmConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvDmConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, dmConfigPtr));

    rc = internal_cpssDxChOamSrvDmConfigGet(devNum, flowId, dmConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, dmConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvDmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the Delay Measurement statistical counters per flow ID from service CPU .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] clearOnRead              - GT_TRUE - clear the counter.
*                                      GT_FALSE - don't clear the counter.
*
* @param[out] dmCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvDmStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnRead,
    OUT CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC   *dmCountersPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf[32];
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT       txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT       rxMsgType;/*rx message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_DM_COUNTERS_STC   msgDmCounters;
    GT_U32                                      timeout = 0;

            /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(dmCountersPtr);
    if(timeout >= 1000 && timeout != 0xFFFFFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CNTR_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;

    /* wait after sending request for the response to arrive */
    if(timeout > 0 && timeout != 0xFFFFFFFF)
        cpssOsDelay(timeout*1000000);

    if(timeout == 0xFFFFFFFF)
    {
        do
        {
            rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
            if(rc != GT_OK)
                return rc;
        } while(size > 0);
    }
    else
    {
        /* receive reply message from Service CPU */
        rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
        if(rc != GT_OK)
            return rc;
    }
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_DM_COUNTERS_E)
        {
            cpssOsMemSet(&msgDmCounters, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_COUNTERS_STC));
            cpssOsMemCpy(&msgDmCounters, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_DM_COUNTERS_STC));
            if(CPSS_32BIT_LE(msgDmCounters.flowId) == flowId)
            {
                dmCountersPtr->basicCounters.twoWayFrameDelay = CPSS_32BIT_LE(msgDmCounters.twFd);
                dmCountersPtr->basicCounters.oneWayFrameDelayForward = CPSS_32BIT_LE(msgDmCounters.owFdFw);
                dmCountersPtr->basicCounters.oneWayFrameDelayBackward = CPSS_32BIT_LE(msgDmCounters.owFdBw);
                rc = GT_OK;
            }
       }
    }
    if(rc != GT_OK)
        return rc;

    if(clearOnRead)
        rc =  cpssDxChOamSrvDmStatisticalCountersClear(devNum, flowId);

    return rc;
}

/**
* @internal cpssDxChOamSrvDmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the Delay Measurement statistical counters per flow ID from service CPU .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] clearOnRead              - GT_TRUE - clear the counter.
*                                      GT_FALSE - don't clear the counter.
*
* @param[out] dmCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnRead,
    OUT CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC   *dmCountersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvDmStatisticalCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, clearOnRead, dmCountersPtr));

    rc = internal_cpssDxChOamSrvDmStatisticalCountersGet(devNum, flowId, clearOnRead, dmCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, clearOnRead, dmCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvDmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Delay Measurement statistical counters per flow ID in service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvDmStatisticalCountersClear
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      flowId
)
{
    GT_U8   msgBuf [8];
    GT_U32  swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_DM_CNTR_CLR_E);
    cpssOsMemCpy(msgBuf, &msgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvDmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Delay Measurement statistical counters per flow ID in service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvDmStatisticalCountersClear
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      flowId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvDmStatisticalCountersClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId));

    rc = internal_cpssDxChOamSrvDmStatisticalCountersClear(devNum, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loss Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] lmConfigPtr              - (pointer to) struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLmConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_LM_CONFIG_STC *lmConfigPtr
)
{
    GT_U8   msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC)];/*message buffer*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;/*message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC msgData;/*Loopback configuration message data*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(lmConfigPtr);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CONFIG_SET_E);
    msgData.flowId              = CPSS_32BIT_LE(flowId);
    msgData.countersEnable      = CPSS_32BIT_LE(lmConfigPtr->countersEnable);

    cpssOsMemCpy(msgBuf, &msgType, 4);
    cpssOsMemCpy(&(msgBuf[4]), &msgData, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC));

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvLmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loss Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] lmConfigPtr              - (pointer to) struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    IN  CPSS_DXCH_OAM_SRV_LM_CONFIG_STC *lmConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLmConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, lmConfigPtr));

    rc = internal_cpssDxChOamSrvLmConfigSet(devNum, flowId, lmConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, lmConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loss Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @param[out] lmConfigPtr              - pointer to struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLmConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_LM_CONFIG_STC *lmConfigPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC)];
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC msgLmConfig;
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   txMsgType;
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT   rxMsgType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(lmConfigPtr);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CONFIG_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;
    /* receive reply message from Service CPU */
    rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
    if(rc != GT_OK)
        return rc;
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LM_CONFIG_E)
        {
            cpssOsMemSet(&msgLmConfig, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC));
            cpssOsMemCpy(&msgLmConfig, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_CONFIG_STC));
            if(CPSS_32BIT_LE(msgLmConfig.flowId) == flowId)
            {
                lmConfigPtr->countersEnable = CPSS_32BIT_LE(msgLmConfig.countersEnable);
                rc = GT_OK;
            }
       }
    }
    return rc;
}

/**
* @internal cpssDxChOamSrvLmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loss Measurement configuration per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @param[out] lmConfigPtr              - pointer to struct of LM configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          flowId,
    OUT CPSS_DXCH_OAM_SRV_LM_CONFIG_STC *lmConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLmConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, lmConfigPtr));

    rc = internal_cpssDxChOamSrvLmConfigGet(devNum, flowId, lmConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, lmConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM service CPU Loss Measurement statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] clearOnReadEnable        - GT_TRUE - clear the counter on read.
*                                      GT_FALSE - don't clear the counter.
*
* @param[out] lmCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLmStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnReadEnable,
    OUT CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC   *lmCountersPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf[32];
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT       txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT       rxMsgType;/*rx message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LM_COUNTERS_STC   msgLmCounters;
    GT_U32                                      timeout = 0;

            /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(lmCountersPtr);
    if(timeout >= 1000 && timeout != 0xFFFFFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CNTR_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;

    /* wait after sending request for the response to arrive */
    if(timeout > 0 && timeout != 0xFFFFFFFF)
        cpssOsDelay(timeout*1000000);

    if(timeout == 0xFFFFFFFF)
    {
        do
        {
            rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
            if(rc != GT_OK)
                return rc;
        } while(size > 0);
    }
    else
    {
        /* receive reply message from Service CPU */
        rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
        if(rc != GT_OK)
            return rc;
    }
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LM_COUNTERS_E)
        {
            cpssOsMemSet(&msgLmCounters, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_COUNTERS_STC));
            cpssOsMemCpy(&msgLmCounters, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LM_COUNTERS_STC));
            if(CPSS_32BIT_LE(msgLmCounters.flowId) == flowId)
            {
                lmCountersPtr->basicCounters.txFrameCntForward = CPSS_32BIT_LE(msgLmCounters.txFCf);
                lmCountersPtr->basicCounters.rxFrameCntForward = CPSS_32BIT_LE(msgLmCounters.rxFCf);
                lmCountersPtr->basicCounters.txFrameCntBackward = CPSS_32BIT_LE(msgLmCounters.txFCb);
                lmCountersPtr->basicCounters.rxFrameCntBackward = CPSS_32BIT_LE(msgLmCounters.rxFCl);
                lmCountersPtr->basicCounters.farEndFrameLoss = CPSS_32BIT_LE(msgLmCounters.farEndFrameLoss);
                lmCountersPtr->basicCounters.nearEndFrameLoss = CPSS_32BIT_LE(msgLmCounters.nearEndFrameLoss);
                rc = GT_OK;
            }
       }
    }
    if(rc != GT_OK)
        return rc;

    if(clearOnReadEnable)
        rc =  cpssDxChOamSrvDmStatisticalCountersClear(devNum, flowId);

    return rc;
}

/**
* @internal cpssDxChOamSrvLmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM service CPU Loss Measurement statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
* @param[in] clearOnReadEnable        - GT_TRUE - clear the counter on read.
*                                      GT_FALSE - don't clear the counter.
*
* @param[out] lmCountersPtr            - (pointer to) struct of current counter values.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmStatisticalCountersGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowId,
    IN  GT_BOOL                             clearOnReadEnable,
    OUT CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC   *lmCountersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLmStatisticalCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, clearOnReadEnable, lmCountersPtr));

    rc = internal_cpssDxChOamSrvLmStatisticalCountersGet(devNum, flowId, clearOnReadEnable, lmCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, clearOnReadEnable, lmCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Loss Measurement statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLmStatisticalCountersClear
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      flowId
)
{
    GT_U8   msgBuf [8];
    GT_U32  swappedFlowId;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LM_CNTR_CLR_E);
    cpssOsMemCpy(msgBuf, &msgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvLmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Loss Measurement statistical counters per flow ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - OAM MEP flow ID
*                                      (APPLICABLE RANGES: 1..2048).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLmStatisticalCountersClear
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      flowId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLmStatisticalCountersClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId));

    rc = internal_cpssDxChOamSrvLmStatisticalCountersClear(devNum, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLocalMacAddressSet function
* @endinternal
*
* @brief   Globally configure the MAC address table in the service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Local MAC Address Entry.
*                                      This index will be used in cpssDxChOamSrvLocalInterfaceMacIndexSet.
*                                      (APPLICABLE RANGES:0..127)
* @param[in] macAddrPtr               - pointer to mac address.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLocalMacAddressSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_ETHERADDR                   *macAddrPtr
)
{
    GT_U8   msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC)];/*message buffer*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;/*message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC msgData;/*Loopback configuration message data*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(index > CPSS_DXCH_OAM_SRV_MAX_LOCAL_MAC_ADDR_INDEX_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_MAC_ADDR_SET_E);
    msgData.localMacAddrIndex = CPSS_32BIT_LE(index);
    cpssOsMemCpy(msgData.macAddr, macAddrPtr->arEther, 6*sizeof(GT_U8));

    cpssOsMemCpy(msgBuf, &msgType, 4);
    cpssOsMemCpy(&(msgBuf[4]), &msgData, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC));

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvLocalMacAddressSet function
* @endinternal
*
* @brief   Globally configure the MAC address table in the service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Local MAC Address Entry.
*                                      This index will be used in cpssDxChOamSrvLocalInterfaceMacIndexSet.
*                                      (APPLICABLE RANGES:0..127)
* @param[in] macAddrPtr               - pointer to mac address.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalMacAddressSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_ETHERADDR                   *macAddrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLocalMacAddressSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, macAddrPtr));

    rc = internal_cpssDxChOamSrvLocalMacAddressSet(devNum, index, macAddrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, macAddrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLocalMacAddressGet function
* @endinternal
*
* @brief   Get the MAC address table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Local MAC Address Entry
*                                      (APPLICABLE RANGES:0..127)
*
* @param[out] macAddrPtr               - pointer to mac address.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLocalMacAddressGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT GT_ETHERADDR                    *macAddrPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf[32];
    GT_U32      size;/*received message size*/
    GT_U32      swappedIndex;/*swapped flowId in case of BE mode*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT       txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT       rxMsgType;/*rx message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC     msgLocalMac;
    GT_U32                                      timeout = 0;

            /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(index > CPSS_DXCH_OAM_SRV_MAX_LOCAL_MAC_ADDR_INDEX_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if(timeout >= 1000 && timeout != 0xFFFFFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_MAC_ADDRS_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedIndex = CPSS_32BIT_LE(index);
    cpssOsMemCpy(&(msgBuf[4]), &swappedIndex, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;

    /* wait after sending request for the response to arrive */
    if(timeout > 0 && timeout != 0xFFFFFFFF)
        cpssOsDelay(timeout*1000000);

    if(timeout == 0xFFFFFFFF)
    {
        do
        {
            rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
            if(rc != GT_OK)
                return rc;
        } while(size > 0);
    }
    else
    {
        /* receive reply message from Service CPU */
        rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
        if(rc != GT_OK)
            return rc;
    }
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LOCAL_MAC_ADDR_E)
        {
            cpssOsMemSet(&msgLocalMac, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC));
            cpssOsMemCpy(&msgLocalMac, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_MAC_STC));
            if(CPSS_32BIT_LE(msgLocalMac.localMacAddrIndex) == index)
            {
                cpssOsMemCpy(macAddrPtr->arEther, msgLocalMac.macAddr, 6*sizeof(GT_U8));
                rc = GT_OK;
            }
       }
    }
    return rc;
}

/**
* @internal cpssDxChOamSrvLocalMacAddressGet function
* @endinternal
*
* @brief   Get the MAC address table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Local MAC Address Entry
*                                      (APPLICABLE RANGES:0..127)
*
* @param[out] macAddrPtr               - pointer to mac address.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalMacAddressGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT GT_ETHERADDR                    *macAddrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLocalMacAddressGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, macAddrPtr));

    rc = internal_cpssDxChOamSrvLocalMacAddressGet(devNum, index, macAddrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, macAddrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLocalInterfaceMacIndexSet function
* @endinternal
*
* @brief   Globally configure the MAC address index of a local interface.
*         If OAM packet that come to service CPU is multicast packet, and this packet
*         need to be send back from the CPU - the source mac for this packet will be
*         taken from this table according to source interface that packet comes from.
*         the destination mac will be the incoming source mac.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port or E-port number.
* @param[in] isPhysicalPort           - defines type of port parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - port is physical port
*                                      GT_FALSE - port is ePort
* @param[in] macAddIndex              - mac address index.
*                                      (the mac address index that configured in cpssDxChOamSrvLocalMacAddressSet)
*                                      (APPLICABLE RANGES:0..127)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLocalInterfaceMacIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             isPhysicalPort,
    IN  GT_U32                              macAddIndex
)
{
    GT_U8   msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC)];/*message buffer*/
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;/*message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC msgData;/*Loopback configuration message data*/


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(macAddIndex > CPSS_DXCH_OAM_SRV_MAX_LOCAL_MAC_ADDR_INDEX_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if(portNum > 27)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_INTERFACE_MAC_ADDR_SET_E);
    msgData.interfaceType = (isPhysicalPort==GT_TRUE)?0:1;
    msgData.interfaceIndex = CPSS_32BIT_LE(portNum);
    msgData.localMacAddIndx = (GT_U8)macAddIndex;

    cpssOsMemCpy(msgBuf, &msgType, 4);
    cpssOsMemCpy(&(msgBuf[4]), &msgData, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC));

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvLocalInterfaceMacIndexSet function
* @endinternal
*
* @brief   Globally configure the MAC address index of a local interface.
*         If OAM packet that come to service CPU is multicast packet, and this packet
*         need to be send back from the CPU - the source mac for this packet will be
*         taken from this table according to source interface that packet comes from.
*         the destination mac will be the incoming source mac.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port or E-port number.
* @param[in] isPhysicalPort           - defines type of port parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - port is physical port
*                                      GT_FALSE - port is ePort
* @param[in] macAddIndex              - mac address index.
*                                      (the mac address index that configured in cpssDxChOamSrvLocalMacAddressSet)
*                                      (APPLICABLE RANGES:0..127)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalInterfaceMacIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             isPhysicalPort,
    IN  GT_U32                              macAddIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLocalInterfaceMacIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isPhysicalPort, macAddIndex));

    rc = internal_cpssDxChOamSrvLocalInterfaceMacIndexSet(devNum, portNum, isPhysicalPort, macAddIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isPhysicalPort, macAddIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvLocalInterfaceMacIndexGet function
* @endinternal
*
* @brief   Get the MAC address index of a local interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port or E-port number.
* @param[in] isPhysicalPort           - defines type of port parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - port is physical port
*                                      GT_FALSE - port is ePort
*
* @param[out] macAddIndexPtr           - mac address index.
*                                      (the mac address index that configured in cpssDxChOamSrvLocalMacAddressSet).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvLocalInterfaceMacIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             isPhysicalPort,
    OUT GT_U32                              *macAddIndexPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf[32];
    GT_U32      size;/*received message size*/
    GT_U32      swappedPort;
    GT_U8       interfaceType;
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT         txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT         rxMsgType;/*rx message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC msgLocalMac;
    GT_U32                                        timeout = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(portNum > 27)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(macAddIndexPtr);
    if(timeout >= 1000 && timeout != 0xFFFFFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_LOCAL_INTERFACE_MAC_ADDR_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedPort = CPSS_32BIT_LE(portNum);
    interfaceType = (isPhysicalPort==GT_TRUE)?0:1;
    cpssOsMemCpy(&(msgBuf[4]), &swappedPort, 4);
    cpssOsMemCpy(&(msgBuf[8]), &interfaceType, 1);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 9);
    if(rc != GT_OK)
        return rc;

    /* wait after sending request for the response to arrive */
    if(timeout > 0 && timeout != 0xFFFFFFFF)
        cpssOsDelay(timeout*1000000);

    if(timeout == 0xFFFFFFFF)
    {
        do
        {
            rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
            if(rc != GT_OK)
                return rc;
        } while(size > 0);
    }
    else
    {
        /* receive reply message from Service CPU */
        rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
        if(rc != GT_OK)
            return rc;
    }
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_LOCAL_INTERFACE_MAC_ADDR_E)
        {
            cpssOsMemSet(&msgLocalMac, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC));
            cpssOsMemCpy(&msgLocalMac, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_LOCAL_INTERFACE_STC));
            if((CPSS_32BIT_LE(msgLocalMac.interfaceIndex) == portNum) &&
               (CPSS_32BIT_LE(msgLocalMac.interfaceType) == interfaceType))
            {
                *macAddIndexPtr = msgLocalMac.localMacAddIndx;
                rc = GT_OK;
            }
       }
    }
    return rc;
}

/**
* @internal cpssDxChOamSrvLocalInterfaceMacIndexGet function
* @endinternal
*
* @brief   Get the MAC address index of a local interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port or E-port number.
* @param[in] isPhysicalPort           - defines type of port parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - port is physical port
*                                      GT_FALSE - port is ePort
*
* @param[out] macAddIndexPtr           - mac address index.
*                                      (the mac address index that configured in cpssDxChOamSrvLocalMacAddressSet).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvLocalInterfaceMacIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             isPhysicalPort,
    OUT GT_U32                              *macAddIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvLocalInterfaceMacIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isPhysicalPort, macAddIndexPtr));

    rc = internal_cpssDxChOamSrvLocalInterfaceMacIndexGet(devNum, portNum, isPhysicalPort, macAddIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isPhysicalPort, macAddIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvRxFlowEntrySet function
* @endinternal
*
* @brief   Add new Rx OAM entry to OAM service CPU database
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - Flow ID mapped to the peer MEP/MIP.
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] cookiePtr                - for application use
* @param[in] frameParamsPtr           - (pointer to) parameters per frame type to define how the packet will be sent
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvRxFlowEntrySet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      flowId,
    IN  GT_PTR                                      cookiePtr,
    IN  CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC  *frameParamsPtr
)
{
    GT_STATUS rc;
    GT_U8   msgBuf [4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC)];
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT           msgType;
    PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC    msgData;
    GT_U8                                           dsaBytes[16];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(cookiePtr != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_RX_FLOW_ENTRY_SET_E);

    cpssOsMemSet(&msgData, 0, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC));
    msgData.flowId = CPSS_32BIT_LE(flowId);
    if(frameParamsPtr->frameType == CPSS_DXCH_OAM_SRV_FRAME_TYPE_1731_E)
    {
        msgData.frame_type = 0;
        msgData.mpType = (GT_U8)frameParamsPtr->frame.frame1731.mpType;
        msgData.cfmOffset = (GT_U8)frameParamsPtr->frame.frame1731.cfmOffset;
        if(frameParamsPtr->frame.frame1731.transmitParams.l2HeaderValid)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); /* currently still not supported */
        if(frameParamsPtr->frame.frame1731.transmitParams.dsaParamValid)
        {
            CPSS_DXCH_NET_DSA_PARAMS_STC   dsaInfo;
            cpssOsMemSet(&dsaInfo, '\0',sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));
            cpssOsMemCpy(&dsaInfo, &frameParamsPtr->frame.frame1731.transmitParams.dsaParam, sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));

            msgData.eDsaTagValid = 1;

            cpssOsMemSet(dsaBytes, '\0', 16*sizeof(GT_U8));
            rc = cpssDxChNetIfDsaTagBuild(devNum, &dsaInfo, dsaBytes);
            if(rc != GT_OK)
                return rc;

            cpssOsMemCpy(msgData.eDsaTag, dsaBytes, 16*sizeof(GT_U8));
        }
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    cpssOsMemCpy(msgBuf, &msgType, 4);
    cpssOsMemCpy(&(msgBuf[4]), &msgData, sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC));

    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, sizeof(msgBuf));
}

/**
* @internal cpssDxChOamSrvRxFlowEntrySet function
* @endinternal
*
* @brief   Add new Rx OAM entry to OAM service CPU database
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - Flow ID mapped to the peer MEP/MIP.
*                                      (APPLICABLE RANGES: 1..2048)
* @param[in] cookiePtr                - for application use
* @param[in] frameParamsPtr           - (pointer to) parameters per frame type to define how the packet will be sent
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvRxFlowEntrySet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      flowId,
    IN  GT_PTR                                      cookiePtr,
    IN  CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC  *frameParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvRxFlowEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, cookiePtr, frameParamsPtr));

    rc = internal_cpssDxChOamSrvRxFlowEntrySet(devNum, flowId, cookiePtr, frameParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, cookiePtr, frameParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvRxFlowEntryGet function
* @endinternal
*
* @brief   Add new Rx OAM entry to OAM service CPU database
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - Flow ID mapped to the peer MEP/MIP.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @param[out] cookiePtr                - for application use
* @param[out] frameParamsPtr           - parameters per frame type to define how the packet will be sent
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChOamSrvRxFlowEntryGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      flowId,
    OUT GT_PTR                                      *cookiePtr,
    OUT CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC  *frameParamsPtr
)
{
    GT_STATUS   rc;
    GT_U8       msgBuf[4 + sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC)];
    GT_U32      size;/*received message size*/
    GT_U32      swappedFlowId;
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT         txMsgType;/*tx message type*/
    PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_ENT         rxMsgType;/*rx message type*/
    PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC  msgRxParams;
    GT_U32                                        timeout = 0;
    GT_U8                                         dsaBytes[16];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    if(flowId > CPSS_DXCH_OAM_SRV_MAX_FLOW_ID_CNS || flowId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(frameParamsPtr);
    if(timeout >= 1000 && timeout != 0xFFFFFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(cookiePtr != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    txMsgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_RX_FLOW_ENTRY_GET_E);
    cpssOsMemCpy(msgBuf, &txMsgType, 4);
    swappedFlowId = CPSS_32BIT_LE(flowId);
    cpssOsMemCpy(&(msgBuf[4]), &swappedFlowId, 4);
    /* send request message to service CPU */
    rc = prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
    if(rc != GT_OK)
        return rc;

    /* wait after sending request for the response to arrive */
    if(timeout > 0 && timeout != 0xFFFFFFFF)
        cpssOsDelay(timeout*1000000);

    if(timeout == 0xFFFFFFFF)
    {
        do
        {
            rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
            if(rc != GT_OK)
                return rc;
        } while(size > 0);
    }
    else
    {
        /* receive reply message from Service CPU */
        rc = prvCpssGenSrvCpuIpcMessageGet(devNum, msgBuf, &size);
        if(rc != GT_OK)
            return rc;
    }
    rc = GT_FAIL;
    if(size > 0)
    {
        cpssOsMemCpy(&rxMsgType, msgBuf, 4);
        if(CPSS_32BIT_LE(rxMsgType) == PRV_CPSS_DXCH_OAM_SRV_RX_MSG_TYPE_RX_FLOW_ENTRY_E)
        {
            cpssOsMemSet(&msgRxParams, '\0', sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC));
            cpssOsMemCpy(&msgRxParams, &(msgBuf[4]), sizeof(PRV_CPSS_DXCH_OAM_SRV_MSG_RX_FLOW_PARAMS_STC));
            if(CPSS_32BIT_LE(msgRxParams.flowId) == flowId)
            {
                frameParamsPtr->frameType = CPSS_DXCH_OAM_SRV_FRAME_TYPE_1731_E;
                frameParamsPtr->frame.frame1731.mpType = msgRxParams.mpType;
                frameParamsPtr->frame.frame1731.cfmOffset = msgRxParams.cfmOffset;
                if(msgRxParams.eDsaTagValid)
                {
                    frameParamsPtr->frame.frame1731.transmitParams.dsaParamValid = GT_TRUE;

                    cpssOsMemCpy(dsaBytes, msgRxParams.eDsaTag, 16*sizeof(GT_U8));
                    rc = cpssDxChNetIfDsaTagParse(devNum, dsaBytes, &frameParamsPtr->frame.frame1731.transmitParams.dsaParam);
                    if(rc != GT_OK)
                        return rc;
                }
                rc = GT_OK;
            }
       }
    }
    return rc;
}

/**
* @internal cpssDxChOamSrvRxFlowEntryGet function
* @endinternal
*
* @brief   Add new Rx OAM entry to OAM service CPU database
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] flowId                   - Flow ID mapped to the peer MEP/MIP.
*                                      (APPLICABLE RANGES: 1..2048)
*
* @param[out] cookiePtr                - for application use
* @param[out] frameParamsPtr           - parameters per frame type to define how the packet will be sent
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChOamSrvRxFlowEntryGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      flowId,
    OUT GT_PTR                                      *cookiePtr,
    OUT CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC  *frameParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvRxFlowEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, flowId, cookiePtr, frameParamsPtr));

    rc = internal_cpssDxChOamSrvRxFlowEntryGet(devNum, flowId, cookiePtr, frameParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, flowId, cookiePtr, frameParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChOamSrvSystemInit function
* @endinternal
*
* @brief   Initialize the service CPU OAM Firmware.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] sysParamsPtr             - (pointer to) struct of system paramaters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Must be called after cpssDxChNetIfInit Is called.
*
*/
static GT_STATUS internal_cpssDxChOamSrvSystemInit
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC   *sysParamsPtr
)
{
    PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_ENT   msgType;/*message type*/
    GT_U8   msgBuf[4 + sizeof(CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC)];/*message buffer*/
    GT_U32  i;/*for loop index*/

        /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(sysParamsPtr);

    msgType = CPSS_32BIT_LE(PRV_CPSS_DXCH_OAM_SRV_TX_MSG_TYPE_FW_INIT_E);
    cpssOsMemCpy(msgBuf, &msgType, 4);

    /* swap values in case of BE mode */
    for(i = 0; i < CPSS_DXCH_OAM_RX_SDMA_QUEUE_NUM_CNS; i++)
        sysParamsPtr->rxQueueNum[i] = CPSS_32BIT_LE(sysParamsPtr->rxQueueNum[i]);
    for(i = 0; i < CPSS_DXCH_OAM_TX_SDMA_QUEUE_NUM_CNS; i++)
        sysParamsPtr->txQueueNum[i] = CPSS_32BIT_LE(sysParamsPtr->txQueueNum[i]);

    sysParamsPtr->downMepsNum    = CPSS_32BIT_LE(sysParamsPtr->downMepsNum);
    sysParamsPtr->upMepsNum      = CPSS_32BIT_LE(sysParamsPtr->upMepsNum);
    sysParamsPtr->rxCcmFlowsNum  = CPSS_32BIT_LE(sysParamsPtr->rxCcmFlowsNum);
    cpssOsMemCpy(msgBuf + 4, sysParamsPtr, sizeof(CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC));
    /* Send to the service cpu a request for the address of the FW flow DB*/
    return prvCpssGenSrvCpuIpcMessageSend(devNum, msgBuf, 8);
}

/**
* @internal cpssDxChOamSrvSystemInit function
* @endinternal
*
* @brief   Initialize the service CPU OAM Firmware.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] sysParamsPtr             - (pointer to) struct of system paramaters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Must be called after cpssDxChNetIfInit Is called.
*
*/
GT_STATUS cpssDxChOamSrvSystemInit
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC   *sysParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChOamSrvSystemInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sysParamsPtr));

    rc = internal_cpssDxChOamSrvSystemInit(devNum, sysParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sysParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


