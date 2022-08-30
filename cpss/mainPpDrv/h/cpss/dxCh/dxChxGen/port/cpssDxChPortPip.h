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
* @file cpssDxChPortPip.h
*
* @brief CPSS DxCh Port's Pre-Ingress Prioritization (PIP) .
*
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortPip
#define __cpssDxChPortPip

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
/* include to get <CPSS_BRG_TPID_SIZE_TYPE_ENT> */
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>

/**
* @enum CPSS_DXCH_PORT_PIP_PROTOCOL_ENT
 *
 * @brief Enumeration of PIP type of supported protocols
 * (etherTypes to identify protocol).
*/
typedef enum{

    /** VLAN tag (TPID) */
    CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E,

    /** MPLS protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E   ,

    /** IPV4 protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E   ,

    /** IPV6 protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E   ,

    /** 'user defined ethertype' protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E   ,

    /** @brief : Rx unit parser : 'latency sensitive' protocol.(such as QCN and PFC)
        such traffic uses dedicated latency sensitive FIFO

        APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
    */
    CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E ,

    CPSS_DXCH_PORT_PIP_PROTOCOL___LAST___E
} CPSS_DXCH_PORT_PIP_PROTOCOL_ENT;

/**
* @enum CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT
 *
 * @brief Enumeration for PIP classification of packet for priority assignment (per port).
*/
typedef enum{

    /** packet with matched etherType and vid assign priority per port */
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E,

    /** packet that was not classified assigned 'default' priority per port */
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E

} CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
 *
 * @brief Enumeration for PIP classification of packet for priority assignment (per profile).
*/
typedef enum{

    /** @brief packet with DSA TAG and <QOS PROFILE> assign priority per PIP profile
     *  per <QOS PROFILE> (APPLICABLE RANGES: 0..127)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E,

    /** @brief packet with DSA TAG assign priority per PIP profile
     *  per <UP> (APPLICABLE RANGES: 0..7)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E,

    /** @brief packet with matched VLAN TAG assign priority per PIP profile
     *  per <UP> (APPLICABLE RANGES: 0..7)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E,

    /** @brief packet with MPLS assign priority per PIP profile
     *  per <EXP> (APPLICABLE RANGES: 0..7)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E,

    /** @brief packet with IPV4 assign priority per PIP profile
     *  per <TOS> (APPLICABLE RANGES: 0..255)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E,

    /** @brief packet with IPV6 assign priority per PIP profile
     *  per <TC> (APPLICABLE RANGES: 0..255)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E,

    /** @brief packet with matched UDE (user defined ethertype) assign priority per PIP profile
     *  per <ude index> (APPLICABLE RANGES: 0..3)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E,

    /** @brief packet with matched MAC DA assign priority per PIP profile
     *  per <macDa index> (APPLICABLE RANGES: 0..3)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E

} CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT;

/**
* @struct CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC
 *
 * @brief structure for PIP MAC DA classification info
*/
typedef struct{

    /** the mac DA value */
    GT_ETHERADDR macAddrValue;

    /** the mac DA mask */
    GT_ETHERADDR macAddrMask;

} CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC;


/**
* @enum CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT
 *
 * @brief Enumeration for PIP threshold types.
 *       APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef enum{
    /* @brief threshold for 'latency sensitive' : To absorb short bursts of latency
     *   sensitive packets when the device is oversubscribed, a FIFO maintains
     *   a fill level and a threshold */
     CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_LATENCY_SENSITIVE_E
    /* @brief threshold for 'pfc fill level low' : Determines the FIFOs global fill
     *   level value (maximum fill level among all Rx FIFOs) below which FIFOs
     *   are considered to be not congested. PIP PFC Global Fill Level Low
     *   Threshold should be configured to a value lower or equal to PIP PFC
     *   Global Fill Level High Threshold*/
    ,CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_LOW_E
    /* @brief threshold for 'pfc fill level high' : Determines the FIFOs global fill
     *   level value (maximum fill level among all Rx FIFOs) above which FIFOs
     *   are considered to be heavily congested. PIP PFC Global Fill Level High
     *   Threshold should be configured to a value higher or equal to PIP PFC
     *   Global Fill Level Low Threshold*/
    ,CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_FILL_LEVEL_HIGH_E
    /* @brief threshold for 'pfc counter' : Determines the PIP counters value threshold
     *  for PFC trigger activation at lightly congested FIFO state */
    ,CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E

}CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT;


