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
* @file cpssDxChPortPip.c
*
* @brief CPSS DxCh Port's Pre-Ingress Prioritization (PIP) implementation.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* check that index is in proper 0..max (as parameter) */
#define CHECK_PARAM_INDEX_MAC(_index,_firstNotValidIndex)   \
    if(_index >= _firstNotValidIndex)                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that index is in proper 0..max (as 'data') */
#define CHECK_DATA_INDEX_MAC(_index,_firstNotValidIndex)   \
    if(_index >= _firstNotValidIndex)                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG)


/* check that pip profile as parameter */
#define CHECK_PARAM_PIP_PROFILE_MAC(_devNum,_pipProfile) \
    if(_pipProfile > 3)                                  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that pip priority as parameter */
#define CHECK_PARAM_PIP_PRIORITY_MAC(_devNum,_pipProfile) \
    if((_pipProfile > 3)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
    else if((_pipProfile > 2) && !PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that pip priority as value */
#define CHECK_VALUE_PIP_PRIORITY_MAC(_devNum,_pipPriority) \
    if((_pipPriority > 3)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);  \
    else if((_pipPriority > 2) && !PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG)

#define  IS_PIP_SUPPORTED_MAC(_devNum) \
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_10_E);                \

#define  IS_IA_UNIT_SUPPORTED_MAC(_devNum) \
    if(PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.pipInfo.isIaUnitNotSupported == GT_TRUE) \
    {                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,                    \
            "The device not supports the 'PIP' feature");                          \
    }


/* LookUp Tables for priorities */
static const GT_U8 toHwPrioLut[]  = {3/*very high*/, 0/*high*/, 1/*med*/, 2/*low*/};
static const GT_U8 toApiPrioLut[] = {1/*high*/     , 2/*med*/ , 3/*low*/, 0/*very high*/};

/**
* @internal pip_setPerPort function
* @endinternal
*
* @brief   PIP : set 'per port' field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have RxDma)
* @param[in] startBit                 -  of the field to set
* @param[in] numBits                  - number of bit of the field to set
* @param[in] value                    -  to set to the field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_OUT_OF_RANGE          - on value out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   pip_setPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32       startBit,
    IN GT_U32       numBits,
    IN GT_U32       value
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 portRxDmaNum; /* RxDma number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32  localPort;   /* local port - support multi-port-group device */


    /* check portNum and get converted rxDma port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    /* convert the 'global RxDma number' to portGroupId,local port -- supporting multi port group device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.channelPIPConfigReg[localPort];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAPip.
            SCDMA_n_PIP_Config[localPort];
    }

    if(value >= (GT_U32)(1 << numBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                               regAddr, startBit, numBits, value);
}

/**
* @internal pip_getPerPort function
* @endinternal
*
* @brief   PIP : get 'per port' field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have RxDma)
* @param[in] startBit                 -  of the field to get
* @param[in] numBits                  - number of bit of the field to get
*
* @param[out] valuePtr                 - (pointer to)value to get of the field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   pip_getPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32       startBit,
    IN GT_U32       numBits,
    OUT GT_U32       *valuePtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 portRxDmaNum; /* RxDma number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32  localPort;   /* local port - support multi-port-group device */

    /* check portNum and get converted rxDma port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    /* convert the 'global RxDma number' to portGroupId,local port -- supporting multi port group device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.channelPIPConfigReg[localPort];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAPip.
            SCDMA_n_PIP_Config[localPort];
    }

    return  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
                               regAddr, startBit, numBits, valuePtr);
}

/**
* @internal pip_counterGetPerPort function
* @endinternal
*
* @brief   PIP : counter get 'per port'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have RxDma)
*
* @param[out] valuePtr                 - (pointer to)value to get of the counter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   pip_counterGetPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32       *valuePtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 portRxDmaNum; /* RxDma number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32  localPort;   /* local port - support multi-port-group device */

    /* check portNum and get converted rxDma port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    /* convert the 'global RxDma number' to portGroupId,local port -- supporting multi port group device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAPip.
        SCDMA_n_Drop_Pkt_Counter[localPort];

    return  prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                               regAddr, valuePtr);
}

/**
* @internal pip_etherTypeProtocolInfoGet function
* @endinternal
*
* @brief   PIP : get register address for the EtherType - 'per protocol' 'per index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] regAddrPtr               - (pointer to) register address that hold this field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong protocol or index
*/
static GT_STATUS pip_etherTypeProtocolInfoGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U32      *regAddrPtr
)
{
    GT_U32 regAddr;

    switch(protocol)
    {
        case CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(index,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.VLANEtherTypeConf[index];
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E    :
            /* check index*/
            CHECK_PARAM_INDEX_MAC(index,2);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.MPLSEtherTypeConf[index];
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E    :
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.IPv4EtherTypeConf;
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E    :
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.IPv6EtherTypeConf;
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E     :
            /* check index*/
            CHECK_PARAM_INDEX_MAC(index,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.genericEtherType[index];
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "only sip6 supports 'latency sensitive' protocols");
            }
            CHECK_PARAM_INDEX_MAC(index,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.latencySensitiveEtherTypeConf[index];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(protocol);
    }

    *regAddrPtr = regAddr;

    return GT_OK;
}

/**
* @internal pip_pofilePriorityInfoGet function
* @endinternal
*
* @brief   PIP : get register address for the pip priority - 'per profile' 'per type' 'per fieldIndex'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] regAddrPtr               - (pointer to) register address that hold this field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profile or type or fieldIndex
*/
static GT_STATUS pip_pofilePriorityInfoGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 regAddr;

    /* check the pip profile (0..3) as parameter (not as value) */
    CHECK_PARAM_PIP_PROFILE_MAC(devNum,pipProfile);

    switch(type)
    {
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_7);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                qoSDSAPriorityArrayProfileSegment[pipProfile][fieldIndex/16];

            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_3);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                UPDSATagArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_3);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                VLANUPPriorityArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_3);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                MPLSEXPPriorityArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_8);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                IPv4ToSPriorityArrayProfileSegment[pipProfile][fieldIndex/16];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_8);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                IPv6TCPriorityArrayProfileSegment[pipProfile][fieldIndex/16];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                etherTypePriorityArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                MACDAPriorityArrayProfile[pipProfile];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;

    return GT_OK;

}

/**
* @internal internal_cpssDxChPortPipGlobalEnableSet function
* @endinternal
*
* @brief   Enable/disable the PIP engine. (Global to the device)
*         if enabled packets are dropped according to their parsed priority if FIFOs
*         before the control pipe are above a configurable threshold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -   PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32 regAddr;       /* register address */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPEnable;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.preingressPrioritizationEnable;
    }

    /*<Pre-ingress prioritization En>*/
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPortPipGlobalEnableSet function
* @endinternal
*
* @brief   Enable/disable the PIP engine. (Global to the device)
*         if enabled packets are dropped according to their parsed priority if FIFOs
*         before the control pipe are above a configurable threshold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -   PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortPipGlobalEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalEnableGet function
* @endinternal
*
* @brief   Get the state Enable/disable of the PIP engine. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable  PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_U32 value;         /* register value */
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPEnable;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.preingressPrioritizationEnable;
    }

    /*<Pre-ingress prioritization En>*/
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalEnableGet function
* @endinternal
*
* @brief   Get the state Enable/disable of the PIP engine. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable  PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortPipGlobalEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalEtherTypeProtocolSet function
* @endinternal
*
* @brief   Set the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  to recognize the protocols packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    rc = pip_etherTypeProtocolInfoGet(devNum,protocol,index,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*<etherType>*/
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, etherType);
}

/**
* @internal cpssDxChPortPipGlobalEtherTypeProtocolSet function
* @endinternal
*
* @brief   Set the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  to recognize the protocols packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEtherTypeProtocolSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocol, index, etherType));

    rc = internal_cpssDxChPortPipGlobalEtherTypeProtocolSet(devNum, protocol, index, etherType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocol, index, etherType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalEtherTypeProtocolGet function
* @endinternal
*
* @brief   Get the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
)
{
    GT_U32 value;         /* register value */
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    rc = pip_etherTypeProtocolInfoGet(devNum,protocol,index,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*<etherType>*/
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &value);

    *etherTypePtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalEtherTypeProtocolGet function
* @endinternal
*
* @brief   Get the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEtherTypeProtocolGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocol, index, etherTypePtr));

    rc = internal_cpssDxChPortPipGlobalEtherTypeProtocolGet(devNum, protocol, index, etherTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocol, index, etherTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipTrustEnableSet function
* @endinternal
*
* @brief   Set port as trusted/not trusted PIP port.
*         A 'trusted' port will set priority level for ingress packets according to
*         packet's fields.
*         Otherwise, for the 'not trusted' port will use the port’s default priority
*         level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    /* <Port <n> Trusted> */
    return  pip_setPerPort(devNum, portNum , 0, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPortPipTrustEnableSet function
* @endinternal
*
* @brief   Set port as trusted/not trusted PIP port.
*         A 'trusted' port will set priority level for ingress packets according to
*         packet's fields.
*         Otherwise, for the 'not trusted' port will use the port’s default priority
*         level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipTrustEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPipTrustEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipTrustEnableGet function
* @endinternal
*
* @brief   Get port's trusted/not trusted .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 value;         /* register value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Port <n> Trusted> */
    rc =  pip_getPerPort(devNum, portNum , 0, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortPipTrustEnableGet function
* @endinternal
*
* @brief   Get port's trusted/not trusted .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipTrustEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPipTrustEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipProfileSet function
* @endinternal
*
* @brief   Set port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] pipProfile               - the profile of the port.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    /* <Port <n> Pip Profile> */
    return  pip_setPerPort(devNum, portNum , 3, 2, pipProfile);
}

/**
* @internal cpssDxChPortPipProfileSet function
* @endinternal
*
* @brief   Set port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] pipProfile               - the profile of the port.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pipProfile));

    rc = internal_cpssDxChPortPipProfileSet(devNum, portNum, pipProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pipProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipProfileGet function
* @endinternal
*
* @brief   Get port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] pipProfilePtr            - (pointer to) the profile of the port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pipProfilePtr);

    /* <Port <n> Pip Profile> */
    return pip_getPerPort(devNum, portNum , 3, 2, pipProfilePtr);
}

/**
* @internal cpssDxChPortPipProfileGet function
* @endinternal
*
* @brief   Get port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] pipProfilePtr            - (pointer to) the profile of the port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pipProfilePtr));

    rc = internal_cpssDxChPortPipProfileGet(devNum, portNum, pipProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pipProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipVidClassificationEnableSet function
* @endinternal
*
* @brief   Set port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE   -  VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    /* <Port <n> VID Enable> */
    return  pip_setPerPort(devNum, portNum , 5, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPortPipVidClassificationEnableSet function
* @endinternal
*
* @brief   Set port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE   -  VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipVidClassificationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPipVidClassificationEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipVidClassificationEnableGet function
* @endinternal
*
* @brief   Get port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE   - enable VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 value;         /* register value */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Port <n> VID Enable> */
    rc = pip_getPerPort(devNum, portNum , 5, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortPipVidClassificationEnableGet function
* @endinternal
*
* @brief   Get port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE   - enable VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipVidClassificationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPipVidClassificationEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipPrioritySet function
* @endinternal
*
* @brief   Set priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
)
{
    GT_U32       startBit;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check the pip priority (0..3) as value (not as parameter) */
    CHECK_VALUE_PIP_PRIORITY_MAC(devNum,priority);

    if(type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E)
    {
        /* <Ingress Port <n> Pip Priority> */
        startBit = 6;
    }
    else
    if (type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E)
    {
        /* <Port <n> Default PIP Priority>*/
        startBit = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Translate priority */
        priority = toHwPrioLut[priority];
    }
    return  pip_setPerPort(devNum, portNum , startBit, 2, priority);
}

/**
* @internal cpssDxChPortPipPrioritySet function
* @endinternal
*
* @brief   Set priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipPrioritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, type, priority));

    rc = internal_cpssDxChPortPipPrioritySet(devNum, portNum, type, priority);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, type, priority));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipPriorityGet function
* @endinternal
*
* @brief   Get priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
*
* @param[out] priorityPtr              - (pointer to)the priority value to set.
*                                      For Bobcat3:     For Bobcat2, Caelum, Aldrin, AC3X:
*                                      0 - Very High    0 - High
*                                      1 - High         1 - Medium
*                                      2 - Medium       2 - Low
*                                      3 - Low
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
)
{
    GT_U32       startBit;
    GT_STATUS    rc;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(priorityPtr);

    if(type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E)
    {
        /* <Ingress Port <n> Pip Priority> */
        startBit = 6;
    }
    else
    if (type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E)
    {
        /* <Port <n> Default PIP Priority>*/
        startBit = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = pip_getPerPort(devNum, portNum , startBit, 2, priorityPtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Translate priority */
        *priorityPtr = toApiPrioLut[*priorityPtr];
    }
    return  rc;
}

/**
* @internal cpssDxChPortPipPriorityGet function
* @endinternal
*
* @brief   Get priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
*
* @param[out] priorityPtr              - (pointer to)the priority value to set.
*                                      For Bobcat3:     For Bobcat2, Caelum, Aldrin, AC3X:
*                                      0 - Very High    0 - High
*                                      1 - High         1 - Medium
*                                      2 - Medium       2 - Low
*                                      3 - Low
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipPriorityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, type, priorityPtr));

    rc = internal_cpssDxChPortPipPriorityGet(devNum, portNum, type, priorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, type, priorityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalProfilePrioritySet function
* @endinternal
*
* @brief   Set priority per PIP profile Per 'classification' type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or
*                                       fieldIndex
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 regAddr;       /* register address */
    GT_U32 numBits = 2;   /* number of bits to set */
    GT_U32 hwValue = priority;/* value to write to HW */
    GT_U32 hwIndex = fieldIndex;/* index to write to HW */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check the pip priority (0..3) as value (not as parameter) */
    CHECK_VALUE_PIP_PRIORITY_MAC(devNum,priority);

    /* Translate priority */
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        priority = toHwPrioLut[priority];
        hwValue = priority;
    }

    if ((type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E) &&
        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
    {
        if (fieldIndex >= BIT_6)
        {
            /* only 6 bits of the TC actually influence the ipv6 packet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* the actual field is (TC_from_packet <<2) + 2 bits from 'flow label' */
        hwIndex = fieldIndex << 2;
        /* we need to duplicate the value to 4 consecutive places */
        hwValue = (priority << 6) | (priority << 4) | (priority << 2) | priority;
        numBits = 8;/* 2 bits time the 4 consecutive places */
    }

    rc = pip_pofilePriorityInfoGet(devNum,pipProfile,type,hwIndex,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* each field is 2 bits , (up to) 16 fields in the register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 2 * (hwIndex%16), numBits, hwValue);
}

/**
* @internal cpssDxChPortPipGlobalProfilePrioritySet function
* @endinternal
*
* @brief   Set priority per PIP profile Per 'classification' type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or
*                                       fieldIndex
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalProfilePrioritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipProfile, type, fieldIndex, priority));

    rc = internal_cpssDxChPortPipGlobalProfilePrioritySet(devNum, pipProfile, type, fieldIndex, priority);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipProfile, type, fieldIndex, priority));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalProfilePriorityGet function
* @endinternal
*
* @brief   Get priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] priorityPtr              - (pointer to)the priority value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 regAddr;       /* register address */
    GT_U32 hwIndex = fieldIndex;/* index to write to HW */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(priorityPtr);

    if ((type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E) &&
        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
    {
        if (fieldIndex >= BIT_6)
        {
            /* only 6 bits of the TC actually influence the ipv6 packet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* the actual field is (TC_from_packet <<2) + 2 bits from 'flow label' */
        hwIndex = fieldIndex << 2;
        /* the 'set' function did : duplicate the value to 4 places consecutive places.
           this 'get' function we return the 'first' one.
        */
    }

    rc = pip_pofilePriorityInfoGet(devNum,pipProfile,type,hwIndex,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* each field is 2 bits , (up to) 16 fields in the register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2 * (hwIndex%16), 2, priorityPtr);

    /* Translate priority */
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        *priorityPtr = toApiPrioLut[*priorityPtr];
    }

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalProfilePriorityGet function
* @endinternal
*
* @brief   Get priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] priorityPtr              - (pointer to)the priority value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalProfilePriorityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipProfile, type, fieldIndex, priorityPtr));

    rc = internal_cpssDxChPortPipGlobalProfilePriorityGet(devNum, pipProfile, type, fieldIndex, priorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipProfile, type, fieldIndex, priorityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalVidClassificationSet function
* @endinternal
*
* @brief   Set PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] vid                      - the  to match
*                                      (APPLICABLE RANGES: 0..(4k-1))
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range of vid
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
)
{
    GT_U32 regAddr;       /* register address */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CHECK_PARAM_INDEX_MAC(index,4);
    CHECK_DATA_INDEX_MAC(vid,BIT_12);

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.VIDConf[index];

    /* <VId Conf <n>> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 12, vid);
}

/**
* @internal cpssDxChPortPipGlobalVidClassificationSet function
* @endinternal
*
* @brief   Set PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] vid                      - the  to match
*                                      (APPLICABLE RANGES: 0..(4k-1))
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range of vid
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalVidClassificationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, vid));

    rc = internal_cpssDxChPortPipGlobalVidClassificationSet(devNum, index, vid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, vid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalVidClassificationGet function
* @endinternal
*
* @brief   Get PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] vidPtr                   - (pointer to)the vid to match
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 value;         /* register value */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CHECK_PARAM_INDEX_MAC(index,4);
    CPSS_NULL_PTR_CHECK_MAC(vidPtr);

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.VIDConf[index];

    /* <VId Conf <n>> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 12, &value);

    *vidPtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalVidClassificationGet function
* @endinternal
*
* @brief   Get PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] vidPtr                   - (pointer to)the vid to match
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalVidClassificationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, vidPtr));

    rc = internal_cpssDxChPortPipGlobalVidClassificationGet(devNum, index, vidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, vidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPipGlobalMacDaClassificationEntrySet function
* @endinternal
*
* @brief   Set PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of value in entry
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */
    GT_U32  lsWord , msWord;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CHECK_PARAM_INDEX_MAC(index,4);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /*******************/
    /* set the pattern */
    /*******************/

    lsWord =
        entryPtr->macAddrValue.arEther[2] << 24 |
        entryPtr->macAddrValue.arEther[3] << 16 |
        entryPtr->macAddrValue.arEther[4] <<  8 |
        entryPtr->macAddrValue.arEther[5] <<  0 ;
    msWord =
        entryPtr->macAddrValue.arEther[0] <<  8 |
        entryPtr->macAddrValue.arEther[1] <<  0 ;

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[index];

    /* <MAC DA 4 Lsb Bytes Conf <n>> */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[index];

    /* <MAC DA 2 Msb Bytes Conf <n>> */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16 , msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    /****************/
    /* set the mask */
    /****************/
    lsWord =
        entryPtr->macAddrMask.arEther[2] << 24 |
        entryPtr->macAddrMask.arEther[3] << 16 |
        entryPtr->macAddrMask.arEther[4] <<  8 |
        entryPtr->macAddrMask.arEther[5] <<  0 ;
    msWord =
        entryPtr->macAddrMask.arEther[0] <<  8 |
        entryPtr->macAddrMask.arEther[1] <<  0 ;

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask4LsbConf[index];

    /* <MAC DA Mask 4 Lsb Conf <n>> */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask2MsbConf[index];

    /* <MAC DA Mask 2 Msb Conf <n>> */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16 , msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPipGlobalMacDaClassificationEntrySet function
* @endinternal
*
* @brief   Set PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of value in entry
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalMacDaClassificationEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChPortPipGlobalMacDaClassificationEntrySet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPipGlobalMacDaClassificationEntryGet function
* @endinternal
*
* @brief   Get PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */
    GT_U32  lsWord , msWord;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CHECK_PARAM_INDEX_MAC(index,4);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /*******************/
    /* get the pattern */
    /*******************/
    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[index];

    /* <MAC DA 4 Lsb Bytes Conf <n>> */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[index];

    /* <MAC DA 2 Msb Bytes Conf <n>> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16 , &msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    entryPtr->macAddrValue.arEther[0] = (GT_U8)    (msWord >>  8);
    entryPtr->macAddrValue.arEther[1] = (GT_U8)    (msWord >>  0);

    entryPtr->macAddrValue.arEther[2] = (GT_U8)    (lsWord >> 24);
    entryPtr->macAddrValue.arEther[3] = (GT_U8)    (lsWord >> 16);
    entryPtr->macAddrValue.arEther[4] = (GT_U8)    (lsWord >>  8);
    entryPtr->macAddrValue.arEther[5] = (GT_U8)    (lsWord >>  0);

    /****************/
    /* get the mask */
    /****************/
    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask4LsbConf[index];

    /* <MAC DA Mask 4 Lsb Conf <n>> */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask2MsbConf[index];

    /* <MAC DA Mask 2 Msb Conf <n>> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16 , &msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    entryPtr->macAddrMask.arEther[0] = (GT_U8)    (msWord >>  8);
    entryPtr->macAddrMask.arEther[1] = (GT_U8)    (msWord >>  0);

    entryPtr->macAddrMask.arEther[2] = (GT_U8)    (lsWord >> 24);
    entryPtr->macAddrMask.arEther[3] = (GT_U8)    (lsWord >> 16);
    entryPtr->macAddrMask.arEther[4] = (GT_U8)    (lsWord >>  8);
    entryPtr->macAddrMask.arEther[5] = (GT_U8)    (lsWord >>  0);

    return GT_OK;
}

/**
* @internal cpssDxChPortPipGlobalMacDaClassificationEntryGet function
* @endinternal
*
* @brief   Get PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalMacDaClassificationEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChPortPipGlobalMacDaClassificationEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal pipGlobalBurstFifoThresholdDpMaskCreateCheck function
* @endinternal
*
* @brief   Creates bit mask for available datapathes with non-zero port count and
*         checks if dataPathBitmap passed to
*         cpssDxChPortPipGlobalMacDaClassificationEntryG(S)et uses only them.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; AC5; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*
* @param[out] maskPtr                  - created mask
* @param[out] dpSizePtr                - mask size, may be NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
*/
static GT_STATUS pipGlobalBurstFifoThresholdDpMaskCreateCheck
(
    IN  GT_U8                devNum,
    IN  GT_DATA_PATH_BMP     dataPathBmp,
    OUT GT_U32               *maskPtr,
    OUT GT_U32               *dpSizePtr
)
{
    GT_U32      lMask = 0; /* Local mask */
    GT_U32      ii;
    GT_U32      lDpSize;

    if(dataPathBmp == 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        lDpSize = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;
        lMask = BIT_MASK_0_31_MAC(lDpSize);
    }
    else
    {
        lMask = 0;
        lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
        for(ii = 0; ii < lDpSize; ii++)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.
                    info[ii].dataPathNumOfPorts)
            {
                lMask |= (1 << ii);
            }
        }
    }
    if(dataPathBmp & ~(lMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The bmp[0x%8.8x] hold"
            " bits out of the valid range[0x%8.8x] \n", dataPathBmp, lMask);
    }
    else
    {
        *maskPtr = lMask;
        if(NULL != dpSizePtr)
        {
            *dpSizePtr = lDpSize;
        }
        return GT_OK;
    }
}

/**
* @internal sip6_cpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port groups
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi Port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi Port groups device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the priority to which to set the threshold
*                                      (APPLICABLE RANGES: Falcon; AC5P; AC5X; Harrier; Ironman 0..3).
*                                           - (0-very high,1-high,2-mid,3-low)
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: Falcon; AC5P; AC5X; Harrier; Ironman 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP portGroupsBmp,
    IN GT_U32           priority,
    IN GT_U32           threshold
)
{
    GT_STATUS rc;         /* return code */
    GT_U32 regAddr;       /* register address */
    GT_U32 fieldOffset;   /* offset of threshold in the register*/
    GT_U32  portGroupId;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* check threshold */
    CHECK_DATA_INDEX_MAC(threshold,BIT_16);

    fieldOffset = 16 * (priority & 1);/*0 or 16 */
    regAddr =
        priority  < 2 ?
        PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds0 :
        PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds1 ;

    /*
        NOTE: the loop on port groups requires calling prvCpssHwPpPortGroupSetRegField
        and in function per_pipe_UnitDuplicatedMultiPortGroupsGet the 'address' in pipe 0
        will be translated to the other pipe.
    */

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
             regAddr, fieldOffset, 16, threshold);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}
/**
* @internal internal_cpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the priority to which to set the threshold
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                           - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman. 0..3).
*                                           - (0-very high,1-high,2-mid,3-low)
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x3FF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    IN GT_U32           threshold
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* return code */
    GT_U32 dpIdx;         /* Datapath index */
    GT_U32 thrOff;        /* offset of threshold in the register*/
    GT_U32 mask;          /* mask of available datapathes */
    GT_U32 fieldSize;     /* field size is different for SIP 5.20 */
    GT_U32 isDataPathBmpApplicable; /* Is dataPathBmp applicable flag */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    /* check the pip priority (0..3) as parameter (not as value) */
    CHECK_PARAM_PIP_PRIORITY_MAC(devNum, priority);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return sip6_cpssDxChPortPipGlobalBurstFifoThresholdsSet(devNum,
            dataPathBmp,/* in sip6 treated as 'port groups' */
            priority,threshold);
    }
    else
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(priority < 2)
        {
            thrOff = (priority == 0) ? 0 : 16;
        }
        else
        {
            thrOff = (priority == 2) ? 0 : 16;
        }
        fieldSize = 16;

        /* the FIFOs are in IA units.
           Number of IA units equal to number of processing pipes. */
        isDataPathBmpApplicable = (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1) ? GT_TRUE : GT_FALSE;


    }
    else /* (priority < 4) */
    {
        thrOff = priority * 10;
        fieldSize = 10;

        /* the FIFOs are in RXDMA units. */
        isDataPathBmpApplicable = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath;
    }

    /* check threshold */
    if(threshold >= (GT_U32)(1 << fieldSize))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(isDataPathBmpApplicable
       && (dataPathBmp != CPSS_DATA_PATH_UNAWARE_MODE_CNS))
    {

        rc = pipGlobalBurstFifoThresholdDpMaskCreateCheck(devNum,
            dataPathBmp, &mask, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Only selected datapathes */
        dpIdx = 0;
        rc = GT_OK; /* If nothing to change -- OK by default */
        for(; mask; mask >>= 1)
        {
            if(mask & dataPathBmp & 0x01)
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    if(priority < 2)
                    {
                        regAddr =
                            PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dpIdx).
                                PIPPrioThresholds0;
                    }
                    else /* (priority < 4) */
                    {
                        regAddr =
                            PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dpIdx).
                                PIPPrioThresholds1;
                    }
                }
                else
                {
                    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(devNum, dpIdx).
                        globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                        PIPPrioThresholds;
                }
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, thrOff,
                    fieldSize, threshold);
                if(rc != GT_OK)
                {
                    break;
                }
            }
            dataPathBmp >>= 1;
            dpIdx++;
        }
    }
    else
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /* Globally configure multi-pipe device */
            if(priority < 2)
            {
                regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds0;
            }
            else /* (priority < 4) */
            {
                regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds1;
            }

            rc = prvCpssHwPpSetRegField(devNum, regAddr, thrOff, fieldSize,
                threshold);
        }
        else
        {
            /* Not the case with multidatapath device or datapath unaware */
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.PIPPrioThresholds;

            rc = prvCpssHwPpSetRegField(devNum, regAddr, thrOff, fieldSize,
                threshold);
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the priority to which to set the threshold
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                           - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman. 0..3).
*                                           - (0-very high,1-high,2-mid,3-low)
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x3FF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    IN GT_U32           threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
        cpssDxChPortPipGlobalBurstFifoThresholdsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dataPathBmp, priority, threshold));

    rc = internal_cpssDxChPortPipGlobalBurstFifoThresholdsSet(devNum,
        dataPathBmp, priority, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dataPathBmp,
        priority, threshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip6_cpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port groups
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi Port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi Port groups device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the priority to which to set the threshold
*                                           - (0-very high,1-high,2-mid,3-low,4-latency sensitive)
* @param[out] thresholdPtr             - (pointer to)the threshold value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP portGroupsBmp,
    IN GT_U32           priority,
    OUT GT_U32         *thresholdPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 fieldOffset;   /* offset of threshold in the register*/
    GT_U32  portGroupId;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    fieldOffset = 16 * (priority & 1);/*0 or 16 */
    regAddr =
        priority  < 2 ?
        PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds0 :
        PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds1 ;
    /* read from the first port groups available */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);

    return prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
         regAddr, fieldOffset, 16, thresholdPtr);
}

