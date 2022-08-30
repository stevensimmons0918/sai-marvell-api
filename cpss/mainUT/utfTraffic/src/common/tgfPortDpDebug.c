/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfPortDpDebug.c
*
* DESCRIPTION:
*       Generic API for Port Datapath debug APIs.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortDpDebug.h>

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPortDpDebugCountersResetAndEnableSet function
* @endinternal
*
* @brief   Reset RX_DMA all debug counters and enable/disable counting.
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
GT_STATUS prvTgfPortDpDebugCountersResetAndEnableSet
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    IN GT_BOOL                       enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortDpDebugCountersResetAndEnableSet(
        devNum, unitRepresentingPort, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPortDpDebugCountersResetAndEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(unitRepresentingPort);
    TGF_PARAM_NOT_USED(enable);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfPortDpDebugRxToCpCountingConfigSet function
* @endinternal
*
* @brief   Set configuration for counting Rx to Cp packets.
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
GT_STATUS prvTgfPortDpDebugRxToCpCountingConfigSet
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    IN GT_U32                        criteriaParamBmp,
    IN GT_BOOL                       cutThroughPacket,
    IN GT_U32                        rxToCpPriority,
    IN GT_BOOL                       latencySensitive,
    IN GT_BOOL                       headOrTailDispatch
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortDpDebugRxToCpCountingConfigSet(
        devNum, unitRepresentingPort, criteriaParamBmp,
        cutThroughPacket, rxToCpPriority,
        latencySensitive, headOrTailDispatch);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPortDpDebugRxToCpCountingConfigSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(unitRepresentingPort);
    TGF_PARAM_NOT_USED(criteriaParamBmp);
    TGF_PARAM_NOT_USED(cutThroughPacket);
    TGF_PARAM_NOT_USED(rxToCpPriority);
    TGF_PARAM_NOT_USED(latencySensitive);
    TGF_PARAM_NOT_USED(headOrTailDispatch);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfPortDpDebugRxToCpCountValueGet function
* @endinternal
*
* @brief   Get Rx to Cp packets count value.
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
GT_STATUS prvTgfPortDpDebugRxToCpCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortDpDebugRxToCpCountValueGet(
        devNum, unitRepresentingPort, countValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPortDpDebugRxToCpCountValueGet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(unitRepresentingPort);
    TGF_PARAM_NOT_USED(countValuePtr);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfPortDpDebugCpToRxCountingConfigSet function
* @endinternal
*
* @brief   Set configuration for counting Cp to Rx packets.
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
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortDpDebugCpToRxCountingConfigSet
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
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortDpDebugCpToRxCountingConfigSet(
        devNum, unitRepresentingPort, criteriaParamBmp,
        multicastPacket, cutThroughPacket, cutThroughTerminated,
        trunkatedHeader, dummyDescriptor,
        highPriorityPacket, contextDone, descriptorValid);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPortDpDebugCpToRxCountingConfigSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(unitRepresentingPort);
    TGF_PARAM_NOT_USED(criteriaParamBmp);
    TGF_PARAM_NOT_USED(multicastPacket);
    TGF_PARAM_NOT_USED(cutThroughPacket);
    TGF_PARAM_NOT_USED(cutThroughTerminated);
    TGF_PARAM_NOT_USED(trunkatedHeader);
    TGF_PARAM_NOT_USED(dummyDescriptor);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfPortDpDebugCpToRxCountValueGet function
* @endinternal
*
* @brief   Get Cp to Rx packets count value.
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
GT_STATUS prvTgfPortDpDebugCpToRxCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortDpDebugCpToRxCountValueGet(
        devNum, unitRepresentingPort, countValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPortDpDebugRxToCpCountValueGet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(unitRepresentingPort);
    TGF_PARAM_NOT_USED(countValuePtr);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}



