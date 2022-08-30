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
* @file cpssDxChPcl.h
*
* @brief CPSS DxCh PCL lib API.
*
* @version   91
********************************************************************************
*/
#ifndef __cpssDxChPclh
#define __cpssDxChPclh

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>

/**
* General NOTEs about portGroupsBmp In TCAM related APIs:
*   1. In Falcon :
*     the TCAM is shared between 2 port groups.
*     the caller need to use 'representative' port groups , for example:
*     value 0x00000014 hold bits 2,4 represent TCAMs of port groups 2,3 and 4,5
*     value 0x00000041 hold bits 0,6 represent TCAMs of port groups 0,1 and 6,7
*     Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.(represent all port groups)
*
**/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Size of Bobcat2; Caelum; Bobcat3 action is 8 words                    */
/* Size of xCat3, Lion2 action is 4 words */
#define CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS 8

/* Rule entry size in words - defined by Bobcat2; Caelum; Bobcat3 size 7 x 3 words */
#define CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS  21

/**
* @enum CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT
 *
 * @brief This enum describes the type of second sublookup of Ingress PCL lookup0.
*/
typedef enum{

    /** @brief Partial lookup -
     *  in IPCL01 the Configuration table is accesses with the
     *  same index as the IPCL0-0 index.
     */
    CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_PARTIAL_E,

    /** @brief Full lookup -
     *  IPCL01 lookup is a full lookup that includes a Policy
     *  Configuration table lookup and a new key creation.
     *  IPCL0-1 accesses the IPCL0 Configuration table, and uses the
     *  <Enable Lookup0-1> and <PCL-ID for Lookup0-1> fields.
     *  Notice that IPCL0-1 and IPCL0-0 may access different entries
     *  in the configuration table, depending on the configuration
     *  table lookup mechanism.
     */
    CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_FULL_E

} CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_RULE_OPTION_ENT
 *
 * @brief Enumerator for the TCAM rule write option flags.
*/
typedef enum{

    /** @brief default options: rule is valid
     */
    CPSS_DXCH_PCL_RULE_OPTION_WRITE_DEFAULT_E      = 0,

    /** @brief write all fields
     *  of rule to TCAM but rule state is invalid
     *  (no match during lookups).
     *  The cpssDxChPclRuleValidStatusSet can turn the rule
     *  to valid state.
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E      = (0x1 << 0)

} CPSS_DXCH_PCL_RULE_OPTION_ENT;

/* Action definitions for Policy Rules */

/**
* @struct CPSS_DXCH_PCL_ACTION_MIRROR_STC
 *
 * @brief This structure defines the mirroring related Actions.
*/
typedef struct{

    /** @brief The CPU code assigned to packets
     *  Mirrored to CPU or Trapped to CPU due
     *  to a match in the Policy rule entry
     *
     *  Relevant for drop code: the CPU code also acts as 'drop code' for soft/hard drop commands.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *
     *  Relevant for egress actions for next devices:
     *  APPLICABLE DEVICES : Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;

    /** @brief Enables mirroring the packet to
     *  the ingress analyzer interface.
     *  GT_FALSE - Packet is not mirrored to ingress analyzer interface.
     *  GT_TRUE - Packet is mirrored to ingress analyzer interface.
     *
     */
    GT_BOOL mirrorToRxAnalyzerPort;

    /** @brief index of analyzer interface
     *  (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Relevant only when (mirrorToRxAnalyzerPort == GT_TRUE)
     *
     */
    GT_U32 ingressMirrorToAnalyzerIndex;

    /** @brief TCP RST and FIN packets
     *  (i.e. TCP disconnect cases) can be mirrored to CPU
     *  GT_TRUE - TCP RST and FIN packets mirrored to CPU.
     *  GT_FALSE - TCP RST and FIN packets not mirrored to CPU.
     *  Relevant for xCat3 and above devices only.
     *  Comments:
     */
    GT_BOOL mirrorTcpRstAndFinPacketsToCpu;

    /** @brief Enables mirroring the packet to
     *  the egress analyzer interface.
     *  GT_FALSE - Packet is not mirrored to egress analyzer interface.
     *  GT_TRUE - Packet is mirrored to egress analyzer interface.
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_BOOL mirrorToTxAnalyzerPortEn;

    /** @brief index of analyzer interface
     *  (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Relevant only when (mirrorToTxAnalyzerPort == GT_TRUE)
     */
    GT_U32 egressMirrorToAnalyzerIndex;

    /** @brief egress mode
     *  Relevant only when (mirrorToTxAnalyzerPort == GT_TRUE)
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirrorToAnalyzerMode;

} CPSS_DXCH_PCL_ACTION_MIRROR_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
 *
 * @brief This structure defines the using of rule match counter.
*/
typedef struct{

    /** @brief Enables the binding of this
     *  policy action entry to the Policy Rule Match Counter<n>
     *  (0<=n<32) indexed by matchCounterIndex
     *  GT_FALSE = Match counter binding is disabled.
     *  GT_TRUE = Match counter binding is enabled.
     */
    GT_BOOL enableMatchCount;

    /** @brief A index one of the 32 Policy Rule Match Counter<n>
     *  (0<=n<32) The counter is incremented for every packet
     *  satisfying both of the following conditions:
     *  - Matching this rule.
     *  - The previous packet command is not hard drop.
     *  NOTE: for DxCh3 and above (APPLICABLE RANGES: 0..(16K-1))
     *  Comments:
     *  DxCh3 and above: the CNC mechanism is used for the match counters.
     */
    GT_U32 matchCounterIndex;

} CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC;


/**
* @enum CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_ENT
 *
 *  @brief Enumerator for Egress CNC Index mode.
 *
 *   (APPLICABLE DEVICES: Ironman)
*/
typedef enum{

    /** @brief Hit counter mode
     *  CNC Index is Action<CNC Index> (legacy mode).
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_HIT_COUNTER_E,

    /** @brief Max Service Data Unit (SDU) Pass/Fail mode
     *  CNC Index is {Action<CNC Index>, MaxSDUSizePass/Fail (1b)}.
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_MAX_SDU_PASS_FAIL_E

} CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT
 *
 * @brief Enumerator for ingress Tag1 UP assignment command.
*/
typedef enum{

    /** @brief Do not modify the <UP1> assigned to the packet
     *  by previous engines.
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E,

    /** @brief If packet does not contain Tag1
     *  assign according to action entry's <UP1>,
     *  else retain previous engine <UP1> assignment
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E,

    /** @brief If packet contains Tag0 use UP0,
     *  else use action entry's <UP1> field.
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E,

    /** Assign action entry's <UP1> field to all packets. */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E

} CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT
 *
 * @brief Enumerator for egress Tag0 VID and UP assignment command.
*/
typedef enum{

    /** don't assign Tag0 value from entry. */
    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E,

    /** @brief assign Tag0 value from entry to outer Tag.
     *  It is Tag0 for not TS packets
     *  or Tunnel header Tag for TS packets.
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E,

    /** @brief assign Tag0 value from entry to Tag0
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E

} CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT
 *
 * @brief Enumerator for egress DSCP assignment command.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** Keep previous packet DSCP/EXP settings. */
    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E = GT_FALSE,

    /** @brief Modify Outer
     *  If packet is MPLS, modify EXP of
     *  packet's outer label to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the tunnel header
     *  Otherwise, If packet is IP, modify the packet's DSCP to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the tunnel header.
     *  (Backward Compatible mode).
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E = GT_TRUE,

    /** @brief Modify Inner DSCP;
     *  If packet is IP, modify the packet's DSCP to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the passenger header.
     *  This mode is not applicable for MPLS packets;
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E

} CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_QOS_INGRESS_STC
*
*  @brief This structure defines the ingress packet's QoS
*         attributes mark Actions.
*/
typedef struct{
        /** @brief The Modify DSCP (or EXP for MPLS) QoS attribute of the packet.
         * Enables modification of the packet's DSCP field.
         * Only relevant if QoS precedence of the previous
         * QoS assignment mechanisms (Port, Protocol Based
         * QoS, and previous matching rule) is Soft.
         * Relevant for IPv4/IPv6 packets, only.
         * ModifyDSCP enables the following:
         *   - Keep previous DSCP modification command.
         *   - Enable modification of the DSCP field in
         *     the packet.
         *   - Disable modification of the DSCP field in
         *     the packet.
        */
        CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT         modifyDscp;

        /** @brief The Modify UP QoS attribute of the packet.
         * Enables modification of the packet's
         * 802.1p User Priority field.
         * Only relevant if QoS precedence of the previous
         * QoS assignment mechanisms  (Port, Protocol Based
         * QoS, and previous matching rule) is Soft.
         * ModifyUP enables the following:
         * - Keep previous QoS attribute <ModifyUP> setting.
         * - Set the QoS attribute <modifyUP> to 1.
         * - Set the QoS attribute <modifyUP> to 0.
         */
        CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT         modifyUp;

        /** @brief The QoS Profile Attribute of the packet.
         * Only relevant if the QoS precedence of the
         * previous QoS Assignment Mechanisms (Port,
         * Protocol Based QoS, and previous matching rule)
         * is Soft and profileAssignIndex is set
         * to GT_TRUE.
         * The QoSProfile is used to index the QoSProfile
         * Table Entry and assign the QoS Parameters
         * which are TC, DP, UP, EXP and DSCP
         * to the packet.
         * 0..127 for xCat3, Lion2
         * 0..255 for Bobcat2; Caelum; Bobcat3, Aldrin, AC3X.
         */
        GT_U32                                        profileIndex;

        /** @brief Enable marking of QoS Profile Attribute of the packet.
         *        GT_TRUE - Assign <profileIndex> to the packet.
         *        GT_FALSE - Preserve previous QoS Profile setting.
         */
        GT_BOOL                                       profileAssignIndex;

        /** @brief Marking of the QoSProfile Precedence.
         * Setting this bit locks the QoS parameters setting
         * from being modified by subsequent QoS
         * assignment engines in the ingress pipe.
         * QoSPrecedence enables the following:
         * - QoS precedence is soft and the packet's QoS
         *   parameters may be overridden by subsequent
         *   QoS assignment engines.
         * - QoS precedence is hard and the packet's QoS
         *   parameters setting is performed until
         *   this stage is locked. It cannot be overridden
         *   by subsequent QoS assignment engines.
         */
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT   profilePrecedence;

        /** @brief Command for setting the Tag1 UP value.
         *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
         */
        CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT      up1Cmd;

        /** @brief The UP1 value to assign to the packet according to up1Cmd.
         *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
         */
        GT_U32                                        up1;

} CPSS_DXCH_PCL_ACTION_QOS_INGRESS_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_QOS_EGRESS_STC
*
*  @brief This structure defines the egress packet's QoS
*         attributes mark Actions.
*/
typedef struct{
        /** @brief Enable modify DSCP (or EXP for MPLS)
         *  For xCat3; Lion2 as Boolean:
         *  CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_CMD_KEEP_E - don't modify DSCP.
         *  any other - modify DSCP.
         *  For Bobcat2; Caelum; Bobcat3; Aldrin see enum comments.
          */
        CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT  modifyDscp;

        /** @brief The DSCP field (or EXP for MPLS) set to the transmitted packets.
          */
        GT_U32                                        dscp;

        /** @brief This field determines the Tag0 UP assigned
         *  to the packet matching the EPCL entry.
         */
        CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT      modifyUp;

        /** @brief The 802.1p UP field set to the transmitted packets.
          */
        GT_U32                                        up;

        /** @brief This field determines the Tag1 UP assigned
         *  to the packet matching the EPCL entry.
         *  GT_FALSE - Keep: Keep previous Tag1 UP
         *     assigned to the packet.
         *  GT_TRUE  - Modify: EPCL Action entry <UP1>
         *     is assigned to tag 1 UP of the packet.
         *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
         */
        GT_BOOL                                       up1ModifyEnable;

        /** @brief The UP1 value to assign to Tag1 of
         *   the packet according to <up1ModifyEnable>
         *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
         */
        GT_U32                                        up1;

} CPSS_DXCH_PCL_ACTION_QOS_EGRESS_STC;

/**
* @union CPSS_DXCH_PCL_ACTION_QOS_UNT
*
*  @brief This union defines the packet's QoS attributes mark Actions.
*/
typedef union
{
    /** Ingress QoS Action */
    CPSS_DXCH_PCL_ACTION_QOS_INGRESS_STC ingress;

    /** Egress QoS Action */
    CPSS_DXCH_PCL_ACTION_QOS_EGRESS_STC egress;

} CPSS_DXCH_PCL_ACTION_QOS_UNT;

/**
* @enum CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT
 *
 * @brief enumerator for PCL redirection target
*/
typedef enum{

    /** no redirection */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,

    /** redirection to output interface */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E,

    /** @brief Ingress control pipe not
     *  bypassed. If packet is triggered for routing,
     *  Redirect the packet to the Router Lookup Translation
     *  Table Entry specified in <routerLttIndex> bypassing
     *  DIP lookup.
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman;).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E,

    /** @brief redirect to virtual
     *  router with the specified Id (VRF ID)
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman;).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E,

    /** @brief replace MAC source address
     *  with specified value.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E,

    /** @brief assign logical source port
     *  (APPLICABLE DEVICES: xCat3; AC5)
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E,

    /** @brief assign VRF ID and redirect to next hop. The <routerLttIndex>
     *         parameter and vrfId parameter in
     *         CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_ROUTE_AND_VRF_ID_STC
     *         are index to the nextHop and vrfId respectively.
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E,

    /** @brief redirect to ECMP table. The <routerLttIndex> parameter in
    *          CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_UNT is index to the ECMP table.
    *          Router also performs LPM destination lookup.
    *          The <routerLttIndex> as ECMP index is applied only if LPM Leaf Node
    *          <applyPbr> is set in the LPM Leaf entry;
    *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
    */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E,

    /** @brief redirect to ECMP table like CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E
    *          and assign VRF Id.
    *          The <routerLttIndex> parameter and vrfId parameter in
    *          CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_ROUTE_AND_VRF_ID_STC are index to
    *          the ECMP table and vrdId respectively.
    *          Router also performs LPM destination lookup. The <routerLttIndex> as
    *          ECMP index is applied only if LPM Leaf Node <applyPbr> is set in the LPM
    *  Leaf entry; (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
    */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E,

    /** @brief Assign Generic action
     *  (APPLICABLE DEVICES: Ironman).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ASSIGN_GENERIC_ACTION_E
} CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT
 *
 * @brief enumerator for PCL redirection types of the passenger packet.
*/
typedef enum{

    /** @brief The passenger packet
     *  is Ethernet.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E,

    /** The passenger packet is IP. */
    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E

} CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT;


/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_OUT_IF_STC
 *
 * @brief This struct defines data for the redirection to interface.
*/
typedef struct
{
    /** @brief port/trunk/vidx interface for packet redirection.
     */
    CPSS_INTERFACE_INFO_STC  outInterface;

    /** @brief The packet MAC SA and MAC DA are switched on the transmitting.
     */
    GT_BOOL                  vntL2Echo;

    /** @brief Packet is redirected to a Tunnel Egress Interface.
     */
    GT_BOOL                  tunnelStart;

    /** @brief Pointer to Tunnel Start table entry.
     */
    GT_U32                   tunnelPtr;

    /** @brief Type of passenger packet being to be encapsulated.
     *  When packet is not TT and <MPLS Command> != NOP,
     *  the <Tunnel Type> bit must be set to OTHER.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT tunnelType;

    /** @brief Pointer to ARP table entry with MAC DA.
     */
    GT_U32                   arpPtr;

    /** @brief When enabled, the packet is marked for MAC DA modification,
     *  and the <ARP Index> specifies the new MAC DA.
     */
    GT_BOOL                  modifyMacDa;

    /** @brief When enabled, the packet is marked for MAC SA modification,
     *  similar to routed packets.
     */
    GT_BOOL                  modifyMacSa;
} CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_OUT_IF_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_MODIFY_MAC_SA_STC
 *
 * @brief This struct defines data for the MAC SA modification.
*/
typedef struct
{
    /** @brief Pointer to ARP table entry with MAC SA to replace
     *  in egress packet.
     */
    GT_U32                   arpPtr;

    /** @brief The MAC SA that is sent to the bridge engine.
     *  The field muxed with Policer fields - MAC SA modification
     *  cannot be done with policer metering in the same action.
     */
    GT_ETHERADDR             macSa;
} CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_MODIFY_MAC_SA_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_LOGICAL_SRC_IF_STC
 *
 * @brief This struct defines data for the assignment of logical source interface.
 *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
*/
typedef struct
{
    /** @brief Logical source interface.
     */
    CPSS_INTERFACE_INFO_STC  logicalInterface;

    /** @brief Enable Source mesh Id assignment.
     */
    GT_BOOL                  sourceMeshIdSetEnable;

    /** @brief Source mesh Id.
     */
    GT_U32                   sourceMeshId;

    /** @brief Enable User Tag AC.
     */
    GT_BOOL                  userTagAcEnable;

} CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_LOGICAL_SRC_IF_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_GENERIC_ACTION_STC
 *
 * @brief This struct defines data for redirection mode - GENERIC ACTION MODE
 *  (APPLICABLE DEVICES: Ironman)
*/
typedef struct
{
    /** @brief Generic Action.
     */
    GT_U32                  genericAction;
} CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_GENERIC_ACTION_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_ROUTE_AND_VRF_ID_STC
 *
 * @brief This struct defines data for redirection to LTT entry and assign VRF ID.
 *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*/
typedef struct
{
    /** @brief Data for the redirection to Router LTT entry.
     */
    GT_U32                  routerLttIndex;

    /** @brief Data for the VRF ID setting.
     */
    GT_U32                  vrfId;
} CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_ROUTE_AND_VRF_ID_STC;

/**
* @union CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_UNT
 *
 * @brief This union defines the redirection related Actions cases.
*/
typedef union
{
    /** @brief Data for the redirection to port/trunk/vidx interface.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_OUT_IF_STC            outIf;

    /** @brief Data for the redirection to Router LTT entry.
     */
    GT_U32                                                   routerLttIndex;

    /** @brief Data for the VRF ID setting.
     */
    GT_U32                                                   vrfId;

    /** @brief Data for the MAC SA modification.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_MODIFY_MAC_SA_STC     modifyMacSa;

    /** @brief Data for the assignment of logical source interface.
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_LOGICAL_SRC_IF_STC    logicalSourceInterface;

    /** @brief Data for the redirection to Router LTT entry
     *   and assign VRF ID. Relevant when pcl action redirect command is
     *   CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E
     *   (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_ROUTE_AND_VRF_ID_STC  routeAndVrfId;

    /** @brief Data for the redirection mode - Generic Action Mode
     *   (APPLICABLE DEVICES: Ironman)
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_GENERIC_ACTION_STC  genericActionInfo;
} CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_UNT;


/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_STC
 *
 * @brief This structure defines the redirection related Actions.
*/
typedef struct
{
    /** @brief Redirection command.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT  redirectCmd;

    /** @brief Data for redirection according to command.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_DATA_UNT data;
} CPSS_DXCH_PCL_ACTION_REDIRECT_STC;

/**
* @enum CPSS_DXCH_PCL_POLICER_ENABLE_ENT
 *
 * @brief enumerator for policer options.
*/
typedef enum{

    /** Meter and Counter are disabled. */
    CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E              = GT_FALSE,

    /** Both Meter and Counter enabled. */
    CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E = GT_TRUE,

    /** Meter only enabled. */
    CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E,

    /** Counter only enabled. */
    CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E

} CPSS_DXCH_PCL_POLICER_ENABLE_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_POLICER_STC
 *
 * @brief This structure defines the policer related Actions.
*/
typedef struct{

    /** @brief policer enable,
     *  xCat3 and above devices support all values of the enum
     */
    CPSS_DXCH_PCL_POLICER_ENABLE_ENT policerEnable;

    /** @brief policers table entry index
     *  Comments:
     */
    GT_U32 policerId;

} CPSS_DXCH_PCL_ACTION_POLICER_STC;

/**
* @enum CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT
 *
 * @brief Enumerator for ingress Tag1 VID assignment command.
*/
typedef enum{

    /** Do not modify Tag1 VID. */
    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E,

    /** @brief The entry's <Tag1 VID> is assigned as Tag1 VID for
     *  packets that do not have Tag1
     *  or Tag1-priority tagged packets.
     *  For packets received with Tag1 VID,
     *  retain previous engine assignment.
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E,

    /** Assign action entry's <VID1> field to all packets. */
    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E

} CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT;


/**
* @enum CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT
*
* @brief Enumerator for ingress CFI value update.
*/
typedef enum{

    /** @brief Do not update the CFI bit **/
    CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_CFI_DO_NOT_MODIFY_E,

    /** @brief update the CFI bit to 1 **/
    CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_CFI_SET_1_E,

    /** @brief update the CFI bit to 0 **/
    CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_CFI_SET_0_E
} CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_STC
*
*  @brief This structure defines the ingress VLAN modification
*         related Actions.
*/
typedef struct{

        CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT           modifyVlan;
        GT_BOOL                                        nestedVlan;
        GT_U32                                         vlanId;
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT    precedence;
        CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT  vlanId1Cmd;
        GT_U32                                         vlanId1;

        /** @brief Tag1 DEI/CFI value update command for the packet:
         *  (APPLICABLE DEVICES: AC5X; Harrier; Ironman; AC5P)
         */
        CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT cfi1;

        /** @brief Tag0 DEI/CFI value update command for the packet:
         *  (APPLICABLE DEVICES: AC5X; Harrier; Ironman; AC5P)
         */
        CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT cfi0;

        /** @brief Flag to indicate "To update tag1 from UDB"
         *  GT_TRUE  - Copy UDB 48 and UDB 49 into Tag 1 VLAN-ID
         *  GT_FALSE - Do not update
         *  (APPLICABLE DEVICES: AC5X; Harrier; AC5P)
         */
        GT_BOOL updateTag1FromUdb;

        /** @brief Flag to indicate "copy UDB" to descriptor
         *  GT_TRUE  - Copy UDB 48 and UDB 49 into descriptor field, depending on cpssDxChPclEpgConfigSet
         *  GT_FALSE - Do not update
         *  (APPLICABLE DEVICES: Ironman)
         */
        GT_BOOL copyUdb;
} CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_VLAN_EGRESS_STC
*
*  @brief This structure defines the egress VLAN modification
*         related Actions.
*/
typedef struct{

        CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT       vlanCmd;
        GT_U32                                         vlanId;
        GT_BOOL                                        vlanId1ModifyEnable;
        GT_U32                                         vlanId1;

} CPSS_DXCH_PCL_ACTION_VLAN_EGRESS_STC;


/*
 * Typedef: union CPSS_DXCH_PCL_ACTION_VLAN_UNT
 *
 * Description:
 *     This union defines the VLAN modification related Actions.
 *
 * Fields:
 *       INGRESS fields:
 *       modifyVlan  - VLAN id modification command
 *
 *       nestedVlan  -  When this field is set to GT_TRUE, this rule matching
 *                      flow is defined as an access flow. The VID of all
 *                      packets received on this flow is discarded and they
 *                      are assigned with a VID using the device's VID
 *                      assignment algorithms, as if they are untagged. When
 *                      a packet received on an access flow is transmitted via
 *                      a core port or a Cascading port, a VLAN tag is added
 *                      to the packet (on top of the existing VLAN tag, if
 *                      any). The VID field is the VID assigned to the packet
 *                      as a result of all VLAN assignment algorithms. The
 *                      802.1p User Priority field of this added tag may be
 *                      one of the following, depending on the ModifyUP QoS
 *                      parameter set to the packet at the end of the Ingress
 *                      pipe:
 *                      - If ModifyUP is GT_TRUE, it is the UP extracted
 *                      from the QoSProfile to QoS Table Entry<n>
 *                      - If ModifyUP is GT_FALSE, it is the original packet
 *                      802.1p User Priority field if it is tagged and is UP
 *                      if the original packet is untagged.
 *
 *       vlanId      - VLAN id used for VLAN id modification if command
 *                     not CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E
 *       precedence  - The VLAN Assignment precedence for the subsequent
 *                     VLAN assignment mechanism, which is the Policy engine
 *                     next policy-pass rule. Only relevant if the
 *                     VID precedence set by the previous VID assignment
 *                     mechanisms (Port, Protocol Based VLANs, and previous
 *                     matching rule) is Soft.
 *                     - Soft precedence The VID assignment can be overridden
 *                       by the subsequent VLAN assignment mechanism,
 *                       which is the Policy engine.
 *                     - Hard precedence The VID assignment is locked to the
 *                       last VLAN assigned to the packet and cannot be overridden.
 *       vlanId1Cmd  - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *                     This field determines the Tag1 VID assigned
 *                     to the packet matching the IPCL entry.
 *       vlanId1     - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *                     The Tag1 VID to assign to the packet
 *                     according to <vlanId1Cmd>.
 *       EGRESS fields:
 *       vlanCmd     - This field determines the Tag0 VID command
 *                     assigned to the packet matching the EPCL entry.
 *       vlanId      - The VID to assign to Tag0
 *                     of the packet according to <vlanCmd>
 *       vlanId1ModifyEnable - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *                     This field determines the Tag1 VID
 *                     assigned to the packet matching the EPCL entry.
 *                     GT_TRUE - modify, GT_FALSE - don't modify.
 *       vlanId1     - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *                     The VID to assign to Tag1 of the packet
 *                     according to <vlanId1ModifyEnable>.
 *
 *  Comments:
 *
 */
typedef union
{

    CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_STC ingress;

    CPSS_DXCH_PCL_ACTION_VLAN_EGRESS_STC egress;

} CPSS_DXCH_PCL_ACTION_VLAN_UNT;

/**
* @struct CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC
 *
 * @brief This structure defines the IP unicast route parameters.
 * Dedicated to override the relevant field of
 * general action definitions.
 * APPLICABLE DEVICES: xCat3, AC5, Lion2.
*/
typedef struct{

    /** @brief Configure IP Unicast Routing Actions
     *  GT_TRUE - the action used for IP unicast routing
     *  GT_FALSE - the action is not used for IP unicast
     *  routing, all data of the structure
     *  ignored.
     */
    GT_BOOL doIpUcRoute;

    /** Route Entry ARP Index to the ARP Table (10 bit) */
    GT_U32 arpDaIndex;

    /** @brief Decrement IPv4 <TTL> or IPv6 <Hop Limit> enable
     *  GT_TRUE - TTL Decrement for routed packets is enabled
     *  GT_FALSE - TTL Decrement for routed packets is disabled
     */
    GT_BOOL decrementTTL;

    /** @brief Bypass Router engine TTL and Options Check
     *  GT_TRUE - the router engine bypasses the
     *  IPv4 TTL/Option check and the IPv6 Hop
     *  Limit/Hop-by-Hop check. This is used for
     *  IP-TO-ME host entries, where the packet
     *  is destined to this device
     *  GT_FALSE - the check is not bypassed
     */
    GT_BOOL bypassTTLCheck;

    /** @brief ICMP Redirect Check Enable
     *  GT_TRUE - the router engine checks if the next hop
     *  VLAN is equal to the packet VLAN
     *  assignment, and if so, the packet is
     *  mirrored to the CPU, in order to send an
     *  ICMP Redirect message back to the sender.
     *  GT_FALSE - the check disabled
     *  Comments:
     *  To configure IP Unicast route entry next elements of the action struct
     *  should be configured
     *  1. doIpUcRoute set to GT_TRUE
     *  2. redirection action set to CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
     *  Out Interface data should be configured too.
     *  3. Policer should be disabled
     *  4. VLAN command CPSS_DXCH_PCL_ACTION_VLAN_CMD_MODIFY_ALL_E
     *  5. Nested VLAN should be disabled
     *  6. The packet command <pktCmd> should be set according to route entry
     *  purpose:
     *  - SOFT_DROP    - packet is dropped
     *  - HARD_DROP    - packet is dropped
     *  - TRAP      - packet is trapped to CPU with CPU code
     *  IPV4_UC_ROUTE or IPV6_UC_ROUTE
     *  - FORWARD     - packet is routed
     *  - MIRROR_TO_CPU  - packet is routed and mirrored to the CPU with
     *  CPU code IPV4_UC_ROUTE or IPV6_UC_ROUTE
     */
    GT_BOOL icmpRedirectCheck;

} CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC
 *
 * @brief This structure defines packet Source Id assignment.
*/
typedef struct{

    /** @brief assign Source Id enable:
     *  - GT_TRUE - Assign Source Id.
     *  - GT_FALSE - Don't assign Source Id.
     */
    GT_BOOL assignSourceId;

    /** @brief the Source Id value to be assigned
     *  Comments:
     */
    GT_U32 sourceIdValue;

} CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_OAM_STC
 *
 * @brief This structure defines packet OAM relevant fields.
*/
typedef struct{

    /** @brief indicates that a timestamp should be inserted into the
     *  packet:
     *  - GT_TRUE - enable timestamp insertion.
     *  - GT_FALSE - disable timestamp insertion.
     */
    GT_BOOL timeStampEnable;

    /** @brief When <Timestamp Enable> is enabled, indicates
     *  the offset index for the timestamp offset table.
     *  (APPLICABLE RANGES: 0..127)
     */
    GT_U32 offsetIndex;

    /** @brief binds the packet to an entry in the OAM Table.
     *  - GT_TRUE - enable OAM packet processing.
     *  - GT_FALSE - disable OAM packet processing.
     */
    GT_BOOL oamProcessEnable;

    /** @brief determines the set of UDBs where the key attributes
     *  (opcode, MEG level, RDI, MEG level) of the OAM message
     *  are taken from.
     *  (APPLICABLE RANGES: 0..1)
     *  Comments:
     */
    GT_U32 oamProfile;

} CPSS_DXCH_PCL_ACTION_OAM_STC;

/**
* @enum CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT
 *
 * @brief enumerator Controls the index used for IPCL lookup.
*/
typedef enum{

    /** @brief Retain;
     *  use SrcPort/VLAN <Lookup PCL Cfg Mode>.
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E,

    /** @brief Override;
     *  use TTI/PCL Action <ipclConfigIndex>.
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E

} CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
 *
 * @brief This structure defines future lookups configuration.
*/
typedef struct{

    /** @brief index of Pcl Configuration table for next lookup.
     *  0 means no value to update.
     */
    GT_U32 ipclConfigIndex;

    /** @brief algorithm of selection
     *  index of PCL Cfg Table for IPCL lookup0_1
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0_1OverrideConfigIndex;

    /** @brief algorithm of selection
     *  index of PCL Cfg Table for IPCL lookup1
     *  Comments:
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl1OverrideConfigIndex;

} CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC
 *
 * @brief This structure defines Source ePort Assignment.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_FALSE
     *  - GT_TRUE - Assign source ePort from sourcePortValue.
     */
    GT_BOOL assignSourcePortEnable;

    /** @brief Source ePort value.
     *  Comments:
     */
    GT_U32 sourcePortValue;

} CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC;

/**
 * @struct CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC
 *
 * @brief A structure that describes latency PCL action.
 */
typedef struct
{
    /** @brief Enable latency monitoring */
    GT_BOOL monitoringEnable;

    /** @brief Assign latency monitoring profile */
    GT_U32  latencyProfile;
} CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC;


/**
 * @enum CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT
 *
 * @brief This enumeration lists all the Egress PCL Action PHA thread-id assignment modes.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{
    /** @brief the threadId is not modified by the EPCL action */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E,
    /** @brief the threadId is modified by the EPCL action .
     *  NOTE: error : GT_NOT_INITIALIZED if the PHA library was not initialized.
    */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E,
    /** @brief the EPCL action set to bypass the PHA thread processing
     *  NOTE: error : GT_NOT_INITIALIZED if the PHA library was not initialized.
    */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT;

/**
 * @enum CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT
 *
 * @brief This enumeration lists all the Egress PCL Action PHA thread info types.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{
    /** @brief the PHA threadId is not used */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E,
    /*  @brief
     *  NOTE: error : GT_NOT_INITIALIZED if the PHA library was not initialized.
    */
    /** @brief type for thread to handle IOAM Ipv4/Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E,

    /** @brief type for thread to handle ERSPAN packets in the egress switch in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_E,

    /** @brief type for thread to handle Classifier NSH over VXLAN-GPE packets */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E,

    /** @brief type for thread to handle ERSPAN Type II mirroring packets */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E,

    /** @brief type for thread to handle SFLOW mirroring packets */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_MIRROR_E,

    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD___LAST___E   /* not to be used */
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4/ipv6 packets in the ingress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{
    /** @brief indication that the IOAM includes both the E2E option and trace option
        GT_TRUE  - both the E2E option and trace option
        GT_FALSE - only one option exists
    */
    GT_BOOL  ioamIncludesBothE2EOptionAndTraceOption;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_INT_IOAM_MIRROR_STC
 *
 * @brief This structure defines the fields, used for ERSPAN III header update
 *        PHA thread does the update for INT/IOAM Ipv4/ipv6 packets.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{
    /** @brief indication that the ERSPAN payload is an Ethernet protocol frame (PDU frame)
    */
    GT_BOOL  erspanIII_P;

    /** @brief The frame type
     * (APPLICABLE RANGES: 0..31).
    */
    GT_U32  erspanIII_FT;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_STC;

/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_STC
 *
 * @brief This structure defines the PHA shared memory index used for ERSPAN II header update,
 *        PHA thread does the update for ERSPAN Ipv4/ipv6 packets.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief The analyzer index in PHA shared memory for IP and L2 header template
     * (APPLICABLE RANGES: 0..6).
    */
    GT_U32  erspanAnalyzerIndex;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_STC;

/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_STC
 *
 * @brief This structure defines the PHA metadata for SFLOW v5 PHA threads.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief The Sflow sampling rate information
    */
    GT_U32  sflowSamplingRate;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_STC ;
/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC
 *
 * @brief This structure defines the NSH metadata fields that are updated by PHA in
 *        classifier NSH over vxlan-gpe thread.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{
    /** @brief From standard:
             The tenant identifier is used to represent the tenant that the Service Function Chain
             is being applied to. The Tenant ID is a unique value assigned by a control plane.
             (APPLICABLE RANGES: 0..0xFFFF)
     */
    GT_U32  classifierNshOverVxlanGpe_TenantID;

    /** @brief From standard:
             Represents the logical classification of the source of the traffic. For example,
             it might represent a source application, a group of endpoints, or a set of users
             originating the traffic. This grouping is done for the purposes of applying policy.
             Policy is applied to groups rather than individual endpoints.
             (APPLICABLE RANGES: 0..0xFFFF)
    */
    GT_U32 classifierNshOverVxlanGpe_SourceClass;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC;


/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT
 *
 * @brief This union if for all the Egress PCL Action PHA thread info types.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef union{
    /** @brief dummy field for type that no extra info needed. */
    GT_U32 notNeeded;

    /** @brief EPCL info for thread to handle IOAM Ipv4/Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_STC  epclIoamIngressSwitch;

    /** @brief EPCL info for thread to handle IOAM Ipv4/Ipv6 packets in the egress switch in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_STC  epclIntIoamMirror;

    /** @brief EPCL info for thread to handle Classifier NSH over VXLAN-GPE packets type */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC  epclClassifierNshOverVxlanGpe;

    /** @brief EPCL info for thread to handle ERSPAN Ipv4/Ipv6 packets in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_STC epclErspanTypeIImirror;

    /** @brief EPCL info for thread to handle SFLOW packets in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_STC epclSflowV5Mirror;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT;

/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC
 *
 * @brief This structure defines the Egress PCL Action fields relate to PHA thread info.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{
    /** @brief epcl pha thread Id assignment mode.
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT      phaThreadIdAssignmentMode;
    /** @brief pha threadId . relevant only if phaThreadIdAssignmentMode == CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E
     * (APPLICABLE RANGES: 1..255).
     */
    GT_U32       phaThreadId;
    /** @brief epcl pha thread info type
     *  NOTE: relevant regardless to mode of 'phaThreadIdAssignmentMode'
     *
     *  IMPORTANT: for 'Get' function this parameter is [in] parameter !!!
     *      (if the HW value meaning that value other than
     *      CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E needed)
     *      because CPSS need to use it in order to parse the HW value and fill
     *      the SW 'phaThreadUnion' info.
     *      (CPSS not hold shadow for it , because 'per action' per 'port group'
     *      (and 'per device') too large shadow to support it ...)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT      phaThreadType;
    /** @brief union of all the epcl pha thread info.(according to phaThreadType)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT      phaThreadUnion;

}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC;

/**
 * @struct CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC
 *
 * @brief Defines the IPCL/EPCL action <copyReserved> field
 *
 *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct
{
    /** @brief IPCL/EPCL Copy Reserved Assignment Enable
     *  GT_TRUE -  Assigns IPCL/EPCL action<copyReserved>
     *             to packet's descriptor<copyReserved> field
     *  GT_FALSE - No change to the packet descriptor<copyReserved>
     *             field by IPCL/EPCL engine
     *  Note: For devices AC5X; Harrier; AC5P.  If set, enables assigning the 
     *  packet’s Source Group ID and Destination Group ID. 
     */
    GT_BOOL  assignEnable;

    /** @brief IPCL/EPCL reserved field value assignment
     *  Relevant when assignEnable == GT_TRUE
     *  copyReserved mask enables selected bit assignment of reserved
     *  IPCL/EPCL action entry field into descr<copyReserved>
     *  For IPCL mask can set per stage
     *  cpssDxChPclCopyReservedMaskSet is used to set the mask.
     *  (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon 0..0x3FFFF)
     *  (APPLICABLE RANGES: AC5X; Harrier; Ironman, AC5P 0..0x7FFFF)
     *  Note: For devices AC5X; Harrier; AC5P.  IPCL Action<Group IDs> after masking is 
     *  assigned to the packet’s Source Group ID (9 bits) and to the packet’s Destination Group ID (9 bits).
     *  Bits [17:0] are used as Group IDs.
     */
    GT_U32   copyReserved;

}CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_STC
 *
 * @brief Policy Engine Action
*/
typedef struct{

    /** @brief packet command (forward, mirror
     *  hard-drop, soft-drop, or trap-to-cpu)
     */
    CPSS_PACKET_CMD_ENT pktCmd;

    /** @brief Action Stop
     *  GT_TRUE - to skip the following PCL lookups
     *  GT_FALSE - to continue with following PCL lookups
     *  Supported by DxCh3 and xCat above devices.
     *  Relevant to Policy Action Entry only.
     */
    GT_BOOL actionStop;

    /** @brief the Bridge engine processed or bypassed
     *  GT_TRUE - the Bridge engine is bypassed.
     *  GT_FALSE - the Bridge engine is processed.
     */
    GT_BOOL bypassBridge;

    /** @brief the ingress pipe bypassed or not.
     *  GT_TRUE - the ingress pipe is bypassed.
     *  GT_FALSE - the ingress pipe is not bypassed.
     */
    GT_BOOL bypassIngressPipe;

    /** @brief GT_TRUE
     *  GT_FALSE - Action is used for the Ingress Policy
     */
    GT_BOOL egressPolicy;

    /** configuration of IPCL lookups. */
    CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC lookupConfig;

    /** @brief packet mirroring configuration
     *  Relevant for ingress actions only.
     *
     *  Relevant for egress actions , for next devices:
     *  (APPLICABLE DEVICES : Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    CPSS_DXCH_PCL_ACTION_MIRROR_STC mirror;

    /** match counter configuration */
    CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC matchCounter;

    /** packet QoS attributes modification configuration */
    CPSS_DXCH_PCL_ACTION_QOS_UNT qos;

    /** @brief packet Policy redirection configuration
     *  Relevant for ingress actions only.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_STC redirect;

    /** @brief packet Policing configuration
     *  Relevant to Policy Action Entry only.
     */
    CPSS_DXCH_PCL_ACTION_POLICER_STC policer;

    /** packet VLAN modification configuration */
    CPSS_DXCH_PCL_ACTION_VLAN_UNT vlan;

    /** @brief special DxCh (not relevant for DxCh2 and above) Ip Unicast Route
     *  action parameters configuration.
     *  Relevant to Unicast Route Entry only.
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC ipUcRoute;

    /** @brief packet source Id assignment
     *  Relevant to Policy Action Entry only.
     */
    CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC sourceId;

    /** OAM relevant fields. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_DXCH_PCL_ACTION_OAM_STC oam;

    /** @brief flow Id. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Relevant for ingress actions only.
     *  (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon: 0 - 8K-1
     *                      AC5P; AC5X; Harrier; Ironman: 0 - 64K-1)
     */
    GT_U32 flowId;

    /** @brief source ePort Assignment.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC sourcePort;

    /** @brief GT_FALSE
     *  - GT_TRUE - Override the Mac2Me mechanism and set Mac2Me to 0x1.
     *  Ingress PCL Only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL setMacToMe;

    /** @brief When enabled, the MPLS G
     *  to an OAM Opcode that is used by the OAM engine.
     *  - GT_FALSE - Disable.
     *  - GT_TRUE - Enable.
     *  Egress PCL Only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL channelTypeToOpcodeMapEnable;

    /** @brief TM Queue ID assignment.
     *  This value is assigned to the outgoing descriptor.
     *  It may be subsequently modified by the TM Q-Mapper.
     *  Egress PCL Only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     */
    GT_U32 tmQueueId;

    /** @brief GT_TRUE
     *  GT_FALSE configure action to preserve previous assignment.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat3; AC5)
     */
    GT_BOOL unknownSaCommandEnable;

    /** @brief Assigns the unknown
     *  Relevant when unknownSaCommandEnable == GT_TRUE.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat3; AC5)
     *  Comment:
     */
    CPSS_PACKET_CMD_ENT unknownSaCommand;

    /** @brief GT_TRUE - Terminate Cut Through packet mode (switch to Store and Forward).
     *         GT_FALSE - leave packet mode (CT or SF) unchanged.
     *   Terminate Cut Through packet mode (switch to Store and Forward).
     *   Relevant to Egress PCL only.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL terminateCutThroughMode;

    /** @brief Latency monitoring
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC latencyMonitor;

    /** @brief GT_TRUE - Skip FDB SA lookup.
     *   GT_FALSE - Don't modify the state of Skip FDB SA lookup.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL skipFdbSaLookup;

    /** @brief GT_TRUE - The Ingress PCL will trigger a dedicated
     *         interrupt towards the CPU.
     *   GT_FALSE - will not trigger.
     *   Relevant to Ingress PCL Only.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL triggerInterrupt;

     /** @brief GT_TRUE - The Exact Match action has a priority over the PCL action.
     *          GT_FALSE - The Exact Match action doesn't have a
     *          priority over the PCL action.
     *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL exactMatchOverPclEn;

     /** @brief EPCL action info related to egress PHA thread processing.
     *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC     epclPhaInfo;

    /**  @brief IPCL/EPCL Action info related to IPCL/EPCL copyReserved
                value assignment
        (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
    */
    CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC  copyReserved;

    /** @brief GT_TRUE - Packet subject to IPFIX processing and access IPFIX
     *   table with flowId as index.
     *   GT_FALSE - Packet not subject to IPFIX processing and no IPFIX table
     *   access.
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL ipfixEnable;

    /** @brief PCL action to trigger Hash CNC client
     *  GT_FALSE - Don't enable CNC Hash client
     *  GT_TRUE - Enable CNC Hash client
     *  Relevant to Ingress PCL Only.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
     GT_BOOL  triggerHashCncClient;

    /** @brief EPCL action: CNC Index mode
     *   (APPLICABLE DEVICES: Ironman)
     */
     CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_ENT     egressCncIndexMode;

    /** @brief EPCL action: Enable Max SDU Size Check
     *   GT_FALSE - The Max SDU Size Check is disabled
     *   GT_TRUE - The Max SDU Size Check is enabled
     *   (APPLICABLE DEVICES: Ironman)
     */
     GT_BOOL     enableEgressMaxSduSizeCheck;

    /** @brief EPCL action: The Egress Max SDU Size Profile for this flow.
     *   The value is used as an index into Maximum SDU Size Profile Check.
     *   (APPLICABLE DEVICES: Ironman)
     *   (APPLICABLE RANGES: 0..3)
     */
     GT_U32     egressMaxSduSizeProfile;

} CPSS_DXCH_PCL_ACTION_STC;


/**
* @enum CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT
 *
 * @brief This enum describes possible formats of Policy rules.
*/
typedef enum{

    /** @brief Standard (24B) L2
     *  AKA ingress Key #0
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                        ,

    /** @brief Standard (24B)
     *  L2+IPv4/v6 QoS
     *  AKA - ingress Key #1
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E                     ,

    /** @brief Standard (24B)
     *  IPv4+L4
     *  AKA - ingress Key #2
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                       ,

    /** @brief Standard (24B)
     *  DxCh specific IPV6 DIP (used for routing)
     *  AKA - ingress Key #12
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                      ,

    /** @brief Extended (48B)
     *  L2+IPv4 + L4
     *  AKA - ingress Key #4
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                      ,

    /** @brief Extended (48B)
     *  L2+IPv6
     *  AKA - ingress Key #5
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                       ,

    /** @brief Extended (48B)
     *  L4+IPv6
     *  AKA - ingress Key #6
     *  key formats for Egress (DxCh2 and above device)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                       ,

    /** Standard (24B) L2 */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                         ,

    /** @brief Standard (24B)
     *  L2+IPv4/v6 QoS
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                      ,

    /** @brief Standard (24B)
     *  IPv4+L4
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                        ,

    /** @brief Extended (48B)
     *  L2+IPv4 + L4
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                      ,

    /** @brief Extended (48B)
     *  L2+IPv6
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                       ,

    /** @brief Extended (48B)
     *  L4+IPv6
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                       ,

    /** @brief standard UDB key (24B)
     *  AKA ingress Key #3
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                          ,

    /** @brief extended UDB key (48B)
     *  AKA ingress Key #7
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                          ,

    /** @brief (ingress)
     *  Standard (24B) Routed ACL+QoS+IPv4
     *  AKA - ingress Key #10
     *  CH3 : key for IPv4 packets used in first lookup
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E          ,

    /** @brief (ingress)
     *  Extended (48B) Port/VLAN+QoS+IPv4
     *  AKA - ingress Key #8
     *  CH3 : key for IPv4 packets use in second lookup
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E           ,

    /** @brief (ingress)
     *  Ultra (72B) Port/VLAN+QoS+IPv6
     *  AKA - ingress Key #9
     *  CH3 : key for IPv6 packets used in first lookup
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E         ,

    /** @brief (ingress)
     *  Ultra (72B) Routed ACL+QoS+IPv6
     *  AKA - ingress Key #11
     *  CH3 : key for IPv6 packets use in second lookup.
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E        ,

    /** @brief (egress)
     *  Extended (48B) RACL/VACL IPv4
     *  CH3 : Custom egress key for not-IP, IPV4, ARP
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E                ,

    /** @brief (egress)
     *  Ultra (72B) RACL/VACL IPv6
     *  CH3 : Custom egress key for IPV6
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E              ,

    /** @brief (ingress)
     *  10 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E                           ,

    /** @brief (ingress)
     *  20 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E                           ,

    /** @brief (ingress)
     *  30 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E                           ,

    /** @brief (ingress)
     *  40 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E                           ,

    /** @brief (ingress)
     *  50 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E                           ,

    /** @brief (ingress)
     *  50 byte UDB and fixed fields
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E                           ,

    /** @brief (egress)
     *  10 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E                            ,

    /** @brief (egress)
     *  20 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E                            ,

    /** @brief (egress)
     *  30 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E                            ,

    /** @brief (egress)
     *  40 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E                            ,

    /** @brief (egress)
     *  50 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E                            ,

    /** @brief (egress)
     *  50 byte UDB and fixed fields
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E                            ,

    /** @brief (ingress)
     *  60 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E           ,

    /** @brief PCL key 80B (ingress)
     *   70 Byte UDB and 10B fixed fields
     *   (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E                           ,

    /** @brief  PCL key 60B (Egress)
     *   60 byte UDB only key
     *   (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E            ,

    /** @brief last element to
     *  calculate amount
     */
    CPSS_DXCH_PCL_RULE_FORMAT_LAST_E

} CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_OFFSET_TYPE_ENT
 *
 * @brief Offset types for packet headers parsing used for user defined
 * bytes configuration
*/
typedef enum{

    /** offset from start of mac(L2) header */
    CPSS_DXCH_PCL_OFFSET_L2_E,

    /** @brief offset from start of L3 header.
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_DXCH_PCL_OFFSET_L3_E,

    /** offset from start of L4 header */
    CPSS_DXCH_PCL_OFFSET_L4_E,

    /** @brief offset from start of
     *  IPV6 Extension Header .
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_DXCH_PCL_OFFSET_IPV6_EXT_HDR_E,

    /** @brief the user defined byte used
     *  for TCP or UDP comparator, not for byte from packet
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2)
     */
    CPSS_DXCH_PCL_OFFSET_TCP_UDP_COMPARATOR_E,

    /** @brief offset from start of
     *  L3 header minus 2. Ethertype of IP.
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,

    /** @brief offset from
     *  Ethertype of MPLS minus 2.
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E,

    /** @brief offset from Tunnel L2
     *  For IPCL refers to the beginning of the original packet's MAC header.
     *  (prior to tunnel-termination).
     *  If packet was not tunnel-terminated - this anchor is invalid.
     *  For EPCL refers to the beginning of the outgoing packet's MAC header
     *  (after tunnel-start).
     *  If packet was not tunnel-start this anchor is invalid;
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E,

    /** @brief offset from Tunnel L3
     *  The beginning of the L3 header in tunnel minus 2 bytes.
     *  For IPCL refers to the beginning of original packet's header.
     *  (prior to tunnel termination).
     *  If packet was not tunnel terminated this anchor is invalid.
     *  Only fields in the tunnel can be selected.
     *  Offset cannot exceed 56 bytes.
     *  For EPCL valid only if packet is tunnel start.
     *  Points to the beginning of L4 header in the added tunnel header
     *  minus 2 bytes. Offset cannot exceed 64 bytes.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,

    /** @brief offset from Metadata.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_METADATA_E,

    /** @brief offset from Tunnel L4
     *  For IPCL, this is the start of the original packet's tunnel L4 header.
     *  This anchor type is valid only for tunnel-terminated packets.
     *  For EPCL, not supported
     *  If packet was not tunnel-start this anchor is invalid;
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L4_E,

    /** @brief invalid UDB - contains 0 always
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_OFFSET_INVALID_E

} CPSS_DXCH_PCL_OFFSET_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_PACKET_TYPE_ENT
 *
 * @brief Packet types.
*/
typedef enum{

    /** @brief IPV4 TCP
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,

    /** @brief IPV4 UDP
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,

    /** @brief MPLS
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E,

    /** @brief IPV4 Fragment
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E,

    /** @brief IPV4 Other
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,

    /** @brief Ethernet Other
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,

    /** @brief User Defined Ethertype
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE_E,

    /** @brief IPV6
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E,

    /** @brief User Defined Ethertype1
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,

    /** @brief User Defined Ethertype2
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E,

    /** @brief User Defined Ethertype3
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E,

    /** @brief User Defined Ethertype4
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E,

    /** @brief IPV6 TCP
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E,

    /** @brief IPV6 UDP
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E,

    /** @brief IPV6 not UDP and not TCP
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E,

    /** @brief User Defined Ethertype5
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E,

    /** @brief User Defined Ethertype6
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E,

    /** last value (for internal use) */
    CPSS_DXCH_PCL_PACKET_TYPE_LAST_E

} CPSS_DXCH_PCL_PACKET_TYPE_ENT;

/* Maximal UDB index. Devices Bobcat2, Caelum, Aldrin, AC3X support only 50 UDBs */
/* (APPLICABLE DEVICES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman, Aldrin, AC3X) */
#define CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS 70

/* Amount of replaceable UDBs in Ingress UDB only keys */
#define CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS 19

/**
* @enum CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_ENT
 *
 * @brief Defines the value of Key Bytes 9,15,16 when Key Bytes 9,15,16 are replaced with descriptor fields.
 *        (APPLICABLE DEVICES: Ironman)
*/
typedef enum{

    /** @brief UDBs 9,15,16 are replaced with hash and QoS
     *  byte_9       = QosProfile[9:8]
     *  byte_15      = hashValue [31:24]
     *  byte_16      = hashValue [23:16]
     */
    CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_HASH_AND_QOS_E,

    /** @brief UDBs 9,15,16 are replaced with {Source EPG and Destination EPG}
     *  byte_9       = src_epg[11:4]
     *  byte_15[3:0] = src_epg[3:0]
     *  byte_15[7:4] = dst_epg[11:8]
     *  byte_16      = dst_epg[7:0]
     */
    CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_SRC_DST_EPG_E,

    /** @brief UDBs 9,15,16 are replaced with {Source-ID and Copy Reserved}
     *  byte_9       = sst_id[11:4]
     *  byte_15[3:0] = sst_id[3:0]
     *  byte_15[7:4] = copy_reserved[12:9]
     *  byte_16      = copy_reserved[8:1]
     */
    CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_SRC_ID_11_0_COPY_RES_12_1_E,

    /** @brief UDBs 9,15,16 are replaced with {Source-ID[3:0] and Copy Reserved[19:0]}
     *  byte_9[3:0]  = copy_reserved[19:16]
     *  byte_9[7:4]  = sst_id[3:0]
     *  byte_15      = copy_reserved[15:8]
     *  byte_16      = copy_reserved[7:0]
     */
    CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_SRC_ID_3_0_COPY_RES_19_0_E

} CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_ENT;

/**
* @struct CPSS_DXCH_PCL_UDB_SELECT_STC
 *
 * @brief This structure defines User Defined Bytes Selection
 * for UDB only keys.
 * (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief The UDB number in the global UDB pool that
     *  is selected as UDB number-n in this UDB only key.
     *  The index in this array is the number of UDB-position
     *  in appropriate UDB only key.
     */
    GT_U32 udbSelectArr[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief  enabling replacing UDBs in the key by specific not-UDB data.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Ingress PCL only.
     *  udbReplaceArr[0]  - UDB0  replaced with {PCL-ID[7:0]}
     *  udbReplaceArr[1]  - UDB1  replaced with {UDB Valid,reserved,PCL-ID[9:8]}
     *  udbReplaceArr[1]  - UDB1  replaced with {UDB Valid, PCL-ID[11:8]}
     *                      (APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman)
     *  udbReplaceArr[2]  - UDB2  replaced with eVLAN[7:0]
     *  udbReplaceArr[3]  - UDB3  replaced with eVLAN[12:8]
     *  udbReplaceArr[4]  - UDB4  replaced with Source-ePort[7:0]
     *  udbReplaceArr[5]  - UDB5  replaced with Src-Dev-Is-Own and Source-ePort[14:8]
     *  udbReplaceArr[6]  - UDB6  replaced with Tag1-VID[7:0]
     *  udbReplaceArr[7]  - UDB7  replaced with MAC2ME and Tag1-{UP[2:0],VID[11:8]}
     *  udbReplaceArr[8]  - UDB8  replaced with QoS-Profile[7:0]
     *  udbReplaceArr[9]  - UDB9  replaced with QoS-Profile[9:8]
     *  udbReplaceArr[10] - UDB10 replaced with Flow-ID[7:0]
     *  udbReplaceArr[11] - UDB11 replaced with Flow-ID[11:8]
     *  udbReplaceArr[12] - UDB12 replaced with IPCL stage n port PCLID2[23:16]
     *  udbReplaceArr[13] - UDB13 replaced with IPCL stage n port PCLID2[15:8]
     *  udbReplaceArr[14] - UDB14 replaced with IPCL stage n port PCLID2[7:0]
     *  udbReplaceArr[15] - UDB15 replaced with hash[31:24]
     *  udbReplaceArr[16] - UDB16 replaced with hash[23:16]
     *  udbReplaceArr[17] - UDB17 replaced with hash[15:8]
     *  udbReplaceArr[18] - UDB18 replaced with hash[7:0]
     *  Note: From UDB[12]-[18] (APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman)
     */
    GT_BOOL ingrUdbReplaceArr[CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS];

    /** @brief UDBs 0,1 are replaced with {PCL-ID[9:0]},
     *  padded with leading zeros.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Egress PCL only.
     */
    GT_BOOL egrUdb01Replace;

    /** @brief <UDB Valid> indication replaces bit 15 of the key,
     *  overriding any other configuration of bit 15 content,
     *  including the configuration in egrUdb01Replace.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Egress PCL only.
     *  Comments:
     */
    GT_BOOL egrUdbBit15Replace;

    /** @brief <Key Byte replacement mode> indicates
     *  Defines the value of Key Bytes 9,15,16 when Key, Bytes 9,15,16 are replaced with descriptor fields
     *  Relevant for ingress PCL only.
     *  (APPLICABLE DEVICES: Ironman)
     */
    CPSS_DXCH_PCL_KEY_BYTE_REPLACEMENT_MODE_ENT keyByteReplacementMode;

} CPSS_DXCH_PCL_UDB_SELECT_STC;

/**
* @enum CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT
 *
 * @brief User Defined Bytes override types.
*/
typedef enum{

    /** @brief VRF ID MSB
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,

    /** @brief VRF ID LSB
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,

    /** @brief Qos profile index
     *  (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,

    /** @brief Trunk Hash
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,

    /** @brief Tag1 info
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,

    /** @brief VRF ID MODE
     *  common for all key types
     *  (APPLICABLE DEVICES: xCat3; AC5)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MODE_E

} CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT;

/**
* @enum CPSS_DXCH_UDB_ERROR_CMD_ENT
 *
 * @brief This enum defines packet command taken in the UDB error case.
*/
typedef enum{

    /** continue Policy Lookup */
    CPSS_DXCH_UDB_ERROR_CMD_LOOKUP_E,

    /** trap packet to CPU */
    CPSS_DXCH_UDB_ERROR_CMD_TRAP_TO_CPU_E,

    /** hard drop packet */
    CPSS_DXCH_UDB_ERROR_CMD_DROP_HARD_E,

    /** soft drop packet */
    CPSS_DXCH_UDB_ERROR_CMD_DROP_SOFT_E

} CPSS_DXCH_UDB_ERROR_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT
 *
 * @brief Type of access to Ingress / Egress PCL configuration Table.
 * Type of access defines how device calculates index of PCL
 * configuration Table for packet.
*/
typedef enum{

    /** @brief by (physical) PORT access mode.
     *  Ingress port or trunk id is used to calculate index of the PCL
     *  configuration table
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,

    /** @brief by VLAN access mode
     *  VLAN ID is used to calculate index of the PCL configuration table.
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E,

    /** @brief by ePORT access mode.
     *  eport is used to calculate index of the PCL configuration table.
     *  for ingress : local device source ePort is used (12 LSBits)
     *  for egress : target ePort is used (12 LSBits)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E,

    /** @brief by SOURCE Target mode.
     *  Use Src Trg; Use the Source Index (6 bits) and Target Index (6 bits)
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_SRC_TRG_E

} CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT
 *
 * @brief This enum defines the ip-total-length check algorithm
 * that used for calculate the key "IP Header OK" bit.
*/
typedef enum{

    /** @brief compare
     *  iptotallengts with the size of L3 level part of the packet
     */
    CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_L3_E,

    /** @brief compare
     *  iptotallengts with the total size of the packet
     */
    CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_L2_E

} CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT
 *
 * @brief EPCL Key<SrcPort> field source selector for DSA tagged packets.
*/
typedef enum{

    /** For DSA tagged packet, <SrcPort> is taken from the DSA tag. */
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E,

    /** @brief For all packets (DSA tagged or not), <SrcPort> indicates the
     *  local device ingress physical port.
     */
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E

} CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT;


/**
* @enum CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT
 *
 * @brief EPCL Key<Target port> field source selector.
*/
typedef enum{

    /** Local Device Physical Port. */
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E,

    /** Final Destination Port. */
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E

} CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT;

/*-------------------------------------------------------------------------*/
/*                           Policy Rules Formats                          */
/*-------------------------------------------------------------------------*/

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC
 *
 * @brief This structure describes the common segment of all key formats.
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: AC5; Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     *  For AC5 the portListBmp[27:16] is muxed with:
     *      STD_NOT_IP     -  UDB15, UDB15 and vrfId
     *      STD_IP_L2_QOS  -  commonStdIp.l4Byte2 and commonStdIp.l4Byte3
     *      STD_IPV4_L4    -  commonStdIp.l4Byte2 and commonStdIp.l4Byte3
     *      EXT_NOT_IPV6   -  commonExt.l4Byte2 and commonExt.l4Byte3
     *      EXT_IPV6_l2    -  commonExt.l4Byte2 and commonExt.l4Byte3
     *      EXT_IPV6_l4    -  commonExt.l4Byte2 and commonExt.l4Byte3
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** @brief Flag indicating the packet Tag state
     *  For Ingress PCL Keys:
     *  For non-tunnel terminated packets:
     *  - If packet was received on DSA-tagged, this field is set
     *  to the DSA tag <SrcTagged> field.
     *  - If packet was received non-DSA-tagged, this field is set
     *  to 1 if the packet was received VLAN or Priority-tagged.
     *  For Ethernet-Over-xxx tunnel-terminated packets: Passenger
     *  packet VLAN tag format.
     *  For Egress PCL Keys:
     *  For non Tunnel Start packets: Transmitted packet's VLAN tag
     *  format after tag addition/removal/modification
     *  For Ethernet-Over-xxx Tunnel Start packets:
     *  Passenger packet's VLAN tag format after tag
     *  addition/removal/modification.
     *  0 = Packet is untagged.
     *  1 = Packet is tagged.
     */
    GT_U8 isTagged;

    /** @brief VLAN ID assigned to the packet.
     *  For xCat3, Lion2
     *  VLAN Id is 12 bit.
     *  For Bobcat2; Caelum; Bobcat3; Aldrin devices support of 13-bit VLAN Id.
     *  Standard keys contain bits 11:0 only.
     *  Some keys contains all bits.
     */
    GT_U32 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** @brief The QoS Profile assigned to the packet until this lookup
     *  stage, as assigned by the device,
     *  according to the QoS marking algorithm
     */
    GT_U32 qosProfile;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     *  Supported by DxCh2 and above.
     */
    GT_U8 isL2Valid;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  Comments:
     */
    GT_U8 isUdbValid;

} CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
 *
 * @brief This structure describes the common segment of all extended key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv6
     *  (if packet was identified as IP packet):
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** The DSCP field of the IPv4/6 header. */
    GT_U32 dscp;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** @brief Indicates a valid IP header.
     *  0 = Packet IP header is invalid.
     *  1 = Packet IP header is valid.
     *  Comments:
     */
    GT_U8 ipHeaderOk;

} CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
 *
 * @brief This structure describes the common segment
 * of all standard IP key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv4
     *  (if packet was identified as IP packet):
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv4;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 ipProtocol;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 dscp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 ipHeaderOk;

    /** @brief Identifies an IPv4 fragment.
     *  0 = Not an IPv4 packet or not an IPv4 fragment.
     *  1 = Packet is an IPv4 fragment (could be
     *  the first fragment or any subsequent fragment)
     *  Comments:
     */
    GT_U8 ipv4Fragmented;

} CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
 *
 * @brief This structure describes the standard not-IP key.
*/
typedef struct{

    /** the common part for all formats (see above) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** @brief An indication that the packet is IPv4
     *  (if packet was identified as IP packet):
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv4;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  For xCat2 and above
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 without LLC.
     *  2 = Reserved.
     *  3 = LLC with SNAP.
     *  For xCat3.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    GT_U8 udb15_17[3];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 15
     *  vrfId[11:8] muxed with UDB 16
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash is muxed with UDB 17
     */
    GT_U32 trunkHash;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  muxed with UDB 16
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  bits [11:6] muxed with UDB 16
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  muxed with UDB 16
     */
    GT_U32 cfi1;

    GT_U8 udb23_26[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
 *
 * @brief This structure describes the standard IPV4/V6 L2_QOS key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    GT_U8 udb18_19[2];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 18
     *  vrfId[11:8] muxed with UDB 19
     */
    GT_U32 vrfId;

    GT_U8 udb27_30[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
 *
 * @brief This structure describes the standard IPV4_L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    GT_U8 udb20_22[3];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 20
     *  vrfId[11:8] muxed with UDB 21
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 22
     */
    GT_U32 trunkHash;

    GT_U8 udb31_34[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
 *
 * @brief This structure describes the standard IPV6 DIP key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6ExtHdrExist;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6HopByHop;

    /** 16 bytes IPV6 destination address. */
    GT_IPV6ADDR dip;

    GT_U8 udb47_49[3];

    /** @brief User Defined Byte 0
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Comments:
     *  Not supported by DxCh2 and DxCh3
     *  Used for Unicast IPV6 routing, lookup1 only
     */
    GT_U8 udb0;

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
 *
 * @brief This structure describes the extended not-IPV6 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 header destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U16 etherType;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC */
    GT_U8 ipv4Fragmented;

    GT_U8 udb0_5[6];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[11:8] muxed with UDB 1
     *  vrfId[7:0] muxed with UDB 5
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 3
     */
    GT_U32 trunkHash;

    GT_U8 udb39_46[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
 *
 * @brief This structure describes the extended IPV6+L2 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPV6 destination address highest 8 bits. */
    GT_U8 dipBits127to120;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6ExtHdrExist;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6HopByHop;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    GT_U8 udb6_11[6];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 11
     *  vrfId[11:8] muxed with UDB 6
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 7
     */
    GT_U32 trunkHash;

    GT_U8 udb47_49[3];

    GT_U8 udb0_4[5];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
 *
 * @brief This structure describes the extended IPV6+L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6ExtHdrExist;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb12_14[3];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 14
     *  vrfId[11:8] muxed with UDB 12
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 13
     */
    GT_U32 trunkHash;

    GT_U8 udb15_22[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
 *
 * @brief PCL Rule Key fields common to ingress "UDB" key formats.
 * xCat3 and above devices only
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: AC5; Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     *  For AC5 portListBmp[27:16] is muxed with
     *      STD_UDB - UDB2, UDB5 and qosProfile
     *      EXT_UDB - UDB2, UDB5 qosProfile, vrfId[11:8]
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** VLAN ID assigned to the packet. */
    GT_U32 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** IP DSCP or MPLS EXP */
    GT_U32 dscpOrExp;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    /** @brief The packet's VLAN Tag format . Valid when <IsL2Valid> = 1.
     *  0 = Untagged
     *  1 = Reserved.
     *  2 = PriorityTagged
     *  3 = IsTagged
     */
    GT_U32 pktTagging;

    /** @brief This flag indicates that the Layer 3 offset was
     *  successfully found. 0=Valid; 1=Invalid
     */
    GT_U8 l3OffsetInvalid;

    /** @brief 0=Other/Ivalid;
     *  1=TCP;
     *  2=ICMP;
     *  3=UDP;
     */
    GT_U32 l4ProtocolType;

    /** @brief 1
     *  2- IPv4 UDP
     *  4- MPLS
     *  8- IPv4 Fragment
     *  16 IPv4 Other
     *  32- Ethernet Other
     *  64- IPv6
     *  128- UDE
     */
    GT_U32 pktType;

    /** 0 */
    GT_U8 ipHeaderOk;

    /** @brief 0=Unicast; Known and Unknown Unicast
     *  1=Multicast; Known and Unknown Multicast
     *  2=NonARP BC; Not ARP Broadcast packet
     *  3=ARP BC; ARP Broadcast packet
     */
    GT_U32 macDaType;

    /** @brief This flag indicates that the Layer 4 offset was
     *  successfully found. 0=Valid; 1=Invalid
     */
    GT_U8 l4OffsetInvalid;

    /** @brief 0=LLC not Snap;
     *  1=Ethernet V2;
     *  2=reserved
     *  3=LLC Snap
     */
    GT_U32 l2Encapsulation;

    /** @brief Indicates that an IPv6 extension exists
     *  0=NotExists; Non-IPv6 packet or IPv6 extension header
     *  does not exists.
     *  1=Exists; Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6Eh;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is hop-by-hop 0=NonHopByHop; Non-IPv6 packet or
     *  IPv6 extension header type is not Hop-by-Hop Option Header.
     *  1=HopByHop; Packet is IPv6 and extension header type is
     *  Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
 *
 * @brief PCL Rule ingress standard "UDB" Key fields.
 * xCat3 and above devices only
*/
typedef struct{

    /** fields common for ingress "UDB" styled keys */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC commonIngrUdb;

    /** @brief An indication that the packet is IPv4
     *  (if packet was identified as IP packet):
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv4;

    GT_U8 udb0_15[16];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 0
     *  vrfId[11:8] muxed with UDB 1
     */
    GT_U32 vrfId;

    /** @brief QoS Profile(APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  qosProfile muxed with UDB2
     */
    GT_U32 qosProfile;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 3
     */
    GT_U32 trunkHash;

    GT_U8 udb35_38[4];

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
 *
 * @brief PCL Rule ingress extended "UDB" Key fields.
 * xCat3 and above devices only
*/
typedef struct{

    /** fields common for ingress "UDB" styled keys */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC commonIngrUdb;

    /** @brief An indication that the packet is IPv6
     *  (if packet was identified as IP packet):
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    GT_U8 sipBits31_0[4];

    GT_U8 sipBits79_32orMacSa[6];

    GT_U8 sipBits127_80orMacDa[6];

    GT_U8 dipBits127_112[2];

    GT_U8 dipBits31_0[4];

    GT_U8 udb0_15[16];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 1
     *  vrfId[11:8] muxed with UDB 2
     */
    GT_U32 vrfId;

    /** @brief QoS Profile(APPLICABLE DEVICES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  qosProfile muxed with UDB5
     */
    GT_U32 qosProfile;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 3
     */
    GT_U32 trunkHash;

    GT_U8 udb23_30[8];

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC;

/* EGRESS KEY FORMATS */

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_TO_CPU_STC
 *
 * @brief This structure describes the common TO CPU data of egress key formats.
*/
typedef struct
{
    /** @brief CPU Code of packet.
     */
    GT_U32            cpuCode;

    /** @brief 0 - packet TO CPU from ingress pipe,
     *         1 - packet TO CPU from egress pipe.
     */
    GT_U8             srcTrg;
} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_TO_CPU_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_FROM_CPU_STC
 *
 * @brief This structure describes the common FROM CPU data of egress key formats.
*/
typedef struct
{
    /** @brief Traffic Class.
     */
    GT_U32            tc;
    /** @brief Drop precedence.
     */
    GT_U32            dp;
    /** @brief Egress filtering enable.
     */
    GT_U8             egrFilterEnable;
} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_FROM_CPU_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_TO_ANALYZER_STC
 *
 * @brief This structure describes the common TO_ANALYZER data of egress key formats.
*/
typedef struct
{
    /** @brief 0 - egress mirrored, 1 - igress mirrored.
     */
    GT_U8             rxSniff;
} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_TO_ANALYZER_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_FORWARD_DATA_STC
 *
 * @brief This structure describes the common Forward data of egress key formats.
*/
typedef struct
{
    /** @brief QoS profile index.
     */
    GT_U32            qosProfile;

    /** @brief Source trunk Id.
     */
    GT_TRUNK_ID       srcTrunkId;

    /** @brief 0 - source is port, 1 - source is trunk.
     */
    GT_U8             srcIsTrunk;

    /** @brief 0 - MAC DA found in FDB, 1 - MAC DA not found in FDB.
     */
    GT_U8             isUnknown;

    /** @brief 0 - packet was not routed, 1 - packet was routed.
     */
    GT_U8             isRouted;
} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_FORWARD_DATA_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
 *
 * @brief This structure describes the common segment of all egress key formats.
*/
typedef struct
{
    /** @brief Always set to 1 in the key.
     *  (APPLICABLE DEVICES: xCat3; AC5)
     */
    GT_U8                 valid;

    /** @brief 10 bit PCL ID copied to the key from PCL Configuration Table entry.
     */
    GT_U32                pclId;

    /** @brief Source physical port.
     */
    GT_PHYSICAL_PORT_NUM  sourcePort;

    /** @brief Bitmap of target ports - relevant for port list mode only.
     *  muxed with PCL ID and Source port
     */
    CPSS_PORTS_BMP_STC    portListBmp;

    /** @brief 0 - packet not VLAN tagged, 1 - packet VLAN tagged.
     */
    GT_U8                 isTagged;

    /** @brief packet VID.
     */
    GT_U32                vid;

    /** @brief packet User priority.
     */
    GT_U32                up;

    /** @brief 0 - not IP packet, 1 - IP packet.
     */
    GT_U8                 isIp;

    /** @brief 0 -L 2 invalid (Tunnel passenger without L2), 1 - L2 valid.
     */
    GT_U8                 isL2Valid;

    /** @brief PacketCmd assigned to the packet:
     *   0x0 = TO_CPU;
     *   0x1 = FROM_CPU;
     *   0x2 = TO_ANALYZER;
     *   0x3 = FORWARD.
     */
    GT_U32                egrPacketType;

    /** @brief Data relevant for TO CPU packets.
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_TO_CPU_STC       toCpu;

    /** @brief Data relevant for FROM CPU packets.
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_FROM_CPU_STC     fromCpu;

    /** @brief Data relevant for TO ANALYZER packets.
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_TO_ANALYZER_STC  toAnalyzer;

    /** @brief Data relevant for FORWARDED packets.
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_FORWARD_DATA_STC fwdData;

    /** @brief Source device HW Id.
     */
    GT_HW_DEV_NUM         srcHwDev;

    /** @brief Source Id.
     */
    GT_U32                sourceId;

    /** @brief 0 - The packet is a Unicast packet forwarded to a specific target ePort or trunk.
     *         1 - The packet is a multi-destination packet forwarded to a Multicast group..
     */
    GT_U8                 isVidx;

    /** @brief There are at least two Ethernet Tags in the packet.
     */
    GT_U8                 tag1Exist;


    /** @brief 0x0 - At least one UDB is invalid.
     *         0x1 - All UDBs are valid.
     */
    GT_U8                 isUdbValid;
} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
 *
 * @brief This structure describes the common segment of all extended
 * egress key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv6.
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** The DSCP field of the IPv4/6 header. */
    GT_U32 dscp;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** @brief For non Tunnel Start packets:
     *  The transmitted packet TCP/UDP comparator result
     *  For Tunnel Start packets:
     *  The transmitted passenger packet TCP/UDP comparator result
     */
    GT_U32 egrTcpUdpPortComparator;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Comments:
     */
    GT_U8 isUdbValid;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC
 *
 * @brief This structure describes the common segment
 * of all egress standard IP key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv4.
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     */
    GT_U8 isIpv4;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 ipProtocol;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 dscp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** @brief Identifies an IPv4 fragment.
     *  0 = Not an IPv4 packet or not an IPv4 fragment.
     *  1 = Packet is an IPv4 fragment (could be
     *  the first fragment or any subsequent fragment)
     */
    GT_U8 ipv4Fragmented;

    /** @brief For non Tunnel Start packets:
     *  The transmitted packet TCP/UDP comparator result
     *  For Tunnel Start packets:
     *  The transmitted passenger packet TCP/UDP comparator result
     *  Comments:
     */
    GT_U32 egrTcpUdpPortComparator;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC;


/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
 *
 * @brief This structure describes the standard egress not-IP key.
*/
typedef struct{

    /** the common part for all formats (see above) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** @brief An indication that the packet is IPv4.
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     */
    GT_U8 isIpv4;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    GT_U8 udb0_3[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
 *
 * @brief This structure describes the standard egress IPV4/V6 L2_QOS key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    GT_U8 dipBits0to31[4];

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    GT_U8 udb4_7[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
 *
 * @brief This structure describes the standard egress IPV4_L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    GT_U8 udb8_11[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
 *
 * @brief This structure describes the egress extended not-IPV6 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC commonExt;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 header destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U16 etherType;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC */
    GT_U8 ipv4Fragmented;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  1 - MPLS, 0 - not MPLS.
     */
    GT_U8 isMpls;

    /** @brief (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  number of MPLS Labels (APPLICABLE RANGES: 0..3).
     */
    GT_U32 numOfMplsLabels;

    /** @brief (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  inner protocol type (2-bit value)
     */
    GT_U32 protocolTypeAfterMpls;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X) MPLS Label0. */
    GT_U32 mplsLabel0;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X) MPLS Exp0. */
    GT_U32 mplsExp0;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X) MPLS Label1. */
    GT_U32 mplsLabel1;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X) MPLS Exp1. */
    GT_U32 mplsExp1;

    GT_U8 udb12_19[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
 *
 * @brief This structure describes the egress extended IPV6+L2 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPV6 destination address highest 8 bits. */
    GT_U8 dipBits127to120;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb20_27[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC
 *
 * @brief This structure describes the egress extended IPV6+L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb28_35[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC
 *
 * @brief This structure describes the common fields of ingress keys.
 * The fields placed at the same bits of the keys.
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: AC5; Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     *  For AC5 portListBmp[27:16] is muxed with
     *      STD_IPV4_ROUTED_ACL_QOS   - ingressIpCommon.l4Byte2 and ingressIpCommon.l4Byte3
     *      EXT_IPV4_PORT_VLAN_QOS    - ingressIpCommon.l4Byte2 and ingressIpCommon.l4Byte3
     *      ULTRA_IPV6_PORT_VLAN_QOS  - ingressIpCommon.l4Byte2 and ingressIpCommon.l4Byte3
     *      ULTRA_IPV6_ROUTED_ACL_QOS - ingressIpCommon.l4Byte2 and ingressIpCommon.l4Byte3
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** @brief Flag indicating the packet Tag state
     *  For Ingress PCL Keys:
     *  For non-tunnel terminated packets:
     *  - If packet was received on DSA-tagged, this field is set
     *  to the DSA tag <SrcTagged> field.
     *  - If packet was received non-DSA-tagged, this field is set
     *  to 1 if the packet was received VLAN or Priority-tagged.
     *  For Ethernet-Over-xxx tunnel-terminated packets: Passenger
     *  packet VLAN tag format.
     */
    GT_U8 isTagged;

    /** VLAN ID assigned to the packet. */
    GT_U32 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** 8 */
    GT_U32 tos;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** @brief see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
     *  Comments:
     */
    GT_U8 l4Byte13;

} CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
 *
 * @brief This structure describes the ingress Standard Routed Qos IPV4 Key.
 * CH3 : ingress standard IPV4 key format, used in first lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief packet type
     *  0 = Non of the following.
     *  1 = ARP.
     *  2 = IPv6 Over MPLS.
     *  3 = IPv4 Over MPLS.
     *  4 = MPLS.
     *  6 = IPv6.
     *  7 = IPv4.
     */
    GT_U32 pktType;

    /** @brief IP Fragmented
     *  0 = no fragment
     *  1 = first fragment
     *  2 = mid fragment
     *  3 = last fragment
     */
    GT_U8 ipFragmented;

    /** @brief IP Header information
     *  0 = Normal
     *  1 = IP options present
     *  2 = IP header validation fail
     *  3 = Is small offset
     */
    GT_U32 ipHeaderInfo;

    /** IP Packet length 14 */
    GT_U32 ipPacketLength;

    /** time to live */
    GT_U32 ttl;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[7:0] muxed with UDB 5
     */
    GT_U32 vrfId;

    /** @brief The transmitted packet TCP/UDP
     *  comparator result.
     *  Supported only for DxCh3 device and ignored for other.
     */
    GT_U32 tcpUdpPortComparators;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    /** @brief UDB5
     *  Supported only for DxChXCat and above devices
     *  and ignored for DxCh3 devices.
     */
    GT_U8 udb5;

    GT_U8 udb41_44[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
 *
 * @brief This structure describes the ingress Extended Port/VLAN Qos IPV4 Key.
 * CH3 : ingress extended IPV4 key format, used in second lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief IP Fragmented
     *  0 = no fragment
     *  1 = first fragment
     *  2 = mid fragment
     *  3 = last fragment
     */
    GT_U8 ipFragmented;

    /** @brief IP Header information
     *  0 = Normal
     *  1 = IP options present
     *  2 = IP header validation fail
     *  3 = Is small offset
     */
    GT_U32 ipHeaderInfo;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[11:8] muxed with UDB 14
     *  vrfId[7:0] muxed with UDB 13
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 15
     */
    GT_U32 trunkHash;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbStdIpL2Qos[2];

    GT_U8 UdbStdIpV4L4[3];

    GT_U8 UdbExtIpv6L2[4];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb5_16[12];

    GT_U8 udb31_38[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
 *
 * @brief This structure describes the ingress Ultra Port/VLAN Qos IPV6 Key.
 * CH3: ingress ultra IPV6 L2 key format, used in first lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** IS ND */
    GT_U8 isNd;

    /** Ethernet Broadcast packet. */
    GT_U8 isBc;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** @brief IPv6 destination IP address field.
     *  For ARP packets this field holds the target IPv6 address.
     */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** @brief Indicates a valid IP header.
     *  0 = Packet IP header is invalid.
     *  1 = Packet IP header is valid.
     */
    GT_U8 ipHeaderOk;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[11:8] muxed with UDB 0
     *  vrfId[7:0] muxed with UDB 12
     */
    GT_U32 vrfId;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbExtNotIpv6[4];

    GT_U8 UdbExtIpv6L2[5];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb0_11[12];

    /** UDB 12 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    GT_U8 udb12;

    GT_U8 udb39_40[2];

    /** @brief (The source port or trunk assigned to the packet.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman)
     */
    GT_U32 srcPortOrTrunk;

    /** @brief Indicates packets received from a Trunk on a
     *  remote device or on the local device
     *  0 = Source is not a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is not a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD
     *  and DSA<SrcIsTrunk> =0
     *  1 = Source is a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD and
     *  DSA<SrcIsTrunk =1
     *  Relevant only when packet is of type is FORWARD.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 srcIsTrunk;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 1
     *  Comments:
     */
    GT_U32 trunkHash;

} CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
 *
 * @brief This structure describes the ingress Ultra Routed Qos IPV6 Key.
 * CH3 : ingress ultra IPV6 MPLS key format, used in second lookup.
*/
typedef struct{

    /** The common fields of all ingress keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief packet type
     *  0 = Non of the following.
     *  1 = ARP.
     *  2 = IPv6 Over MPLS.
     *  3 = IPv4 Over MPLS.
     *  4 = MPLS.
     *  6 = IPv6.
     *  7 = IPv4.
     */
    GT_U32 pktType;

    /** IS ND */
    GT_U8 isNd;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** MPLS Outer Label */
    GT_U32 mplsOuterLabel;

    /** MPLS Outer Label EXP */
    GT_U32 mplsOuterLabExp;

    /** MPLS Outer Label S */
    GT_U8 mplsOuterLabSBit;

    /** IP Packet Length */
    GT_U32 ipPacketLength;

    /** IPv6 Header <Flow Label> */
    GT_U32 ipv6HdrFlowLabel;

    /** time to live */
    GT_U32 ttl;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** @brief IPv6 destination IP address field.
     *  For ARP packets this field holds the target IPv6 address.
     */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** Is IPv6 Link local */
    GT_U8 isIpv6LinkLocal;

    /** Is IPv6 MLD */
    GT_U8 isIpv6Mld;

    /** Indicates a valid IP header. */
    GT_U8 ipHeaderOk;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  vrfId[11:8] muxed with UDB 19
     *  vrfId[7:0] muxed with UDB 18
     */
    GT_U32 vrfId;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbStdNotIp[2];

    GT_U8 UdbStdIpL2Qos[3];

    GT_U8 UdbStdIpV4L4[3];

    GT_U8 UdbExtNotIpv6[4];

    GT_U8 UdbExtIpv6L2[3];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb0_11[12];

    GT_U8 udb17_22[6];

    GT_U8 udb45_46[2];

    /** @brief (The source port or trunk assigned to the packet.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman)
     */
    GT_U32 srcPortOrTrunk;

    /** @brief Indicates packets received from a Trunk on a
     *  remote device or on the local device
     *  0 = Source is not a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is not a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD
     *  and DSA<SrcIsTrunk> =0
     *  1 = Source is a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD and
     *  DSA<SrcIsTrunk =1
     *  Relevant only when packet is of type is FORWARD.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 srcIsTrunk;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  trunkHash muxed with UDB 1
     *  Comments:
     */
    GT_U32 trunkHash;

} CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
 *
 * @brief This structure describes the common part of egress IP keys.
 * The fields placed at the same bits of the keys.
*/
typedef struct{

    /** The PCL */
    GT_U32 pclId;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** The QoS Profile assigned to the packet until this lookup */
    GT_U32 qosProfile;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief original VLAN ID assigned to the packet.
     *  For xCat3, Lion2
     *  VLAN Id is 12 bit.
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices support of 16-bit VLAN Id.
     */
    GT_U32 originalVid;

    /** @brief Is Source information (srcDevOrTrunkId) is Trunk ID
     *  0 = srcDevOrTrunkId is source device number
     *  1 = srcDevOrTrunkId is trunk ID
     */
    GT_U8 isSrcTrunk;

    /** @brief Source device number or trunk ID. See isSrcTrunk.
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices used only for TrunkId,
     *  for srcHwDev value see separate field srcDev.
     */
    GT_U32 srcDevOrTrunkId;

    /** @brief Together with <SrcPort> and <srcHwDev> indicates the
     *  network port at which the packet was received.
     *  Relevant for all packet types exclude the
     *  FWD_DATA packed entered to the PP from Trunk
     *  Fields srcTrunkId and srcHwDev are muxed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_HW_DEV_NUM srcHwDev;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 tag1Exist;

    /** @brief The Source ID assigned to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 sourceId;

    /** 8 */
    GT_U32 tos;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** time to live */
    GT_U32 ttl;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** The transmitted packet TCP/UDP comparator result */
    GT_U32 tcpUdpPortComparators;

    /** traffic class 3 */
    GT_U32 tc;

    /** drop priority 2 */
    GT_U32 dp;

    /** @brief two bits field:
     *  0 - packet to CPU
     *  1 - packet from CPU
     *  2 - packet to ANALYZER
     *  3 - forward DATA packet
     */
    GT_U32 egrPacketType;

    /** @brief Source/Target or Tx Mirror
     *  For TO CPU packets this field is <SrcTrg> of DSA tag.
     *  For TO_TARGET_SNIFFER packets this field is inverted
     *  <RxSniff> of DSA tag.
     *  For FORWARD and FROM_CPU packets this field is 0
     */
    GT_U8 srcTrgOrTxMirror;

    /** 3 */
    GT_U32 assignedUp;

    /** target physical port */
    GT_PHYSICAL_PORT_NUM trgPhysicalPort;

    /** @brief Indicates if this packet is Ingress or Egress
     *  mirrored to the Analyzer.
     *  0 = This packet is Egress mirrored to the analyzer.
     *  1 = This packet is Ingress mirrored to the analyzer.
     *  Relevant only when packet is of type is TO_ANALYZER.
     */
    GT_U8 rxSniff;

    /** @brief Indicates that the packet has been routed either by the
     *  local device or some previous device in a cascaded system.
     *  0 = The packet has not been routed.
     *  1 = The packet has been routed.
     *  Relevant only when packet is of type is FORWARD.
     */
    GT_U8 isRouted;

    /** @brief An indication that the packet is IPv6.
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     */
    GT_U8 isIpv6;

    /** @brief The CPU Code forwarded to the CPU.
     *  Relevant only when packet is of type is TO_CPU.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cpuCode;

    /** @brief The <SrcTrg> field extracted from the TO_CPU DSA<SrcTrg>:
     *  0 = The packet was sent to the CPU by the ingress pipe and
     *  the DSA tag contain attributes related to the
     *  packet ingress.
     *  1 = The packet was sent to the CPU by the egress pipe
     *  and the DSA tag contains attributes related to the
     *  packet egress.
     *  Relevant only when packet is of type is TO_CPU.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 srcTrg;

    /** @brief The <EgressFilterEn> extracted from
     *  FROM_CPU DSA<EgressFilterEn>:
     *  0 = FROM_CPU packet is subject to egress filtering,
     *  e.g. data traffic from the CPU
     *  1 = FROM_CPU packet is not subject egress filtering,
     *  e.g. control traffic from the CPU
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Comments:
     */
    GT_U8 egrFilterEnable;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC;


/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
 *
 * @brief This structure describes the Egress Extended (48B) RACL/VACL IPv4 key format.
 * CH3 : egress extended (48B) IPV4 key format.
*/
typedef struct{

    /** The common fields of egress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC egressIpCommon;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief IP V4 Options
     *  For non-tunnel start packets:
     *  0 = The transmitted packet is non-Ipv4 or an IPv4 options
     *  do not exist.
     *  1 = The transmitted packet is IPv4 and an IPv4 options
     *  exist.
     *  For tunnel-start packets:
     *  - always 0
     */
    GT_U8 ipv4Options;

    /** @brief Indicates that a packet is forwarded to a Multicast group.
     *  0 = The packet is a Unicast packet forwarded to a
     *  specific target port or trunk.
     *  1 = The packet is a multi-destination packet forwarded
     *  to a Multicast group.
     *  Relevant only for DxChXcat and above devices.
     */
    GT_U8 isVidx;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 isUdbValid;

    GT_U8 udb36_49[14];

    /** @brief UDB 0 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Comments:
     */
    GT_U8 udb0;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
 *
 * @brief This structure describes the Egress Ultra (72B) RACL/VACL IPv6 key format.
 * CH3 : egress Ultra (72B) IPV6 key format.
*/
typedef struct{

    /** The common fields of egress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC egressIpCommon;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** Is ND */
    GT_U8 isNd;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** @brief Indicates that a packet is forwarded to a Multicast group.
     *  0 = The packet is a Unicast packet forwarded to a
     *  specific target port or trunk.
     *  1 = The packet is a multi-destination packet forwarded
     *  to a Multicast group.
     *  Relevant only for DxChXcat and above devices.
     */
    GT_U8 isVidx;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 cfi1;

    /** @brief Source port assign to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 srcPort;

    /** @brief Egress port of the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 trgPort;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U8 isUdbValid;

    GT_U8 udb1_4[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC
 *
 * @brief This structure describes the Ingress UDB only keys fields
 * replacing the user defined bytes and being enabled/disabled
 * by cpssDxChPclUserDefinedBytesSelectSet.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief pclId UDB0 muxed with pclId[7:0],
     *  UDB1[1:0] muxed with pclId[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     *  UDB1[1] muxed with pclId[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  (APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman)
     *
     *  Note: For AC5P/X,  the ingress PCL-ID is extended to 12 bits
     */
    GT_U32 pclId;

    /** @brief UDB Valid: 1 when all UDBs in the key valid, otherwise 0,
     *  muxed with UDB1[7]
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief eVlan, UDB2 muxed with vid[7:0],
     *  UDB3 muxed with vid[12:8]
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort, UDB4 muxed with srcPort[7:0],
     *  UDB5 muxed with srcPort[14:8]
     *  (APPLICABLE RANGES: 0..0x7FFF)
     */
    GT_U32 srcPort;

    /** @brief source Device Is Own, UDB5 muxed with src-Dev-Is-Own
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 srcDevIsOwn;

    /** @brief tag1 VID, UDB6 muxed with vid1[7:0],
     *  UDB7[3:0] muxed with vid1[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 vid1;

    /** @brief tag1 UP, UDB7[6:4] muxed with up1[2:0]
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 up1;

    /** @brief mac to me, UDB7[7] muxed with mac2me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief qosProfile, UDB8 muxed with qosProfile[7:0],
     *  UDB9[1:0] muxed with qosProfile[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 qosProfile;

    /** @brief flow Id, UDB10 muxed with flowId[7:0]
     *  UDB11[3:0] muxed with flowId[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  Comments:
     */
    GT_U32 flowId;

    /** @brief Port PCL-ID2
     *  UDB12 muxed with IPCL stage n Port PCLID2[23:16]
     *  UDB13 muxed with IPCL stage n Port PCLID2[15:8]
     *  UDB14 muxed with IPCL stage n Port PCLID2[7:0]
     *  (APPLICABLE DEVICES: AC5X; Harrier; Ironman; AC5P)
     *  (APPLICABLE RANGES: 0..0xFFFFFF)
     */
    GT_U32 pclId2;

    /** @brief Packet hash, UDB18 muxed with hash[7:0],
     *  UDB17 muxed with hash[15:8]
     *  UDB16 muxed with hash[23:16]
     *  UDB15 muxed with hash[31:24]
     *  (APPLICABLE DEVICES: AC5X; Harrier; Ironman; AC5P)
     *  (APPLICABLE RANGES: 0..0xFFFFFFFF)
     */
    GT_U32 hash;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC
 *
 * @brief This structure describes the Ingress UDB60 keys fixed fields.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief UDB Valid
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief PCL
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief eVlan
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief source Device Is Own
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 srcDevIsOwn;

    /** @brief tag1 VID
     *  (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 vid1;

    /** @brief tag1 UP
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 up1;

    /** @brief mac to me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief qosProfile
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 qosProfile;

    /** @brief flow Id
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  Comments:
     */
    GT_U32 flowId;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC;

/**
 * @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB80_FIXED_STC
 *
 * @brief This structure describes the Ingress UDB80 keys fixed fields.
 *        (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief  MAC SA
     *  The 48 bits MAC SA
     */
    GT_ETHERADDR macSa;

    /** @brief IPV6 flow lable
     *  (APPLICABLE RANGES: 0..0xFFFFF)
     */
    GT_U32 ipv6FlowLabel;

    /** @brief mac to me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief is source frame is VLAN tagged
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 tag0SrcTag;

    /** @brief is source frame is VLAN tagged
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 tag1SrcTag;

    /** @brief IPv6 hop by hop extension
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 ipv6EhHopByHop;

    /** @brief packet type of service
     *  (APPLICABLE RANGES: 0..FF)
     */
    GT_U32 typeOfService;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB80_FIXED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC
 *
 * @brief This structure describes the Ingress UDB only keys formats.
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
 * (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E
 * (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    GT_U8 udb[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    /** replaced fixed fields. */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC replacedFld;

    /** @brief fixed fields,
     *  relevant only for CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E keys.
     *  Comments:
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC udb60FixedFld;

    /** @brief fixed fields(10B),
     *  relevant only for CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E keys.
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB80_FIXED_STC udb80FixedFld;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC
 *
 * @brief This structure describes the Egress UDB only keys fields
 * replacing the user defined bytes and being enabled/disabled
 * by cpssDxChPclUserDefinedBytesSelectSet.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief PCL
     *  UDB1[1:0] muxed with PCL-ID[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid: 1 when all UDBs in the key valid, otherwise 0,
     *  muxed with UDB1[7]
     *  (APPLICABLE RANGES: 0..1)
     *  Comments:
     */
    GT_U8 isUdbValid;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC
 *
 * @brief This structure describes the Egress UDB60 keys fixed fields.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief PCL
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief eVlan
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief target ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 trgPort;

    /** @brief source Device
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 srcDev;

    /** @brief target Device
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 trgDev;

    /** @brief local device target physical port
     *  (APPLICABLE RANGES: 0..0xFF)
     *  Comments:
     */
    GT_PHYSICAL_PORT_NUM localDevTrgPhyPort;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC
 *
 * @brief This structure describes the Egress UDB only keys formats.
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    GT_U8 udb[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    /** replaced fixed fields. */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC replacedFld;

    /** @brief fixed fields,
     *  relevant only for CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E keys.
     *  Comments:
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC udb60FixedFld;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC;


/**
* @union CPSS_DXCH_PCL_RULE_FORMAT_UNT
 *
 * @brief This union describes the PCL key.
 *
*/

typedef union{
    /** Standard Not IP packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC ruleStdNotIp;

    /** @brief Standard IPV4 and IPV6 packets
     *  L2 and QOS styled key
     */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC ruleStdIpL2Qos;

    /** Standard IPV4 packet L4 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC ruleStdIpv4L4;

    /** Standard IPV6 packet DIP styled key (DxCh only) */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC ruleStdIpv6Dip;

    /** Ingress Standard UDB styled packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC ruleIngrStdUdb;

    /** Extended Not IP and IPV4 packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC ruleExtNotIpv6;

    /** Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC ruleExtIpv6L2;

    /** Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC ruleExtIpv6L4;

    /** Ingress Extended UDB styled packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC ruleIngrExtUdb;

    /** Egress Standard Not IP packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC ruleEgrStdNotIp;

    /** @brief Egress Standard IPV4 and IPV6 packets
     *  L2 and QOS styled key
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC ruleEgrStdIpL2Qos;

    /** Egress Standard IPV4 packet L4 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC ruleEgrStdIpv4L4;

    /** Egress Extended Not IP and IPV4 packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC ruleEgrExtNotIpv6;

    /** Egress Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC ruleEgrExtIpv6L2;

    /** Egress Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC ruleEgrExtIpv6L4;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC ruleStdIpv4RoutedAclQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC ruleExtIpv4PortVlanQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC ruleUltraIpv6PortVlanQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC ruleUltraIpv6RoutedAclQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC ruleEgrExtIpv4RaclVacl;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC ruleEgrUltraIpv6RaclVacl;

    /** @brief Ingress UDB only formats.
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC ruleIngrUdbOnly;

    /** @brief Egress UDB only formats.
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E
     *  Comments:
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC ruleEgrUdbOnly;

} CPSS_DXCH_PCL_RULE_FORMAT_UNT;


/**
* @enum CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT
 *
 * @brief packet types that can be enabled/disabled for Egress PCL.
 * By default on all ports and on all packet types are disabled
*/
typedef enum{

    /** @brief Enable/Disable Egress Policy
     *  for Control packets FROM CPU.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,

    /** @brief Enable/Disable Egress Policy
     *  for data packets FROM CPU.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_DATA_E,

    /** @brief Enable/Disable Egress Policy
     *  on TOCPU packets
     */
    CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E,

    /** @brief Enable/Disable Egress Policy
     *  for TO ANALYZER packets.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E,

    /** @brief Enable/Disable Egress Policy
     *  for data packets that are tunneled in this device.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_TS_E,

    /** @brief Enable/Disable Egress Policy
     *  for data packets that are not tunneled in this device.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E,

    /** @brief Enable/Disable Egress Policy
     *  for the packets with packet command as HARD/SOFT DROP
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PCL_EGRESS_PKT_DROP_E


} CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT
 *
 * @brief Global Egress PCL or Ingress Policy
 * Configuration Table Access mode
*/
typedef enum{

    /** access by local port number */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E,

    /** @brief access by remote device Id
     *  and Port number or by trunk Id,
     *  Used for Value Blade systems based on DX switch device.
     *  The remote port and device are taken from DSA tag and
     *  represent real source port and device for a packets.
     */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_NON_LOCAL_PORT_E

} CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT;


/**
* @enum CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT
 *
 * @brief size of Ingress Policy / Egress PCL Configuration
 * table segment accessed by "non-local device and port"
*/
typedef enum{

    /** support 32 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E,

    /** support 64 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_64_E

} CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT;

/**
* @enum CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT
 *
 * @brief base of Ingress Policy / Egress PCL Configuration
 * table segment accessed
 * by "non-local device and port"
*/
typedef enum{

    /** the base is 0 */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E,

    /** the base is 1024 (only 32-port) */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE1024_E,

    /** the base is 2048 */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE2048_E,

    /** the base is 3072 (only 32-port) */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE3072_E

} CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT;

/**
* @enum CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT
 *
 * @brief TCAM segment mode.
 * Defines whether the 4 TCAM segments assigned to the PCL are
 * regarded as one logical TCAM returning a single reply,
 * two separate TCAM segments returning two replies,
 * or four separate TCAM segments returning four replies.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
 * Note: Bobcat3; Aldrin2 does not support it ! feature was removed.
 * and behavior is as 'CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E'
 * APIs that get this parameter will ignore it.
 * (see PRV_CPSS_DXCH_BOBCAT3_RM_TCAM_SEGMENT_MODE_OBSOLETE_WA_E)
*/
typedef enum{

    /** @brief The PCL TCAM block sets 0, 1, 2, and 3 are treated as a single logical TCAM, and return a
     *  single action associated with the first match in the combined TCAM block sets 0, 1, 2, and 3.
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E,

    /** @brief segmentation into 2 logical TCAMs:
     * - The PCL TCAM block sets 0 and 1 are treated as a single logical TCAM and return an action
     *   associated with the first match in the combined TCAM block sets 0 and 1.
     * - The PCL TCAM block sets 2 and 3 are treated as a single logical TCAM and return an action
     *   associated with the first match in the combined TCAM block sets 2 and 3.
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E,

    /** @brief segmentation into 2 logical TCAMs:
     * - The PCL TCAM block set 0 is treated as a single logical TCAM and returns an action
     *   associated with the first match in the TCAM block set 0.
     * - The PCL TCAM block sets 1, 2, and 3 are treated as a single logical TCAM and return an
     *   action associated with the first match in the combined TCAM block sets 1, 2, and 3.
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E,

    /** @brief segmentation into 2 logical TCAMs:
     * - The PCL TCAM block sets 0, 1, and 2 are treated as a single logical TCAM and return an
     *   action associated with the first match in the combined TCAM block sets 0, 1, and 2.
     * - The PCL TCAM block set 3 is treated as a single logical TCAM and return an action
     *   associated with the first match in the TCAM block set 3.
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E,

    /** @brief segmentation into 4 logical TCAMs:
     * - The PCL TCAM block set 0 is treated as a single logical TCAM and returns an action
     *   associated with the first match in the TCAM block set 0.
     * - The PCL TCAM block set 1 is treated as a single logical TCAM and returns an action
     *   associated with the first match in the TCAM block set 1.
     * - The PCL TCAM block set 2 is treated as a single logical TCAM and returns an action
     *   associated with the first match in the TCAM block set 2.
     * - The PCL TCAM block set 3 is treated as a single logical TCAM and returns an action
     *   associated with the first match in the TCAM block set 3.
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E

} CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT;

/**
* @struct CPSS_DXCH_PCL_LOOKUP_CFG_STC
 *
 * @brief PCL Configuration Table Entry Per Lookup Attributes.
*/
typedef struct
{
    /** @brief Enable Lookup
     *   GT_TRUE - enable Lookup
     *   GT_FALSE - disable Lookup
     *   The enableLookup enables or disables only
     *   Lookup0_0 when the CPSS_PCL_LOOKUP_0_E lookup is used.
     *   The dualLookup enables or disables Lookup0_1
     *   in this case.
     */
    GT_BOOL                                   enableLookup;

    /** @brief PCL-ID bits in the TCAM search key. */
    GT_U32                                    pclId;

    /** @brief Relevant for CPSS_PCL_LOOKUP_0_E  lookup only.
     *   GT_TRUE -  lookup0_1 enabled.
     *              lookup0_1 uses pclIdL01.
     *   GT_FALSE - lookup0_1 disabled.
     */
    GT_BOOL                                   dualLookup;

    /** @brief independent PCL ID for lookup0_1
     *   relevant for CPSS_PCL_LOOKUP_0_E  lookup only.
     */
    GT_U32                                    pclIdL01;
    struct
    {
        /** @brief type of TCAM search key for NON-IP packets. */
        CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    nonIpKey;

        /** @brief type of TCAM search key for IPV4 packets. */
        CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ipv4Key;

        /** @brief type of TCAM search key for IPV6 packets. */
        CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ipv6Key;
    }                                         groupKeyTypes;

    /** @brief replace 32 UDBs in the key by 256-bit bitmap
     *   GT_FALSE - Disable; No key change;
     *   GT_TRUE  - Enable.
     *   For Ingress PCL
     *   Enables replacing the first 32 UDBs in a UDB only key
     *   (minimal key size of 40B) with a bitmap vector of 256 bits
     *   that has only one bit set,representing the value of UDB23:
     *   UDB Bitmap = 2^UDB23.
     *   Related keys:
     *   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
     *   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
     *   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
     *   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
     *   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E
     *   For Egress PCL
     *   Bitmap vector of 256 bits that has only one bit set,
     *   representing the value of UDB49. That is: UDB Bitmap = 2^UDB49
     *   If the selected key is a UDB only key with at least 50 UDBs,
     *   UDBs 14-45 are replaced with bitmap representing the value of UDB49
     *   (2^UDB49);
     *   Related keys:
     *   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
     *   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
     *   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E
     *   (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    GT_BOOL                                   udbKeyBitmapEnable;

    /** @brief TCAM Segment Mode (for parallel lookup)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     *  NOTE: In Bobcat3 and above devices tcamSegmentMode is obsolete and can't be set by
     *   application (see PRV_CPSS_DXCH_BOBCAT3_RM_TCAM_SEGMENT_MODE_OBSOLETE_WA_E)
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT       tcamSegmentMode;

    /** @brief PCL Sub Profile Id for exact match
     *   APPLICABLE DEVICE: Falcon.
     *   APPLICABLE RANGES: 0..7.
     *   APPLICABLE DEVICE: AC5P.
     *   APPLICABLE RANGES: 0..15.
     *   Used as index for table contains EM profile Id1, EM profile Id2, TCAM profile Id
     */
    GT_U32                                    exactMatchLookupSubProfileId;

} CPSS_DXCH_PCL_LOOKUP_CFG_STC;

/**
* @struct CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC
 *
 * @brief global access mode to Ingress or Egress
 * PCL configuration tables.
*/
typedef struct{

    /** Ingress Policy local/non */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT ipclAccMode;

    /** Ingress Policy support 32/64 port remote devices  */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT ipclMaxDevPorts;

    /** Ingress Policy Configuration table access base */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT ipclDevPortBase;

    /** Egress PCL local/non */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT epclAccMode;

    /** Egress PCL support 32/64 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT epclMaxDevPorts;

    /** @brief Egress PCL Configuration table access base */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT epclDevPortBase;

} CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC;

/**
* @struct CPSS_DXCH_PCL_LOOKUP_TCAM_INDEXES_STC
 *
 * @brief TCAM indexes mapped to PCL lookups.
 *     (APPLICABLE DEVICES AC5)
*/
typedef struct{

    /** Ingress PCL lookup0 TCAM index (APPLICABLE RANGES: 0..1) */
    GT_U32         ipcl0TcamIndex;

    /** Ingress PCL lookup1 TCAM index (APPLICABLE RANGES: 0..1) */
    GT_U32         ipcl1TcamIndex;

    /** Ingress PCL lookup2 TCAM index (APPLICABLE RANGES: 0..1) */
    GT_U32         ipcl2TcamIndex;

    /** Egress PCL lookup TCAM index (APPLICABLE RANGES: 0..1) */
    GT_U32         epclTcamIndex;

} CPSS_DXCH_PCL_LOOKUP_TCAM_INDEXES_STC;

/**
* @enum CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT
 *
 * @brief enumerator for Egress PCL
 * VID and UP key fields content mode
*/
typedef enum{

    /** extract from packet Tag0 */
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E,

    /** extract from packet Tag1 */
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E

} CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT
 *
 * @brief enumerator for OAM packet MEG Level mode
*/
typedef enum{

    /** @brief MEG Level is UDB2[7:5] (OAM profile0)
     *  or UDB3[7:5] (OAM profile1)
     */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_UDB_E,

    /** MEG Level is MAC DA[2:0] */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_MAC_DA_E,

    /** MEG Level is MPLS EXP[2:0] (egress only) */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_EXP_E,

    /** MEG Level is QOS profile[2:0] (ingress only) */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_QOS_E

} CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT
 *
 * @brief enumerator for OAM Packet Detection Mode mode
*/
typedef enum{

    /** @brief OAM packets
     *  are detected based on EPCL rules.
     */
    CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_STANDARD_E,

    /** @brief OAM packets
     *  are detected based on EPCL rule AND a dedicated
     *  Source ID value. This mode allows the CPU to mark
     *  OAM packets by sending them with a dedicated Source ID.
     */
    CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_SOURCE_ID_E

} CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_ENT
 *
 *   @brief enumerator for User Defind Byte Values Bitmap Mode.
 *   Modifies the legacy feature of UDB49 values bitmap in the key:
 *   For Ingress and Egress PCL UDB only 50 and 60 byte keys when udbKeyBitmapEnable member
 *   of PCL configuration table entry structure is GT_TRUE key bytes 14-45 replaced
 *   by a bitmap of 256 bits. Only one bit is not zero in this bitmap - it is the bit
 *   with index equal to calculated value of UDB49.
 *   This enum defines new sizes of this bitmap: 64 bits (8 bytes) and 128 bits (16 bytes).
 *   Relevant for Ingress PCL only.
*/
typedef enum{

    /** @brief replacing UDBs 14-21 by bitmap vector of 64 bits representing (1 << UDB49). */
    CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_8_BYTES_E,

    /** @brief replacing UDBs 14-29 by bitmap vector of 128 bits representing (1 << UDB49). */
    CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_16_BYTES_E,

    /** @brief replacing UDBs 14-45 by bitmap vector of 256 bits representing (1 << UDB49). */
    CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_32_BYTES_E

} CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_ENT;

/**
 * @enum CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_ENT
 *
 *   @brief enumerator for IPCL/EPCL action field of 'flow-id' assignment mode
 *      to assign 'flow-id' field descriptor and/or 'stream-id' field descriptor
 *
 *      NOTE: the mode is per lookup number (stage) , per hit number (parallel lookup)
*/
typedef enum {
    /** @brief : the action field 'flow-id' assign both descriptor fields : 'flow-id' , 'stream-id' */
    CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E,
    /** @brief : the action field 'flow-id' assign single descriptor field : 'flow-id' */
    CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_ONLY_E,
    /** @brief : the action field 'flow-id' assign single descriptor field : 'stream-id' */
    CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_STREAM_ID_ONLY_E
}CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_USE_SRC_TRG_MODE_ENT
 *
 * @brief Defines the indexes used to access the policy configuration table.
 *        (APPLICABLE DEVICES Ironman)
*/
typedef enum{

    /** @brief Desc<Src EPG> and Desc<Dst EPG> */
    CPSS_DXCH_PCL_USE_SRC_TRG_MODE_SRC_DST_EPG_E,

    /** @brief Copy Reserved field and Source ID field */
    CPSS_DXCH_PCL_USE_SRC_TRG_MODE_COPY_RESERVED_SRC_ID_E

} CPSS_DXCH_PCL_USE_SRC_TRG_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_ENT
 *
 * @brief Defines the destination of the copy triggered by ActionEntry<Copy UDBs>.
*/
typedef enum{

    /** @brief Tag 1 VLAN Tag is the destination. */
    CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_TAG1_DEST_E,

    /** @brief SRC EPG is the destination */
    CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_SRC_EPG_E,

    /** @brief Tag 1 VLAN Tag & SRC EPG is the destination */
    /** @brief (APPLICABLE DEVICES: Ironman)               */
    CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_TAG1_SRC_EPG_E

} CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_ENT;


/**
 * @struct CPSS_DXCH_PCL_EPG_CONFIG_STC
 *
 *  @brief This structure defines the per stage EPG related configuration.
 */
typedef struct{

    /** Defines the indexes used to access the policy configuration table in the "USE_SRC_TRG" mode */
    /** APPLICABLE DEVICES: Ironman */
    CPSS_DXCH_PCL_USE_SRC_TRG_MODE_ENT          useSrcTrgMode;

    /** Defines the destination of the copy triggered by ActionEntry<Copy UDBs> */
    CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_ENT      assignFromUdbsMode;
} CPSS_DXCH_PCL_EPG_CONFIG_STC;

/**
* @enum CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT
 *
 * @brief Action mode for Action<Generic Action> field.
 *        Used only when Action<Redirect_Cmd> = Assign_Generic_Action
*/
typedef enum{

    /** @brief : Set action field <Generic Action> to represent 802.1Qci data
     *  ACTION[8:0] = Gate-id: The Gate-ID assigned to the flow.
     *                0x1FF - indicates Gate Control List is DISABLED
     *                0-510 - Gate-ID bound to this flow used by the Stream Gate Control List (SGCL) in the SMU.
     *  ACTION[10:9] = Max SDU Size Profile
     *  ACTION[11] = CNC Index Mode:
     *               0 - Hit Count Mode - CNC Index is Action <CNC_Index> (legacy mode)
     *               1 - Max SDU Pass/Fail Mode - CNC Index is {Action<CNC Index>, MaxSDUSizePass/Fail (1b)}
     */
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_802_1_QCI_E,

    /** @brief Generic action destination EPG mode */
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_DST_EPG_E,

    /** @brief  Generic action source EPG mode */
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_SRC_EPG_E

} CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT;

/**
* @struct CPSS_DXCH_PCL_PORT_EPG_CONFIG_STC
 *
 * @brief Source EPG related configuration in default ePort table
*/
typedef struct{

    /** @brief The source End Point Group
     * (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 srcEpg;

    /** @brief Enable to use the value extracted from Tag1 VID,
     *  as the source End Point Group of the packet
     */
    GT_BOOL copyTag1VidToSrcEpg;

} CPSS_DXCH_PCL_PORT_EPG_CONFIG_STC;

/**
* @struct CPSS_DXCH_PCL_PORT_DST_EPG_CONFIG_STC
 *
 * @brief Destination EPG related per physical port configuration
*/
typedef struct{

    /** @brief Destination EPG value
     *         This value will be ignored if <setDstEpgEnable> is GT_FALSE
     * (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 dstEpg;

    /** @brief Enable setting <Destination EPG> to the dstEPG,
     *         in case it was not set by previous stages.
     *         GT_TRUE  - Set destination EPG
     *         GT_FALSE - Do not set destination EPG
     */
    GT_BOOL setDstEpgEnable;

} CPSS_DXCH_PCL_PORT_DST_EPG_CONFIG_STC;

/**
* @internal cpssDxChPclInit function
* @endinternal
*
* @brief   The function initializes the device for following configuration
*         and using Policy engine
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclInit
(
    IN GT_U8                           devNum
);

/**
* @internal cpssDxChPclUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES xCat3; AC5)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
* @param[in] offset                   - the type of offset (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more than HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);

/**
* @internal cpssDxChPclUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES xCat3; AC5)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
*                                      (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_DIRECTION_ENT               direction,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_PCL_OFFSET_TYPE_ENT        *offsetTypePtr,
    OUT GT_U8                                *offsetPtr
);

/**
* @internal cpssDxChPclRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM. The rule index defines
*                                      order of action resolution in the cases of multiple
*                                      rules match with packet's search key. Action of the
*                                      matched rule with lowest index is taken in this case
*                                      With reference to Standard and Extended rules
*                                      indexes, the partitioning is as follows:
*                                      For DxCh and DxCh2 devices:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1023.
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 511.
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index  0 <= n <= 511
*                                      is placed in the space of two standard rules with
*                                      indexes n and n + 512.
*                                      For DxCh3 device:
*                                      The TCAM has up to 3.5K (3584) rows.
*                                      See datasheet of particular device for TCAM size.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 14335.
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 7167.
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index  0 <= n < 3584
*                                      is placed in the space of two standard rules with
*                                      indexes n and n + 3584.
*                                      - Extended rule with index  3584 <= n < 7168
*                                      is placed in the space of two standard rules with
*                                      indexes n + 3584, n + 7168.
*                                      For xCat3 and above devices:
*                                      See datasheet of particular device for TCAM size.
*                                      For example describe the TCAM that has 1/4K (256) rows.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1K (1024).
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 0.5K (512).
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index 2n (even index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n and 4n + 1.
*                                      - Extended rule with index 2n+1 (odd index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n+2 and 4n + 3.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*                                      Rule state is valid.
* @param[in] ruleOptionsBmp           = CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E -
*                                      write all fields of rule to TCAM but rule's
*                                      state is invalid (no match during lookups).
* @param[in] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The pattern and mask input structures should be set to zero
*       by an application before assignment of fields.
*       The function places all muxed fields to rule patern and mask
*       by using bitwise OR operation.
*       The function does not check validity of muxed (overlapped) fields.
*
*/
GT_STATUS cpssDxChPclRuleSet
(
    IN GT_U8                              devNum,
    IN GT_U32                             tcamIndex,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_RULE_OPTION_ENT      ruleOptionsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclRuleParsedGet function
* @endinternal
*
* @brief   The function gets the Policy Rule valid status, Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*
* @param[out] isRuleValidPtr           - GT_TRUE - rule is valid, GT_FALSE - rule is invalid
* @param[out] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[out] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - when size is trash or not related to ruleFormat.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclRuleParsedGet
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           tcamIndex,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT               ruleFormat,
    IN  GT_U32                                           ruleIndex,
    IN  CPSS_DXCH_PCL_RULE_OPTION_ENT                    ruleOptionsBmp,
    OUT GT_BOOL                                          *isRuleValidPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                         *actionPtr
);

/**
* @internal cpssDxChPclRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the Rule Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclRuleSet.
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclRuleActionUpdate
(
    IN GT_U8                              devNum,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclRuleActionGet function
* @endinternal
*
* @brief   The function gets the Rule Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclRuleSet.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*                                      Needed for parsing
*
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclRuleActionGet
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             tcamIndex,
    IN  CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN  GT_U32                             ruleIndex,
    IN CPSS_PCL_DIRECTION_ENT              direction,
    OUT CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclRuleInvalidate function
* @endinternal
*
* @brief   The function invalidates the Policy Rule.
*         For DxCh and DxCh2 devices start indirect rule write operation
*         with "garbage" pattern and mask content and "valid-flag" == 0
*         For DxCh3 devices calculates the TCAM position by ruleSize and ruleIndex
*         parameters and updates the first "valid" bit matching X/Y pair to (1,1)
*         i.e. don't match any value. If origin of valid or invalid rule found at the
*         specified TCAM position it will be invalid and available to back validation.
*         If the specified TCAM position contains the not first 24 byte segment of
*         extended rule, the rule also will be invalidated,
*         but unavailable to back validation.
*         If the garbage found in TCAM the X/Y bits will be updated to be as
*         in invalid rule.
*         The function does not check the TCAM contents.
*         GT_OK will be returned in each of described cases.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclRuleSet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Backward compatible styled API.
*       The cpssDxChPclRuleValidStatusSet recommended for using instead.
*
*/
GT_STATUS cpssDxChPclRuleInvalidate
(
    IN GT_U8                              devNum,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex
);

/**
* @internal cpssDxChPclRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] valid                    - new rule status: GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if in TCAM found rule of size different
*                                       from the specified
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclRuleValidStatusSet
(
    IN GT_U8                              devNum,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN GT_BOOL                            valid
);

/**
* @internal cpssDxChPclRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclRuleCopy should be
*         called. And after this cpssDxChPclRuleInvalidate or
*         cpssDxChPclRuleValidStatusSet should be used
*         to invalidate Rule in old position.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclRuleCopy
(
    IN GT_U8                              devNum,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
);

/**
* @internal cpssDxChPclPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - Ingress Policy enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressPolicyEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChPclPortIngressPolicyEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable ingress policy status per port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - Pointer to ingress policy status.
*                                      GT_TRUE - Ingress Policy is enabled.
*                                      GT_FALSE - Ingress Policy is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressPolicyEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChPclPortLookupCfgTabAccessModeSet function
* @endinternal
*
* @brief   Configures VLAN/PORT/ePort access mode to Ingress or Egress PCL
*         configuration table per lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      In eArch devices
*                                      - for each ingress lookup portNum is default ePort.
*                                      - for egress lookup portNum is physical port.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      Supported only for  xCat3 and above devices.
*                                      Ignored for other devices.
*                                      xCat3 and above devices supports two sub lookups only for
*                                      ingress lookup CPSS_PCL_LOOKUP_0_E , (APPLICABLE RANGES: 0..1).
*                                      All other lookups have not sub lookups,
*                                      acceptable value is 0.
* @param[in] mode                     - PCL Configuration Table access mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - on bad entryIndex
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - since dxCh1 supports Ingress direction only, this API set the same access
*       type for both lookups regardless <lookupNum> parameter value for dxCh1.
*
*/
GT_STATUS cpssDxChPclPortLookupCfgTabAccessModeSet
(
    IN GT_U8                                          devNum,
    IN GT_PORT_NUM                                    portNum,
    IN CPSS_PCL_DIRECTION_ENT                         direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                     lookupNum,
    IN GT_U32                                         subLookupNum,
    IN CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT mode
);

/**
* @internal cpssDxChPclPortLookupCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets VLAN/PORT/ePort access mode to Ingress or Egress PCL
*         configuration table per lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices
*                                      - for each ingress lookup portNum is default ePort.
*                                      - for egress lookup portNum is physical port.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      Supported only for  xCat3 and above devices.
*                                      Ignored for other devices.
*                                      xCat3 and above devices supports two sub lookups only for
*                                      ingress lookup CPSS_PCL_LOOKUP_0_E , (APPLICABLE RANGES: 0..1).
*                                      All other lookups have not sub lookups,
*                                      acceptable value is 0.
*
* @param[out] modePtr                  - (pointer to)PCL Configuration Table access mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclPortLookupCfgTabAccessModeGet
(
    IN  GT_U8                                          devNum,
    IN  GT_PORT_NUM                                    portNum,
    IN  CPSS_PCL_DIRECTION_ENT                         direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT                     lookupNum,
    IN  GT_U32                                         subLookupNum,
    OUT CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPclCfgTblAccessModeSet function
* @endinternal
*
* @brief   Configures global access mode to Ingress or Egress PCL configuration tables.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] accModePtr               - global configuration of access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChPclCfgTblAccessModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC   *accModePtr
);

/**
* @internal cpssDxChPclCfgTblAccessModeGet function
* @endinternal
*
* @brief   Get global access mode to Ingress or Egress PCL configuration tables.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] accModePtr               - global configuration of access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note see cpssDxChPclCfgTblAccessModeSet
*
*/
GT_STATUS cpssDxChPclCfgTblAccessModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC   *accModePtr
);

/**
* @internal cpssDxChPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for setting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
* @param[in] lookupCfgPtr             - lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclCfgTblSet
(
    IN GT_U8                           devNum,
    IN CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclCfgTblGet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for getting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
*
* @param[out] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclCfgTblGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclIngressPolicyEnable function
* @endinternal
*
* @brief   Enables Ingress Policy.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  ingress policy
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclIngressPolicyEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPclIngressPolicyEnableGet function
* @endinternal
*
* @brief   Get status (Enable or Disable) of Ingress Policy.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable Policy
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclIngressPolicyEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPclUdeEtherTypeSet function
* @endinternal
*
* @brief   This function sets UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] index                    - UDE Ethertype index
*                                      (APPLICABLE RANGES: Lion2 0..4; Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..6)
* @param[in] ethType                  - Ethertype value (APPLICABLE RANGES:  0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclUdeEtherTypeSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          ethType
);

/**
* @internal cpssDxChPclUdeEtherTypeGet function
* @endinternal
*
* @brief   This function gets the UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] index                    - UDE Ethertype index
*                                      (APPLICABLE RANGES: Lion2 0..4; Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..6)
*
* @param[out] ethTypePtr               - points to Ethertype value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclUdeEtherTypeGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *ethTypePtr
);

/**
* @internal cpssDxChPclTwoLookupsCpuCodeResolution function
* @endinternal
*
* @brief   Resolve the result CPU Code if both lookups has action commands
*         are either both TRAP or both MIRROR To CPU
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number from which the CPU Code is selected
*                                      when action commands are either both TRAP or
*                                      both MIRROR To CPU
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclTwoLookupsCpuCodeResolution
(
    IN GT_U8                       devNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum
);

/**
* @internal cpssDxChPclInvalidUdbCmdSet function
* @endinternal
*
* @brief   Set the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header
*         depth (i.e, the field resides deeper than 128 bytes into the packet.
*         In Falcon and above ingress packet depth increased to 160 bytes.).
*         This command is NOT applied when the policy key <User-Defined>
*         field cannot be extracted due to the offset being relative to a layer
*         start point that does not exist in the packet. (e.g. the offset is relative
*         to the IP header but the packet is a non-IP).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] udbErrorCmd              - command applied to a packet:
*                                      continue lookup, trap to CPU, hard drop or soft drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclInvalidUdbCmdSet
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_UDB_ERROR_CMD_ENT  udbErrorCmd
);

/**
* @internal cpssDxChPclInvalidUdbCmdGet function
* @endinternal
*
* @brief   Get the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header
*         depth (i.e, the field resides deeper than 128 bytes into the packet.
*         In Falcon and above ingress packet depth increased to 160 bytes.).
*         This command is NOT applied when the policy key <User-Defined>
*         field cannot be extracted due to the offset being relative to a layer
*         start point that does not exist in the packet. (e.g. the offset is relative
*         to the IP header but the packet is a non-IP).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] udbErrorCmdPtr           - command applied to a packet:
*                                      continue lookup, trap to CPU, hard drop or soft drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclInvalidUdbCmdGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_UDB_ERROR_CMD_ENT  *udbErrorCmdPtr
);

/**
* @internal cpssDxChPclIpLengthCheckModeSet function
* @endinternal
*
* @brief   Set the mode of checking IP packet length.
*         To determine if an IP packet is a valid IP packet, one of the checks is a
*         length check to find out if the total IP length field reported in the
*         IP header is less or equal to the packet's length.
*         This function determines the check mode.
*         The results of this check sets the policy key <IP Header OK>.
*         The result is also used by the router engine to determine whether
*         to forward or trap/drop the packet.
*         There are two check modes:
*         1. For IPv4: ip_total_length <= packet's byte count
*         For IPv6: ip_total_length + 40 <= packet's byte count
*         2. For IPv4: ip_total_length + L3 Offset + 4 (CRC) <= packet's byte count,
*         For IPv6: ip_total_length + 40 +L3 Offset + 4 (CRC) <= packet's
*         byte count
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - IP packet length checking mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclIpLengthCheckModeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT  mode
);

/**
* @internal cpssDxChPclIpLengthCheckModeGet function
* @endinternal
*
* @brief   Get the mode of checking IP packet length.
*         To determine if an IP packet is a valid IP packet, one of the checks is a
*         length check to find out if the total IP length field reported in the
*         IP header is less or equal to the packet's length.
*         This function determines the check mode.
*         The results of this check sets the policy key <IP Header OK>.
*         The result is also used by the router engine to determine whether
*         to forward or trap/drop the packet.
*         There are two check modes:
*         1. For IPv4: ip_total_length <= packet's byte count
*         For IPv6: ip_total_length + 40 <= packet's byte count
*         2. For IPv4: ip_total_length + L3 Offset + 4 (CRC) <= packet's byte count,
*         For IPv6: ip_total_length + 40 +L3 Offset + 4 (CRC) <= packet's
*         byte count
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - IP packet length checking mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclIpLengthCheckModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT  *modePtr
);

/**
* @internal cpssDxCh2PclEgressPolicyEnable function
* @endinternal
*
* @brief   Enables Egress Policy.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  Egress Policy
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported enable parameter value
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh2PclEgressPolicyEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxCh2PclEgressPolicyEnableGet function
* @endinternal
*
* @brief   Get status of Egress Policy.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - status of Egress Policy
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL poinet
*/
GT_STATUS cpssDxCh2PclEgressPolicyEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPclEgressForRemoteTunnelStartEnableSet function
* @endinternal
*
* @brief   Enable/Disable the Egress PCL processing for the packets,
*         which Tunnel Start already done on remote ingress device.
*         These packets are ingessed with DSA Tag contains source port 60.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  Remote Tunnel Start Packets Egress Pcl
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressForRemoteTunnelStartEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPclEgressForRemoteTunnelStartEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable of the Egress PCL processing for the packets,
*         which Tunnel Start already done on remote ingress device.
*         These packets are ingessed with DSA Tag contains source port 60.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable Remote Tunnel Start Packets Egress Pcl
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressForRemoteTunnelStartEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet function
* @endinternal
*
* @brief   Sets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cfgTabAccMode            - PCL Configuration Table access mode
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E - VLAN ID
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E - Normal
*                                      Normal means that access to PCL configuration table is done
*                                      according to settings done by cpssDxChPclCfgTblAccessModeSet
*                                      and cpssDxChPclPortLookupCfgTabAccessModeSet
*                                      as non tunnel start packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet
(
    IN  GT_U8                                            devNum,
    IN  CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT   cfgTabAccMode
);

/**
* @internal cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cfgTabAccModePtr         - (pointer to) PCL Configuration Table access mode
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E - VLAN ID
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E - Normal
*                                      Normal means that access to PCL configuration table is done
*                                      according to settings done by cpssDxChPclCfgTblAccessModeSet
*                                      and cpssDxChPclPortLookupCfgTabAccessModeSet
*                                      as non tunnel start packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet
(
    IN  GT_U8                                            devNum,
    OUT CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT   *cfgTabAccModePtr
);

/**
* @internal cpssDxChPclEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] vidUpMode                - VID and UP key fields content mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressKeyFieldsVidUpModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
);

/**
* @internal cpssDxChPclEgressKeyFieldsVidUpModeGet function
* @endinternal
*
* @brief   Gets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] vidUpModePtr             - (pointer to) VID and UP key fields
*                                      calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressKeyFieldsVidUpModeGet
(
    IN   GT_U8                                      devNum,
    OUT  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   *vidUpModePtr
);

/**
* @internal cpssDxCh2EgressPclPacketTypesSet function
* @endinternal
*
* @brief   Enables/disables Egress PCL (EPCL) for set of packet types on port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] pktType                  - packet type to enable/disable EPCL for it
* @param[in] enable                   -  EPCL for specific packet type
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note - after reset EPCL disabled for all packet types on all ports
*
*/
GT_STATUS cpssDxCh2EgressPclPacketTypesSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              port,
    IN CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT pktType,
    IN GT_BOOL                           enable
);

/**
* @internal cpssDxCh2EgressPclPacketTypesGet function
* @endinternal
*
* @brief   Get status of Egress PCL (EPCL) for set of packet types on port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] pktType                  - packet type to enable/disable EPCL for it
*
* @param[out] enablePtr                - enable EPCL for specific packet type
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxCh2EgressPclPacketTypesGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              port,
    IN  CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT pktType,
    OUT GT_BOOL                           *enablePtr
);

/**
* @internal cpssDxCh3PclTunnelTermForceVlanModeEnableSet function
* @endinternal
*
* @brief   The function enables/disables forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - force TT packets assigned to PCL
*                                      configuration table entry
*                                      GT_TRUE  - By VLAN
*                                      GT_FALSE - according to ingress port
*                                      per lookup settings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh3PclTunnelTermForceVlanModeEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxCh3PclTunnelTermForceVlanModeEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable of the forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) force TT packets assigned to PCL
*                                      configuration table entry
*                                      GT_TRUE  - By VLAN
*                                      GT_FALSE - according to ingress port
*                                      per lookup settings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh3PclTunnelTermForceVlanModeEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxCh2PclTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index
*                                       (APPLICABLE RANGES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman 0..7;
*                                                           AC5P 0..63)
* @param[in] l4PortType               - TCP/UDP port type, source or destination port
* @param[in] compareOperator          - compare operator FALSE, LTE, GTE, NEQ
* @param[in] value                    - 16 bit  to compare with
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh2PclTcpUdpPortComparatorSet
(
    IN GT_U8                             devNum,
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U8                             entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
);

/**
* @internal cpssDxCh2PclTcpUdpPortComparatorGet function
* @endinternal
*
* @brief   Get TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index
*                                       (APPLICABLE RANGES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman 0..7;
*                                                           AC5P 0..63)
*
* @param[out] l4PortTypePtr            - TCP/UDP port type, source or destination port
* @param[out] compareOperatorPtr       - compare operator FALSE, LTE, GTE, NEQ
* @param[out] valuePtr                 - 16 bit value to compare with
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxCh2PclTcpUdpPortComparatorGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PCL_DIRECTION_ENT            direction,
    IN  CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN  GT_U8                             entryIndex,
    OUT CPSS_L4_PROTOCOL_PORT_TYPE_ENT    *l4PortTypePtr,
    OUT CPSS_COMPARE_OPERATOR_ENT         *compareOperatorPtr,
    OUT GT_U16                            *valuePtr
);

/**
* @internal cpssDxChPclRuleStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*         Old function for DxCh1 and DxCh2 devices and for
*         standard rules on DxCh3 devices.
*         The general function is cpssDxChPclRuleAnyStateGet
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleIndex                - index of rule
*
* @param[out] validPtr                 -  rule's validity
*                                      GT_TRUE  - rule valid
*                                      GT_FALSE - rule invalid
* @param[out] ruleSizePtr              -  rule's size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        tcamIndex,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChPclRuleAnyStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of rule
* @param[in] ruleIndex                - index of rule
*
* @param[out] validPtr                 -  rule's validity
*                                      GT_TRUE  - rule valid
*                                      GT_FALSE - rule invalid
* @param[out] ruleSizePtr              -  rule's size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleAnyStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        tcamIndex,
    IN  CPSS_PCL_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChPclRuleGet function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in Hw format
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
*
* @param[out] maskArr[]                - rule mask          - 21 words.
* @param[out] patternArr[]             - rule pattern       - 21 words.
* @param[out] actionArr[]              - Policy rule action - 8 words.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleGet
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 tcamIndex,
    IN  CPSS_PCL_RULE_SIZE_ENT ruleSize,
    IN  GT_U32                 ruleIndex,
    OUT GT_U32                 maskArr[], /*maxArraySize=21*/
    OUT GT_U32                 patternArr[], /*maxArraySize=21*/
    OUT GT_U32                 actionArr[] /*maxArraySize=8*/
);

/**
* @internal cpssDxChPclCfgTblEntryGet function
* @endinternal
*
* @brief   Gets the PCL configuration table entry in Hw format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Ingress Policy or Egress Policy
*                                      (CPSS_PCL_DIRECTION_ENT member),
* @param[in] lookupNum                - Lookup number: lookup0 or lookup1
*                                      DxCh1-DxCh3 devices ignores the parameter
*                                      xCat3 and above supports the parameter
*                                      xCat3 and above devices has separate HW entries
*                                      for both ingress lookups.
*                                      Only lookup 0 is supported for egress.
* @param[in] entryIndex               - PCL configuration table entry Index
*
* @param[out] pclCfgTblEntryPtr        - pcl Cfg Tbl Entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_STATE             - in case of already bound pclHwId with same
*                                       combination of slot/direction
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh devices support ingress direction only
*
*/
GT_STATUS cpssDxChPclCfgTblEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN  GT_U32                          entryIndex,
    OUT GT_U32                          *pclCfgTblEntryPtr
);

/**
* @internal cpssDxChPclLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
* @param[in] enable                   - GT_TRUE  -  port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclLookupCfgPortListEnableSet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPclLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPclPortGroupRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM. The rule index defines
*                                      order of action resolution in the cases of multiple
*                                      rules match with packet's search key. Action of the
*                                      matched rule with lowest index is taken in this case
*                                      With reference to Standard and Extended rules
*                                      indexes, the partitioning is as follows:
*                                      For xCat3 and above devices:
*                                      See datasheet of particular device for TCAM size.
*                                      For example describe the TCAM that has 1/4K (256) rows.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1K (1024).
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 0.5K (512).
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index 2n (even index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n and 4n + 1.
*                                      - Extended rule with index 2n+1 (odd index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n+2 and 4n + 3.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*                                      Write rule to TCAM not using port-list format.
*                                      Rule state is valid.
* @param[in] ruleOptionsBmp           = CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E -
*                                      write all fields of rule to TCAM but rule's
*                                      state is invalid (no match during lookups).
* @param[in] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The pattern and mask input structures should be set to zero
*       by an application before assignment of fields.
*       The function places all muxed fields to rule patern and mask
*       by using bitwise OR operation.
*       The function does not check validity of muxed (overlapped) fields.
*
*/
GT_STATUS cpssDxChPclPortGroupRuleSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             tcamIndex,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_RULE_OPTION_ENT      ruleOptionsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleParsedGet function
* @endinternal
*
* @brief   The function gets the Policy Rule valid state, Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*
* @param[out] isRuleValidPtr           - GT_TRUE - rule is valid, GT_FALSE - rule is invalid
* @param[out] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[out] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - when size is trash or not related to ruleFormat.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortGroupRuleParsedGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           tcamIndex,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT               ruleFormat,
    IN GT_U32                                           ruleIndex,
    IN CPSS_DXCH_PCL_RULE_OPTION_ENT                    ruleOptionsBmp,
    OUT GT_BOOL                                         *isRuleValidPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                   *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                   *patternPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                        *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the Rule Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclPortGroupRuleSet.
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclPortGroupRuleActionUpdate
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleActionGet function
* @endinternal
*
* @brief   The function gets the Rule Action
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclPortGroupRuleSet.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*                                      Needed for parsing
*
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclPortGroupRuleActionGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN  GT_U32                             tcamIndex,
    IN  CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN  GT_U32                             ruleIndex,
    IN  CPSS_PCL_DIRECTION_ENT             direction,
    OUT CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleInvalidate function
* @endinternal
*
* @brief   The function invalidates the Policy Rule.
*         For DxCh and DxCh2 devices start indirect rule write operation
*         with "garbage" pattern and mask content and "valid-flag" == 0
*         For DxCh3 devices calculates the TCAM position by ruleSize and ruleIndex
*         parameters and updates the first "valid" bit matching X/Y pair to (1,1)
*         i.e. don't match any value. If origin of valid or invalid rule found at the
*         specified TCAM position it will be invalid and available to back validation.
*         If the specified TCAM position contains the not first 24 byte segment of
*         extended rule, the rule also will be invalidated,
*         but unavailable to back validation.
*         If the garbage found in TCAM the X/Y bits will be updated to be as
*         in invalid rule.
*         The function does not check the TCAM contents.
*         GT_OK will be returned in each of described cases.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclPortGroupRuleSet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Backward compatible styled API.
*       The cpssDxChPclRuleValidStatusSet recommended for using instead.
*
*/
GT_STATUS cpssDxChPclPortGroupRuleInvalidate
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex
);

/**
* @internal cpssDxChPclPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] valid                    port group(s). If a bit of non  port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] valid                    - new rule status: GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if in TCAM found rule of size different
*                                       from the specified
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortGroupRuleValidStatusSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN GT_BOOL                            valid
);

/**
* @internal cpssDxChPclPortGroupRuleAnyStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of rule. (APPLICABLE DEVICES xCat3; AC5; Lion2)
* @param[in] ruleIndex                - index of rule
*
* @param[out] validPtr                 -  rule's validity
*                                      GT_TRUE  - rule valid
*                                      GT_FALSE - rule invalid
* @param[out] ruleSizePtr              -  rule's size
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclPortGroupRuleAnyStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        tcamIndex,
    IN  CPSS_PCL_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChPclPortGroupRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclPortGroupRuleCopy should be
*         called. And after this cpssDxChPclPortGroupRuleInvalidate or
*         cpssDxChPclPortGroupRuleValidStatusSet should
*         be used to invalidate Rule in old position.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of Rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortGroupRuleCopy
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
);

/**
* @internal cpssDxChPclPortGroupRuleGet function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in Hw format
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
*
* @param[out] maskArr[]                - rule mask          - 21  words.
* @param[out] patternArr[]             - rule pattern       - 21  words.
* @param[out] actionArr[]              - Policy rule action - 8 words.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupRuleGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN  GT_U32                 tcamIndex,
    IN  CPSS_PCL_RULE_SIZE_ENT ruleSize,
    IN  GT_U32                 ruleIndex,
    OUT GT_U32                 maskArr[], /*maxArraySize=21*/
    OUT GT_U32                 patternArr[], /*maxArraySize=21*/
    OUT GT_U32                 actionArr[] /*maxArraySize=8*/
);

/**
* @internal cpssDxChPclPortGroupCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for setting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
* @param[in] lookupCfgPtr             - lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclPortGroupCfgTblSet
(
    IN GT_U8                           devNum,
    IN GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN CPSS_INTERFACE_INFO_STC         *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclPortGroupCfgTblGet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for getting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
*
* @param[out] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*/
GT_STATUS cpssDxChPclPortGroupCfgTblGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclPortGroupCfgTblEntryGet function
* @endinternal
*
* @brief   Gets the PCL configuration table entry in Hw format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Ingress Policy or Egress Policy
*                                      (CPSS_PCL_DIRECTION_ENT member),
* @param[in] lookupNum                - Lookup number: lookup0 or lookup1
*                                      DxCh1-DxCh3 devices ignores the parameter
*                                      xCat3 and above supports the parameter
*                                      xCat3 and above devices has separate HW entries
*                                      for both ingress lookups.
*                                      Only lookup 0 is supported for egress.
* @param[in] entryIndex               - PCL configuration table entry Index
*
* @param[out] pclCfgTblEntryPtr        - pcl Cfg Tbl Entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_STATE             - in case of already bound pclHwId with same
*                                       combination of slot/direction
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh devices support ingress direction only
*
*/
GT_STATUS cpssDxChPclPortGroupCfgTblEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN  GT_U32                          entryIndex,
    OUT GT_U32                          *pclCfgTblEntryPtr
);

/**
* @internal cpssDxChPclPortGroupTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
* @param[in] value                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index
*                                       (APPLICABLE RANGES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman 0..7;
*                                                           AC5P 0..63)
* @param[in] l4PortType               - TCP/UDP port type, source or destination port
* @param[in] compareOperator          - compare operator FALSE, LTE, GTE, NEQ
* @param[in] value                    - 16 bit  to compare with
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclPortGroupTcpUdpPortComparatorSet
(
    IN GT_U8                             devNum,
    IN GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U8                             entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
);

/**
* @internal cpssDxChPclPortGroupTcpUdpPortComparatorGet function
* @endinternal
*
* @brief   Get TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index
*                                       (APPLICABLE RANGES: xCat3, AC5, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman 0..7;
*                                                           AC5P 0..63)
*
* @param[out] l4PortTypePtr            - TCP/UDP port type, source or destination port
* @param[out] compareOperatorPtr       - compare operator FALSE, LTE, GTE, NEQ
* @param[out] valuePtr                 - 16 bit value to compare with
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclPortGroupTcpUdpPortComparatorGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT            direction,
    IN  CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN  GT_U8                             entryIndex,
    OUT CPSS_L4_PROTOCOL_PORT_TYPE_ENT    *l4PortTypePtr,
    OUT CPSS_COMPARE_OPERATOR_ENT         *compareOperatorPtr,
    OUT GT_U16                            *valuePtr
);

/**
* @internal cpssDxChPclPortGroupUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES xCat3; AC5)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
* @param[in] offset                   - the type of offset (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter value more than HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclPortGroupUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);

/**
* @internal cpssDxChPclPortGroupUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat3; AC5; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES xCat3; AC5)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
*                                      (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclPortGroupUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_DIRECTION_ENT               direction,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_PCL_OFFSET_TYPE_ENT        *offsetTypePtr,
    OUT GT_U8                                *offsetPtr
);

/**
* @internal cpssDxChPclUserDefinedBytesSelectSet function
* @endinternal
*
* @brief   Set the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Valid only UDB only key formats.
*                                      The parameter used also to determinate Ingress or Egress.
*                                      Relevant values are:
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E
*                                      (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E
*                                      (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
* @param[in] packetType               - packet Type
* @param[in] lookupNum                - Lookup number. Ignored for egress.
* @param[in] udbSelectPtr             - (pointer to) structure with UDB Selection configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclUserDefinedBytesSelectSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    IN  CPSS_DXCH_PCL_UDB_SELECT_STC         *udbSelectPtr
);

/**
* @internal cpssDxChPclUserDefinedBytesSelectGet function
* @endinternal
*
* @brief   Get the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Valid only UDB only key formats.
*                                      The parameter used also to determinate Ingress or Egress.
*                                      Relevant values are:
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E
*                                      (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E
*                                      (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
* @param[in] packetType               - packet Type
* @param[in] lookupNum                - Lookup number. Ignored for egress.
*
* @param[out] udbSelectPtr             - (pointer to) structure with UDB Selection configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unsupported HW state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclUserDefinedBytesSelectGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    OUT CPSS_DXCH_PCL_UDB_SELECT_STC         *udbSelectPtr
);

/**
* @internal cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet function
* @endinternal
*
* @brief   Set the User Defined Byte (UDB) Values bitmap expansion mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] udbValuesBmpMode         - UDB Values bitmap mode (see enum description)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_ENT   udbValuesBmpMode
);

/**
* @internal cpssDxChPclUserDefinedByteValuesBitmapExpansionModeGet function
* @endinternal
*
* @brief   Get the User Defined Byte (UDB) Values bitmap expansion mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum                   - device number
* @param[out] udbValuesBmpModePtr      - (pointer to)UDB Values bitmap mode (see enum description)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on wrong HW value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclUserDefinedByteValuesBitmapExpansionModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_ENT   *udbValuesBmpModePtr
);

/**
* @internal cpssDxChPclPortGroupEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] vidUpMode                - VID and UP key fields content mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupEgressKeyFieldsVidUpModeSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PORT_GROUPS_BMP                         portGroupsBmp,
    IN  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
);

/**
* @internal cpssDxChPclPortGroupEgressKeyFieldsVidUpModeGet function
* @endinternal
*
* @brief   Gets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*
* @param[out] vidUpModePtr             - (pointer to) VID and UP key fields
*                                      calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupEgressKeyFieldsVidUpModeGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PORT_GROUPS_BMP                         portGroupsBmp,
    OUT  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   *vidUpModePtr
);

/**
* @internal cpssDxChPclPortGroupLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
* @param[in] enable                   - GT_TRUE  -  port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupLookupCfgPortListEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPclPortGroupLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPclL3L4ParsingOverMplsEnableSet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclL3L4ParsingOverMplsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_BOOL                            enable
);

/**
* @internal cpssDxChPclL3L4ParsingOverMplsEnableGet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclL3L4ParsingOverMplsEnableGet
(
    IN  GT_U8                              devNum,
    OUT GT_BOOL                            *enablePtr
);

/**
* @internal cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet function
* @endinternal
*
* @brief   The function enables or disables overriding of the
*         UDB content for specific UDB in specific key format
*         where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
* @param[in] enable                   - GT_TRUE -  overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB configuration is used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    IN  GT_BOOL                              enable
);

/**
* @internal cpssDxChPclPortGroupOverrideUserDefinedBytesEnableGet function
* @endinternal
*
* @brief   The function get status (enabled or disabled) of the
*         overriding of the UDB content for specific UDB
*         in specific key format where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB content may be overriden
*                                      by content of another udbOverrideType.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupOverrideUserDefinedBytesEnableGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    OUT GT_BOOL                              *enablePtr
);

/**
* @internal cpssDxChPclOverrideUserDefinedBytesEnableSet function
* @endinternal
*
* @brief   The function enables or disables overriding of the
*         UDB content for specific UDB in specific key format
*         where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
* @param[in] enable                   - GT_TRUE -  overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB configuration is used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclOverrideUserDefinedBytesEnableSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    IN  GT_BOOL                              enable
);

/**
* @internal cpssDxChPclOverrideUserDefinedBytesEnableGet function
* @endinternal
*
* @brief   The function get status (enabled or disabled) of the
*         overriding of the UDB content for specific UDB
*         in specific key format where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB content may be overriden
*                                      by content of another udbOverrideType.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclOverrideUserDefinedBytesEnableGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    OUT GT_BOOL                              *enablePtr
);

/**
* @internal cpssDxChPclLookup0ForRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup0 for routed packets.
*         For not routed packets lookup0 always enable.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclLookup0ForRoutedPacketsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  GT_BOOL                            enable
);


/**
* @internal cpssDxChPclLookup0ForRoutedPacketsEnableGet function
* @endinternal
*
* @brief   Gets Enable/disable status of PCL lookup0 for routed packets.
*         For not routed packets lookup0 always enable.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssDxChPclLookup0ForRoutedPacketsEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    OUT GT_BOOL                            *enablePtr
);

/**
* @internal cpssDxChPclLookup1ForNotRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup1 for not routed packets.
*         For routed packets lookup1 always enable.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclLookup1ForNotRoutedPacketsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_BOOL                            enable
);

/**
* @internal cpssDxChPclLookup1ForNotRoutedPacketsEnableGet function
* @endinternal
*
* @brief   Gets Enable/disable status of PCL lookup1 for not routed packets.
*         For routed packets lookup1 always enable.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssDxChPclLookup1ForNotRoutedPacketsEnableGet
(
    IN  GT_U8                              devNum,
    OUT GT_BOOL                            *enablePtr
);

/**
* @internal cpssDxChPclIpMinOffsetSet function
* @endinternal
*
* @brief   Set the IP Minimum Offset parameter.
*         If packet is IPV4 fragment and has non zero offset
*         and it's offset less than IP Minimum Offset
*         the packet is counted as small offset.
*         The ipHeaderInfo field in PCL search key is set to be 3 i.e. "Ip Small Offset".
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ipMinOffset              - IP Minimum Offset  parameter (APPLICABLE RANGES: 0..0x1FFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range minIpOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclIpMinOffsetSet
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             ipMinOffset
);

/**
* @internal cpssDxChPclIpMinOffsetGet function
* @endinternal
*
* @brief   Get the IP Minimum Offset parameter.
*         If packet is IPV4 fragment and has non zero offset
*         and it's offset less than IP Minimum Offset
*         the packet is counted as small offset.
*         The ipHeaderInfo field in PCL search key is set to be 3 i.e. "Ip Small Offset".
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] ipMinOffsetPtr           - (pointer to)IP Minimum Offset  parameter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range minIpOffset
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclIpMinOffsetGet
(
    IN  GT_U8                              devNum,
    OUT GT_U32                             *ipMinOffsetPtr
);

/**
* @internal cpssDxChPclPortIngressLookup0Sublookup1TypeSet function
* @endinternal
*
* @brief   Configures Ingress PCL Lookup0 sublookup1 type.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] lookupType               - partial or full IPCL lookup0 sublookup1 type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressLookup0Sublookup1TypeSet
(
    IN GT_U8                                      devNum,
    IN CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT lookupType
);

/**
* @internal cpssDxChPclPortIngressLookup0Sublookup1TypeGet function
* @endinternal
*
* @brief   Configures Ingress PCL Lookup0 sublookup1 type.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] lookupTypePtr            - (pointer to)partial or full IPCL lookup0 sublookup1 type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressLookup0Sublookup1TypeGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT *lookupTypePtr
);

/**
* @internal cpssDxChPclPortListGroupingEnableSet function
* @endinternal
*
* @brief   Enable/disable port grouping mode per direction. When enabled, the 4 MSB
*         bits of the <Port List> field in the PCL keys are replaced by a 4-bit
*         <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] enable                   - enable/disable port grouping mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortListGroupingEnableSet
(
    IN GT_U8                    devNum,
    IN CPSS_PCL_DIRECTION_ENT   direction,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChPclPortListGroupingEnableGet function
* @endinternal
*
* @brief   Get the port grouping mode enabling status per direction. When enabled,
*         the 4 MSB bits of the <Port List> field in the PCL keys are replaced by a
*         4-bit <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
*
* @param[out] enablePtr                - (pointer to) the port grouping mode enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortListGroupingEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPclPortListPortMappingSet function
* @endinternal
*
* @brief   Set port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
* @param[in] enable                   - port participate in the port list vector.
* @param[in] group                    - PCL port group
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] offset                   -  in the port list bit vector.
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..27)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When working in PCL Port List mode with port grouping enabled, <offset>
*       value in the range 23..27 has no influence.
*
*/
GT_STATUS cpssDxChPclPortListPortMappingSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  group,
    IN  GT_U32                  offset
);

/**
* @internal cpssDxChPclPortListPortMappingGet function
* @endinternal
*
* @brief   Get port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) port participate in the port list vector.
* @param[out] groupPtr                 - (pointer to) PCL port group
* @param[out] offsetPtr                - (pointer to) offset in the port list bit vector.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortListPortMappingGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *groupPtr,
    OUT GT_U32                  *offsetPtr
);

/**
* @internal cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet function
* @endinternal
*
* @brief   Set Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] channelTypeProfile       - MPLS G-ACh Channel Type Profile
*                                      (APPLICABLE RANGES: 1..15)
* @param[in] opcode                   - OAM  (APPLICABLE RANGES: 0..255)
* @param[in] RBitAssignmentEnable     - replace Bit0 of opcode by packet R-flag
*                                      GT_TRUE  - Bit0 of result opcode is packet R-flag
*                                      GT_FALSE - Bit0 of result opcode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    channelTypeProfile,
    IN  GT_U32                    opcode,
    IN  GT_BOOL                   RBitAssignmentEnable
);

/**
* @internal cpssDxChPclOamChannelTypeProfileToOpcodeMappingGet function
* @endinternal
*
* @brief   Get Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] channelTypeProfile       - MPLS G-ACh Channel Type Profile
*                                      (APPLICABLE RANGES: 1..15)
*
* @param[out] opcodePtr                - (pointer to)OAM opcode
* @param[out] RBitAssignmentEnablePtr  - (pointer to)replace Bit0 of opcode by packet R-flag
*                                      GT_TRUE  - Bit0 of result opcode is packet R-flag
*                                      GT_FALSE - Bit0 of result opcode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamChannelTypeProfileToOpcodeMappingGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    channelTypeProfile,
    OUT GT_U32                    *opcodePtr,
    OUT GT_BOOL                   *RBitAssignmentEnablePtr
);

/**
* @internal cpssDxChPclOamMegLevelModeSet function
* @endinternal
*
* @brief   Set OAM MEG Level Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
* @param[in] megLevelMode             - OAM MEG Level Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamMegLevelModeSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PCL_DIRECTION_ENT                    direction,
    IN  GT_U32                                    oamProfile,
    IN  CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT  megLevelMode
);

/**
* @internal cpssDxChPclOamMegLevelModeGet function
* @endinternal
*
* @brief   Get OAM MEG Level Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
*
* @param[out] megLevelModePtr          - (pointer to) OAM MEG Level Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_STATE             - on unexpected value in HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamMegLevelModeGet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PCL_DIRECTION_ENT                    direction,
    IN  GT_U32                                    oamProfile,
    OUT CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT  *megLevelModePtr
);

/**
* @internal cpssDxChPclOamRdiMatchingSet function
* @endinternal
*
* @brief   Set OAM RDI(Remote Direct Indicator) Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
* @param[in] rdiMask                  - Mask for RDI bit.(APPLICABLE RANGES: 0..255)
* @param[in] rdiPattern               - Pattern for RDI bit.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB4 (profile0) or UDB5 (profile1)
*                                      Result = ((packetByte & rdiMask) == rdiPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRdiMatchingSet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  GT_U32                        oamProfile,
    IN  GT_U32                        rdiMask,
    IN  GT_U32                        rdiPattern
);

/**
* @internal cpssDxChPclOamRdiMatchingGet function
* @endinternal
*
* @brief   Get OAM RDI(Remote Direct Indicator) Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
*
* @param[out] rdiMaskPtr               - (pointer to)Mask for RDI bit.(APPLICABLE RANGES: 0..255)
* @param[out] rdiPatternPtr            - (pointer to)Pattern for RDI bit.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB4 (profile0) or UDB5 (profile1)
*                                      Result = ((packetByte & rdiMask) == rdiPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRdiMatchingGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  GT_U32                        oamProfile,
    OUT GT_U32                        *rdiMaskPtr,
    OUT GT_U32                        *rdiPatternPtr
);

/**
* @internal cpssDxChPclOamEgressPacketDetectionSet function
* @endinternal
*
* @brief   Set OAM Egress Packet Detection parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress Packet Detection mode.
* @param[in] sourceId                 - packet Source Id. When mode ==
*                                      CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_SOURCE_ID_E
*                                      and packet source ID equal to sourceId value
*                                      the packet treated as OAM packet
*                                      regardless to PCL action results.
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamEgressPacketDetectionSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT  mode,
    IN  GT_U32                                    sourceId
);

/**
* @internal cpssDxChPclOamEgressPacketDetectionGet function
* @endinternal
*
* @brief   Get OAM Egress Packet Detection parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to)Egress Packet Detection mode.
* @param[out] sourceIdPtr              - (pointer to)packet Source Id. When mode ==
*                                      CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_SOURCE_ID_E
*                                      and packet source ID equal to sourceId value
*                                      the packet treated as OAM packet
*                                      regardless to PCL action results.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamEgressPacketDetectionGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT  *modePtr,
    OUT GT_U32                                    *sourceIdPtr
);

/**
* @internal cpssDxChPclOamRFlagMatchingSet function
* @endinternal
*
* @brief   Set OAM R-Flag Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
* @param[in] rFlagMask                - Mask for R-Flag.(APPLICABLE RANGES: 0..255)
* @param[in] rFlagPattern             - Pattern for R-Flag.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB8 (profile0) or UDB9 (profile1)
*                                      Result = ((packetByte & rFlagMask) == rFlagPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRFlagMatchingSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        oamProfile,
    IN  GT_U32                        rFlagMask,
    IN  GT_U32                        rFlagPattern
);

/**
* @internal cpssDxChPclOamRFlagMatchingGet function
* @endinternal
*
* @brief   Get OAM R-Flag Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
*
* @param[out] rFlagMaskPtr             - (pointer to)Mask for R-Flag.(APPLICABLE RANGES: 0..255)
* @param[out] rFlagPatternPtr          - (pointer to)Pattern for R-Flag.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB8 (profile0) or UDB9 (profile1)
*                                      Result = ((packetByte & rFlagMask) == rFlagPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRFlagMatchingGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        oamProfile,
    OUT GT_U32                        *rFlagMaskPtr,
    OUT GT_U32                        *rFlagPatternPtr
);

/**
* @internal cpssDxChPclEgressSourcePortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portSelectionMode        - Source port selection mode for DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS cpssDxChPclEgressSourcePortSelectionModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT portSelectionMode
);

/**
* @internal cpssDxChPclEgressSourcePortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Source port selection mode for
*                                      DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS cpssDxChPclEgressSourcePortSelectionModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT *portSelectionModePtr
);

/**
* @internal cpssDxChPclEgressTargetPortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portSelectionMode        - Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressTargetPortSelectionModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT portSelectionMode
);

/**
* @internal cpssDxChPclEgressTargetPortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressTargetPortSelectionModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT
                                                        *portSelectionModePtr
);

/**
* @internal cpssDxChPclSourceIdMaskSet function
* @endinternal
*
* @brief   Set mask so only certain bits in the source ID will be modified due to PCL
*         action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number
* @param[in] mask                     - Source ID  (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclSourceIdMaskSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    IN  GT_U32                      mask
);

/**
* @internal cpssDxChPclSourceIdMaskGet function
* @endinternal
*
* @brief   Get mask used for source ID modify due to PCL action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number
*
* @param[out] maskPtr                  - (pointer to) Source ID mask (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclSourceIdMaskGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    OUT GT_U32                      *maskPtr
);

/**
* @internal cpssDxChPclIpPayloadMinSizeSet function
* @endinternal
*
* @brief   Set the IP payload minimum size. This value will determine the Layer 4
*         validity
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipPayloadMinSize         - the IP payload minimum size
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The L4_Valid in the packet descriptor will be cleared if:
*       For IPv4: total length - header length < IP payload minimum size
*       For IPv6: payload length < IP payload minimum size
*
*/
GT_STATUS cpssDxChPclIpPayloadMinSizeSet
(
    IN GT_U8        devNum,
    IN GT_U32       ipPayloadMinSize
);

/**
* @internal cpssDxChPclIpPayloadMinSizeGet function
* @endinternal
*
* @brief   Get the IP payload minimum size. This value will determine the Layer 4
*         validity
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ipPayloadMinSizePtr      - the IP payload minimum size
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The L4_Valid in the packet descriptor will be cleared if:
*       For IPv4: total length - header length < IP payload minimum size
*       For IPv6: payload length < IP payload minimum size
*
*/
GT_STATUS cpssDxChPclIpPayloadMinSizeGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *ipPayloadMinSizePtr
);


/**
* @internal cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet function
* @endinternal
*
* @brief   Function enables/disables for ingress analyzed packet the using of VID from
*         the incoming packet to access the EPCL Configuration table and for lookup
*         keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      enable
);


/**
* @internal cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet function
* @endinternal
*
* @brief   Get state of the flag in charge of the using of VID of the incoming packet
*         for ingress analyzed packet to access the EPCL Configuration table and for
*         lookup keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable/disable the using of original VID.
*                                      GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet
(
    IN  GT_U8         devNum,
    OUT GT_BOOL       *enablePtr
);

/**
* @internal cpssDxChPclEgressSubLookupPhaInfoSet function
* @endinternal
*
* @brief   Function sets the masks for PHA related info that will
*          be overridden by the specific sub-lookup.
*       NOTE: relate only to EPCL (Egress PCL)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      (APPLICABLE RANGES: 0..3).
* @param[in] phaMetadataMask          - the PHA 'meta data' mask for this sub-lookup.(32 bits mask)
* @param[in] phaThreadIdMask          - the PHA 'thread Id' mask for this sub-lookup.( 8 bits mask)
*                                      (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or subLookupNum or phaThreadIdMask
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressSubLookupPhaInfoSet
(
    IN GT_U8        devNum,
    IN GT_U32       subLookupNum,
    IN GT_U32       phaMetadataMask,
    IN GT_U32       phaThreadIdMask
);

/**
* @internal cpssDxChPclEgressSubLookupPhaInfoGet function
* @endinternal
*
* @brief   Function gets the masks for PHA related info that will
*          be overridden by the specific sub-lookup.
*       NOTE: relate only to EPCL (Egress PCL)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      (APPLICABLE RANGES: 0..3).
* @param[out] phaMetadataMaskPtr      - (pointer to)the PHA 'meta data' mask for this sub-lookup.(32 bits mask)
* @param[out] phaThreadIdMaskPtr      - (pointer to)the PHA 'thread Id' mask for this sub-lookup.( 8 bits mask)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or subLookupNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressSubLookupPhaInfoGet
(
    IN GT_U8        devNum,
    IN GT_U32       subLookupNum,
    OUT GT_U32      *phaMetadataMaskPtr,
    OUT GT_U32      *phaThreadIdMaskPtr
);

/**
* @internal cpssDxChPclRulesDump function
* @endinternal
*
* @brief    Dump all valid PCL rules of specified size.
*
* @note     APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                          (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                          bitmap must be set with at least one bit representing
*                                          valid port group(s). If a bit of non valid port group
*                                          is set then function returns GT_BAD_PARAM.
*                                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] tcamIndex                - index of the TCAM unit.
*                                        (APPLICABLE DEVICES AC5) (APPLICABLE RANGES 0..1)
* @param[in] ruleSize                 - size of rule
* @param[in] startIndex               - index of first rule
* @param[in] rulesAmount              - number of rules to scan and dump
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - on trying to dump rules from powered down TCAM floors
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             tcamIndex,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             startIndex,
    IN GT_U32                             rulesAmount
);

/**
* @internal cpssDxChPclCopyReservedMaskSet function
* @endinternal
*
* @brief   Sets the copyReserved mask for specific direction and lookup stages.  The mask is used to
*          set the bit of the IPCL/EPCL action <copyReserved> field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                         - device number
* @param[in] direction                      - Policy direction: Ingress or Egress
* @param[in] lookupNum                      - lookup stage number for which mask to be set
* @param[in] mask                           - CopyReserved bit mask for IPCL/EPCL action <copyReserved> Field.
*                                             (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon 0..0x3FFFF)
*                                             (APPLICABLE RANGES: AC5X; Harrier; Ironman, AC5P 0..0x7FFFF)
*
* @retval GT_OK                             - on success
* @retval GT_OUT_OF_RANGE                   - on out of range value for mask
* @retval GT_BAD_PARAM                      - on wrong value devNum, direction or lookupNum
* @retval GT_HW_ERROR                       - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE          - on not applicable device
*/
GT_STATUS cpssDxChPclCopyReservedMaskSet
(
    IN   GT_U8                             devNum,
    IN   CPSS_PCL_DIRECTION_ENT            direction,
    IN   CPSS_PCL_LOOKUP_NUMBER_ENT        lookupNum,
    IN   GT_U32                            mask
);

/**
* @internal cpssDxChPclCopyReservedMaskGet function
* @endinternal
*
* @brief  Get the mask based on specific direction(ingress/egress) and lookup stage
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                  - device number
* @param[in] direction               - Policy direction: Ingress or Egress
* @param[in] lookupNum               - lookup stage number for which mask to be set
*
* @param[out] maskPtr                - (pointer to) copyReserved bit mask for IPCL action <copyReserved> Field.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM               - on wrong value for devNum, direction or lookupNum
* @retval GT_HW_ERROR                - on hardware error
* @retval GT_BAD_PTR                 - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE   - on not applicable device
*/
GT_STATUS cpssDxChPclCopyReservedMaskGet
(
    IN    GT_U8                          devNum,
    IN    CPSS_PCL_DIRECTION_ENT         direction,
    IN    CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum,
    OUT   GT_U32                        *maskPtr
);

/**
* @internal cpssDxChPclTriggerHashCncClientMaskSet function
* @endinternal
*
* @brief   Enable/disable of CNC Hash client trigger overwrite by specific lookup stage
*
* @note   APPLICABLE DEVICES:      Falcon;
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                         - device number
* @param[in] lookupNum                      - lookup stage number for which mask to be set
* @param[in] maskEn                         - GT_TRUE - enable overwrite by IPCL action
*                                             GT_FALSE - disable overwrite by IPCL action.
*
* @retval GT_OK                             - on success
* @retval GT_BAD_PARAM                      - on wrong value devNum or lookupNum
* @retval GT_HW_ERROR                       - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE          - on not applicable device
*/
GT_STATUS cpssDxChPclTriggerHashCncClientMaskSet
(
    IN   GT_U8                             devNum,
    IN   CPSS_PCL_LOOKUP_NUMBER_ENT        lookupNum,
    IN   GT_BOOL                           maskEn
);

/**
* @internal cpssDxChPclTriggerHashCncClientMaskGet function
* @endinternal
*
* @brief  Get the mask of CNC Hash client trigger based on specific lookup stage
*
* @note   APPLICABLE DEVICES:      Falcon;
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                  - device number
* @param[in] lookupNum               - lookup stage number for which mask to be set
*
* @param[out] maskPtr                - (pointer to) overwrite mask by IPCL action state
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM               - on wrong value for devNum or lookupNum
* @retval GT_HW_ERROR                - on hardware error
* @retval GT_BAD_PTR                 - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE   - on not applicable device
*/
GT_STATUS cpssDxChPclTriggerHashCncClientMaskGet
(
    IN    GT_U8                          devNum,
    IN    CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum,
    OUT   GT_BOOL                        *maskPtr
);

/**
* @internal cpssDxChPclMapLookupsToTcamIndexesSet function
* @endinternal
*
* @brief   Maps 4 PCL lookups each to one of TCAM units of the device.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                         - device number
* @param[in] pclLookupsTcamIndexesPtr       - (pointer to)TCAM indexes mapped to PCL lookups.
*
* @retval GT_OK                             - on success
* @retval GT_OUT_OF_RANGE                   - on out of range value for mask
* @retval GT_BAD_PARAM                      - on wrong parameter value
* @retval GT_BAD_PTR                        - on NULL pointer parameter
* @retval GT_HW_ERROR                       - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE          - on not applicable device
*/
GT_STATUS cpssDxChPclMapLookupsToTcamIndexesSet
(
    IN   GT_U8                                  devNum,
    IN   CPSS_DXCH_PCL_LOOKUP_TCAM_INDEXES_STC  *pclLookupsTcamIndexesPtr
);

/**
* @internal cpssDxChPclMapLookupsToTcamIndexesGet function
* @endinternal
*
* @brief   Gets map of 4 PCL lookups each to one of TCAM units of the device.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                        - device number
* @param[out] pclLookupsTcamIndexesPtr      - (pointer to)TCAM indexes mapped to PCL lookups.
*
* @retval GT_OK                             - on success
* @retval GT_BAD_PARAM                      - on wrong parameter value
* @retval GT_BAD_PTR                        - on NULL pointer parameter
* @retval GT_HW_ERROR                       - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE          - on not applicable device
*/
GT_STATUS cpssDxChPclMapLookupsToTcamIndexesGet
(
    IN   GT_U8           devNum,
    OUT  CPSS_DXCH_PCL_LOOKUP_TCAM_INDEXES_STC  *pclLookupsTcamIndexesPtr
);

/**
* @internal cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet function
* @endinternal
*
* @brief   Sets Map Configuration Table Entry Profile Id to TCAM Profile Id
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                         - device number
* @param[in] direction                      - Policy direction: Ingress or Egress
* @param[in] packetType                     - packet Type
* @param[in] cfgTableEmProfileId            - Configuration Table Entry EM Profile Id (used 4 LSBs only)
*                                             (APPLICABLE RANGES: 0 ... 15)
* @param[in] tcamProfileId                  - TCAM Profile Id.
*                                             (APPLICABLE RANGES: 0 ... 63)
*
* @retval GT_OK                             - on success
* @retval GT_BAD_PARAM                      - on wrong parameter value
* @retval GT_OUT_OF_RANGE                   - on out-of-range parameter value
* @retval GT_HW_ERROR                       - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE          - on not applicable device
*/
GT_STATUS cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet
(
    IN   GT_U8                             devNum,
    IN   CPSS_PCL_DIRECTION_ENT            direction,
    IN   CPSS_DXCH_PCL_PACKET_TYPE_ENT     packetType,
    IN   GT_U32                            cfgTableEmProfileId,
    IN   GT_U32                            tcamProfileId
);

/**
* @internal cpssDxChPclMapCfgTableProfileIdToTcamProfileIdGet function
* @endinternal
*
* @brief   Gets Map Configuration Table Entry Profile Id to TCAM Profile Id
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                         - device number
* @param[in] direction                      - Policy direction: Ingress or Egress
* @param[in] packetType                     - packet Type
* @param[in] cfgTableEmProfileId            - Configuration Table Entry EM Profile Id (used 4 LSBs only)
*                                             (APPLICABLE RANGES: 0 ... 15)
* @param[out] tcamProfileIdPtr              - (Pointer to)TCAM Profile Id.
*
* @retval GT_OK                             - on success
* @retval GT_BAD_PARAM                      - on wrong parameter value
* @retval GT_BAD_PTR                        - on NULL pointer parameter value
* @retval GT_HW_ERROR                       - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE          - on not applicable device
*/
GT_STATUS cpssDxChPclMapCfgTableProfileIdToTcamProfileIdGet
(
    IN   GT_U8                             devNum,
    IN   CPSS_PCL_DIRECTION_ENT            direction,
    IN   CPSS_DXCH_PCL_PACKET_TYPE_ENT     packetType,
    IN   GT_U32                            cfgTableEmProfileId,
    OUT  GT_U32                            *tcamProfileIdPtr
);

/**
* @internal cpssDxChPclPortPclId2Set function
* @endinternal
*
* @brief   Set source/target port PCL-ID2 value per lookup stage
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; AC5 Bobcat3; Aldrin2; Falcon;
*
* @param[in] devNum                  - device number
* @param[in] portNum                 - port number
* @param[in] direction               - Policy direction: Ingress or Egress
* @param[in] lookupNum               - lookup stage number for which port PCL-ID2 need to be configured
* @param[in] portPclId2              - PCL-ID2 bytes to be included in PCL lookup key
*                                      (APPLICABLE RANGE: 0...FFFFFF)
*
* @retval GT_OK               - on success
* @retval GT_HW_ERROR         - on hardware error
* @retval GT_BAD_PARAM        - on wrong device or port number, direction or lookupNum
* @retval GT_OUT_OF_RANGE     - portPclId2 value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclPortPclId2Set
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          portNum,
    IN CPSS_PCL_DIRECTION_ENT        direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN GT_U32                        portPclId2
);

/**
* @internal cpssDxChPclPortPclId2Get function
* @endinternal
*
* @brief   Get source/target port PCL-ID2 value per lookup stage
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; AC5 Bobcat3; Aldrin2; Falcon;
*
* @param[in] devNum         - device number
* @param[in] portNum        - port number
* @param[in] direction      - Policy direction: Ingress or Egress
* @param[in] lookupNum      - lookup stage number for which port PCL-ID2 need to be configured
*
* @param[out] portPclId2Ptr - (pointer to) PCL-ID2 bytes to be included in PCL lookup key
*
* @retval GT_OK             - on success
* @retval GT_HW_ERROR       - on hardware error
* @retval GT_BAD_PARAM      - on wrong device, port number, direction or lookupNum
* @retval GT_BAD_PTR        - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclPortPclId2Get
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT GT_U32                         *portPclId2Ptr
);

/**
* @internal cpssDxChPclFlowIdActionAssignModeSet function
* @endinternal
*
* @brief   Set per stage per parallel lookup the action field of 'flow-id' assignment mode.
*          to assign 'flow-id' field descriptor and/or 'stream-id' field descriptor.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] direction             - Policy direction:
*                                   Ingress or Egress
* @param[in] lookupNum             - Lookup number: 0/1/2
*                                    Relevant only to Ingress direction
* @param[in] hitNum                - the hit num. (in the parallel lookup)
*                                    the device may support 2 or 4 parallel lookups.
* @param[in] mode                  - the action field of 'flow-id' assignment mode
*                                    to assign 'flow-id' field descriptor and/or 'stream-id' field descriptor.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclFlowIdActionAssignModeSet
(
    IN GT_U8                           devNum,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN GT_U32                          hitNum,
    IN CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_ENT mode
);

/**
* @internal cpssDxChPclFlowIdActionAssignModeGet function
* @endinternal
*
* @brief   Get per stage per parallel lookup the action field of 'flow-id' assignment mode.
*          to assign 'flow-id' field descriptor and/or 'stream-id' field descriptor.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] direction             - Policy direction:
*                                   Ingress or Egress
* @param[in] lookupNum             - Lookup number: 0/1/2
*                                    Relevant only to Ingress direction
* @param[in] hitNum                - the hit num. (in the parallel lookup)
*                                    the device may support 2 or 4 parallel lookups.
*
* @param[out] modePtr              - (pointer to) the action field of 'flow-id' assignment mode
*                                    to assign 'flow-id' field descriptor and/or 'stream-id' field descriptor.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclFlowIdActionAssignModeGet
(
    IN GT_U8                           devNum,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN GT_U32                          hitNum,
    OUT CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPclNumberOfStreamIdsSet function
* @endinternal
*
* @brief   Set the maxNumber of stream-Ids that the IPCL/EPCL will assign into
*           descriptor 'stream-Id' , in mode :
*           CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; AC5 Bobcat3; Aldrin2; Falcon;AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number
* @param[in] direction      - Policy direction: Ingress or Egress
* @param[in] maxNum         - The maxNumber of stream-Ids that the IPCL/EPCL will assign into descriptor 'stream-Id'
*                              in mode : CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E
*                              APPLICABLE RANGE : 0..(64K-1)
*
* @retval GT_OK             - on success
* @retval GT_HW_ERROR       - on hardware error
* @retval GT_BAD_PARAM      - on wrong device, direction
* @retval GT_OUT_OF_RANGE   - on out of range maxNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclNumberOfStreamIdsSet
(
    IN GT_U8                           devNum,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN GT_U32                          maxNum
);

/**
* @internal cpssDxChPclNumberOfStreamIdsGet function
* @endinternal
*
* @brief   Get the maxNumber of stream-Ids that the IPCL/EPCL will assign into
*           descriptor 'stream-Id' , in mode :
*           CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; AC5 Bobcat3; Aldrin2; Falcon;AC5P; AC5X; Harrier;
*
* @param[in] devNum         - device number
* @param[in] direction      - Policy direction: Ingress or Egress
*
* @param[out] maxNumPtr     - (pointer to)The maxNumber of stream-Ids that the IPCL/EPCL will assign into descriptor 'stream-Id'
*                              in mode : CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E
*                              APPLICABLE RANGE : 0..(64K-1)
*
* @retval GT_OK             - on success
* @retval GT_HW_ERROR       - on hardware error
* @retval GT_BAD_PARAM      - on wrong device, direction
* @retval GT_BAD_PTR        - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclNumberOfStreamIdsGet
(
    IN GT_U8                           devNum,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    OUT GT_U32                         *maxNumPtr
);

/**
* @internal cpssDxChPclEpgConfigSet function
* @endinternal
*
* @brief   Sets per stage EPG related config.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] lookupNum             - Lookup number: 0/1/2, Ignored for AC5P, AC5X, Harrier.
* @param[in] epgConfigPtr          - (Pointer to) EPG config.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclEpgConfigSet
(
    IN GT_U8                           devNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_EPG_CONFIG_STC    *epgConfigPtr
);

/**
* @internal cpssDxChPclEpgConfigGet function
* @endinternal
*
* @brief   Gets per stage EPG related config.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] lookupNum             - Lookup number: 0/1/2, Ignored for AC5P, AC5X, Harrier.
* @param[out] epgConfigPtr         - (Pointer to) EPG config.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on unexpected value in HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclEpgConfigGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT CPSS_DXCH_PCL_EPG_CONFIG_STC   *epgConfigPtr
);

/**
* @internal cpssDxChPclGenericActionModeSet function
* @endinternal
*
* @brief   Sets the per lookup, generic action mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] lookupNum             - Lookup number: 0/1/2
* @param[in] hitNum                - the hit num. (in the parallel lookup)
*                                    the device may support 2 or 4 parallel lookups.
* @param[in] genericActionMode     - Generic Action mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclGenericActionModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT               lookupNum,
    IN GT_U32                                   hitNum,
    IN CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT    genericActionMode
);

/**
* @internal cpssDxChPclGenericActionModeGet function
* @endinternal
*
* @brief  Gets the per lookup, generic action mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] lookupNum             - Lookup number: 0/1/2
* @param[in] hitNum                - the hit num. (in the parallel lookup)
*                                    the device may support 2 or 4 parallel lookups.
* @param[out] genericActionModePtr - (pointer to)Generic Action mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on unexpected value in HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclGenericActionModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT              lookupNum,
    IN  GT_U32                                  hitNum,
    OUT CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT   *genericActionModePtr
);

/**
* @internal cpssDxChPclPortEpgConfigSet function
* @endinternal
*
* @brief   Sets the per port EPG Configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] epgConfigPtr          - (pointer to)EPG configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - NULL pointer parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortEpgConfigSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_PCL_PORT_EPG_CONFIG_STC          *epgConfigPtr
);

/**
* @internal cpssDxChPclPortEpgConfigGet function
* @endinternal
*
* @brief   Gets the per port EPG Configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] epgConfigPtr         - (pointer to)EPG configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - NULL pointer parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortEpgConfigGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    OUT CPSS_DXCH_PCL_PORT_EPG_CONFIG_STC          *epgConfigPtr
);

/**
* @internal cpssDxChPclPortDestinationEpgConfigSet function
* @endinternal
*
* @brief  Sets the Destination EPG configuration per physical port
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portDstEpgConfigPtr   - (Pointer to)Port Destination EPG configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclPortDestinationEpgConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PCL_PORT_DST_EPG_CONFIG_STC   *portDstEpgConfigPtr
);

/**
* @internal cpssDxChPclPortDestinationEpgConfigGet function
* @endinternal
*
* @brief  Gets the Destination EPG configuration per physical port
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] portDstEpgConfigPtr  - (Pointer to)Port Destination EPG configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPclPortDestinationEpgConfigGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_PCL_PORT_DST_EPG_CONFIG_STC   *portDstEpgConfigPtr
);

/**
* @internal cpssDxChPclMaxSduSizeProfileSet function
* @endinternal
*
* @brief   Set the maximum Service Data Unit (SDU) bytes size per profile
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum             - device number
* @param[in] direction          - Policy direction: Ingress or Egress
* @param[in] profileNum         - profile number (APPLICABLE RANGES:  0..3)
* @param[in] maxSduSize         - maximum Service Data Unit (SDU) bytes size for this profileNum
*                                 (APPLICABLE RANGES:  0..2^16 - 1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - on maxSduSize value out of range.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclMaxSduSizeProfileSet
(
    IN GT_U8                     devNum,
    IN CPSS_PCL_DIRECTION_ENT    direction,
    IN GT_U32                    profileNum,
    IN GT_U32                    maxSduSize
);

/**
* @internal cpssDxChPclMaxSduSizeProfileGet function
* @endinternal
*
* @brief   Get the maximum Service Data Unit (SDU) bytes size per profile
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum           - device number
* @param[in]  direction        - Policy direction: Ingress or Egress
* @param[in]  profileNum       - profile number (APPLICABLE RANGES:  0..3)
* @param[out] maxSduSizePtr    - (pointer to) maximum Service Data Unit (SDU) bytes size for this profileNum
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclMaxSduSizeProfileGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_PCL_DIRECTION_ENT    direction,
    IN  GT_U32                    profileNum,
    OUT GT_U32                    *maxSduSizePtr
);

/**
* @internal cpssDxChPclMaxSduSizeExceptionSet function
* @endinternal
*
* @brief  Set the packet command and CPU drop code for exception case in which
*         packet byte count is bigger than Service Data Unit (SDU) Size Profile byte count
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum        - device number.
* @param[in] direction     - Ingress Policy or Egress Policy
* @param[in] command       - the packet command.
*                            APPLICABLE VALUES: CPSS_PACKET_CMD_FORWARD_E;
*                                               CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
*                                               CPSS_PACKET_CMD_TRAP_TO_CPU_E;
*                                               CPSS_PACKET_CMD_DROP_HARD_E;
*                                               CPSS_PACKET_CMD_DROP_SOFT_E.
* @param[in] cpuCode       - the cpu code in case of none forward command
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclMaxSduSizeExceptionSet
(
    IN GT_U8                       devNum,
    IN CPSS_PCL_DIRECTION_ENT      direction,
    IN CPSS_PACKET_CMD_ENT         command,
    IN CPSS_NET_RX_CPU_CODE_ENT    cpuCode
);

/**
* @internal cpssDxChPclMaxSduSizeExceptionGet function
* @endinternal
*
* @brief  Get the packet command and CPU drop code for exception case in which
*         packet byte count is bigger than Service Data Unit (SDU) Size Profile byte count
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]   devNum        - device number.
* @param[in]   direction     - Ingress Policy or Egress Policy
* @param[out] commandPtr     - (pointer to) the packet command.
* @param[out] cpuCodePtr     - (pointer to) the cpu code in case of none forward command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclMaxSduSizeExceptionGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_PCL_DIRECTION_ENT    direction,
    OUT CPSS_PACKET_CMD_ENT       *commandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT  *cpuCodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPclh */

