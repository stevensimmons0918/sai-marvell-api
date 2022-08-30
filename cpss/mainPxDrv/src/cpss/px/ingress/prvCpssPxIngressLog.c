/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxIngressLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/generic/log/prvCpssPxGenLog.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/ingress/private/prvCpssPxIngressLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_INGRESS_ETHERTYPE_ENT[]  =
{
    "CPSS_PX_INGRESS_ETHERTYPE_IPV4_E",
    "CPSS_PX_INGRESS_ETHERTYPE_IPV6_E",
    "CPSS_PX_INGRESS_ETHERTYPE_MPLS1_E",
    "CPSS_PX_INGRESS_ETHERTYPE_MPLS2_E",
    "CPSS_PX_INGRESS_ETHERTYPE_IPV6_EH_E",
    "CPSS_PX_INGRESS_ETHERTYPE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_ETHERTYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT[]  =
{
    "CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E",
    "CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_INGRESS_HASH_MODE_ENT[]  =
{
    "CPSS_PX_INGRESS_HASH_MODE_CRC32_E",
    "CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_HASH_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT[]  =
{
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_SINGLE_LABEL_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_MULTI_LABEL_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E",
    "CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT[]  =
{
    "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E",
    "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT[]  =
{
    "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E",
    "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E",
    "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT[]  =
{
    "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E",
    "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E",
    "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E",
    "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_HASH_UDBP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, anchor, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, offset);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, nibbleMaskArr, 4, GT_BOOL);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_IP2ME_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, valid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prefixLength);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isIpv6);
    prvCpssLogStcLogStart(contextLib,  logType, "ipAddr");
    PRV_CPSS_LOG_STC_IPV4_MAC((&valPtr->ipAddr), ipv4Addr);
    PRV_CPSS_LOG_STC_IPV6_MAC((&valPtr->ipAddr), ipv6Addr);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_16_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isLLCNonSnap);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, profileIndex);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, udbPairsArr, CPSS_PX_UDB_PAIRS_MAX_CNS, CPSS_PX_UDB_PAIR_DATA_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ip2meIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bitFieldArr, CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_MAX_CNS, CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, indexConst);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, indexMax);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortProfile);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, portUdbPairArr, CPSS_PX_UDB_PAIRS_MAX_CNS, CPSS_PX_UDB_PAIR_KEY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_TPID_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, val);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, size);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, valid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, byteOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, startBit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numBits);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_UDB_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_UDB_KEY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udbByteOffset);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_UDB_PAIR_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_UDB_PAIR_DATA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, udb, 2, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_UDB_PAIR_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_UDB_PAIR_KEY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udbByteOffset);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_INGRESS_ETHERTYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_ETHERTYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_INGRESS_ETHERTYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_INGRESS_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_INGRESS_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INGRESS_HASH_UDBP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INGRESS_IP2ME_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INGRESS_TPID_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIRECTION_ENT_direction = {
     "direction", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_ETHERTYPE_ENT_configType = {
     "configType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_INGRESS_ETHERTYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_lagTableMode = {
     "lagTableMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_MODE_ENT_hashMode = {
     "hashMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_INGRESS_HASH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType = {
     "hashPacketType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_indexMode = {
     "indexMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR_udbpArr = {
     "udbpArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_HASH_UDBP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR_ip2meEntryPtr = {
     "ip2meEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_IP2ME_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr = {
     "keyDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr = {
     "keyMaskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR_packetTypeFormatPtr = {
     "packetTypeFormatPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType = {
     "tableType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR_portKeyPtr = {
     "portKeyPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR_tpidEntryPtr = {
     "tpidEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_TPID_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORTS_BMP_targetPortsBmp = {
     "targetPortsBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORTS_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_bypassLagDesignatedBitmap = {
     "bypassLagDesignatedBitmap", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_sourcePortEnable = {
     "sourcePortEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_valid = {
     "valid", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U16_HEX_etherType = {
     "etherType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U16_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_bitOffsetsArr = {
     "bitOffsetsArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_crc32Seed = {
     "crc32Seed", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_entryIndex = {
     "entryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_lagTableNumber = {
     "lagTableNumber", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_offset = {
     "offset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_PTR_lagTableModePtr = {
     "lagTableModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_MODE_ENT_PTR_hashModePtr = {
     "hashModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_HASH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_PTR_indexModePtr = {
     "indexModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR_udbpArr = {
     "udbpArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_HASH_UDBP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR_ip2meEntryPtr = {
     "ip2meEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_IP2ME_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_errorKeyPtr = {
     "errorKeyPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr = {
     "keyDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr = {
     "keyMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR_packetTypeFormatPtr = {
     "packetTypeFormatPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR_portKeyPtr = {
     "portKeyPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR_tpidEntryPtr = {
     "tpidEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INGRESS_TPID_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_bypassLagDesignatedBitmapPtr = {
     "bypassLagDesignatedBitmapPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_sourcePortEnablePtr = {
     "sourcePortEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U16_HEX_PTR_etherTypePtr = {
     "etherTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U16_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_bitOffsetsArr = {
     "bitOffsetsArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_crc32SeedPtr = {
     "crc32SeedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_lagTableNumberPtr = {
     "lagTableNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_offsetPtr = {
     "offsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressFilteringEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_DIRECTION_ENT_direction,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressFilteringEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_DIRECTION_ENT_direction,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressEtherTypeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_ETHERTYPE_ENT_configType,
    &PX_IN_GT_U16_HEX_etherType,
    &PX_IN_GT_BOOL_valid
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressEtherTypeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_ETHERTYPE_ENT_configType,
    &PX_OUT_GT_U16_HEX_PTR_etherTypePtr,
    &PX_OUT_GT_BOOL_PTR_validPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashLagTableModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_lagTableMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPacketTypeHashModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType,
    &PX_IN_CPSS_PX_INGRESS_HASH_MODE_ENT_hashMode,
    &PX_IN_GT_U32_PTR_bitOffsetsArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPacketTypeEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType,
    &PX_IN_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR_udbpArr,
    &PX_IN_GT_BOOL_sourcePortEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashUdeEthertypeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType,
    &PX_IN_GT_U16_HEX_etherType,
    &PX_IN_GT_BOOL_valid
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPacketTypeHashModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType,
    &PX_OUT_CPSS_PX_INGRESS_HASH_MODE_ENT_PTR_hashModePtr,
    &PX_OUT_GT_U32_PTR_bitOffsetsArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPacketTypeEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType,
    &PX_OUT_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR_udbpArr,
    &PX_OUT_GT_BOOL_PTR_sourcePortEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashUdeEthertypeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType,
    &PX_OUT_GT_U16_HEX_PTR_etherTypePtr,
    &PX_OUT_GT_BOOL_PTR_validPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMapPacketTypeFormatEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR_packetTypeFormatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMapPacketTypeFormatEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR_packetTypeFormatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressForwardingPortMapExceptionSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_IN_CPSS_PX_PORTS_BMP_portsBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMapEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_IN_GT_U32_entryIndex,
    &PX_IN_CPSS_PX_PORTS_BMP_portsBmp,
    &PX_IN_GT_BOOL_bypassLagDesignatedBitmap
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMapEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_IN_GT_U32_entryIndex,
    &PX_OUT_CPSS_PX_PORTS_BMP_PTR_portsBmpPtr,
    &PX_OUT_GT_BOOL_PTR_bypassLagDesignatedBitmapPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressForwardingPortMapExceptionGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_OUT_CPSS_PX_PORTS_BMP_PTR_portsBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMapExceptionCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType,
    &PX_OUT_GT_U32_PTR_errorCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPacketTypeKeyEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr,
    &PX_IN_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPacketTypeKeyEntryEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPacketTypeLagTableNumberSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_GT_U32_lagTableNumber
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPacketTypeKeyEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr,
    &PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPacketTypeKeyEntryEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPacketTypeLagTableNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_GT_U32_PTR_lagTableNumberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPortIndexModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_indexMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortPacketTypeKeySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR_portKeyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortRedirectSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORTS_BMP_targetPortsBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMacDaByteOffsetSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_offset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashPortIndexModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_PTR_indexModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortPacketTypeKeyGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR_portKeyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPortMacDaByteOffsetGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_offsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashSeedSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_crc32Seed
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressIp2MeEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_entryIndex,
    &PX_IN_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR_ip2meEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressTpidEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_entryIndex,
    &PX_IN_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR_tpidEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashDesignatedPortsEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_entryIndex,
    &PX_IN_CPSS_PX_PORTS_BMP_portsBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressIp2MeEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_entryIndex,
    &PX_OUT_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR_ip2meEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressTpidEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_entryIndex,
    &PX_OUT_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR_tpidEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashDesignatedPortsEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_entryIndex,
    &PX_OUT_CPSS_PX_PORTS_BMP_PTR_portsBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashLagTableModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_PTR_lagTableModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressHashSeedGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_crc32SeedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxIngressPacketTypeErrorGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_errorCounterPtr,
    &PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_errorKeyPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxIngressLogLibDb[] = {
    {"cpssPxIngressTpidEntrySet", 3, cpssPxIngressTpidEntrySet_PARAMS, NULL},
    {"cpssPxIngressTpidEntryGet", 3, cpssPxIngressTpidEntryGet_PARAMS, NULL},
    {"cpssPxIngressEtherTypeSet", 4, cpssPxIngressEtherTypeSet_PARAMS, NULL},
    {"cpssPxIngressEtherTypeGet", 4, cpssPxIngressEtherTypeGet_PARAMS, NULL},
    {"cpssPxIngressPortMacDaByteOffsetSet", 3, cpssPxIngressPortMacDaByteOffsetSet_PARAMS, NULL},
    {"cpssPxIngressPortMacDaByteOffsetGet", 3, cpssPxIngressPortMacDaByteOffsetGet_PARAMS, NULL},
    {"cpssPxIngressPortPacketTypeKeySet", 3, cpssPxIngressPortPacketTypeKeySet_PARAMS, NULL},
    {"cpssPxIngressPortPacketTypeKeyGet", 3, cpssPxIngressPortPacketTypeKeyGet_PARAMS, NULL},
    {"cpssPxIngressPacketTypeKeyEntrySet", 4, cpssPxIngressPacketTypeKeyEntrySet_PARAMS, NULL},
    {"cpssPxIngressPacketTypeKeyEntryGet", 4, cpssPxIngressPacketTypeKeyEntryGet_PARAMS, NULL},
    {"cpssPxIngressPortMapPacketTypeFormatEntrySet", 4, cpssPxIngressPortMapPacketTypeFormatEntrySet_PARAMS, NULL},
    {"cpssPxIngressPortMapPacketTypeFormatEntryGet", 4, cpssPxIngressPortMapPacketTypeFormatEntryGet_PARAMS, NULL},
    {"cpssPxIngressPortMapEntrySet", 5, cpssPxIngressPortMapEntrySet_PARAMS, NULL},
    {"cpssPxIngressPortMapEntryGet", 5, cpssPxIngressPortMapEntryGet_PARAMS, NULL},
    {"cpssPxIngressPortTargetEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxIngressPortTargetEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxIngressPacketTypeErrorGet", 3, cpssPxIngressPacketTypeErrorGet_PARAMS, NULL},
    {"cpssPxIngressForwardingPortMapExceptionSet", 3, cpssPxIngressForwardingPortMapExceptionSet_PARAMS, NULL},
    {"cpssPxIngressForwardingPortMapExceptionGet", 3, cpssPxIngressForwardingPortMapExceptionGet_PARAMS, NULL},
    {"cpssPxIngressPortMapExceptionCounterGet", 3, cpssPxIngressPortMapExceptionCounterGet_PARAMS, NULL},
    {"cpssPxIngressPacketTypeKeyEntryEnableSet", 3, cpssPxIngressPacketTypeKeyEntryEnableSet_PARAMS, NULL},
    {"cpssPxIngressPacketTypeKeyEntryEnableGet", 3, cpssPxIngressPacketTypeKeyEntryEnableGet_PARAMS, NULL},
    {"cpssPxIngressPortDsaTagEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxIngressPortDsaTagEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxIngressPortRedirectSet", 3, cpssPxIngressPortRedirectSet_PARAMS, NULL},
    {"cpssPxIngressFilteringEnableSet", 4, cpssPxIngressFilteringEnableSet_PARAMS, NULL},
    {"cpssPxIngressFilteringEnableGet", 4, cpssPxIngressFilteringEnableGet_PARAMS, NULL},
    {"cpssPxIngressIp2MeEntrySet", 3, cpssPxIngressIp2MeEntrySet_PARAMS, NULL},
    {"cpssPxIngressIp2MeEntryGet", 3, cpssPxIngressIp2MeEntryGet_PARAMS, NULL},
    {"cpssPxIngressHashUdeEthertypeSet", 4, cpssPxIngressHashUdeEthertypeSet_PARAMS, NULL},
    {"cpssPxIngressHashUdeEthertypeGet", 4, cpssPxIngressHashUdeEthertypeGet_PARAMS, NULL},
    {"cpssPxIngressHashPacketTypeEntrySet", 4, cpssPxIngressHashPacketTypeEntrySet_PARAMS, NULL},
    {"cpssPxIngressHashPacketTypeEntryGet", 4, cpssPxIngressHashPacketTypeEntryGet_PARAMS, NULL},
    {"cpssPxIngressHashPacketTypeHashModeSet", 4, cpssPxIngressHashPacketTypeHashModeSet_PARAMS, NULL},
    {"cpssPxIngressHashPacketTypeHashModeGet", 4, cpssPxIngressHashPacketTypeHashModeGet_PARAMS, NULL},
    {"cpssPxIngressHashSeedSet", 2, cpssPxIngressHashSeedSet_PARAMS, NULL},
    {"cpssPxIngressHashSeedGet", 2, cpssPxIngressHashSeedGet_PARAMS, NULL},
    {"cpssPxIngressHashPortIndexModeSet", 3, cpssPxIngressHashPortIndexModeSet_PARAMS, NULL},
    {"cpssPxIngressHashPortIndexModeGet", 3, cpssPxIngressHashPortIndexModeGet_PARAMS, NULL},
    {"cpssPxIngressHashDesignatedPortsEntrySet", 3, cpssPxIngressHashDesignatedPortsEntrySet_PARAMS, NULL},
    {"cpssPxIngressHashDesignatedPortsEntryGet", 3, cpssPxIngressHashDesignatedPortsEntryGet_PARAMS, NULL},
    {"cpssPxIngressHashLagTableModeSet", 2, cpssPxIngressHashLagTableModeSet_PARAMS, NULL},
    {"cpssPxIngressHashLagTableModeGet", 2, cpssPxIngressHashLagTableModeGet_PARAMS, NULL},
    {"cpssPxIngressHashPacketTypeLagTableNumberSet", 3, cpssPxIngressHashPacketTypeLagTableNumberSet_PARAMS, NULL},
    {"cpssPxIngressHashPacketTypeLagTableNumberGet", 3, cpssPxIngressHashPacketTypeLagTableNumberGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_INGRESS(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxIngressLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxIngressLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

