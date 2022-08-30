/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPtpManagerLog.c
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
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtpLog.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManagerCommonTypes.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManagerTypes.h>
#include <cpss/dxCh/dxChxGen/ptpManager/private/prvCpssDxChPtpManagerLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E",
    "CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E",
    "CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E",
    "CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_AM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT[]  =
{
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED4_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED5_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED6_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED7_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_FOLLOW_UP_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ANNOUNCE_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SIGNALING_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_MANAGMENT_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED14_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED15_E",
    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_PHY_E",
    "CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E",
    "CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_MARVELL_FORMAT_E",
    "CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_FORMAT_E",
    "CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E",
    "CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_MAC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_DISABLED_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_FC_FEC_MODE_ENABLED_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_528_514_MODE_ENABLED_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_544_514_MODE_ENABLED_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_DEFAULT_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_2LANE_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_4LANE_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT[]  =
{
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100M_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_1G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_10G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_25G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_40G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_50G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_200G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_400G_E",
    "CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E",
    "CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E",
    "CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E",
    "CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, todValueIsValid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, todValue, CPSS_DXCH_PTP_TOD_COUNT_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_DOMAIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_DOMAIN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, domainIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, domainMode, CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, v1DomainIdArr, 4, GT_U32);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, v2DomainId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, entryValid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, signature, CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seconds);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressPipeDelayEncrypted);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressPipeDelayFsuOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressPipeDelay);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressPortDelay);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpOverEthernetEnable);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, etherType0value);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, etherType1value);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpOverIpUdpEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpPort0value);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpPort1value);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, udpCsUpdMode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, taiNumber, CPSS_DXCH_PTP_TAI_NUMBER_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, outputInterfaceMode, CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, entryValid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queueEntryId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, taiSelect);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, todUpdateFlag);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seconds);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, signatureQueueEntryId, CPSS_DXCH_PTP_MANAGER_SIGNATURE_QUEUE_ENTRY_ID_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, signatureCtr, CPSS_DXCH_PTP_MANAGER_SIGNATURE_CTR_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_SIGNATURE_CTR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_CTR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, signatureCtr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_SIGNATURE_QUEUE_ENTRY_ID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_QUEUE_ENTRY_ID_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queueEntryId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queueSelect);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_DOMAIN_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_DOMAIN_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_DOMAIN_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_domainEntryPtr = {
     "domainEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT_tsMode = {
     "tsMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_DOMAIN_STC_PTR_domainConfPtr = {
     "domainConfPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_DOMAIN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT_messageType = {
     "messageType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT_operationMode = {
     "operationMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT_fecMode = {
     "fecMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT_interfaceMode = {
     "interfaceMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT_speed = {
     "speed", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR_delayValPtr = {
     "delayValPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC_PTR_ptpOverEthernetConfPtr = {
     "ptpOverEthernetConfPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC_PTR_ptpOverIpUdpConfPtr = {
     "ptpOverIpUdpConfPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT_signatureMode = {
     "signatureMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC_PTR_outputInterfaceConfPtr = {
     "outputInterfaceConfPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_inputMode = {
     "inputMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_maskValuePtr = {
     "maskValuePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TOD_COUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_triggerTimePtr = {
     "triggerTimePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TOD_COUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_updateValuePtr = {
     "updateValuePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TOD_COUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_ACTION_ENT_action = {
     "action", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TS_ACTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_egressPortBindMode = {
     "egressPortBindMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_ptpTsTagGlobalConfPtr = {
     "ptpTsTagGlobalConfPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_capturePrevTodValueEnable = {
     "capturePrevTodValueEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_followUpEnable = {
     "followUpEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_maskEnable = {
     "maskEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_offsetPtr = {
     "offsetPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_phaseValue = {
     "phaseValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_signatureValue = {
     "signatureValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_updateValue = {
     "updateValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR_todValueEntry0Ptr = {
     "todValueEntry0Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR_todValueEntry1Ptr = {
     "todValueEntry1Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR_delayValPtr = {
     "delayValPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT_PTR_signatureModePtr = {
     "signatureModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_followUpEnablePtr = {
     "followUpEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_syncEnablePtr = {
     "syncEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_signatureValuePtr = {
     "signatureValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerGlobalPtpDomainSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_DOMAIN_STC_PTR_domainConfPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerTsQueueConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT_messageType,
    &DX_IN_GT_U32_queueNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPtpInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC_PTR_outputInterfaceConfPtr,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC_PTR_ptpOverEthernetConfPtr,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC_PTR_ptpOverIpUdpConfPtr,
    &DX_IN_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_ptpTsTagGlobalConfPtr,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerCaptureTodValueSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerInputPulseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_inputMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerTodValueSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_todValuePtr,
    &DX_IN_GT_BOOL_capturePrevTodValueEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerTodValueUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_updateValuePtr,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_function,
    &DX_IN_GT_U32_gracefulStep,
    &DX_IN_GT_BOOL_capturePrevTodValueEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManager8021AsMsgSelectionEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_BOOL_followUpEnable,
    &DX_IN_GT_BOOL_syncEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_U32_PTR_offsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerTriggerGenerate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_U32_extPulseWidth,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_triggerTimePtr,
    &DX_IN_GT_BOOL_maskEnable,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_maskValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_U32_offset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerMasterPpsActivate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_U32_phaseValue,
    &DX_IN_GT_U32_seconds,
    &DX_IN_GT_U32_nanoSeconds
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerTodValueFrequencyUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_U32_updateValue,
    &DX_IN_GT_BOOL_capturePrevTodValueEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerTodValueGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_OUT_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR_todValueEntry0Ptr,
    &DX_OUT_CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC_PTR_todValueEntry1Ptr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManager8021AsMsgSelectionEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_OUT_GT_BOOL_PTR_followUpEnablePtr,
    &DX_OUT_GT_BOOL_PTR_syncEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortTimestampingModeConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT_tsMode,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT_operationMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortDelayValuesSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT_speed,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT_interfaceMode,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT_fecMode,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR_delayValPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortDelayValuesGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT_speed,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT_interfaceMode,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT_fecMode,
    &DX_OUT_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC_PTR_delayValPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortMacTsQueueSigatureSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT_signatureMode,
    &DX_IN_GT_U32_signatureValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortAllocateTai_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_egressPortBindMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortPacketActionsConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_domainEntryPtr,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT_messageType,
    &DX_IN_CPSS_DXCH_PTP_TS_ACTION_ENT_action
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortCommandAssignment_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT_messageType,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerEgressTimestampPortQueueEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_queueNum,
    &DX_OUT_CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerPortMacTsQueueSigatureGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT_PTR_signatureModePtr,
    &DX_OUT_GT_U32_PTR_signatureValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChPtpManagerLogLibDb[] = {
    {"cpssDxChPtpManagerPtpInit", 6, cpssDxChPtpManagerPtpInit_PARAMS, NULL},
    {"cpssDxChPtpManagerInputPulseSet", 3, cpssDxChPtpManagerInputPulseSet_PARAMS, NULL},
    {"cpssDxChPtpManagerMasterPpsActivate", 5, cpssDxChPtpManagerMasterPpsActivate_PARAMS, NULL},
    {"cpssDxChPtpManagerSlavePpsActivate", 5, cpssDxChPtpManagerMasterPpsActivate_PARAMS, NULL},
    {"cpssDxChPtpManagerPortAllocateTai", 4, cpssDxChPtpManagerPortAllocateTai_PARAMS, NULL},
    {"cpssDxChPtpManagerCaptureTodValueSet", 2, cpssDxChPtpManagerCaptureTodValueSet_PARAMS, NULL},
    {"cpssDxChPtpManagerTodValueSet", 4, cpssDxChPtpManagerTodValueSet_PARAMS, NULL},
    {"cpssDxChPtpManagerTodValueFrequencyUpdate", 4, cpssDxChPtpManagerTodValueFrequencyUpdate_PARAMS, NULL},
    {"cpssDxChPtpManagerTodValueUpdate", 6, cpssDxChPtpManagerTodValueUpdate_PARAMS, NULL},
    {"cpssDxChPtpManagerTodValueGet", 4, cpssDxChPtpManagerTodValueGet_PARAMS, NULL},
    {"cpssDxChPtpManagerTriggerGenerate", 6, cpssDxChPtpManagerTriggerGenerate_PARAMS, NULL},
    {"cpssDxChPtpManagerGlobalPtpDomainSet", 2, cpssDxChPtpManagerGlobalPtpDomainSet_PARAMS, NULL},
    {"cpssDxChPtpManagerPortCommandAssignment", 5, cpssDxChPtpManagerPortCommandAssignment_PARAMS, NULL},
    {"cpssDxChPtpManagerPortPacketActionsConfig", 7, cpssDxChPtpManagerPortPacketActionsConfig_PARAMS, NULL},
    {"cpssDxChPtpManagerPortTsTagConfig", 3, prvCpssLogGenDevNumPortNumTsTagPortCfgPtr_PARAMS, NULL},
    {"cpssDxChPtpManagerTsQueueConfig", 3, cpssDxChPtpManagerTsQueueConfig_PARAMS, NULL},
    {"cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead", 3, cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead_PARAMS, NULL},
    {"cpssDxChPtpManagerEgressTimestampPortQueueEntryRead", 4, cpssDxChPtpManagerEgressTimestampPortQueueEntryRead_PARAMS, NULL},
    {"cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead", 3, cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead_PARAMS, NULL},
    {"cpssDxChPtpManagerPortTimestampingModeConfig", 4, cpssDxChPtpManagerPortTimestampingModeConfig_PARAMS, NULL},
    {"cpssDxChPtpManagerPortDelayValuesSet", 6, cpssDxChPtpManagerPortDelayValuesSet_PARAMS, NULL},
    {"cpssDxChPtpManagerPortDelayValuesGet", 6, cpssDxChPtpManagerPortDelayValuesGet_PARAMS, NULL},
    {"cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead", 3, cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead_PARAMS, NULL},
    {"cpssDxChPtpManager8021AsMsgSelectionEnableSet", 4, cpssDxChPtpManager8021AsMsgSelectionEnableSet_PARAMS, NULL},
    {"cpssDxChPtpManager8021AsMsgSelectionEnableGet", 4, cpssDxChPtpManager8021AsMsgSelectionEnableGet_PARAMS, NULL},
    {"cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet", 3, cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet_PARAMS, NULL},
    {"cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet", 3, cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet_PARAMS, NULL},
    {"cpssDxChPtpManagerPortMacTsQueueSigatureSet", 4, cpssDxChPtpManagerPortMacTsQueueSigatureSet_PARAMS, NULL},
    {"cpssDxChPtpManagerPortMacTsQueueSigatureGet", 4, cpssDxChPtpManagerPortMacTsQueueSigatureGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PTP_MANAGER(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChPtpManagerLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChPtpManagerLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

