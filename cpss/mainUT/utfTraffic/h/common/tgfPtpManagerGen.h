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
* @file tgfPtpManagerGen.h
*
* @brief Generic API for Presision Time Protocol (PTP) Manager API.
*
* @version   1
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfPtpManagerGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfPtpManagerGenh
#define __tgfPtpManagerGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <common/tgfPtpGen.h>
#endif /* CHX_FAMILY */

/**
* @enum PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT
*
* @brief PTP output interface mode.
*/
typedef enum{

    /** @brief there is no output signal configured.
     */
    PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E,

    /** @brief PTP clock output mode.
     */
    PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E,

    /** @brief PPS master output mode.
     */
    PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E,

} PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT;

/**
* @struct PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC
*
* @brief Structure for clock output interface configuration.
*/
typedef struct{

    /** @brief TAI number.
     */
    PRV_TGF_PTP_TAI_NUMBER_ENT    taiNumber;

    /** @brief PTP output interface mode.
     */
    PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT    outputInterfaceMode;

    /** @brief PTP clock nano seconds cycle.
     *  applicable only if outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E
     */
    GT_U32 nanoSeconds;

} PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC;

/**
* @struct PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC
*
* @brief Structure for PTP over Ethernet configuration.
*/
typedef struct{

    /** @brief  enable/disbale PTP over Ethernet frames identification
     */
    GT_BOOL ptpOverEthernetEnable;

    /** @brief ethertype0 of PTP over Ethernet packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverEthernetEnable=GT_TRUE
     */
    GT_U32  etherType0value;

    /** @brief ethertype1 of PTP over Ethernet packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverEthernetEnable=GT_TRUE
     */
    GT_U32  etherType1value;

} PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC;

/**
* @struct PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC
*
* @brief Structure for PTP over IP/UDP configuration.
*/
typedef struct{

    /** @brief  enable/disbale PTP over IP/UDP frames identification
     */
    GT_BOOL ptpOverIpUdpEnable;

    /** @brief UDP destination port0 of PTP over UDP packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverIpUdpEnable=GT_TRUE
     */
    GT_U32  udpPort0value;

    /** @brief UDP destination port1 of PTP over UDP packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverIpUdpEnable=GT_TRUE
     */
    GT_U32  udpPort1value;

    /** @brief  Timestamp UDP Checksum Update Mode.
     *          applicable only when ptpOverIpUdpEnable=GT_TRUE
     */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC udpCsUpdMode;

} PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC;

/**
* @struct PRV_TGF_PTP_MANAGER_CAPTURE_TOD_VALUE_STC
*
* @brief Structure for captured TOD value.
*/
typedef struct{

    /** @brief  whether captured TOD value is valid
     */
    GT_BOOL todValueIsValid;

    /** @brief captured TOD value
     */
    PRV_TGF_PTP_TOD_COUNT_STC todValue;

} PRV_TGF_PTP_MANAGER_CAPTURE_TOD_VALUE_STC;

/**
* @struct PRV_TGF_PTP_MANAGER_DOMAIN_STC
*
* @brief Structure for PTP domain configuration.
*/
typedef struct{

    /** @brief domain index. (APPLICABLE RANGES: 0..3)
     *  the domain number (values up to 255 from the PTP header) is
     *  associated with an internal domain index.
     */
    GT_U32  domainIndex;

    /** @brief PTP protocol (v1 or v2).
     */
    PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT  domainMode;

    /** @brief PTP V1 domain id. applicable only when
     *         domainMode=PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E
     *  PTP V1 Header contains 128-bit domain Id.
     *  4 domain Id values are mapped to domainIndex 0-3,
     *  all other domain Id values are mapped to default domain.
     */
    GT_U32  v1DomainIdArr[4];

    /** @brief PTP V2 domain id. applicable only when
     *         domainMode=PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E
     *  PTP V2 Header contains 8-bit domain Id.
     *  4 domain Id values are mapped to domainIndex 0-3,
     *  all other domain Id values are mapped to default domain.
     */
    GT_U32  v2DomainId;

} PRV_TGF_PTP_MANAGER_DOMAIN_STC;

/**
* @struct PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC
 *
 * @brief Structure Timestamp Tag Global Configuration.
*/
typedef struct{

    /** @brief GT_TRUE
     *  as timestamp tagged with Extended or Non-Extended Tag.
     *  GT_FALSE - no packets are identified
     *  as timestamp tagged with Extended or Non-Extended Tag.
     */
    GT_BOOL tsTagParseEnable;

    /** @brief GT_TRUE
     *  are identified as timestamp tagged with Hybrid Tag.
     *  GT_FALSE - no packets
     *  are identified as timestamp tagged with Hybrid Tag.
     */
    GT_BOOL hybridTsTagParseEnable;

    /** @brief Ethertype of Extended and Non Extended TS Tags.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 tsTagEtherType;

    /** @brief Ethertype of Hybrid TS Tags.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 hybridTsTagEtherType;

} PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC;

/**
* @enum PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT
*
* @brief PTP message type.
*/
typedef enum{

    /** Sync */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,

    /** Delay_request */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,

    /** Pdelay_request */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,

    /** Pdelay_Response */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,

    /** Reserved4 */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED4_E,

    /** Reserved5 */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED5_E,

    /** Reserved6 */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED6_E,

    /** Reserved7 */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED7_E,

    /** Follow_up */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E,

    /** Delay_response */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,

    /** Pdelay_Response_follow_up */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_FOLLOW_UP_E,

    /** Announce */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ANNOUNCE_E,

    /** Signaling */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_SIGNALING_E,

    /** Management */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_MANAGMENT_E,

    /** Reserved14 */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED14_E,

    /** Reserved15 */
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED15_E,

    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_LAST_E,

} PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT;

/**
* @struct PRV_TGF_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC
*
*  @brief Structure for TSU (Time Stamping Unit) TX Timestamp Queue Entry.
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief This field is used to match between the egress timestamp
     *  queue entry and the per-port timestamp entry.
     *  The same <Queue Entry ID> is also stored in the
     *  per-port timestamp queue.
     */
    GT_U32 queueEntryId;

    /** selected TAI. (APPLICABLE RANGES: 0..1.) */
    GT_U32 taiSelect;

    /** @brief The least significant bit of the <Time Update Counter>
     *  in the TAI, at the time that <Timestamp> is captured.
     */
    GT_U32 todUpdateFlag;

     /** @brief The 2 lsbits of the seconds value of the TOD
     *  counter at the times of transmission/reception of the packet.
     *  (APPLICABLE RANGES: 0..3)
     */
    GT_U32 seconds;

    /** @brief The 30 lsbits of the nanoseconds value of the TOD
     *  counter at the times of transmission/reception of the
     *  packet. (APPLICABLE RANGES: 0..2^30-1)
     */
    GT_U32 nanoSeconds;

} PRV_TGF_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @internal prvTgfPtpManagerPtpInit function
* @endinternal
*
* @brief  Set one-time global PTP-related settings.
*
* @param[in] devNum                   - device number
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*                                       The command is configured in cpssDxChPtpManagerPortCommandAssignment
*                                       per{source port, domain index, message type}.
*                                       Applicable if command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E or
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS prvTgfPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC    *outputInterfaceConfPtr,
    IN PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC       *ptpOverEthernetConfPtr,
    IN PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC         *ptpOverIpUdpConfPtr,
    IN PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC       *ptpTsTagGlobalConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
);

/**
* @internal prvTgfPtpManagerInputPulseSet function
* @endinternal
*
* @brief   Set input pulse configuration.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] inputMode                - Whether an incoming pulse comes from the PULSE pin (slave PPS) or the CLK pin.
*                                       (APPLICABLE VALUES:
*                                           PRV_TGF_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,
*                                           PRV_TGF_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  PRV_TGF_PTP_TAI_PULSE_IN_MODE_ENT     inputMode
);

/**
* @internal prvTgfPtpManagerMasterPpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Master PPS.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS prvTgfPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);

/**
* @internal prvTgfPtpManagerSlavePpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave PPS.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS prvTgfPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);

/**
* @internal prvTgfPtpManagerPortAllocateTai function
* @endinternal
*
* @brief   Allocating Port Interface to TAI.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] taiNumber                - TAI number.
* @param[in] egressPortBindMode       - determines the mode in which egress port is bound to TAI,
*                                       either static or dynamic per packet.
*                                       (APPLICABLE VALUES:
*                                           PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E, (dynamic bind, per ingress port of a specific packet)
*                                           PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E)  (static bind per taiNumber param)
*                                       NOTE: for high accuracy use PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT                 taiNumber,
    IN  PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT      egressPortBindMode
);

/**
* @internal prvTgfPtpManagerCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current TOD values.
*
* @note    NOTE: Call API prvTgfPtpManagerTodValueGet to retreive captured TOD values.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN PRV_TGF_PTP_TAI_NUMBER_ENT                 taiNumber
);

/**
* @internal prvTgfPtpManagerTodValueSet function
* @endinternal
*
* @brief   Set a new TOD Value.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] todValuePtr               - (pointer to) TOD value.
* @param[in] capturePrevTodValueEnable - enable/disbale capturing previous mode.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_PTP_TAI_NUMBER_ENT             taiNumber,
    IN PRV_TGF_PTP_TOD_COUNT_STC             *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
);

/**
* @internal prvTgfPtpManagerTodValueFrequencyUpdate function
* @endinternal
*
* @brief   Adjusts the fractional nanosecond drift value which is added to the TOD stepSet logic.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] updateValue               - fractional nanosecond drift value.
* @param[in] capturePrevTodValueEnable - enable/disbale capturing previous mode.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_PTP_TAI_NUMBER_ENT             taiNumber,
    IN GT_U32                                 updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
);

/**
* @internal prvTgfPtpManagerTodValueUpdate function
* @endinternal
*
* @brief  update the current TOD value.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] updateValuePtr           - (pointer to) increment/decrement value.
* @param[in] function                 - TOD counter functions.
*                                       (APPLICABLE VALUES:
*                                           PRV_TGF_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
*                                           PRV_TGF_PTP_TOD_COUNTER_FUNC_DECREMENT_E,
*                                           PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,
*                                           PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E)
* @param[in] gracefulStep             - Graceful Step in nanoseconds. Applicable if "function" ==
*                                       PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E or PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] capturePrevTodValueEnable - enable/disbale capturing previous mode.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  PRV_TGF_PTP_TOD_COUNT_STC             *updateValuePtr,
    IN  PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT      function,
    IN  GT_U32                                gracefulStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
);

/**
* @internal prvTgfPtpManagerTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           PRV_TGF_PTP_TAI_NUMBER_0_E,
*                                           PRV_TGF_PTP_TAI_NUMBER_1_E)
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT                  taiNumber,
    OUT PRV_TGF_PTP_MANAGER_CAPTURE_TOD_VALUE_STC   *todValueEntry0Ptr,
    OUT PRV_TGF_PTP_MANAGER_CAPTURE_TOD_VALUE_STC   *todValueEntry1Ptr
);

/**
* @internal prvTgfPtpManagerTriggerGenerate function
* @endinternal
*
* @brief   Generates a trigger on the PTP_PULSE_OUT interface at a specified time.
*
* @note    NOTE: This functionality should only be triggered on the master device.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] extPulseWidth            - external pulse width in nanoseconds of generated output trigger.
*                                       (APPLICABLE RANGES: 0..2^28-1.)
* @param[in] triggerTimePtr           - (pointer to) trigger generation time.
* @param[in] maskEnable               - enable/disbale masking of trigger generation time.
* @param[in] maskValuePtr             - (pointer to) trigger generation time mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT              taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  PRV_TGF_PTP_TOD_COUNT_STC               *triggerTimePtr,
    IN  GT_BOOL                                 maskEnable,
    IN  PRV_TGF_PTP_TOD_COUNT_STC               *maskValuePtr
);

/**
* @internal prvTgfPtpManagerGlobalPtpDomainSet function
* @endinternal
*
* @brief   Configures the global settings for PTP Domain.
*
* @param[in] devNum                   - device number
* @param[in] domainConfPtr            - (pointer to) PTP domain configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN PRV_TGF_PTP_MANAGER_DOMAIN_STC                  *domainConfPtr
);

/**
* @internal prvTgfPtpManagerPortCommandAssignment function
* @endinternal
*
* @brief   Assign packet command per{ingress port, domain index, message type}.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - PTP Message Type to trigger the selected Command.
* @param[in] command                  - command assigned  to the packet.
*                                      (APPLICABLE VALUES:
*                                         CPSS_PACKET_CMD_FORWARD_E,
*                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                         CPSS_PACKET_CMD_DROP_HARD_E,
*                                         CPSS_PACKET_CMD_DROP_SOFT_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerPortCommandAssignment
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     portNum,
    IN GT_U32                                   domainIndex,
    IN PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT     messageType,
    IN CPSS_PACKET_CMD_ENT                      command
);

/**
* @internal prvTgfPtpManagerPortPacketActionsConfig function
* @endinternal
*
* @brief  Configure action to be performed on PTP packets per{egress port, domain index, message type}.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiNumber                - TAI number.
* @param[in] domainIndex              - domain index.
*                                       (APPLICABLE RANGES: 0..4.)
* @param[in] domainEntryPtr           - (pointer to) domain table entry.
* @param[in] messageType              - PTP message type
* @param[in] action                   - PTP action on the packet
*                                       (APPLICABLE VALUES:
*                                           PRV_TGF_PTP_TS_ACTION_NONE_E
*                                           PRV_TGF_PTP_TS_ACTION_DROP_E
*                                           PRV_TGF_PTP_TS_ACTION_CAPTURE_E
*                                           PRV_TGF_PTP_TS_ACTION_ADD_TIME_E
*                                           PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
*                                           PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
*                                           PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,
*                                           PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT              taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC     *domainEntryPtr,
    IN  PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT    messageType,
    IN  PRV_TGF_PTP_TS_ACTION_ENT               action
);

/**
* @internal prvTgfPtpManagerPortTsTagConfig function
* @endinternal
*
* @brief    Controlling Timestamp tag for Ingress identification and Egress Addition.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpManagerPortTsTagConfig
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  PRV_TGF_PTP_TS_TAG_PORT_CFG_STC      *tsTagPortCfgPtr
);

/**
* @internal prvTgfPtpManagerTsQueueConfig function
* @endinternal
*
* @brief  Configure queue number per message type.
*
* @param[in] devNum                 - device number
* @param[in] messageType            - PTP Message Type
* @param[in] queueNum               - Queue number; 0 - queue0, 1 - queue1.
*                                     (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS prvTgfPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT        messageType,
    IN  GT_U32                                      queueNum
);

/**
* @internal prvTgfPtpManagerEgressTimestampGlobalQueueEntryRead
*           function
* @endinternal
*
* @brief   Read Egress Timestamp Global Queue Entry.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS prvTgfPtpManagerEgressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
);

/**
* @internal prvTgfPtpManagerEgressTimestampPortQueueEntryRead
*           function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry per port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS prvTgfPtpManagerEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  GT_U32                                                      queueNum,
    OUT PRV_TGF_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC       *entryPtr
);

/**
* @internal prvTgfPtpManagerIngressTimestampGlobalQueueEntryRead
*           function
* @endinternal
*
* @brief   Read Ingress Timestamp Global Queue Entry.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS prvTgfPtpManagerIngressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC     *entryPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPtpManagerGenh */


