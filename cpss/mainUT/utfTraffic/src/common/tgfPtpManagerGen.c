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
* @file tgfPtpManagerGen.c
*
* @brief Generic API for Presision Time Protocol (PTP) Manager API.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPtpGen.h>
#include <common/tgfPtpManagerGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
    #include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#endif /* CHX_FAMILY */

/******************************************************************************\
 *                              Macro definitions                             *
\******************************************************************************/

#ifdef CHX_FAMILY

#endif /* CHX_FAMILY */

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY

static GT_STATUS tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT
(
    IN  PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT    tgfCsUpdMode,
    OUT CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  *dxchCsUpdModePtr
)
{
    switch (tgfCsUpdMode)
    {
        case PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E:
            *dxchCsUpdModePtr = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E;
            break;
        case PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E:
            *dxchCsUpdModePtr = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E;
            break;
        case PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E:
            *dxchCsUpdModePtr = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TAI_NUMBER_ENT
(
    IN  PRV_TGF_PTP_TAI_NUMBER_ENT    tgfTaiNumber,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT  *dxchTaiNumberPtr
)
{
    switch (tgfTaiNumber)
    {
        case PRV_TGF_PTP_TAI_NUMBER_0_E:
            *dxchTaiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
            break;
        case PRV_TGF_PTP_TAI_NUMBER_1_E:
            *dxchTaiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_1_E;
            break;
        case PRV_TGF_PTP_TAI_NUMBER_ALL_E:
            *dxchTaiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TAI_OUTPUT_INTERFACE_MODE_ENT
(
    IN  PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT    tgfTaiOutputInterface,
    OUT CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT  *dxchTaiOutputInterfacePtr
)
{
    switch (tgfTaiOutputInterface)
    {
        case PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E:
            *dxchTaiOutputInterfacePtr = CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E;
            break;
        case PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E:
            *dxchTaiOutputInterfacePtr = CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E;
            break;
        case PRV_TGF_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E:
            *dxchTaiOutputInterfacePtr = CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TAI_PULSE_IN_MODE_ENT
(
    IN  PRV_TGF_PTP_TAI_PULSE_IN_MODE_ENT    tgfTaiPulseInMode,
    OUT CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT  *dxchTaiPulseInModePtr
)
{
    switch (tgfTaiPulseInMode)
    {
        case PRV_TGF_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E:
            *dxchTaiPulseInModePtr = CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E;
            break;
        case PRV_TGF_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E:
            *dxchTaiPulseInModePtr = CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E;
            break;
        case PRV_TGF_PTP_TAI_PULSE_IN_DISABLED_E:
            *dxchTaiPulseInModePtr = CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TS_DELAY_TAI_SEL_MODE_ENT
(
    IN  PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT    tgfTsDelayTaiSelMode,
    OUT CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT  *dxchTsDelayTaiSelModePtr
)
{
    switch (tgfTsDelayTaiSelMode)
    {
        case PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E:
            *dxchTsDelayTaiSelModePtr = CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E;
            break;
        case PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E:
            *dxchTsDelayTaiSelModePtr = CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_INGRESS_DOMAIN_MODE_ENT
(
    IN  PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT    tgfIngressDomainMode,
    OUT CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  *dxchIngressDomainModePtr
)
{
    switch (tgfIngressDomainMode)
    {
        case PRV_TGF_PTP_INGRESS_DOMAIN_MODE_DISABLE_E:
            *dxchIngressDomainModePtr = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E;
            break;
        case PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E:
            *dxchIngressDomainModePtr = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E;
            break;
        case PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E:
            *dxchIngressDomainModePtr = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TOD_COUNTER_FUNC_ENT
(
    IN  PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT   tgfFunction,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT *dxchFunctionPtr
)
{
    switch (tgfFunction)
    {
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_UPDATE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_INCREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_CAPTURE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_GENERATE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_DECREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_NOP_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT
(
    IN  PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT   tgfMessageType,
    OUT CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT *dxchMessageTypePtr
)
{
    switch (tgfMessageType)
    {
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_SYNC_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED4_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED4_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED5_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED5_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED6_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED6_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED7_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED7_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_FOLLOW_UP_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_FOLLOW_UP_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ANNOUNCE_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ANNOUNCE_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_SIGNALING_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SIGNALING_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_MANAGMENT_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_MANAGMENT_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED14_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED14_E;
            break;
        case PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_RESERVED15_E:
            *dxchMessageTypePtr = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED15_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TS_ACTION_ENT
(
    IN  PRV_TGF_PTP_TS_ACTION_ENT    tgfTsAction,
    OUT CPSS_DXCH_PTP_TS_ACTION_ENT  *dxchTsActionPtr
)
{
    switch (tgfTsAction)
    {
        case PRV_TGF_PTP_TS_ACTION_NONE_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_NONE_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_DROP_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_DROP_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TS_TAG_MODE_ENT
(
    IN  PRV_TGF_PTP_TS_TAG_MODE_ENT    tgfTsTag,
    OUT CPSS_DXCH_PTP_TS_TAG_MODE_ENT  *dxchTsTagPtr
)
{
    switch (tgfTsTag)
    {
        case PRV_TGF_PTP_TS_TAG_MODE_NONE_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E:
            *dxchTsTagPtr = CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}
#if 0
static GT_STATUS tgf2dxchPTP_TS_PACKET_TYPE_ENT
(
    IN  PRV_TGF_PTP_TS_PACKET_TYPE_ENT    tgfPacketType,
    OUT CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT  *dxchPacketTypePtr
)
{
    switch (tgfPacketType)
    {
        case PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TS_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_NTP_RX_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TX_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_WAMP_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif
static GT_STATUS dxch2tgfPTP_TS_PACKET_TYPE_ENT
(
    IN  CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT  dxchPacketType,
    OUT PRV_TGF_PTP_TS_PACKET_TYPE_ENT    *tgfPacketTypePtr
)
{
    switch (dxchPacketType)
    {
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_RESERVED_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_RESERVED_E;
            break;
        default: return GT_BAD_STATE;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChPtpManagerOutputInterfaceStc
*           function
* @endinternal
*
* @brief   Convert generic Ptp Manager output interface stc into
*          device specific Ptp Manager output interface stc
*
* @param[in] outputInterfacePtr       - (pointer to) Ptp Manager output interface structure
*
* @param[out] dxChOutputInterfacePtr  - (pointer to) DxCh Ptp Manager output interface structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPtpManagerOutputInterfaceStc
(
    IN  PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfacePtr,
    OUT CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC *dxChOutputInterfacePtr
)
{
    GT_STATUS rc;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChOutputInterfacePtr, 0, sizeof(CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC));

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(outputInterfacePtr->taiNumber,&(dxChOutputInterfacePtr->taiNumber));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }
    rc = tgf2dxchPTP_TAI_OUTPUT_INTERFACE_MODE_ENT(outputInterfacePtr->outputInterfaceMode,&(dxChOutputInterfacePtr->outputInterfaceMode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_OUTPUT_INTERFACE_MODE_ENT FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOutputInterfacePtr, outputInterfacePtr, nanoSeconds);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChPtpManagerPtpOverEthernetStc
*           function
* @endinternal
*
* @brief   Convert generic Ptp Manager Ptp Over Ethernet stc into
*          device specific Ptp Manager Ptp Over Ethernet stc
*
* @param[in] ptpOverEthernetPtr       - (pointer to) Ptp Manager Ptp Over Ethernet structure
*
* @param[out] dxChPtpOverEthernetPtr  - (pointer to) DxCh Ptp Manager Ptp Over Ethernet structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPtpManagerPtpOverEthernetStc
(
    IN  PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC   *ptpOverEthernetPtr,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC *dxChPtpOverEthernetPtr
)
{
    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChPtpOverEthernetPtr, 0, sizeof(CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpOverEthernetPtr, ptpOverEthernetPtr, ptpOverEthernetEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpOverEthernetPtr, ptpOverEthernetPtr, etherType0value);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpOverEthernetPtr, ptpOverEthernetPtr, etherType1value);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChPtpManagerPtpOverIpUdpStc
*           function
* @endinternal
*
* @brief   Convert generic Ptp Manager Ptp Over Ethernet stc into
*          device specific Ptp Manager Ptp Over Ethernet stc
*
* @param[in] ptpOverEthernetPtr       - (pointer to) Ptp Manager Ptp Over Ethernet structure
*
* @param[out] dxChPtpOverEthernetPtr  - (pointer to) DxCh Ptp Manager Ptp Over Ethernet structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPtpManagerPtpOverIpUdpStc
(
    IN  PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC   *ptpOverIpUdpPtr,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC *dxChPtpOverIpUdpPtr
)
{
    GT_STATUS rc;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChPtpOverIpUdpPtr, 0, sizeof(CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpOverIpUdpPtr, ptpOverIpUdpPtr, ptpOverIpUdpEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpOverIpUdpPtr, ptpOverIpUdpPtr, udpPort0value);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpOverIpUdpPtr, ptpOverIpUdpPtr, udpPort1value);
    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(ptpOverIpUdpPtr->udpCsUpdMode.ptpIpv4Mode, &(dxChPtpOverIpUdpPtr->udpCsUpdMode.ptpIpv4Mode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT FAILED, rc = [%d]", rc);
    }
    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(ptpOverIpUdpPtr->udpCsUpdMode.ptpIpv6Mode, &(dxChPtpOverIpUdpPtr->udpCsUpdMode.ptpIpv6Mode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT FAILED, rc = [%d]", rc);
    }
    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(ptpOverIpUdpPtr->udpCsUpdMode.ntpIpv4Mode, &(dxChPtpOverIpUdpPtr->udpCsUpdMode.ntpIpv4Mode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT FAILED, rc = [%d]", rc);
    }
    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(ptpOverIpUdpPtr->udpCsUpdMode.ntpIpv6Mode, &(dxChPtpOverIpUdpPtr->udpCsUpdMode.ntpIpv6Mode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT FAILED, rc = [%d]", rc);
    }
    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(ptpOverIpUdpPtr->udpCsUpdMode.wampIpv4Mode, &(dxChPtpOverIpUdpPtr->udpCsUpdMode.wampIpv4Mode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT FAILED, rc = [%d]", rc);
    }
    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(ptpOverIpUdpPtr->udpCsUpdMode.wampIpv6Mode, &(dxChPtpOverIpUdpPtr->udpCsUpdMode.wampIpv6Mode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT FAILED, rc = [%d]", rc);
    }

    return GT_OK;
}

/*
* @internal prvTgfConvertGenericToDxChPtpTsTagGlobalConfStc
*           function
* @endinternal
*
* @brief   Convert generic Ptp Manager timestamp tag global
*          configuration stc into device specific Ptp timestamp
*          tag global configuration stc
*
* @param[in] ptpTsTagGlobalConfPtr       - (pointer to) Ptp Manager ts tag global config structure
*
* @param[out] dxChPtpTsTagGlobalConfPtr  - (pointer to) DxCh Ptp ts tag global config structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPtpTsTagGlobalConfStc
(
    IN  PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC   *ptpTsTagGlobalConfPtr,
    OUT CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC         *dxChPtpTsTagGlobalConfPtr
)
{

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChPtpTsTagGlobalConfPtr, 0, sizeof(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTsTagGlobalConfPtr, ptpTsTagGlobalConfPtr, tsTagParseEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTsTagGlobalConfPtr, ptpTsTagGlobalConfPtr, hybridTsTagParseEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTsTagGlobalConfPtr, ptpTsTagGlobalConfPtr, tsTagEtherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTsTagGlobalConfPtr, ptpTsTagGlobalConfPtr, hybridTsTagEtherType);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc
*           function
* @endinternal
*
* @brief   Convert generic Ptp Manager Ptp Tod Count stc into
*          device specific Ptp Manager Ptp Tod Count stc
*
* @param[in] ptpTodCountPtr       - (pointer to) Ptp Manager Ptp Tod Count structure
*
* @param[out] dxChPtpTodCountPtr  - (pointer to) DxCh Ptp Manager Ptp Tod Count structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc
(
    IN  PRV_TGF_PTP_TOD_COUNT_STC   *ptpTodCountPtr,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC *dxChPtpTodCountPtr
)
{
    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChPtpTodCountPtr, 0, sizeof(CPSS_DXCH_PTP_TOD_COUNT_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTodCountPtr, ptpTodCountPtr, nanoSeconds);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTodCountPtr, ptpTodCountPtr, seconds);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPtpTodCountPtr, ptpTodCountPtr, fracNanoSeconds);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericPtpManagerPtpTodCountStc
*           function
* @endinternal
*
* @brief   Convert device specific Ptp Manager Ptp Tod Count stc
*          into generic Ptp Manager Ptp Tod Count stc
*
*
* @param[in] dxChPtpTodCountPtr    - (pointer to) DxCh Ptp Manager Ptp Tod Count structure
*
* @param[out] ptpTodCountPtr       - (pointer to) Ptp Manager Ptp Tod Count structure
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericPtpManagerPtpTodCountStc
(
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC *dxChPtpTodCountPtr,
    OUT PRV_TGF_PTP_TOD_COUNT_STC   *ptpTodCountPtr

)
{
    /* reset variables */
    cpssOsMemSet((GT_VOID*) ptpTodCountPtr, 0, sizeof(PRV_TGF_PTP_TOD_COUNT_STC));

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChPtpTodCountPtr, ptpTodCountPtr, nanoSeconds);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChPtpTodCountPtr, ptpTodCountPtr, seconds);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChPtpTodCountPtr, ptpTodCountPtr, fracNanoSeconds);

    return GT_OK;
}

#endif /* CHX_FAMILY */

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC      dxChOutputInterfaceConf;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC         dxChPtpOverEthernetConf;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC           dxChPtpOverIpUdpConf;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC                 dxChPtpTsTagGlobalConf;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChOutputInterfaceConf, 0, sizeof(CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC));
    cpssOsMemSet((GT_VOID*) &dxChPtpOverEthernetConf, 0, sizeof(CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC));
    cpssOsMemSet((GT_VOID*) &dxChPtpOverIpUdpConf, 0, sizeof(CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC));
    cpssOsMemSet((GT_VOID*) &dxChPtpTsTagGlobalConf, 0, sizeof(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC));

    rc = prvTgfConvertGenericToDxChPtpManagerOutputInterfaceStc(outputInterfaceConfPtr,&dxChOutputInterfaceConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerOutputInterfaceStc FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpManagerPtpOverEthernetStc(ptpOverEthernetConfPtr,&dxChPtpOverEthernetConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerPtpOverEthernetStc FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpManagerPtpOverIpUdpStc(ptpOverIpUdpConfPtr,&dxChPtpOverIpUdpConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerPtpOverIpUdpStc FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpTsTagGlobalConfStc(ptpTsTagGlobalConfPtr,&dxChPtpTsTagGlobalConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpTsTagGlobalConfStc FAILED, rc = [%d]", rc);
    }

    /* Pause memory leakage counting to avoid false memory leakage errors. */
    if(utfMemoryLeakPauseSet_func) utfMemoryLeakPauseSet_func(GT_TRUE);

    rc = cpssDxChPtpManagerPtpInit(devNum,&dxChOutputInterfaceConf,&dxChPtpOverEthernetConf,&dxChPtpOverIpUdpConf,&dxChPtpTsTagGlobalConf,cpuCode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerPtpInit FAILED, rc = [%d]", rc);
    }

    /* enable memory leakage counting after manager was created. */
    if(utfMemoryLeakPauseSet_func) utfMemoryLeakPauseSet_func(GT_FALSE);

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(outputInterfaceConfPtr);
    TGF_PARAM_NOT_USED(ptpOverEthernetConfPtr);
    TGF_PARAM_NOT_USED(ptpOverIpUdpConfPtr);
    TGF_PARAM_NOT_USED(ptpTsTagGlobalConfPtr);
    TGF_PARAM_NOT_USED(cpuCode);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     dxChInputMode = CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = tgf2dxchPTP_TAI_PULSE_IN_MODE_ENT(inputMode,&dxChInputMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_PULSE_IN_MODE_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerInputPulseSet(devNum,dxChTaiNumber,dxChInputMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerInputPulseSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(inputMode);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerMasterPpsActivate(devNum,dxChTaiNumber,phaseValue,seconds,nanoSeconds);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerMasterPpsActivate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(phaseValue);
    TGF_PARAM_NOT_USED(seconds);
    TGF_PARAM_NOT_USED(nanoSeconds);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerSlavePpsActivate(devNum,dxChTaiNumber,phaseValue,seconds,nanoSeconds);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerSlavePpsActivate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(phaseValue);
    TGF_PARAM_NOT_USED(seconds);
    TGF_PARAM_NOT_USED(nanoSeconds);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT dxChEgressPortBindMode = CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = tgf2dxchPTP_TS_DELAY_TAI_SEL_MODE_ENT(egressPortBindMode,&dxChEgressPortBindMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_DELAY_TAI_SEL_MODE_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerPortAllocateTai(devNum,portNum,dxChTaiNumber,dxChEgressPortBindMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerPortAllocateTai FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(egressPortBindMode);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerCaptureTodValueSet(devNum,dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerCaptureTodValueSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_TOD_COUNT_STC             dxChTodValue;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc(todValuePtr,&dxChTodValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerTodValueSet(devNum,dxChTaiNumber,&dxChTodValue,capturePrevTodValueEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerTodValueSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(todValuePtr);
    TGF_PARAM_NOT_USED(capturePrevTodValueEnable);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_TAI_ID_STC                taiId;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerTodValueFrequencyUpdate(devNum,dxChTaiNumber,updateValue,capturePrevTodValueEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerTodValueFrequencyUpdate FAILED, rc = [%d]", rc);
    }

    /* trigger update*/
    taiId.taiNumber = dxChTaiNumber;
    rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTodCounterFunctionTriggerSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(updateValue);
    TGF_PARAM_NOT_USED(capturePrevTodValueEnable);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_TOD_COUNT_STC             dxChUpdateValue;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      dxChFunction = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E;
    CPSS_DXCH_PTP_TAI_ID_STC                taiId;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc(updateValuePtr,&dxChUpdateValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc FAILED, rc = [%d]", rc);
    }

    rc = tgf2dxchPTP_TOD_COUNTER_FUNC_ENT(function,&dxChFunction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TOD_COUNTER_FUNC_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerTodValueUpdate(devNum,dxChTaiNumber,&dxChUpdateValue,dxChFunction,gracefulStep,capturePrevTodValueEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerTodValueSet FAILED, rc = [%d]", rc);
    }

    /* trigger update*/
    taiId.taiNumber = dxChTaiNumber;
    rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTodCounterFunctionTriggerSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(updateValuePtr);
    TGF_PARAM_NOT_USED(function);
    TGF_PARAM_NOT_USED(gracefulStep);
    TGF_PARAM_NOT_USED(capturePrevTodValueEnable);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC dxChTodValueEntry0;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC dxChTodValueEntry1;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerTodValueGet(devNum,dxChTaiNumber,&dxChTodValueEntry0, &dxChTodValueEntry1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerTodValueGet FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertDxChToGenericPtpManagerPtpTodCountStc(&(dxChTodValueEntry0.todValue),&(todValueEntry0Ptr->todValue));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericPtpManagerPtpTodCountStc FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChTodValueEntry0, todValueEntry0Ptr, todValueIsValid);

    rc = prvTgfConvertDxChToGenericPtpManagerPtpTodCountStc(&(dxChTodValueEntry1.todValue),&(todValueEntry1Ptr->todValue));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericPtpManagerPtpTodCountStc FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChTodValueEntry1, todValueEntry1Ptr, todValueIsValid);

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(todValueEntry0Ptr);
    TGF_PARAM_NOT_USED(todValueEntry1Ptr);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_TOD_COUNT_STC             dxChTriggerTime;
    CPSS_DXCH_PTP_TOD_COUNT_STC             dxChMaskValue;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc(triggerTimePtr,&dxChTriggerTime);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc(maskValuePtr,&dxChMaskValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPtpManagerPtpTodCountStc FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerTriggerGenerate(devNum,dxChTaiNumber,extPulseWidth, &dxChTriggerTime, maskEnable, &dxChMaskValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerTriggerGenerate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(extPulseWidth);
    TGF_PARAM_NOT_USED(triggerTimePtr);
    TGF_PARAM_NOT_USED(maskEnable);
    TGF_PARAM_NOT_USED(maskValuePtr);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U32 i;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC             dxChDomainConf;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainConf, domainConfPtr, domainIndex);
    rc = tgf2dxchPTP_INGRESS_DOMAIN_MODE_ENT(domainConfPtr->domainMode,&dxChDomainConf.domainMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_INGRESS_DOMAIN_MODE_ENT FAILED, rc = [%d]", rc);
    }

    for (i=0; i<4; i++) {
        dxChDomainConf.v1DomainIdArr[i] = domainConfPtr->v1DomainIdArr[i];
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainConf, domainConfPtr, v2DomainId);

    rc = cpssDxChPtpManagerGlobalPtpDomainSet(devNum,&dxChDomainConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerGlobalPtpDomainSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(domainConfPtr);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT             dxChMessageType;

    rc = tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT(messageType,&dxChMessageType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerPortCommandAssignment(devNum,portNum,domainIndex,dxChMessageType,command);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerPortCommandAssignment FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainIndex);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(command);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                    dxChTaiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC           dxChDomainEntry;
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT          dxChMessageType;
    CPSS_DXCH_PTP_TS_ACTION_ENT                     dxChAction = CPSS_DXCH_PTP_TS_ACTION_NONE_E;

    rc = tgf2dxchPTP_TAI_NUMBER_ENT(taiNumber,&dxChTaiNumber);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TAI_NUMBER_ENT FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainEntry, domainEntryPtr, ptpOverEhernetTsEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainEntry, domainEntryPtr, ptpOverUdpIpv4TsEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainEntry, domainEntryPtr, ptpOverUdpIpv6TsEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainEntry, domainEntryPtr, messageTypeTsEnableBmp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDomainEntry, domainEntryPtr, transportSpecificTsEnableBmp);

    rc = tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT(messageType,&dxChMessageType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT FAILED, rc = [%d]", rc);
    }

    rc = tgf2dxchPTP_TS_ACTION_ENT(action,&dxChAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_ACTION_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerPortPacketActionsConfig(devNum,portNum,dxChTaiNumber,domainIndex,&dxChDomainEntry,dxChMessageType,dxChAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerPortCommandAssignment FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(taiNumber);
    TGF_PARAM_NOT_USED(domainIndex);
    TGF_PARAM_NOT_USED(domainEntryPtr);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(action);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC               dxChTsTagPortCfg;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChTsTagPortCfg, tsTagPortCfgPtr, tsReceptionEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChTsTagPortCfg, tsTagPortCfgPtr, tsPiggyBackEnable);
    rc = tgf2dxchPTP_TS_TAG_MODE_ENT(tsTagPortCfgPtr->tsTagMode,&(dxChTsTagPortCfg.tsTagMode));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_TS_TAG_MODE_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerPortTsTagConfig(devNum,portNum,&dxChTsTagPortCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerPortTsTagConfig FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tsTagPortCfgPtr);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT             dxChMessageType;

    rc = tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT(messageType,&dxChMessageType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgf2dxchPTP_MANAGER_MESSAGE_TYPE_ENT FAILED, rc = [%d]", rc);
    }

    rc = cpssDxChPtpManagerTsQueueConfig(devNum,dxChMessageType,queueNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerPortCommandAssignment FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(queueNum);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC             dxChEntry;

    rc = cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(devNum,queueNum,&dxChEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, entryValid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, isPtpExeption);
    rc = dxch2tgfPTP_TS_PACKET_TYPE_ENT(dxChEntry.packetFormat,&(entryPtr->packetFormat));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: dxch2tgfPTP_TS_PACKET_TYPE_ENT FAILED, rc = [%d]", rc);
    }
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, messageType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, domainNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, sequenceId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, queueEntryId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, portNum);

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(queueNum);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

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
    IN  GT_U8                                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                                    portNum,
    IN  GT_U32                                                  queueNum,
    OUT PRV_TGF_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC    *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC       dxChEntry;

    rc = cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(devNum,portNum,queueNum,&dxChEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampPortQueueEntryRead FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, entryValid);
    entryPtr->queueEntryId = dxChEntry.queueEntryId;
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, taiSelect);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, todUpdateFlag);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, seconds);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, nanoSeconds);

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(queueNum);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

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
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC             dxChEntry;

    rc = cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead(devNum,queueNum,&dxChEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, entryValid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, isPtpExeption);
    dxch2tgfPTP_TS_PACKET_TYPE_ENT(dxChEntry.packetFormat,&(entryPtr->packetFormat));
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, taiSelect);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, todUpdateFlag);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, messageType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, domainNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, messageType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, sequenceId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, timestamp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChEntry, entryPtr, portNum);

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(queueNum);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}