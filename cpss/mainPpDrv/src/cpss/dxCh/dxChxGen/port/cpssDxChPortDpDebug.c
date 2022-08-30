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
* @file cpssDxChPortDpDebug.c
*
* @brief Port Datapath Debug API.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDpDebug.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal internal_cpssDxChPortDpDebugCountersResetAndEnableSet function
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
static GT_STATUS internal_cpssDxChPortDpDebugCountersResetAndEnableSet
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    IN GT_BOOL                       enable
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum, &dpIndex, &localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.dp_rx_enable_debug_counters;
    /* reset all debug counters */
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, regAddr, 1/*mask*/, 0/*value*/);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (enable != GT_FALSE)
    {
        /* enable all debug counters */
        rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
            devNum, 0/*portGroupId*/, regAddr, 1/*mask*/, 1/*value*/);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugCountersResetAndEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, enable));

    rc = internal_cpssDxChPortDpDebugCountersResetAndEnableSet(
        devNum, unitRepresentingPort, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

static const GT_U32 fieldRxToCpOffsetArr[] = {0, 1, 3, 4, 5};
static const GT_U32 fieldRxToCpSizeArr[]   = {1, 2, 1, 1, 6};
#define RX_TO_CP_FIELDS_NUM (sizeof(fieldRxToCpOffsetArr) / sizeof(fieldRxToCpOffsetArr[0]))

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
static GT_STATUS internal_cpssDxChPortDpDebugRxToCpCountingConfigSet
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
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    hwValueRef;       /* HW value of Ref register  */
    GT_U32    hwValueMask;      /* HW value of mask register */
    GT_U32    hwWriteMask;      /* write API of mask register */
    GT_U32    mask;             /* work mask */
    GT_U32    i;                /* parameter index */
    GT_U32    fieldValArr[RX_TO_CP_FIELDS_NUM];   /* parameters converted to HW values */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum, &dpIndex, &localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    fieldValArr[0] = BOOL2BIT_MAC(cutThroughPacket);
    fieldValArr[1] = rxToCpPriority;
    fieldValArr[2] = BOOL2BIT_MAC(latencySensitive);
    fieldValArr[3] = BOOL2BIT_MAC(headOrTailDispatch);
    fieldValArr[4] = localDmaNum;
    hwValueRef  = 0;
    hwValueMask = 0;
    hwWriteMask = 0;

    for (i = 0; (i < RX_TO_CP_FIELDS_NUM); i++)
    {
        mask = ((1 << fieldRxToCpSizeArr[i]) - 1);
        hwValueRef |= ((mask & fieldValArr[i]) << fieldRxToCpOffsetArr[i]);
        if (criteriaParamBmp & (1 << i))
        {
            hwValueMask |= (mask  << fieldRxToCpOffsetArr[i]);
        }
        hwWriteMask |= (mask  << fieldRxToCpOffsetArr[i]);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.rx_to_cp_cnt.rx_to_cp_count_type_mask;
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, regAddr, hwWriteMask, hwValueMask);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.rx_to_cp_cnt.rx_to_cp_count_type_ref;
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, regAddr, hwWriteMask, hwValueRef);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugRxToCpCountingConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, criteriaParamBmp, cutThroughPacket,
         rxToCpPriority, latencySensitive, headOrTailDispatch));

    rc = internal_cpssDxChPortDpDebugRxToCpCountingConfigSet(
        devNum, unitRepresentingPort, criteriaParamBmp, cutThroughPacket,
         rxToCpPriority, latencySensitive, headOrTailDispatch);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, criteriaParamBmp, cutThroughPacket,
         rxToCpPriority, latencySensitive, headOrTailDispatch));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortDpDebugRxToCpCountingConfigGet function
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
static GT_STATUS internal_cpssDxChPortDpDebugRxToCpCountingConfigGet
(
    IN  GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    OUT GT_U32                        *criteriaParamBmpPtr,
    OUT GT_BOOL                       *cutThroughPacketPtr,
    OUT GT_U32                        *rxToCpPriorityPtr,
    OUT GT_BOOL                       *latencySensitivePtr,
    OUT GT_BOOL                       *headOrTailDispatchPtr
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    hwValueRef;       /* HW value of Ref register  */
    GT_U32    hwValueMask;      /* HW value of mask register */
    GT_U32    criteriaParamBmp; /* criteria Param Bmp */
    GT_U32    mask;             /* work mask */
    GT_U32    i;                /* parameter index */
    GT_U32    fieldValArr[RX_TO_CP_FIELDS_NUM];   /* parameters converted to HW values */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(criteriaParamBmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(cutThroughPacketPtr);
    CPSS_NULL_PTR_CHECK_MAC(rxToCpPriorityPtr);
    CPSS_NULL_PTR_CHECK_MAC(latencySensitivePtr);
    CPSS_NULL_PTR_CHECK_MAC(headOrTailDispatchPtr);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum,&dpIndex,&localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.rx_to_cp_cnt.rx_to_cp_count_type_mask;
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, &hwValueMask);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.rx_to_cp_cnt.rx_to_cp_count_type_ref;
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, &hwValueRef);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    criteriaParamBmp = 0;
    for (i = 0; (i < RX_TO_CP_FIELDS_NUM); i++)
    {
        mask = ((1 << fieldRxToCpSizeArr[i]) - 1);
        fieldValArr[i] = ((hwValueRef >> fieldRxToCpOffsetArr[i]) & mask);
        if ((mask  << fieldRxToCpOffsetArr[i]) & hwValueMask)
        {
            criteriaParamBmp |= (1 << i);
        }
    }

    *criteriaParamBmpPtr     = criteriaParamBmp;
    *cutThroughPacketPtr     = BIT2BOOL_MAC(fieldValArr[0]);
    *rxToCpPriorityPtr       = fieldValArr[1];
    *latencySensitivePtr     = BIT2BOOL_MAC(fieldValArr[2]);
    *headOrTailDispatchPtr   = BIT2BOOL_MAC(fieldValArr[3]);

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugRxToCpCountingConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, criteriaParamBmpPtr, cutThroughPacketPtr,
         rxToCpPriorityPtr, latencySensitivePtr, headOrTailDispatchPtr));

    rc = internal_cpssDxChPortDpDebugRxToCpCountingConfigGet(
        devNum, unitRepresentingPort, criteriaParamBmpPtr, cutThroughPacketPtr,
         rxToCpPriorityPtr, latencySensitivePtr, headOrTailDispatchPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, criteriaParamBmpPtr, cutThroughPacketPtr,
         rxToCpPriorityPtr, latencySensitivePtr, headOrTailDispatchPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortDpDebugRxToCpCountValueGet function
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
static GT_STATUS internal_cpssDxChPortDpDebugRxToCpCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */
    GT_U32    dummy;            /* place to read not used data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(countValuePtr);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum,&dpIndex,&localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.rx_to_cp_cnt.rx_to_cp_count;
    rc =  prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, countValuePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    /* needed to read to unlock counter */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.rx_to_cp_cnt.rx_to_cp_count_high;
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc =  prvCpssDrvHwPpPortGroupReadRegister(
            devNum, 0/*portGroupId*/, regAddr, &dummy);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugRxToCpCountValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, countValuePtr));

    rc = internal_cpssDxChPortDpDebugRxToCpCountValueGet(
        devNum, unitRepresentingPort, countValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, countValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* size of null bits means that the field does not exist */
static const GT_U32 sip6_fieldCpToRxOffsetArr[]    = {0, 1, 2, 3, 4, 0, 0, 0, 5};
static const GT_U32 sip6_fieldCpToRxSizeArr[]      = {1, 1, 1, 1, 1, 0, 0, 0, 6};
static const GT_U32 sip6_10_fieldCpToRxOffsetArr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
static const GT_U32 sip6_10_fieldCpToRxSizeArr[]   = {1, 1, 1, 1, 1, 1, 1, 1, 6};
static const GT_U32 sip6_30_fieldCpToRxOffsetArr[] = {0, 1, 2, 3, 4, 5, 6, 7, 9};
static const GT_U32 sip6_30_fieldCpToRxSizeArr[]   = {1, 1, 1, 1, 1, 1, 1, 1, 6};
#define CP_TO_RX_FIELDS_NUM (sizeof(sip6_30_fieldCpToRxOffsetArr) / sizeof(sip6_30_fieldCpToRxOffsetArr[0]))

/**
* @internal internal_cpssDxChPortDpDebugCpToRxCountingConfigSet function
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
*                                    GT_FALSE - descriptor invalid.
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
static GT_STATUS internal_cpssDxChPortDpDebugCpToRxCountingConfigSet
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
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    hwValueRef;       /* HW value of Ref register  */
    GT_U32    hwValueMask;      /* HW value of mask register */
    GT_U32    hwWriteMask;      /* write API of mask register */
    GT_U32    mask;             /* work mask */
    GT_U32    i;                /* parameter index */
    GT_U32    fieldValArr[CP_TO_RX_FIELDS_NUM];   /* parameters converted to HW values */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */
    const GT_U32 *fieldCpToRxOffsetArr; /* pointer to array of field offsets */
    const GT_U32 *fieldCpToRxSizeArr;   /* pointer to array of field sizes */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum,&dpIndex,&localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    fieldValArr[0] = BOOL2BIT_MAC(multicastPacket);
    fieldValArr[1] = BOOL2BIT_MAC(cutThroughPacket);
    fieldValArr[2] = BOOL2BIT_MAC(cutThroughTerminated);
    fieldValArr[3] = BOOL2BIT_MAC(trunkatedHeader);
    fieldValArr[4] = BOOL2BIT_MAC(dummyDescriptor);
    fieldValArr[5] = BOOL2BIT_MAC(highPriorityPacket);
    fieldValArr[6] = BOOL2BIT_MAC(contextDone);
    fieldValArr[7] = BOOL2BIT_MAC(descriptorValid);
    fieldValArr[8] = localDmaNum;
    hwValueRef  = 0;
    hwValueMask = 0;
    hwWriteMask = 0;

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        fieldCpToRxOffsetArr = sip6_30_fieldCpToRxOffsetArr;
        fieldCpToRxSizeArr   = sip6_30_fieldCpToRxSizeArr;
    }
    else
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        fieldCpToRxOffsetArr = sip6_10_fieldCpToRxOffsetArr;
        fieldCpToRxSizeArr   = sip6_10_fieldCpToRxSizeArr;
    }
    else
    {
        fieldCpToRxOffsetArr = sip6_fieldCpToRxOffsetArr;
        fieldCpToRxSizeArr   = sip6_fieldCpToRxSizeArr;
    }

    for (i = 0; (i < CP_TO_RX_FIELDS_NUM); i++)
    {
        /* bypass unsupported fields*/
        if (fieldCpToRxSizeArr[i] == 0) continue;

        mask = ((1 << fieldCpToRxSizeArr[i]) - 1);
        hwValueRef |= ((mask & fieldValArr[i]) << fieldCpToRxOffsetArr[i]);
        if (criteriaParamBmp & (1 << i))
        {
            hwValueMask |= (mask  << fieldCpToRxOffsetArr[i]);
        }
        hwWriteMask |= (mask  << fieldCpToRxOffsetArr[i]);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.cp_to_rx_cnt[0].cp_to_rx_count_type_mask;
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, regAddr, hwWriteMask, hwValueMask);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.cp_to_rx_cnt[0].cp_to_rx_count_type_ref;
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0/*portGroupId*/, regAddr, hwWriteMask, hwValueRef);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

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
*                                    GT_FALSE - descriptor invalid.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugCpToRxCountingConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, criteriaParamBmp,
         multicastPacket, cutThroughPacket, cutThroughTerminated,
         trunkatedHeader, dummyDescriptor, highPriorityPacket,
         contextDone, descriptorValid));

    rc = internal_cpssDxChPortDpDebugCpToRxCountingConfigSet(
        devNum, unitRepresentingPort, criteriaParamBmp,
        multicastPacket, cutThroughPacket, cutThroughTerminated,
        trunkatedHeader, dummyDescriptor, highPriorityPacket,
        contextDone, descriptorValid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, criteriaParamBmp,
         multicastPacket, cutThroughPacket, cutThroughTerminated,
         trunkatedHeader, dummyDescriptor, highPriorityPacket,
         contextDone, descriptorValid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortDpDebugCpToRxCountingConfigGet function
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
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortDpDebugCpToRxCountingConfigGet
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
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    hwValueRef;       /* HW value of Ref register  */
    GT_U32    hwValueMask;      /* HW value of mask register */
    GT_U32    criteriaParamBmp; /* criteria Param Bmp */
    GT_U32    mask;             /* work mask */
    GT_U32    i;                /* parameter index */
    GT_U32    fieldValArr[CP_TO_RX_FIELDS_NUM] = {0,0,0,0,0,0,0,0};   /* parameters converted to HW values */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */
    const GT_U32 *fieldCpToRxOffsetArr; /* pointer to array of field offsets */
    const GT_U32 *fieldCpToRxSizeArr;   /* pointer to array of field sizes */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(criteriaParamBmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(multicastPacketPtr);
    CPSS_NULL_PTR_CHECK_MAC(cutThroughPacketPtr);
    CPSS_NULL_PTR_CHECK_MAC(cutThroughTerminatedPtr);
    CPSS_NULL_PTR_CHECK_MAC(trunkatedHeaderPtr);
    CPSS_NULL_PTR_CHECK_MAC(dummyDescriptorPtr);
    CPSS_NULL_PTR_CHECK_MAC(highPriorityPacketPtr);
    CPSS_NULL_PTR_CHECK_MAC(contextDonePtr);
    CPSS_NULL_PTR_CHECK_MAC(descriptorValidPtr);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum,&dpIndex,&localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.cp_to_rx_cnt[0].cp_to_rx_count_type_mask;
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, &hwValueMask);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.cp_to_rx_cnt[0].cp_to_rx_count_type_ref;
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, &hwValueRef);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        fieldCpToRxOffsetArr = sip6_30_fieldCpToRxOffsetArr;
        fieldCpToRxSizeArr   = sip6_30_fieldCpToRxSizeArr;
    }
    else
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        fieldCpToRxOffsetArr = sip6_10_fieldCpToRxOffsetArr;
        fieldCpToRxSizeArr   = sip6_10_fieldCpToRxSizeArr;
    }
    else
    {
        fieldCpToRxOffsetArr = sip6_fieldCpToRxOffsetArr;
        fieldCpToRxSizeArr   = sip6_fieldCpToRxSizeArr;
    }

    criteriaParamBmp = 0;
    for (i = 0; (i < CP_TO_RX_FIELDS_NUM); i++)
    {
        /* bypass unsupported fields*/
        if (fieldCpToRxSizeArr[i] == 0) continue;

        mask = ((1 << fieldCpToRxSizeArr[i]) - 1);
        fieldValArr[i] = ((hwValueRef >> fieldCpToRxOffsetArr[i]) & mask);
        if ((mask  << fieldCpToRxOffsetArr[i]) & hwValueMask)
        {
            criteriaParamBmp |= (1 << i);
        }
    }

    *criteriaParamBmpPtr     = criteriaParamBmp;
    *multicastPacketPtr      = BIT2BOOL_MAC(fieldValArr[0]);
    *cutThroughPacketPtr     = BIT2BOOL_MAC(fieldValArr[1]);
    *cutThroughTerminatedPtr = BIT2BOOL_MAC(fieldValArr[2]);
    *trunkatedHeaderPtr      = BIT2BOOL_MAC(fieldValArr[3]);
    *dummyDescriptorPtr      = BIT2BOOL_MAC(fieldValArr[4]);
    *highPriorityPacketPtr   = BIT2BOOL_MAC(fieldValArr[5]);
    *contextDonePtr          = BIT2BOOL_MAC(fieldValArr[6]);
    *descriptorValidPtr      = BIT2BOOL_MAC(fieldValArr[7]);

    return GT_OK;
}

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
*                                                     GT_FALSE - descriptor invalid.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugCpToRxCountingConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, criteriaParamBmpPtr,
         multicastPacketPtr, cutThroughPacketPtr, cutThroughTerminatedPtr,
         trunkatedHeaderPtr, dummyDescriptorPtr,
         highPriorityPacketPtr, contextDonePtr, descriptorValidPtr));

    rc = internal_cpssDxChPortDpDebugCpToRxCountingConfigGet(
        devNum, unitRepresentingPort, criteriaParamBmpPtr,
        multicastPacketPtr, cutThroughPacketPtr, cutThroughTerminatedPtr,
        trunkatedHeaderPtr, dummyDescriptorPtr,
        highPriorityPacketPtr, contextDonePtr, descriptorValidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, criteriaParamBmpPtr,
         multicastPacketPtr, cutThroughPacketPtr, cutThroughTerminatedPtr,
         trunkatedHeaderPtr, dummyDescriptorPtr,
         highPriorityPacketPtr, contextDonePtr, descriptorValidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortDpDebugCpToRxCountValueGet function
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
static GT_STATUS internal_cpssDxChPortDpDebugCpToRxCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    globalDmaNum;     /* (global) rxdma port number  */
    GT_U32    localDmaNum = 0;  /* local rxdma port number  */
    GT_U32    dpIndex = GT_NA;  /* indication for a specific DMA that counting for specific 'local DMA port' */
    GT_U32    dummy;            /* place to read not used data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(countValuePtr);

    /* check and convert physical portNum to global portRxdmaNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(
        devNum, unitRepresentingPort, globalDmaNum);
    /* convert global portRxdmaNum to local_portRxdmaNum and to DP index  */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, globalDmaNum,&dpIndex,&localDmaNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.cp_to_rx_cnt[0].cp_to_rx_count;
    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, regAddr, countValuePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    /* needed to read to unlock counter */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(devNum, dpIndex).
        debug.cp_to_rx_cnt[0].cp_to_rx_count_high;
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc =  prvCpssDrvHwPpPortGroupReadRegister(
            devNum, 0/*portGroupId*/, regAddr, &dummy);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDpDebugCpToRxCountValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, unitRepresentingPort, countValuePtr));

    rc = internal_cpssDxChPortDpDebugCpToRxCountValueGet(
        devNum, unitRepresentingPort, countValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, unitRepresentingPort, countValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


