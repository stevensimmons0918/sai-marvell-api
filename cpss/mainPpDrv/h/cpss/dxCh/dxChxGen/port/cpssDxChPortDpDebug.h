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
* @file cpssDxChPortDpDebug.h
*
* @brief Port Datapath Debug API.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortDpDebugh
#define __cpssDxChPortDpDebugh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/generic/cpssCommonDefs.h>

/**
* @internal cpssDxChPortDpDebugCountersResetAndEnableSet function
* @endinternal
*
* @brief   Reset RX_DMA all debug counters and enable/disable counting.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum    - device number.
* @param[in] enable    - GT_TRUE  - enable counting after reset.
*                        GT_FALSE - reset done but counting disabled.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugCountersResetAndEnableSet
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    IN GT_BOOL                       enable
);

/**
* @internal cpssDxChPortDpDebugRxToCpCountingConfigSet function
* @endinternal
*
* @brief   Set configuration for counting Rx to Cp packets.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] unitRepresentingPort  - physical port number representing Rx unit.
* @param[in] criteriaParamBmp      - bitmap of used criterias according to following parameters.
*                                    1 means that criteria used, 0 - that criteria ignored.
*                                    bit0 - cutThroughPacket
*                                    bit1 - rxToCpPriority
*                                    bit2 - latencySensitive
*                                    bit3 - headOrTailDispatch
*                                    bit4 - channel - packets received from unitRepresentingPort only
*                                    example: 5 means that used cutThroughPacket and latencySensitive
*                                    criterias, all other criterias ignored
* @param[in] cutThroughPacket      - GT_TRUE - packet in Cut Through mode.
*                                    GT_FALSE - packet in Store and Forward mode.
* @param[in] rxToCpPriority        - PIP priority (APPLICABLE_RANGES: 0..3).
* @param[in] latencySensitive      - GT_TRUE - packet is Latency Sensitive.
*                                    GT_FALSE - packet is not Latency Sensitive.
* @param[in] headOrTailDispatch    - GT_TRUE - head dispatch.
*                                    GT_FALSE - tail dispatch.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugRxToCpCountingConfigSet
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    IN GT_U32                        criteriaParamBmp,
    IN GT_BOOL                       cutThroughPacket,
    IN GT_U32                        rxToCpPriority,
    IN GT_BOOL                       latencySensitive,
    IN GT_BOOL                       headOrTailDispatch
);

/**
* @internal cpssDxChPortDpDebugRxToCpCountingConfigGet function
* @endinternal
*
* @brief   Get configuration for counting Rx to Cp packets.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum                  - device number.
* @param[in] unitRepresentingPort    - physical port number representing Rx unit.
* @param[out] criteriaParamBmpPtr    - (pointer to) bitmap of used criterias according to following parameters.
*                                      1 means that criteria used, 0 - that criteria ignored.
*                                      bit0 - cutThroughPacket
*                                      bit1 - rxToCpPriority
*                                      bit2 - latencySensitive
*                                      bit3 - headOrTailDispatch
*                                      bit4 - channel - packets received from unitRepresentingPort only
*                                      Example: 5 means that used cutThroughPacket and latencySensitive
*                                      criterias, all other criterias ignored
* @param[out] cutThroughPacketPtr    - (pointer to) GT_TRUE - packet in Cut Through mode.
*                                                   GT_FALSE - packet in Store and Forward mode.
* @param[out] rxToCpPriorityPtr      - (pointer to) PIP priority.
* @param[out] latencySensitivePtr    - (pointer to) GT_TRUE - packet is Latency Sensitive.
*                                                   GT_FALSE - packet is not Latency Sensitive.
* @param[out] headOrTailDispatchPtr  - (pointer to) GT_TRUE - head dispatch.
*                                                   GT_FALSE - tail dispatch.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugRxToCpCountingConfigGet
(
    IN  GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    OUT GT_U32                        *criteriaParamBmpPtr,
    OUT GT_BOOL                       *cutThroughPacketPtr,
    OUT GT_U32                        *rxToCpPriorityPtr,
    OUT GT_BOOL                       *latencySensitivePtr,
    OUT GT_BOOL                       *headOrTailDispatchPtr
);

/**
* @internal cpssDxChPortDpDebugRxToCpCountValueGet function
* @endinternal
*
* @brief   Get Rx to Cp packets count value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum                  - device number.
* @param[in] unitRepresentingPort    - physical port number representing Rx unit.
* @param[out] countValuePtr          - (pointer to) count value.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugRxToCpCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
);

/**
* @internal cpssDxChPortDpDebugCpToRxCountingConfigSet function
* @endinternal
*
* @brief   Set configuration for counting Cp to Rx packets.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] unitRepresentingPort  - physical port number representing Rx unit.
* @param[in] criteriaParamBmp      - bitmap of used criterias according to following parameters.
*                                    1 means that criteria used, 0 - that criteria ignored.
*                                    bit0 - multicastPacket
*                                    bit1 - cutThroughPacket
*                                    bit2 - cutThroughTerminated
*                                    bit3 - trunkatedHeader
*                                    bit4 - dummyDescriptor
*                                    bit5 - highPriorityPacket
*                                    bit6 - contextDone
*                                    bit7 - descriptorValid
*                                    bit8 - channel - packets received from unitRepresentingPort only
*                                    example: 5 means that used multicastPacket and cutThroughTerminated
*                                    criterias, all other criterias ignored
* @param[in] multicastPacket       - GT_TRUE - multicast packet.
*                                    GT_FALSE - unicast packet.
* @param[in] cutThroughPacket      - GT_TRUE - packet in Cut Through mode.
*                                    GT_FALSE - packet in Store and Forward mode.
* @param[in] cutThroughTerminated  - GT_TRUE - CP terminated Cut Through mode.
*                                    GT_FALSE - Cut Through or Store and Forward mode not changed.
* @param[in] trunkatedHeader       - GT_TRUE - trunkated Header.
*                                    GT_FALSE - no trunkated Header.
* @param[in] dummyDescriptor       - GT_TRUE - dummy Descriptor.
*                                    GT_FALSE - no dummy Descriptor.
* @param[in] highPriorityPacket    - GT_TRUE - high Priority Packet.
*                                    GT_FALSE - no high Priority Packet.
* @param[in] contextDone           - GT_TRUE - context Done.
*                                    GT_FALSE - no context Done.
* @param[in] descriptorValid       - GT_TRUE - descriptor Valid.
*                                    GT_FALSE - descriptor invsalid.
*                                    Note: Parameters highPriorityPacket, contextDone, descriptorValid
*                                          Relevant for AC5P and above only.
*                                          Events contextDone and descriptorValid are different events
*                                          for the same packet - when both of them ignored the packet
*                                          counted twice.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugCpToRxCountingConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 unitRepresentingPort,
    IN GT_U32                               criteriaParamBmp,
    IN GT_BOOL                              multicastPacket,
    IN GT_BOOL                              cutThroughPacket,
    IN GT_BOOL                              cutThroughTerminated,
    IN GT_BOOL                              trunkatedHeader,
    IN GT_BOOL                              dummyDescriptor,
    IN GT_BOOL                              highPriorityPacket,
    IN GT_BOOL                              contextDone,
    IN GT_BOOL                              descriptorValid
);

/**
* @internal cpssDxChPortDpDebugCpToRxCountingConfigGet function
* @endinternal
*
* @brief   Get configuration for counting Cp to Rx packets.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number.
* @param[in] unitRepresentingPort      - physical port number representing Rx unit.
* @param[out] criteriaParamBmpPtr      - (pointer to) bitmap of used criterias according to following parameters.
*                                         bit0 - multicastPacket
*                                         bit1 - cutThroughPacket
*                                         bit2 - cutThroughTerminated
*                                         bit3 - trunkatedHeader
*                                         bit4 - dummyDescriptor
*                                         bit5 - highPriorityPacket
*                                         bit6 - contextDone
*                                         bit7 - descriptorValid
*                                         bit8 - channel - packets received from unitRepresentingPort only
*                                        1 means that criteria used, 0 - that criteria ignored.
*                                        example: 5 means that used multicastPacket and cutThroughTerminated
*                                        criterias, all other criterias ignored
* @param[out] multicastPacketPtr       - (pointer to) GT_TRUE - multicast packet.
*                                                     GT_FALSE - unicast packet.
* @param[out] cutThroughPacketPtr      - (pointer to) GT_TRUE - packet in Cut Through mode.
*                                                     GT_FALSE - packet in Store and Forward mode.
* @param[out] cutThroughTerminatedPtr  - (pointer to) GT_TRUE - CP terminated Cut Through mode.
*                                                     GT_FALSE - Cut Through or Store and Forward mode not changed.
* @param[out] trunkatedHeaderPtr       - (pointer to) GT_TRUE - trunkated Header.
*                                                     GT_FALSE - no trunkated Header.
* @param[out] dummyDescriptorPtr       - (pointer to) GT_TRUE - dummy Descriptor.
*                                                     GT_FALSE - no dummy Descriptor.
* @param[out] highPriorityPacketPtr    - (pointer to) GT_TRUE - high Priority Packet.
*                                                     GT_FALSE - no high Priority Packet.
* @param[out] contextDonePtr           - (pointer to) GT_TRUE - context Done.
*                                                     GT_FALSE - no context Done.
* @param[out] descriptorValidPtr       - (pointer to) GT_TRUE - descriptor Valid.
*                                                     GT_FALSE - descriptor invsalid.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugCpToRxCountingConfigGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             unitRepresentingPort,
    OUT GT_U32                           *criteriaParamBmpPtr,
    OUT GT_BOOL                          *multicastPacketPtr,
    OUT GT_BOOL                          *cutThroughPacketPtr,
    OUT GT_BOOL                          *cutThroughTerminatedPtr,
    OUT GT_BOOL                          *trunkatedHeaderPtr,
    OUT GT_BOOL                          *dummyDescriptorPtr,
    OUT GT_BOOL                          *highPriorityPacketPtr,
    OUT GT_BOOL                          *contextDonePtr,
    OUT GT_BOOL                          *descriptorValidPtr
);

/**
* @internal cpssDxChPortDpDebugCpToRxCountValueGet function
* @endinternal
*
* @brief   Get Cp to Rx packets count value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X Bobcat3; Aldrin2.
*
* @param[in] devNum                  - device number.
* @param[in] unitRepresentingPort    - physical port number representing Rx unit.
* @param[out] countValuePtr          - (pointer to) count value.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDpDebugCpToRxCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortDpDebugh */

