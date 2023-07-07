// xpsAcl.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsAcl.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Ingress ACL Manager
 */

#ifndef _XPSACL_H_
#define _XPSACL_H_

#include "xpTypes.h"
#include "cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h"
#include "cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPS_ACL_MAX_TBLS_IN_GROUP 4
#define MAX_UDB_SUPPORT           10
#define XP_ACL_TABLE_MAX_IDS      32
#define XP_ACL_TABLE_RANGE_START  0

#define XPS_EQUAL_PRIORITY_RULE_POSITION_FIRST_E  CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E
#define XPS_EQUAL_PRIORITY_RULE_POSITION_LAST_E   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E

#define XPS_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
#define XPS_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E
#define XPS_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E
#define XPS_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
#define XPS_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
#define XPS_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
#define XPS_PCL_RULE_FORMAT_INGRESS_UDB_30_E      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
#define XPS_PCL_RULE_FORMAT_INGRESS_UDB_60_E      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
#define XPS_PCL_RULE_FORMAT_EGRESS_UDB_30_E       CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
#define XPS_PCL_RULE_FORMAT_EGRESS_UDB_60_E       CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E

#define XPS_PACKET_CMD_FORWARD_E       CPSS_PACKET_CMD_FORWARD_E
#define XPS_PACKET_CMD_DROP_HARD_E     CPSS_PACKET_CMD_DROP_HARD_E
#define XPS_PACKET_CMD_MIRROR_TO_CPU_E CPSS_PACKET_CMD_MIRROR_TO_CPU_E
#define XPS_PACKET_CMD_DROP_SOFT_E     CPSS_PACKET_CMD_DROP_SOFT_E

#define XPS_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E         CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E
#define XPS_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E
#define XPS_PCL_ACTION_REDIRECT_CMD_NONE_E                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E
#define XPS_PCL_ACTION_REDIRECT_CMD_OUT_IF_E              CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
#define XPS_PCL_ACTION_REDIRECT_CMD_ROUTER_E            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E

#define XPS_HARD_DROP_REASON_CODE ((CPSS_NET_RX_CPU_CODE_ENT)(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E +1))
#define XPS_SOFT_DROP_REASON_CODE ((CPSS_NET_RX_CPU_CODE_ENT)(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E +2))
#define XPS_TRAP_DROP_REASON_CODE ((CPSS_NET_RX_CPU_CODE_ENT)(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E +3))
#define XPS_LOG_DROP_REASON_CODE  ((CPSS_NET_RX_CPU_CODE_ENT)(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E +4))

#define XPS_FABRICO_ERROR_EGRESS_DROP_REASON_CODE ((CPSS_NET_RX_CPU_CODE_ENT)(CPSS_NET_FIRST_USER_DEFINED_E + 16))

#define XPS_PORT_LIST_OFFSET 24
#define XPS_PORT_LIST_PORT_GROUP_MAX 16
#define XPS_ACL_INVALID_TABLE_ID 0xFFFFFFFFu
#define XPS_ENTRY_MAX (1024*6)

#define XPS_PCL_PACKET_TYPE_IPV4_TCP       0
#define XPS_PCL_PACKET_TYPE_IPV4_UDP       1
#define XPS_PCL_PACKET_TYPE_IPV4_FRAG      3
#define XPS_PCL_PACKET_TYPE_IPV4_OTHER     4
#define XPS_PCL_PACKET_TYPE_ETHERNET_OTHER 5
#define XPS_PCL_PACKET_TYPE_UDE1           8
#define XPS_PCL_PACKET_TYPE_IPV6_TCP       12
#define XPS_PCL_PACKET_TYPE_IPV6_UDP       13
#define XPS_PCL_PACKET_TYPE_IPV6_OTHER     14

#define XPS_VXLAN_IPV6_PCL_UDB_PACKET_TYPE_CNS CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E
#define XPS_VXLAN_IPV6_PCL_CFG_TABLE_INDEX_CNS 0x0
#define XPS_VXLAN_IPV6_PCL_ID_CNS              0x10

#define XPS_PCL_DROP_PACKET_PCL_ID_CNS         0x11

