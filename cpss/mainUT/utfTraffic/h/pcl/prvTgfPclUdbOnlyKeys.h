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
* @file prvTgfPclUdbOnlyKeys.h
*
* @brief UDB Only Keys specific PCL features testing
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPclUdbOnlyKeysh
#define __prvTgfPclUdbOnlyKeysh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbSet
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] portNum                  - port Num
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] accessMode               - PCL CFG table access mode
* @param[in] cfgIndex                 - PCL CFG table entry index (if mode not "port")
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] pclId                    - PCL Id
* @param[in] ruleFormat               - rule Format
* @param[in] ruleIndex                - rule index
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
* @param[in] actionPtr                - (pointer to) Action
* @param[in] udb49BitmapEnable        - enable/disable bitmap of UDB49 values
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt
(
    IN GT_U32                                       portNum,
    IN CPSS_PCL_DIRECTION_ENT                       direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                   lookupNum,
    IN PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode,
    IN GT_U32                                       cfgIndex,
    IN GT_U32                                       pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN GT_U32                                       pclId,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT             ruleFormat,
    IN GT_U32                                       ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT                  *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT                  *patternPtr,
    IN PRV_TGF_PCL_ACTION_STC                       *actionPtr,
    IN GT_BOOL                                      udb49BitmapEnable
);

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] pclId                    - PCL Id
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
* @param[in] udb49BitmapEnable        - enable/disable bitmap of UDB49 values
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN GT_U32                           pclId,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN GT_BOOL                          udb49BitmapEnable
);

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr
);

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet function
* @endinternal
*
* @brief   Set UDB Only PCL Key UDB Range selection
*         Replaced UDB positions ommited.
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
* @param[in] udbReplaceBitmap         - bitmap of replace UDBs (12 bits ingress, 2 bits egress)
* @param[in] udbAmount                - udb Amount
* @param[in] udbIndexBase             - udb Index Base
*                                      udbOffsetIncrement  - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbReplaceBitmap,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexBase,
    IN GT_32                            udbIndexIncrement
);

/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet function
* @endinternal
*
* @brief   Set PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] offsetType               - offset Type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
* @param[in] udbOffsetBase            - udb Offset Base
* @param[in] udbOffsetIncrement       - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbOffsetBase,
    IN GT_32                            udbOffsetIncrement
);

/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbReset
(
    GT_VOID
);
/**
* @internal prvTgfPclUdbOnlyKeysVidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
GT_VOID prvTgfPclUdbOnlyKeysVidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
);

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgResetExt function
* @endinternal
*
* @brief   Restore Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
* @param[in] ruleIndex                - Rule Index
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgResetExt
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize,
    IN GT_U32                          ruleIndex
);

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgReset function
* @endinternal
*
* @brief   Restore Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize
);
/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset function
* @endinternal
*
* @brief   Reset PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount
);
/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset function
* @endinternal
*
* @brief   Reset UDB Only PCL Key UDB selection
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat
);
/**
* @internal prvTgfPclUdbOnlyKeysIngressMetadataTest function
* @endinternal
*
* @brief   Test on Ingress PCL metadata
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressMetadataTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgressMetadataTest function
* @endinternal
*
* @brief   Test on Egress PCL metadata
*
* @note Metadata[207:196] - Original VID
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressMetadataTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_SA
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL3Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L3 offset type
*
* @note Tunnel Header DIP[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL3Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_DA[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L3 Minus 2 offset type
*
* @note Tunnel Header SIP[31:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 UDP packet
*
* @note 10 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 MplsMinus2 offset type on Ethernet over MPLS packet
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet
*
* @note 40 first bytes from L2 header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 L3Minus2 offset type on IPV6 Other packet
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60 L3Minus2 offset type on IPV4 Other packet
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_NO_FIXED L3Minus2 offset type on IPV4 Other
*         packet
*
* @note 60 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 MplsMinus2 offset type on ETH over MPLS packet.
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB40 L2 offset type on ETH Other packet.
*
* @note 40 first bytes from L2 Header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB50 L3Minus2 offset type on IPV6 Other packet.
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 L3Minus2 offset type on Ethernet over MPLS packet
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 L3Minus2 offset type on ETH over MPLS packet.
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest function
* @endinternal
*
* @brief   Test Egress PCL on LLC Non SNAP packet.
*
* @note Used 10 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpReplacedPclIdTest function
* @endinternal
*
* @brief   Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 2 replaced bytes with PCL Id.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpReplacedPclIdTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressUdb20Ipv6UdpReplacedPclIdSrcTrgModeTest function
* @endinternal
*
* @brief   Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet in SRC-TRG mode 
*
* @note 2 replaced bytes with PCL Id.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressUdb20Ipv6UdpReplacedPclIdSrcTrgModeTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb50EthOtherUDB49Bitmap function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb50EthOtherUDB49Bitmap
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb60EthOtherUDB49Bitmap function
* @endinternal
*
* @brief   Test on Egress PCL UDB60 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb60EthOtherUDB49Bitmap
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB49Bitmap16BytesMode function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 SIP6 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
* @brief   Used 16-byte bitmap mode.
* @brief   Fixed from 60-byte key - test used 60-byte key with fixed fields.
* @brief   Falcon does not support it correct.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB49Bitmap16BytesMode
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB8Bitmap16BytesMode function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 SIP6 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
* @brief   Used 8-byte bitmap mode.
*/
GT_VOID prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB49Bitmap8BytesMode
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressUdb60FixedFields function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_with_fixed_fields fixed_fields.
*          Configurationm, traffic and check results
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsConfigurationRestore function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_with_fixed_fields fixed_fields.
*          Configuration restore.
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2Test function
* @endinternal
*
* @brief   PclId2 Test on UDB Only Key
*
*/
GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2Test(GT_VOID);

/**
* @internal tgfPclUdbSelectInfoGet function
* @endinternal
*
* @brief   get info relevant to UDB only keys
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleFormat               - rule Format (10/20/30..60 ingress/egress)
* @param[out] egressPtr               - (pointer to) is egress rule format
* @param[out] supportUdbSelectPtr     - (pointer to) is the lookup support Udb Select
* @param[out] sizePtr                 - (pointer to) the size of the key (10/20/30..60)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong ruleFormat
*/
GT_STATUS   tgfPclUdbSelectInfoGet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    OUT GT_BOOL     *egressPtr,
    OUT GT_BOOL     *supportUdbSelectPtr,
    OUT GT_U32      *sizePtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclUdbOnlyKeysh */


