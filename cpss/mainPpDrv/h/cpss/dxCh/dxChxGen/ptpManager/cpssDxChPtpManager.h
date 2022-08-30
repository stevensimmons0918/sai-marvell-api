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
* @file cpssDxChPtpManager.h
*
* @brief Precision Time Protocol Manager Support - API definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPtpManager_h
#define __cpssDxChPtpManager_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* include the PTP manager types */
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManagerTypes.h>

/**
* @internal cpssDxChPtpManagerPtpInit function
* @endinternal
*
* @brief  Set one-time global PTP-related settings.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
);

/**
* @internal cpssDxChPtpManagerInputPulseSet function
* @endinternal
*
* @brief   Set input pulse configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] inputMode                - Whether an incoming pulse comes from the PULSE pin (slave PPS) or the CLK pin.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT   inputMode
);

/**
* @internal cpssDxChPtpManagerMasterPpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Master PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);

/**
* @internal cpssDxChPtpManagerSlavePpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);

/**
* @internal cpssDxChPtpManagerPortAllocateTai function
* @endinternal
*
* @brief   Allocating Port Interface to TAI.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] egressPortBindMode       - determines the mode in which egress port is bound to TAI,
*                                       either static or dynamic per packet.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E, (dynamic bind, per ingress port of a specific packet)
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E)  (static bind per taiNumber param)
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
*                                       NOTE: for high accuracy use CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT               taiNumber,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    egressPortBindMode
);

/**
* @internal cpssDxChPtpManagerCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerTodValueGet to retreive captured TOD values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT                 taiNumber
);

/**
* @internal cpssDxChPtpManagerTodValueSet function
* @endinternal
*
* @brief   Set a new TOD Value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] todValuePtr               - (pointer to) TOD value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
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
GT_STATUS cpssDxChPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC            *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
);

/**
* @internal cpssDxChPtpManagerTodValueFrequencyUpdate function
* @endinternal
*
* @brief   Adjusts the fractional nanosecond drift value which is added to the TOD stepSet logic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] updateValue               - fractional nanosecond drift value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
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
GT_STATUS cpssDxChPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN GT_U32                                 updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
);

/**
* @internal cpssDxChPtpManagerTodValueUpdate function
* @endinternal
*
* @brief  update the current TOD value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] updateValuePtr           - (pointer to) increment/decrement value.
* @param[in] function                 - TOD counter functions.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E)
* @param[in] gracefulStep             - Graceful Step in nanoseconds. Applicable if "function" ==
*                                       CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E or CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
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
GT_STATUS cpssDxChPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *updateValuePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT    function,
    IN  GT_U32                                gracefulStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
);

/**
* @internal cpssDxChPtpManagerTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
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
GT_STATUS cpssDxChPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
);

/**
* @internal cpssDxChPtpManagerTriggerGenerate function
* @endinternal
*
* @brief   Generates a trigger on the PTP_PULSE_OUT interface at a specified time.
*
* @note    NOTE: This functionality should only be triggered on the master device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] extPulseWidth            - external pulse width in nanoseconds of generated output trigger.
*                                       (APPLICABLE RANGES: 0..2^28-1.)
* @param[in] triggerTimePtr           - (pointer to) trigger generation time.
* @param[in] maskEnable               - enable/disable masking of trigger generation time.
* @param[in] maskValuePtr             - (pointer to) trigger generation time mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *triggerTimePtr,
    IN  GT_BOOL                                 maskEnable,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *maskValuePtr
);

/**
* @internal cpssDxChPtpManagerGlobalPtpDomainSet function
* @endinternal
*
* @brief   Configures the global settings for PTP Domain.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                *domainConfPtr
);

/**
* @internal cpssDxChPtpManagerPortCommandAssignment function
* @endinternal
*
* @brief   Assign packet command per{ingress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerPortCommandAssignment
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     portNum,
    IN GT_U32                                   domainIndex,
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType,
    IN CPSS_PACKET_CMD_ENT                      command
);

/**
* @internal cpssDxChPtpManagerPortPacketActionsConfig function
* @endinternal
*
* @brief  Configure action to be performed on PTP packets per{egress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] domainIndex              - domain index.
*                                       (APPLICABLE RANGES: 0..4.)
* @param[in] domainEntryPtr           - (pointer to) domain table entry.
* @param[in] messageType              - PTP message type
* @param[in] action                   - PTP action on the packet
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_ACTION_NONE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_DROP_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *domainEntryPtr,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT  messageType,
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT             action
);

/**
* @internal cpssDxChPtpManagerPortTsTagConfig function
* @endinternal
*
* @brief    Controlling Timestamp tag for Ingress identification and Egress Addition.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerPortTsTagConfig
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
);

/**
* @internal cpssDxChPtpManagerTsQueueConfig function
* @endinternal
*
* @brief  Configure queue number per message type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT      messageType,
    IN  GT_U32                                      queueNum
);

/**
* @internal cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *entryPtr
);

/**
* @internal cpssDxChPtpManagerEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  GT_U32                                                      queueNum,
    OUT CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
);

/**
* @internal cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChPtpManagerPortTimestampingModeConfig function
* @endinternal
*
* @brief   Configure Enhanced Timestamping Mode per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsMode                   - enhance timestamping mode.
* @param[in] operationMode            - operation mode.
*                                       (APPLICABLE VALUES:
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_MAC_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_PHY_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_FORMAT_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E
*                                           (APPLICABLE DEVICES: AC5P; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortTimestampingModeConfig
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT    tsMode,
    IN  CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT        operationMode
);

/**
* @internal cpssDxChPtpManagerPortDelayValuesSet function
* @endinternal
*
* @brief   Set delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[in] delayValPtr              - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortDelayValuesSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    IN  CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
);

/**
* @internal cpssDxChPtpManagerPortDelayValuesGet function
* @endinternal
*
* @brief   Get delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[out] delayValPtr             - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortDelayValuesGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
);

/**
* @internal cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read MAC Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] entryPtr                - (pointer to) MAC Egress Timestamp Queue Entry.
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
GT_STATUS cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
);

/**
* @internal cpssDxChPtpManager8021AsMsgSelectionEnableSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] FollowUpEn                           - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[in] SyncEn                               - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_BOOL                                     followUpEnable,
    IN  GT_BOOL                                     syncEnable
);

/**
* @internal cpssDxChPtpManager8021AsMsgSelectionEnableGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[out] followUpEnablePtr                  - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[out] syncEnablePtr                      - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_BOOL                                     *followUpEnablePtr,
    OUT GT_BOOL                                     *syncEnablePtr
);

/**
* @internal cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offset                               - value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      offset
);

/**
* @internal cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[out] offsetPtr                           - (pointer to) value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_U32                                      *offsetPtr
);

/**
* @internal cpssDxChPtpManagerPortMacTsQueueSigatureSet function
* @endinternal
*
* @brief   Set MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] signatureMode            - MCH signature mode.
* @param[in] signatureValue           - MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
GT_STATUS cpssDxChPtpManagerPortMacTsQueueSigatureSet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    signatureMode,
    IN  GT_U32                                                      signatureValue
);

/**
* @internal cpssDxChPtpManagerPortMacTsQueueSigatureGet function
* @endinternal
*
* @brief   Get MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[out] signatureModePtr        - (pointer to) MCH signature mode.
* @param[out] signatureValuePtr       - (pointer to) MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
GT_STATUS cpssDxChPtpManagerPortMacTsQueueSigatureGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    *signatureModePtr,
    OUT GT_U32                                                      *signatureValuePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPtpMananer_h */
