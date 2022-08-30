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
* @file tgfExactMatchGen.h
*
* @brief Generic API for Exact Match
*
* @version   1
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfExactMatchGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfExactMatchGenh
#define __tgfExactMatchGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <common/tgfTunnelGen.h>
    #include <common/tgfPclGen.h>
    #include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#endif /* CHX_FAMILY */

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/
#ifdef CHX_FAMILY
/* max key size is 47 byte --> 376 bits of data */
#define PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS 47

/* max number of banks in EM */
#define PRV_TGF_EXACT_MATCH_MAX_NUM_BANKS_CNS   16

/* Converts key size to number of Exact Match banks value */
#define PRV_TGF_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(_numOfBanks, _keySize)\
    switch (_keySize)                                                                   \
    {                                                                                   \
        case PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E:                                       \
            _numOfBanks = 1;                                                            \
            break;                                                                      \
        case PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E:                                      \
            _numOfBanks = 2;                                                            \
            break;                                                                      \
        case PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E:                                      \
            _numOfBanks = 3;                                                            \
            break;                                                                      \
        case PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E:                                      \
            _numOfBanks = 4;                                                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* Converts number of Exact Match banks to key size value */
#define PRV_TGF_EXACT_MATCH_CONVERT_NUM_OF_BANKS_TO_KEY_SIZE_VAL_MAC(_keySize, _numOfBanks)\
    switch (_numOfBanks)                                                                \
    {                                                                                   \
        case 1:                                                                         \
            _keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;                             \
            break;                                                                      \
        case 2:                                                                         \
            _keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;                            \
            break;                                                                      \
        case 3:                                                                         \
            _keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;                            \
            break;                                                                      \
        case 4:                                                                         \
            _keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* check Exact Match hit number range   */
#define PRV_TGF_EXACT_MATCH_HIT_NUM_CHECK_MAC(_hitNum)                                   \
    if ((_hitNum < 0) || (_hitNum > 1))                                               \
    {                                                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                \
    }

/**
 * @enum PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT
 *
 * @brief Exact Match key size
 * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 *
*/
typedef enum{
    /** Exact Match key size 5 Bytes (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E,

    /** Exact Match key size 14 Bytes (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
    PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E,

    /** Exact Match key size 33 Bytes (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
    PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E,

    /** Exact Match key size 47 Bytes (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
    PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E,

    PRV_TGF_EXACT_MATCH_KEY_SIZE_LAST_E

} PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT;

/**
* @enum PRV_TGF_EXACT_MATCH_CLIENT_ENT
 *
*  @brief Exact Match clients.
*  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef enum{

    /** EM client tunnel termination (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_CLIENT_TTI_E,

    /** EM client ingress policy 0 (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E,

    /** EM client ingress policy 1 (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_CLIENT_IPCL_1_E,

    /** EM client ingress policy 2 (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_CLIENT_IPCL_2_E,

   /** EM client egress policy (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E,

    PRV_TGF_EXACT_MATCH_CLIENT_LAST_E

} PRV_TGF_EXACT_MATCH_CLIENT_ENT;


/**
 * @struct PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC
 *
 * @brief Exact Match Profile key parameters
*/
typedef struct{

    /** @brief key Size */
    PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT keySize;

    /** @brief Start of Exact Match Search key in the TCAM key. */
    GT_U32 keyStart;

    /** @brief Mask the bits in the EM Search Key */
    GT_U8 mask[PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS];

} PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC;


/**
* @enum PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT
 *
 * @brief Exact Match action type enum
*/
typedef enum{

    /** Exact Match action type is TTI action
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_ACTION_TTI_E   = 0,

    /** Exact Match action type is PCL action
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_ACTION_PCL_E   = 1,

    /** Exact Match action type is EPCL action
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_ACTION_EPCL_E   = 2,

    PRV_TGF_EXACT_MATCH_ACTION_LAST_E

} PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT;


/**
 * @enum PRV_TGF_EXACT_MATCH_ACTION_UNT
 *
 * @brief Exact Match action union
*/
typedef union{
    /** Exact Match action is TTI action (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_TTI_ACTION_2_STC ttiAction;

    /** Exact Match action is PCL action (APPLICABLE DEVICES:
     *  Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_PCL_ACTION_STC   pclAction;

} PRV_TGF_EXACT_MATCH_ACTION_UNT;

/**
* @struct PRV_TGF_EXACT_MATCH_USE_TTI_ACTIONS_STC
*
*  @brief EXACT MATCH use TTI TCAM rule action parameters.
*
*         Determines whether the associated byte is assigned
*         from the EM reduces Action, or assigned a default
*         value.
*         GT_FALSE = Assigned default value
*         GT_TRUE = Assigned from Action bytes of the exact
*         match Entry
*
*         The “overwritten” mean field will be taken from
*         reduced Action (aka replacing value at expended
*         action)
*/
typedef struct{

    /** @brief Command assigned to packets matching this TTI rule
     *         valid values: forward, mirror, trap, hard_drop_soft_drop
     *  bits 0-2  */
    GT_BOOL overwriteExpandedActionCommand;

    /** @brief If ActionCommand = TRAP or MIRROR or SOFT/HARD DROP,
     *         this code is the CPU/DROP Code passed to the CPU as
     *         part of the packet.
     *  bits 3-10  */
    GT_BOOL overwriteExpandedActionUserDefinedCpuCode;

     /** @brief Enables mirroring the packet to an Ingress Analyzer interface.
      *         The field is three bits long, and can indicate one of seven possible analyzers.
      *         A value of ZERO indicates that mirroring is not enabled.
      *         If the port configuration assigns a different
      *         analyzer index, the higher index wins.
      *  bits 11-13  */
    GT_BOOL overwriteExpandedActionMirrorToIngressAnalyzerIndex;

   /** @brief where to redirect the packet
    *  0x0 = NoRedirect; Do not redirect this packet.
    *  0x1 = Egress Interface; Redirect this packet to the
    *  configured Egress Interface.
    *  0x2 = IPNextHop; Policy-based routing: assign index to
    *  Router Lookup Translation Table (LTT).
    *  0x4 = Assign VRF-ID; Assign VRF-ID from the TTI Action
    *  entry. The packet is not redirected.
    *  bits 14-16 */
    GT_BOOL overwriteExpandedActionRedirectCommand;

    /** @brief egress interface when
    *           redirect command = "Egress Interface"
    * bits 17 - target is lag
    * bits 17-32 eVidx
    * bits 18-29 trunk ID
    * bits 18-31 target ePort
    * bits 32-41 target device
    * bit 42 useVidx */
    GT_BOOL overwriteExpandedActionEgressInterface;

   /** @brief The egressInterfaceMask specify the bits to
     *  be taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionEgressInterface == GT_TRUE
     *
     *  egress Interface attribute in TTI action is located in:
     *  bits 17-32 eVidx
     *      bit [16] useVidx + bits [23:17] are placed in one byte
     *      bits [31:24] are placed in another byte
     *      bits [32]    is placed in another byte
     *
     *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of egressInterfaceMask[15] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  any combination of the bytes must include the first byte since the useVidx is located in the first byte and will always be set
     *
     *  bits 18-29 trunk ID
     *      bit [17] isTrunk + bits [23:18] are placed in one byte
     *      bits [29:24] are placed in another byte
     *
     *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[11:6]  - this option is not supported alone - we can not set only second
     *  byte since the isTrunk(bit 17) and useVidx(bit 16) are located in the first byte and will always be set
     *  when setting trunkId
     *
     *  bits 18-31 target ePort
     *      bit [17] isTrunk + bits [23:18] are placed in one byte
     *      bits [31:24] are placed in another byte
     *
     *  0 or a value>=BIT_14- use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[13:6] is set, than 8 msb are taken from the reduced entry - this option is not supported
     *  alone - we can not set only second byte since the isTrunk(bit 17) and useVidx(bit 16) are located in the first byte
     *  and will always be set when setting eport
     *
     *  bits 32-41 target device
     *      bits [39:32] are placed in one byte
     *      bits [41:40] are placed in another byte
     *
     *  0 or a value>=BIT_10 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[9:8] is set, than 2 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32  egressInterfaceMask;

    /** @brief VRF
     * bits 17-28 */
    GT_BOOL overwriteExpandedActionVrfId;

    /** @brief The vrfIdmask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionVrfId == GT_TRUE
     *  vrfId attribute in TTI action is located in bits [28:17]
     *  bits [23:17] are placed in one byte
     *  bits [28:24] are placed in another byte
     *
     *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of vrfIdMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of vrfIdMask[12:7] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32  vrfIdMask;

    /** @brief The flow ID when redirect command = "No Redirect"
     *   bits 17-29
     *   if this field is TRUE and the flowId!=0 then we use the
     *   flowId value to calculate the max number of bits it can
     *   hold in the reduced entry
     */
    GT_BOOL overwriteExpandedActionFlowId;

    /** @brief The flow ID mask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionFlowId == GT_TRUE
     *  flowId attribute in TTI action is located in bits [29:17]
     *  bits [23:17] are placed in one byte
     *  bits [29:24] are placed in  another byte
     *
     *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of flowIdMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of flowIdMask[12:7] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  flowId attribute in TTI action is located in bits [32:17]
     *  bits [23:17] are placed in one byte
     *  bits [31:24] are placed in  another byte
     *  bit  [32]    is placed in  another byte
     *
     *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
     *  if any bit of flowIdMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of flowIdMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of flowIdMask[15] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32  flowIdMask;

    /** @brief The IP Next Hop Entry Index
     *      when redirect command = "IP Next Hop"
     *   bits 17-34 */
    GT_BOOL overwriteExpandedActionRouterLttPtr;

   /** @brief The routerLttPtrMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionRouterLttPtr == GT_TRUE
    *  routerLttPtr attribute in TTI action is located in bits [34:17]
    *  bits [23:17] are placed in one byte
    *  bits [31:24] are placed in another byte
    *  bits [34:32] are placed in another byte
    *
    *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of routerLttPtrMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of routerLttPtrMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of routerLttPtrMask[17:15] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 routerLttPtrMask;

    /** @brief Used for building hash key and IPCL keys.
        Direct index to one of 7 UDB configuration table user defined
        EtherType entries, from index 9 to 15 (UDE entries in index 6 and 8
        are not accessible using this direct index). Used for extracting
        packet header fields for packets that differ in their header format,
        but share the same EtherType (e.g., MPLS and MPLS-TP).
        0 - Disabled: Use packet EtherType to select the UDB
        configuration table entry
        1- Assign UDB configuration table index 9
        2 - Assign UDB configuration table index 10
        .....
        7 - Assign UDB configuration table index 15.
        relevant when redirect command = "No Redirect"
        bits 30-32 */
    GT_BOOL overwriteExpandedActionIpclUdbConfigTable;

    /** @brief If set, this a network testing flow.
     *  Therefore the packet should be redirected to an
     *  egress interface where prior to transmission the packet's
     *  MAC SA and MAC DA are switched.
     *  NOTE: This should always be accompanied with
     *  Redirect Command = 1 (Policy Switching).
     *  bit 43   */
    GT_BOOL overwriteExpandedActionVntl2Echo;

    /** @brief When set, packet is redirected to a tunnel start
     *         entry pointed by Tunnel_Index
     * NOTE: This field is valid only when Redirect Command =
     * Redirect to Egress Interface (Policy-switching)
     * bit 44 */
    GT_BOOL overwriteExpandedActionTunnelStart;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and
     *  TunnelStart="False"
     *  bits 45-62 */
    GT_BOOL overwriteExpandedActionArpPtr;

    /** @brief The arpPtrMask specify the bits to be
      *  taken from the reduced entry, the rest will be taken
      *  from the expander.
      *  Relevant only if overwriteExpandedActionArpPtr == GT_TRUE
      *  arpPtr attribute in TTI action is located in bits [45:62]
      *  bits [47:45] are placed in one byte
      *  bits [55:48] are placed in another byte
      *  bits [62:56] are placed in another byte
      *
      *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
      *  if any bit of arpPtrMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
      *  if any bit of arpPtrMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
      *  if any bit of arpPtrMask[17:11] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
      */
      GT_U32 arpPtrMask;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and TunnelStart="True"
     *  bits 45-60 */
    GT_BOOL overwriteExpandedActionTunnelIndex;

  /** @brief The tunnelIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionTunnelIndex == GT_TRUE
    *  tunnelIndex attribute in TTI action is located in bits [45:60]
    *  bits [47:45] are placed in one byte
    *  bits [55:48] are placed in another byte
    *  bits [60:56] are placed in another byte
    *
    *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of tunnelIndexMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of tunnelIndexMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of tunnelIndexMask[15:11] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 tunnelIndexMask;

     /** @brief Controls the index used for IPCL0 lookup
      *  relevant when Redirect_Command !="Egress_Interface"
      *  bit 51 */
    GT_BOOL overwriteExpandedActionPcl0OverrideConfigIndex;

    /** @brief Controls the index used for IPCL1 lookup
     *  relevant when Redirect_Command !="Egress_Interface"
     *  bit 50 */
    GT_BOOL overwriteExpandedActionPcl1OverrideConfigIndex;

    /** @brief Controls the index used for IPCL2 lookup
     *  relevant when Redirect_Command !="Egress_Interface"
     *  bit 49 */
    GT_BOOL overwriteExpandedActionPcl2OverrideConfigIndex;

    /** @brief Pointer to IPCL configuration entry to be used when
     *  fetching IPCL parameters.Relevant when TTI Action "PCL0/1/2
     *  Override Config Index" = Override. Min-Max: 0x0 - 0x10FF
     *  relevant when Redirect_Command !="Egress
     * bits 52-64 */
    GT_BOOL overwriteExpandedActionIpclConfigIndex;

  /** @brief The ipclConfigIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionIpclConfigIndex == GT_TRUE
    *  ipclConfigIndex attribute in TTI action is located in bits [52:64]
    *  bits [55:52] are placed in one byte
    *  bits [63:56] are placed in another byte
    *  bits [64] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of ipclConfigIndexMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of ipclConfigIndexMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of ipclConfigIndexMask[12] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  ipclConfigIndex attribute in TTI action is located in bits [53:65]
    *  bits [55:53] are placed in one byte
    *  bits [63:56] are placed in another byte
    *  bits [65:64] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of ipclConfigIndexMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of ipclConfigIndexMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of ipclConfigIndexMask[12:11] is set, than 2 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    */
    GT_U32 ipclConfigIndexMask;

    /** @brief Type of passenger packet being to be encapsulated.
        This field is valid only when Redirect Command = 1
        (Policy-switching) and TunnelStart = 1 When packet is not TT
         and MPLS_Command != NOP, the Tunnel_Type bit must be set
         to OTHER. 0x0 = Ethernet; 0x1 = Other;
         bit 61 */
    GT_BOOL overwriteExpandedActionTsPassengerPacketType;

    /** @brief When enabled, the packet is marked for MAC DA
     *         modification, and the ARP_Index (bits 45-62)
     *         specifies the new MAC DA.
     *  relevant when Redirect_Command =="Egress_Interface"
     *  bit 63 */
    GT_BOOL overwriteExpandedActionModifyMacDa;

     /** @brief When enabled, the packet is marked for MAC SA
      *         modification - similar to routed packets.
      *  relevant when Redirect_Command =="Egress_Interface"
      *  bit 64 */
    GT_BOOL overwriteExpandedActionModifyMacSa;

    /** @brief If set, the packet is bound to the a counter
     *         specified in CNCCounterIndex
     *  bit 65 */
    GT_BOOL overwriteExpandedActionBindToCentralCounter;

    /** @brief Counter index to be used if the BindToCNCCounter
     *  is set.
     *  bits 66-79 */
    GT_BOOL overwriteExpandedActionCentralCounterIndex;

 /** @brief The centralCounterIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionCentralCounterIndex == GT_TRUE
    *  centralCounterIndex attribute in TTI action is located in bits [66-79]
    *  bits [71:66] are placed in one byte
    *  bits [79:72] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of centralCounterIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of centralCounterIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  centralCounterIndex attribute in TTI action is located in bits [81-95]
    *  bits [81:87] are placed in one byte
    *  bits [88:95] are placed in another byte
    *
    *  0 or a value>=BIT_15 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of centralCounterIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of centralCounterIndexMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 centralCounterIndexMask;

    /** @brief  If set, the packet is bound to the a meter
     *          specified in PolicerIndex.
     *  bit 80 */
    GT_BOOL overwriteExpandedActionBindToPolicerMeter;

    /** @brief If set, the packet is bound to the counter specified
     *         in the PolicerIndex.
     *  NOTE: When both BindToPolicerMeter and BindToPolicerCounter
     *  are set, the PolicerIndex is used for metering and the
     *  counter index is assigned by the metering entry.
     *  bit 81 */
    GT_BOOL overwriteExpandedActionBindToPolicer;

    /** @brief  Traffic profile or counter to be used when the
     *  BindToPolicerMeter or BindToPolicerCounter are set.
     *  bits 82-95 */
    GT_BOOL overwriteExpandedActionPolicerIndex;

    /** @brief The policerIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionPolicerIndex == GT_TRUE
    *  PolicerIndex attribute in TTI action is located in bits [82-95]
    *  bits [87:82] are placed in one byte
    *  bits [95:88] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of policerIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of policerIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  PolicerIndex attribute in TTI action is located in bits [98-111]
    *  bits [98:103] are placed in one byte
    *  bits [104:111] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of policerIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of policerIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 policerIndexMask;

     /** if set, then the sourceId is assign to the packet
      *  bit 96 */
    GT_BOOL overwriteExpandedActionSourceIdSetEnable;

    /** @brief Source ID (also known as SST-ID) that is assign to
     *         the packet if SourceIDAssignmentEnable is set.
     *  bits 97-108 */
    GT_BOOL overwriteExpandedActionSourceId;

    /** @brief The sourceIdMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionSourceId == GT_TRUE
    *  SourceId attribute in TTI action is located in bits [97-108]
    *  bits [103:97] are placed in one byte
    *  bits [108:103] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of sourceIdMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of sourceIdMask[11:7] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  SourceId attribute in TTI action is located in bits [67:78]
    *  bits [67:71] are placed in one byte
    *  bits [72:78] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of sourceIdMask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of sourceIdMask[11:5] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 sourceIdMask;

    /** @brief if set, the packet will not have any searches
      *  in the IPCL mechanism
      *  bit 109 */
    GT_BOOL overwriteExpandedActionActionStop;

    /** @brief if set, the packet isn't subject to any bridge
      * mechanisms bit 110 */
    GT_BOOL overwriteExpandedActionBridgeBypass;

    /** If set, the ingress engines are all bypassed for this packet
     *  bit 111 */
    GT_BOOL overwriteExpandedActionIngressPipeBypass;

    /** @brief VLAN0 precedence options
     *  0x0 = Soft; VID assigned by TTI can be modified by the
     *  following VLAN assignment mechanisms.
     *  0x1 = Hard; VID assigned by TTI cannot be modified by the
     *  following VLAN assignment mechanisms.
     *  bit 112   */
    GT_BOOL overwriteExpandedActionTag0VlanPrecedence;

    /** @brief enable/disable nested vlan
        If set, the packet is classified as Untagged.
        NOTE: Relevant only if the packet is Ethernet or
        tunnel-terminated Ethernet-over-X.
        bit 113 */
    GT_BOOL overwriteExpandedActionNestedVlanEnable;

   /** @brief tag0 vlan command. This field determines the eVLAN
    *         assigned to the packet
    *  bits 114-116 */
    GT_BOOL overwriteExpandedActionTag0VlanCmd;

    /** @brief tag0 VLAN
     *  NOTE: This field is valid only when VLAN0Command!= 0.
     *  bits 117-129 */
    GT_BOOL overwriteExpandedActionTag0VlanId;

    /** @brief The tag0VlanIdMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionTag0VlanId == GT_TRUE
    *  tag0VlanId attribute in TTI action is located in bits [117-129]
    *  bits [119:117] are placed in one byte
    *  bits [120-127] are placed in another byte
    *  bits [129-128] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of tag0VlanIdMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of tag0VlanIdMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of tag0VlanIdMask[12:11] is set, than 2 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 tag0VlanIdMask;

    /** @brief tag1 vlan command
     *  This field determines the VLAN1 assigned to the packet
     *  matching the TTI entry. If the packet is TT, the following
     *  applies to the passenger Ethernet packet; otherwise the
     *  following applies to the packet as it was received.
     *  bit 130 */
    GT_BOOL overwriteExpandedActionTag1VlanCmd;

    /** @brief tag1 VLAN
     *  bits 131-142 */
    GT_BOOL overwriteExpandedActionTag1VlanId;

    /** @brief The tag1VlanIdMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionTag1VlanId == GT_TRUE
    *  tag1VlanId attribute in TTI action is located in bits [142-131]
    *  bits [135:131] are placed in one byte
    *  bits [142:136] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of tag1VlanIdMask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of tag1VlanIdMask[11:5] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 tag1VlanIdMask;

 /** @brief  QoS precedence options
    0x0 = Soft; QoS Parameters may be modified by the following engines.
    0x1 = Hard; QoS Parameters cannot be modified by the
    following engines.
    bit 143 */
    GT_BOOL overwriteExpandedActionQosPrecedence;

    /** @brief QoS profile This field is 7-bit wide although the QoS
     *         mechanism supports 10 bit QoS.
     *  bits 144-150 */
    GT_BOOL overwriteExpandedActionQosProfile;

   /** @brief modify DSCP mode (or EXP for MPLS)
     *  (refer to CPSS_DXCH_TTI_MODIFY_DSCP_ENT)
     *   Controls the different options of DSCP/EXP modifications
     *   0x0 = Keep Previous; Do not modify the previous
     *   0x1 = Enable; Enable packet DSCP/EXP modification
     *   0x2 = Disable; Disable packet DSCP/EXP modification
         bits 151-152 */
    GT_BOOL overwriteExpandedActionModifyDscp;

    /** @brief Control the different options of tag0 UP
       *         modifications
       *  bits 153-154 */
    GT_BOOL overwriteExpandedActionModifyTag0;

    /** @brief Control the different options of QoS Profile assignments.
        0x0 = False; Assign QoS profile based on TTI-AE QoS fields.
        0x1 = True; Keep prior QoS Profile assignment.
        bit 155 */
    GT_BOOL overwriteExpandedActionKeepPreviousQoS;

     /** @brief When set, enables the QoS Profile to be assigned
      *         by the UP2QoS algorithm.
      *  bit 156 */
    GT_BOOL overwriteExpandedActionTrustUp;

    /** @brief When set, enables the QoS Profile to be assigned
     *         by the DSCP2Qos algorithm.
     *  bit 157 */
    GT_BOOL overwriteExpandedActionTrustDscp;

    /** @brief When set, enables the QoS Profile to be assigned
     *         by the EXP2QoS algorithm
     *  bit 158 */
    GT_BOOL overwriteExpandedActionTrustExp;

    /** @brief When set, enables the DSCP to be re-assigned by
     *         DSCP2DSCP algorithm.
     *  bit 159 */
    GT_BOOL overwriteExpandedActionRemapDscp;

    /** @brief  tag0 UP assignment
     *  bits 160-162 */
    GT_BOOL overwriteExpandedActionTag0Up;

    /** @brief tag1 UP command (refer to
     *  CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT) When the packet is TT,
     *  Tag1 identification is based on passenger header.
     *  bits 163-164 */
    GT_BOOL overwriteExpandedActionTag1UpCommand;

    /** @brief tag1 UP assignment
     *  bits 165-167 */
    GT_BOOL overwriteExpandedActionTag1Up;

    /** @brief This field is the tunnel passenger packet type when
     *  either terminating a tunnel (TTI Action Tunnel_Terminate> =
     *  Enabled) or parsing a non-MPLS transit tunnel(TTI Action<
     *  Passenger_Parsing_of_Non-MPLS_Transit_Tunnels> = 1)
     *  This field is not applicable to transit MPLS packets.
     *  bits 168-169 */
    GT_BOOL overwriteExpandedActionTtPassengerPacketType;

    /** @brief If set, the packet's TTL is assigned according to
     *  the tunnel header rather than from passenger packet's
     *  TTL/Hop-Limit.
     *  bit 170 */
    GT_BOOL overwriteExpandedActionCopyTtlExpFromTunnelHeader;

    /** @brief whether the packet's tunnel header is removed;
     *  must not be set for non-tunneled packets
     *  bit 171
     */
    GT_BOOL overwriteExpandedActionTunnelTerminate;

     /** @brief MPLS Action applied to the packet
      *  bits 172-174   */
    GT_BOOL overwriteExpandedActionMplsCommand;

    /** @brief Interface-based CRC hash mask selection.
        This field determines which mask is used in the
        CRC-based hash. A value of 0x0 means that the mask is not
        determined by the TTI action entry.
        bits 175-178 */
    GT_BOOL overwriteExpandedActionHashMaskIndex;

     /** @brief the 'Trust Qos Mapping Table Index'
      *  There are 12 global Trust QoS Mapping Table used for Trust
      *  L2, L3, EXP, DSCP-to-DSCP mapping. This field selects which
      *  of the 12 global sets of Trust QoS Mapping tables is
      *  used.
      *  bits 179-182 */
    GT_BOOL overwriteExpandedActionQosMappingTableIndex;

     /** @brief This is used for supporting MPLS L-LSP QoS, where the
        scheduling traffic class is derived from the MPLS label
        and the drop precedence is derived from the EXP. NOTE: This
        field is only relevant if TTI Action TRUST_EXP is unset,
        and TTI Action Keep_QoS is unset
        0x0 = Disabled;
        0x1 = Enabled; the packet is assigned a QoSProfile that is
        the sum of the TTI Action QoS_Profile + packet outer label
        EXP bit 183 */
    GT_BOOL overwriteExpandedActionMplsLLspQoSProfileEnable;

    /** @brief TTL to be used in MPLS header that is Pushed or
        Swapped according to MPLS Command. If (TTL == 0) the TTL is
        taken from the packet's header, else TTL is set according to
        this field. NOTE: If the original packet does not contain
        TTL and a label is pushed or swapped, this field must be
        assigned a value different than 0. NOTE: This field is relevant
        for MPLS packets that have a match in the TTI lookup or for
        non-MPLS packets when the TTI action MPLS command is PUSH.
        bits 184-191 */
   GT_BOOL overwriteExpandedActionMplsTtl;

    /** @brief If set, the TTL is decremented by one.
     *  This field is relevant for MPLS packet that are not tunnel terminated.
     *  NOTE: When MPLS_Command is NOP, this field should be disabled
     *  bit 192 */
    GT_BOOL overwriteExpandedActionEnableDecrementTtl;

    /** @brief if enabled new source ePort number is taken from
     *  TTI Action Entry Source_ePort>
     *  bit 193 */
    GT_BOOL overwriteExpandedActionSourceEPortAssignmentEnable;

    /** @brief Source ePort number assigned by TTI entry when
        Source_ePort_Assignment_Enable = Enabled
        NOTE: Overrides any previous assignment of source ePort
        number.
        bits 194-207 */
    GT_BOOL overwriteExpandedActionSourceEPort;

    /** @brief The sourceEPortMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionSourceEPort == GT_TRUE
    *  sourceEPort attribute in TTI action is located in bits [194-207]
    *  bits [199:194] are placed in one byte
    *  bits [207-200] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of sourceEPortMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of sourceEPortMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 sourceEPortMask;

    /** @brief This field defines the tunnel header length in resolution of 2B,
     *  where the tunnel header is relative to the TTI Action
     *  Tunnel_Header_Length_Anchor_Type.
     *  If '0' then the TT Header Len is default TT
     *  bits 208-213 */
    GT_BOOL overwriteExpandedActionTtHeaderLength;

    /** @brief When enabled, the TTI Action defines where the
     *  parser should start parsing the PTP header. The location is
     *  defined by TTI Action PTP_Offset
     *  bit 214 */
    GT_BOOL overwriteExpandedActionIsPtpPacket;

    /** @brief  When enabled, it indicates that a timestamp
     *  should be inserted into the packet. relevant when
     *  IsPtpPacket = "0"
     *  bit 215 */
    GT_BOOL overwriteExpandedActionOamTimeStampEnable;

    /** @brief When TTI Action IsPTPPacket is enabled, this field
     *  specifies the PTP Trigger Type.
     *  bits 215-216 */
    GT_BOOL overwriteExpandedActionPtpTriggerType;

    /** @brief When TTI Action IsPTPPacket is enabled, this field
     *  defines the byte offset to start of the PTP header, relative
     *  to the start of the L3 header.
     *  NOTE: This offset resolution is 2 bytes.
     *  relevant when IsPtpPacket = "1"
     *  bits 217-222 */
    GT_BOOL overwriteExpandedActionPtpOffset;

    /** @brief When TTI Action TimestampEnable is enabled, this
      *  field indicates the offset index itno the timestamp offset
      *  table. relevant when IsPtpPacket = "0"
      *  bits 216-222 */
    GT_BOOL overwriteExpandedActionOamOffsetIndex;

    /** @brief When this field is set, and there is a label with
      *        GAL_Value or OAL_Value anywhere in the label stack,
      *        OAM processing is enabled for this packet.
      * bit 223 */
    GT_BOOL overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable;

    /** @brief Enable for OAM Processing
     *  bit 224 */
    GT_BOOL overwriteExpandedActionOamProcessEnable;

        /** @brief When set, indicates that the MPLS packet contains
         *         a control word immediately following the bottom of
         *         the label stack.
         *  bit 225 */
    GT_BOOL overwriteExpandedActionCwBasedPw;

    /** @brief Enables TTL expiration exception command
     *         assignment for Pseudo Wire
     *  bit 226 */
    GT_BOOL overwriteExpandedActionTtlExpiryVccvEnable;

    /** @brief When set, indicates that this MPLS stack includes
     *         a flow (entropy) label at the bottom of the stack.
     *         The PW label is one label above the bottom of the
     *         label stack.
     *  bit 227 */
    GT_BOOL overwriteExpandedActionPwe3FlowLabelExist;

        /** @brief When set, indicates that this PW-CW supports
         *         E-Tree Root/Leaf indication via Control Word L bit
         *  bit 228 */
    GT_BOOL overwriteExpandedActionPwCwBasedETreeEnable;

    /** @brief The mode to select one of 12 mapping tables.
      *  0x0 = according to "Trust QoS Mapping Table Select";
      *  The table is selected  based on the TTI Action
      *  TrustQoSMappingTableSelect>;
      *  0x1 = according to packet UP;The table is selected based on
      *  the packet VLAN tag UP. In this case, the table selected is
      *  in the range of 0-7. Based on ePort configuration, the
      *  packet UP used can be taken from either Tag0 or Tag1.
         bit 229  */
    GT_BOOL overwriteExpandedActionQosUseUpAsIndexEnable;

    /** @brief When protection switching is enabled on this flow,
     *  this field indicates whether the received packet arrived on
     *  the working or protection path.
     *  bit 230 */
    GT_BOOL overwriteExpandedActionRxIsProtectionPath;

    /** @brief Enables Rx protection switching filtering.
     *  bit 231 */
    GT_BOOL overwriteExpandedActionRxProtectionSwitchEnable;

    /** @brief Override MAC2ME check and set to 1.
     *  bit 232  */
    GT_BOOL overwriteExpandedActionSetMacToMe;

    /** @brief The OAM profile determines the set of IPCL key UDBs
      * used to contain the OAM message attributes
      * (opcode, MEG level, RDI, MEG level).
      * bit 233 */
    GT_BOOL overwriteExpandedActionOamProfile;

        /** @brief Determines whether to apply the non-data CW packet
         *  command to the packet with non-data CW
         *  bit 234 */
    GT_BOOL overwriteExpandedActionApplyNonDataCwCommand;

    /** @brief Controls whether to continue with the next TTI Lookup
     *  bit 237 */
    GT_BOOL overwriteExpandedActionContinueToNextTtiLookup;

    /** @brief When enabled, the MPLS G-ACh Channel Type is
     *  mapped to an OAM Opcode that is used by the OAM engine
     *  bit 238 */
    GT_BOOL overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable;

    /** @brief This feature enables passenger parsing of transit MPLS tunnels.
      * This configuration is relevant only IF Packet is MPLS, AND
      * TTI Action TunnelTerminate = Disabled, AND Legacy global
      * EnableL3L4ParsingoverMPLS is disabled
      * bits 239-240   */
    GT_BOOL overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode;

    /** @brief This is a generic mechanism to parse the passenger
     *  of transit tunnel packets other than MPLS, for
     *  example, IP-based tunnels, MiM tunnels, etc.
     *  bit 241 */
    GT_BOOL overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable;

     /** @brief Enables skipping the FDB SA Lookup:
     * 0: If ingress port is network port then don't skip the SA
     * Lookup. If the ingress port is eDSA Cascade port then don't
     * modify the state received in eDSA SkipFdbSALookup field.
     * 1: Skip the FDB SA lookup
     * bit 242 */
    GT_BOOL overwriteExpandedActionSkipFdbSaLookupEnable;

    /** @brief Sets that this Device is an End Node of IPv6 Segment Routing
     *  bit 243 */
    GT_BOOL overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable;

    /** @brief The Anchor Type used by the TTI Action
     *  TunnelHeaderLength
       '0': L3 Anchor - TTI Action TunnelHeaderLength is relative to
        the start of the L3 header.
       '1': L4 Anchor - TTI Action TunnelHeaderLength is relative
        to the start of the L4 header.
        '2': Profile Based Anchor - TTI Action TunnelHeaderLength
        is an index to the Tunnel Termination Profiles table - only for Hawk
        bit 244 for Falcon; AC5P; AC5X.
        bit 244 - 255 for Hawk */
    GT_BOOL overwriteExpandedActionTunnelHeaderStartL4Enable;

    /** @brief If enabled, the current packet is subject to IPFIX processing,
     *  and will access the IPFIX table. If enabled, the <Flow-ID>
     *  indicates to the Policer engine the index to the IPFIX table
     *  bit 249 for Hawk */
    GT_BOOL overwriteExpandedActionIpfixEnable;

    /** @brief TTI copyReserved Assignment Enable
     *  GT_TRUE -  Assigns TTI action<copyReserved> to
     *             packet's descriptor<copyReserved> field
     *  GT_FALSE - No change in packet's descriptor<copyReserved> field by
     *             TTI engine.
     *  BIT: 35
     */
     GT_BOOL  overwriteExpandedActionCopyReservedAssignmentEnable;

    /** @brief TTI Reserved field value assignment
     *  Relevant when copyReservedAssignmentEnable = GT_TRUE
     *  (APPLICABLE RANGES: 0..2047)
     *  BIT: 36-48
     */
     GT_U32   overwriteExpandedActionCopyReserved;

     /** @brief TTI action to trigger Hash CNC client
     *  GT_FALSE - Don't enable CNC Hash client
     *  GT_TRUE - Enable CNC Hash client
     *  BIT: 37 For Falcon.
     *  BIT: 247 For AC5P; AC5X.
     */
     GT_BOOL  overwriteExpandedActionTriggerHashCncClient;

} PRV_TGF_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC;


/**
* @struct PRV_TGF_EXACT_MATCH_EXPANDED_PCL_ACTIONS_STC
*
*  @brief EXACT MATCH use PCL TCAM rule action parameters.
*
*         Determines whether the associated byte is assigned
*         from the EM reduces Action, or assigned a default
*         value.
*         GT_FALSE = Assigned default value
*         GT_TRUE = Assigned from Action bytes of the exact
*         match Entry
*
*         The “overwritten” mean field will be taken from
*         reduced Action (aka replacing value at expended
*         action)
*/
typedef struct{

    /** @brief If ActionCommand = TRAP or MIRROR or SOFT/HARD DROP,
     *         this code is the CPU/DROP Code passed to the CPU as
     *         part of the packet.
     *  bits 0-7 for IPCL
     *  bits 170-177 for EPCL */
    GT_BOOL     overwriteExpandedActionUserDefinedCpuCode;

    /** @brief Packet commands assigned to packets matching this
     *  rule (forward, mirror hard-drop, soft-drop, or trap-to-cpu)
     *  bits 8-10 for IPCL
     *  bits 0-2 for EPCL
     */
    GT_BOOL overwriteExpandedActionPktCmd;

    /** @brief where to redirect the packet
     *  0x0 = NoRedirect; Do not redirect this packet.
     *  0x1 = Egress Interface; Redirect this packet to the
     *  configured Egress Interface.
     *  0x2 = IPNextHop; Policy-based routing: assign index to
     *  Router Lookup Translation Table (LTT).
     *  0x4 = Assign VRF-ID; Assign VRF-ID from the TTI Action
     *  entry. The packet is not redirected.
        bits 11-13 IPCL only */
    GT_BOOL overwriteExpandedActionRedirectCommand;

    /** @brief egress interface when
     *           redirect command = "Egress Interface"
     * bits 17 - target is lag
     * bits 17-32 eVidx
     * bits 18-29 trunk ID
     * bits 18-31 target ePort
     * bits 32-41 target device
     * bit 16 useVidx IPCL only */
    GT_BOOL overwriteExpandedActionEgressInterface;

    /** @brief The egressInterfaceMask specify the bits to
     *  be taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionEgressInterface == GT_TRUE
     *
     *  egress Interface attribute in IPCL action is located in:
     *  bits 17-32 eVidx
     *      bit [16] useVidx + bits [23:17] are placed in one byte
     *      bits [31:24] are placed in another byte
     *      bits [32]    is placed in another byte
     *
     *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of egressInterfaceMask[15] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  any combination of the bytes must include the first byte since the useVidx is located in the first byte and will always be set
     *
     *  bits 18-29 trunk ID
     *      bit [17] isTrunk + bits [23:18] are placed in one byte
     *      bits [29:24] are placed in another byte
     *
     *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[11:6]  - this option is not supported alone - we can not set only second
     *  byte since the isTrunk(bit 17) and useVidx(bit 16) are located in the first byte and will always be set
     *  when setting trunkId
     *
     *  bits 18-31 target ePort
     *      bit [17] isTrunk + bits [23:18] are placed in one byte
     *      bits [31:24] are placed in another byte
     *
     *  0 or a value>=BIT_14- use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[13:6] is set, than 8 msb are taken from the reduced entry - this option is not supported
     *  alone - we can not set only second byte since the isTrunk(bit 17) and useVidx(bit 16) are located in the first byte
     *  and will always be set when setting eport
     *
     *  bits 32-41 target device
     *      bits [39:32] are placed in one byte
     *      bits [41:40] are placed in another byte
     *
     *  0 or a value>=BIT_10 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of egressInterfaceMask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of egressInterfaceMask[9:8] is set, than 2 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
     GT_U32  egressInterfaceMask;

    /** @brief this field set the Expanded action for MAC SA
     *  The MAC SA that is sent to the bridge engine when
     *  RedirectCommand == Replace MACSA
     *  NOTE: This field is muxed with a set of other IPCL-AE fields
     *  (Egress interface fields Policer Index)
     *  bits 16-43 macSA[27:0]
     *  bits 62-81 macSA[47:28] IPCL only */
    GT_BOOL overwriteExpandedActionMacSa;

   /** @brief The macSa_27_0_Mask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionMacSa == GT_TRUE
    *  macSaMask_27_0 attribute in IPCL action is located in bits [16:43]
    *  bits [16:23] are placed in one byte
    *  bits [24:31] are placed in another byte
    *  bits [32:39] are placed in another byte
    *  bits [40:43] are placed in another byte
    *
    *  0 or a value>=BIT_28 - use all the bits from the reduced entry (will occupy 4 bytes in the reduced entry)
    *  if any bit of macSa_27_0_Mask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of macSa_27_0_Mask[15:8] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of macSa_27_0_Mask[23:16] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of macSa_27_0_Mask[27:24] is set, than 4 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 macSa_27_0_Mask;

    /** @brief The macSa_47_28_Mask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionMacSa == GT_TRUE
    *  macSa_47_28_Mask attribute in IPCL action is located in bits [62:81]
    *  bits [62:69] are placed in one byte
    *  bits [70:77] are placed in another byte
    *  bits [78:81] are placed in another byte
    *
    *  0 or a value>=BIT_20 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of macSa_47_28_Mask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of macSa_47_28_Mask[15:8] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of macSa_47_28_Mask[19:16] is set, than 4 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 macSa_47_28_Mask;

    /** @brief The IP Next Hop Entry Index when redirect command =
     *  "IP Next Hop"
     *  bits 33-50
     *  IPCL only
     *  SIP6_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E */
    GT_BOOL overwriteExpandedActionRouterLttPtr;

/** @brief The routerLttPtrMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionRouterLttPtr == GT_TRUE
    *  routerLttPtr attribute in IPCL action is located in bits [33:50]
    *  bits [33:39] are placed in one byte
    *  bits [40:47] are placed in another byte
    *  bits [48:50] are placed in another byte
    *
    *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of routerLttPtrMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of routerLttPtrMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of routerLttPtrMask[17:15] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 routerLttPtrMask;

    /** @brief If set, this a network testing flow.
     *  Therefore the packet should be redirected to an
     *  egress interface where prior to transmission the packet's
     *  MAC SA and MAC DA are switched.
     *  NOTE: This should always be accompanied with
     *  Redirect Command = 1 (Policy Switching).
     *  bit 42 VNTL2Echo
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E   */
    GT_BOOL overwriteExpandedActionVntl2Echo;

    /** @brief When set, packet is redirected to a tunnel start
     *         entry pointed by Tunnel_Index
     * NOTE: This field is valid only when Redirect Command =
     * Redirect to Egress Interface (Policy-switching)
     * bit 43
     * IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E   */
    GT_BOOL overwriteExpandedActionTunnelStart;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and
     *  TunnelStart="False"
     *  bits 44-61
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E   */
    GT_BOOL overwriteExpandedActionArpPtr;

    /** @brief The arpPtrMask specify the bits to be
      *  taken from the reduced entry, the rest will be taken
      *  from the expander.
      *  Relevant only if overwriteExpandedActionArpPtr == GT_TRUE
      *  arpPtr attribute in IPCL action is located in bits [44:61]
      *  bits [44:47] are placed in one byte
      *  bits [48:55] are placed in another byte
      *  bits [56:61] are placed in another byte
      *
      *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
      *  if any bit of arpPtrMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
      *  if any bit of arpPtrMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
      *  if any bit of arpPtrMask[17:12] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
      */
      GT_U32 arpPtrMask;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and TunnelStart="True"
     *  bits 44-59
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E */
    GT_BOOL overwriteExpandedActionTunnelIndex;

    /** @brief The tunnelIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionTunnelIndex == GT_TRUE
    *  tunnelIndex attribute in IPCL action is located in bits [44:59]
    *  bits [47:44] are placed in one byte
    *  bits [55:48] are placed in another byte
    *  bits [59:56] are placed in another byte
    *
    *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
    *  if any bit of tunnelIndexMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of tunnelIndexMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *  if any bit of tunnelIndexMask[15:12] is set, than 4 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 tunnelIndexMask;

    /** @brief Type of passenger packet being to be encapsulated.
     *   This field is valid only when Redirect Command = 1 (Policy-switching)
     *   and TunnelStart = 1 When packet is not TT and MPLS_Command != NOP,
     *   the Tunnel_Type bit must be set to OTHER.
     *   0x0 = Ethernet; 0x1 = Other;
     *   bit 60 IPCL only
     *    SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E */
    GT_BOOL overwriteExpandedActionTsPassengerPacketType;

    /** @brief vrf-id
     *  bits 52-63 VRF-ID
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E */
    GT_BOOL overwriteExpandedActionVrfId;

    /** @brief The vrfIdMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionVrfId == GT_TRUE
     *  vrfId attribute in IPCL action is located in bits [52:63]
     *  bits [52:55] are placed in one byte
     *  bits [56:63] are placed in another byte
     *
     *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of vrfIdMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of vrfIdMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32  vrfIdMask;

    /** @brief Action Stop
     *  GT_TRUE - to skip the following PCL lookups
     *  GT_FALSE - to continue with following PCL lookups
     *  Relevant to Policy Action Entry only.
     *  bit 14 - IPCL only   */
    GT_BOOL overwriteExpandedActionActionStop;

    /** @brief Override MAC2ME check and set to 1
     *  0x0 = Do not override Mac2Me mechanism;
     *  0x1 = Set Mac2Me to 1; Override the Mac2Me mechanism and set
     *  Mac2Me to 0x1;
     *  bit 15 - IPCL only  */
    GT_BOOL overwriteExpandedActionSetMacToMe;

    /** @brief see CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
     *  bit 16 IPCL1 override Config Index
     *  - IPCL only */
    GT_BOOL overwriteExpandedActionPCL1OverrideConfigIndex;

    /** @brief see CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
     *  bit 17 PCL2 override Config Index
     *  - IPCL only */
    GT_BOOL overwriteExpandedActionPCL2OverrideConfigIndex;

    /** @brief see CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
     *   bit 18-30 IPC Configuration Index
    *  - IPCL only */
    GT_BOOL overwriteExpandedActionIPCLConfigurationIndex;

    /** @brief The ipclConfigurationIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionIPCLConfigurationIndex == GT_TRUE
    *  ipclConfigurationIndex attribute in IPCL action is located in bits [18:30]
    *  bits [18:23] are placed in one byte
    *  bits [24:30] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of ipclConfigurationIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of ipclConfigurationIndexMask[12:6] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 ipclConfigurationIndexMask;

    /** @brief packet Policing configuration
     *  see CPSS_DXCH_PCL_ACTION_POLICER_STC
     *  for IPCL: bit 64 bindToPolicerMeter
     *            bit 65 bindToPolicerCounter
     *  for EPCL: bit 62 bindToPolicerCounter
     *            bit 63 bindToPolicerMeter  */
    GT_BOOL overwriteExpandedActionPolicerEnable;

    /** @brief packet Policing configuration
     *  see CPSS_DXCH_PCL_ACTION_POLICER_STC
     *  for IPCL: bits 66-79
     *  for EPCL: bits 64-77  */
    GT_BOOL overwriteExpandedActionPolicerIndex;

   /** @brief The policerIndexMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionPolicerIndex == GT_TRUE
    *  PolicerIndex attribute
    *  in IPCL action is located in bits [66-79]
    *  bits [66:71] are placed in one byte
    *  bits [72:79] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of policerIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of policerIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  in EPCL action is located in bits[64-77]
    *  bits [64:71] are placed in one byte
    *  bits [72:77] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of policerIndexMask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of policerIndexMask[13:8] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  in EPCL action is located in bits[65-78]
    *  bits [65:71] are placed in one byte
    *  bits [72:78] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of policerIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of policerIndexMask[13:7] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    */
    GT_U32 policerIndexMask;

    /** @brief packet mirroring configuration
     *  see CPSS_DXCH_PCL_ACTION_MIRROR_STC
     *   for EPCL: bit 165-166 -Egress Mirroring Mode */
     GT_BOOL overwriteExpandedActionMirrorMode;

    /** @brief packet mirroring configuration
     *  see CPSS_DXCH_PCL_ACTION_MIRROR_STC
     *  for IPCL only :bit 82 -Enable Mirror TCP RST/FIN */
     GT_BOOL overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu;

    /** @brief packet mirroring configuration
     *  see CPSS_DXCH_PCL_ACTION_MIRROR_STC
     *  for IPCL: bits 83-85 - Mirror To Analyzer Port
     *  for EPCL: bits 167-169 -Egress Analyzer Index */
     GT_BOOL overwriteExpandedActionMirror;

    /** @brief this field set the Expanded action modify MAC DA
     *  bits 86
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E   */
    GT_BOOL overwriteExpandedActionModifyMacDA;

    /** @brief this field set the Expanded action modify MAC SA
     *  bits 87
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E   */
    GT_BOOL overwriteExpandedActionModifyMacSA;

    /** @brief the Bridge engine processed or bypassed
     *  GT_TRUE - the Bridge engine is bypassed.
     *  GT_FALSE - the Bridge engine is processed.
     *  NOTE: This field should be set if this packet if <Redirect
     *  Command> = Redirect to Egress Interface
     *  bit 88 - IPCL only  */
    GT_BOOL overwriteExpandedActionBypassBridge;

    /** @brief the ingress pipe bypassed or not.
     *  GT_TRUE - the ingress pipe is bypassed.
     *  GT_FALSE - the ingress pipe is not bypassed.
     *  NOTE: This field should be set if this packet if <Redirect
     *  Command> = Redirect to Egress Interface.
     *  bit 89 - IPCL only  */
    GT_BOOL overwriteExpandedActionBypassIngressPipe;

    /** @brief packet VLAN modification configuration
    *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
    *  for IPCL Only:
    *  bit 90 -Enable Nested VLAN */
    GT_BOOL overwriteExpandedActionNestedVlanEnable;

    /** @brief packet VLAN modification configuration
    *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
    *  for IPCL Only:
    *  bit 120 VLANPrecedence */
    GT_BOOL overwriteExpandedActionVlanPrecedence;

    /** @brief packet VLAN modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: bit 121-122 VLANCommand
     *  for EPCL: bit 48-49 tag0 VID cmd */
    GT_BOOL overwriteExpandedActionVlan0Command;

    /** @brief packet VLAN modification configuration
    *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
    *  for IPCL: 123-135 VID0
    *  for EPCL: 50-61 tag0 VID */
    GT_BOOL overwriteExpandedActionVlan0;

    /** @brief The vlan0Mask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionVlan0 == GT_TRUE
    *  vlan0 attribute
    *  in IPCL action is located in bits [123:135]
    *  bits [123-127] are placed in one byte
    *  bits [128-135] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of vlan0Mask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of vlan0Mask[12:5] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  in EPCL action is located in bits [50-61]
    *  bits [50-55] are placed in one byte
    *  bits [56-61] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of vlan0Mask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of vlan0Mask[12:6] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  bits [51-55] are placed in one byte
    *  bits [56-62] are placed in another byte
    *
    *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of vlan0Mask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of vlan0Mask[12:5] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    */
    GT_U32 vlan0Mask;

    /** @brief packet VLAN modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 152-153 vlan1_cmd
     *  for EPCL: 31 tag1 VID cmd */
    GT_BOOL overwriteExpandedActionVlan1Command;

    /** @brief packet VLAN modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 156-167 VID1
     *  for EPCL: 33-44 tag1 VID */
    GT_BOOL overwriteExpandedActionVlan1;

    /** @brief The vlan1Mask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionVlan1 == GT_TRUE
    *  vlan1 attribute
    *  in IPCL action is located in bits [156:167]
    *  bits [156-159] are placed in one byte
    *  bits [160-167] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of vlan1Mask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of vlan1Mask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  in EPCL action is located in bits [33-44]
    *  bits [33-39] are placed in one byte
    *  bits [40-44] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of vlan1Mask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of vlan1Mask[11:7] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  in EPCL action is located in bits [34-45]
    *  bits [34-39] are placed in one byte
    *  bits [40-45] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of vlan1Mask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of vlan1Mask[11:6] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    */
    GT_U32 vlan1Mask;

    /** @brief packet source Id assignment
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC,
     *  bit 91 -Source-ID Assignment Enable
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourceIdEnable;

    /** @brief packet source Id assignment
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC
     *  bits 92-103 Source-ID
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourceId;

    /** @brief The sourceIdMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionSourceId == GT_TRUE
    *  SourceId attribute in IPCL action is located in bits [92:103]
    *  bits [92:95] are placed in one byte
    *  bits [96:103] are placed in another byte
    *
    *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of sourceIdMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of sourceIdMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    */
    GT_U32 sourceIdMask;

    /** @brief match counter configuration
     *  see CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
     * for IPCL:
     * bit 105 - Bind To CNC Counter
     * bits 106-119 - CNC Counter Index
     * for EPCL:
     * bit 16 - Bind To CNC Counter
     * bits 17-30 - CNC Counter Index */
     GT_BOOL overwriteExpandedActionMatchCounterEnable;

     /** @brief match counter configuration
      *  see CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
      * for IPCL:
      * bit 105 - Bind To CNC Counter
      * bits 106-119 - CNC Counter Index
      * for EPCL:
      * bit 16 - Bind To CNC Counter
      * bits 17-30 - CNC Counter Index */
     GT_BOOL overwriteExpandedActionMatchCounterIndex;

    /** @brief The matchCounterIndexMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionMatchCounterIndex == GT_TRUE
     *  matchCounterIndex attribute in
     *  EPCL action is located in bits [17-30]
     *  bits [17:23] are placed in one byte
     *  bits [24:30] are placed in another byte
     *
     *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of matchCounterIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of matchCounterIndexMask[13:7] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  EPCL action is located in bits [17-31]
     *  bits [17:23] are placed in one byte
     *  bits [24:31] are placed in another byte
     *
     *  0 or a value>=BIT_15 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of matchCounterIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of matchCounterIndexMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  IPCL action is located in bits [106-119]
     *  bits [106:111] are placed in one byte
     *  bits [112:119] are placed in another byte
     *
     *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of matchCounterIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of matchCounterIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  IPCL action is located in bits [105-119]
     *  bits [105:111] are placed in one byte
     *  bits [112:119] are placed in another byte
     *
     *  0 or a value>=BIT_15 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of matchCounterIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of matchCounterIndexMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32  matchCounterIndexMask;

    /** @brief packet QoS attributes
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     * for IPCL only: bit 136  */
    GT_BOOL overwriteExpandedActionQosProfileMakingEnable;

    /** @brief packet QoS attributes
    *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
    * for IPCL only: bit 137
    */
    GT_BOOL overwriteExpandedActionQosPrecedence;

    /** @brief packet QoS attributes
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     * for IPCL only: bit 138-147  */
    GT_BOOL overwriteExpandedActionQoSProfile;

    /** @brief The qosProfileMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionQoSProfile == GT_TRUE
     *  qosProfileMask attribute in IPCL action is located in bits [138:147]
     *
     *  bits [138:143] are placed in one byte
     *  bits [144:147] are placed in another byte
     *
     *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of qosProfileMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of qosProfileMask[8:6] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
     GT_U32  qosProfileMask;

    /** @brief packet QoS attributes
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     * for IPCL : bit 148-149 ModifyDSCP
     * for EPCL: bits 12-13 modify dscp/exp */
    GT_BOOL overwriteExpandedActionQoSModifyDSCP;

    /** @brief packet UP1 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     *  IPCL only : 154-155 UP command */
    GT_BOOL overwriteExpandedActionUp1Command;

    /** @brief packet UP1 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 150-151 modify UP1
     *  for EPCL: bit 32 modify UP1 */
    GT_BOOL overwriteExpandedActionModifyUp1;

    /** @brief packet UP1 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 168-170 UP1
     *  for EPCL: 45-47 UP1 */
    GT_BOOL overwriteExpandedActionUp1;

    /** @brief packet DSCP/EXP modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for EPCL only: bit 3-8 dscp/exp */
    GT_BOOL overwriteExpandedActionDscpExp;

   /** @brief The dscpExpMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionDscpExp == GT_TRUE
     *  dscpExp attribute in EPCL action is located in bits [8:3]
     *  bits [7:3] are placed in one byte
     *  bit [8] are placed in another byte
     *
     *  0 or a value>=BIT_6 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of dscpExpMask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of dscpExpMask[5] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32  dscpExpMask;

    /** @brief packet UP0 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for EPCL only: 9-11 UP0 */
    GT_BOOL overwriteExpandedActionUp0;

    /** @brief packet UP0 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for EPCL only: 14-15 modify Up0  */
    GT_BOOL overwriteExpandedActionModifyUp0;

     /** @brief OAM configuration Timestamp Enable
      *  see CPSS_DXCH_PCL_ACTION_OAM_STC
      *  for IPCL:bit 192
      *  for EPCL: bit 93 */
     GT_BOOL overwriteExpandedActionOamTimestampEnable;

     /** @brief OAM configuration offset_index
     *  see CPSS_DXCH_PCL_ACTION_OAM_STC
     * for IPCL:bit 193-199
     * for EPCL: bits 94-100 */
     GT_BOOL overwriteExpandedActionOamOffsetIndex;

     /** @brief The Oam Offset Index mask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionOamOffsetIndex == GT_TRUE
     *  relevant only for EPCL. for IPCL all bits are set in one byte
     *  OamOffsetIndex attribute in EPCL action is located in bits [94:100]
     *
     *  EPCL:
     *  bits [94:95] are placed in one byte
     *  bits [96:100] are placed in  another byte
     *
     *  0 or a value>=BIT_7 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of oamOffsetIndexMask[1:0] is set, than 2 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of oamOffsetIndexMask[6:2] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  bits [98:103] are placed in one byte
     *  bits [104] are placed in  another byte
     *
     *  0 or a value>=BIT_7 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of oamOffsetIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of oamOffsetIndexMask[6] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
     GT_U32 oamOffsetIndexMask;

     /** @brief OAM configuration processing Enable
      *  see CPSS_DXCH_PCL_ACTION_OAM_STC
      *  for IPCL: bit 200
      *  for EPCL: bit 91 */
     GT_BOOL overwriteExpandedActionOamProcessingEnable;

     /** @brief OAM configuration Profile
      *  see CPSS_DXCH_PCL_ACTION_OAM_STC
      * for IPCL:201 OAM Profile
      * for EPCL:92 OAM Profile  */
     GT_BOOL overwriteExpandedActionOamProfile;

     /** @brief OAM configuration Channel Type to Opcode Mapping En
      * When enabled, the MPLS G-ACh Channel Type is mapped
      * to an OAM Opcode that is used by the OAM engine.
      * for EPCL only: bit 101  */
     GT_BOOL overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable;

    /** @brief The flow ID assigned to the packet
     *  (the value in this field is assigned into Desc "FlowID").
     *  The value 0x0 represents do not assign Flow ID. The PCL
     *  overrides the existing Flow ID value if and only if this
     *  field in the PCL action is non-zero.
     *  bit 202-214 for IPCL
     *  bit 78-90 for EPCL
     */
    GT_BOOL overwriteExpandedActionFlowId;

    /** @brief The flow ID mask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionFlowId == GT_TRUE
     *  flowId attribute in EPCL action is located in bits [78:90]
     *  in IPCL action located in bits [202:214]
     *
     * IPCL:
     *  bits [202:207] are placed in one byte
     *  bits [208-214] are placed in  another byte
     *
     *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of flowIdMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of flowIdMask[11:6] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  IPCL:
     *  bits [208:215] are placed in one byte
     *  bits [216-223] are placed in  another byte
     *
     *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of flowIdMask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of flowIdMask[15:8] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  EPCL:
     *  bits [78:79] are placed in one byte
     *  bits [80-87] are placed in  another byte
     *  bits [88-90] are placed in  another byte
     *
     *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
     *  if any bit of flowIdMask[1:0] is set, than 2 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of flowIdMask[9:2] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of flowIdMask[12:10] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  EPCL:
     *  bits [79] are placed in one byte
     *  bits [80-87] are placed in  another byte
     *  bits [88-94] are placed in  another byte
     *
     *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
     *  if any bit of flowIdMask[0] is set, than 1 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of flowIdMask[8:1] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of flowIdMask[15:9] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
    GT_U32 flowIdMask;

    /** @brief source ePort Assignment.
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC
     *  bit 216 - Assign Source ePort Enable
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourcePortEnable;

    /** @brief source ePort Assignment.
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC,
     *  bits 217-230 Source ePort
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourcePort;

    /** @brief The sourceEPortMask specify the bits to be
    *  taken from the reduced entry, the rest will be taken
    *  from the expander.
    *  Relevant only if overwriteExpandedActionSourcePort == GT_TRUE
    *  sourceEPort attribute in IPCL action is located in bits [217:230]
    *  bits [217:223] are placed in one byte
    *  bits [224:230] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of sourceEPortMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of sourceEPortMask[13:7] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    *  For AC5X, AC5P, Harrier:
    *  bits [225:231] are placed in one byte
    *  bits [232:238] are placed in another byte
    *
    *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
    *  if any bit of sourceEPortMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
    *  if any bit of sourceEPortMask[13:7] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
    *
    */
    GT_U32 sourceEPortMask;

    /** @brief Latency monitoring
     *  see CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC
     * for IPCL: bits 231-239
     * for EPCL: bits 178-186  */
    GT_BOOL overwriteExpandedActionLatencyMonitor;

    /** @brief The latencyMonitorMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionLatencyMonitor == GT_TRUE
     *  latencyMonitor attribute in EPCL action is located in bits [178:186]
     *  in IPCL action located in bits [231-239]
     *
     * IPCL:
     *  bits [231] are placed in one byte
     *  bits [232:239] are placed in  another byte
     *
     *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of latencyMonitorMask[0] is set, than 1 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of latencyMonitorMask[8:1] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  IPCL:
     *  bits [240:247] are placed in one byte
     *  bits [248] are placed in  another byte
     *
     *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of latencyMonitorMask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of latencyMonitorMask[8] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  EPCL:
     *  bits [178:183] are placed in one byte
     *  bits [184:186] are placed in  another byte
     *
     *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of latencyMonitorMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of latencyMonitorMask[8:6] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  bits [198:199] are placed in one byte
     *  bits [200:206] are placed in  another byte
     *
     *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
     *  if any bit of latencyMonitorMask[1:0] is set, than 2 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of latencyMonitorMask[8:2] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     */
    GT_U32 latencyMonitorMask;

    /** @brief Latency monitoring Enable
     *  see CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC
     * for IPCL: bit 240
     * for EPCL: bit 187 */
    GT_BOOL overwriteExpandedActionLatencyMonitorEnable;

    /** @brief Enables skipping the FDB SA Lookup:
     * 0: Don't modify the state of Skip FDB SA lookup
     * 1: Skip the FDB SA lookup bit 241 - IPCL only */
    GT_BOOL overwriteExpandedActionSkipFdbSaLookup;

    /** @brief When set, the IPCL will trigger a dedicated interrupt
     *  towards the CPU
     *  bit 242 - IPCL only  */
    GT_BOOL overwriteExpandedActionTriggerInterrupt;

    /** @brief Enable assignment of PHA Metadata:
     *  bit 123 - EPCL only  */
    GT_BOOL overwriteExpandedActionPhaMetadataAssignEnable;

    /** @brief PHA Metadata assigned
     *  bit 124-155 - EPCL only */
    GT_BOOL overwriteExpandedActionPhaMetadata;

    /** @brief The phaMetadataMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionPhaMetadata == GT_TRUE
     *  relevant only for EPCL.
     *  phaMetadata attribute in EPCL action is located in bits [124:155]
     *
     *  EPCL:
     *  bits [124:127] are placed in one byte
     *  bits [128:135] are placed in  another byte
     *  bits [136:143] are placed in  another byte
     *  bits [144:151] are placed in  another byte
     *  bits [152:155] are placed in  another byte
     *
     *  0 or a value>=BIT_32 - use all the bits from the reduced entry (will occupy 5 bytes in the reduced entry)
     *  if any bit of phaMetadataMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of phaMetadataMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of phaMetadataMask[19:12] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of phaMetadataMask[27:20] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of phaMetadataMask[31:28] is set, than 4 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     */
     GT_U32 phaMetadataMask;

    /** @brief Enable assignment of PHA Thread Number
     *  bit 156 - EPCL only */
    GT_BOOL overwriteExpandedActionPhaThreadNumberAssignEnable;

    /** @brief PHA Thread number assigned
     *  bit 157-164 - EPCL only  */
    GT_BOOL overwriteExpandedActionPhaThreadNumber;

    /** @brief The phaThreadNumberMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpandedActionPhaThreadNumber == GT_TRUE
     *  relevant only for EPCL.
     *  phaThreadNumber attribute in EPCL action is located in bits [157:164]
     *
     *  EPCL:
     *  bits [157:159] are placed in one byte
     *  bits [160:164] are placed in  another byte
     *
     *  0 or a value>=BIT_8 - use all the bits from the reduced entry (will occupy 5 bytes in the reduced entry)
     *  if any bit of phaThreadNumberMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of phaThreadNumberMask[7:3] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  For AC5X, AC5P, Harrier:
     *  bits [177:179] are placed in one byte
     *  bits [180:184] are placed in  another byte
     *
     *  0 or a value>=BIT_8 - use all the bits from the reduced entry (will occupy 5 bytes in the reduced entry)
     *  if any bit of phaThreadNumberMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of phaThreadNumberMask[7] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     */
     GT_U32 phaThreadNumberMask;

    /** @brief This field indicates the RX that this packet should be
     *  treated as Store and Forward in the same manner Cut through
     *  Slow to Fast are treated.
     *  bit 188 - EPCL only  */
    GT_BOOL overwriteExpandedActionCutThroughTerminateId;

     /** @brief Copy Reserved Enable
     *  see CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC
     *  for IPCL: bit 171
     *  for EPCL: bit 102 */
    GT_BOOL overwriteExpendedActionCopyReservedEnable;

    /** @brief Copy Reserved
     *  see CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC
     *  for IPCL: bit 172-191
     *  for EPCL: bit 103-122 */
    GT_BOOL overwriteExpendedActionCopyReserved;

    /** @brief The copyReservedMask specify the bits to be
     *  taken from the reduced entry, the rest will be taken
     *  from the expander.
     *  Relevant only if overwriteExpendedActionCopyReserved == GT_TRUE
     *  copyReserved attribute in IPCL action is located in bits [172:191]
     *  copyReserved attribute in EPCL action is located in bits [103-122]
     *
     *  IPCL:
     *  bits [172:175] are placed in one byte
     *  bits [176:183] are placed in  another byte
     *  bits [184:191] are placed in  another byte
     *
     *  0 or a value>=BIT_20 - use all the bits from the reduced entry (will occupy 4 bytes in the reduced entry)
     *  if any bit of copyReservedMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of copyReservedMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of copyReservedMask[19:12] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     *  EPCL:
     *  bits [103] are placed in one byte
     *  bits [104:111] are placed in  another byte
     *  bits [112:119] are placed in  another byte
     *  bits [102:122] are placed in  another byte
     *
     *  0 or a value>=BIT_20 - use all the bits from the reduced entry (will occupy 4 bytes in the reduced entry)
     *  if any bit of copyReservedMask[0] is set, than 1 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
     *  if any bit of copyReservedMask[8:1] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of copyReservedMask[16:9] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *  if any bit of copyReservedMask[19:17] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
     *
     */
     GT_U32 copyReservedMask;

    /** @brief If enabled, the current packet is subject to IPFIX processing,
     *  and will access the IPFIX table. If enabled, the <Flow-ID>
     *  indicates to the Policer engine the index to the IPFIX table
     *  bit 249 for Hawk */
    GT_BOOL overwriteExpandedActionIpfixEnable;

    /** @brief PCL action to trigger Hash CNC client
     *  GT_FALSE - Don't enable CNC Hash client
     *  GT_TRUE - Enable CNC Hash client
     *  BIT: 173 For Falcon.
     *  BIT: 251 For AC5P; AC5X.
     */
     GT_BOOL  overwriteExpandedActionTriggerHashCncClient;

    /** @brief EPCL action: CNC Index mode
     *  see CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_ENT
     *  BIT: 213 for Ironman.
     */
     GT_BOOL     overwriteExpandedActionEgressCncIndexMode;

    /** @brief EPCL action: Enable or disable the Max SDU Size Check
     *  BIT: 212 for Ironman.
     */
     GT_BOOL     overwriteExpandedActionEnableEgressMaxSduSizeCheck;

    /** @brief EPCL action: The Egress Max SDU Size Profile for this flow.
     *  The value is used as an index into Maximum SDU Size Profile Check.
     *  BIT: 210-211 for Ironman.
     */
     GT_BOOL     overwriteExpandedActionEgressMaxSduSizeProfile;

} PRV_TGF_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC;


/**
 * @enum PRV_TGF_EXACT_MATCH_ACTION_UNT
 *
 * @brief Exact Match action union
*/
typedef union{
    /** Exact Match action is TTI action (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC   ttiExpandedActionOrigin;

    /** Exact Match action is PCL action (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC   pclExpandedActionOrigin;

}PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT;

/**
 * @enum PRV_TGF_EXACT_MATCH_LOOKUP_ENT
 *
 * @brief Exact Match lookup number
 * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 *
*/
typedef enum{
    /** Exact Match first lookup (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E,

    /** Exact Match second lookup (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
    PRV_TGF_EXACT_MATCH_LOOKUP_SECOND_E,

    PRV_TGF_EXACT_MATCH_LOOKUP_LAST_E,

} PRV_TGF_EXACT_MATCH_LOOKUP_ENT;

/**
* @struct PRV_TGF_EXACT_MATCH_KEY_STC
 *
 * @brief Exact Match key
*/
typedef struct{

    /** @brief key Size */
    PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT keySize;

    GT_U8 pattern[PRV_TGF_EXACT_MATCH_MAX_KEY_SIZE_CNS];

} PRV_TGF_EXACT_MATCH_KEY_STC;

/**
* @struct PRV_TGF_EXACT_MATCH_ENTRY_STC
 *
 * @brief Exact Match Entry
*/
typedef struct{

    /** @brief key Size */
    PRV_TGF_EXACT_MATCH_KEY_STC         key;

    /** @brief lookup number */
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT lookupNum;

} PRV_TGF_EXACT_MATCH_ENTRY_STC;

#endif

/* convert ExactMatch action type into device specific format */
#define PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dstActionType, srcActionType)       \
    do                                                                          \
    {                                                                           \
        switch (srcActionType)                                                  \
        {                                                                       \
        case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:                                  \
            dstActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;                   \
            break;                                                              \
        case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:                                  \
            dstActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;                   \
            break;                                                              \
        case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:                                 \
            dstActionType=CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;                  \
            break;                                                              \
        default:                                                                \
            return GT_BAD_PARAM;                                                \
        }                                                                       \
    } while (0)

/**
* @internal prvTgfExactMatchTtiProfileIdModePacketTypeSet
*           function
* @endinternal
*
* @brief   Sets the Exact Match Profile Id for TTI keyType
*
*   NOTE: Client lookup for given ttiLookupNum MUST be
*      configured before this API is called
*      (cpssDxChExactMatchClientLookupSet).
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger EMlookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[in] profileId    - Exact Match profile identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_OUT_OF_RANGE       - parameter not in valid range.
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*
*/
GT_STATUS  prvTgfExactMatchTtiProfileIdModePacketTypeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT            keyType,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      ttiLookupNum,
    IN  GT_BOOL                             enableExactMatchLookup,
    IN  GT_U32                              profileId
);

/**
* @internal prvTgfExactMatchTtiProfileIdModePacketTypeGet
*           function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id form TTI keyType
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger EMlookup.
*                           GT_FALSE: Do not trigger EM lookup.
*
* @param[out] profileIdPtr  - (pointer to) Exact Match profile
*                              identifier
*       (APPLICABLE RANGES: 1..15)
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid
*         range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*/
GT_STATUS prvTgfExactMatchTtiProfileIdModePacketTypeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT            keyType,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      ttiLookupNum,
    OUT GT_BOOL                             *enableExactMatchLookupPtr,
    OUT GT_U32                              *profileIdPtr
);

/**
* @internal prvTgfExactMatchPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger EMlookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[in] profileId    - Exact Match profile identifier
*       (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  PRV_TGF_PCL_PACKET_TYPE_ENT         packetType,
   IN  GT_U32                              subProfileId,
   IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum,
   IN  GT_BOOL                             enableExactMatchLookup,
   IN  GT_U32                              profileId
);

/**
* @internal prvTgfExactMatchPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id form PCL packet type
*
* @param[in] devNum                   - device number
* @param[in] packetType               - PCL packet type
* @param[in] direction                - ingress/egress
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger EMlookup.
*                           GT_FALSE: Do not trigger EM lookup.
* @param[in] profileIdPtr - (pointer to)Exact Match profile
*       identifier (APPLICABLE RANGES: 1..15)
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  PRV_TGF_PCL_PACKET_TYPE_ENT         packetType,
   IN  GT_U32                              subProfileId,
   IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum,
   OUT GT_BOOL                             *enableExactMatchLookupPtr,
   OUT GT_U32                              *profileIdPtr
);

/**
* @internal prvTgfExactMatchClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first EM lookup and the client of the second EM
*          lookup.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientType           - client type (TTI/EPCL/IPCL)
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfExactMatchClientLookupSet
(
    IN  GT_U8                                     devNum,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  PRV_TGF_EXACT_MATCH_CLIENT_ENT            clientType
);

/**
* @internal prvTgfExactMatchClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first EM lookup and the client of the second EM
*          lookup.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientTypePtr        - (pointer to)client type
*                                 (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfExactMatchClientLookupGet
(
    IN  GT_U8                                     devNum,
    IN  PRV_TGF_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT  PRV_TGF_EXACT_MATCH_CLIENT_ENT           *clientTypePtr
);

/**
* @internal prvTgfExactMatchActivityBitEnableSet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @param[in] devNum           - device number
* @param[in] exactMatchActivityBit - GT_TRUE - enable refreshing
*                                  GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL exactMatchActivityBit
);

/**
* @internal prvTgfExactMatchActivityBitEnableGet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @param[in] devNum             - device number
* @param[in] enablePtr          - (pointer to)
* @param[in] exactMatchActivityBitPtr - (pointer to)
*                               GT_TRUE - enable refreshing
*                               GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *exactMatchActivityBitPtr
);

/**
* @internal prvTgfExactMatchActivityStatusGet function
* @endinternal
*
* @brief   Return the Exact Match activity bit for a given
*          entry.The bit is set by the device when the entry is
*          matched in the EM Lookup. The bit is reset by the CPU
*          as part of the aging process.
*
* @param[in]  devNum             - device number
* @param[in] portGroupsBmp       - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  exactMatchEntryIndex - EM entry index
* @param[in]  exactMatchClearActivity - set activity bit to 0
* @param[out] exactMatchActivityStatusPtr  - (pointer to)
*                  GT_FALSE = Not Refreshed; next_age_pass;
*                  Entry was matched since the last reset;
*                  GT_TRUE = Refreshed; two_age_pass; Entry was
*                  not matched since the last reset;
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
);

/**
* @internal prvTgfExactMatchProfileKeyParamsSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileKeyParamsSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       exactMatchProfileIndex,
    IN PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC   *keyParamsPtr
);
/**
* @internal prvTgfExactMatchProfileKeyParamsGet function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                         devNum,
    IN  GT_U32                                        exactMatchProfileIndex,
    OUT PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
);
/**
* @internal prvTgfExactMatchProfileDefaultActionSet function
* @endinternal
*
* @brief   Sets the default Action in case there is no match in the
*          Exact Match lookup and in the TCAM lookup
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                    identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEn        - Enable using Profile
*                               Table default Action in case
*                               there is no match in the Exact
*                               Match lookup and in the TCAM
*                               lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileDefaultActionSet
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 exactMatchProfileIndex,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN PRV_TGF_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                defActionEn
);

/**
* @internal prvTgfExactMatchProfileDefaultActionGet function
* @endinternal
*
* @brief   Gets the default Action in case there is no match in
*          the Exact Match lookup and in the TCAM lookup
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                     identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[out] actionPtr         - (pointer to)Exact Match Action
* @param[out] defActionEnPtr    - (pointer to) Enable using
*                               Profile Table default Action in
*                               case there is no match in the
*                               Exact Match lookup and in the
*                               TCAM lookup
*
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_BAD_PTR            - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  exactMatchProfileIndex,
    IN  PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType,
    OUT PRV_TGF_EXACT_MATCH_ACTION_UNT          *actionPtr,
    OUT GT_BOOL                                 *defActionEnPtr
);

/**
* @internal prvTgfExactMatchExpandedActionSet function
* @endinternal
*
* @brief   Sets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute wheather to take it from the Exact Match
*          entry action or from the Exact Match profile
*
*         Note: This API must be called before configuration of
*         Exact Match Entry by API cpssDxChExactMatchEntrySet
*
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT              actionType,
    IN PRV_TGF_EXACT_MATCH_ACTION_UNT                   *actionPtr,
    IN PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   *expandedActionOriginPtr
);
/**
* @internal prvTgfExactMatchExpandedActionGet function
* @endinternal
*
* @brief   Gets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute wheather to take it from the Exact Match
*          entry action or from the Exact Match profile
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         -(pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchExpandedActionGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT              actionType,
    OUT PRV_TGF_EXACT_MATCH_ACTION_UNT                  *actionPtr,
    OUT PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  *expandedActionOriginPtr
);


/**
* @internal prvTgfExactMatchPortGroupEntrySet function
* @endinternal
*
* @brief   Sets the exact match entry and its action
*
*   NOTE: this API should be called when there is a valid entry
*   in expandedActionProfile in the Profile Expander Table
*
* @param[in] devNum       - device number
* @param[in] index        - entry index in the exact match table
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] expandedActionIndex-Exact Match profile identifier
*                     (APPLICABLE RANGES:1..15)
* @param[in] entryPtr      - (pointer to)Exact Match entry
* @param[in] actionType    - Exact Match Action Type(TTI or PCL)
* @param[in] actionPtr     - (pointer to)Exact Match Action (TTI
*                            Action or PCL Action)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntrySet
(
    IN GT_U8                                  devNum,
    IN GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN GT_U32                                 index,
    IN GT_U32                                 expandedActionIndex,
    IN PRV_TGF_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN PRV_TGF_EXACT_MATCH_ACTION_UNT         *actionPtr
);

/**
* @internal prvTgfExactMatchPortGroupEntryGet function
* @endinternal
*
* @brief   Gets the exact match entry and its action
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                 NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] index - entry index in the exact match table
* @param[out]validPtr   - (pointer to) is the entry valid or not
* @param[in] actionType - Exact Match Action Type(TTI or PCL)
* @param[out] actionPtr - (pointer to)Exact Match Action
*                         (TTI Action or PCL Action)
* @param[out] entryPtr  - (pointer to)Exact Match entry
* @param[out] expandedActionIndexPtr-(pointer to)Exact Match
*             profile identifier (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntryGet
(
    IN GT_U8                                  devNum,
    IN GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN GT_U32                                 index,
    OUT GT_BOOL                               *validPtr,
    OUT PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT PRV_TGF_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT PRV_TGF_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                *expandedActionIndexPtr
);

/**
* @internal prvTgfExactMatchPortGroupEntryInvalidate function
* @endinternal
*
* @brief   Invalidate the exact match entry
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntryInvalidate
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN GT_U32                               index
);

/**
* @internal prvTgfExactMatchPortGroupEntryStatusGet function
* @endinternal
*
* @brief   Return exact match entry status
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index        - entry index in the Exact Match table
* @param[out]validPtr     - (pointer to) is the entry valid or not
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on index out of range
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfExactMatchPortGroupEntryStatusGet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN GT_U32                               index,
    OUT GT_BOOL                             *validPtr
);

/**
* @internal prvTgfExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                           - device number
* @param[in] entryKeyPtr                      - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]     - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id'
*                                               size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*/
GT_STATUS prvTgfExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_EXACT_MATCH_KEY_STC     *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                          exactMatchCrcMultiHashArr[]
);

/**
* @internal prvTgfExactMatchCheck function
* @endinternal
*
* @brief   check if the device supports Exact Match
*
* @retval GT_TRUE         - the device supports Exact Match
* @retval GT_FALSE        - the device not supports Exact Match
*/
GT_BOOL prvTgfExactMatchCheck(
    void
);

/**
* @internal prvTgfExactMatchEntryCompare function
* @endinternal
*
* @brief   compare 2 exact match entries (key+lookup).
*
* @param[in] entry1Ptr                  - (pointer to) entry 1
* @param[in] entry2Ptr                  - (pointer to) entry 2
*                                      OUTPUTS:
*                                      > 0  - if entry1Ptr is  bigger than entry2Ptr
*                                      == 0 - if entry1Ptr is equal to entry2Ptr
*                                      < 0  - if entry1Ptr is smaller than entry2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
int prvTgfExactMatchEntryCompare
(
    IN  PRV_TGF_EXACT_MATCH_ENTRY_STC    *entry1Ptr,
    IN  PRV_TGF_EXACT_MATCH_ENTRY_STC    *entry2Ptr
);

/**
* @internal prvTgfExactMatchEntryIndexFind_MultiHash function
* @endinternal
*
* @brief  for CRC multi hash mode. function calculate hash index
*         for this key , and then start to read from HW
*         the entries to find an existing entry that match the key.
* @param[in] entryPtr              - (pointer to) exact match entry(key+lookup)
*
* @param[out] indexPtr             - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index first index
*                                      on other return value --> not relevant
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfExactMatchEntryIndexFind_MultiHash
(
    IN  PRV_TGF_EXACT_MATCH_ENTRY_STC *entryPtr,
    OUT GT_U32                        *indexPtr
);
/**
* @internal
*           prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry
*           function
* @endinternal
*
* @brief   Convert generic exact match into device specific
*          exact match
*
* @param[in]  entryPtr     - (pointer to) exact match entry
* @param[out] dxChEntryPtr - (pointer to) dxCh exact match entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry
(
    IN  PRV_TGF_EXACT_MATCH_ENTRY_STC       *entryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC     *dxChEntryPtr
);
/**
* @internal
*           prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry
*           function
* @endinternal
*
* @brief   Convert specific exact match into device generic
*          exact match
*
* @param[in]  dxChEntryPtr - (pointer to) dxCh exact match entry
* @param[out] entryPtr     - (pointer to) exact match entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry
(
    IN  CPSS_DXCH_EXACT_MATCH_ENTRY_STC   *dxChEntryPtr,
    OUT PRV_TGF_EXACT_MATCH_ENTRY_STC     *entryPtr
);

/**
* @internal prvTgfConvertGenericUseExpandedToDxChUseExpanded
*           function
* @endinternal
*
* @brief   Convert generic Expander flags into device specific
*          Expander flags
*
* @param[in] actionType       - TTI/PCL/EPCL
* @param[in] expandedActionOriginPtr       - (pointer to) expander flags
* @param[out] dxChExpandedActionOriginPtr  - (pointer to) dxCh expander flags
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericUseExpandedToDxChUseExpanded
(
    IN  PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT                 actionType,
    IN  PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT      *expandedActionOriginPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *dxChExpandedActionOriginPtr
);

/**
* @internal prvTgfConvertGenericToDxChExactMatchKeyParams
*           function
* @endinternal
*
* @brief   Convert generic Exact Match key params into
*          device specific Exact Match key params
*
* @param[in] keyParamsPtr       - (pointer to) Exact Match
*       key params
* @param[out] dxChKeyParamsPtr  - (pointer to) DxCh Exact Match
*       key params
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChExactMatchKeyParams
(
    IN  PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      *keyParamsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *dxChKeyParamsPtr
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __tgfExactMatchGenh */