/**
* @enum CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT
 *
 * @brief Enumeration for PIP PFC to TC vector types.
 *       APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef enum{
    /* @brief vector for PFC to TC for PIP low priority traffic :
     *   Low Priority PIP to TC association affective for PFC trigger
     *   while FIFOs are lightly congested */
     CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_LOW_E
    /* @brief vector for PFC to TC for PIP medium priority traffic :
     *   Medium Priority PIP to TC association affective for PFC trigger
     *   while FIFOs are lightly congested */
    ,CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_MEDIUM_E
    /* @brief vector for PFC to TC for PIP high priority traffic :
     *   High Priority PIP to TC association affective for PFC trigger
     *   while FIFOs are lightly congested */
    ,CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_HIGH_E
    /* @brief vector for PFC to TC for PIP very high priority traffic :
     *   Very High Priority PIP to TC association affective for PFC trigger
     *   while FIFOs are lightly congested */
    ,CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_PRIORITY_VERY_HIGH_E
    /* @brief vector for PFC to TC for no congestion :
     *   Definition of TC PFC trigger while FIFOs are not congested.
     *   This vector is also affective if any PIP counter value has
     *   not passed threshold for PFC trigger activation at lightly
     *   congested FIFO state */
    ,CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_NO_CONGESTION_E
    /* @brief vector for PFC to TC for heavy congestion traffic
     *  Definition of TC PFC trigger while FIFOs are heavily congested */
    ,CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E

}CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT;

/**
* @enum CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODES_ENT
 *
 * @brief Enumeration for RXDMA drop counter modes.
 *          the modes can be aggregated (to bitmap of 'modes')
 *       APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef enum{
    /* @brief count PIP related drops */
     CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PIP_E                      = BIT_0
    /* @brief count Packet Buffer fill level drops */
    ,CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PACKET_BUFFER_FILL_LEVEL_E = BIT_1

}CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODES_ENT;


/**
* @struct CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC
 *
 * @brief structure for RXDMA drop counter mode info
 *       APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    /* @brief bitmap of modes to count RXDMA drops
     * combination of modes from  CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODES_ENT  */
    GT_U32           modesBmp;
    /* @brief pattern for 'physical port number' that relate to the 'drops'
     * ignored if portNumMask == 0x0 , otherwise must be valid 'physical port number'
     * (hold 'port mapping' to RXDMA port number) */
    GT_PHYSICAL_PORT_NUM portNumPattern;
    /* @brief mask    for 'physical port number' that relate to the 'drops'
     *   NOTE: supported values are limited to :
     *       0x0        - meaning count for ALL ports
     *       0xFFFFFFFF - meaning count only for port 'portNumPattern' */
    GT_PHYSICAL_PORT_NUM portNumMask;
    /* @brief pattern for 'priority' that relate to the 'drops'
     * ignored if priorityMask == 0x0
     *  value 0 - low       priority
     *  value 1 - medium    priority
     *  value 2 - high      priority
     *  value 3 - very high priority
     *  APPLICABLE RANGE: 0..3 (2 bits)
     */
    GT_U32           priorityPattern;
    /* @brief mask for 'priority' that relate to the 'drops'
     *  APPLICABLE RANGE: 0..3 (2 bits)
     */
    GT_U32           priorityMask;
}CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC;


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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);


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
);

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
);

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
);


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
);


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
);

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
);

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
);

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
);

/**
* @internal cpssDxChPortPipDropCounterGet function
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
GT_STATUS cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
);

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
);

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
);

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
);

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
);

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
);

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
);


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
);

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
);
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
*
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
    IN GT_U8             devNum,
    IN  GT_DATA_PATH_BMP dataPathBmp,
    OUT GT_U64           *counterPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortPip */


