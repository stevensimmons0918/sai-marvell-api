/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxIngressLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxIngressLogh
#define __prvCpssPxIngressLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_ETHERTYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_HASH_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_UDB_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_UDB_PAIR_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_UDB_PAIR_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_INGRESS_ETHERTYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIRECTION_ENT_direction;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_ETHERTYPE_ENT_configType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_lagTableMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_MODE_ENT_hashMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT_hashPacketType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_indexMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR_udbpArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR_ip2meEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR_packetTypeFormatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT_tableType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR_portKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR_tpidEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORTS_BMP_targetPortsBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_bypassLagDesignatedBitmap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_sourcePortEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_valid;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U16_HEX_etherType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_bitOffsetsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_crc32Seed;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_entryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_lagTableNumber;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_offset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT_PTR_lagTableModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_MODE_ENT_PTR_hashModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT_PTR_indexModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_HASH_UDBP_STC_PTR_udbpArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_IP2ME_ENTRY_STC_PTR_ip2meEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_errorKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC_PTR_packetTypeFormatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC_PTR_portKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_INGRESS_TPID_ENTRY_STC_PTR_tpidEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_bypassLagDesignatedBitmapPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_sourcePortEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U16_HEX_PTR_etherTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_bitOffsetsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_crc32SeedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_lagTableNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_offsetPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxIngressTpidEntrySet_E = (CPSS_LOG_LIB_INGRESS_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxIngressTpidEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressEtherTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressEtherTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMacDaByteOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMacDaByteOffsetGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortPacketTypeKeySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortPacketTypeKeyGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPacketTypeKeyEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPacketTypeKeyEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMapPacketTypeFormatEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMapPacketTypeFormatEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMapEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMapEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortTargetEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortTargetEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPacketTypeErrorGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressForwardingPortMapExceptionSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressForwardingPortMapExceptionGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortMapExceptionCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPacketTypeKeyEntryEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPacketTypeKeyEntryEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortDsaTagEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortDsaTagEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressPortRedirectSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressFilteringEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressIp2MeEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressIp2MeEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashUdeEthertypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashUdeEthertypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPacketTypeEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPacketTypeEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPacketTypeHashModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPacketTypeHashModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashSeedSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashSeedGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPortIndexModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPortIndexModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashDesignatedPortsEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashDesignatedPortsEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashLagTableModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashLagTableModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPacketTypeLagTableNumberSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxIngressHashPacketTypeLagTableNumberGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxIngressLogh */
