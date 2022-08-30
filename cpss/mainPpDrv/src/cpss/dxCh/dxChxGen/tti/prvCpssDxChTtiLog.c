/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTtiLog.c
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
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPclLog.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/tti/private/prvCpssDxChTtiLog.h>
#include <cpss/dxCh/dxChxGen/tunnel/private/prvCpssDxChTunnelLog.h>
#include <cpss/generic/bridge/private/prvCpssGenBrgLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>
#include <cpss/generic/tunnel/private/prvCpssGenTunnelLog.h>


/********* enums *********/

const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TTI_EXCEPTION_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_EXCEPTION_ILLEGAL_TUNNEL_LENGTH_ERROR_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_EXCEPTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT[]  =
{
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_VER_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_SOF_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_R_CTL_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_HEADER_LENGTH_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_S_ID_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_MC_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_VER_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_TYPE_E",
    "CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_GENERIC_ACTION_MODE_DST_EPG_E",
    "CPSS_DXCH_TTI_GENERIC_ACTION_MODE_SRC_EPG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_KEY_SIZE_ENT[]  =
{
    "CPSS_DXCH_TTI_KEY_SIZE_10_B_E",
    "CPSS_DXCH_TTI_KEY_SIZE_20_B_E",
    "CPSS_DXCH_TTI_KEY_SIZE_30_B_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_KEY_SIZE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E",
    "CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E",
    "CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E",
    "CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E",
    "CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E",
    "CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_KEY_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_KEY_IPV4_E",
    "CPSS_DXCH_TTI_KEY_MPLS_E",
    "CPSS_DXCH_TTI_KEY_ETH_E",
    "CPSS_DXCH_TTI_KEY_MIM_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E",
    "CPSS_DXCH_TTI_KEY_UDB_MPLS_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E",
    "CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV6_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E",
    "CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE1_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE2_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE3_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE4_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE5_E",
    "CPSS_DXCH_TTI_KEY_UDB_UDE6_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_KEY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_MAC_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_MAC_MODE_DA_E",
    "CPSS_DXCH_TTI_MAC_MODE_SA_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_MAC_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E",
    "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E",
    "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_GRE_E",
    "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_GRE_E",
    "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_UDP_PORT_E",
    "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_UDP_PORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_OFFSET_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_OFFSET_L2_E",
    "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",
    "CPSS_DXCH_TTI_OFFSET_L4_E",
    "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E",
    "CPSS_DXCH_TTI_OFFSET_METADATA_E",
    "CPSS_DXCH_TTI_OFFSET_INVALID_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_OFFSET_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_PCL_ID_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_PCL_ID_MODE_PER_PACKET_TYPE_E",
    "CPSS_DXCH_TTI_PCL_ID_MODE_PER_PORT_E",
    "CPSS_DXCH_TTI_PCL_ID_MODE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_PCL_ID_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT[]  =
{
    "CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E",
    "CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E",
    "CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E",
    "CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E",
    "CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E",
    "CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_RULE_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_RULE_IPV4_E",
    "CPSS_DXCH_TTI_RULE_MPLS_E",
    "CPSS_DXCH_TTI_RULE_ETH_E",
    "CPSS_DXCH_TTI_RULE_MIM_E",
    "CPSS_DXCH_TTI_RULE_UDB_10_E",
    "CPSS_DXCH_TTI_RULE_UDB_20_E",
    "CPSS_DXCH_TTI_RULE_UDB_30_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_RULE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PACKET_TYPE_E",
    "CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_E",
    "CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_PER_PORT_AND_PACKET_TYPE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_IP_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_IP_TO_ME_DONT_USE_SRC_INTERFACE_FIELDS_E",
    "CPSS_DXCH_TTI_IP_TO_ME_USE_SRC_INTERFACE_FIELDS_E",
    "CPSS_DXCH_TTI_IP_TO_ME_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_IP_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E",
    "CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E",
    "CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_MODIFY_DSCP_ENT[]  =
{
    "CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E",
    "CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E",
    "CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E",
    "CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_MODIFY_DSCP_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_MODIFY_UP_ENT[]  =
{
    "CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E",
    "CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E",
    "CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E",
    "CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_MODIFY_UP_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_MPLS_CMD_ENT[]  =
{
    "CPSS_DXCH_TTI_MPLS_NOP_CMD_E",
    "CPSS_DXCH_TTI_MPLS_SWAP_CMD_E",
    "CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E",
    "CPSS_DXCH_TTI_MPLS_POP1_CMD_E",
    "CPSS_DXCH_TTI_MPLS_POP2_CMD_E",
    "CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E",
    "CPSS_DXCH_TTI_MPLS_POP3_CMD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_MPLS_CMD_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E",
    "CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E",
    "CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E",
    "CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_PASSENGER_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_PASSENGER_IPV4_E",
    "CPSS_DXCH_TTI_PASSENGER_IPV6_E",
    "CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E",
    "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E",
    "CPSS_DXCH_TTI_PASSENGER_IPV4V6_E",
    "CPSS_DXCH_TTI_PASSENGER_MPLS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_PASSENGER_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT[]  =
{
    "CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E",
    "CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E",
    "CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E",
    "CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_RESERVED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_PW_TAG_MODE_ENT[]  =
{
    "CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E",
    "CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E",
    "CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_PW_TAG_MODE_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_NO_REDIRECT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_VRF_ID_ASSIGN_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TTI_ASSIGN_GENERIC_ACTION_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT[]  =
{
    "CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E",
    "CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E",
    "CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E",
    "CPSS_DXCH_TTI_TAG1_UP_ASSIGN_NONE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TTI_VLAN_COMMAND_ENT[]  =
{
    "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E",
    "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
    "CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E",
    "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TTI_VLAN_COMMAND_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_ACTION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_ACTION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tunnelTerminate);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ttPassengerPacketType, CPSS_DXCH_TTI_PASSENGER_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tsPassengerPacketType, CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttHeaderLength);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tunnelHeaderLengthAnchorType, CPSS_DXCH_TUNNEL_HEADER_LENGTH_ANCHOR_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, continueToNextTtiLookup);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, copyTtlExpFromTunnelHeader);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mplsCommand, CPSS_DXCH_TTI_MPLS_CMD_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mplsTtl);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableDecrementTtl);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, passengerParsingOfTransitMplsTunnelMode, CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, passengerParsingOfTransitNonMplsTransitTunnelEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, command, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, redirectCommand, CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egressInterface, CPSS_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, arpPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tunnelStart);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tunnelStartPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, routerLttPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrfId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sourceIdSetEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sourceId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tag0VlanCmd, CPSS_DXCH_TTI_VLAN_COMMAND_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tag0VlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tag1VlanCmd, CPSS_DXCH_TTI_VLAN_COMMAND_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tag1VlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tag0VlanPrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, nestedVlanEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bindToPolicerMeter);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bindToPolicer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, policerIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ipfixEn);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, qosPrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, keepPreviousQoS);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, trustUp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, trustDscp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, trustExp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosProfile);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyTag0Up, CPSS_DXCH_TTI_MODIFY_UP_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tag1UpCommand, CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyDscp, CPSS_DXCH_TTI_MODIFY_DSCP_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tag0Up);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tag1Up);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, remapDSCP);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qosUseUpAsIndexEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosMappingTableIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, mplsLLspQoSProfileEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pcl0OverrideConfigIndex, CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pcl0_1OverrideConfigIndex, CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pcl1OverrideConfigIndex, CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iPclConfigIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, iPclUdbConfigTableEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, iPclUdbConfigTableIndex, CPSS_DXCH_PCL_PACKET_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, mirrorToIngressAnalyzerEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mirrorToIngressAnalyzerIndex);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, userDefinedCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bindToCentralCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, centralCounterIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, vntl2Echo);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bridgeBypass);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ingressPipeBypass);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, actionStop);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, hashMaskIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, modifyMacSa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, modifyMacDa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ResetSrcPortGroupId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, multiPortGroupTtiEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sourceEPortAssignmentEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sourceEPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, setMacToMe);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, rxProtectionSwitchEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, rxIsProtectionPath);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pwTagMode, CPSS_DXCH_TTI_PW_TAG_MODE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, oamTimeStampEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, oamOffsetIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, oamProcessEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, oamProcessWhenGalOrOalExistsEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, oamProfile);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, oamChannelTypeToOpcodeMappingEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isPtpPacket);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ptpTriggerType, CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ptpOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, cwBasedPw);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlExpiryVccvEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pwe3FlowLabelExist);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pwCwBasedETreeEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, applyNonDataCwCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unknownSaCommandEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, unknownSaCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sourceMeshIdSetEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sourceMeshId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, skipFdbSaLookupEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ipv6SegmentRoutingEndNodeEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, exactMatchOverTtiEn);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, copyReservedAssignmentEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, copyReserved);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, triggerHashCncClient);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, genericAction);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_DIP_VLAN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isValid);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipType, CPSS_IP_PROTOCOL_STACK_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, destIp, GT_IP_ADDR_TYPE_UNT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prefixLength);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, includeVlanId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, includeSrcInterface, CPSS_DXCH_TTI_IP_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcHwDevice);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcIsTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortTrunk);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, includeSrcInterface, CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcHwDevice);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcIsTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortTrunk);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_MAC_VLAN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, mac);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, lengthFieldAnchorType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lengthFieldUdbMsb);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lengthFieldUdbLsb);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lengthFieldSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lengthFieldLsBit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, constant);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, multiplier);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_ETH_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_ETH_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, common, CPSS_DXCH_TTI_RULE_COMMON_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi0);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isVlan1Exists);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi1);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, macToMe);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dsaQosProfile);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tag0TpidIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tag1TpidIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, eTagGrp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_IPV4_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_IPV4_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, common, CPSS_DXCH_TTI_RULE_COMMON_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tunneltype);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, srcIp);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, destIp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isArp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_MIM_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_MIM_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, common, CPSS_DXCH_TTI_RULE_COMMON_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bUp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bDp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iSid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iUp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iDp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iRes1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iRes2);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, macToMe);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, passengerPacketOuterTagExists);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, passengerPacketOuterTagVid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, passengerPacketOuterTagUp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, passengerPacketOuterTagDei);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_MPLS_RULE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_MPLS_RULE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, common, CPSS_DXCH_TTI_RULE_COMMON_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, label0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, label1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, label2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfLabels);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, protocolAboveMPLS);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, reservedLabelExist);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reservedLabelValue);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, channelTypeProfile);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dataAfterInnerLabel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cwFirstNibble);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_UDB_ARR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_UDB_ARR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, udb, CPSS_DXCH_TTI_MAX_UDB_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TTI_RULE_COMMON_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TTI_RULE_COMMON_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pclId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcIsTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortTrunk);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, mac);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTagged);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dsaSrcIsTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dsaSrcPortTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dsaSrcDevice);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sourcePortGroupId);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TTI_ACTION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_ACTION_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_ACTION_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_DIP_VLAN_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_EXCEPTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_EXCEPTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_DXCH_TTI_EXCEPTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_KEY_SIZE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_KEY_SIZE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_KEY_SIZE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_KEY_SIZE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_KEY_SIZE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_KEY_SIZE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_KEY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_KEY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_KEY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_MAC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_MAC_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_MAC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_MAC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_MAC_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_MAC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_MAC_VLAN_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_OFFSET_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_OFFSET_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_OFFSET_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_OFFSET_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_OFFSET_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_PCL_ID_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_PCL_ID_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_PCL_ID_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_PCL_ID_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_PCL_ID_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_PCL_ID_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_RULE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_RULE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_RULE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_RULE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_RULE_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_RULE_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr = {
     "actionPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_ACTION_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR_valuePtr = {
     "valuePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_DIP_VLAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_EXCEPTION_ENT_exceptionType = {
     "exceptionType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_EXCEPTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT_exceptionType = {
     "exceptionType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT_genericActionMode = {
     "genericActionMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr = {
     "interfaceInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_KEY_SIZE_ENT_size = {
     "size", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_KEY_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT_segmentMode = {
     "segmentMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_packetType = {
     "packetType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_KEY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_MAC_MODE_ENT_macMode = {
     "macMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_MAC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr = {
     "interfaceInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_VLAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_valuePtr = {
     "valuePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_VLAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_offsetType = {
     "offsetType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_OFFSET_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_PCL_ID_MODE_ENT_pclIdMode = {
     "pclIdMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_PCL_ID_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT_exceptionType = {
     "exceptionType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_patternPtr = {
     "patternPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT_tcamProfileIdMode = {
     "tcamProfileIdMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR_tunnelHeaderLengthProfilePtr = {
     "tunnelHeaderLengthProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_ETHER_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_ETHER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_ipType = {
     "ipType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IP_PROTOCOL_STACK_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NET_RX_CPU_CODE_ENT_code = {
     "code", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_NET_RX_CPU_CODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TUNNEL_ETHERTYPE_TYPE_ENT_ethertypeType = {
     "ethertypeType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TUNNEL_ETHERTYPE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TUNNEL_MULTICAST_TYPE_ENT_protocol = {
     "protocol", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TUNNEL_MULTICAST_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_addressPtr = {
     "addressPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_udpPort = {
     "udpPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U16)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_ethertype = {
     "ethertype", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cpuCodeBase = {
     "cpuCodeBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_extensionHeaderId = {
     "extensionHeaderId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_extensionHeaderValue = {
     "extensionHeaderValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lookup = {
     "lookup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_overrideBitmap = {
     "overrideBitmap", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pclId = {
     "pclId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portProfileId = {
     "portProfileId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_profileIdx = {
     "profileIdx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr = {
     "actionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_ACTION_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR_valuePtr = {
     "valuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_DIP_VLAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT_PTR_genericActionModePtr = {
     "genericActionModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr = {
     "interfaceInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_KEY_SIZE_ENT_PTR_sizePtr = {
     "sizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_KEY_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT_PTR_segmentModePtr = {
     "segmentModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_MAC_MODE_ENT_PTR_macModePtr = {
     "macModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr = {
     "interfaceInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_VLAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_valuePtr = {
     "valuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MAC_VLAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_PTR_offsetTypePtr = {
     "offsetTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_OFFSET_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_PCL_ID_MODE_ENT_PTR_pclIdModePtr = {
     "pclIdModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_PCL_ID_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_patternPtr = {
     "patternPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT_PTR_tcamProfileIdModePtr = {
     "tcamProfileIdModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR_tunnelHeaderLengthProfilePtr = {
     "tunnelHeaderLengthProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_ETHER_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_ETHER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_codePtr = {
     "codePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_NET_RX_CPU_CODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_addressPtr = {
     "addressPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_udpPortPtr = {
     "udpPortPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U16)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_ethertypePtr = {
     "ethertypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cpuCodeBasePtr = {
     "cpuCodeBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_extensionHeaderValuePtr = {
     "extensionHeaderValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_overrideBitmapPtr = {
     "overrideBitmapPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pclIdPtr = {
     "pclIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiExceptionCpuCodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_EXCEPTION_ENT_exceptionType,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_code
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiExceptionCmdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_EXCEPTION_ENT_exceptionType,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiExceptionCpuCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_EXCEPTION_ENT_exceptionType,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_codePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiExceptionCmdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_EXCEPTION_ENT_exceptionType,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiFcoeExceptionConfiguratiosEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT_exceptionType,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiFcoeExceptionConfiguratiosEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT_exceptionType,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiFcoeExceptionCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT_exceptionType,
    &DX_OUT_GT_U32_PTR_counterValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPacketTypeKeySizeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_TTI_KEY_SIZE_ENT_size
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiTcamSegmentModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT_segmentMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMacModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_TTI_MAC_MODE_ENT_macMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPclIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_GT_U32_pclId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiUserDefinedByteSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_GT_U32_udbIndex,
    &DX_IN_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_offsetType,
    &DX_IN_GT_U8_offset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiUserDefinedByteGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_GT_U32_udbIndex,
    &DX_OUT_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_PTR_offsetTypePtr,
    &DX_OUT_GT_U8_PTR_offsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPacketTypeKeySizeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_OUT_CPSS_DXCH_TTI_KEY_SIZE_ENT_PTR_sizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiTcamSegmentModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_OUT_CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT_PTR_segmentModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMacModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_OUT_CPSS_DXCH_TTI_MAC_MODE_ENT_PTR_macModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPclIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_OUT_GT_U32_PTR_pclIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortAndPacketTypeTcamProfileIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_packetType,
    &DX_IN_GT_U32_portProfileId,
    &DX_IN_GT_U32_tcamProfileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortAndPacketTypeTcamProfileIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_packetType,
    &DX_IN_GT_U32_portProfileId,
    &DX_OUT_GT_U32_PTR_tcamProfileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPacketTypeTcamProfileIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_packetType,
    &DX_IN_GT_U32_tcamProfileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPacketTypeTcamProfileIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_packetType,
    &DX_OUT_GT_U32_PTR_tcamProfileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPwCwExceptionCmdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT_exceptionType,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPwCwExceptionCmdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT_exceptionType,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiIpTotalLengthDeductionValueSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_ipType,
    &DX_IN_GT_U32_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiIpTotalLengthDeductionValueGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_ipType,
    &DX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiEthernetTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TUNNEL_ETHERTYPE_TYPE_ENT_ethertypeType,
    &DX_IN_GT_U32_HEX_ethertype
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiEthernetTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TUNNEL_ETHERTYPE_TYPE_ENT_ethertypeType,
    &DX_OUT_GT_U32_HEX_PTR_ethertypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMplsMcTunnelTriggeringMacDaSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_ETHERADDR_PTR_addressPtr,
    &DX_IN_GT_ETHERADDR_PTR_maskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupMacModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_TTI_MAC_MODE_ENT_macMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupUserDefinedByteSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_GT_U32_udbIndex,
    &DX_IN_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_offsetType,
    &DX_IN_GT_U8_offset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupUserDefinedByteGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_GT_U32_udbIndex,
    &DX_OUT_CPSS_DXCH_TTI_OFFSET_TYPE_ENT_PTR_offsetTypePtr,
    &DX_OUT_GT_U8_PTR_offsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupMacModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_OUT_CPSS_DXCH_TTI_MAC_MODE_ENT_PTR_macModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupMacToMeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_valuePtr,
    &DX_IN_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_maskPtr,
    &DX_IN_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupMacToMeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_valuePtr,
    &DX_OUT_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_maskPtr,
    &DX_OUT_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupRuleActionUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupRuleSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TTI_RULE_TYPE_ENT_ruleType,
    &DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_patternPtr,
    &DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_maskPtr,
    &DX_IN_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortGroupRuleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TTI_RULE_TYPE_ENT_ruleType,
    &DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_patternPtr,
    &DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_maskPtr,
    &DX_OUT_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortLookupEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortLookupEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortPclIdModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TTI_PCL_ID_MODE_ENT_pclIdMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortTcamProfileIdModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT_tcamProfileIdMode,
    &DX_IN_GT_U32_tcamProfileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortPassengerOuterIsTag0Or1Set_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_ETHER_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMcTunnelDuplicationModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_TUNNEL_MULTICAST_TYPE_ENT_protocol,
    &DX_IN_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMcTunnelDuplicationModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_TUNNEL_MULTICAST_TYPE_ENT_protocol,
    &DX_OUT_CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortPclIdModePortSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_pclId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortPclIdModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_TTI_PCL_ID_MODE_ENT_PTR_pclIdModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortTcamProfileIdModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_TTI_TCAM_PROFILE_ID_MODE_ENT_PTR_tcamProfileIdModePtr,
    &DX_OUT_GT_U32_PTR_tcamProfileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortPassengerOuterIsTag0Or1Get_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_ETHER_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPortPclIdModePortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_pclIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMcTunnelDuplicationUdpDestPortSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_udpPort
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiFcoeEtherTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_HEX_etherType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPwCwCpuCodeBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cpuCodeBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiIpToMeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR_valuePtr,
    &DX_IN_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMacToMeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_valuePtr,
    &DX_IN_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_maskPtr,
    &DX_IN_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiIpToMeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_DXCH_TTI_DIP_VLAN_STC_PTR_valuePtr,
    &DX_OUT_CPSS_DXCH_TTI_IP_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMacToMeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_valuePtr,
    &DX_OUT_CPSS_DXCH_TTI_MAC_VLAN_STC_PTR_maskPtr,
    &DX_OUT_CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC_PTR_interfaceInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiIPv6ExtensionHeaderSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_extensionHeaderId,
    &DX_IN_GT_U32_extensionHeaderValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiIPv6ExtensionHeaderGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_extensionHeaderId,
    &DX_OUT_GT_U32_PTR_extensionHeaderValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiRuleActionUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiRuleSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TTI_RULE_TYPE_ENT_ruleType,
    &DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_patternPtr,
    &DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_maskPtr,
    &DX_IN_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiRuleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TTI_RULE_TYPE_ENT_ruleType,
    &DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_patternPtr,
    &DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_maskPtr,
    &DX_OUT_CPSS_DXCH_TTI_ACTION_STC_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiRuleValidStatusSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_BOOL_valid
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiRuleValidStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_validPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiGenericActionModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lookup,
    &DX_IN_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT_genericActionMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiSourceIdBitsOverrideSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lookup,
    &DX_IN_GT_U32_overrideBitmap
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiGenericActionModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lookup,
    &DX_OUT_CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT_PTR_genericActionModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiSourceIdBitsOverrideGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lookup,
    &DX_OUT_GT_U32_PTR_overrideBitmapPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiTunnelHeaderLengthProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIdx,
    &DX_IN_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR_tunnelHeaderLengthProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiTunnelHeaderLengthProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIdx,
    &DX_OUT_CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC_PTR_tunnelHeaderLengthProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMplsMcTunnelTriggeringMacDaGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_ETHERADDR_PTR_addressPtr,
    &DX_OUT_GT_ETHERADDR_PTR_maskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiMcTunnelDuplicationUdpDestPortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U16_PTR_udpPortPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiFcoeEtherTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U16_HEX_PTR_etherTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTtiPwCwCpuCodeBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_cpuCodeBasePtr
};


/********* lib API DB *********/

extern void cpssDxChTtiRuleSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTtiRuleSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTtiRuleActionUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTtiPortGroupRuleSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTtiPortGroupRuleSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTtiPortGroupRuleActionUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTtiLogLibDb[] = {
    {"cpssDxChTtiMacToMeSet", 5, cpssDxChTtiMacToMeSet_PARAMS, NULL},
    {"cpssDxChTtiMacToMeGet", 5, cpssDxChTtiMacToMeGet_PARAMS, NULL},
    {"cpssDxChTtiPortLookupEnableSet", 4, cpssDxChTtiPortLookupEnableSet_PARAMS, NULL},
    {"cpssDxChTtiPortLookupEnableGet", 4, cpssDxChTtiPortLookupEnableGet_PARAMS, NULL},
    {"cpssDxChTtiPortIpv4OnlyTunneledEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChTtiPortIpv4OnlyTunneledEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChTtiPortIpv4OnlyMacToMeEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChTtiPortIpv4OnlyMacToMeEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChTtiIpv4McEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTtiIpv4McEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTtiPortMplsOnlyMacToMeEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChTtiPortMplsOnlyMacToMeEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChTtiPortMimOnlyMacToMeEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChTtiPortMimOnlyMacToMeEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChTtiRuleSet", 6, cpssDxChTtiRuleSet_PARAMS, cpssDxChTtiRuleSet_preLogic},
    {"cpssDxChTtiRuleGet", 6, cpssDxChTtiRuleGet_PARAMS, cpssDxChTtiRuleSet_preLogic},
    {"cpssDxChTtiRuleActionUpdate", 3, cpssDxChTtiRuleActionUpdate_PARAMS, cpssDxChTtiRuleActionUpdate_preLogic},
    {"cpssDxChTtiRuleValidStatusSet", 3, cpssDxChTtiRuleValidStatusSet_PARAMS, NULL},
    {"cpssDxChTtiRuleValidStatusGet", 3, cpssDxChTtiRuleValidStatusGet_PARAMS, NULL},
    {"cpssDxChTtiMacModeSet", 3, cpssDxChTtiMacModeSet_PARAMS, NULL},
    {"cpssDxChTtiMacModeGet", 3, cpssDxChTtiMacModeGet_PARAMS, NULL},
    {"cpssDxChTtiPclIdSet", 3, cpssDxChTtiPclIdSet_PARAMS, NULL},
    {"cpssDxChTtiPclIdGet", 3, cpssDxChTtiPclIdGet_PARAMS, NULL},
    {"cpssDxChTtiPortPclIdModeSet", 3, cpssDxChTtiPortPclIdModeSet_PARAMS, NULL},
    {"cpssDxChTtiPortPclIdModeGet", 3, cpssDxChTtiPortPclIdModeGet_PARAMS, NULL},
    {"cpssDxChTtiPortPclIdModePortSet", 3, cpssDxChTtiPortPclIdModePortSet_PARAMS, NULL},
    {"cpssDxChTtiPortPclIdModePortGet", 3, cpssDxChTtiPortPclIdModePortGet_PARAMS, NULL},
    {"cpssDxChTtiExceptionCmdSet", 3, cpssDxChTtiExceptionCmdSet_PARAMS, NULL},
    {"cpssDxChTtiExceptionCmdGet", 3, cpssDxChTtiExceptionCmdGet_PARAMS, NULL},
    {"cpssDxChTtiExceptionCpuCodeSet", 3, cpssDxChTtiExceptionCpuCodeSet_PARAMS, NULL},
    {"cpssDxChTtiExceptionCpuCodeGet", 3, cpssDxChTtiExceptionCpuCodeGet_PARAMS, NULL},
    {"cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTtiPortIpTotalLengthDeductionEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChTtiPortIpTotalLengthDeductionEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChTtiIpTotalLengthDeductionValueSet", 3, cpssDxChTtiIpTotalLengthDeductionValueSet_PARAMS, NULL},
    {"cpssDxChTtiIpTotalLengthDeductionValueGet", 3, cpssDxChTtiIpTotalLengthDeductionValueGet_PARAMS, NULL},
    {"cpssDxChTtiEthernetTypeSet", 3, cpssDxChTtiEthernetTypeSet_PARAMS, NULL},
    {"cpssDxChTtiEthernetTypeGet", 3, cpssDxChTtiEthernetTypeGet_PARAMS, NULL},
    {"cpssDxChTtiPortGroupMacToMeSet", 6, cpssDxChTtiPortGroupMacToMeSet_PARAMS, NULL},
    {"cpssDxChTtiPortGroupMacToMeGet", 6, cpssDxChTtiPortGroupMacToMeGet_PARAMS, NULL},
    {"cpssDxChTtiPortGroupMacModeSet", 4, cpssDxChTtiPortGroupMacModeSet_PARAMS, NULL},
    {"cpssDxChTtiPortGroupMacModeGet", 4, cpssDxChTtiPortGroupMacModeGet_PARAMS, NULL},
    {"cpssDxChTtiPortGroupRuleSet", 7, cpssDxChTtiPortGroupRuleSet_PARAMS, cpssDxChTtiPortGroupRuleSet_preLogic},
    {"cpssDxChTtiPortGroupRuleGet", 7, cpssDxChTtiPortGroupRuleGet_PARAMS, cpssDxChTtiPortGroupRuleSet_preLogic},
    {"cpssDxChTtiPortGroupRuleActionUpdate", 4, cpssDxChTtiPortGroupRuleActionUpdate_PARAMS, cpssDxChTtiPortGroupRuleActionUpdate_preLogic},
    {"cpssDxChTtiPortGroupRuleValidStatusSet", 4, prvCpssLogGenDevNumPortGroupsBmpIndexValid_PARAMS, NULL},
    {"cpssDxChTtiPortGroupRuleValidStatusGet", 4, prvCpssLogGenDevNumPortGroupsBmpIndexValidPtr_PARAMS, NULL},
    {"cpssDxChTtiPortGroupUserDefinedByteSet", 6, cpssDxChTtiPortGroupUserDefinedByteSet_PARAMS, NULL},
    {"cpssDxChTtiPortGroupUserDefinedByteGet", 6, cpssDxChTtiPortGroupUserDefinedByteGet_PARAMS, NULL},
    {"cpssDxChTtiPortPassengerOuterIsTag0Or1Set", 3, cpssDxChTtiPortPassengerOuterIsTag0Or1Set_PARAMS, NULL},
    {"cpssDxChTtiPortPassengerOuterIsTag0Or1Get", 3, cpssDxChTtiPortPassengerOuterIsTag0Or1Get_PARAMS, NULL},
    {"cpssDxChTtiPacketTypeKeySizeSet", 3, cpssDxChTtiPacketTypeKeySizeSet_PARAMS, NULL},
    {"cpssDxChTtiPacketTypeKeySizeGet", 3, cpssDxChTtiPacketTypeKeySizeGet_PARAMS, NULL},
    {"cpssDxChTtiTcamSegmentModeSet", 3, cpssDxChTtiTcamSegmentModeSet_PARAMS, NULL},
    {"cpssDxChTtiTcamSegmentModeGet", 3, cpssDxChTtiTcamSegmentModeGet_PARAMS, NULL},
    {"cpssDxChTtiMcTunnelDuplicationModeSet", 4, cpssDxChTtiMcTunnelDuplicationModeSet_PARAMS, NULL},
    {"cpssDxChTtiMcTunnelDuplicationModeGet", 4, cpssDxChTtiMcTunnelDuplicationModeGet_PARAMS, NULL},
    {"cpssDxChTtiMcTunnelDuplicationUdpDestPortSet", 2, cpssDxChTtiMcTunnelDuplicationUdpDestPortSet_PARAMS, NULL},
    {"cpssDxChTtiMcTunnelDuplicationUdpDestPortGet", 2, cpssDxChTtiMcTunnelDuplicationUdpDestPortGet_PARAMS, NULL},
    {"cpssDxChTtiMplsMcTunnelTriggeringMacDaSet", 3, cpssDxChTtiMplsMcTunnelTriggeringMacDaSet_PARAMS, NULL},
    {"cpssDxChTtiMplsMcTunnelTriggeringMacDaGet", 3, cpssDxChTtiMplsMcTunnelTriggeringMacDaGet_PARAMS, NULL},
    {"cpssDxChTtiPwCwExceptionCmdSet", 3, cpssDxChTtiPwCwExceptionCmdSet_PARAMS, NULL},
    {"cpssDxChTtiPwCwExceptionCmdGet", 3, cpssDxChTtiPwCwExceptionCmdGet_PARAMS, NULL},
    {"cpssDxChTtiPwCwCpuCodeBaseSet", 2, cpssDxChTtiPwCwCpuCodeBaseSet_PARAMS, NULL},
    {"cpssDxChTtiPwCwCpuCodeBaseGet", 2, cpssDxChTtiPwCwCpuCodeBaseGet_PARAMS, NULL},
    {"cpssDxChTtiPwCwSequencingSupportEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTtiPwCwSequencingSupportEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTtiIPv6ExtensionHeaderSet", 3, cpssDxChTtiIPv6ExtensionHeaderSet_PARAMS, NULL},
    {"cpssDxChTtiIPv6ExtensionHeaderGet", 3, cpssDxChTtiIPv6ExtensionHeaderGet_PARAMS, NULL},
    {"cpssDxChTtiSourceIdBitsOverrideSet", 3, cpssDxChTtiSourceIdBitsOverrideSet_PARAMS, NULL},
    {"cpssDxChTtiSourceIdBitsOverrideGet", 3, cpssDxChTtiSourceIdBitsOverrideGet_PARAMS, NULL},
    {"cpssDxChTtiGreExtensionsCheckEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTtiGreExtensionsCheckEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTtiUserDefinedByteSet", 5, cpssDxChTtiUserDefinedByteSet_PARAMS, NULL},
    {"cpssDxChTtiUserDefinedByteGet", 5, cpssDxChTtiUserDefinedByteGet_PARAMS, NULL},
    {"cpssDxChTtiFcoeForwardingEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTtiFcoeForwardingEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTtiFcoeEtherTypeSet", 2, cpssDxChTtiFcoeEtherTypeSet_PARAMS, NULL},
    {"cpssDxChTtiFcoeEtherTypeGet", 2, cpssDxChTtiFcoeEtherTypeGet_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionConfiguratiosEnableSet", 3, cpssDxChTtiFcoeExceptionConfiguratiosEnableSet_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionConfiguratiosEnableGet", 3, cpssDxChTtiFcoeExceptionConfiguratiosEnableGet_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionCountersGet", 3, cpssDxChTtiFcoeExceptionCountersGet_PARAMS, NULL},
    {"cpssDxChTtiFcoeAssignVfIdEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChTtiFcoeAssignVfIdEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionPacketCommandSet", 2, prvCpssLogGenDevNumCommand_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionPacketCommandGet", 2, prvCpssLogGenDevNumCommandPtr_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionCpuCodeSet", 2, prvCpssLogGenDevNumCpuCode_PARAMS, NULL},
    {"cpssDxChTtiFcoeExceptionCpuCodeGet", 2, prvCpssLogGenDevNumCpuCodePtr_PARAMS, NULL},
    {"cpssDxChTtiEcnPacketCommandSet", 2, prvCpssLogGenDevNumCommand_PARAMS, NULL},
    {"cpssDxChTtiEcnPacketCommandGet", 2, prvCpssLogGenDevNumCommandPtr_PARAMS, NULL},
    {"cpssDxChTtiEcnCpuCodeSet", 2, prvCpssLogGenDevNumCpuCode_PARAMS, NULL},
    {"cpssDxChTtiEcnCpuCodeGet", 2, prvCpssLogGenDevNumCpuCodePtr_PARAMS, NULL},
    {"cpssDxChTtiTunnelHeaderLengthProfileSet", 3, cpssDxChTtiTunnelHeaderLengthProfileSet_PARAMS, NULL},
    {"cpssDxChTtiTunnelHeaderLengthProfileGet", 3, cpssDxChTtiTunnelHeaderLengthProfileGet_PARAMS, NULL},
    {"cpssDxChTtiPortTcamProfileIdModeSet", 4, cpssDxChTtiPortTcamProfileIdModeSet_PARAMS, NULL},
    {"cpssDxChTtiPortTcamProfileIdModeGet", 4, cpssDxChTtiPortTcamProfileIdModeGet_PARAMS, NULL},
    {"cpssDxChTtiPacketTypeTcamProfileIdSet", 3, cpssDxChTtiPacketTypeTcamProfileIdSet_PARAMS, NULL},
    {"cpssDxChTtiPacketTypeTcamProfileIdGet", 3, cpssDxChTtiPacketTypeTcamProfileIdGet_PARAMS, NULL},
    {"cpssDxChTtiPortAndPacketTypeTcamProfileIdSet", 4, cpssDxChTtiPortAndPacketTypeTcamProfileIdSet_PARAMS, NULL},
    {"cpssDxChTtiPortAndPacketTypeTcamProfileIdGet", 4, cpssDxChTtiPortAndPacketTypeTcamProfileIdGet_PARAMS, NULL},
    {"cpssDxChTtiIpToMeSet", 4, cpssDxChTtiIpToMeSet_PARAMS, NULL},
    {"cpssDxChTtiIpToMeGet", 4, cpssDxChTtiIpToMeGet_PARAMS, NULL},
    {"cpssDxChTtiGenericActionModeSet", 3, cpssDxChTtiGenericActionModeSet_PARAMS, NULL},
    {"cpssDxChTtiGenericActionModeGet", 3, cpssDxChTtiGenericActionModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TTI(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTtiLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTtiLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