#define DEFAULT_NULL_VALUE                   0
#define XPS_PCL_APP_FLOW_TEMP_MASK_ALL       0
#define XPS_PCL_APP_FLOW_TEMP_IPV4_TCP       1
#define XPS_PCL_APP_FLOW_TEMP_IPV4_UDP       2
#define XPS_PCL_APP_FLOW_TEMP_IPV4_OTHER     16
#define XPS_PCL_APP_FLOW_TEMP_ETHERNET_OTHER 32
#define XPS_PCL_APP_FLOW_TEMP_IPV6           64
#define XPS_PCL_APP_FLOW_TEMP_UDE1           128
#define XPS_PCL_APP_FLOW_TEMP_MASK_UDE1      0xFF
#define XPS_PCL_ARP_REQUEST_OP_CODE          1
#define XPS_PCL_ARP_REPLY_OP_CODE            2
#define XPS_PCL_NXT_HDR_IPV6_TCP             6
#define XPS_PCL_NXT_HDR_IPV6_UDP             17
#define XPS_PCL_IS_L4_VALID_IPCL_METADATA_VAL     2
#define XPS_PCL_IS_L4_VALID_EPCL_METADATA_VAL     8
#define XPS_PCL_APP_FLOW_ALL_IPV4_PACKETS_MASK    0xE4
#define XPS_PCL_APP_FLOW_ALL_NON_IP_PACKETS_MASK  0x5F
#define XPS_PCL_EGRESS_PKT_TYPE_NON_IP      0
#define XPS_PCL_EGRESS_PKT_TYPE_IPV4        6
#define XPS_PCL_EGRESS_PKT_TYPE_IPV6        10
#define XPS_PCL_EGRESS_PKT_TYPE_UDE1        10
#define XPS_PCL_EGRESS_PKT_TYPE_MASK        0xFF
/* Assuming max rules in a table are 1K in order to accomodate more tables in the system. */
#define XPS_PCL_TABLE_ID_KEY(tableId, ruleId)  ((tableId << 10) | ruleId)

typedef GT_BOOL                                        XPS_BOOL;
typedef CPSS_DXCH_PCL_ACTION_STC                       xpsPclAction_t;
typedef CPSS_DXCH_PCL_RULE_FORMAT_UNT                  xpsPclRuleFormat_t;
typedef CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT             xpsRuleFormat_t;
typedef CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC           xpsVirtualTcamRuleData_t;
typedef CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC           xpsVirtualTcamRuleType_t;
typedef CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC
xpsPclRuleFormatIngrUdbOnly_t;
typedef CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC
xpsPclRuleFormatEgrUdbOnly_t;

typedef enum xpsAclErrorEgressDropMatch
{
    XPS_FABRICO_ERROR_EGRESS_DROP_61_IPV4_MATCH_E = 0,
    XPS_FABRICO_ERROR_EGRESS_DROP_62_IPV4_MATCH_E = 1,
    XPS_FABRICO_ERROR_EGRESS_DROP_63_IPV4_MATCH_E = 2,
    XPS_FABRICO_ERROR_EGRESS_DROP_61_IPV6_MATCH_E = 3,
    XPS_FABRICO_ERROR_EGRESS_DROP_62_IPV6_MATCH_E = 4,
    XPS_FABRICO_ERROR_EGRESS_DROP_63_IPV6_MATCH_E = 5,
    XPS_FABRICO_ERROR_EGRESS_DROP_END_MATCH_E
} xpsAclErrorEgressDropMatch;

typedef enum xpsAclKeyFormat
{
    XPS_PCL_NON_IP_KEY = 0,
    XPS_PCL_IPV4_L4_KEY = 1,
    XPS_PCL_IPV6_L4_KEY = 2,
    XPS_PCL_ARP_KEY = 3,
    XPS_PCL_VXLAN_IPV6_KEY = 4,
    XPS_PCL_KEY_FORMAT_END
} xpsAclKeyFormat;

typedef enum xpsAclStage
{
    XPS_PCL_STAGE_INGRESS = 0,
    XPS_PCL_STAGE_EGRESS,
    XPS_PCL_STAGE_MAX
} xpsAclStage_e;

