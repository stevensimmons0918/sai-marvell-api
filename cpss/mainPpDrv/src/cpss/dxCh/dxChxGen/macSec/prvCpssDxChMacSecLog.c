/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChMacSecLog.c
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
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_MACSEC_DIRECTION_ENT[]  =
{
    "CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E",
    "CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_DIRECTION_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_MACSEC_SECY_PORT_TYPE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_MACSEC_SECY_PORT_CONTROLLED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_MACSEC_SECY_PORT_UNCONTROLLED_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_SECY_PORT_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT[]  =
{
    "CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E",
    "CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E",
    "CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E",
    "CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E",
    "CPSS_DXCH_MACSEC_SECY_SA_ACTION_CRYPT_AUTH_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT[]  =
{
    "CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_DISABLE_COMPARE_E",
    "CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ENABLE_COMPARE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT[]  =
{
    "CPSS_DXCH_MACSEC_CLASSIFY_KAY_DATA_PACKET_E",
    "CPSS_DXCH_MACSEC_CLASSIFY_KAY_CONTROL_PACKET_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT[]  =
{
    "CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_OUTER_E",
    "CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_INNER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MACSEC_SECY_VALIDATE_FRAMES_ENT[]  =
{
    "CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_DISABLE_E",
    "CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_CHECK_E",
    "CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MACSEC_SECY_VALIDATE_FRAMES_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, macDaEtRules, CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS, CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULE_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, macDaEtRange, CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULES_COUNT_CNS, CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macDaRange, CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_RANGE_MATCH_RULE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macDa44BitsConstMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa44BitsConst);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macDa48BitsConstMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa48BitsConst);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macSecKayPktMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_MASK_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassMacsecDevice);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, exceptionCfgEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, forceDrop);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, defaultVPortValid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, defaultVPort);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, key, CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, mask, CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, data, 4, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, dataMask, 4, GT_U32_HEX);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, policy, CPSS_DXCH_MACSEC_CLASSIFY_RULE_POLICY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, compType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkVersion);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkKay);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, lookupUseSci);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macSecTagValue);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, autoStatCntrsReset);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, countFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, portCountFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, countIncDisCtrl);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tcamHitMultiple, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, headerParserDroppedPkts, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tcamMiss, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pktsCtrl, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pktsData, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pktsDropped, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pktsErrIn, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, eccCountersInfo, CPSS_DXCH_MACSEC_CLASSIFY_ECC_NOF_STATUS_COUNTERS_CNS, CPSS_DXCH_MACSEC_ECC_COUNTERS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pktProcessDebug, CPSS_DXCH_MACSEC_CLASSIFY_PKT_PROCESSING_DEBUG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, cp, CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, scpFlag);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, scp, CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sTagUpEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qTagUpEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, defaultUp);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, upTable1, CPSS_DXCH_MACSEC_CLASSIFY_VLAN_UP_MAX_COUNT_CNS, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, upTable2, CPSS_DXCH_MACSEC_CLASSIFY_VLAN_UP_MAX_COUNT_CNS, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qTag);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sTag1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sTag2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sTag3);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, secTagOffset);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, pktExpansion, CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassMacsecDevice);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, statCtrl, CPSS_DXCH_MACSEC_SECY_PORT_STAT_CONTROL_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pktNumThrStrictCompareModeEnable);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ruleSecTag, CPSS_DXCH_MACSEC_SECY_PORT_RULE_SECTAG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, params, CPSS_DXCH_MACSEC_SECY_SA_PARAM_UNT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, actionType, CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, destPort, CPSS_DXCH_MACSEC_SECY_PORT_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, autoStatCntrsReset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seqNrThreshold);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, seqNrThreshold64, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, saCountFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, secyCountFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ifcCountFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ifc1CountFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, rxCamCountFrameThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, saCountOctetThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ifcCountOctetThr, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ifc1CountOctetThr, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, countIncDisCtrl);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egress, CPSS_DXCH_MACSEC_SECY_STAT_IFC_E_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ingress, CPSS_DXCH_MACSEC_SECY_STAT_IFC_I_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, an);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, keyArr, CPSS_DXCH_MACSEC_SECY_MAX_KEY_LENGTH_CNS, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, keyByteCount);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, sciArr, CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, seqTypeExtended);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seqNumLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seqNumHi);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ssciArr, CPSS_DXCH_MACSEC_SECY_SSCI_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, saltArr, CPSS_DXCH_MACSEC_SECY_SALT_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, seqMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, customHkeyEnable);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, customHkeyArr, CPSS_DXCH_MACSEC_SECY_MAX_HKEY_LENGTH_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, eccCountersInfo, CPSS_DXCH_MACSEC_SECY_ECC_NOF_STATUS_COUNTERS_CNS, CPSS_DXCH_MACSEC_ECC_COUNTERS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pktProcessDebug, CPSS_DXCH_MACSEC_SECY_PKT_PROCESSING_DEBUG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egress, CPSS_DXCH_MACSEC_SECY_STAT_SA_E_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ingress, CPSS_DXCH_MACSEC_SECY_STAT_SA_I_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egress, CPSS_DXCH_MACSEC_SECY_STAT_SECY_E_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ingress, CPSS_DXCH_MACSEC_SECY_STAT_SECY_I_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_MASK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_MASK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, primaryKay, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, secondaryKay, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, primaryMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, secondaryMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macDaMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, etherTypeMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC);
    PRV_CPSS_LOG_STC_16_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, etherTypeMatchMode, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macDaRangeEtherTypeMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDaStart);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDaEnd);
    PRV_CPSS_LOG_STC_16_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, etherTypeMatchMode, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_RANGE_MATCH_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_RANGE_MATCH_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macRangeMask, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDaStart);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDaEnd);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_PKT_PROCESSING_DEBUG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PKT_PROCESSING_DEBUG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpMatchDebug);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcamDebug);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, secTagDebug);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedDaLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedDaHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSaLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSaHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSecTagLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSecTagHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, debugFlowLookup);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numTags);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_POLICY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_POLICY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rulePriority);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, drop);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, controlPacket);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, parseQTag);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, parseStag1);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, parseStag2);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, parseStag3);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, parseQinQ);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_ECC_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_ECC_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, correctableCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, uncorrectableCount);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, correctableThr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, uncorrectableThr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_PKT_PROCESSING_DEBUG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_PKT_PROCESSING_DEBUG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedDaLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedDaHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSaLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSaHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSecTagLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSecTagHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSciLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parsedSciHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, secTagDebug);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxCamSciLo);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxCamSciHi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, parserInDebug);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_PORT_RULE_SECTAG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_PORT_RULE_SECTAG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, compEtype);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkV);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkKay);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkCe);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkSc);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkSl);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkPn);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_PORT_STAT_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_PORT_STAT_CONTROL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seqNrThreshold);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, seqNrThreshold64, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_PARAM_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_PARAM_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egress, CPSS_DXCH_MACSEC_SECY_SA_EGR_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ingress, CPSS_DXCH_MACSEC_SECY_SA_ING_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, bypassDrop, CPSS_DXCH_MACSEC_SECY_SA_BYPASS_DROP_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, cryptAuth, CPSS_DXCH_MACSEC_SECY_SA_CRYPT_AUTH_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_IFC_E_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_IFC_E_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsUnicastUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsMulticastUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsBroadcastUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsUnicastControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsMulticastControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsBroadcastControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outOctetsUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outOctetsControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outOctetsCommon, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_IFC_I_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_IFC_I_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsUnicastUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsMulticastUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsBroadcastUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsUnicastControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsMulticastControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsBroadcastControlled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inOctetsUncontrolled, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inOctetsControlled, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_SA_E_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_SA_E_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsEncryptedProtected, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsTooLong, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsSANotInUse, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outOctetsEncryptedProtected, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_SA_I_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_SA_I_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsUnchecked, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsDelayed, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsLate, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsOK, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsInvalid, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsNotValid, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsNotUsingSA, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsUnusedSA, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inOctetsDecrypted, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inOctetsValidated, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_SECY_E_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_SECY_E_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsTransformError, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsControl, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outPktsUntagged, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_SECY_I_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_SECY_I_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsTransformError, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsControl, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsUntagged, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsNoTag, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsBadTag, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsNoSCI, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsUnknownSCI, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, inPktsTaggedCtrl, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, primaryMatchMode, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, secondaryMatchMode, CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_BYPASS_DROP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_BYPASS_DROP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, saInUse);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_CRYPT_AUTH_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_CRYPT_AUTH_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, zeroLengthMessage);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, confidentialityOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ivMode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, icvAppend);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, icvVerify);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, confProtect);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_EGR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_EGR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, saInUse);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, confidentialityOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, protectFrames);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, includeSci);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, useEs);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, useScb);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, confProtect);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, controlledPortEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, preSecTagAuthStart);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, preSecTagAuthLength);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_ING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_ING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, saInUse);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, confidentialityOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, replayProtect);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, validateFramesTagged, CPSS_DXCH_MACSEC_SECY_VALIDATE_FRAMES_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, sciArr, CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, an);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, allowTagged);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, allowUntagged);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, validateUntagged);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, preSecTagAuthStart);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, preSecTagAuthLength);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainSecTag);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainIcv);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_DIRECTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_DIRECTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MACSEC_DIRECTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_SA_HANDLE(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_SECY_SA_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_GT_MACSEC_UNIT_BMP(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_MACSEC_UNIT_BMP, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR_ctrlPktDetectPtr = {
     "ctrlPktDetectPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR_portCfgPtr = {
     "portCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle = {
     "ruleHandle", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandleDisable = {
     "ruleHandleDisable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandleEnable = {
     "ruleHandleEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR_ruleParamsPtr = {
     "ruleParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR_secTagParserPtr = {
     "secTagParserPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr = {
     "statisticsCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR_vlanParserPtr = {
     "vlanParserPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle = {
     "vPortHandle", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR_vPortParamsPtr = {
     "vPortParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction = {
     "direction", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR_portCfgPtr = {
     "portCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_activeSaHandle = {
     "activeSaHandle", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_currentSaHandle = {
     "currentSaHandle", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle = {
     "saHandle", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR_saParamsPtr = {
     "saParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_SA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr = {
     "statisticsCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR_newTrRecParamsPtr = {
     "newTrRecParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR_trRecParamsPtr = {
     "trRecParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_allPorts = {
     "allPorts", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_allRules = {
     "allRules", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_allSas = {
     "allSas", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_allvPorts = {
     "allvPorts", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_disableAll = {
     "disableAll", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableAll = {
     "enableAll", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_getEccStatus = {
     "getEccStatus", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_getPktProcessDebug = {
     "getPktProcessDebug", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_includeRule = {
     "includeRule", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_includeSa = {
     "includeSa", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_MACSEC_UNIT_BMP_unitBmp = {
     "unitBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_MACSEC_UNIT_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_devNum = {
     "devNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nextPnHi = {
     "nextPnHi", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nextPnLo = {
     "nextPnLo", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_saIndex = {
     "saIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_scIndex = {
     "scIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vPort = {
     "vPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vPortId = {
     "vPortId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vPortIndex = {
     "vPortIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_sciArr = {
     "sciArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR_ctrlPktDetectPtr = {
     "ctrlPktDetectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR_portCfgPtr = {
     "portCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_PTR_ruleHandlePtr = {
     "ruleHandlePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR_ruleDataPtr = {
     "ruleDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR_secTagParserPtr = {
     "secTagParserPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr = {
     "statisticsCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC_PTR_portStatPtr = {
     "portStatPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC_PTR_unitStatusPtr = {
     "unitStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR_vlanParserPtr = {
     "vlanParserPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_PTR_vPortHandlePtr = {
     "vPortHandlePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR_vPortDataPtr = {
     "vPortDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR_portCfgPtr = {
     "portCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_activeSaHandleArr = {
     "activeSaHandleArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_newSaHandlePtr = {
     "newSaHandlePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_nextSaHandlePtr = {
     "nextSaHandlePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_saHandlePtr = {
     "saHandlePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_SA_HANDLE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR_saParamsPtr = {
     "saParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_SA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr = {
     "statisticsCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT_PTR_statIfcPtr = {
     "statIfcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC_PTR_unitStatusPtr = {
     "unitStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT_PTR_statSaPtr = {
     "statSaPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT_PTR_statSecyPtr = {
     "statSecyPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_extendedPnPtr = {
     "extendedPnPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_nextPnWrittenPtr = {
     "nextPnWrittenPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ruleEnablePtr = {
     "ruleEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portsArr = {
     "portsArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_indexArr = {
     "indexArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_indexArrSizePtr = {
     "indexArrSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_indexesArrSizePtr = {
     "indexesArrSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxPortNumPtr = {
     "maxPortNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxRuleNumPtr = {
     "maxRuleNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxSaNumPtr = {
     "maxSaNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxScNumPtr = {
     "maxScNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxVportNumPtr = {
     "maxVportNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextPnHiPtr = {
     "nextPnHiPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextPnLoPtr = {
     "nextPnLoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portsArrSizePtr = {
     "portsArrSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ruleIndexPtr = {
     "ruleIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_saIndexPtr = {
     "saIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_scIndexPtr = {
     "scIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vPortIdPtr = {
     "vPortIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vPortIndexPtr = {
     "vPortIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_statRxCamCounterPtr = {
     "statRxCamCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_statTcamHitsCounterPtr = {
     "statTcamHitsCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_sciArr = {
     "sciArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportIndexGet_PARAMS[] =  {
    &DX_IN_GT_U32_devNum,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle,
    &DX_OUT_GT_U32_PTR_vPortIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle,
    &DX_OUT_GT_U32_PTR_ruleIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_OUT_GT_U32_PTR_saIndexPtr,
    &DX_OUT_GT_U32_PTR_scIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyStatisticsConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyGlobalCfgDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleRemove_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle,
    &DX_IN_GT_BOOL_allRules
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_PTR_vPortHandlePtr,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR_ruleDataPtr,
    &DX_OUT_GT_BOOL_PTR_ruleEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleEnableDisable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandleEnable,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandleDisable,
    &DX_IN_GT_BOOL_enableAll,
    &DX_IN_GT_BOOL_disableAll
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportRemove_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_ruleHandle,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR_ruleParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC_PTR_ruleParamsPtr,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_PTR_ruleHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR_vPortParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle,
    &DX_IN_GT_BOOL_allvPorts,
    &DX_IN_GT_BOOL_includeRule
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_vPortHandle,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR_vPortDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC_PTR_vPortParamsPtr,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_PTR_vPortHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaRemove_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR_saParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_IN_GT_BOOL_allSas
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsSaGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_IN_GT_BOOL_syncEnable,
    &DX_OUT_CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT_PTR_statSaPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR_saParamsPtr,
    &DX_OUT_GT_U32_PTR_vPortIdPtr,
    &DX_OUT_GT_U8_PTR_sciArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaNextPnGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_OUT_GT_U32_PTR_nextPnLoPtr,
    &DX_OUT_GT_U32_PTR_nextPnHiPtr,
    &DX_OUT_GT_BOOL_PTR_extendedPnPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_BOOL_getEccStatus,
    &DX_IN_GT_BOOL_getPktProcessDebug,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC_PTR_unitStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_BOOL_getEccStatus,
    &DX_IN_GT_BOOL_getPktProcessDebug,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC_PTR_unitStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortSecyDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_allPorts
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyStatisticsTcamHitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_ruleId,
    &DX_IN_GT_BOOL_syncEnable,
    &DX_OUT_GT_U64_PTR_statTcamHitsCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyRuleHandleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_ruleIndex,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE_PTR_ruleHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaHandleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_saIndex,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_saHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsVportClear_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPort
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsIfcGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPort,
    &DX_IN_GT_BOOL_syncEnable,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT_PTR_statIfcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsSecyGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPort,
    &DX_IN_GT_BOOL_syncEnable,
    &DX_OUT_CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT_PTR_statSecyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPortId,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_STC_PTR_saParamsPtr,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR_trRecParamsPtr,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_saHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyVportDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPortId,
    &DX_IN_GT_BOOL_allvPorts,
    &DX_IN_GT_BOOL_includeSa
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaActiveGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPortId,
    &DX_IN_GT_U8_PTR_sciArr,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_activeSaHandleArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyVportHandleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_vPortIndex,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE_PTR_vPortHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecCfyePortCountSummaryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portsArr,
    &DX_OUT_GT_U32_PTR_portsArrSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecCfyeTcamCountSummaryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_GT_U32_PTR_indexArr,
    &DX_OUT_GT_U32_PTR_indexArrSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSAExpiredSummaryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_GT_U32_PTR_indexesArr,
    &DX_OUT_GT_U32_PTR_indexesArrSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaChainSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_activeSaHandle,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC_PTR_newTrRecParamsPtr,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_newSaHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaChainGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_currentSaHandle,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_PTR_nextSaHandlePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecySaNextPnUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_SA_HANDLE_saHandle,
    &DX_IN_GT_U32_nextPnLo,
    &DX_IN_GT_U32_nextPnHi,
    &DX_OUT_GT_BOOL_PTR_nextPnWrittenPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsRxCamGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_IN_GT_U32_scIndex,
    &DX_IN_GT_BOOL_syncEnable,
    &DX_OUT_GT_U64_PTR_statRxCamCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyStatisticsConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyStatisticsConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC_PTR_statisticsCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecClassifyLimitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_OUT_GT_U32_PTR_maxPortNumPtr,
    &DX_OUT_GT_U32_PTR_maxVportNumPtr,
    &DX_OUT_GT_U32_PTR_maxRuleNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSecyLimitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_OUT_GT_U32_PTR_maxPortNumPtr,
    &DX_OUT_GT_U32_PTR_maxVportNumPtr,
    &DX_OUT_GT_U32_PTR_maxSaNumPtr,
    &DX_OUT_GT_U32_PTR_maxScNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecSAPNThresholdSummaryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_MACSEC_UNIT_BMP_unitBmp,
    &DX_OUT_GT_U32_PTR_indexesArr,
    &DX_OUT_GT_U32_PTR_indexesArrSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyControlPktDetectConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR_ctrlPktDetectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR_portCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR_secTagParserPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyHdrParserVlanConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR_vlanParserPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortSecyConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR_portCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyStatisticsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_IN_GT_BOOL_syncEnable,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC_PTR_portStatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyControlPktDetectConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC_PTR_ctrlPktDetectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC_PTR_portCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC_PTR_secTagParserPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortClassifyHdrParserVlanConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC_PTR_vlanParserPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMacSecPortSecyConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_MACSEC_DIRECTION_ENT_direction,
    &DX_OUT_CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC_PTR_portCfgPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChMacSecLogLibDb[] = {
    {"cpssDxChMacSecInit", 2, cpssDxChMacSecInit_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyConfigSet", 4, cpssDxChMacSecPortClassifyConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyConfigGet", 4, cpssDxChMacSecPortClassifyConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportAdd", 5, cpssDxChMacSecClassifyVportAdd_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportRemove", 4, cpssDxChMacSecClassifyVportRemove_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportIndexGet", 3, cpssDxChMacSecClassifyVportIndexGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleAdd", 6, cpssDxChMacSecClassifyRuleAdd_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleRemove", 4, cpssDxChMacSecClassifyRuleRemove_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleEnable", 5, cpssDxChMacSecClassifyRuleEnable_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleIndexGet", 3, cpssDxChMacSecClassifyRuleIndexGet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaAdd", 7, cpssDxChMacSecSecySaAdd_PARAMS, NULL},
    {"cpssDxChMacSecSecySaRemove", 4, cpssDxChMacSecSecySaRemove_PARAMS, NULL},
    {"cpssDxChMacSecClassifyStatisticsTcamHitsGet", 6, cpssDxChMacSecClassifyStatisticsTcamHitsGet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyStatisticsGet", 5, cpssDxChMacSecPortClassifyStatisticsGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsSaGet", 6, cpssDxChMacSecSecyStatisticsSaGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsSecyGet", 6, cpssDxChMacSecSecyStatisticsSecyGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsIfcGet", 6, cpssDxChMacSecSecyStatisticsIfcGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsRxCamGet", 5, cpssDxChMacSecSecyStatisticsRxCamGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsVportClear", 4, cpssDxChMacSecSecyStatisticsVportClear_PARAMS, NULL},
    {"cpssDxChMacSecSecySaChainSet", 5, cpssDxChMacSecSecySaChainSet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaChainGet", 4, cpssDxChMacSecSecySaChainGet_PARAMS, NULL},
    {"cpssDxChMacSecPortSecyConfigSet", 4, cpssDxChMacSecPortSecyConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecPortSecyConfigGet", 4, cpssDxChMacSecPortSecyConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet", 4, cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet", 4, cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyHdrParserVlanConfigSet", 4, cpssDxChMacSecPortClassifyHdrParserVlanConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyHdrParserVlanConfigGet", 4, cpssDxChMacSecPortClassifyHdrParserVlanConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyStatisticsConfigSet", 3, cpssDxChMacSecClassifyStatisticsConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyStatisticsConfigGet", 3, cpssDxChMacSecClassifyStatisticsConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyControlPktDetectConfigSet", 4, cpssDxChMacSecPortClassifyControlPktDetectConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyControlPktDetectConfigGet", 4, cpssDxChMacSecPortClassifyControlPktDetectConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsConfigSet", 3, cpssDxChMacSecSecyStatisticsConfigSet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatisticsConfigGet", 3, cpssDxChMacSecSecyStatisticsConfigGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportSet", 5, cpssDxChMacSecClassifyVportSet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportGet", 5, cpssDxChMacSecClassifyVportGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportHandleGet", 5, cpssDxChMacSecClassifyVportHandleGet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaSet", 5, cpssDxChMacSecSecySaSet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaGet", 7, cpssDxChMacSecSecySaGet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaIndexGet", 4, cpssDxChMacSecSecySaIndexGet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaHandleGet", 5, cpssDxChMacSecSecySaHandleGet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaActiveGet", 6, cpssDxChMacSecSecySaActiveGet_PARAMS, NULL},
    {"cpssDxChMacSecSecySaNextPnUpdate", 6, cpssDxChMacSecSecySaNextPnUpdate_PARAMS, NULL},
    {"cpssDxChMacSecSecySaNextPnGet", 7, cpssDxChMacSecSecySaNextPnGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleSet", 6, cpssDxChMacSecClassifyRuleSet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleGet", 7, cpssDxChMacSecClassifyRuleGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleHandleGet", 5, cpssDxChMacSecClassifyRuleHandleGet_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleEnableDisable", 7, cpssDxChMacSecClassifyRuleEnableDisable_PARAMS, NULL},
    {"cpssDxChMacSecClassifyLimitsGet", 5, cpssDxChMacSecClassifyLimitsGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyLimitsGet", 6, cpssDxChMacSecSecyLimitsGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyGlobalCfgDump", 3, cpssDxChMacSecSecyGlobalCfgDump_PARAMS, NULL},
    {"cpssDxChMacSecPortSecyDump", 5, cpssDxChMacSecPortSecyDump_PARAMS, NULL},
    {"cpssDxChMacSecSecyVportDump", 6, cpssDxChMacSecSecyVportDump_PARAMS, NULL},
    {"cpssDxChMacSecSecySaDump", 5, cpssDxChMacSecSecySaDump_PARAMS, NULL},
    {"cpssDxChMacSecClassifyGlobalCfgDump", 3, cpssDxChMacSecSecyGlobalCfgDump_PARAMS, NULL},
    {"cpssDxChMacSecPortClassifyDump", 5, cpssDxChMacSecPortSecyDump_PARAMS, NULL},
    {"cpssDxChMacSecClassifyVportDump", 6, cpssDxChMacSecClassifyVportDump_PARAMS, NULL},
    {"cpssDxChMacSecClassifyRuleDump", 5, cpssDxChMacSecClassifyRuleDump_PARAMS, NULL},
    {"cpssDxChMacSecClassifyStatusGet", 6, cpssDxChMacSecClassifyStatusGet_PARAMS, NULL},
    {"cpssDxChMacSecSecyStatusGet", 6, cpssDxChMacSecSecyStatusGet_PARAMS, NULL},
    {"cpssDxChMacSecSAExpiredSummaryGet", 5, cpssDxChMacSecSAExpiredSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecSAPNThresholdSummaryGet", 4, cpssDxChMacSecSAPNThresholdSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecSACountSummaryGet", 5, cpssDxChMacSecSAExpiredSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecSecYCountSummaryGet", 5, cpssDxChMacSecSAExpiredSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecIfc0CountSummaryGet", 5, cpssDxChMacSecSAExpiredSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecIfc1CountSummaryGet", 4, cpssDxChMacSecSAPNThresholdSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecRxCamCountSummaryGet", 4, cpssDxChMacSecSAPNThresholdSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecCfyePortCountSummaryGet", 5, cpssDxChMacSecCfyePortCountSummaryGet_PARAMS, NULL},
    {"cpssDxChMacSecCfyeTcamCountSummaryGet", 5, cpssDxChMacSecCfyeTcamCountSummaryGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_MAC_SEC(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChMacSecLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChMacSecLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

