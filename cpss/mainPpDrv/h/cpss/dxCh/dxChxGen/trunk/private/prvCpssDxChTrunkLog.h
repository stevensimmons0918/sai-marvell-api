/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTrunkLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChTrunkLogh
#define __prvCpssDxChTrunkLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_HASH_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_LBH_MASK_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_HASH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_LBH_MASK_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_TRUNK_ID_PTR_trunkIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfDisabledMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfEnabledMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashClient;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT_hashEntityType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT_fieldType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_HASH_MODE_ENT_hashMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_hashMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_hashMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_crcMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_hashMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC_PTR_hashInputDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_LBH_MASK_ENT_maskedField;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_trunkMembersMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_designatedPortsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_disabledMembersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_enabledMembersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_MEMBER_STC_PTR_membersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_manageMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR_sharedPortInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR_weightedMembersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isSrcIp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_memberOfTrunk;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_keyBitOffsetArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_byteIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_crcSeed;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_crcSeedHash1;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hashIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_l2EcmpStartIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxNumOfMembers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxNumberOfTrunks;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numMembers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfDisabledMembers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfEnabledMembers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfMembers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfMembers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_shiftValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_maskValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_saltValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_HASH_MODE_ENT_PTR_hashModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT_PTR_hashModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT_PTR_hashModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT_PTR_crcModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT_PTR_hashModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT_PTR_trunkMembersModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_designatedPortsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_nonTrunkPortsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_disabledMembersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_enabledMembersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_memberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_MEMBER_STC_PTR_membersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_PTR_manageModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR_sharedPortInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_TYPE_ENT_PTR_typePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR_weightedMembersArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_memberOfTrunkPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_overridePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_crcSeedHash1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_crcSeedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_keyBitOffsetArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_l2EcmpStartIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxNumOfMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxNumberOfTrunksPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_shiftValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_maskValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_saltValuePtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbMembersSortingEnableSet_E = (CPSS_LOG_LIB_TRUNK_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbMembersSortingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkPortTrunkIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkPortTrunkIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkTableEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkTableEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkNonTrunkPortsEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkNonTrunkPortsEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDesignatedPortsEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDesignatedPortsEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashL4ModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashL4ModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpv6ModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpv6ModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpAddMacModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpAddMacModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashGlobalModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashGlobalModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashDesignatedTableModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashDesignatedTableModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashNumBitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashNumBitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbEnabledMembersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbDisabledMembersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbIsMemberOfTrunk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbIsMemberOfTrunkGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbTrunkTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDesignatedMemberSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbDesignatedMemberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMembersSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkCascadeTrunkPortsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkCascadeTrunkPortsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkCascadeTrunkWithWeightedPortsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkCascadeTrunkWithWeightedPortsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMemberAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMemberRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMemberDisable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMemberEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkNonTrunkPortsAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkNonTrunkPortsRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbInitInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMplsModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMplsModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMaskSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMaskGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpShiftSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIpShiftGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashCrcParametersSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashCrcParametersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkPortHashMaskInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkPortHashMaskInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMaskCrcEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMaskCrcEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashIndexCalculate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashPearsonValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashPearsonValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMcLocalSwitchingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbMcLocalSwitchingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMemberSelectionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkMemberSelectionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkLearnPrioritySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkLearnPriorityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkUserGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkUserGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashCrcSaltByteSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashCrcSaltByteGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMaskCrcParamOverrideSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashMaskCrcParamOverrideGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkPortMcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbPortMcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbPortTrunkIdModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbPortTrunkIdModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkFlexInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbFlexInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkEcmpLttTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkEcmpLttTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDestroy_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashPacketTypeHashModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashPacketTypeHashModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashPacketTypeHashModeBitOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashPacketTypeHashModeBitOffsetGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashBitSelectionProfileIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkHashBitSelectionProfileIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbSharedPortEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbSharedPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbSharedPortInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTrunkDbSharedPortInfoGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChTrunkLogh */
