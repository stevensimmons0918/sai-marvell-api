/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvAppIpFix.h
*
* @brief App demo IpFix header file.
*
* @version   1
********************************************************************************
*/
#ifndef __prvAppIpFixh
#define __prvAppIpFixh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>

/**
* @internal appDemoIpfixIpv4TcpFlowFieldConfig function
* @endinternal
*
* @brief Configures flow classification fields for IPV4-TCP packet.
*
* @param[in] flowMngId               - unique flow manager id
* @param[in] enable                  - enable/disable
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixIpv4TcpFlowFieldConfig
(
    IN  GT_U32 flowMngId,
    IN GT_BOOL enable
);

/**
* @internal appDemoIpfixIpv4UdpFlowFieldConfig function
* @endinternal
*
* @brief Configures flow classification fields for IPV4-UDP packet.
*
* @param[in] flowMngId               - unique flow manager id
* @param[in] enable                  - enable/disable
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixIpv4UdpFlowFieldConfig
(
    IN GT_U32  flowMngId,
    IN GT_BOOL enable
);

/**
* @internal appDemoIpfixOtherFlowFieldConfig function
* @endinternal
*
* @brief Configures flow classification fields for OTHER packet.
*
* @param[in] flowMngId               - unique flow manager id
* @param[in] enable                  - enable/disable
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixOtherFlowFieldConfig
(
    IN GT_U32 flowMngId,
    IN GT_BOOL enable
);

/**
* @internal appDemoIpfixPortEnableSet function
* @endinternal
*
* @brief Configures IPFIX flow classification per port.
*
* @param[in] flowMngId               - unique flow manager id
* @param[in] devNum                  - unique device number
* @param[in] portNum                 - device port number
* @param[in] enable                  - enable/disable
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixPortEnableSet
(
    IN  GT_U32       flowMngId,
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    IN  GT_BOOL      enable
);

/**
* @internal appDemoIpfixPacketTypeEnableSet function
* @endinternal
*
* @brief Enable IPFIX flow classification based on packet type.
*
* @param[in] devNum                  - unique device number
* @param[in] pktType                 - IPFIX flow type
* @param[in] enable                  - enable/disable
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixPacketTypeEnableSet
(
    IN  GT_U32                                 flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT pktType,
    IN  GT_BOOL                                enable
);

/**
* @enum APP_DEMO_IPFIX_CONFIG_PARAM_ENT
 *
 * @brief Config cpssEnabler params as per test requirement.
*/
typedef enum{

    /** @brief Key Table index config for flow field classification.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_KEY_TABLE_INDEX_E,

    /** @brief UDB index for flow fields.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_UDB_INDEX_E,

    /** @brief OffsetOrPattern for udb indexes.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_ANCHOR_TYPE_E,

    /** @brief Offset value for udb indexes.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_OFFSET_E,

    /** @brief Mask for udb indexes.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_MASK_E,

    /** @brief Source port to bind flow classification.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_PORT_NUM_E,

    /** @brief Port bitmap to denote enable/disable of flow classification per port.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_ENABLE_BITMAP_E,

    /** @brief Packet type Ipv4 Tcp key table index.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_IPV4_TCP_KEY_TABLE_INDEX_E,

    /** @brief Packet type Ipv4 Udp key table index.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_IPV4_UDP_KEY_TABLE_INDEX_E,

    /** @brief Packet type other key table index.
     */
    APP_DEMO_IPFIX_CONFIG_PARAM_OTHER_KEY_TABLE_INDEX_E

} APP_DEMO_IPFIX_CONFIG_PARAM_ENT;

/**
* @internal appDemoIpfixConfigParamSet function
* @endinternal
*
* @brief Setting configuration parameters as per test requirements.
*
* @param[in] flowMngId                  - unique floe manager id
* @param[in] paramId                    - paramter id to be changed
* @param[in] paramValue                 - parameter value to be used
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS appDemoIpfixConfigParamSet
(
    IN  GT_U32                          flowMngId,
    IN  APP_DEMO_IPFIX_CONFIG_PARAM_ENT paramId,
    IN  GT_U32                          paramValue
);

/**
* @internal appDemoIpfixConfigParamGet function
* @endinternal
*
* @brief To get the test parameter value.
*
* @param[in] flowMngId                  - unique floe manager id
* @param[in] paramId                    - paramter id to be changed
* @param[in] paramValuePtr              - (pointer to) retrieve the test parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixConfigParamGet
(
    IN  GT_U32                          flowMngId,
    IN  APP_DEMO_IPFIX_CONFIG_PARAM_ENT paramId,
    OUT GT_U32                          *paramValuePtr
);

/**
* @internal appDemoIpfixInit function
* @endinternal
*
* @brief Init the IPFIX flow classification.
*
* @param[in] flowMngId                  - unique floe manager id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixInit
(
    IN  GT_U32                                flowMngId
);

/**
* @internal appDemoIpfixDelete function
* @endinternal
*
* @brief De-Init the IPFIX flow classification.
*
* @param[in] flowMngId                  - unique floe manager id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoIpfixDelete
(
    IN  GT_U32                                flowMngId
);

/**
* @internal appDemoIpfixQinQEnableSet function
* @endinternal
*
* @brief Enable/disable QinQ for flows of different packet type.
*
* @param[in] enable                  - enable/disable
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoIpfixQinQEnableSet
(
    IN  GT_BOOL                                enableQinQ
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvAppIpFixh */

