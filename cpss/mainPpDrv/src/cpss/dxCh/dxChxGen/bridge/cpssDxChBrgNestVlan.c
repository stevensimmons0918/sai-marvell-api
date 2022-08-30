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
* @file cpssDxChBrgNestVlan.c
*
* @brief CPSS DxCh Nested VLANs facility implementation.
*
* @version   17
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal internal_cpssDxChBrgNestVlanAccessPortSet function
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
static GT_STATUS internal_cpssDxChBrgNestVlanAccessPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      hwData;      /* data to write to HW */
    GT_U32      fieldOffset; /* offset of the field in the VLAN QoS Table */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Convert boolean value to the HW format */
    hwData = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            fieldOffset = 28;
        }
        else
        {
            fieldOffset = 24;
        }

        /* Update port mode in the VLAN and QoS Configuration Entry */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                               (GT_U32)portNum,
                                               0,       /* start at word 0 */
                                               fieldOffset, /* start at bit 24 or 28 */
                                               1,       /* 1 bit */
                                               hwData);
    }
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgNestVlanAccessPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgNestVlanAccessPortSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgNestVlanAccessPortGet function
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
static GT_STATUS internal_cpssDxChBrgNestVlanAccessPortGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      hwData;             /* data to write to HW */
    GT_STATUS   retStatus = GT_OK;  /* generic return status code */
    GT_U32      fieldOffset;        /* offset of the field in the VLAN QoS Table */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table  */
        retStatus = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            fieldOffset = 28;
        }
        else
        {
            fieldOffset = 24;
        }
        /* Read port mode from VLAN and QoS Configuration Entry */
        retStatus = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             (GT_U32)portNum,
                                             0,        /* start at word 0 */
                                             fieldOffset, /* start at bit 24 or 28 */
                                             1,        /* 1 bit */
                                             &hwData);
    }
    if(GT_OK == retStatus)
    {
        /* Convert gathered HW value to the SW format */
        *enablePtr = (hwData == 1) ? GT_TRUE : GT_FALSE;
    }

    return retStatus;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgNestVlanAccessPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgNestVlanAccessPortGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgNestVlanPortTargetEnableSet function
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
* @param[in] enable            -  GT_TRUE - to enable Nested VLAN per target port.
*                              -  GT_FALSE - to diable Nested VLAN per target port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgNestVlanPortTargetEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   BOOL2BIT_MAC(enable));
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgNestVlanPortTargetEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgNestVlanPortTargetEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgNestVlanPortTargetEnableGet function
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
static GT_STATUS internal_cpssDxChBrgNestVlanPortTargetEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgNestVlanPortTargetEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgNestVlanPortTargetEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgNestVlanPortTargetUseIngressModeSet function
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
static GT_STATUS internal_cpssDxChBrgNestVlanPortTargetUseIngressModeSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT mode
)
{
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);

    switch(mode)
    {
        case CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ALLOWED_E:
            hwValue = 0x0;
            break;
        case CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_IGNORE_FORCE_UNTAGGED_E:
            hwValue = 0x1;
            break;
        case CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_IGNORE_E:
            hwValue = 0x2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mode);
    }

    /* write to HA eport 1 table */
    return prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgNestVlanPortTargetUseIngressModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , mode ));

    rc = internal_cpssDxChBrgNestVlanPortTargetUseIngressModeSet(devNum, portNum , mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgNestVlanPortTargetUseIngressModeGet function
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
static GT_STATUS internal_cpssDxChBrgNestVlanPortTargetUseIngressModeGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* read from HA eport 1 table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwValue)
    {
        case 0x0:
            *modePtr = CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ALLOWED_E;
            break;
        case 0x1:
            *modePtr = CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_IGNORE_FORCE_UNTAGGED_E;
            break;
        case 0x2:
            *modePtr = CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_IGNORE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"unknown NESTED_VLAN_MODE [%d] in the register",
                hwValue);
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgNestVlanPortTargetUseIngressModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , modePtr ));

    rc = internal_cpssDxChBrgNestVlanPortTargetUseIngressModeGet(devNum, portNum , modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

