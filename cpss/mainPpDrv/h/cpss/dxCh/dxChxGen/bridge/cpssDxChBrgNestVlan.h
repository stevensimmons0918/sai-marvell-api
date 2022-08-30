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
* @file cpssDxChBrgNestVlan.h
*
* @brief CPSS DxCh Nested VLANs facility implementation.
*
* @version   9
********************************************************************************
*/

#ifndef __cpssDxChBrgNestVlanh
#define __cpssDxChBrgNestVlanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>

/**
* @enum CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT
 *
 * @brief types of egress port treatment of ingress nested vlan decision.
 *
*/
typedef enum{

    /** @brief : type of egress port treatment of ingress nested vlan decision :
        Nested vlan by ingress is allowed. */
    CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ALLOWED_E,
    /** @brief : type of egress port treatment of ingress nested vlan decision :
        If Nested vlan by ingress then force egress untagged and ignored.
    */
    CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_IGNORE_FORCE_UNTAGGED_E,
    /** @brief : type of egress port treatment of ingress nested vlan decision :
        Ignore ingress. */
    CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_IGNORE_E

} CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT;

/**
* @internal cpssDxChBrgNestVlanAccessPortSet function
* @endinternal
*
* @brief   Configure given port as Nested VLAN access port.
*         The VID of all the packets received on Nested VLAN access port is
*         discarded and they are assigned with the Port VID that set by
*         cpssDxChBrgVlanPortVidSet(). This VLAN assignment may be subsequently
*         overridden by the protocol based VLANs or policy based VLANs algorithms.
*         When a packet received on an access port is transmitted via a core port
*         or a cascading port, a VLAN tag is added to the packet, in addition to
*         any existing VLAN tag.
*         The 802.1p User Priority field of this added tag may be one of the
*         following, based on the ModifyUP QoS parameter set to the packet at
*         the end of the Ingress pipe:
*         - If ModifyUP is "Modify", it is the UP extracted from the
*         QoS Profile table entry that configured by the
*         cpssDxChCosProfileEntrySet().
*         - If ModifyUP is "Not Modify", it is the original packet
*         802.1p User Priority field, if it is tagged. If untagged,
*         it is ingress port's default user priority that configured by
*         cpssDxChPortDefaultUPSet().
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - port number or CPU port number.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   -  GT_TRUE  - port is Nested VLAN Access Port.
*                                      GT_FALSE - port is usual port (Core or Customer).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To complete Access Port Configuration do the following:
*       For xCat3 and above devices:
*       See CPSS user guide how to configure Nested VLAN or TR101 features.
*
*/
GT_STATUS cpssDxChBrgNestVlanAccessPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssDxChBrgNestVlanAccessPortGet function
* @endinternal
*
* @brief   Gets configuration of the given port (Nested VLAN access port
*         or Core/Customer port).
*         The VID of all the packets received on Nested VLAN access port is
*         discarded and they are assigned with the Port VID that set by
*         cpssDxChBrgVlanPortVidSet(). This VLAN assignment may be subsequently
*         overridden by the protocol based VLANs or policy based VLANs algorithms.
*         When a packet received on an access port is transmitted via a core port
*         or a cascading port, a VLAN tag is added to the packet, in addition to
*         any existing VLAN tag.
*         The 802.1p User Priority field of this added tag may be one of the
*         following, based on the ModifyUP QoS parameter set to the packet at
*         the end of the Ingress pipe:
*         - If ModifyUP is "Modify", it is the UP extracted from the
*         QoS Profile table entry that configured by the
*         cpssDxChCosProfileEntrySet().
*         - If ModifyUP is "Not Modify", it is the original packet
*         802.1p User Priority field, if it is tagged. If untagged,
*         it is ingress port's default user priority that configured by
*         cpssDxChPortDefaultUPSet().
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - physical or CPU port number.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - Pointer to the Boolean value:
*                                      GT_TRUE   - port is Nested VLAN Access Port.
*                                      GT_FALSE  - port is usual port:
*                                      - Core Port in case of
*                                      CPSS_VLAN_ETHERTYPE1_E selected;
*                                      - Customers Bridges (bridges that don't
*                                      employ Nested VLANs) in case of
*                                      CPSS_VLAN_ETHERTYPE0_E selected;
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum values.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgNestVlanAccessPortGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgNestVlanPortTargetEnableSet function
* @endinternal
*
* @brief  Configure Nested VLAN per target port.
*         If set, "Nested VLAN Enable" causes the egress tag
*         modification to treat the packet as untagged, i.e.
*         neither Tag0-tagged nor Tag1- tagged.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum          - device number
* @param[in] portNum        -  port number
* @param[in] enable            -  GT_TRUE  - to enable Nested VLAN per target port.
*                              -  GT_FALSE - to diable Nested VLAN per target port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgNestVlanPortTargetEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChBrgNestVlanPortTargetEnableGet function
* @endinternal
*
* @brief   Gets the configuration of Nested VLAN per target port.
*          If set, "Nested VLAN Enable" causes the egress tag
*          modification to treat the packet as untagged, i.e.
*          neither Tag0-tagged nor Tag1- tagged .
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum         - device number
* @param[in] portNum        - port number
*
* @param[out] enablePtr     - (pointer to)
*                               GT_TRUE: Nested VLAN enabled on target port.
*                               GT_FALSE: Nested VLAN disabled on target port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgNestVlanPortTargetEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgNestVlanPortTargetUseIngressModeSet function
* @endinternal
*
* @brief  Set on egress port the mode about how to treat ingress nested vlan decision.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: eport range.
* @param[in] mode           - The mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgNestVlanPortTargetUseIngressModeSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT mode
);

/**
* @internal cpssDxChBrgNestVlanPortTargetUseIngressModeGet function
* @endinternal
*
* @brief  Get from egress port the mode about how to treat ingress nested vlan decision.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: eport range.
*
* @param[out] modePtr       - (pointer to) The mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgNestVlanPortTargetUseIngressModeGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT *modePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgNestVlanh */

