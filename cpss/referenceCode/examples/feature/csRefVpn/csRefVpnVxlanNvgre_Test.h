/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*   csRefVpnVxlanNvgre_Test.c
*
* DESCRIPTION:
*  This file contains test scenarios for the vxLan, using the APIs in vpnVxlanNvgre.h
*
*         Test-1
*           1.1. configure UNI interface on physical port
*           1.2. configure NNI on different physical port
*           1.3. Send Ethernet packet, from TG to UNI port
*                 Verify packet egress NNI port, with IPv6 vxLAN encapsulation
*           1.4. Send IPv6 vxLan packet encapsulating Ethernet packet, from TG to NNI port.
*                 Verify packet egress UNI port, after removing vxLAN encapsulation.
*
*         Test-2
*           2.1. Same UNI NNI configuration as in test 2..
*           2.2. Addtional configuration for triggering router:
*                             - By default LPM-DB created, VRF created and routing enabled globally.
*                             - Enable routing on UNI port, NNI port and VSI.
*                             - Configure MAC2ME
*           2.3. Send Ethernet packet, from TG to UNI port
*                 Verify packet egress NNI port, with IPv6 vxLAN encapsulation
*           2.4. Send IPv6 vxLan packet encapsulating Ethernet packet, from TG to NNI port.
*                 Verify packet egress UNI port, after removing vxLAN encapsulation.
*
*         Test-3....
*         Test-4....
*         Test-5....
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>

/***********************************************/
/*         Type Definitions  */
/***********************************************/

/**
* @struct CSREF_TEST_UNI_ATTRIBUTE_STC
 *
 * @brief Structure with parameters which are used for
 * Ingress Uni classification, and Egress Uni setting.
*/ 
typedef struct{
  GT_PORT_NUM     portNum;       /* Physical port, on which to define the interface. */
  GT_U16          vlanId;        /* Ingress packet's vlan id. */
  GT_ETHERADDR    srcMacAddress; /* packet's source MAC address. */
}CSREF_TEST_UNI_ATTRIBUTE_STC;


/**
* @struct CSREF_TEST_UNI_ATTRIBUTE_STC
 *
 * @brief Structure with parameters which are used for
 * Ingress Nni classification, and Egress Nni setting.
*/ 
typedef struct{
  GT_PORT_NUM     portNum;             /* Physical port, on which to define the interface. */
  GT_U8           localIpAddress[16];  /* 16 bytes array of local devcie IP address. */
  GT_U8           remoteIpAddress[16]; /* 16 bytes array of remote device IP address. */
  GT_ETHERADDR    nhMacAddress;        /* MAC address of next attached device. DA of encasulated packets */
}CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC;

/***********************************************/
/*         Function Definitions                */
/***********************************************/

/**
* @internal csRefVpnVxLanTestConfigure function
* @endinternal
*
* @brief   This test function uses the Vxlan API to configure the device for the various of test scenario.
*            Phase1: Configure one NNI and one UNI in one segment L2-VPN
*            Phase2: Add NNI and UNI and check flooding behavior.
*            Phase3: Same configuration in phase1, different segment id and different port.
*                        Show that routing/bridging is only within the segment
*            Phase4: Routing of vxLAN passenger packet after TT.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] phaseNumber           - test phase number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanConfig
(
  IN GT_U8         devNum,
  IN GT_PORT_NUM   phaseNumber
);

/**
* @internal csRefVpnVxLanConfigClear function
* @endinternal
*
* @brief   This function clean all configuration done by csRefVpnVxLanTestConfigure
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum     - device number
* @param[in] resetDb    - Reset DB or not
*                         For robustness purposes
*                         don't reset DB between phases
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanConfigClear
(
  IN GT_U8         devNum,
  IN GT_BOOL       resetDb 
);

/**
* @internal csRefVpnVxLanTestInfoSet function
* @endinternal
*
* @brief   This test function override the default vxLan setting.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                        - device number
* @param[in] domainVsiId                   - domain VSI id
* @param[in] mac2meAddressPtr              - pointer to man2me
* @param[in] uniAttributeOvveridePtr       - pointer to first elements in uni attribute array
* @param[in] numberOfUniAttributeEelements - number of elements in uni array
* @param[in] nniAttributeOvveridePtr       - pointer to first elements in nni attribute array
* @param[in] numberOfNniAttributeEelements - number of elements in nni array 
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM                  - wrong value in any of the parameter 
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanTestInfoSet
(
  IN GT_U8                               devNum,
  IN GT_U32                              domainVsiId,
  IN GT_ETHERADDR                       *mac2meAddressPtr,
  IN CSREF_TEST_UNI_ATTRIBUTE_STC       *uniAttributeOvveridePtr,
  IN GT_U32                              numberOfUniAttributeEelements,
  IN CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC  *nniAttributeOvveridePtr,
  IN GT_U32                              numberOfNniAttributeEelements  
);