typedef enum xpsIaclKeyFlds
{
    XPS_PCL_NONE,
    XPS_PCL_PCLID,
    XPS_PCL_MAC_TO_ME,
    XP_PCL_SRC_PORT,
    XP_PCL_SOURCE_DEVICE,
    XPS_PCL_DEST_PORT,
    XPS_PCL_IS_TAGGED,
    XP_PCL_VID,
    XP_PCL_UP,
    XP_PCL_QOS_PROFILE,
    XPS_PCL_IS_IP,
    XPS_PCL_IS_L2_VALID,
    XPS_PCL_IS_UDB_VALID,
    XPS_PCL_IS_IPV4,
    XPS_PCL_IS_NON_IPV4,
    XPS_PCL_IS_IPV6,
    XPS_PCL_IS_NON_IPV6,
    XPS_PCL_IP_DSCP,
    XPS_PCL_IS_L4_VALID,
    XPS_PCL_IP_HEADER_OK,
    XPS_PCL_IPV4_FRAGMENT,
    XP_PCL_PORT_BIT_MAP,
    XPS_PCL_IP_PROTOCOL,
    XPS_PCL_TOS,
    XPS_PCL_DSCP,
    XPS_PCL_ETHER_TYPE,
    XPS_PCL_IS_ARP,
    XPS_PCL_IS_ARP_REQUEST,
    XPS_PCL_IS_ARP_REPLY,
    XPS_PCL_L2_ENCAP,
    XPS_PCL_MAC_DA,
    XPS_PCL_MAC_SA,
    XPS_PCL_VRF_ID,
    XPS_PCL_TRUNK_HASH,
    XPS_PCL_TAG1_EXIST,
    XPS_PCL_VID1,
    XPS_PCL_UP1,
    XPS_PCL_CFI1,
    XPS_PCL_IS_IPV6_HEADER_EXIST,
    XPS_PCL_IS_IPV6_HOP_BY_HOP,
    XPS_PCL_IS_DIP_BIT_0_TO_31,
    XPS_PCL_IS_BC,
    XPS_PCL_SIP,
    XPS_PCL_DIP,
    XPS_PCL_PKT_TYPE,
    XPS_PCL_IP_FRAGMENT,
    XPS_PCL_IP_HEADER_INFO,
    XPS_PCL_IP_PKT_LEN,
    XPS_PCL_TTL,
    XPS_PCL_TCP_UDP_PORT_COMPRATOR,
    XPS_PCL_IS_ND,
    XPS_PCL_IPV6_SIP,
    XPS_PCL_IPV6_DIP,
    XPS_PCL_IS_MPLS,
    XPS_PCL_NO_OF_MPLS_LABLE,
    XPS_PCL_PROTO_AFTER_MPLS_LABLE,
    XPS_PCL_MPLS_LABLE0,
    XPS_PCL_MPLS_EXP0,
    XPS_PCL_MPLS_LABLE1,
    XPS_PCL_MPLS_EXP1,
    XPS_PCL_DIP_BITS_127_T0_120,
    XPS_PCL_L4_BYTE0,
    XPS_PCL_L4_BYTE1,
    XPS_PCL_L4_BYTE2,
    XPS_PCL_L4_BYTE3,
    XPS_PCL_L4_BYTE_13,
    XPS_PCL_L2_VALID,
    XP_PCL_TC,
    XPS_PCL_ICMP_CODE,
    XPS_PCL_ICMP_MSG_TYPE,
    XPS_PCL_ICMPV6_CODE,
    XPS_PCL_ICMPV6_MSG_TYPE,
    XPS_PCL_ARP_OPCODE,
    XPS_PCL_IS_NON_IP,
    XPS_PCL_IS_IPV4_TCP,
    XPS_PCL_IS_IPV4_UDP,
    XPS_PCL_IS_IPV4_OTHER,
    XPS_PCL_IS_IPV6_TCP,
    XPS_PCL_IS_IPV6_UDP,
    XPS_PCL_IS_IPV6_OTHER,
    XPS_PCL_IS_ANY,
    XPS_PCL_VALID,
    XPS_PCL_FC_OPCODE,
    XPS_PCL_L4_BYTE13,
    XP_PCL_TCP_FLAGS = XPS_PCL_L4_BYTE13,
    XPS_PCL_DSA_CMD,
    XPS_PCL_VNI,
    XPS_PCL_CPU_CODE,
    XPS_PCL_EPORT,
    XPS_PCL_MAX_NUM_FLDS,
} xpsAclKeyFlds_t;

typedef struct xpsAclkeyField
{
    xpsAclKeyFlds_t keyFlds;
    uint8_t*  value;   //this is a pointer to an byte array
    uint8_t*  mask;    //this is a pointer to an byte mask array
} xpsAclkeyField_t;

typedef struct xpsAclkeyFieldList
{
    uint32_t numFlds;
    uint32_t isValid;
    xpsAclkeyField_t* fldList;
} xpsAclkeyFieldList_t;

typedef struct xpAclKeyMask_t
{
    uint8_t* value;
    uint8_t* mask;
} xpsAclKeyMask_t;

typedef struct udbConfig
{
    GT_U8                                devNum;
    GT_PORT_GROUPS_BMP                   portGroupsBmp;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType;
    CPSS_PCL_DIRECTION_ENT               direction;
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
} udbConfig_t;

typedef struct xpsDefineKeyConfig
{
    uint32_t                             numberOfUDB;
    udbConfig_t                          udbConfigList[MAX_UDB_SUPPORT];
    uint32_t                             pclId0;
    uint32_t                             pclId01;
    uint32_t                             pclId1;
    uint32_t                             epclId0;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookup0NonIpKey;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookup0Ipv4Key;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookup0Ipv6Key;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookup1NonIpKey;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookup1Ipv4Key;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookup1Ipv6Key;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookupEgrNonIpKey;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookupEgrIpv4Key;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   lookupEgrIpv6Key;
} xpsDefineKeyConfig_t;

typedef enum
{
    /** @brief TCAM rule size to be used 10 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    XPS_VIRTUAL_TCAM_RULE_SIZE_10_B_E = 0,

    /** @brief TCAM rule size to be used 20 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    XPS_VIRTUAL_TCAM_RULE_SIZE_20_B_E = 1,

    /** TCAM rule size to be used 30 Bytes */
    XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E = 2,

    /** @brief TCAM rule size to be used 40 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    XPS_VIRTUAL_TCAM_RULE_SIZE_40_B_E = 3,

    /** @brief TCAM rule size to be used 50 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    XPS_VIRTUAL_TCAM_RULE_SIZE_50_B_E = 4,

    /** TCAM rule size to be used 60 Bytes */
    XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E = 5,

    /** TCAM rule size to be used 80 Bytes */
    XPS_VIRTUAL_TCAM_RULE_SIZE_80_B_E = 7,

    /** Standard rule size for xCat3 devices. */
    XPS_VIRTUAL_TCAM_RULE_SIZE_STD_E = XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E,

    /** Extended rule size for xCat3 devices. */
    XPS_VIRTUAL_TCAM_RULE_SIZE_EXT_E = XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E,

    /** Ultra rule size for xCat3 devices. */
    XPS_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E = XPS_VIRTUAL_TCAM_RULE_SIZE_80_B_E

}  xpsAclVTcamRuleSize_e;

typedef enum
{
    /** @brief Rule placed by given logical index.
     *  If logical index of rule1 more than logical index of rule2
     *  than rule1 will be checked to match after rule2
     */
    XPS_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E,

    /** @brief Rule created by unique rule Id and attributes(priority).
     *  If priority of rule1 less than priority of rule2
     *  than rule1 will be checked to match before rule2
     *  If two rules created with the same priority
     *  the rule created later be checked to match later.
     */
    XPS_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E

} xpsAclTcamRuleAdditionMethod_e;

typedef enum xpsAclTableType
{
    XPS_ACL_TABLE_TYPE_ARP=0,
    XPS_ACL_TABLE_TYPE_NON_IP,
    XPS_ACL_TABLE_TYPE_IPV4,
    XPS_ACL_TABLE_TYPE_IPV6,
    XPS_ACL_TABLE_TYPE_IPV4_IPV6,
    XPS_ACL_TABLE_TYPE_MIRROR
} xpsAclTableType_t;

typedef struct xpsVTcamInfo
{
    /** TCAM Client Group. */
    uint32_t clientGroup;

    /** @brief TCAM paralell sublookup number;
     *  (APPLICABLE RANGES: 0..3);
     */
    uint32_t hitNumber;

    /** size of rules used in virtual TCAM. */
    xpsAclVTcamRuleSize_e ruleSize;

    /** @brief GT_TRUE
     *  GT_FALSE - resized only when calling "Resize" API.
     *  Relevant only to priority driven vTCAMs,
     *  For locical-index driven vTCAMs GT_TRUE causes return GT_BAD_PARAM,.
     *  Automatic resize option never reduces vTCAM size less
     *  than guaranteedNumOfRules rules.
     */
    bool autoResize;

    /** guaranteed amount of rules. */
    uint32_t guaranteedNumOfRules;

    /** @brief rules addition method.
     *  Comments:
     */
    xpsAclTcamRuleAdditionMethod_e ruleAdditionMethod;

    xpsRuleFormat_t          ruleFormat;

} xpsVTcamInfo_t;

/**
 * \struct xpAclTableInfo_t
 * \brief This structure will be passed in createTable
 * to create tables based of the keySize and num db's
 * user wants to create
 */
typedef struct xpAclTableInfo
{
    xpsAclStage_e       stage;
    xpsVTcamInfo_t      vTcamInfo;
    xpsAclTableType_t   tableType;
    bool                isMirror;
} xpAclTableInfo_t;

typedef enum
{
    XPS_L4_PROTOCOL_PORT_SRC,

    XPS_L4_PROTOCOL_PORT_DST

} XPS_L4_PROTOCOL_PORT_TYPE;

typedef enum
{
    XPS_ACL_DIRECTION_INGRESS,
    XPS_ACL_DIRECTION_EGRESS
} xpsAclDirection;

typedef enum
{
    XPS_ACL_PARALLEL_LOOKUP_0,
    XPS_ACL_PARALLEL_LOOKUP_1,
    XPS_ACL_PARALLEL_LOOKUP_2,
    XPS_ACL_PARALLEL_LOOKUP_3,
    XPS_ACL_PARALLEL_LOOKUP_INVALID,
} xpsAclParallelLookUp;

typedef struct xpsAclEntryContextDbEntry
{
    uint32_t             key; //consists of table id and rule id
    xpsAclkeyFieldList_t rule;
    xpsPclAction_t       action;
    uint32_t             priority;
    xpsAclKeyFormat      keyFormat;
    bool                 counterEnable;
    uint32_t             counterId;
    bool                 isDirty;// "true" indiactes sw and hw is out of sync
    uint32_t
    portGroupBmp[XPS_PORT_LIST_PORT_GROUP_MAX]; //indexed with port group
    uint32_t
    portListBmp[XPS_PORT_LIST_PORT_GROUP_MAX]; //indexed with port group
} xpsAclEntryContextDbEntry_t;

typedef struct xpsAclTableContextDbEntry
{
    uint32_t            tableId;
    uint32_t            tcamMgrId;
    uint32_t            vtcamId[XPS_ACL_MAX_TBLS_IN_GROUP];
    uint32_t            currentVtcamId;
    uint32_t
    portGroupBmp[XPS_PORT_LIST_PORT_GROUP_MAX]; //indexed with port group
    uint32_t
    portListBmp[XPS_PORT_LIST_PORT_GROUP_MAX]; //indexed with port group
    uint32_t            bindCount;
    xpsAclTableType_t   tableType;
    xpsAclStage_e       stage;
    uint32_t            numEntries;
    uint32_t            ruleId[XPS_ENTRY_MAX];
    uint8_t             numOfDuplicates;
    bool                isVlan;
    uint32_t            vlanId;
    bool                isMirror;

} xpsAclTableContextDbEntry_t;

typedef struct xpsAclTableVtcamMapContextDbEntry
{
    uint32_t        groupId; //key
    uint32_t        tableCount;
    xpsAclTableType_t tableType;
    /*
      There can be more than 4 table per grp, in which case,
      the look-up is sequential.
     */
    uint32_t        *tableId;
    uint32_t         tableMax;
    uint32_t        vTcamRegionIngressL3[XPS_ACL_MAX_TBLS_IN_GROUP];
    uint32_t        vTcamRegionIngressL3V6[XPS_ACL_MAX_TBLS_IN_GROUP];
    uint32_t        vTcamRegionEgressL3[XPS_ACL_MAX_TBLS_IN_GROUP];
    uint32_t        vTcamRegionEgressL3V6[XPS_ACL_MAX_TBLS_IN_GROUP];

} xpsAclTableVtcamMapContextDbEntry_t;

typedef struct xpsAclCounterIdMappingContextDbEntry
{
    uint32_t globalCounterId;
    uint32_t localCounterId[XPS_ACL_MAX_TBLS_IN_GROUP];
    uint32_t parallelLookupNum[XPS_ACL_MAX_TBLS_IN_GROUP];
    uint32_t numOfDuplicates;
    uint32_t numOfEntries; //Number of entries sharing this global counter Id
} xpsAclCounterIdMappingContextDbEntry_t;

/**
 * \brief This method initializes xpAclMgr with system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *
 * \param [in] scopeId : Scope ID
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclInitScope(xpsScope_t scopeId);

/**
 * \brief This method de-initializes xpAclMgr by clearing all system specific
 *        references to various primitive managers for a scope.
 *
 * \param [in] scopeId : Scope Id
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclDeInitScope(xpsScope_t scopeId);

/**
 * \brief Enable/Disable acl lookup on the system,
 *
 * \param [in] devId       : device Id.
 * \param [in] ingressEn   : true in case when Enable/Disable need to apply on ingress ACL else false for Egress Acl.
 * \param [in] aclEnLookup : This to enable/disable lookup.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsAclEnableLookup(xpsDevice_t devId, bool ingressEn,
                             bool aclEnLookup);

/**
 * \brief This will allow user to create tables. KeySize , KeyType ,number of entries,
 *        dynamic grow is allowed or not etc. can be pass as input at time of table
 *        creation.
 *
 * \param [in]  devId  : device Id
 *
 * \param [in]  tableInfo :  Its structure with info about number of tcam entries ,keysize,
 *              guaranteed Number Of Rules, dynamic table grow and other ACL table related info.
 *
 * \param [out] tableIdNum : Table Id that will be used at time of Acl entry addition.
 *
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclCreateTable(xpsDevice_t devId, xpAclTableInfo_t tableInfo,
                            uint32_t *tableIdNum);

/**
 * \brief This will allow the user to delete tables created previously based on the table
 *        info . It will free the memory allocated for the given table.
 *
 *
 * \param [in] devId    : device Id
 * \param [in] tableId  : table ID
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclDeleteTable(xpsDevice_t devId, uint32_t tableId);

/**
 * \brief Method to set ACL rule valid bit
 *
 *
 * \param [in] devId : Device Id
 * \param [in] tableId : table Id
 * \param [in] index   : Its index at which the ACL entry was written on the HW(can be logical/priority).
 * \param [in] valid Valid bit parameter
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclSetRuleValid(xpsDevice_t devId, uint32_t tableId,
                             uint32_t index, uint8_t valid);

/**
 * \brief Api to enable/disable the Acl policer globally.
 *
 * Acl policing is enabled by deafault globally.
 *
 * \param [in] devId
 * \param [in] enable Enable status
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclSetPolicingEnable(xpsDevice_t devId, uint32_t enable);

/**
 * \brief Copy ACL entry in HW table of device.
 *
 * \param [in] devId : Device Id
 * \param [in] tableId : Table Id
 * \param [in] indexSrc index at which the ACL source entry
 *        that need to be copied is written on the HW.
 * \param [in] indexDst index at which the ACL target entry is
 *        written on the HW.
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclCopyEntry(xpDevice_t devId, uint32_t tableId,
                          uint32_t indexSrc, uint32_t indexDst);

/**
 * \public
 * brief This method prints valid table entries.
 *
 * \param [in] devId
 * \param [in] tableId, It can be of port,bridge or router aclType(XP_ACL0, XP_ACL1 and XP_ACL2).
 * \param [in] keyType It can be ipv4 or ipv6 key(XP_ACL_V4_TYPE, XP_ACL_V6_TYPE)
 * \param [in] displayEntryFlag It can be invalid, valid or both(XP_TCAM_DISP_ENTRY_INVALID, XP_TCAM_DISP_ENTRY_VALID, XP_TCAM_DISP_ENTRY_ALL)
 * \param [out] *numOfValidEntries Number of valid entry available from startIndex to endIndex in Hardware.
 * \param [in] startIndex Starting index of table
 * \param [in] endIndex Ending index of table
 * \param [in] logFile Pointer of log file to print log in perticular file  or it can be to NULL to display on console
 * \param [in] detailFormat It can be 1 or 0 (1-To display all valid entry with bitvectori prints)
 * \param [in] silentMode It can be 1 or 0 (1- To display only valid entries count, 0- To print valid entries in table format)
 * \param [in] tblCopyIdx Table copy number
 *
 * \return XP_STATUS
 */
#if 0
XP_STATUS xpAclDisplayTable(xpDevice_t devId, xpAclType_e iaclType,
                            xpsAclKeyType_e keyType, xpTcamDisplayEntryFlag_e displayEntryFlag,
                            uint32_t *numOfValidEntries, uint32_t startIndex, uint32_t endIndex,
                            char *logFile, uint32_t detailFormat, uint32_t silentMode, uint8_t tblCopyIdx);
#endif

/**
 * \brief This method initializes xpAclMgr with system specific
 *        references to various primitive managers and their initialization.
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclInit(void);

/**
 * \brief This method de-initializes xpAclMgr by clearing all system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclDeInit(void);

/**
 * \brief Add device for ACL module.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief this method is used to write ACL Entry in given Table.
 *
 * \param [in] devId      : Device Id
 * \param [in] tableId    : table Id
 * \param [in] indexOrPri : will be treated as logical index or priority(as configure in given table creation)
 * \param [in] fieldList  : all key realted info.
 * \param [in] action_ptr : all action realted info.
 * \param [in] keyFormat  : keyFormat info
 *
 * \return XP_STATUS
*/

XP_STATUS xpsAclWriteEntry(xpsDevice_t devId, uint32_t tableId,
                           uint32_t priority, uint32_t ruleId, xpsAclkeyFieldList_t *fieldList,
                           xpsPclAction_t actionPtr, xpsAclKeyFormat keyFormat, bool xpsAclWriteEntryKey);

/**
 * \brief this method is used to delete ACL Entry in given Table.
 *
 * \param [in] devId      : Device Id
 * \param [in] tableId    : table Id
 * \param [in] indexOrPri : will be treated as logical index or priority(as configure in given table creation)
 *
 * \return XP_STATUS
*/

XP_STATUS xpsAclDeleteEntry(xpsDevice_t devId, uint32_t tableId,
                            uint32_t indexOrPri, bool is_switch_bind_enable);

/**
 * \brief this method is used to read  ACL Entry in given Table.
 *
 * \param [in] devId      : Device Id
 * \param [in] tableId    : table Id
 * \param [in] indexOrPri : will be treated as logical index or priority(as configure in given table creation)
 * \param [in] ruleData   : rule Data
 * \param [in] keyFormat  : keyFormat
 *
 * \return XP_STATUS
*/

XP_STATUS xpsAclReadEntry(xpsDevice_t devId, uint32_t tableId,
                          uint32_t indexOrPri, xpsVirtualTcamRuleData_t *ruleData,
                          xpsAclKeyFormat keyFormat);

/**
 * \brief this method is used to read ACL action in given Table.
 *
 * \param [in] devId      : Device Id
 * \param [in] tableId    : table Id
 * \param [in] indexOrPri : will be treated as logical index or priority(as configure in given table creation)
 * \param [in] action_ptr   : action pointer data
 * \param [in] keyFormat  : keyFormat
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclReadData(xpsDevice_t devId, uint32_t tableId,
                         uint32_t indexOrPri, xpsPclAction_t *actionPtr, xpsAclKeyFormat keyFormat);

/**
 * \brief this method is used to update ACL Entry in given Table.
 *
 * \param [in] devId      : Device Id
 * \param [in] tableId    : table Id
 * \param [in] indexOrPri :  will be treated as logical index or priority(as configure in given table creation)
 * \param [in] fieldList  :  key info
 * \param [in] action_ptr   : action pointer data
 * \param [in] keyFormat    : keyFormat
 *
 * \return XP_STATUS
*/
XP_STATUS xpsUpdateAclEntry(xpsDevice_t devId, uint32_t tableId,
                            uint32_t indexOrPri, uint32_t ruleId, xpsAclkeyFieldList_t* fieldList,
                            xpsPclAction_t actionPtr, xpsAclKeyFormat keyFormat, bool xpsAclWriteEntryKey);

/**
 * \brief this method is used to get the available entries in the PCL table
 *
 * \param [in]  devId            : Device Id
 * \param [in]  tableIdNum       : Table Id
 * \param [out] availableEntries : number of available acl entries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclGetAvailableEntries(xpsDevice_t devId,
                                    uint32_t tableIdNum, uint32_t *availableEntries);

/**
 * \brief this method is used to get the Counter value associated with the PCL table
 *
 * \param [in] devId      : Device Id
 * \param [in] counterIdx : CounterIndex
 * \param [out] byteCount  : ByteCount
 * \param [out] packCount  : packetCount
 *
 * \return XP_STATUS
 */

XP_STATUS xpsAclGetCounterValue(xpDevice_t devId, uint32_t counterIdx,
                                uint64_t *byteCount, uint64_t *packCount);


XP_STATUS xpsAclGetCounterValueExt(xpDevice_t devId, uint32_t counterId,
                                   uint64_t *pktCnt, uint64_t *byteCnt);


/**
 * \brief this method is used to invalidate all tcp-udp comparator entries.
 *
 * \param [in] devId      : Device Id
 *
 * \return XP_STATUS
*/
XP_STATUS xpsAclDeleteL4PortRange(xpsDevice_t devId);

/**
 * \brief Binds ACL rules to port at ingress.
 *
 * \param [in] devId Device Id
 * \param [in] portNum Port Number
 * \param [in] pclId PCL Id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclSetPortAcl(xpsDevice_t devId, uint32_t portNum,
                           uint32_t pclId, uint32_t groupId);

/**
 * \brief Binds ACL rules to port at ingress.
 *
 * \param [in] devId Device Id
 * \param [in] portNum Port Number
 * \param [in] pclId PCL Id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclSetLagAcl(xpsDevice_t devId, uint32_t lagId,
                          uint32_t pclId, uint32_t groupId);

/**
 * \brief Binds ACL rules to port at ingress.
 *
 * \param [in] devId Device Id
 * \param [in] portNum Port Number
 * \param [in] pclId PCL Id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclSetVlanAcl(xpsDevice_t devId, uint32_t vlan,
                           uint32_t pclId, uint32_t groupId);
/**
 * \brief Generic Init API for initialising the function pointers
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsAclPortUnbind(xpsDevice_t devId, uint32_t portNum,
                           uint32_t groupId, uint32_t tableId);


XP_STATUS xpsAclVlanUnbind(xpsDevice_t devId, uint32_t vlanId,
                           uint32_t groupId, uint32_t tableId);

XP_STATUS xpsAclPortUnbind(xpsDevice_t devId, uint32_t portNum,
                           uint32_t groupId, uint32_t tableId);

XP_STATUS xpsAclLagUnbind(xpsDevice_t devId, uint32_t lagId,
                          uint32_t groupId, uint32_t tableId);

XP_STATUS xpsAclUpdatePortIngressAcl(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t tableId, uint32_t groupId, bool enable);

XP_STATUS xpsGetAclTableType(uint32_t tableIdNum,
                             xpsAclTableType_t *tableType);

XP_STATUS xpsGetAclKeyFormat(xpsAclKeyFlds_t entryType,
                             xpsAclTableType_t tableType, xpsAclKeyFormat *keyFormat);

XP_STATUS xpsAclTableInit(xpsDevice_t devId);

XP_STATUS xpsAclUdbKeysInitScope(xpsScope_t scopeId);

XP_STATUS xpsAclTableDeInitScope(xpsScope_t scopeId);

XP_STATUS xpsAclPortListPortMappingSet(xpsDevice_t devId);

XP_STATUS xpsAclPFCRuleUpdate(xpsDevice_t devId, uint32_t portNum, bool enable);

XP_STATUS xpsAclCommonDropRuleUpdate(xpsDevice_t devId, uint32_t portNum,
                                     bool enable);
XP_STATUS xpsAclRouterCommonDropUpdate(xpsDevice_t devId,
                                       xpsInterfaceId_t l3IntfId, bool enable);
XP_STATUS xpsAclUpdateMirrorTableVtcamMap(xpsDevice_t devId, uint32_t tableId);
XP_STATUS xpsAclUpdateTableVtcamMapping(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t groupId);

XP_STATUS xpsAclCounterAllocatorsInit(xpsDevice_t devId);

XP_STATUS xpsAclCounterConfigure(xpsDevice_t devId);

XP_STATUS xpsAclSetRuleInbandMgmt
(
    uint32_t devId, uint32_t port
);

XP_STATUS xpsAclErspanRuleUpdate(xpsDevice_t devId, uint32_t portNum,
                                 bool enable, uint32_t analyzerIndex);
XP_STATUS xpsAclAddCnpRateLimitRule(xpsDevice_t devId, uint32_t portNum);
XP_STATUS xpsAclAddEgressDropPacketsRule(xpsDevice_t devId);
XP_STATUS xpsAclAddIngressCommonDropRules(xpsDevice_t devId);

void xpsAclDropEgressPacketsParamSet(xpsDevice_t devId, uint32_t portNum,
                                     bool enabled);

/**
 * \brief Update portListBmp in ACL rules.
 *
 * \param [in] devId Device Id
 * \param [in] tableId Table Id
 * \param [in] ruleId PCL Id
 * \param [in] ports The array of in_ports/out_ports
 * \param [in] ports_count The number of in_ports/out_ports
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortsAclEntryKeySet(xpsDevice_t devId, uint32_t tableId,
                                 uint32_t ruleId,
                                 uint32_t *ports, uint32_t ports_count);


XP_STATUS xpsUdbKeySet(xpsAclkeyFieldList_t *fieldList,
                       xpsPclRuleFormat_t *pattern, xpsPclRuleFormat_t *mask,
                       xpsAclKeyFormat keyFormat, xpsAclStage_e stage);

XP_STATUS xpsAclCreateNewCounter(xpsDevice_t devId, xpsScope_t scopeId,
                                 CPSS_PP_FAMILY_TYPE_ENT devType, xpsAclTableContextDbEntry_t *tableIdDbEntry,
                                 uint32_t parallelLookupNum, uint32_t *localCounterId);

XP_STATUS xpsAclRemoveCounter(xpsDevice_t devId, xpsScope_t scopeId,
                              CPSS_PP_FAMILY_TYPE_ENT devType, xpsAclTableContextDbEntry_t *tableIdDbEntry,
                              uint32_t localCounterId);


//XP_STATUS xpsAclUpdateErspanAction(xpsDevice_t devId, xpsPclAction_t *aclEntryData, uint32_t analyzerIndex);

/**
 * \brief this method is used to get portlist of given Table.
 *
 * \param [in] devId      : Device Id
 * \param [in] tableId    : table Id
 *
 * \return portmap
*/

XP_STATUS xpsAclGetPortMapFromTableId(xpsDevice_t devId, uint32_t tableId,
                                      uint32_t **);

#ifdef __cplusplus
}
#endif

#endif //_XPSACL_H_
