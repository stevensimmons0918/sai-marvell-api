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
* @file tgfConfigGen.c
*
* @brief Generic Config API implementation
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfConfigGen.h>

#ifdef CHX_FAMILY

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

#endif

/* Convert PRV_TGF_CFG_GLOBAL_EPORT_CONFG_ENT to DXCH value */
#define PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(genericValue, dxChValue) \
    switch (genericValue)                                                              \
    {                                                                                  \
        case PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E:                                 \
            dxChValue = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;                    \
            break;                                                                     \
        case PRV_TGF_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E:                     \
            dxChValue = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;        \
            break;                                                                     \
        case PRV_TGF_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E:                            \
            dxChValue = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;               \
            break;                                                                     \
        default:                                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);             \
     }

/**
* @internal prvTgfCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalPtr                - (pointer to) Global EPorts for the Bridge,L2Mll to use.
*                                      In the Bridge Used for:
*                                      a) Whether the bridge engine looks at the device number
*                                      when performing MAC SA lookup or local switching check.
*                                      b) That the local device ID (own device) is learned in the
*                                      FDB entry with the global ePort
*                                      In the L2MLL Used for:
*                                      Enabling the L2MLL replication engine to ignore the
*                                      device ID when comparing source and target interfaces
*                                      for source filtering.
*                                      Typically configured to include global ePorts
*                                      representing a trunk or an ePort ECMP group
*                                      NOTE: For defining the EPorts that used for 'Multi-Target ePort to eVIDX Mapping'
*                                      use API cpssDxChL2MllMultiTargetPortSet(...)
*                                      (see cpssDxChL2Mll.h file for more related functions)
* @param[in] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*                                      Used for: The 'Primary ePorts' that need to be converted to
*                                      one of their 'Secondary ePorts'.
* @param[in] l2DlbPtr                 - (pointer to) L2 DLB Primary ePorts.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgGlobalEportSet
(
    IN GT_U8                          devNum,
    IN  PRV_TGF_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  PRV_TGF_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    IN  PRV_TGF_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  dxChEcmp;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  dxChGlobal;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  dxChDlb;

    /* Set global eport configuration mode for l2 ecmp */
    PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(l2EcmpPtr->enable, dxChEcmp.enable);
    if(dxChEcmp.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E)
    {
        dxChEcmp.pattern  = l2EcmpPtr->pattern;
        dxChEcmp.mask     = l2EcmpPtr->mask;
        dxChEcmp.minValue = 0x0;
        dxChEcmp.maxValue = 0x0;
    }
    else if(dxChEcmp.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        dxChEcmp.minValue = l2EcmpPtr->minValue;
        dxChEcmp.maxValue = l2EcmpPtr->maxValue;
        dxChEcmp.pattern = 0x0;
        dxChEcmp.mask    = 0x0;
    }
    else /* do not use values from the caller to avoid non-initialized parameters */
    {
        dxChEcmp.pattern = 0xFFFFFFFF;
        dxChEcmp.mask    = 0xFFFFFFFF;
        dxChEcmp.minValue = 0x0;
        dxChEcmp.maxValue = 0x0;
    }

    PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(globalPtr->enable, dxChGlobal.enable);
    if(dxChGlobal.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E)
    {
        dxChGlobal.pattern  = globalPtr->pattern;
        dxChGlobal.mask     = globalPtr->mask;
        dxChGlobal.minValue = 0x0;
        dxChGlobal.maxValue = 0x0;
    }
    else if(dxChGlobal.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        dxChGlobal.minValue = globalPtr->minValue;
        dxChGlobal.maxValue = globalPtr->maxValue;
        dxChGlobal.pattern  = 0x0;
        dxChGlobal.mask     = 0x0;
    }
    else /* do not use values from the caller to avoid non-initialized parameters */
    {
        dxChGlobal.pattern  = 0xFFFFFFFF;
        dxChGlobal.mask     = 0xFFFFFFFF;
        dxChGlobal.minValue = 0x0;
        dxChGlobal.maxValue = 0x0;
    }

    PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(l2DlbPtr->enable, dxChDlb.enable);
    if(dxChDlb.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E)
    {
        dxChDlb.pattern  = l2DlbPtr->pattern;
        dxChDlb.mask     = l2DlbPtr->mask;
        dxChDlb.minValue = 0x0;
        dxChDlb.maxValue = 0x0;
    }
    else if(dxChDlb.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        dxChDlb.minValue = l2DlbPtr->minValue;
        dxChDlb.maxValue = l2DlbPtr->maxValue;
        dxChDlb.pattern  = 0x0;
        dxChDlb.mask     = 0x0;
    }
    else /* do not use values from the caller to avoid non-initialized parameters */
    {
        dxChDlb.pattern  = 0xFFFFFFFF;
        dxChDlb.mask     = 0xFFFFFFFF;
        dxChDlb.minValue = 0x0;
        dxChDlb.maxValue = 0x0;
    }
    /* make sure test not over the limit */
    dxChEcmp.mask       &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);
    dxChEcmp.pattern    &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);
    dxChGlobal.mask     &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);
    dxChGlobal.pattern  &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);
    dxChDlb.mask        &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);
    dxChDlb.pattern     &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);

    /* call device specific API */
    rc = cpssDxChCfgGlobalEportSet(devNum, &dxChGlobal , &dxChEcmp, &dxChDlb);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCfgGlobalEportSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] globalPtr                - (pointer to) Global EPorts for the Bridge,L2MLL to use.
* @param[out] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
* @param[out] l2EcmpPtr                - (pointer to) L2 DLB Primary ePorts.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgGlobalEportGet
(
    IN  GT_U8                         devNum,
    OUT PRV_TGF_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT PRV_TGF_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    OUT PRV_TGF_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  dxChEcmp = {0,0,0,0,0};
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  dxChGlobal = {0,0,0,0,0};
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  dxChDlb = {0,0,0,0,0};

    /* call device specific API */
    rc = cpssDxChCfgGlobalEportGet(devNum, &dxChGlobal , &dxChEcmp, &dxChDlb);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCfgGlobalEportSet FAILED, rc = [%d]", rc);
        return rc;
    }

    PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(dxChEcmp.enable, l2EcmpPtr->enable);
    l2EcmpPtr->pattern  = dxChEcmp.pattern;
    l2EcmpPtr->mask     = dxChEcmp.mask;
    l2EcmpPtr->minValue = dxChEcmp.minValue;
    l2EcmpPtr->maxValue = dxChEcmp.maxValue;

    PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(dxChGlobal.enable, globalPtr->enable);
    globalPtr->pattern  = dxChGlobal.pattern;
    globalPtr->mask     = dxChGlobal.mask;
    globalPtr->minValue = dxChGlobal.minValue;
    globalPtr->maxValue = dxChGlobal.maxValue;

    PRV_TGF_CONVERT_GENERIC_TO_DXCH_GLOBAL_EPORT_CFG_MODE(dxChDlb.enable, l2DlbPtr->enable);
    l2DlbPtr->pattern   = dxChDlb.pattern;
    l2DlbPtr->mask      = dxChDlb.mask;
    l2DlbPtr->minValue  = dxChDlb.minValue;
    l2DlbPtr->maxValue  = dxChDlb.maxValue;

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfCfgIngressDropCntrModeSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter Mode.
*
* @param[in] mode                     - Ingress Drop Counter Mode.
*                                      port      - port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[in] vlan                     - VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong mode, portNum or vlan.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgIngressDropCntrModeSet
(
    IN  PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_ENT   mode,
    IN  GT_PORT_NUM                                 portNum,
    IN  GT_U16                                      vlan
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCfgIngressDropCntrModeSet(devNum, mode, portNum, vlan);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTgfCfgIngressDropCntrModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(mode);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(vlan);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}
/**
* @internal prvTgfCfgIngressDropCntrSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter value.
*
* @param[in] counter                  - Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgIngressDropCntrSet
(
    IN  GT_U32      counter
)
{

#ifdef CHX_FAMILY

    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        if(counter == 0 &&
           PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            GT_U32  tmpCounter;
            /* the register not support 'set' it but to get value 0 in the register
               ... we can read it ! because it is 'ROC' register */
            /* call device specific API */
            rc = cpssDxChCfgIngressDropCntrGet(devNum, &tmpCounter);
        }
        else
        {
            /* call device specific API */
            rc = cpssDxChCfgIngressDropCntrSet(devNum, counter);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCfgIngressDropCntrSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(counter);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfCfgIngressDropCntrGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgIngressDropCntrGet
(
    OUT GT_U32      *counterPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U32 counter; /* ingres drop counter value for specific devices */
    *counterPtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCfgIngressDropCntrGet(devNum, &counter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCfgIngressDropCntrGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        /* sum of counters from all devices */
        *counterPtr += counter;
    }
    return rc1;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(counterPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfCfgPortDefaultSourceEportNumberSet function
* @endinternal
*
* @brief   Set Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ePort                    -  number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number. In Multi-core
*       architecture it is the global physical port number (i.e. Local Core
*       Port number bitwise ORed with LocalCoreID<<4)
*       2. Can be overriden by TTI Action/RBridge
*
*/
GT_STATUS prvTgfCfgPortDefaultSourceEportNumberSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         ePort
)
{
#ifdef CHX_FAMILY
    return cpssDxChCfgPortDefaultSourceEportNumberSet(devNum,portNum,ePort);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}


/**
* @internal prvTgfCfgPortDefaultSourceEportNumberGet function
* @endinternal
*
* @brief   Get Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ePortPtr                 - pointer to ePort number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number. In Multi-core
*       architecture it is the global physical port number (i.e. Local Core
*       Port number bitwise ORed with LocalCoreID<<4)
*       2. Can be overriden by TTI Action/RBridge
*
*/
GT_STATUS prvTgfCfgPortDefaultSourceEportNumberGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *ePortPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCfgPortDefaultSourceEportNumberGet(devNum,portNum,ePortPtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}
/**
* @internal prvTgfCfgIngressDropEnableSet function
* @endinternal
*
* @brief   Enable/disable packet drop in ingress processing pipe
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - The device number.
* @param[in] pktDropEnable         -  Enable/disable packet drop in ingress processing pipe
*                                     GT_TRUE - drop packet in ingress pipe, egress
*                                               pipe does not get the dropped packets
*                                     GT_FALSE- pass dropped packets to egress
*                                               pipe, drop done in egress pipe
*
*Note: In egress filter, only the ingress drops are forwarded.
*      The drops in egress filtering logic is dropped in egress filter itself.
*
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - wrong device Number
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS prvTgfCfgIngressDropEnableSet
(
    IN  GT_U8         devNum,
    IN  GT_BOOL       pktDropEnable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChCfgIngressDropEnableSet(devNum, pktDropEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCfgIngressDropEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(pktDropEnable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal cpssDxChCfgIngressDropEnableGet function
* @endinternal
*
* @brief   Get packet drop status in ingress processing pipe
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 -  The device number.
*
* @param[out] pktDropEnablePtr      -  (pointer to) the enabling status of drop in ingress processing pipe
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - wrong device Number
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_PTR                - one of the pointers is NULL
*/
GT_STATUS prvTgfCfgIngressDropEnableGet
(
    IN   GT_U8          devNum,
    OUT  GT_BOOL        *pktDropEnablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChCfgIngressDropEnableGet(devNum, pktDropEnablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCfgIngressDropEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(pktDropEnablePtr);
    return GT_BAD_STATE;
#endif
}

