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
* @file snetCheetah3EPcl.h
*
* @brief Cheetah3 Asic Simulation .
* Egress Policy Engine processing for outgoing frame.
* header file.
*
* @version   11
********************************************************************************
*/
#ifndef __snetCheetah3EPclh
#define __snetCheetah3EPclh

#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Pcl.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2EPcl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
    enum :  CHT3_EPCL_KEY_FIELDS_ID_ENT

    description : enum of the egress PCL TCAM fields

*/

/*
Key#        Key Name
0        Standard (24B) L2
1        Standard (24B) L2+IPv4/v6 QoS
2        Standard (24B) IPv4+L4
3        Extended (48B) L2+IPv4+L4
4        Extended (48B) L2+IPv6
5        Extended (48B) L4+IPv6
6        Extended (48B) RACL/VACL IPv4
7        Ultra    (72B) RACL/VACL IPv6

NOTE: the SIP5 devices uses the info from : LION3_EPCL_KEY_FIELDS_ID_ENT
*/

typedef enum{
    CHT3_EPCL_KEY_FIELDS_ID_VALID_E,
    CHT3_EPCL_KEY_FIELDS_ID_PCL_ID_E,
/* RACL/VACL (48B) IPv4 - Egress NonIP or IPv4/ARP */
    CHT3_EPCL_KEY_FIELDS_ID_PCL_ID_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_QOS_PROFILE_6_0_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_ORIG_VID_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_SRC_DEV_TRUNK_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_SRC_IS_TRUNK_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_MAC_SA_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_MAC_DA_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_L2_VALID_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_IP_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IP_PROTOCOL_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_DSCP_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_TOS_1_0_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_TTL_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_ARP_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_SIP_31_0_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_31_0_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_TCP_UDP_PORT_COMPS_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTES_OFFSETS_0_1_13_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTE_OFFSET_2_3_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTE_OFFSET_4_5_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_L4_VALID_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_TC_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_DP_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_PACKET_TYPE_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_SRC_TRG_TX_MIRR_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_ASSIGNED_UP_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_TRG_PORT_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_RX_SNIF_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_ROUTED_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IP_VER_EXT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IPV4_OPT_EXT_E,
/* RACL/VACL (72B) IPv6 - Egress */
    CHT3_EPCL_KEY_FIELDS_ID_IPV6_EH_TRIPLE_E,
    CHT3_EPCL_KEY_FIELDS_ID_IPV6_IS_ND_TRIPLE_E,
    CHT3_EPCL_KEY_FIELDS_ID_IPV6_EH_HOP_TRIPLE_E,
    CHT3_EPCL_KEY_FIELDS_ID_SIP_127_32_TRIPLE_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_127_32_TRIPLE_E,
/* End of RACL/VACL (72B) IPv6 - Egress */
    CHT3_EPCL_KEY_FIELDS_ID_SRC_PORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_TAGGED_E,
    CHT3_EPCL_KEY_FIELDS_ID_VID_E,
    CHT3_EPCL_KEY_FIELDS_ID_UP_E,
    CHT3_EPCL_KEY_FIELDS_ID_QOS_PROFILE_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_IPV4_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_IP_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_42_XCAT2_41_E,
    CHT3_EPCL_KEY_FIELDS_ID_ETHER_TYPE_DSAP_SSAP_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_66_59_XCAT2_65_58_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_72_67_XCAT2_71_66_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_ARP_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_74_XCAT2_73_E,
    CHT3_EPCL_KEY_FIELDS_ID_SRC_IS_TRUNK_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_ROUTED,
    CHT3_EPCL_KEY_FIELDS_ID_IS_L2_VALID_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_ENCAP_TYPE_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_MAC_SA_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_MAC_DA_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_PACKET_TYPE_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_190_XCAT2_189_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_VIDX_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IP_PROTOCOL_E,
    CHT3_EPCL_KEY_FIELDS_ID_DSCP_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_L4_VALID_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTE_OFFSET_2_3_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTE_OFFSET_4_5_E,
    CHT3_EPCL_KEY_FIELDS_ID_TCP_UDP_PORT_COMPS_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_31_0_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTES_OFFSETS_13_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_139_XCAT2_138_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IPV4_FRAGMENTED_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_BC_E,
    CHT3_EPCL_KEY_FIELDS_ID_SIP_31_0_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_31_0_IPV4_L4_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTES_OFFSETS_0_1_13_SHORT_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_187_XCAT2_186_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_IPV6_E,
    CHT3_EPCL_KEY_FIELDS_ID_L4_BYTES_OFFSETS_0_1_13_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_SIP_31_0_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_31_0_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_ENCAP_TYPE_LONG_VALID_E,
    CHT3_EPCL_KEY_FIELDS_ID_ETHER_TYPE_DSAP_SSAP_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_TCP_UDP_PORT_COMPS_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_SIP_127_32_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_IPV6_EH_HOP_BY_HOP_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_127_120_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_MAC_SA_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_MAC_DA_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_380_379_XCAT2_394_393_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_IPV6_EH_EXISTS_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_119_32_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_DIP_0_31_IPV6_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_L2_VALID_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_VIDX_LONG_E,
    CHT3_EPCL_KEY_FIELDS_ID_CPU_CODE_0_6_STANDARD_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_39_33_XCAT2_38_32_E,
    CHT3_EPCL_KEY_FIELDS_ID_TRUNK_ID,
    CHT3_EPCL_KEY_FIELDS_ID_SRCDEV_ID,
    CHT3_EPCL_KEY_FIELDS_ID_SST_ID_E,
    CHT3_EPCL_KEY_FIELDS_ID_IS_UNKNOWN_UC_E,
    CHT3_EPCL_KEY_FIELDS_ID_CPU_CODE_7_STANDARD_E,
    CHT3_EPCL_KEY_FIELDS_ID_LONG_IPV4_FRAGMENT_E,
    CHT3_EPCL_KEY_FIELDS_ID_RESERVED_89_XCAT2_88_E,
    CHT3_EPCL_KEY_FIELDS_ID_PACKET_TYPE_LONG_E,
    XCAT2_EPCL_KEY_FIELDS_ID_SIP_63_32_LONG_E,
    XCAT2_EPCL_KEY_FIELDS_ID_SIP_94_64_LONG_E,
    XCAT2_EPCL_KEY_FIELDS_ID_SIP_127_95_LONG_E,
    XCAT2_EPCL_KEY_FIELDS_ID_PORT_LIST_7_0_E,
    XCAT2_EPCL_KEY_FIELDS_ID_PORT_LIST_13_8_E,
    XCAT2_EPCL_KEY_FIELDS_ID_PORT_LIST_27_14_E,
    XCAT2_EPCL_KEY_FIELDS_ID_RESERVED_38_E,

    /* Lion2 : keys 6,7 with split fields */
    /*         and are not like stated in the FS !!!! */
    LION2_EPCL_KEY_FIELDS_ID_KEY_6_7_SIP_7_0_EXT_E                         ,
    LION2_EPCL_KEY_FIELDS_ID_KEY_6_7_SIP_31_8_EXT_E                        ,
    LION2_EPCL_KEY_FIELDS_ID_KEY_6_IS_VIDX_EXT_E                           ,

    LION2_EPCL_KEY_FIELDS_ID_KEY_7_SIP_95_32_TRIPLE_E                      ,
    LION2_EPCL_KEY_FIELDS_ID_KEY_7_SIP_127_96_TRIPLE_E                     ,
    LION2_EPCL_KEY_FIELDS_ID_KEY_7_DIP_127_32_TRIPLE_E                     ,
    LION2_EPCL_KEY_FIELDS_ID_KEY_7_IS_VIDX_EXT_E                           ,


    CHT3_EPCL_KEY_FIELDS_ID_LAST_E
}CHT3_EPCL_KEY_FIELDS_ID_ENT;

/*
    enum :  SKERNEL_CH2_EPCL_PACKET_CMD_ENT

    description : enum of the egress forward command
    C.12.12 - Policy Action Entry. page 794

*/
typedef enum{
    SKERNEL_CHT3_EPCL_CMD_FORWARD_E = 0,
    SKERNEL_CHT3_EPCL_CMD_HARD_DROP = 3
}SKERNEL_CHT3_EPCL_PACKET_CMD_ENT;


/*
    struct : SNET_CHT3_EPCL_ACTION_STC

    description : The policy action table is accessed of both ingress and
                  egress PCL. The line index of the matching rule is used
                  to index the policy action table and extract the action
                  to perform.

    matchCounterIndex - A pointer to one of the 32 policy rules match counters.
    matchCounterEn    - enables the binding of this policy action entry to
                        the Policy Rule Match Counters<n>
    pceModifyUp       - enables the modification of the 802.1p User Priority field
                        to <epceUp> of packet transmitted tagged.
    pceModifyDscp     - Enables the modification of the IP Header dscp field to
                        <epceDscp> of packet transmitted tagged.
    epceUp            - The UP value.
    epceDscp          - The DSCP value.
    fwdCmd            - 0 - Forward , 1-2 Reserved , 3 - Hard Drop , 4-7 Reserved
    pceVid            - The VID to be set to transmitted
    pceVlanCmd        - Enable modification of transmitted VLAN Tag to PCE_VID
*/
typedef SNET_CHT2_EPCL_ACTION_STC SNET_CHT3_EPCL_ACTION_STC;

/**
* @internal snetCht3EPcl function
* @endinternal
*
* @brief   Egress Policy Engine processing for outgoing frame on Cheetah3
*         asic simulation.
*         ePCL processing , ePcl assignment ,key forming , 1 Lookup ,
*         actions to descriptor processing
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
GT_VOID snetCht3EPcl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN  GT_U32  port
);

/**
* @internal snetCht3EPclActionGet function
* @endinternal
*
* @brief   Get the action entry from the policy action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndex               - index to the action table .
*
* @note C.12.12 - The policy Engine maintains an 1024 entries table, corresponding
*       to the 1024 rules that may be defined in the TCAM lookup
*       structure.
*
*/
GT_VOID snetCht3EPclActionGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr  ,
    IN GT_U32                             matchIndex,
    OUT SNET_CHT3_EPCL_ACTION_STC       * actionDataPtr
);

/**
* @internal snetCht3EPclActionApply function
* @endinternal
*
* @brief   Apply the action entry from the policy action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr   - pointer to the ePCL action entry.
*
* @note C.12.14 - Egress PCL action table and Policy TCAM access control
*       Registers
*
*/
GT_VOID snetCht3EPclActionApply
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_CHT3_EPCL_ACTION_STC            * actionDataPtr
);

/**
* @internal snetCht3EPclEgressVlanTranslate function
* @endinternal
*
* @brief   Egress VLAN Translation logic.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] port                     - egress port.
*/
GT_VOID snetCht3EPclEgressVlanTranslate
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN GT_U32                               port
);

/**
* @internal snetCht3EPclTriggeringCheck function
* @endinternal
*
* @brief   Check the enabling status of egress PCL for the outgoing frame.
*         Get the epcl-id configuration setup.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] egressPort               - local egress port (not global).
*
* @param[out] doLookupPtr              - pointer to enabling the PCL engine.
* @param[out] lookupConfPtr            - pointer to lookup configuration.
*                                      RETURN:
*                                      COMMENTS:
*                                      1. Check is EPCL globally enabled.
*                                      2. Check is EPCL enabled per port .
*                                      3. Get Egress PCL configuration entry.
*
* @note 1. Check is EPCL globally enabled.
*       2. Check is EPCL enabled per port .
*       3. Get Egress PCL configuration entry.
*
*/
GT_VOID snetCht3EPclTriggeringCheck
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN GT_U32                               egressPort,
    OUT GT_U8                            *  doLookupPtr,
    OUT SNET_CHT3_PCL_LOOKUP_CONFIG_STC  *  lookupConfPtr
);

/**
* @internal snetCht3EPclCreateKey function
* @endinternal
*
* @brief   Create Egress PCL search tram key.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
* @param[in] egressPort               - local egress port (not global).
*                                      lookupConfigPtr - pointer to the EPCL configuration table entry.
* @param[in] pclExtraDataPtr          - pointer to extra data needed for the EPCL engine.
* @param[in] keyType                  - egress pcl tcam key type.
*                                      OUTPUT:
* @param[in] pclKeyPtr                - pointer to EPCL key structure.
*                                      RETURN:
*/
GT_VOID snetCht3EPclCreateKey
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN  GT_U32                                egressPort,
    IN SNET_CHT3_PCL_LOOKUP_CONFIG_STC      * lookupConfPtr,
    IN CHT_PCL_EXTRA_PACKET_INFO_STC        * pclExtraDataPtr,
    IN GT_U32                                 keyType,
    OUT SNET_CHT_POLICY_KEY_STC             * pclKeyPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetah3EPclh */



