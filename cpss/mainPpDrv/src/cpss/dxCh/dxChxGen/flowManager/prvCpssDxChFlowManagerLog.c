/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChFlowManagerLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/flowManager/private/prvCpssDxChFlowManagerLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT[]  =
{
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E",
    "CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_60B_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, byteCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstTimeStamp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastTimeStamp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_FLOW_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FLOW_MANAGER_FLOW_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, keySize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, keyId);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, keyData, CPSS_FLOW_MANAGER_MAX_KEY_LEN_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageNewFlowCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageRemovedFlowCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageFlowDataGetCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reSyncCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sequenceNumber);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageNewFlowCountAfterResync);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageRemovedFlowCountAfterResync);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageSequenceNumberForResync);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dbAddedFlowCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dbRemovedFlowCount);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv4TcpFlowsLearnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv4UdpFlowsLearnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv6TcpFlowsLearnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv6UdpFlowsLearnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, otherTypeFlowsLearnt);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FLOW_MANAGER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, cpssFwCfg, CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowDbSize);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, appCbFlowNotificaion);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, appCbOldFlowRemovalNotificaion);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxSdmaQ);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txSdmaQ);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, messagePacketCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, cpuCodeStart, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcamStartIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipfixStartIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reducedRuleSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcamNumOfShortRules);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, tcamShortRuleSize, CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowDBSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowShortDBSize);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_FLOW_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FLOW_MANAGER_FLOW_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_FLOW_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *paramVal, CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_FLOW_MANAGER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FLOW_MANAGER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FLOW_MANAGER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT_offsetType = {
     "offsetType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT_keySize = {
     "keySize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FLOW_MANAGER_STC_PTR_flowMngInfoPtr = {
     "flowMngInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_sourcePort = {
     "sourcePort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_flowIdArr = {
     "flowIdArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_flowIdPtr = {
     "flowIdPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_keyIndexArr = {
     "keyIndexArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_enableBitmap = {
     "enableBitmap", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_flowIdCount = {
     "flowIdCount", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_flowMngId = {
     "flowMngId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_keyTableIndex = {
     "keyTableIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_mask = {
     "mask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_offsetOrPattern = {
     "offsetOrPattern", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT_PTR_offsetTypePtr = {
     "offsetTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC_PTR_flowEntryPtr = {
     "flowEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FLOW_MANAGER_FLOW_STC_PTR_flowDataPtr = {
     "flowDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_FLOW_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT_PTR_keySizePtr = {
     "keySizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC_PTR_messagePacketStatsPtr = {
     "messagePacketStatsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC_PTR_flowsLearntPtr = {
     "flowsLearntPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_enableBitmapPtr = {
     "enableBitmapPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_keyIndexArr = {
     "keyIndexArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextFlowIdPtr = {
     "nextFlowIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerDelete_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerCreate_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_CPSS_DXCH_FLOW_MANAGER_STC_PTR_flowMngInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerEnableSet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerFlowIdGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_BOOL_getFirst,
    &DX_OUT_GT_U32_PTR_nextFlowIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerFlowDataGet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_U32_flowId,
    &DX_OUT_CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC_PTR_flowEntryPtr,
    &DX_OUT_CPSS_DXCH_FLOW_MANAGER_FLOW_STC_PTR_flowDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerFlowCountersToDbGet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_U32_flowIdCount,
    &DX_IN_GT_U32_PTR_flowIdArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerKeyEntryUserDefinedByteSet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_U32_keyTableIndex,
    &DX_IN_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT_keySize,
    &DX_IN_GT_U32_udbIndex,
    &DX_IN_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT_offsetType,
    &DX_IN_GT_U8_offsetOrPattern,
    &DX_IN_GT_U8_mask
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerKeyEntryUserDefinedByteGet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_U32_keyTableIndex,
    &DX_IN_GT_U32_udbIndex,
    &DX_OUT_CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT_PTR_keySizePtr,
    &DX_OUT_CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT_PTR_offsetTypePtr,
    &DX_OUT_GT_U8_PTR_offsetPtr,
    &DX_OUT_GT_U8_PTR_maskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerDevListAdd_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_IN_GT_U8_PTR_devListArr,
    &DX_IN_GT_U32_numOfDevs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerMessagePacketStatisticsGet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_OUT_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC_PTR_messagePacketStatsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerEnableGet_PARAMS[] =  {
    &DX_IN_GT_U32_flowMngId,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerSrcPortFlowIdGetNext_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_sourcePort,
    &DX_IN_GT_BOOL_getFirst,
    &DX_IN_GT_U32_PTR_flowIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_sourcePort,
    &DX_IN_GT_U32_PTR_keyIndexArr,
    &DX_IN_GT_U32_enableBitmap
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerSrcPortFlowLearntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_sourcePort,
    &DX_OUT_CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC_PTR_flowsLearntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_sourcePort,
    &DX_OUT_GT_U32_PTR_keyIndexArr,
    &DX_OUT_GT_U32_PTR_enableBitmapPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFlowManagerMessageParseAndCpssDbUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_numOfBuff,
    &DX_IN_GT_U8_PTR_PTR_packetBuffsArrPtr,
    &DX_IN_GT_U32_PTR_buffLenArr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChFlowManagerLogLibDb[] = {
    {"cpssDxChFlowManagerCreate", 2, cpssDxChFlowManagerCreate_PARAMS, NULL},
    {"cpssDxChFlowManagerDevListAdd", 3, cpssDxChFlowManagerDevListAdd_PARAMS, NULL},
    {"cpssDxChFlowManagerDevListRemove", 3, cpssDxChFlowManagerDevListAdd_PARAMS, NULL},
    {"cpssDxChFlowManagerDelete", 1, cpssDxChFlowManagerDelete_PARAMS, NULL},
    {"cpssDxChFlowManagerEnableSet", 2, cpssDxChFlowManagerEnableSet_PARAMS, NULL},
    {"cpssDxChFlowManagerEnableGet", 2, cpssDxChFlowManagerEnableGet_PARAMS, NULL},
    {"cpssDxChFlowManagerReSync", 1, cpssDxChFlowManagerDelete_PARAMS, NULL},
    {"cpssDxChFlowManagerMessageParseAndCpssDbUpdate", 4, cpssDxChFlowManagerMessageParseAndCpssDbUpdate_PARAMS, NULL},
    {"cpssDxChFlowManagerKeyEntryUserDefinedByteSet", 7, cpssDxChFlowManagerKeyEntryUserDefinedByteSet_PARAMS, NULL},
    {"cpssDxChFlowManagerKeyEntryUserDefinedByteGet", 7, cpssDxChFlowManagerKeyEntryUserDefinedByteGet_PARAMS, NULL},
    {"cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet", 4, cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet_PARAMS, NULL},
    {"cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet", 4, cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet_PARAMS, NULL},
    {"cpssDxChFlowManagerSrcPortFlowLearntGet", 3, cpssDxChFlowManagerSrcPortFlowLearntGet_PARAMS, NULL},
    {"cpssDxChFlowManagerFlowCountersToDbGet", 3, cpssDxChFlowManagerFlowCountersToDbGet_PARAMS, NULL},
    {"cpssDxChFlowManagerFlowIdGetNext", 3, cpssDxChFlowManagerFlowIdGetNext_PARAMS, NULL},
    {"cpssDxChFlowManagerFlowDataGet", 4, cpssDxChFlowManagerFlowDataGet_PARAMS, NULL},
    {"cpssDxChFlowManagerMessagePacketStatisticsGet", 2, cpssDxChFlowManagerMessagePacketStatisticsGet_PARAMS, NULL},
    {"cpssDxChFlowManagerSrcPortFlowIdGetNext", 4, cpssDxChFlowManagerSrcPortFlowIdGetNext_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_FLOW_MANAGER(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChFlowManagerLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChFlowManagerLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

