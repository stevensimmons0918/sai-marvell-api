/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* vpnVxlanNvgre.h
*
* DESCRIPTION:
*  This files provide APIs to create VxLAN UNI and NNI interfaces.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef __vpnVxlanNvgre_h


#include <cpss/generic/cpssTypes.h>

/***********************************************/
/*         Function Definitions                */
/***********************************************/

/**
* @internal csRefVpnMac2MeSet function
* @endinternal
*
* @brief   Set mac2me address entry.
*          Mac2me is one of the condition to trigger VxLAN packet processing
*          ingress UNI or NNI. This MAC address is also the source MAC address
*          of packet that sent to core after encapsulation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum   - device number
* @param[in] mac2me   - Mac2me to trigger VxLAN packet processing
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnMac2MeSet
(
  IN GT_U8          devNum,
  IN GT_ETHERADDR  *mac2me
);


/**
* @internal csRefVpnMac2MeDelete function
* @endinternal
*
* @brief   Delete mac2me address entry.
*            Mac2me is one of the condition to trigger VxLAN packet processing ingress UNI or NNI.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] assignEportNum   - ePort to assigned the NNI
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnMac2MeDelete
(
  IN GT_U8          devNum
);

/**
* @internal csRefVpnUniInterfaceCreate function
* @endinternal
*
* @brief   The function creates a User Network Interface (UNI).
*            Both UNI direction are configured - ingress (to device)
*            and egress (from device) processing.
*            Ingress: Classify packet. Assign ingress ePort and VNI
*            Egress:  forward packet (after tunnel termination),
*                     with its attributes - tagged/untagged etc.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum          - device number
* @param[in] portNum         - port number
* @param[in] vlanId          - packet assigned vlan id.
* @param[in] macAddress      - Packet's source mac address.
*                              if macAddress is all 0, ignore it.
* @param[in] serviceId       - Segment id, VSI/VNI/TNI.
* @param[in] assignedEVlan   - eVlan to represent the VSI domain.
* @param[in] assignEportNum  - ePort to assigned the UNI
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_OUT_OF_RANGE           - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnUniInterfaceCreate
(
  IN GT_U8         devNum,
  IN GT_PORT_NUM   portNum,
  IN GT_U16        vlanId,
  IN GT_ETHERADDR *macAddress,
  IN GT_U32        serviceId,
  IN GT_U16        assignedEVlan,
  IN GT_PORT_NUM   assignEportNum
);


/**
* @internal csRefVpnVxlanNniInterfaceCreate function
* @endinternal
*
* @brief   The function creates a VxLan Network Network Interface (NNI).
*            Both NNI direction are configured - ingress (to device)
*            and egress (from device) processing.
*            Ingress: Packet classification, Tunnel Termination, ingress ePort and eVlan assignment.
*            Egress:  Add tunnel encapsulation according to ePort, and forward packet to core network.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum            - device number
* @param[in] portNum           - port number
* @param[in] localIpAddress    - Array of local device IP address.
*                                It is the SIP in ingress packet
*                                It is the DIP in egress packet
* @param[in] remoteIpAddress   - Array of remote device IP address.
*                                It is the DIP in ingress packet
*                                It is the SIP in egress packet
* @param[in] ipProt            - VPN Ip protocol IPv4 or IPv6
* @param[in] serviceId         - Segment id, VSI/VNI/TNI.
* @param[in] assignedEVlan     - eVlan to represent the VSI domain.
* @param[in] assignEportNum    - ePort to assigned the NNI
* @param[in] innerPacketTagged - Encapsulated packet egress with tagged or not.
* @param[in] nhMacAddress      - destination mac address of egress traffic 
*                                through assigned assignEportNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxlanNniInterfaceCreate
(
  IN GT_U8                       devNum,
  IN GT_PORT_NUM                 portNum,
  IN GT_U8                      *localIpAddress,
  IN GT_U8                      *remoteIpAddress,
  IN CPSS_IP_PROTOCOL_STACK_ENT  ipProt,
  IN GT_U32                      serviceId,
  IN GT_U16                      assignedEVlan,
  IN GT_PORT_NUM                 assignEportNum,
  IN GT_BOOL                     innerPacketTagged,
  IN GT_ETHERADDR               *nhMacAddress
);



/**
* @internal csRefVpnNvgreNniInterfaceCreate function
* @endinternal
*
* @brief   The function creates an NVGRE Network Network Interface (NNI).
*            Both UNI direction are configured - ingress (to device)
*            Ingress: Classify packet. Tunnel Terminate packet, Assign ingress ePort and VNI
*            Egress:  Add tunnel encapsulation according to ePort, forward packet to core network.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum              - device number
* @param[in] portNum             - port number
* @param[in] localIpAddressU32   - IPv4 address of local device.
*                                  It is the SIP in ingress packet
*                                  It is the DIP in egress packet
*
* @param[in] remoteIpAddressU32  - IPv4 address of remote device.
*                                  It is the DIP in ingress packet
*                                  It is the SIP in egress packet
* @param[in] ipProt              - VPN Ip protocol IPv4 or IPv6
* @param[in] serviceId           - Segment id, VSI/VNI/TNI.
* @param[in] assignedEVlan       - eVlan to represent the VSI domain.
* @param[in] assignEportNum      - ePort to assigned the UNI
* @param[in] macAddress          - destination mac address of egress traffic through assigned assignEportNum
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnNvgreNniInterfaceCreate
(
  IN GT_U8                        devNum,
  IN GT_PORT_NUM                  portNum,
  IN GT_U32                       localIpaddrU32,
  IN GT_U32                       remoteIpaddrU32,
  IN CPSS_IP_PROTOCOL_STACK_ENT   ipProt,
  IN GT_U32	                      serviceId,
  IN GT_U16                       assignedEVlan,
  IN GT_PORT_NUM                  assignEportNum,
  IN GT_ETHERADDR                *macAddress
);

/**
* @internal csRefVpnInterfaceDelete function
* @endinternal
*
* @brief   The function clean all VPN related configuration that done by
*          the UNI and NNI create APIs, and remove it from DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] assignEportNum   - ePort to assigned the NNI or UNI
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnInterfaceDelete
(
  IN GT_U8         devNum,
  IN GT_PORT_NUM   assignEportNum
);

#endif
