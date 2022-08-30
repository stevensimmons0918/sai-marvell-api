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
* @file tgfMplsGen.h
*
* @brief Generic API for MPLS
*
* @version   1
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfMplsGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfMplsGenh
#define __tgfMplsGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>
#include <common/tgfCommon.h>

/**
* @enum PRV_TGF_MPLS_EXCEPTION_TYPE_ENT
 *
 * @brief Enumeration of MPLS Exception Types.
*/
typedef enum{

    /** @brief packets with Incoming TTL 0,
     *  and TTL and Options check is not bypassed
     *  (NHLFE <Bypass TTL and Options Check>=0)
     */
    PRV_TGF_MPLS_INCOMING_TTL_EXCP_E,

    /** @brief packets with TTL that is
     *  about to be exceeded, and TTL and Options check is not
     *  bypassed (NHLFE <Bypass TTL and Options Check>=0)
     */
    PRV_TGF_MPLS_OUTGOING_TTL_EXCP_E,

    /** @brief packets length is greater than
     *  the MTU profile pointed to by the NHLFE.
     */
    PRV_TGF_MPLS_MTU_EXCP_E

} PRV_TGF_MPLS_EXCEPTION_TYPE_ENT;

/**
* @enum PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_TYPE_ENT
 *
 * @brief Types of soft dropped or trapped traffic that can be
 * enabled/disabled to trigger MPLS router engine.
*/
typedef enum{

    /** @brief Packets that were
     *  assigned a command of TRAP prior to the MPLS engine
     */
    PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_TRAP_E      = 0,

    /** @brief Packets that were
     *  assigned a command of SOFT DROP prior to the MPLS engine
     */
    PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_SOFT_DROP_E = 1

} PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_TYPE_ENT;


/**
* @enum PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ENT
 *
 * @brief Enumeration of MPLS Next Hop Access Method
 * (controls the method of calculation the NHLFE Pointer)
*/
typedef enum{

    /** @brief The ILM Table consists
     *  of NHLFE entry blocks used for Equal Cost Multi Path.
     */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ECMP_E    = 0,

    /** @brief The ILM Table consists
     *  of NHLFE entry blocks used for QoS routing.
     */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_QOS_E     = 1,

    /** @brief The ILM Table consists
     *  of individual NHLFE entries.
     *  The MPLS label is used as a direct pointer to the NHLFE.
     */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E = 2

} PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ENT;


/**
* @struct PRV_TGF_MPLS_INTERFACE_ENTRY_STC
 *
 * @brief A structure to hold MPLS Interface Table entry fields
*/
typedef struct{

    /** @brief validates the entry data.
     *  FALSE:Entry Not valid, TRUE:Entry is valid
     *  valid range (0...7) when 0 indicates a single entry.
     */
    GT_BOOL valid;

    /** the minimum MPLS label supported for the interface */
    GT_U32 minLabel;

    /** the maximum MPLS label supported for the interface */
    GT_U32 maxLabel;

    /** @brief A base index in the ILM(Incoming Label Map)
     *  Table for the entries that are bound to the interface.
     *  NOTE: this is an offset entry Index relative to
     *  <NHLFE Table Base Address>.
     *  The entries for the minLabel-maxLabel range in the
     *  NHLF table will be written from this base on.
     *  0 means the first entry in the ILM Table relative
     *  to the <MPLS NHLFE Table Base Address>.
     */
    GT_U32 baseIndex;

    /** @brief The number of entries in the ILM(Incoming Label Map)
     *  that are associated with an ECMP or QoS forwarding.
     */
    GT_U32 ecmpQosSize;

    /** Method of calculating the final NHLFE pointer. */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ENT nextHopRouteMethod;

} PRV_TGF_MPLS_INTERFACE_ENTRY_STC;


/**
* @enum PRV_TGF_MPLS_CMD_ENT
 *
 * @brief Enumeration of MPLS command assigned to the packet
*/
typedef enum{

    /** Do nothing */
    PRV_TGF_MPLS_NOP_CMD_E,

    /** Swap the most outer label with another Label */
    PRV_TGF_MPLS_SWAP_CMD_E,

    /** Add a new label on top of the MPLS labels stack */
    PRV_TGF_MPLS_PUSH_CMD_E,

    /** @brief (Penultimate Hop Pop): Pop the label from the
     *  MPLS label stack. Forwarding decision is based on the popped label
     */
    PRV_TGF_MPLS_PHP_CMD_E

} PRV_TGF_MPLS_CMD_ENT;

/**
* @enum PRV_TGF_MPLS_COUNTER_SET_INDEX_ENT
 *
 * @brief Enumeration of Management counters sets updated when packets
 * hits this Route Entry.
*/
typedef enum{

    /** Management Counters Set 0 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_0_E = 0,

    /** Management Counters Set 1 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_1_E = 1,

    /** Management Counters Set 2 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_2_E = 2,

    /** Management Counters Set 3 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_3_E = 3,

    /** @brief No management counter set is
     *  bound to this route entry. Traffic matching this route entry is not counted.
     */
    PRV_TGF_MPLS_COUNTER_SET_DISABLE_E = 7

} PRV_TGF_MPLS_COUNTER_SET_INDEX_ENT;


/**
* @enum PRV_TGF_MPLS_TTL_MODE_ENT
 *
 * @brief Enumeration of TTL Manipulation mode.
*/
typedef enum{

    /** @brief TTL of routed packets associated
     *  with this Next Hop Entry is not decremented when the packet is
     *  transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_NOT_DECREMENTED_E = 0,

    /** @brief TTL of routed packets associated
     *  with this Next Hop Entry is decremented by 1 when the packet is
     *  transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_DECREMENTED_BY_ONE_E,

    /** @brief TTL of routed packets
     *  associated with this Next Hop Entry is decremented by <TTL> when
     *  the packet is transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_DECREMENTED_BY_ENTRY_E,

    /** @brief TTL of routed packets associated with
     *  this Next Hop Entry is assigned by this entry to a new TTL
     *  value, when the packet is transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_COPY_ENTRY_E

} PRV_TGF_MPLS_TTL_MODE_ENT;

/**
* @struct PRV_TGF_MPLS_NHLFE_COMMON_STC
 *
 * @brief Describes the logical content of the NHLF Table entries
*/
typedef struct{

    /** The label to use when MPLS Command is Swap or Push */
    GT_U32 mplsLabel;

    /** MPLS command assigned to the packet. */
    PRV_TGF_MPLS_CMD_ENT mplsCmd;

    /** output logical interfce configuration parameters. */
    PRV_TGF_OUTLIF_INFO_STC outlifConfig;

    /** @brief The next hop VLAN
     *  label lookup
     */
    GT_U16 nextHopVlanId;

    /** @brief This bit is set to GT_TRUE when the
     *  label lookup matches this entry
     */
    GT_BOOL ageRefresh;

    /** index of MTU profile. Range (0..15) */
    GT_U32 mtuProfileIndex;

    /** @brief Management Counter set update when packets
     *  hits this Route Entry.
     */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_ENT counterSetIndex;

    GT_BOOL mirrorToIngressAnalyzer;

    /** @brief The TTL value that is used when the <TTL Mode>
     *  is either: Decrement TTL by NHLFE<TTL> or
     *  Copy NHLFE<TTL>
     *  Range(0...255)
     *  ttMode         - TTL manipulation mode
     */
    GT_U32 ttl;

    PRV_TGF_MPLS_TTL_MODE_ENT ttlMode;

    /** @brief Enables/disables bypassing TTL
     *  exception and Options check.
     */
    GT_BOOL bypassTtlExceptionCheckEnable;

    /** @brief Used for indexing in CPU code table; Relevant
     *  only if <Command>= TRAP or ROUTE_AND_MIRROR
     *  Range(0...3)
     */
    GT_U32 cpuCodeIndex;

    /** @brief The NHLFE assigns a command based on the
     *  entry <Packet Command>.
     *  Valid options :
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     *  CPSS_PACKET_CMD_ROUTE_E
     *  CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E
     */
    CPSS_PACKET_CMD_ENT packetCmd;

} PRV_TGF_MPLS_NHLFE_COMMON_STC;

/**
* @struct PRV_TGF_MPLS_NHLFE_STC
 *
 * @brief Describes the logical content of the NHLF Table entries.
*/
typedef struct{

    /** NHLFE common content */
    PRV_TGF_MPLS_NHLFE_COMMON_STC nhlfeCommonParams;

    /** Enable/Disable modifing per QoS parametes */
    PRV_TGF_QOS_PARAM_MODIFY_STC qosParamsModify;

    /** QoS Parameters */
    PRV_TGF_QOS_PARAM_STC qosParams;

} PRV_TGF_MPLS_NHLFE_STC;

/**
* @enum PRV_TGF_MPLS_FRR_COMMAND_ENT
 *
 * @brief Enumeration of Fast-Reroute Command modes.
*/
typedef enum{

    /** packet's OutLIF isnt mapped from FRR entry */
    PRV_TGF_MPLS_FRR_COMMAND_DONT_MAP_E = 0,

    /** packet's OutLIF is assigned from FRR entry */
    PRV_TGF_MPLS_FRR_COMMAND_MAP_E      = 1

} PRV_TGF_MPLS_FRR_COMMAND_ENT;

/*
 * Typedef: struct PRV_TGF_MPLS_FAST_REROUTE_STC
 *
 * Description:
 *          Logical content of the Fast Re-Route table entry.
 *
 * Fields:
 *  outLif          - The output interface this FRR entry sends to.
 *                       Valid types of outLif.interfaceInfo.type:
 *                           CPSS_INTERFACE_PORT_E
 *                           CPSS_INTERFACE_TRUNK_E
 *                           CPSS_INTERFACE_VIDX_E
 *                           CPSS_INTERFACE_FABRIC_VIDX_E
 *  mapCommand      - whether to map outlif from FRR entry or to retain packet's outlif
 */

typedef struct PRV_TGF_MPLS_FAST_REROUTE_STCT
{
    PRV_TGF_OUTLIF_INFO_STC         outLif;
    PRV_TGF_MPLS_FRR_COMMAND_ENT    mapCommand;

}PRV_TGF_MPLS_FAST_REROUTE_STC;


/**
* @enum PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT
 *
 * @brief Enumeration of MPLS Label space mode.
*/
typedef enum{

    /** @brief Label space is common for
     *  all interfaces.
     */
    PRV_TGF_MPLS_PER_PLATFORM_E,

    /** @brief Label space is defined per
     *  interface according to the assigned VRF.
     */
    PRV_TGF_MPLS_PER_INTERFACE_E

} PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT;

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __tgfMplsGenh */

