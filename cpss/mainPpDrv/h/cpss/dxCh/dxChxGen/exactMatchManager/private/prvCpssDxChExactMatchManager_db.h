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
* @file prvCpssDxChExactMatchManager_db.h
*
* @brief Exact Match manager support.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchManager_db_h
#define __prvCpssDxChExactMatchManager_db_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

/* global variables macros */

#define PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxchEmMgrDir.emMgrDbSrc._var)

#define PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxchEmMgrDir.emMgrDbgSrc._var)

/* maximum value for time to live (TTL) field */
#define TTL_MAX_CNS                     255
/* maximum value for VRF ID field */
#define VRF_ID_MAX_CNS                  4095
/* max num of vlans -- range 0..4096 */
#define MAX_NUM_VLANS_CNS               4096
/* maximum value for QOS profile field for E-arch devices */
#define TTI_QOS_PROFILE_E_ARCH_MAX_CNS       127

/* maximum value for qos mapping table index */
#define QOS_MAPPING_TABLE_INDEX_MAX_CNS     12
/* maximum value for index of analyzer interface */
#define INGRESS_MIRROE_TO_ANALYZER_INDEX_MAX_CNS 6
/* maximum value for OAM profile */
#define OAM_PROFILE_MAX_CNS                 1
/* maximum value for OAM index */
#define OAM_INDEX_MAX_CNS                   15

/* number of words needed for 'BMP' of devices */
#define NUM_WORDS_DEVS_BMP_CNS  ((PRV_CPSS_MAX_PP_DEVICES_CNS+31)/32)

/* number of words needed for 'BMP' of profile Table */
#define NUM_WORDS_PROFILE_ID_MAPPING_ENTRIES_BMP_CNS  ((MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS+31)/32)

/* we force application to give 'num of entries' in steps of 256 */
#define PRV_EXACT_MATCH_MANAGER_NUM_ENTRIES_STEPS_CNS 256

/* Exact Match maximal entries per age-bin */
#define PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_AGE_BIN_ID_CNS    \
    (_256K / PRV_EXACT_MATCH_MANAGER_NUM_ENTRIES_STEPS_CNS)

/* macro to allocate memory (associated with the manager)
  and memset to ZERO , into pointer according to needed 'size in bytes'*/
#define EM_MEM_CALLOC_MAC(_exactMatchManagerId,_pointer,_sizeInBytes)       \
    {                                                             \
        if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsMalloc))               \
            _pointer = PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsMalloc)(_sizeInBytes);  \
        else                                                      \
            _pointer = cpssOsMalloc(_sizeInBytes);                \
        if(NULL == _pointer)                    \
        {                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "manager[%d] [%s] failed to allocate memory [%d] bytes ", \
                _exactMatchManagerId,#_pointer,_sizeInBytes);   \
        }                                       \
                                                \
        cpssOsMemSet(_pointer,0,(_sizeInBytes));\
                                                \
        PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[_exactMatchManagerId]->numOfAllocatedPointers++;        \
        /* cpssOsPrintf("alloc [%s][%p] size[%d] exactMatchManagerId[%d] numOfAllocatedPointers[%d]\n",#_pointer,_pointer,_sizeInBytes, _exactMatchManagerId, prvCpssDxChExactMatchManagerDbArr[_exactMatchManagerId]->numOfAllocatedPointers); */ \
    }

/* macro to free memory (associated with the manager) */
#define EM_MEM_FREE_MAC(_exactMatchManagerId,_pointer)        \
    if(_pointer)                                    \
    {                                               \
        /*cpssOsPrintf("free [%s][%p] exactMatchManagerId[%d] numOfAllocatedPointers[%d]\n",#_pointer,_pointer, _exactMatchManagerId, prvCpssDxChExactMatchManagerDbArr[_exactMatchManagerId]->numOfAllocatedPointers); */ \
        PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[_exactMatchManagerId]->numOfAllocatedPointers--;\
        if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsFree))                           \
        {                                                                   \
            if (_pointer)                                                   \
                PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsFree)(_pointer);            \
            _pointer = NULL;                                                \
        }                                                                   \
        else                                                                \
        {                                                                   \
            FREE_PTR_MAC(_pointer);                                         \
        }                                                                   \
    }

/**
 * @union PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_RESTORE_INFO_UNT
 *
 * @brief internal Exact Match manager restore info.
*/
typedef union {
    /* USED_LIST_1ST_PTR_INVALID */
    /* USED_LIST_DELETE_SCAN_PTR_INVALID */
    struct{
        GT_U32  entryPointer;
        GT_BOOL isValid_entryPointer;
        GT_BOOL isUsedEntry;
    }nodeInfo;

    /* FREE_LIST_1ST_PTR_INVALID,       INDEX_POINT_INVALID,            USED_LIST_GETNEXT_PTR_INVALID,
     * AGING_SCAN_PTR_INVALID,          HW_FREE_ENTRY_VALID_EXIST,      USED_LIST_ENTRY_PREV_PTR_INVALID,
     */
    GT_BOOL entryStatus;

    /* AGING_USAGE_MATRIX_L2_INVALID */
    GT_BOOL l2BitStatus;

    /* TYPE_COUNTER_INVALID,            BANK_COUNTER_INVALID,           HW_SW_COUNTER_MISMATCH
     * FREE_LIST_ENTRIES_NUM_MISMATCH,  USED_LIST_LAST_PTR_INVALID_E,   USED_LIST_ENTRIES_NUM_MISMATCH,
     * AGING_TOTAL_NUM_MISMATCH,        AGING_BIN_ENTRIES_NUM_MISMATCH  AGING_USAGE_MATRIX_L1_INVALID
     * GLOBAL_PARAM_HW_INVALID
     */
    GT_U32  counter;

    /* ENTRY_INDEX_INVALID */
    GT_U32  hwIndex;

    /* AGING_ENTRY_BIN_INDEX_INVALID */
    GT_U32  ageBinIndex;

    /* HW_ENTRY_INVALID */
    GT_U8 pattern_0;/* first octet of the pattern */

    /* Lookup client invalid */
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT lookupClient;

    /* profileId invalid */
    GT_U32 profileId;

    /* invalid key params */
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;

    /* invalid default action enable flag */
    GT_BOOL                                         defaultActionEn;

    /* invalid default action */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                defaultAction;

   /* invalid expanded entry */
    GT_BOOL                                          exactMatchExpandedEntryValid;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            expandedActionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 expandedAction;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT expandedActionOrigin;

    /* invalidate cockoo entry */
    GT_U32 cuckooEntry_sizeInBytes;

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_RESTORE_INFO_UNT;

/**
 * @enum PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_ENT
 *
 * @brief types of HW Exact Match entries of manager format.
 *
*/
typedef enum{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E              = 0,
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_PCL_E              = 1,
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_EPCL_E             = 2,
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_LAST_E             = 3

} PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_ENT;

/**
 * @union PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INTERFACE_INFO_UNT
 *
 * @brief internal Exact Match manager DB (database) Entry ,
 *        target interface : port/trunk/vidx/vid
 *
*/
typedef union{
    /* for type CPSS_INTERFACE_PORT_E */
    struct{
        /* compress 2 fields to 32 bits : using bit fields */
        GT_HW_DEV_NUM   hwDevNum:10;
        GT_PORT_NUM     portNum :22;
    }devPort;
    /* for type CPSS_INTERFACE_TRUNK_E */
    GT_TRUNK_ID     trunkId;
    /* for type CPSS_INTERFACE_VIDX_E */
    GT_U16          vidx;
    /* NOTE: for type CPSS_INTERFACE_VID_E : no extra info needed ! */

    /* NOTE: other types not supported */
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INTERFACE_INFO_UNT;

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC
*
* @brief internal Exact Match manager DB (database) Entry
*       format matching :
*       PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E
*       CPSS_DXCH_TTI_ACTION_STC
*/
typedef struct{
    GT_U32 tunnelTerminate                                      : 1; /* bit 171 */
    GT_U32 ttPassengerPacketType                                : 2; /* bit 168-169 */
    GT_U32 tsPassengerPacketType                                : 1; /* bit 61 */
    GT_U32 ttHeaderLength                                       : 6; /* bit 208-213 */
    GT_U32 tunnelHeaderLengthAnchorType                         : 2; /* bit 244-245 */
    GT_U32 continueToNextTtiLookup                              : 1; /* bit 237 */
    GT_U32 copyTtlExpFromTunnelHeader                           : 1; /* bit 170 */
    GT_U32 mplsCommand                                          : 3; /* bit 172-174 */
    GT_U32 mplsTtl                                              : 8; /* bit 184-191 */
    GT_U32 enableDecrementTtl                                   : 1; /* bit 192 */
    GT_U32 passengerParsingOfTransitMplsTunnelMode              : 2; /* bit 239-240 */
    GT_U32 passengerParsingOfTransitNonMplsTransitTunnelEnable  : 1; /* bit 241 */
    GT_U32 command                                              : 3; /* bit 0-2 */
/* aligned to 32 bits */

   /* the 'union' open new 32 bits alignment */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INTERFACE_INFO_UNT egressInterface;
/* aligned to 32 bits */

    GT_U32 redirectCommand                                      : 3; /* bit 14-16 */
    GT_U32 tunnelStart                                          : 1; /* bit 44 : indication if redirectPtr  used as TS pointer or as ARP pointer */
    GT_U32 redirectPtr                                          : 18;/* treat this field as:
                                                                        routerLttPtr 18 bits(17-34) when Redirect_Command="IPNextHop"
                                                                        vrfId 11 bits(17-28) when Redirect_Command="Assign_VRF_ID"
                                                                        flowId 13 bits(17-29)when Redirect_Command="NoRedirect"
                                                                        16 bits (45-60) for TS when Redirect_Command="Egress_Interface" and TunnelStart="True"
                                                                        18 bits (45-62) for ARP when Redirect_Command="Egress_Interface" and TunnelStart="false" */
    GT_U32 userDefinedCpuCode                                   : 8; /* bit 3-10*/
    GT_U32 vntl2Echo                                            : 1; /* bit 43*/
    GT_U32 bridgeBypass                                         : 1; /* bit 110*/
/* aligned to 32 bits */

    GT_U32 tag0VlanCmd                                          : 3;/* bit 114-116*/
    GT_U32 tag0VlanId                                           : 13;/* bit 117-129*/
    GT_U32 tag1VlanCmd                                          : 1;/* bit 130*/
    GT_U32 tag1VlanId                                           : 12;/* bit 131-142*/
    GT_U32 tag0VlanPrecedence                                   : 1;/* bit 112*/
    GT_U32 nestedVlanEnable                                     : 1;/* bit 113*/
    GT_U32 actionStop                                           : 1;/* bit 109*/
/* aligned to 32 bits */

    GT_U32 bindToPolicerMeter                                   : 1;/* bit 80*/
    GT_U32 bindToPolicer                                        : 1;/* bit 81*/
    GT_U32 policerIndex                                         : 14;/* bit 82-95*/
    GT_U32 qosPrecedence                                        : 1;/* bit 143*/
    GT_U32 keepPreviousQoS                                      : 1;/* bit 155*/
    GT_U32 trustUp                                              : 1;/* bit 156*/
    GT_U32 trustDscp                                            : 1;/* bit 157*/
    GT_U32 trustExp                                             : 1;/* bit 158*/
    GT_U32 qosProfile                                           : 7;/* bit 144-150*/
    GT_U32 modifyTag0Up                                         : 2;/* bit 153-154*/
    GT_U32 tag1UpCommand                                        : 2;/* bit 163-164*/
/* aligned to 32 bits */

    GT_U32 modifyDscp                                           : 2;/* bit 151-152*/
    GT_U32 tag0Up                                               : 2;/* bit 160-162*/
    GT_U32 tag1Up                                               : 2;/* bit 165-167*/
    GT_U32 remapDSCP                                            : 1;/* bit 159*/
    GT_U32 qosUseUpAsIndexEnable                                : 1;/* bit 229*/
    GT_U32 qosMappingTableIndex                                 : 4;/* bit 179-182*/
    GT_U32 mplsLLspQoSProfileEnable                             : 1;/* bit 183*/
    GT_U32 pcl0OverrideConfigIndex                              : 1;/* bit 51*/
    GT_U32 pcl0_1OverrideConfigIndex                            : 1;/* bit 49*/
    GT_U32 pcl1OverrideConfigIndex                              : 1;/* bit 50*/
    GT_U32 iPclConfigIndex                                      : 13;/* bit 52-64*/
    GT_U32 iPclUdbConfigTableIndex                              : 3;/* bit 30-32*/
/* aligned to 32 bits */

    GT_U32 sourceEPortAssignmentEnable                          : 1;/* bit 193*/
    GT_U32 sourceEPort                                          : 14;/* bit 194-207*/
    GT_U32 bindToCentralCounter                                 : 1;/* bit 65*/
    GT_U32 centralCounterIndex                                  : 14;/* bit 66-79*/
    GT_U32 modifyMacSa                                          : 1;/* bit 64*/
    GT_U32 modifyMacDa                                          : 1;/* bit 63*/
/* aligned to 32 bits */

    GT_U32 hashMaskIndex                                        : 4;/* bit 175-178*/
    GT_U32 setMacToMe                                           : 1;/* bit 232*/
    GT_U32 rxProtectionSwitchEnable                             : 1;/* bit 231*/
    GT_U32 rxIsProtectionPath                                   : 1;/* bit 230*/
    GT_U32 pwTagMode                                            : 2;/* bit 235-236*/
    GT_U32 oamTimeStampEnable                                   : 1;/* bit 215*/
    GT_U32 oamOffsetIndex                                       : 7;/* bit 216-222*/
    GT_U32 oamProcessEnable                                     : 1;/* bit 224*/
    GT_U32 oamProcessWhenGalOrOalExistsEnable                   : 1;/* bit 223*/
    GT_U32 oamProfile                                           : 1;/* bit 233*/
    GT_U32 oamChannelTypeToOpcodeMappingEnable                  : 1;/* bit 238*/
    GT_U32 isPtpPacket                                          : 1;/* bit 214*/
    GT_U32 ptpTriggerType                                       : 2;/* bit 215-216*/
    GT_U32 ptpOffset                                            : 7;/* bit 216-222*/
    GT_U32 ipv6SegmentRoutingEndNodeEnable                      : 1;/* bit 243*/
/* aligned to 32 bits */

    GT_U32 sourceIdSetEnable                                    : 1; /* bit 96 */
    GT_U32 sourceId                                             : 12;/* bit 97-108 */
    GT_U32 ingressPipeBypass                                    : 1; /* bit 111*/
    GT_U32 mirrorToIngressAnalyzerIndex                         : 3;/* bit 11-13*/
    GT_U32 cwBasedPw                                            : 1;/* bit 225*/
    GT_U32 ttlExpiryVccvEnable                                  : 1;/* bit 226*/
    GT_U32 pwe3FlowLabelExist                                   : 1;/* bit 227*/
    GT_U32 pwCwBasedETreeEnable                                 : 1;/* bit 228*/
    GT_U32 applyNonDataCwCommand                                : 1;/* bit 234*/
    GT_U32 skipFdbSaLookupEnable                                : 1;/* bit 242*/
    GT_U32 exactMatchOverTtiEn                                  : 1;
    GT_U32 egressInterface_type                                 : 2; /* CPSS_INTERFACE_TYPE_ENT */
    GT_U32 copyReservedAssignmentEnable                         : 1; /* bit 35 */
    GT_U32 triggerHashCncClient                                 : 1; /* bit 37 for Falcon
                                                                        bit 247 for AC5P; AC5X */
    /** 4 bits pleceHolder - can be replaced in the future
     *  by a real 4 bits value */
/* left with 'spare' of : 32-(1+12+1+3+1+1+1+1+1+1+1+2+1+1) = 4 bits */
    GT_U32 pleceHolder_4bits                                    : 4;

/* aligned to 32 bits */

    GT_U32 copyReserved                                         : 13;/* bit 36-48 */

/* left with 'spare' of : 32-(13) = 19 bits plus pleceHolder_4bits  */

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*
*  @brief internal Exact Match manager DB (database) Entry
*       format matching :
*       PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_PCL_E
*       CPSS_DXCH_PCL_ACTION_STC
*
*/
typedef struct{

    GT_U32 bindToCncCounter                 : 1;/* enableMatchCount */
    GT_U32 cncCounterIndex                  : 15;/*matchCounterIndex*/
    GT_U32 bindToPolicerMeter               : 1;
    GT_U32 bindToPolicerCounter             : 1;
    GT_U32 policerIndex                     : 14;
/* aligned to 32 bits */

    GT_U32 pktCmd                           : 3;
    GT_U32 oamTimeStampEnable               : 1;
    GT_U32 oamOffsetIndex                   : 7;
    GT_U32 oamProcessEnable                 : 1;
    GT_U32 oamProfile                       : 1;
    GT_U32 flowId                           : 16;
    GT_U32 ingressMirrorToAnalyzerIndex     : 3; /* 0 mean mirrorToRxAnalyzerPort is disabled*/
/* aligned to 32 bits */

    GT_U32 ipclConfigIndex                  : 13;
    GT_U32 copyReserved                     : 19;
/* aligned to 32 bits */

    GT_U32 copyReservedAssignEnable         : 1;
    GT_U32 mirrorTcpRstAndFinPacketsToCpu   : 1;
    GT_U32 modifyDscp                       : 2;
    GT_U32 modifyUp                         : 2;
    GT_U32 profileIndex                     : 10;
    GT_U32 profileAssignIndex               : 1;
    GT_U32 profilePrecedence                : 1;
    GT_U32 up1Cmd                           : 2;
    GT_U32 up1                              : 3;
    GT_U32 modifyMacDa                      : 1;
    GT_U32 modifyMacSa                      : 1;
    GT_U32 setMacToMe                       : 1;
    GT_U32 skipFdbSaLookup                  : 1;
    GT_U32 triggerInterrupt                 : 1;
    GT_U32 vntL2Echo                        : 1;
    GT_U32 pcl1OverrideConfigIndex          : 1;
    GT_U32 monitoringEnable                 : 1;
    GT_U32 pbrMode                          : 1;
/* aligned to 32 bits */

    /* the 'union' open new 32 bits alignment */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INTERFACE_INFO_UNT outInterface;
/* aligned to 32 bits */

    GT_U32 tunnelType                       : 1;
    GT_U32 tunnelStart                      : 1; /* indication if redirectPtr used as TS pointer or as ARP pointer */
    GT_U32 redirectPtr                      : 18;/* treat this field as:
                                                    routerLttPtr 18 when Redirect_Command="IPNextHop"
                                                    vrfId 11 bits when Redirect_Command="Assign_VRF_ID"
                                                    16 bits for TS when Redirect_Command="Egress_Interface" and TunnelStart="True"
                                                    18 bits for ARP when Redirect_Command="Egress_Interface" and TunnelStart="false" */

    GT_U32 sourceIdValue                    : 12;
/* aligned to 32 bits */

    /* the mac address (48 bits) */
    GT_U32  macSa_low_32;     /* in network order those bits hold are the bytes [2..5] */
                              /* in terms of the device those bits hold bits 31..0     */
/* aligned to 32 bits */
    GT_U32  macSa_high_16 :16;/* in network order those bits hold are the bytes [0..1] */
                                /* in terms of the device those bits hold bits 47..32    */

    GT_U32 assignSourceId                   : 1;
    GT_U32 assignSourcePortEnable           : 1;
    GT_U32 sourcePortValue                  : 14;
/* aligned to 32 bits */

    GT_U32 nestedVlan                      : 1;
    GT_U32 vlanId                          : 13;
    GT_U32 precedence                      : 1;
    GT_U32 vlanId1Cmd                      : 2;
    GT_U32 vlanId1                         : 12;
    GT_U32 outInterface_type               : 3; /* CPSS_INTERFACE_TYPE_ENT */
/* aligned to 32 bits */

    GT_U32 redirectCmd                     : 4;
    GT_U32 modifyVlan                      : 2;
    GT_U32 latencyProfile                  : 9;
    GT_U32 vrfId                           : 12; /* This filed holds vrfId 12 bits when
                                                    Redirect_Command = ROUTER_AND_ASSIGN_VRF_ID or ECMP_AND_ASSIGN_VRF_ID */
    GT_U32 triggerHashCncClient            : 1;
    GT_U32 pcl0_1OverrideConfigIndex       : 1;

    GT_U32 actionStop                      : 1;
    GT_U32 bypassBridge                    : 1;
    GT_U32 bypassIngressPipe               : 1;
/* aligned to 32 bits */

    GT_U32 cpuCode                         : 8;
    GT_U32 exactMatchOverPclEn             : 1;
/*left with 'spare' of : 32-(8+1) = 23 bits */

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC
*
*  @brief internal Exact Match manager DB (database) Entry
*       format matching :
*       PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_EPCL_E
*
*/
typedef struct{
    GT_U32 bindToCncCounter                 : 1;/* enableMatchCount */
    GT_U32 cncCounterIndex                  : 15;/*matchCounterIndex*/
    GT_U32 bindToPolicerMeter               : 1;
    GT_U32 bindToPolicerCounter             : 1;
    GT_U32 policerIndex                     : 14;
/* aligned to 32 bits */

    GT_U32 oamTimeStampEnable               : 1;
    GT_U32 oamOffsetIndex                   : 7;
    GT_U32 oamProcessEnable                 : 1;
    GT_U32 oamProfile                       : 1;
    GT_U32 flowId                           : 16;
    GT_U32 egressMirrorToAnalyzerIndex      : 3;/* 0 mean mirrorToTxAnalyzerPortEn is disabled*/
    GT_U32 egressMirrorToAnalyzerMode       : 2;
    GT_U32 copyReservedAssignEnable         : 1;
/* aligned to 32 bits */

    GT_U32 copyReserved                     : 19;
    GT_U32 modifyUp                         : 2;
    GT_U32 up                               : 3;
    GT_U32 up1ModifyEnable                  : 1;
    GT_U32 up1                              : 3;
    GT_U32 vlanCmd                          : 2;
    GT_U32 vlanId1ModifyEnable              : 2;
/* aligned to 32 bits */

    GT_U32 vlanId                           : 12;
    GT_U32 vlanId1                          : 12;

    GT_U32 phaThreadNumber                  : 8;/* phaThreadId */ /* look at epclPhaInfo for logic*/
/* aligned to 32 bits */
    GT_U32 phaMetadata                      : 32;/* look logic on phaThreadType, phaThreadUnion*/
/* aligned to 32 bits */
    GT_U32 phaThreadNumberAssignmentEnable  : 1;/* phaThreadIdAssignmentMode*/
    GT_U32 phaMetadataAssignEnable          : 1;
    GT_U32 monitoringEnable                 : 1;
    GT_U32 latencyProfile                   : 9;
    GT_U32 phaThreadType                    : 3;
    GT_U32 terminateCutThroughMode          : 1;
    GT_U32 cpuCode                          : 8;
    GT_U32 modifyDscp                       : 2;
    GT_U32 dscp                             : 6;
/* aligned to 32 bits */

    GT_U32 exactMatchOverPclEn              : 1;
    GT_U32 channelTypeToOpcodeMapEnable     : 1;
    GT_U32 pktCmd                           : 3;
    GT_U32 egressCncIndexMode               : 1;
    GT_U32 enableEgressMaxSduSizeCheck      : 1;
    GT_U32 egressMaxSduSizeProfile          : 2;
/*left with 'spare' of : 32-(1+1+3+1+1+2) = 23 bits */

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC;

/* NOTE: we not use 'real pointers' because :
    1. in 64 bits CPU the sizeof(pointer) is 8 bytes while in 32bits CPU it is 4 bytes
    2. and event in 32 bits CPU we can 'compress' it when several pointers needed
       in the same entry
*/
/* 20 bits index - supporting 1M values */
#define ENTRY_PTR_H_NUM_BITS 10 /* 10 bits 'high' (MSBits) */
#define ENTRY_PTR_L_NUM_BITS 10 /* 10 bits 'Low' (LSBits) */
#define ENTRY_PTR_NUM_BITS   (ENTRY_PTR_L_NUM_BITS + ENTRY_PTR_H_NUM_BITS)
/* in many cases should be use as :
    ENTRY_PTR nextEntryPointer : ENTRY_PTR_NUM_BITS;

    in order to 'compress' the 'pointer'
*/
typedef GT_U32  ENTRY_PTR;
typedef GT_U32  ENTRY_PTR_H;
typedef GT_U32  ENTRY_PTR_L;

#define EM_SET_nextEntryPointer_MAC(dbPtr,index) \
    dbPtr->nextEntryPointer_high = (index >> ENTRY_PTR_L_NUM_BITS); \
    dbPtr->nextEntryPointer_low  = (index)/* compiler will chop the extra bits anyway ! */

#define EM_GET_nextEntryPointer_MAC(dbPtr,index) \
    index = (dbPtr->nextEntryPointer_high << ENTRY_PTR_L_NUM_BITS) | \
            dbPtr->nextEntryPointer_low

/* 1 bit field  */
#define GT_BIT_NUM_BITS 1
/* single bit to indicate */
typedef GT_U32  GT_BIT;

/* 20 bits index - supporting 1M values */
#define HW_INDEX_NUM_BITS 20

/* num of bits to the 'age bin' */
#define AGE_BIN_INDEX_NUM_BITS (ENTRY_PTR_NUM_BITS - 8) /* 8 comes from 2^8 = 256 = PRV_EXACT_MATCH_MANAGER_NUM_ENTRIES_STEPS_CNS */

/* the specific 'entry format' */
typedef union{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC    prvTtiEntryFormat;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC    prvPclEntryFormat;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC   prvEpclEntryFormat;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_SPECIFIC_FORMAT;


/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC
*
* @brief internal Exact Match manager DB (database) Entry format
*
* NOTE: the format expected to be 'as compressed as possible'
*/
typedef struct{
    /** HW index of the entry */
    GT_U32                     hwIndex               : HW_INDEX_NUM_BITS/*20*/;
    /** Age-list (age-bin) number */
    GT_U32                     ageBinIndex           : AGE_BIN_INDEX_NUM_BITS/*12*/;
/* aligned to 32 bits */
    /** 'entry pointer' to the previous entry */
    GT_BIT                     isValid_prevEntryPointer : GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  prevEntryPointer         : ENTRY_PTR_NUM_BITS/*20*/;

    /** 'entry pointer' to the next     entry */
    GT_BIT                     isValid_nextEntryPointer : GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR_H                nextEntryPointer_high    : ENTRY_PTR_H_NUM_BITS/*10*/;
/* aligned to 32 bits */
    ENTRY_PTR_L                nextEntryPointer_low     : ENTRY_PTR_L_NUM_BITS/*10*/;

    /** 'entry pointer' to the previous entry in 'age-list' */
    GT_BIT                     isValid_age_prevEntryPointer : GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  age_prevEntryPointer     : ENTRY_PTR_NUM_BITS/*20*/;

    /** 'entry pointer' to the next     entry in 'age-list' */
    GT_BIT                     isValid_age_nextEntryPointer : GT_BIT_NUM_BITS/*1*/;
/* aligned to 32 bits */
    ENTRY_PTR                  age_nextEntryPointer     : ENTRY_PTR_NUM_BITS/*20*/;

    GT_BIT                     isUsedEntry           :GT_BIT_NUM_BITS/*1*/;

    /** indication about the format of the 'HW' , and about 'specificFormat' ,
     *  value is one of : CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT */
    GT_U32                     hwExactMatchKeySize   : 2;

    /** indication about the format of the 'HW' , and about 'specificFormat' ,
     *  value is one of : CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT */
    GT_U32                     hwExactMatchLookupNum   : 1;

   /** indication about the format of the 'HW' , and about 'specificFormat' ,
    *   value is one of 0-15 */
    GT_U32                     hwExactMatchExpandedActionIndex   : 4;

    /* metadata common to all entries */
    /** indication about the format of the 'HW' , and about 'specificFormat' ,
     *  value is one of :
     *  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_ENT */
    GT_U32                     hwExactMatchEntryType        : 2;

    /* isFirst == 1 indicateds that this index holds valid data.
       For example for key size 33B (index 45,46,47)
       hwIndex = 45, isFirst = 1.
       hwIndex = 46, isFirst = 0 (key&action parameters are not relevant).
       hwIndex = 47, isFirst = 0 (key&action parameters are not relevant).
       */
    GT_U32                     isFirst                      :1;
    /** 1 bits pleceHolder - can be replaced in the future by a
     *  real 1 bits value */
    GT_U32                     pleceHolder_1bits            : 1;
/* aligned to 32 bits  */

    /** userDefined value used to specify the entry with a special value
       this value will be used for age_scan and delete_scan
       this is a SW database configuration
       SUPPORTED VALUES : 0..65535 (16 bits)               */
    GT_U32                     exactMatchUserDefined        : 16;/* SW field only, no HW related data */

    GT_U32                     age                          : 1;/* age bit : set to 1 when adding new entry
                                                                             set to 1 when rewriting an entry in SER mechanism */
    /** 15 bits pleceHolder - can be replaced in the future by a
     *  real 15 bits value */
    GT_U32                     pleceHolder_15bits           : 15;

/* aligned to 32 bits  */

    GT_U8 pattern[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    /* the 'entry format' */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_SPECIFIC_FORMAT  specificFormat;

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC;

/**
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC
 *
 * @brief internal Exact Match manager DB (database) 'index' format
 *
 * NOTE: the format expected to be 'as compressed as possible'
*/
typedef struct{
    /** 'entry pointer' */
    GT_BIT                     isValid_entryPointer: GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  entryPointer        : ENTRY_PTR_NUM_BITS/*20*/;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC;


/**
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC
 *
 * @brief internal Exact Match manager DB (database) 'pointer to node' in the DB
 *
*/
typedef struct{
    /** 'entry pointer' */
    GT_BIT                     isValid_entryPointer: GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  entryPointer        : ENTRY_PTR_NUM_BITS/*20*/;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC;

typedef struct{
    /* temp place to hold the info for:
       cpssDxChExactMatchPortGroupEntrySet(...) :
       format of the HW needed for the entry updates */
    GT_U32    exactMatchHwActionArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* the full Hw representation for actionPtr */
    GT_U8     exactMatchHwReducedActionDataArr[PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS];/* the final 8 bytes to set the HW */
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC exactMatchEntry;                             /* keySize + pattern +lookupNum */
    GT_U32    hwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS];           /* full hw data to set to HW:
                                                                                    entryType/Age/keySize/lookupNum/key/reduccedAction/ */
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_TEMP_OUT_OF_STACK_INFO_STC;

/**
 * @enum PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_BIN_STC
 *
 * @brief Exact Match Manager aging bin
*/
typedef struct{
    /** @brief indicates age-bin's occupancy level */
    GT_U32                                              totalUsedEntriesCnt;

    /** @brief indicates age-bin's starting entry pointer */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC   headOfAgeBinEntryList;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_BIN_STC;

/**
* @enum PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC
*
* @brief Exact Match Manager aging bin related info
*         These structures are allocated at the time of Exact
*         Match instance creation and freed at the time of
*         delete instance
*/
typedef struct{
    /** @brief indicates the age-bin ID to be scanned,
     *         aging scan API updates this at the end of each aging process*/
    GT_U32     currentAgingScanBinID;

    /** @brief level1 age bin usage matrix
     *         Range [0-128 Bytes] */
    GT_U32    *level1UsageBitmapPtr;

    /** @brief level2 age bin usage matrix
     *         Range [0-4 Bytes] */
    GT_U32    level2UsageBitmap;

    /** @brief indicates the number of age-bins created at the manager creation time
     *         Range [0-1024] */
    GT_U32     totalAgeBinAllocated;

    /** @brief list of age bins, array index is the age-bin ID.
     *         Range [0-1024] */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_BIN_STC *ageBinListPtr;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC;


/*
 * @enum PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_ENT
 *
 * @brief internal Exact Match manager DB (database) hash calc result for existing/new entry
 *
*/
typedef enum{
    /** @brief there is free place in HASH for Exact Match entry */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E,
    /** @brief the Exact Match entry was found but there is no free place in HASH */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E,
    /** @brief neither Exact Match entry no free place was found in HASH */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_ENT;

/*
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC
 *
 * @brief internal Exact Match manager DB (database) hash calc info for existing/new entry
 *
*/
typedef struct{
    /** the final result of the 'lookup' in the DB */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_ENT calcEntryType;
    /** if found this is the pointer to the entry in the DB */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *dbEntryPtr;
    /** number of valid elements in the crcMultiHashArr */
    GT_U32                              numberOfElemInCrcMultiHashArr;
     /** the hash results for the key of the entry */
    GT_U32                              crcMultiHashArr[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS];
    /** the banks that are already used in the DB , and can't be used for new entry
        the array filled with info for indexes : 0..lastBankIdChecked (inclusive)
    */
    GT_U32                              usedBanksArr[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS];
    /** the last iteration that the 'usedBanksArr' was filled with.
        NOTE: when entry not found this value will hold :
              (exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes - 1) */
    GT_U32                              lastBankIdChecked;
    /** the bankId that on which we found existing entry , or on which we want
        to use for new entry after we checked for 'most populated' bank */
    GT_U32                              selectedBankId;

    /**  the step  between banks depending on the key size
     *   KEY_SIZE_5B = 1 step, KEY_SIZE_19B = 2 steps,
     *   KEY_SIZE_33B = 3 steps, KEY_SIZE_47B = 4 steps */
    GT_U32                              bankStep;
    /** type of entry for statistics */
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT        counterClientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT      counterKeyType;

    /** depth of Cuckoo -- needed for statistics */
    GT_U32                              rehashStageId;

    /** the bank with exactMatchEntry on wich rehashing should be
     *  applied */
    GT_U32                              rehashStageBank;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC;

/*
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_REHASHING_INFO_STC
 *
 * @brief internal structure holds info of rehashing Exact Match entry.
 *        Used by 'cuckoo' algorithm to relocate entries in Exact Match manger database and HW.
 *
*/
typedef struct {
    /** @brief Exact Match entry needed relocation */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    /** @brief Recalculated hash info for entry to be relocated */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfo;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_REHASHING_INFO_STC;


/**
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC
 *
 * @brief Defines the Exact Match Expanded entry
 *
*/
typedef struct{
    /** GT_TRUE:  entry is valid
     *  GT_FALSE: entry was not configured */
    GT_BOOL exactMatchExpandedEntryValid;

    /** Exact Match Expanded entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC exactMatchExpandedEntry;

    /** number of Exact Match entries pointing to expander entry */
    GT_U32 exactMatchEntriesCounter;

    /** Reduced mask to be used when configuring the Exact Match Entry.
     */
    GT_U32   reducedMaskArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC;

/**
* @enum PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_ENT
 *
 * @brief Enumeration of Exact Match Table size in entries
*/
    typedef enum{

    /** 8K Exact Match table entries. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E = 0,

    /** 16K Exact Match table entries. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E,

    /** 32K Exact Match table entries. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E,

    /** 64K Exact Match table entries. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E,

    /** 128K Exact Match table entries. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E,

    /** 256K Exact Match table entries. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E

}PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_ENT;

/**
* @struct CPSS_DXCH_EXACT_MATCH_HASH_PARAMS_STC
 *
*  @brief struct contains the parameters for Exact Match table
*         hash calculation.
*/
typedef struct{

    /** the entries number in the Exact Match table. */
    PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_ENT size;

} PRV_CPSS_DXCH_EXACT_MATCH_HASH_PARAMS_STC;

/**
* @enum PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_ENTRY_STC
*
* @brief Exact Match Entry in the HA temporary DB
*/
typedef struct{
    /** @brief
     *  exact match entry actionType
     * */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   exactMatchActionType;
     /** @brief
     *  exact match entry structure
     * */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT        exactMatchAction;
     /** @brief
     *  exact match entry key parameters
     * */
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC         exactMatchEntry;
     /** @brief
     *  exact match entry expanded action index
     * */
    GT_U32                                  expandedActionIndex;

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_ENTRY_STC;

/**
* @enum PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_STC
*
* @brief Exact Match Manager HA temporary DB
*/
typedef struct{
    /** @brief
     *  GT_TRUE: the entry was already taken in HA replay process
     *  GT_FALSE: the entry is free and was not taken yet by the  HA replay process
     * */
    GT_BOOL    entryWasTaken;

    /** @brief
     *  GT_TRUE: the entry is a valid entry in the HW
     *  GT_FALSE: the entry is NOT a valid entry in the HW */
    GT_BOOL    isValid;

    /** @brief pointer to the exact match entry */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_ENTRY_STC *exactMatchEntryPtr;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_STC;


/**
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC
 *
 * @brief internal Exact Match manager 'per' instance info
 *      dynamically allocated during
 *      cpssDxChExactMatchManagerCreate(...) and freed during
 *      cpssDxChExactMatchManagerDelete(...)
*/
typedef struct{
    /* 'external' info from cpssDxChExactMatchManagerCreate(...) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          capacityInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            lookupInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             agingInfo;

    /* Exact Match Expander table */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC   exactMatchExpanderArray[CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS];

    /** internal info */
    /* the 'pool of entries' according to size : capacityInfo.maxTotalEntries
       dynamic allocated during 'create manager' and released during 'delete manager'

       index to this array is 'DB index' (0..capacityInfo.maxTotalEntries-1)
    */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *entryPoolPtr;

    /* the 'index array' according to size : capacityInfo.hwCapacity.numOfHwIndexes
       dynamic allocated during 'create manager' and released during 'delete manager'

       index to this array is 'HW index' (0..capacityInfo.hwCapacity.numOfHwIndexes-1)
    */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC          *indexArr;

    /* 'semi pointer' to head of free 'list' of entries in entryPoolPtr */
    /* the head hold the last one that was freed */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       headOfFreeList;
    /* 'semi pointer' to head of used 'list' of entries in entryPoolPtr */
    /* the head hold the 'oldest' entry added to 'used' list */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       headOfUsedList;
    /* 'semi pointer' to tail of used 'list' of entries in entryPoolPtr */
    /* the tail hold the 'newest' entry added to 'used' list */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       tailOfUsedList;

    /* values calculated once instead of every time need to do hash calculations */
    PRV_CPSS_DXCH_EXACT_MATCH_HASH_PARAMS_STC               cpssHashParams;

    /* info about the banks : counters , etc... */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_BANK_INFO_STC         bankInfoArr[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS];

    /* number of HW indexes per bank = numOfHwIndexes/numOfHashes */
    /* calculated once ... used many times */
    GT_U32                                          maxPopulationPerBank;

    /* number of entries in the Exact Match manager (must be in range : 0.. maxTotalEntries) */
    GT_U32                                          totalPopulation;

    /* the counters per entry type */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_TYPE_COUNTERS_STC typeCountersArr[CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E][CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];

    /* statistics about the 'entry add'     API cpssDxChExactMatchManagerEntryAdd(...) */
    GT_U32                                          apiEntryAddStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    /* statistics about the 'entry delete'  API cpssDxChExactMatchManagerEntryDelete(...) */
    GT_U32                                          apiEntryDeleteStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E];
    /* statistics about the 'entry update'  API cpssDxChExactMatchManagerEntryUpdate(...) */
    GT_U32                                          apiEntryUpdateStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E];
    /* statistics about the 'Learning scan' API cpssDxChExactMatchManagerDeleteScan(...) */
    GT_U32                                          apiDeleteScanStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E];
    /* statistics about the 'Learning scan' API cpssDxChExactMatchManagerAgingScan(...) */
    GT_U32                                          apiAgingScanStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E];
    /* statistics about the 'Learning scan' API cpssDxChExactMatchManagerEntryRewrite(...) */
    GT_U32                                          apiEntriesRewriteStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E];

    /* bitmap of devices+portGroups registered to the manager */
    GT_U32                                          devsBmpArr[NUM_WORDS_DEVS_BMP_CNS];
    GT_PORT_GROUPS_BMP                              portGroupsBmpArr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    /* number of devices registered to the manager */
    GT_U32                                          numOfDevices;

    /** temp place to hold extra info temporarily needed by implementation of APIs */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_TEMP_OUT_OF_STACK_INFO_STC   tempInfo;

    /** define the supported 'SIP' for the manager , so only relevant devices can registered to it */
    GT_U32                                          supportedSipArr[CPSS_GEN_SIP_BMP_MAX_NUM_CNS];

    /** scan pointers : getNext , delete */
    /* 'semi pointer' to getNext 'last' entry in entryPoolPtr */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       lastGetNextInfo;
    /* 'semi pointer' to delete 'last' entry in entryPoolPtr */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       lastDeleteInfo;

    /** sanity manager of the number of 'malloc' pointers that are associated
        with this manager
        -- incremented for every 'malloc' related to this manager.
        -- decremented for every 'free' related to this manager.
        -- checked by the 'manager delete' that we are ZERO at the end of operation.
    */
    GT_U32                                          numOfAllocatedPointers;

    /** @brief the aging bin related info */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC agingBinInfo;

    /** @brief Self test restore info */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_RESTORE_INFO_UNT selfTestRestoreInfo[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];

    /** @brief indicates the test cases, which are currently error state.
      *        (injected byprvCpssDxChExactMatchManagerDebugSelfTest)
      *        Range [0-3 Bytes] */
    GT_U32    selfTestBitmap;

    /* HA temporary array database - will hold all HW Exact Match entries */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_STC     *haTempDbArr;

}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC;

/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryTtiFormat function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
* Note:
* save values that will be set to HW. use ttiActionType2Hw2LogicFormat as reference
*/
GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryTtiFormat
(
    IN  CPSS_DXCH_TTI_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC   *dbEntryPtr
);
/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryPclFormat function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
* Note:
* save values that will be set to HW. use sip5PclTcamRuleIngressActionHw2SwConvert as reference
*/
GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryPclFormat
(
    IN  CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC   *dbEntryPtr
);
/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryEpclFormat function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
* * Note:
* save values that will be set to HW. use sip5PclTcamRuleEgressActionHw2SwConvert as reference
*/
GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryEpclFormat
(
    IN  CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC  *dbEntryPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbValidateTtiAction function
* @endinternal
*
* @brief   validate the Exact Match TTI Action
*
* @param[in] logicFormatPtr        - points to TTI action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChExactMatchManagerDbValidateTtiAction
(
    IN  CPSS_DXCH_TTI_ACTION_STC   *logicFormatPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbValidatePclAction function
* @endinternal
*
* @brief   validate the Exact Match PCL/EPCL Action
*
* @param[in] logicFormatPtr        - points to PCL action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChExactMatchManagerDbValidatePclAction
(
    IN  CPSS_DXCH_PCL_ACTION_STC   *logicFormatPtr
);
/**
* @internal prvCpssDxChExactMatchManagerDbCreate function
* @endinternal
*
* @brief The function creates the Exact Match Manager and its databases according to
*        input capacity structure. Internally all ExactMatch global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
*        NOTE: the function doing only DB operations (no HW operations)
*
*        NOTE: incase of 'error' : if the manager was allocated the caller must
*           call prvCpssDxChExactMatchManagerDbDelete(...) to 'undo' the partial create.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs ExactMatchs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As Exact Match Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries*
* @param[in] entryAttrPtr          - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
* @param[out] managerAllocatedPtr  - (pointer to) indication that the manager was allocated.
*                                    so on error need to do 'clean-up'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the Exact Match Manager id already exists.
*/
GT_STATUS prvCpssDxChExactMatchManagerDbCreate
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              *agingPtr,
    OUT GT_BOOL                                             *managerAllocatedPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbDelete function
* @endinternal
*
* @brief  The function de-allocates specific Exact Match Manager
*         Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                   (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDelete
(
    IN GT_U32 exactMatchManagerId
);
/**
* @internal prvCpssDxChExactMatchManagerAllDbDelete function
* @endinternal
*
* @brief  The function de-allocates All Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager
*                                    id is not initialized.
*/
GT_STATUS prvCpssDxChExactMatchManagerAllDbDelete();

/**
* @internal prvCpssDxChExactMatchManagerDbConfigGet function
* @endinternal
*
* @brief The function fetches current Exact Match Manager setting as they were defined in Exact Match Manager Create.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed
*                                    PPs EMs: like number of indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity. In Falcon, for
*                                    example, we must know the shared memory allocation mode.
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries
* @param[out] entryAttrPtr         - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to
*          specific Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
);

/**
* @internal prvCpssDxChExactMatchManagerDbDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from
*          specific Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to remove from the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDevListRemove
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
);

/**
* @internal prvCpssDxChExactMatchManagerDbIsAnyDeviceRegistred function
* @endinternal
*
* @brief   This function check if is there are registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr - (pointer to) the Exact Match manager.
*
* @retval 0 - no device registered.
* @retval 1 - at least one device registered.
*/
GT_U32 prvCpssDxChExactMatchManagerDbIsAnyDeviceRegistred
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC     *exactMatchManagerPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the 'current device' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the 'first/next device'
* @param[out] portGroupBmpPtr      - (pointer to) the 'first/next device portGroupBmp'
*
* @retval GT_OK - there is 'first/next device'
* @retval GT_NO_MORE - there are no more devices
*                if isFirst == GT_TRUE --> no devices at all
*                else                  --> no more devices after 'current' (IN *devNumPtr)
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN  GT_BOOL                                                     isFirst,
    INOUT GT_U8                                                     *devNumPtr,
    OUT GT_PORT_GROUPS_BMP                                          *portGroupBmpPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbEntryIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next valid (pointer to) entry from the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the '(pointer to)current entry' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the '(pointer to)first/next entry'
*
*
* @retval GT_OK - there is 'first/next valid entry' from the manager
* @retval GT_NO_MORE - there are no more entries
*                      if isFirst == GT_TRUE --> no entries at all
*                      else                  --> no more entries after 'current' (IN *dbEntryPtrPtr)
*         GT_BAD_STATE - there is error in DB management.
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbEntryIteratorGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN  GT_BOOL                                                     isFirst,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC*    *dbEntryPtrPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult function
* @endinternal
*
* @brief   This function hash calculate place for the new/existing entry.
*          the HASH info filled into calcInfoPtr , with indication if entry found ,
*          or not found and no place for it ,
*          or not found but have place for it.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] addParamsPtr          - (pointer to) info given for 'entry add' that
*                                     may use specific bank from the hash.
*                                     NOTE: ignored if NULL
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC           *addParamsPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbGetMostPopulatedBankId function
* @endinternal
*
* @brief   This function choose the best bank to use for the entry.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->usedBanksArr[] ,
*                                    using calcInfoPtr->lastBankIdChecked
* @param[out] calcInfoPtr          - updating calcInfoPtr->selectedBankId
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not found bank to use
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbGetMostPopulatedBankId
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbAddNewEntry function
* @endinternal
*
* @brief   This function get free entry to be used for new added entry.
*           adding new entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr as valid pointer
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
* @param[out] calcInfoPtr          - updating calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbAddNewEntry
(
    IN    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC    *exactMatchManagerPtr,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbDeleteOldEntry function
* @endinternal
*
* @brief   This function return entry to the free list removing from the used list.
*           delete old entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr about valid pointers
*           lastGetNextInfo , lastTranslplantInfo , lastDeleteInfo
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->dbEntryPtr ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbDeleteOldEntry
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbTableFullDiagnostic function
* @endinternal
*
* @brief   This function do diagnostic to 'Exact Match full'
*          state the caller function will return this error to
*          the application NOTE: the function always return
*          'ERROR'. the 'GT_FULL' is expected ,
*                other values are not
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
*
* @retval GT_BAD_STATE             - non-consistency between different parts of
*                                    the manager.
* @retval GT_FULL                  - the table is really FULL (all banks are full).
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbTableFullDiagnostic
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) entry.
*          it is based on 'internally managed last entry' that the manager
*          returned 'last time' to the application.
*
*          NOTE: this function is to be used only for APPLICATION specific 'get next'
*                and not for iterators inside the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] getFirst              - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node fetched by this scan
*                                    OR - DB pointer from where to start the next search.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    IN GT_BOOL                                                  getFirst,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC    *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC  **dbEntryPtrPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbIsEntryKeyMatchDbEntryKey function
* @endinternal
*
* @brief   The function check if the key of the entry from application (entryPtr)
*          match the key of the entry from the manager DB (dbEntryPtr).
*          the result retrieved in (*isMatchPtr)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] dbEntryPtr            - (pointer to) the entry (manager format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] isMatchPtr           - (pointer to) is the key match ?
*                                    GT_TRUE  - the key     match
*                                    GT_FALSE - the key not match
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbIsEntryKeyMatchDbEntryKey
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr,
    OUT GT_BOOL                                                     *isMatchPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbDevListGet function
* @endinternal
*
* @brief The function returns current Exact Match Manager registered devices (added by 'add device' API).
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPairsPtr      - (pointer to) the number of devices in the array pairListArr as 'input parameter'
*                                    and actual number of pairs as 'output parameter'.
* @param[out] pairListManagedArray[]- array of registered pairs (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDevListGet
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]/*arrSizeVarName=*numOfPairsPtr*/
);

/**
* @internal prvCpssDxChExactMatchManagerDbExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[in] paramsPtr             - the expanded action attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
GT_STATUS prvCpssDxChExactMatchManagerDbExpandedActionUpdate
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbExpandedActionGet function
* @endinternal
*
* @brief  The function set the Expanded Action table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[out] paramsPtr             - the expanded action attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*  None
*/
GT_STATUS prvCpssDxChExactMatchManagerDbExpandedActionGet
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);

/*
* @internal prvCpssDxChExactMatchManagerDbCountersGet function
* @endinternal
*
* @brief The function fetches Exact Match Manager counters.
*
*
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) Exact Match Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbCountersGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC      *countersPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbStatisticsGet function
* @endinternal
*
* @brief The function fetches Exact Match Manager statistics.
*
*
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) Exact Match Manager Statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbStatisticsClear function
* @endinternal
*
* @brief The function clears Exact Match Manager statistics.
*
*
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbStatisticsClear
(
    IN GT_U32       exactMatchManagerId
);

/**
* @internal prvCpssDxChExactMatchManagerDbCheck function
* @endinternal
*
* @brief The function verifies the Exact Match manager DB.
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in]  checksPtr            - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbCheck
(
    IN  GT_U32                                              exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC         *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  resultArray[],
    OUT GT_U32                                              *errorNumPtr
);

/*
* @internal prvCpssDxChExactMatchManagerDebugSelfTest function
* @endinternal
*
* @brief The function injects/corrects error in Exact Match manager DB, as mentioned in expResultArray
*
* @param[in] exactMatchManagerId  - The Exact Match Manager id.
*                                   (APPLICABLE RANGES : 0..31)
* @param[in] expResultArray       - (pointer to) Test error types array.
* @param[in] errorNumPtr          - (pointer to) Number of error types filled in expResultArray
* @param[in] errorEnable          - Status value indicates error/correct state
*                                   GT_TRUE  - Error need to injected according to expResultArray and expErrorNum
*                                   GT_FALSE - Error need to be corrected according to expResultArray and expErrorNum
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDebugSelfTest
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT   expResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    IN GT_U32                                               expErrorNum,
    IN GT_BOOL                                              errorEnable
);

/**
* @internal prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) age-bin entry.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[in] getFirst              - indication to get First (GT_TRUE) or the next (GT_FALSE)
* @param[in] ageBinID              - The age-bin ID to scan for entries.
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node in the ageBin fetched by this agingscan
*                                    OR - DB pointer from where to start the next search in the ageBin.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next age-bin entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_BOOL                                                  getFirst,
    IN GT_U32                                                   ageBinID,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC     *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   **dbEntryPtrPtr
);

/**
* @internal prvCpssDxChExactMatchManagerRehashBankEntryGet function
* @endinternal
*
* @brief   This function retrieves Exact Match entry from
*          calcInfo per bank from HASH calculated in
*          prvCpssDxChExactMatchManagerDbHashCalculate.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match manager
* @param[in] bankId                 - bank ID to get rehashing calc info
* @param[in] calcInfoPtr            - (pointer to) the calculated hash and lookup results
* @param[out] entryPtr              - (pointer to) the current Exact Match entry that need to be rehashed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChExactMatchManagerRehashBankEntryGet
(
    IN      PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr,
    IN      GT_U32                                                      bankId,
    IN      PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr,
    OUT     CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     *entryPtr
);

/**
* @internal prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb function
* @endinternal
*
* @brief  function to restore values of the Exact Match entry
*         from DB copy values from manager format to application
*         format the 'opposite' logic of
*         saveEntryFormatParamsToDb(...)
*
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 *entryPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbRehashEntry function
* @endinternal
*
* @brief   This function re-hash index for the new entry that can't be added to HW
*          when all banks for the current hash index are occupied.
*
*          NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] rehashStageId         - rehashing stage level
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on operation fail
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbRehashEntry
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN GT_U32                                                       rehashStageId,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
);

/**
* @internal prvCpssDxChExactMatchManagerDbEntryReadByHwIndex
*           function
* @endinternal
*
* @brief   This function retrieves Exact Match entry from
*          DB in specific hwIndex.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match manager
* @param[in] hwIndex                - hardware index to read from
* @param[out] entryPtr              - (pointer to) Exact Match entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChExactMatchManagerDbEntryReadByHwIndex
(
    IN    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN    GT_U32                                                    hwIndex,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                   *entryPtr
);

/*
* @internal prvCpssDxChExactMatchManagerDbHashCalculate function
* @endinternal
*
* @brief   This function calculates hash for the new/existing entry.
*
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbHashCalculate
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr
);

/*
* @internal prvCpssDxChExactMatchManagerBankCalcInfoGet function
* @endinternal
*
* @brief   This function gets DB calc info per bank in hash calculated in prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match manager
* @param[in] bankId                 - bank ID to get calc info
* @param[in] entryPtr               - (pointer to) the current Exact Match entry need to be rehashed
* @param[in] addParamsPtr           - (pointer to) parameters used for rehash
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChExactMatchManagerBankCalcInfoGet
(
    IN    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN    GT_U32                                                    bankId,
    IN    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                   *entryPtr,
    IN    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        *addParamsPtr,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
);

/*
* @internal prvCpssDxChExactMatchManagerDbFindValidIndexInHaTempDb function
* @endinternal
*
* @brief   This function uses result from function find if a given index exist in the HW.
*          The lookup is done in the HW tempDb, read when the device was added to the
*          manager, in the HA process.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)*
* @param[in] bankId                - the bankId were we can locate the index we are looking for
* @param[inout] calcInfoPtr        - (pointer to) the calculated hash and lookup results
* @param[out]foundValidEntryPtr    - (pointer to) GT_TRUE: a valid HW entry was found
*                                                 GT_FALSE: no valid HW entry was found
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbFindValidIndexInHaTempDb
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    IN GT_U32                                                       bankId,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr,
    OUT GT_BOOL                                                     *foundValidEntryPtr
);


/**
* @internal prvCpssDxChExactMatchManagerDbSetIndexInHwTempDbAsTaken function
* @endinternal
*
* @brief   This function set a given index in the HW tempDb as taken
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - (pointer to) calcInfo hold all information
*                                    regarding the Index value taken from the HW tempDb
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbSetIndexInHwTempDbAsTaken
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
);

/**
* @internal prvCpssDxChExactMatchManagerCompletionForHa function
* @endinternal
*
* @brief  this function goes over all the managers defined in
*         the system, and for each one, clean all the entries
*         that were not marked as used in the HA temporary DB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChExactMatchManagerCompletionForHa
(
    GT_VOID
);

/**
* @internal prvCpssDxChExactMatchManagerLibraryShutdown function
* @endinternal
*
* @brief   Shutdown Exact Match Manager Library.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on Exact Match Manager not found
*/
GT_STATUS prvCpssDxChExactMatchManagerLibraryShutdown
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChExactMatchManager_db_h */


