/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTrunkLog.c
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
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunkLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/trunk/private/prvCpssGenTrunkLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT[]  =
{
    "CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_MODULO_8_E",
    "CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_MODULO_64_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_CNC_E",
    "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE___LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E",
    "CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E",
    "CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E",
    "CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E",
    "CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_HASH_MODE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_HASH_MODE_CRC32_E",
    "CPSS_DXCH_TRUNK_HASH_MODE_EXTRACT_FROM_HASH_KEY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_HASH_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E",
    "CPSS_DXCH_TRUNK_IPV6_HASH_MSB_SIP_DIP_FLOW_E",
    "CPSS_DXCH_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E",
    "CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E",
    "CPSS_DXCH_TRUNK_L4_LBH_LONG_E",
    "CPSS_DXCH_TRUNK_L4_LBH_SHORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E",
    "CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E",
    "CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E",
    "CPSS_DXCH_TRUNK_LBH_CRC_32_FROM_HASH0_HASH1_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E",
    "CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E",
    "CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_LBH_MASK_ENT[]  =
{
    "CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_MAC_SA_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL1_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL2_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_IPV4_SIP_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_IPV6_DIP_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_IPV6_SIP_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_L4_DST_PORT_E",
    "CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_LBH_MASK_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT[]  =
{
    "CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E",
    "CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E",
    "CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trunkPort);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ecmpStartIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ecmpNumOfPaths);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ecmpEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ecmpRandomPathEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hashBitSelectionProfile);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, l4DstPortMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, l4SrcPortMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ipv6FlowMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ipDipMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ipSipMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, macDaMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, macSaMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, mplsLabel0MaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, mplsLabel1MaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, mplsLabel2MaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, localSrcPortMaskBmp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, udbsMaskBmp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, symmetricMacAddrEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, symmetricIpv4AddrEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, symmetricIpv6AddrEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, symmetricL4PortEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, l4DstPortArray, 2, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, l4SrcPortArray, 2, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ipv6FlowArray, 3, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ipDipArray, 16, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ipSipArray, 16, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, macDaArray, 6, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, macSaArray, 6, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mplsLabel0Array, 3, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mplsLabel1Array, 3, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mplsLabel2Array, 3, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, localSrcPortArray, 1, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, udbsArray, 32, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_HASH_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_HASH_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_MASK_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_LBH_MASK_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_LBH_MASK_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_TRUNK_ID_PTR_trunkIdPtr = {
     "trunkIdPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_TRUNK_ID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfDisabledMembersPtr = {
     "numOfDisabledMembersPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfEnabledMembersPtr = {
     "numOfEnabledMembersPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfMembersPtr = {
     "numOfMembersPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr = {
     "ecmpLttInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashClient = {
     "hashClient", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashEntityType = {
     "hashEntityType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT_fieldType = {
     "fieldType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_MODE_ENT_hashMode = {
     "hashMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_HASH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_hashMode = {
     "hashMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_hashMode = {
     "hashMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_crcMode = {
     "crcMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_hashMode = {
     "hashMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR_hashInputDataPtr = {
     "hashInputDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_MASK_ENT_maskedField = {
     "maskedField", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_LBH_MASK_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_trunkMembersMode = {
     "trunkMembersMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_designatedPortsPtr = {
     "designatedPortsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsBmpPtr = {
     "nonTrunkPortsBmpPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsPtr = {
     "nonTrunkPortsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_disabledMembersArray = {
     "disabledMembersArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_enabledMembersArray = {
     "enabledMembersArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr = {
     "memberPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_membersArray = {
     "membersArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_manageMode = {
     "manageMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR_sharedPortInfoPtr = {
     "sharedPortInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_SHARED_PORT_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR_weightedMembersArray = {
     "weightedMembersArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_WEIGHTED_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isSrcIp = {
     "isSrcIp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_memberOfTrunk = {
     "memberOfTrunk", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_keyBitOffsetArr = {
     "keyBitOffsetArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_byteIndex = {
     "byteIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_crcSeed = {
     "crcSeed", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_crcSeedHash1 = {
     "crcSeedHash1", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hashIndex = {
     "hashIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_l2EcmpStartIndex = {
     "l2EcmpStartIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxNumOfMembers = {
     "maxNumOfMembers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxNumberOfTrunks = {
     "maxNumberOfTrunks", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numMembers = {
     "numMembers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfDisabledMembers = {
     "numOfDisabledMembers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfEnabledMembers = {
     "numOfEnabledMembers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfMembers = {
     "numOfMembers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfMembers = {
     "numberOfMembers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_shiftValue = {
     "shiftValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_maskValue = {
     "maskValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_saltValue = {
     "saltValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr = {
     "ecmpLttInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_HASH_MODE_ENT_PTR_hashModePtr = {
     "hashModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_HASH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_PTR_hashModePtr = {
     "hashModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_PTR_hashModePtr = {
     "hashModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_PTR_crcModePtr = {
     "crcModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_PTR_hashModePtr = {
     "hashModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_PTR_trunkMembersModePtr = {
     "trunkMembersModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_designatedPortsPtr = {
     "designatedPortsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsPtr = {
     "nonTrunkPortsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_disabledMembersArray = {
     "disabledMembersArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_enabledMembersArray = {
     "enabledMembersArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr = {
     "memberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_membersArray = {
     "membersArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_PTR_manageModePtr = {
     "manageModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR_sharedPortInfoPtr = {
     "sharedPortInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_SHARED_PORT_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_TYPE_ENT_PTR_typePtr = {
     "typePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR_weightedMembersArray = {
     "weightedMembersArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TRUNK_WEIGHTED_MEMBER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_memberOfTrunkPtr = {
     "memberOfTrunkPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_overridePtr = {
     "overridePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_crcSeedHash1Ptr = {
     "crcSeedHash1Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_crcSeedPtr = {
     "crcSeedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashIndexPtr = {
     "hashIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_keyBitOffsetArr = {
     "keyBitOffsetArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_l2EcmpStartIndexPtr = {
     "l2EcmpStartIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxNumOfMembersPtr = {
     "maxNumOfMembersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxNumberOfTrunksPtr = {
     "maxNumberOfTrunksPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numMembersPtr = {
     "numMembersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_shiftValuePtr = {
     "shiftValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_maskValuePtr = {
     "maskValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_saltValuePtr = {
     "saltValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashPacketTypeHashModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_IN_CPSS_DXCH_TRUNK_HASH_MODE_ENT_hashMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashIndexCalculate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_IN_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR_hashInputDataPtr,
    &DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashEntityType,
    &DX_IN_GT_U32_numberOfMembers,
    &DX_OUT_GT_U32_PTR_hashIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashPacketTypeHashModeBitOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_IN_GT_U32_PTR_keyBitOffsetArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashPacketTypeHashModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_OUT_CPSS_DXCH_TRUNK_HASH_MODE_ENT_PTR_hashModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashPacketTypeHashModeBitOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_OUT_GT_U32_PTR_keyBitOffsetArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashDesignatedTableModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashNumBitsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashClient,
    &DX_IN_GT_U32_startBit,
    &DX_IN_GT_U32_numOfBits
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashNumBitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashClient,
    &DX_OUT_GT_U32_PTR_startBitPtr,
    &DX_OUT_GT_U32_PTR_numOfBitsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashMaskCrcParamOverrideSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT_fieldType,
    &DX_IN_GT_BOOL_override
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashMaskCrcParamOverrideGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT_fieldType,
    &DX_OUT_GT_BOOL_PTR_overridePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashIpv6ModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_hashMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashL4ModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_hashMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashCrcParametersSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_crcMode,
    &DX_IN_GT_U32_crcSeed,
    &DX_IN_GT_U32_crcSeedHash1
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashGlobalModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_hashMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashMaskSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_LBH_MASK_ENT_maskedField,
    &DX_IN_GT_U8_maskValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashMaskGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TRUNK_LBH_MASK_ENT_maskedField,
    &DX_OUT_GT_U8_PTR_maskValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashIpShiftSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_GT_BOOL_isSrcIp,
    &DX_IN_GT_U32_shiftValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashIpShiftGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_GT_BOOL_isSrcIp,
    &DX_OUT_GT_U32_PTR_shiftValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbSharedPortEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbIsMemberOfTrunkGetNext_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr,
    &DX_INOUT_GT_TRUNK_ID_PTR_trunkIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbSharedPortEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbIsMemberOfTrunk_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr,
    &DX_OUT_GT_TRUNK_ID_PTR_trunkIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkPortMcEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbPortTrunkIdModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_manageMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkPortTrunkIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_memberOfTrunk,
    &DX_IN_GT_TRUNK_ID_trunkId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbPortMcEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbPortTrunkIdModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_PTR_manageModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkPortTrunkIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_memberOfTrunkPtr,
    &DX_OUT_GT_TRUNK_ID_PTR_trunkIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkPortHashMaskInfoSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_overrideEnable,
    &DX_IN_GT_U32_index
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkPortHashMaskInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_overrideEnablePtr,
    &DX_OUT_GT_U32_PTR_indexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkLearnPrioritySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_DXCH_FDB_LEARN_PRIORITY_ENT_learnPriority
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkEcmpLttTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkNonTrunkPortsAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkNonTrunkPortsEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkCascadeTrunkPortsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_PORTS_BMP_STC_PTR_portsMembersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkMemberAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbSharedPortInfoSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR_sharedPortInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkMcLocalSwitchingEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDesignatedMemberSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkFlexInfoSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_U32_l2EcmpStartIndex,
    &DX_IN_GT_U32_maxNumOfMembers
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_U32_numMembers,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_membersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkMembersSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_U32_numOfEnabledMembers,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_enabledMembersArray,
    &DX_IN_GT_U32_numOfDisabledMembers,
    &DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_disabledMembersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkCascadeTrunkWithWeightedPortsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_U32_numOfMembers,
    &DX_IN_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR_weightedMembersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashBitSelectionProfileIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_U32_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkUserGroupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_IN_GT_U32_userGroup
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbDisabledMembersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_INOUT_GT_U32_PTR_numOfDisabledMembersPtr,
    &DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_disabledMembersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbEnabledMembersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_INOUT_GT_U32_PTR_numOfEnabledMembersPtr,
    &DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_enabledMembersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkCascadeTrunkWithWeightedPortsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_INOUT_GT_U32_PTR_numOfMembersPtr,
    &DX_OUT_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR_weightedMembersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkLearnPriorityGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_CPSS_DXCH_FDB_LEARN_PRIORITY_ENT_PTR_learnPriorityPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkEcmpLttTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkNonTrunkPortsEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkCascadeTrunkPortsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_CPSS_PORTS_BMP_STC_PTR_portsMembersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbSharedPortInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR_sharedPortInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbTrunkTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_CPSS_TRUNK_TYPE_ENT_PTR_typePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbMcLocalSwitchingEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbDesignatedMemberGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbFlexInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_GT_U32_PTR_l2EcmpStartIndexPtr,
    &DX_OUT_GT_U32_PTR_maxNumOfMembersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_GT_U32_PTR_numMembersPtr,
    &DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_membersArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashBitSelectionProfileIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkUserGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TRUNK_ID_trunkId,
    &DX_OUT_GT_U32_PTR_userGroupPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDesignatedPortsEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_PORTS_BMP_STC_PTR_designatedPortsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDesignatedPortsEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_PORTS_BMP_STC_PTR_designatedPortsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashCrcSaltByteSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_hashIndex,
    &DX_IN_GT_U32_byteIndex,
    &DX_IN_GT_U8_saltValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashCrcSaltByteGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_hashIndex,
    &DX_IN_GT_U32_byteIndex,
    &DX_OUT_GT_U8_PTR_saltValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashMaskCrcEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_hashIndex,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashMaskCrcEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_hashIndex,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_maxNumberOfTrunks,
    &DX_IN_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_trunkMembersMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashDesignatedTableModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashIpv6ModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_PTR_hashModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashL4ModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_PTR_hashModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashCrcParametersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_PTR_crcModePtr,
    &DX_OUT_GT_U32_PTR_crcSeedPtr,
    &DX_OUT_GT_U32_PTR_crcSeedHash1Ptr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkHashGlobalModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_PTR_hashModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTrunkDbInitInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_maxNumberOfTrunksPtr,
    &DX_OUT_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_PTR_trunkMembersModePtr
};


/********* lib API DB *********/

extern void cpssDxChTrunkTableEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTrunkTableEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTrunkDbEnabledMembersGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTrunkDbEnabledMembersGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTrunkMembersSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTrunkTableEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTrunkDbEnabledMembersGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTrunkLogLibDb[] = {
    {"cpssDxChTrunkDbMembersSortingEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTrunkDbMembersSortingEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTrunkPortTrunkIdSet", 4, cpssDxChTrunkPortTrunkIdSet_PARAMS, NULL},
    {"cpssDxChTrunkPortTrunkIdGet", 4, cpssDxChTrunkPortTrunkIdGet_PARAMS, NULL},
    {"cpssDxChTrunkTableEntrySet", 4, cpssDxChTrunkTableEntrySet_PARAMS, cpssDxChTrunkTableEntrySet_preLogic},
    {"cpssDxChTrunkTableEntryGet", 4, cpssDxChTrunkTableEntryGet_PARAMS, cpssDxChTrunkTableEntryGet_preLogic},
    {"cpssDxChTrunkNonTrunkPortsEntrySet", 3, cpssDxChTrunkNonTrunkPortsEntrySet_PARAMS, NULL},
    {"cpssDxChTrunkNonTrunkPortsEntryGet", 3, cpssDxChTrunkNonTrunkPortsEntryGet_PARAMS, NULL},
    {"cpssDxChTrunkDesignatedPortsEntrySet", 3, cpssDxChTrunkDesignatedPortsEntrySet_PARAMS, NULL},
    {"cpssDxChTrunkDesignatedPortsEntryGet", 3, cpssDxChTrunkDesignatedPortsEntryGet_PARAMS, NULL},
    {"cpssDxChTrunkHashIpModeSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTrunkHashIpModeGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTrunkHashL4ModeSet", 2, cpssDxChTrunkHashL4ModeSet_PARAMS, NULL},
    {"cpssDxChTrunkHashL4ModeGet", 2, cpssDxChTrunkHashL4ModeGet_PARAMS, NULL},
    {"cpssDxChTrunkHashIpv6ModeSet", 2, cpssDxChTrunkHashIpv6ModeSet_PARAMS, NULL},
    {"cpssDxChTrunkHashIpv6ModeGet", 2, cpssDxChTrunkHashIpv6ModeGet_PARAMS, NULL},
    {"cpssDxChTrunkHashIpAddMacModeSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTrunkHashIpAddMacModeGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTrunkHashGlobalModeSet", 2, cpssDxChTrunkHashGlobalModeSet_PARAMS, NULL},
    {"cpssDxChTrunkHashGlobalModeGet", 2, cpssDxChTrunkHashGlobalModeGet_PARAMS, NULL},
    {"cpssDxChTrunkHashDesignatedTableModeSet", 2, cpssDxChTrunkHashDesignatedTableModeSet_PARAMS, NULL},
    {"cpssDxChTrunkHashDesignatedTableModeGet", 2, cpssDxChTrunkHashDesignatedTableModeGet_PARAMS, NULL},
    {"cpssDxChTrunkHashNumBitsSet", 4, cpssDxChTrunkHashNumBitsSet_PARAMS, NULL},
    {"cpssDxChTrunkHashNumBitsGet", 4, cpssDxChTrunkHashNumBitsGet_PARAMS, NULL},
    {"cpssDxChTrunkDbEnabledMembersGet", 4, cpssDxChTrunkDbEnabledMembersGet_PARAMS, cpssDxChTrunkDbEnabledMembersGet_preLogic},
    {"cpssDxChTrunkDbDisabledMembersGet", 4, cpssDxChTrunkDbDisabledMembersGet_PARAMS, cpssDxChTrunkDbEnabledMembersGet_preLogic},
    {"cpssDxChTrunkDbIsMemberOfTrunk", 3, cpssDxChTrunkDbIsMemberOfTrunk_PARAMS, NULL},
    {"cpssDxChTrunkDbIsMemberOfTrunkGetNext", 3, cpssDxChTrunkDbIsMemberOfTrunkGetNext_PARAMS, NULL},
    {"cpssDxChTrunkDbTrunkTypeGet", 3, cpssDxChTrunkDbTrunkTypeGet_PARAMS, NULL},
    {"cpssDxChTrunkDesignatedMemberSet", 4, cpssDxChTrunkDesignatedMemberSet_PARAMS, NULL},
    {"cpssDxChTrunkDbDesignatedMemberGet", 4, cpssDxChTrunkDbDesignatedMemberGet_PARAMS, NULL},
    {"cpssDxChTrunkMembersSet", 6, cpssDxChTrunkMembersSet_PARAMS, cpssDxChTrunkMembersSet_preLogic},
    {"cpssDxChTrunkCascadeTrunkPortsSet", 3, cpssDxChTrunkCascadeTrunkPortsSet_PARAMS, NULL},
    {"cpssDxChTrunkCascadeTrunkPortsGet", 3, cpssDxChTrunkCascadeTrunkPortsGet_PARAMS, NULL},
    {"cpssDxChTrunkCascadeTrunkWithWeightedPortsSet", 4, cpssDxChTrunkCascadeTrunkWithWeightedPortsSet_PARAMS, cpssDxChTrunkTableEntrySet_preLogic},
    {"cpssDxChTrunkCascadeTrunkWithWeightedPortsGet", 4, cpssDxChTrunkCascadeTrunkWithWeightedPortsGet_PARAMS, cpssDxChTrunkDbEnabledMembersGet_preLogic},
    {"cpssDxChTrunkMemberAdd", 3, cpssDxChTrunkMemberAdd_PARAMS, NULL},
    {"cpssDxChTrunkMemberRemove", 3, cpssDxChTrunkMemberAdd_PARAMS, NULL},
    {"cpssDxChTrunkMemberDisable", 3, cpssDxChTrunkMemberAdd_PARAMS, NULL},
    {"cpssDxChTrunkMemberEnable", 3, cpssDxChTrunkMemberAdd_PARAMS, NULL},
    {"cpssDxChTrunkNonTrunkPortsAdd", 3, cpssDxChTrunkNonTrunkPortsAdd_PARAMS, NULL},
    {"cpssDxChTrunkNonTrunkPortsRemove", 3, cpssDxChTrunkNonTrunkPortsAdd_PARAMS, NULL},
    {"cpssDxChTrunkInit", 3, cpssDxChTrunkInit_PARAMS, NULL},
    {"cpssDxChTrunkDbInitInfoGet", 3, cpssDxChTrunkDbInitInfoGet_PARAMS, NULL},
    {"cpssDxChTrunkHashMplsModeEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTrunkHashMplsModeEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTrunkHashMaskSet", 3, cpssDxChTrunkHashMaskSet_PARAMS, NULL},
    {"cpssDxChTrunkHashMaskGet", 3, cpssDxChTrunkHashMaskGet_PARAMS, NULL},
    {"cpssDxChTrunkHashIpShiftSet", 4, cpssDxChTrunkHashIpShiftSet_PARAMS, NULL},
    {"cpssDxChTrunkHashIpShiftGet", 4, cpssDxChTrunkHashIpShiftGet_PARAMS, NULL},
    {"cpssDxChTrunkHashCrcParametersSet", 4, cpssDxChTrunkHashCrcParametersSet_PARAMS, NULL},
    {"cpssDxChTrunkHashCrcParametersGet", 4, cpssDxChTrunkHashCrcParametersGet_PARAMS, NULL},
    {"cpssDxChTrunkPortHashMaskInfoSet", 4, cpssDxChTrunkPortHashMaskInfoSet_PARAMS, NULL},
    {"cpssDxChTrunkPortHashMaskInfoGet", 4, cpssDxChTrunkPortHashMaskInfoGet_PARAMS, NULL},
    {"cpssDxChTrunkHashMaskCrcEntrySet", 4, cpssDxChTrunkHashMaskCrcEntrySet_PARAMS, NULL},
    {"cpssDxChTrunkHashMaskCrcEntryGet", 4, cpssDxChTrunkHashMaskCrcEntryGet_PARAMS, NULL},
    {"cpssDxChTrunkHashIndexCalculate", 6, cpssDxChTrunkHashIndexCalculate_PARAMS, NULL},
    {"cpssDxChTrunkHashPearsonValueSet", 3, prvCpssLogGenDevNumIndexValue_PARAMS, NULL},
    {"cpssDxChTrunkHashPearsonValueGet", 3, prvCpssLogGenDevNumIndexValuePtr_PARAMS, NULL},
    {"cpssDxChTrunkMcLocalSwitchingEnableSet", 3, cpssDxChTrunkMcLocalSwitchingEnableSet_PARAMS, NULL},
    {"cpssDxChTrunkDbMcLocalSwitchingEnableGet", 3, cpssDxChTrunkDbMcLocalSwitchingEnableGet_PARAMS, NULL},
    {"cpssDxChTrunkMemberSelectionModeSet", 2, prvCpssLogGenDevNumSelectionMode_PARAMS, NULL},
    {"cpssDxChTrunkMemberSelectionModeGet", 2, prvCpssLogGenDevNumSelectionModePtr_PARAMS, NULL},
    {"cpssDxChTrunkLearnPrioritySet", 3, cpssDxChTrunkLearnPrioritySet_PARAMS, NULL},
    {"cpssDxChTrunkLearnPriorityGet", 3, cpssDxChTrunkLearnPriorityGet_PARAMS, NULL},
    {"cpssDxChTrunkUserGroupSet", 3, cpssDxChTrunkUserGroupSet_PARAMS, NULL},
    {"cpssDxChTrunkUserGroupGet", 3, cpssDxChTrunkUserGroupGet_PARAMS, NULL},
    {"cpssDxChTrunkHashCrcSaltByteSet", 4, cpssDxChTrunkHashCrcSaltByteSet_PARAMS, NULL},
    {"cpssDxChTrunkHashCrcSaltByteGet", 4, cpssDxChTrunkHashCrcSaltByteGet_PARAMS, NULL},
    {"cpssDxChTrunkHashMaskCrcParamOverrideSet", 3, cpssDxChTrunkHashMaskCrcParamOverrideSet_PARAMS, NULL},
    {"cpssDxChTrunkHashMaskCrcParamOverrideGet", 3, cpssDxChTrunkHashMaskCrcParamOverrideGet_PARAMS, NULL},
    {"cpssDxChTrunkPortMcEnableSet", 3, cpssDxChTrunkPortMcEnableSet_PARAMS, NULL},
    {"cpssDxChTrunkDbPortMcEnableGet", 3, cpssDxChTrunkDbPortMcEnableGet_PARAMS, NULL},
    {"cpssDxChTrunkDbPortTrunkIdModeSet", 3, cpssDxChTrunkDbPortTrunkIdModeSet_PARAMS, NULL},
    {"cpssDxChTrunkDbPortTrunkIdModeGet", 3, cpssDxChTrunkDbPortTrunkIdModeGet_PARAMS, NULL},
    {"cpssDxChTrunkFlexInfoSet", 4, cpssDxChTrunkFlexInfoSet_PARAMS, NULL},
    {"cpssDxChTrunkDbFlexInfoGet", 4, cpssDxChTrunkDbFlexInfoGet_PARAMS, NULL},
    {"cpssDxChTrunkEcmpLttTableSet", 3, cpssDxChTrunkEcmpLttTableSet_PARAMS, NULL},
    {"cpssDxChTrunkEcmpLttTableGet", 3, cpssDxChTrunkEcmpLttTableGet_PARAMS, NULL},
    {"cpssDxChTrunkDestroy", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChTrunkHashPacketTypeHashModeSet", 3, cpssDxChTrunkHashPacketTypeHashModeSet_PARAMS, NULL},
    {"cpssDxChTrunkHashPacketTypeHashModeGet", 3, cpssDxChTrunkHashPacketTypeHashModeGet_PARAMS, NULL},
    {"cpssDxChTrunkHashPacketTypeHashModeBitOffsetSet", 3, cpssDxChTrunkHashPacketTypeHashModeBitOffsetSet_PARAMS, NULL},
    {"cpssDxChTrunkHashPacketTypeHashModeBitOffsetGet", 3, cpssDxChTrunkHashPacketTypeHashModeBitOffsetGet_PARAMS, NULL},
    {"cpssDxChTrunkHashBitSelectionProfileIndexSet", 3, cpssDxChTrunkHashBitSelectionProfileIndexSet_PARAMS, NULL},
    {"cpssDxChTrunkHashBitSelectionProfileIndexGet", 3, cpssDxChTrunkHashBitSelectionProfileIndexGet_PARAMS, NULL},
    {"cpssDxChTrunkDbSharedPortEnableSet", 3, cpssDxChTrunkDbSharedPortEnableSet_PARAMS, NULL},
    {"cpssDxChTrunkDbSharedPortEnableGet", 3, cpssDxChTrunkDbSharedPortEnableGet_PARAMS, NULL},
    {"cpssDxChTrunkDbSharedPortInfoSet", 3, cpssDxChTrunkDbSharedPortInfoSet_PARAMS, NULL},
    {"cpssDxChTrunkDbSharedPortInfoGet", 3, cpssDxChTrunkDbSharedPortInfoGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TRUNK(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTrunkLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTrunkLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