/**
* @internal internal_cpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s). If a bit of non valid data path is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      PSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      4. read only from first data path of the bitmap.
* @param[in] priority                 - the priority to which to set the threshold
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                           - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman. 0..3).
*                                           - (0-very high,1-high,2-mid,3-low)
* @param[out] thresholdPtr             - (pointer to)the threshold value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    OUT GT_U32         *thresholdPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 dmaIdx;
    GT_U32 mask;          /* mask of available datapathes */
    GT_U32 dpSize;
    GT_U32 rc;
    GT_U32 thrOff;
    GT_U32 isDataPathBmpApplicable; /* Is dataPathBmp applicable flag */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    /* check the pip priority (0..3) as parameter (not as value) */
    CHECK_PARAM_PIP_PRIORITY_MAC(devNum, priority);

    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return sip6_cpssDxChPortPipGlobalBurstFifoThresholdsGet(devNum,
            dataPathBmp,/* in sip6 treated as 'port groups' */
            priority,thresholdPtr);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* the FIFOs are in IA units.
           Number of IA units equal to number of processing pipes. */
        isDataPathBmpApplicable = (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        /* the FIFOs are in RXDMA units. */
        isDataPathBmpApplicable = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath;
    }

    dmaIdx = 0;
    if(isDataPathBmpApplicable
       && (dataPathBmp != CPSS_DATA_PATH_UNAWARE_MODE_CNS))
    {
        rc = pipGlobalBurstFifoThresholdDpMaskCreateCheck(devNum,
            dataPathBmp, &mask, &dpSize);
        if (rc != GT_OK)
        {
            return rc;
        }
        while(dmaIdx < dpSize)
        {
            if(mask & dataPathBmp & 0x1)
            {
                break;
            }
            dataPathBmp >>= 1;
            mask >>= 1;
            dmaIdx++;
        }
        if(dmaIdx == dpSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(priority < 2)
        {
            thrOff = (priority == 0) ? 0 : 16;
            regAddr = PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dmaIdx).
                PIPPrioThresholds0;
        }
        else
        {
            thrOff = (priority == 2) ? 0 : 16;
            regAddr = PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dmaIdx).
                PIPPrioThresholds1;
        }
        /* 16 bits per priority <PIP <Low/Mid/High/VeryHigh> Prio Threshold> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, thrOff, 16, thresholdPtr);
    }
    else /* (priority < 4) */
    {
        thrOff = priority * 10;
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(devNum, dmaIdx).
            globalRxDMAConfigs.preIngrPrioritizationConfStatus.
            PIPPrioThresholds;
        /* 10 bits per priority <PIP <Low/Mid/High/VeryHigh> Prio Threshold> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, thrOff, 10, thresholdPtr);
    }
    return rc;
}

/**
* @internal cpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s). If a bit of non valid data path is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      PSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      4. read only from first data path of the bitmap.
* @param[in] priority                 - the priority to which to set the threshold
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                           - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman. 0..3).
*                                           - (0-very high,1-high,2-mid,3-low)
* @param[out] thresholdPtr             - (pointer to)the threshold value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    OUT GT_U32         *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
        cpssDxChPortPipGlobalBurstFifoThresholdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dataPathBmp, priority,
        thresholdPtr));

    rc = internal_cpssDxChPortPipGlobalBurstFifoThresholdsGet(devNum,
        dataPathBmp, priority, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dataPathBmp,
        priority, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal pipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific PIP priority. (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] priority                 - the priority of the counter.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2).
*                                      - (0-very high,1-high,2-mid,3-low)
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS pipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    IN GT_U32       priority,
    OUT GT_U64      *counterPtr
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 regAddrMsb,regAddrLsb;       /* register address */
    GT_U64 tmpCounter1,tmpCounter2;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(priority == 3)/* low */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrLSb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrMsb;
        }
        else
        if(priority == 2)/* med */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrMsb;
        }
        else
        if(priority == 1)/* high */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrMsb;
        }
        else /* very high */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPVeryHighPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPVeryHighPriorityDropGlobalCntrMsb;

        }
    }
    else
    {
        if(priority == 2)/* low */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrLSb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrMsb;
        }
        else
        if(priority == 1)/* med */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrMsb;
        }
        else /* high */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrMsb;
        }
    }

    tmpCounter2.l[0] = 0;
    /* first read MSWord , than read LSWord */
    /* because read of LSWord clears the counter */
    rc =  prvCpssPortGroupsCounterSummary(devNum, regAddrMsb,0,32,
                        &tmpCounter2.l[1],/* get counter as 32 bits */
                        NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPortGroupsCounterSummary(devNum, regAddrLsb ,0,32,
                        NULL,
                        &tmpCounter1);/* get counter as 64 bits , because can be
                                         aggregation from several RXDMA units */
    if(rc != GT_OK)
    {
        return rc;
    }

    /* summary the counters */
    (*counterPtr)= prvCpssMathAdd64(tmpCounter1,tmpCounter2);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get all PIP drop counters (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*
* @param[out] countersArr[4]           - (pointer to) the counters of
*                                      - 3 priorities (0-high, 1-mid, 2-low) for Bobcat2,
*                                      Caelum, Aldrin, AC3X
*                                      - 4 priorities (0-very high, 1-high, 2-mid, 3-low) for
*                                      Bobcat3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32  regAddr;
    GT_U32  priority;
    GT_U32  maxPriority;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(countersArr);

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* This trigger needs to be activated before reading the
           PIP Priority Drop Global Counter (this is a trigger for all priority counters).
           This is a self cleared bit.
        */
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.PIPDropGlobalCounterTrigger;

        /* check that all port groups are ready */
        rc = prvCpssPortGroupBusyWait(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    regAddr,0,/*bit0 - <Load All Global Drop Counters */
                    GT_TRUE);/* only to check the bit --> no 'busy wait' */
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger the bit : <Load All Global Drop Counters > */
        rc = prvCpssHwPpSetRegField(devNum, regAddr,0 , 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* wait that all port groups are ready */
        rc = prvCpssPortGroupBusyWait(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    regAddr,0,/*bit0 - <Load All Global Drop Counters */
                    GT_FALSE);/* 'busy wait' on the bit*/
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        maxPriority = 4;
    }
    else
    {
        countersArr[3].l[0] = 0;
        countersArr[3].l[1] = 0;
        maxPriority = 3;
    }

    for(priority = 0 ; priority < maxPriority ; priority++)
    {
        rc = pipGlobalDropCounterGet(devNum,priority,&countersArr[priority]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;

}
/**
* @internal cpssDxChPortPipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get all PIP drop counters (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*
* @param[out] countersArr[4]           - (pointer to) the counters of
*                                      - 3 priorities (0-high, 1-mid, 2-low) for Bobcat2,
*                                      Caelum, Aldrin, AC3X
*                                      - 4 priorities (0-very high, 1-high, 2-mid, 3-low) for
*                                      Bobcat3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalDropCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, countersArr));

    rc = internal_cpssDxChPortPipGlobalDropCounterGet(devNum, countersArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, countersArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific port.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    /* the counter supports only 32 bits */
    counterPtr->l[1] = 0;

    return pip_counterGetPerPort(devNum,portNum,&counterPtr->l[0]);
}

/**
* @internal cpssDxChPortPipDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific port.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipDropCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterPtr));

    rc = internal_cpssDxChPortPipDropCounterGet(devNum, portNum, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortParserGlobalTpidSet function
* @endinternal
*
* @brief   Set the TPID and it's size to allow the 'Rx parser' to 'jump over' up
*           to 4 such tags in order to recognize upper layer protocol.
*           (Global to the device - not 'per port')
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - index to one of 4 TPIDs
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  the TPID to recognize the protocols packets
* @param[in] tpidSize                 - the TPID size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index or tpidSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortParserGlobalTpidSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_U16       etherType,
    IN CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize
)
{
    GT_U32  regAddr;
    GT_U32  hw_tpidSize;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    /* check index*/
    CHECK_PARAM_INDEX_MAC(index,4);
    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.profileTPID[index];

    switch(tpidSize)
    {
        case CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E:
            hw_tpidSize = 1;
            break;
        case CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E:
            hw_tpidSize = 2;
            break;
        case CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E:
            hw_tpidSize = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(tpidSize);

    }

    /*<etherType> <TPID Tag Size>*/
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16 + 2, etherType | (hw_tpidSize << 16));
}

/**
* @internal cpssDxChPortParserGlobalTpidSet function
* @endinternal
*
* @brief   Set the TPID and it's size to allow the 'Rx parser' to 'jump over' up
*           to 4 such tags in order to recognize upper layer protocol.
*           (Global to the device - not 'per port')
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - index to one of 4 TPIDs
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  the TPID to recognize the protocols packets
* @param[in] tpidSize                 - the TPID size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index or tpidSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortParserGlobalTpidSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_U16       etherType,
    IN CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortParserGlobalTpidSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, etherType, tpidSize));

    rc = internal_cpssDxChPortParserGlobalTpidSet(devNum, index, etherType, tpidSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, etherType, tpidSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortParserGlobalTpidGet function
* @endinternal
*
* @brief   Get the TPID and it's size to allow the 'Rx parser' to 'jump over' up
*           to 4 such tags in order to recognize upper layer protocol.
*           (Global to the device - not 'per port')
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - index to one of 4 TPIDs
*                                      (APPLICABLE RANGES: 0..3)
* @param[out] etherTypePtr            - (pointer to) the TPID to recognize the protocols packets.
* @param[out] tpidSizePtr             - (pointer to) the TPID size.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortParserGlobalTpidGet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr,
    OUT CPSS_BRG_TPID_SIZE_TYPE_ENT  *tpidSizePtr
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  regValue;
    GT_U32  hw_tpidSize;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(tpidSizePtr);

    /* check index*/
    CHECK_PARAM_INDEX_MAC(index,4);
    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.profileTPID[index];

    /*<etherType> <TPID Tag Size>*/
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*<etherType>*/
    *etherTypePtr = (GT_U16)regValue;

    hw_tpidSize = (regValue >> 16) & 0x3;
    /*<TPID Tag Size>*/
    switch(hw_tpidSize)
    {
        case 1:
            *tpidSizePtr = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;
            break;
        case 2:
            *tpidSizePtr = CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E;
            break;
        case 3:
            *tpidSizePtr = CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"unexpected value [%d] read from HW for field <TPID Tag Size>",
                hw_tpidSize);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortParserGlobalTpidGet function
* @endinternal
*
* @brief   Get the TPID and it's size to allow the 'Rx parser' to 'jump over' up
*           to 4 such tags in order to recognize upper layer protocol.
*           (Global to the device - not 'per port')
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - index to one of 4 TPIDs
*                                      (APPLICABLE RANGES: 0..3)
* @param[out] etherTypePtr            - (pointer to) the TPID to recognize the protocols packets.
* @param[out] tpidSizePtr             - (pointer to) the TPID size.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortParserGlobalTpidGet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr,
    OUT CPSS_BRG_TPID_SIZE_TYPE_ENT  *tpidSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortParserGlobalTpidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, etherTypePtr, tpidSizePtr));

    rc = internal_cpssDxChPortParserGlobalTpidGet(devNum, index, etherTypePtr, tpidSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, etherTypePtr, tpidSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet function
* @endinternal
*
* @brief   Set the number of randomized least significant PIP FIFO fill level
*          bits that are compared with PIP Prio Thresholds configuration.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] numLsb                - the number of LSBits to be randomized.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range numLsb
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet
(
    IN GT_U8                devNum,
    IN GT_U32               numLsb
)
{
    GT_STATUS rc;         /* return code */
    GT_U32 regAddr;       /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    CHECK_DATA_INDEX_MAC(numLsb,BIT_4);

    regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholdsRandomization;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, numLsb);

    return rc;
}
/**
* @internal cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet function
* @endinternal
*
* @brief   Set the number of randomized least significant PIP FIFO fill level
*          bits that are compared with PIP Prio Thresholds configuration.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] numLsb                - the number of LSBits to be randomized.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range numLsb
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet
(
    IN GT_U8                devNum,
    IN GT_U32               numLsb
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numLsb));

    rc = internal_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(devNum, numLsb);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numLsb));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet function
* @endinternal
*
* @brief   Get the number of randomized least significant PIP FIFO fill level
*          bits that are compared with PIP Prio Thresholds configuration.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] numLsbPtr             - (pointer to) the number of LSBits to be randomized.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet
(
    IN GT_U8                devNum,
    OUT GT_U32               *numLsbPtr
)
{
    GT_U32 regAddr;       /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(numLsbPtr);

    regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholdsRandomization;

    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, numLsbPtr);

}

/**
* @internal cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet function
* @endinternal
*
* @brief   Get the number of randomized least significant PIP FIFO fill level
*          bits that are compared with PIP Prio Thresholds configuration.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] numLsbPtr             - (pointer to) the number of LSBits to be randomized.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet
(
    IN GT_U8                devNum,
    OUT GT_U32               *numLsbPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numLsbPtr));

    rc = internal_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(devNum, numLsbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numLsbPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalThresholdSet function
* @endinternal
*
* @brief   Set Threshold for specific PIP Threshold type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp         - bitmap of port groups
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi port groups (or pipes) device:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] thresholdType            - the type of threshold
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portGroupsBmp or thresholdType
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalThresholdSet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType,
    IN GT_U32           threshold
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;       /* register address */
    GT_U32 fieldOffset;   /* offset of threshold in the register*/
    GT_U32  portGroupId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* check threshold */
    CHECK_DATA_INDEX_MAC(threshold,BIT_16);

    switch(thresholdType)
    {
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_LATENCY_SENSITIVE_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPLsThreshold;
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_LOW_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPPfcGlobalFillLevelThresholds;
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_HIGH_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPPfcGlobalFillLevelThresholds;
            fieldOffset = 16;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPPfcCounterThresholds;
            fieldOffset = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(thresholdType);
    }

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* the GM not supports the register */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"the register not supported by the device \n");
    }

    /*
        NOTE: the loop on port groups requires calling prvCpssHwPpPortGroupSetRegField
        and in function per_pipe_UnitDuplicatedMultiPortGroupsGet the 'address' in pipe 0
        will be translated to the other pipe.
    */

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
             regAddr, fieldOffset, 16, threshold);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChPortPipGlobalThresholdSet function
* @endinternal
*
* @brief   Set Threshold for specific PIP Threshold type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp         - bitmap of port groups
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi port groups (or pipes) device:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] thresholdType            - the type of threshold
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portGroupsBmp or thresholdType
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalThresholdSet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType,
    IN GT_U32           threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, thresholdType, threshold));

    rc = internal_cpssDxChPortPipGlobalThresholdSet(devNum, portGroupsBmp, thresholdType, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, thresholdType, threshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalThresholdGet function
* @endinternal
*
* @brief   Get Threshold for specific PIP Threshold type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp         - bitmap of port groups
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi port groups (or pipes) device:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] thresholdType            - the type of threshold
* @param[out] thresholdPtr            - (pointer to)the  value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portGroupsBmp or thresholdType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalThresholdGet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType,
    OUT GT_U32           *thresholdPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 fieldOffset;   /* offset of threshold in the register*/
    GT_U32  portGroupId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    switch(thresholdType)
    {
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_LATENCY_SENSITIVE_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPLsThreshold;
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_LOW_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPPfcGlobalFillLevelThresholds;
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_HIGH_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPPfcGlobalFillLevelThresholds;
            fieldOffset = 16;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPPfcCounterThresholds;
            fieldOffset = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(thresholdType);
    }

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* the GM not supports the register */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"the register not supported by the device \n");
    }

    /* read from the first port groups available */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);

    return prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
         regAddr, fieldOffset, 16, thresholdPtr);
}
/**
* @internal cpssDxChPortPipGlobalThresholdGet function
* @endinternal
*
* @brief   Get Threshold for specific PIP Threshold type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp         - bitmap of port groups
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      NOTEs:
*                                      1. for non multi port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi port groups (or pipes) device:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] thresholdType            - the type of threshold
* @param[out] thresholdPtr            - (pointer to)the  value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portGroupsBmp or thresholdType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalThresholdGet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType,
    OUT GT_U32           *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, thresholdType, thresholdPtr));

    rc = internal_cpssDxChPortPipGlobalThresholdGet(devNum, portGroupsBmp, thresholdType, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, thresholdType, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalPfcTcVectorSet function
* @endinternal
*
* @brief   Set 'TC vector' for PFC generation for specific PIP congestion type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] vectorType            - the type of TC vector
* @param[in] tcVector              - the TC vector. (bitmap of TCs : 8 bits)
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or vectorType
* @retval GT_OUT_OF_RANGE          - on out of range tcVector
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalPfcTcVectorSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType,
    IN GT_U32               tcVector
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;       /* register address */
    GT_U32 fieldOffset;   /* offset of threshold in the register*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    /* check TC vector */
    CHECK_DATA_INDEX_MAC(tcVector,BIT_8);

    switch(vectorType)
    {
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_LOW_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[0];
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_MEDIUM_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[0];
            fieldOffset = 16;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_HIGH_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[1];
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_VERY_HIGH_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[1];
            fieldOffset = 16;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_NO_CONGESTION_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.GlobalTcPfcTriggerVectorsConfig;
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.GlobalTcPfcTriggerVectorsConfig;
            fieldOffset = 16;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(vectorType);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 8, tcVector);

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalPfcTcVectorSet function
* @endinternal
*
* @brief   Set 'TC vector' for PFC generation for specific PIP congestion type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] vectorType            - the type of TC vector
* @param[in] tcVector              - the TC vector. (bitmap of TCs : 8 bits)
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or vectorType
* @retval GT_OUT_OF_RANGE          - on out of range tcVector
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalPfcTcVectorSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType,
    IN GT_U32               tcVector
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalPfcTcVectorSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vectorType, tcVector));

    rc = internal_cpssDxChPortPipGlobalPfcTcVectorSet(devNum, vectorType, tcVector);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vectorType, tcVector));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPipGlobalPfcTcVectorGet function
* @endinternal
*
* @brief   Get 'TC vector' for PFC generation for specific PIP congestion type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] vectorType            - the type of TC vector
* @param[out] tcVectorPtr           - (pointer to)the TC vector. (bitmap of TCs : 8 bits)
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or vectorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalPfcTcVectorGet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType,
    OUT GT_U32               *tcVectorPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 fieldOffset;   /* offset of threshold in the register*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);
    /* check if the IA unit supported ! */
    IS_IA_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(tcVectorPtr);

    switch(vectorType)
    {
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_LOW_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[0];
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_MEDIUM_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[0];
            fieldOffset = 16;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_HIGH_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[1];
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_VERY_HIGH_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.PIPTcPfcTriggerVectorsConfig[1];
            fieldOffset = 16;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_NO_CONGESTION_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.GlobalTcPfcTriggerVectorsConfig;
            fieldOffset = 0;
            break;
        case CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E:
            regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).pfcTriggering.GlobalTcPfcTriggerVectorsConfig;
            fieldOffset = 16;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(vectorType);
    }

    return prvCpssHwPpGetRegField(devNum,
         regAddr, fieldOffset, 8, tcVectorPtr);
}

/**
* @internal cpssDxChPortPipGlobalPfcTcVectorGet function
* @endinternal
*
* @brief   Get 'TC vector' for PFC generation for specific PIP congestion type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] vectorType            - the type of TC vector
* @param[out] tcVectorPtr           - (pointer to)the TC vector. (bitmap of TCs : 8 bits)
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or vectorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalPfcTcVectorGet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType,
    OUT GT_U32               *tcVectorPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalPfcTcVectorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vectorType, tcVectorPtr));

    rc = internal_cpssDxChPortPipGlobalPfcTcVectorGet(devNum, vectorType, tcVectorPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vectorType, tcVectorPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxDmaGlobalDropCounterModeSet function
* @endinternal
*
* @brief   Set the RXDMA drop counter mode info (what drops to count)
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] counterModeInfoPtr    - (pointer to) counter mode info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or counterModeInfoPtr->modesBmp
* @retval GT_OUT_OF_RANGE          - on out of range
*                                       counterModeInfoPtr->portNumPattern  or
*                                       counterModeInfoPtr->portNumMask     or
*                                       counterModeInfoPtr->priorityPattern or
*                                       counterModeInfoPtr->priorityMask
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxDmaGlobalDropCounterModeSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  *counterModeInfoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 regAddr;         /* register address */
    GT_U32 mask     = BIT_1 | BIT_2 | BIT_3 | BIT_5;/* reserved drop modes that 'must not count' */
    GT_U32 pattern  = 0;
    GT_U32  globalDmaNum; /* (global) rxdma port number  */
    GT_U32  localDmaNum = 0; /* local rxdma port number  */
    GT_U32  dpIndex = GT_NA;/* indication for a specific DMA that counting for specific 'local DMA port' */
    GT_U32  ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(counterModeInfoPtr);

    if(counterModeInfoPtr->modesBmp & ~(CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PIP_E |
                                        CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PACKET_BUFFER_FILL_LEVEL_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"modesBmp [0x%x] set with unknown 'modes'",
            counterModeInfoPtr->modesBmp);
    }

    if(counterModeInfoPtr->modesBmp & CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PIP_E)
    {
        /* we want to count 'PIP' drops */
        /* mask bit 0 is 0 */
    }
    else
    {
        /* we NOT want to count 'PIP' drops */

        /* set mask bit 0 */
        mask |= BIT_0;
        /*pattern bit 0 is 0*/
    }

    if(counterModeInfoPtr->modesBmp & CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PACKET_BUFFER_FILL_LEVEL_E)
    {
        /* we want to count 'PB FL' drops */
        /* mask bit 4 is 0 */
    }
    else
    {
        /* we NOT want to count 'PB FL' drops */

        /* set mask bit 4 */
        mask |= BIT_4;
        /*pattern bit 4 is 0*/
    }

    if(counterModeInfoPtr->priorityMask >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"priorityMask [0x%x] value must be 0..3",
            counterModeInfoPtr->priorityMask);
    }

    if(counterModeInfoPtr->priorityMask)
    {
        if(counterModeInfoPtr->priorityPattern >= BIT_2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"priorityPattern [0x%x] value must be 0..3 (when priorityMask != 0)",
                counterModeInfoPtr->priorityPattern);
        }

        /* set mask    bits 6..7 as requested */
        mask    |= counterModeInfoPtr->priorityMask     << 6;
        /* set pattern bits 6..7 as requested */
        pattern |= counterModeInfoPtr->priorityPattern  << 6;
    }
    else
    {
        /* mask    bits 6..7 are 0 */
        /* pattern bits 6..7 are 0 */
    }


    if(counterModeInfoPtr->portNumMask == 0)
    {
        /* we don't care about the 'portNumPattern' */
        /* mask of bits 8..15 is 0 --> meaning count all ports */
    }
    else
    if(counterModeInfoPtr->portNumMask == 0xFFFFFFFF)
    {
        /* check and convert physical portNum to global portRxdmaNum */
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum,counterModeInfoPtr->portNumPattern,globalDmaNum);

        /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
            globalDmaNum,&dpIndex,&localDmaNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* mask of bits 8..13 is 0x3F --> meaning count specific port */
        mask    |=       0x3F << 8;
        /* pattern of bits 8..13 is 'local DMA' (local_portRxdmaNum) for the
           'actual DMA' and DO NOT count for other units !!! */
        pattern |= localDmaNum << 8;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"portNumMask [0x%x] must be 0x0 or 0xFFFFFFFF",
            counterModeInfoPtr->portNumMask);
    }

    if(dpIndex != GT_NA)
    {
        /* this unit need specific value that differs from all other units that 'must not count' */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum,dpIndex).
            debug.rxIngressDropCounter[0].rx_ingress_drop_count_type_ref;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, pattern);
        if(rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum,dpIndex).
            debug.rxIngressDropCounter[0].rx_ingress_drop_count_type_mask;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, mask);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* by those values we cause 'count nothing' because no drop reason is
           'all drop' reasons (bits 0..5) */
        mask    = 0xFFFFFFFF;
        pattern = 0xFFFFFFFF;
    }


    /* loop on all RXDMA units and set each with it's value */
    for(ii = 0 ; ii < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp; ii++)
    {
        if(dpIndex == ii)
        {
            /* this DP gets different value than the others */
            /* and was already set outsize this loop */
            continue;
        }

        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum,ii).
            debug.rxIngressDropCounter[0].rx_ingress_drop_count_type_ref;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, pattern);
        if(rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum,ii).
            debug.rxIngressDropCounter[0].rx_ingress_drop_count_type_mask;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, mask);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* save the info to return on the 'Get' API */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.rxdmaDropCounterModeInfo = *counterModeInfoPtr;

    return GT_OK;
}

/**
* @internal cpssDxChPortRxDmaGlobalDropCounterModeSet function
* @endinternal
*
* @brief   Set the RXDMA drop counter mode info (what drops to count)
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] counterModeInfoPtr    - (pointer to) counter mode info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or counterModeInfoPtr->modesBmp
* @retval GT_OUT_OF_RANGE          - on out of range
*                                       counterModeInfoPtr->portNumPattern  or
*                                       counterModeInfoPtr->portNumMask     or
*                                       counterModeInfoPtr->priorityPattern or
*                                       counterModeInfoPtr->priorityMask
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxDmaGlobalDropCounterModeSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  *counterModeInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxDmaGlobalDropCounterModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterModeInfoPtr));

    rc = internal_cpssDxChPortRxDmaGlobalDropCounterModeSet(devNum, counterModeInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterModeInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxDmaGlobalDropCounterModeGet function
* @endinternal
*
* @brief   Get the RXDMA drop counter mode info (what drops to count)
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] counterModeInfoPtr   - (pointer to) counter mode info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unexpected mode/value read from the HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxDmaGlobalDropCounterModeGet
(
    IN GT_U8            devNum,
    OUT CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  *counterModeInfoPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(counterModeInfoPtr);

    *counterModeInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.rxdmaDropCounterModeInfo;
    return GT_OK;
}

/**
* @internal cpssDxChPortRxDmaGlobalDropCounterModeGet function
* @endinternal
*
* @brief   Get the RXDMA drop counter mode info (what drops to count)
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] counterModeInfoPtr   - (pointer to) counter mode info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unexpected mode/value read from the HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxDmaGlobalDropCounterModeGet
(
    IN GT_U8            devNum,
    OUT CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  *counterModeInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxDmaGlobalDropCounterModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterModeInfoPtr));

    rc = internal_cpssDxChPortRxDmaGlobalDropCounterModeGet(devNum, counterModeInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterModeInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxDmaGlobalDropCounterGet function
* @endinternal
*
* @brief  Get the RXDMA drop counter value.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon; AC5P; Harrier)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[out] counterPtr              - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, dataPathBmp
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxDmaGlobalDropCounterGet
(
    IN  GT_U8             devNum,
    IN  GT_DATA_PATH_BMP  dataPathBmp,
    OUT GT_U64            *counterPtr
)
{
    GT_U32     regAddr;    /* register address   */
    GT_U64     msbCounter; /* buffer for counter */
    GT_STATUS  rc;         /* return code        */
    GT_U32     regValue;   /* register value */
    GT_U32     ii;         /* data path index iterator */
    GT_U32     lDpSize;    /* maximum data paths size */
    GT_U32     dataPathBmpCalc; /* calculated bitmap */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check if pip supported by the device */
    IS_PIP_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    /* Single DP devices(AC5X,Ironman) ignore dataPathBmp */
    if(lDpSize == 1)
    {
        dataPathBmp = CPSS_DATA_PATH_UNAWARE_MODE_CNS;
    }
    else
    {
        if(lDpSize<32)
        {
            if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
            {
               dataPathBmp = (1<<lDpSize)-1;
            }

            if(dataPathBmp>>lDpSize)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
            {
                dataPathBmp=(GT_U32)-1;
            }
        }
        if(dataPathBmp == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* init output value */
    counterPtr->l[0] =
    counterPtr->l[1] = 0;

    /* store value for second loop */
    dataPathBmpCalc = dataPathBmp;

    /* loop through all the DP indexes */
    for(ii = 0; dataPathBmp && (ii < lDpSize); ++ii,dataPathBmp>>=1)
    {
        if(dataPathBmp & 1)
        {
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, ii).
                        debug.rxIngressDropCounter[0].rx_ingress_drop_count;

            /* the counter for the respective DP unit */
            rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regValue);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* the counter accumulated from all the DP units as per dataPathBmp */
            counterPtr->l[0] += regValue;
        }
    }

    if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* done for Falcon */
        return GT_OK;
    }

    regValue = 0;
    msbCounter.l[0] = 0;

    /* get saved value */
    dataPathBmp = dataPathBmpCalc;

    for(ii = 0; dataPathBmp && (ii < lDpSize); ++ii,dataPathBmp>>=1)
    {
        if(dataPathBmp & 1)
        {
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, ii).
                        debug.rxIngressDropCounter[0].rx_ingress_drop_count_high;

            /* the counter for the respective DP unit */
            rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regValue);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* the counter accumulated from all the DP units as per dataPathBmp */
            msbCounter.l[0] += regValue;
        }
    }

    /* LSB of msbCounter holds summary of counter's MSBs.
       Assign this value to MSB of output counter. */
    counterPtr->l[1] = msbCounter.l[0];

    return GT_OK;
}

/**
* @internal cpssDxChPortRxDmaGlobalDropCounterGet function
* @endinternal
*
* @brief  Get the RXDMA drop counter value.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon; AC5P; Harrier)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[out] counterPtr              - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, dataPathBmp
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxDmaGlobalDropCounterGet
(
    IN  GT_U8             devNum,
    IN  GT_DATA_PATH_BMP  dataPathBmp,
    OUT GT_U64            *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxDmaGlobalDropCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dataPathBmp, counterPtr));

    rc = internal_cpssDxChPortRxDmaGlobalDropCounterGet(devNum, dataPathBmp, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dataPathBmp, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


