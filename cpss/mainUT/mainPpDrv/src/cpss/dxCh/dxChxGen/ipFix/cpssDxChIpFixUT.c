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
* @file cpssDxChIpFixUT.c
*
* @brief Unit tests for cpssDxChIpFix, that provides
* The CPSS DXCH IpFix APIs
*
* @version   13
********************************************************************************
*/
/* includes */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicer.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Tests use this vlan id for testing VLAN functions */
#define IPFIX_VLAN_TESTED_VLAN_ID_CNS 100

/* marco to check if IPLR second stage exists */
#define CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(_dev)    \
    (GT_TRUE == PRV_CPSS_DXCH_PP_MAC(_dev)->fineTuning.featureInfo.iplrSecondStageSupported)

/* marco to check if EPLR exists */
#define CHECK_EPLR_EXISTS_MAC(_dev) \
    (PRV_CPSS_DXCH_PP_MAC(_dev)->fineTuning.tableSize.egressPolicersNum > 0)

/* Policer stage number: Ingress #0, Ingress #1 or Egress */
static CPSS_DXCH_POLICER_STAGE_TYPE_ENT plrStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

/**
* @internal prvUtfCompareIpfixEntries function
* @endinternal
*
* @brief   The function compares between configurable fields of IPFIX entries
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] ipfixEntry1Ptr           - pointer to the first IPFIX Entry.
* @param[in] ipfixEntry2Ptr           - pointer to the second IPFIX Entry.
*
* @retval GT_TRUE                  - entries are the same.
* @retval GT_FALSE                 - entries are different.
*/
static GT_BOOL prvUtfCompareIpfixEntries
(
    CPSS_DXCH_IPFIX_ENTRY_STC   *ipfixEntry1Ptr,
    CPSS_DXCH_IPFIX_ENTRY_STC   *ipfixEntry2Ptr
)
{
    if( ipfixEntry1Ptr->packetCount         == ipfixEntry2Ptr->packetCount          &&
        ipfixEntry1Ptr->byteCount.l[0]      == ipfixEntry2Ptr->byteCount.l[0]       &&
        ipfixEntry1Ptr->byteCount.l[1]      == ipfixEntry2Ptr->byteCount.l[1]       &&
        ipfixEntry1Ptr->dropCounter         == ipfixEntry2Ptr->dropCounter          &&
        ipfixEntry1Ptr->samplingWindow.l[0] == ipfixEntry2Ptr->samplingWindow.l[0]  &&
        ipfixEntry1Ptr->samplingWindow.l[1] == ipfixEntry2Ptr->samplingWindow.l[1]  &&
        ipfixEntry1Ptr->samplingAction      == ipfixEntry2Ptr->samplingAction       &&
        ipfixEntry1Ptr->logSamplingRange    == ipfixEntry2Ptr->logSamplingRange     &&
        ipfixEntry1Ptr->randomFlag          == ipfixEntry2Ptr->randomFlag           &&
        ipfixEntry1Ptr->samplingMode        == ipfixEntry2Ptr->samplingMode         &&
        ipfixEntry1Ptr->cpuSubCode          == ipfixEntry2Ptr->cpuSubCode           &&
        ipfixEntry1Ptr->timeStamp           == ipfixEntry2Ptr->timeStamp            &&
        ipfixEntry1Ptr->randomOffset        == ipfixEntry2Ptr->randomOffset         &&
        ipfixEntry1Ptr->lastSampledValue.l[0] == ipfixEntry2Ptr->lastSampledValue.l[0]  &&
        ipfixEntry1Ptr->lastSampledValue.l[1] == ipfixEntry2Ptr->lastSampledValue.l[1])
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixAgingEnableSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixAgingEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
              and enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage     = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_BOOL                          enable    = GT_TRUE;
    GT_BOOL                          enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                      and enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] and enable [GT_TRUE]*/
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        enable = GT_TRUE;

        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] and enable [GT_TRUE]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E] and enable [GT_TRUE]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);


        /*call with [GT_FALSE]*/
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        enable = GT_FALSE;

        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);


        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] and enable [GT_FALSE]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E] and enable [GT_FALSE]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call cpssDxChIpfixAgingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enableGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d", dev);

        /*
            1.3. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        enable = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixAgingEnableSet
                            (dev, stage, enable),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixAgingEnableSet(dev, stage, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixAgingEnableGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixAgingEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
              and enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_BOOL enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, enable);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixAgingEnableGet
                            (dev, stage, &enable),
                            stage);

        /*
            1.3. Call with wrong enable pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixAgingEnableGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, stage, enable);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixAgingEnableGet(dev, stage, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixAgingEnableGet(dev, stage, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixAgingStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixAgingStatusGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
              and reset [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32  startIndex = 0;
    GT_U32  endIndex = 0;
    GT_BOOL reset = GT_TRUE;
    GT_U32  bmpPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixAgingStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixAgingStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E] and reset [GT_FALSE*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }
        reset  = GT_FALSE;

        st = cpssDxChIpfixAgingStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixAgingStatusGet
                            (dev, stage, startIndex, endIndex, reset, &bmpPtr),
                            stage);

        /*
            1.3. Call with wrong bmp pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixAgingStatusGet(dev, stage, startIndex,
                                         endIndex, reset, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixAgingStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixAgingStatusGet(dev, stage, startIndex,
                                     endIndex, reset, &bmpPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixAlarmEventsGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E].
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong eventsNumPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32  eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] = {0};
    GT_U32  eventsNumPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixAlarmEventsGet(dev, stage, eventsArr, &eventsNumPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixAlarmEventsGet(dev, stage, eventsArr, &eventsNumPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E] and reset [GT_FALSE]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixAlarmEventsGet(dev, stage, eventsArr, &eventsNumPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixAlarmEventsGet
                            (dev, stage, eventsArr, &eventsNumPtr),
                            stage);

        /*
            1.3. Call with wrong eventsNumPtr pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixAlarmEventsGet(dev, stage, eventsArr, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixAlarmEventsGet(dev, stage, eventsArr, &eventsNumPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixAlarmEventsGet(dev, stage, eventsArr, &eventsNumPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixCpuCodeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_NET_RX_CPU_CODE_ENT         cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixCpuCodeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
             and cpuCode [(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)]
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixCpuCodeGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage      = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode    = 0;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCodeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                     and cpuCode [CPSS_NET_FIRST_USER_DEFINED_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]
         * and cpuCode [CPSS_NET_FIRST_USER_DEFINED_E]*/
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

        st = cpssDxChIpfixCpuCodeSet(dev, stage, cpuCode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixCpuCodeGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCodeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                                     "get another cpuCode than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] and cpuCode [(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixCpuCodeSet(dev, stage, cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);

        /*
            1.2. Call cpssDxChIpfixCpuCodeGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCodeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                                     "get another cpuCode than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E] and cpuCode [(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixCpuCodeSet(dev, stage, cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);

        /*
            1.2. Call cpssDxChIpfixCpuCodeGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCodeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                                     "get another cpuCode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        cpuCode = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);

        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixCpuCodeSet
                            (dev, stage, cpuCode),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixCpuCodeSet(dev, stage, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixCpuCodeSet(dev, stage, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixCpuCodeGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT        *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixCpuCodeGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong cpuCode [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_NET_RX_CPU_CODE_ENT        cpuCode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, cpuCode);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixCpuCodeGet
                            (dev, stage, &cpuCode),
                            stage);

        /*
            1.3. Call with wrong cpuCode pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixCpuCodeGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, stage, cpuCode);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixCpuCodeGet(dev, stage, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixDropCountModeSet
(
    IN GT_U8                                 devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    IN CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixDropCountModeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
        and mode [CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E /
                  CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E].
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage   = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT mode    = 0;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT modeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                and mode [CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E /
                          CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]
         * and mode [CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E]*/
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        mode  = CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E;

        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &modeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        /*
            1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &modeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        /*
            1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &modeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]
         * and mode [CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E]*/
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        mode  = CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E;

        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &modeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        /*
            1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &modeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        /*
            1.2. Call cpssDxChIpfixDropCountModeGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &modeGet);

        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                     "get another mode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixDropCountModeSet
                            (dev, stage, mode),
                            stage);

        /*
            1.4. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixDropCountModeSet
                            (dev, stage, mode),
                            mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixDropCountModeSet(dev, stage, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixDropCountModeGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT     *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixDropCountModeGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong mode [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixDropCountModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixDropCountModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixDropCountModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, stage, mode);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixDropCountModeGet
                            (dev, stage, &mode),
                            stage);

        /*
            1.3. Call with wrong mode pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixDropCountModeGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, stage, mode);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixDropCountModeGet(dev, stage, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixDropCountModeGet(dev, stage, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixEntrySet
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage,
    IN  GT_U32                           entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC       *ipfixEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixEntrySet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
       entryIndex[0 / max - 1],

       ipfixEntry{timeStamp             [0 / BIT_16 - 1];
                  packetCount           [0 / BIT_30 - 1];
                  byteCount.l[0]        [0 / 100];
                  byteCount.l[1]        [0 / BIT_4  - 1];
                  dropCounter           [0 / BIT_30 - 1];
                  randomOffset          [0 / BIT_31];
                  lastSampledValue.l[0] [0 / 100];
                  lastSampledValue.l[1] [0 / BIT_4  - 1];
                  samplingWindow.l[0]   [0 / 100];
                  samplingWindow.l[1]   [0 / BIT_4  - 1];
                  samplingAction        [CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E/
                                         CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E];
                  logSamplingRange      [0 / BIT_6 - 1];
                  randomFlag            [CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E/
                                         CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E];
                  samplingMode          [CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E /
                                         CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E /
                                         CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E];
                  cpuSubCode            [0 / BIT_2 - 1] }
        Expected: GT_OK.
    1.2. Call cpssDxChIpfixEntryGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong stage [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong entryIndex [max].
    Expected: NOT GT_OK.
    1.5. Call with wrong ipfixEntry.timeStamp = BIT_16.
    Expected: NOT GT_OK.
    1.6. Call with wrong ipfixEntry.packetCount = BIT_30;
    Expected: NOT GT_OK.
    1.7. Call with wrong ipfixEntry.byteCount.l[1] = BIT_4;
    Expected: NOT GT_OK.
    1.8. Call with wrong ipfixEntry.dropCounter = BIT_30;
    Expected: NOT GT_OK.
    1.9. Call with wrong ipfixEntry.lastSampledValue.l[1] = BIT_4;
    Expected: NOT GT_OK.
    1.10. Call with wrong ipfixEntry.samplingWindow.l[1] = BIT_4;
    Expected: NOT GT_OK.
    1.11. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
    Expected: NOT GT_OK with random flag.
    1.12. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
    Expected: GT_OK with deterministic flag.
    1.13. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
    Expected: NOT GT_OK with alarm sampling action.
    1.14. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
    Expected: GT_OK with mirror sampling action (not relevant).
    1.15. Call api with wrong ipfixEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.16. Call with wrong entryIndex[max].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                            entryIndex = 0;
    CPSS_DXCH_IPFIX_ENTRY_STC         ipfixEntry;
    CPSS_DXCH_IPFIX_ENTRY_STC         ipfixEntryGet;

    CPSS_DXCH_POLICER_COUNTING_MODE_ENT saveCountMode;

    GT_BOOL     reset = GT_FALSE;
    GT_BOOL     isEqual = GT_FALSE;

    cpssOsBzero((GT_VOID*)&ipfixEntry, sizeof(ipfixEntry));
    cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E],
               entryIndex[0 / max - 1],

               ipfixEntry{timeStamp             [0 / BIT_16 - 1];
                          packetCount           [0 / BIT_30 - 1];
                          byteCount.l[0]        [0 / 100];
                          byteCount.l[1]        [0 / BIT_4  - 1];
                          dropCounter           [0 / BIT_30 - 1];
                          randomOffset          [0 / BIT_31];
                          lastSampledValue.l[0] [0 / 100];
                          lastSampledValue.l[1] [0 / BIT_4  - 1];
                          samplingWindow.l[0]   [0 / 100];
                          samplingWindow.l[1]   [0 / BIT_4  - 1];
                          samplingAction        [CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E/
                                                 CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E];
                          logSamplingRange      [0 / BIT_6 - 1];
                          randomFlag            [CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E/
                                                 CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E];
                          samplingMode          [CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E /
                                                 CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E /
                                                 CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E];
                          cpuSubCode            [0 / BIT_2 - 1] }
                Expected: GT_OK.
        */

        /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E],
                     entryIndex[0] */
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        entryIndex = 0;

        ipfixEntry.timeStamp             = 0;
        ipfixEntry.packetCount           = 0;
        ipfixEntry.byteCount.l[0]        = 0;
        ipfixEntry.byteCount.l[1]        = 0;
        ipfixEntry.dropCounter           = 0;
        ipfixEntry.randomOffset          = 0;
        ipfixEntry.lastSampledValue.l[0] = 0;
        ipfixEntry.lastSampledValue.l[1] = 0;
        ipfixEntry.samplingWindow.l[0]   = 0;
        ipfixEntry.samplingWindow.l[1]   = 0;
        ipfixEntry.samplingAction        = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
        ipfixEntry.logSamplingRange      = 0;
        ipfixEntry.randomFlag            = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
        ipfixEntry.samplingMode          = CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E;
        ipfixEntry.cpuSubCode            = 0;

        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChIpfixEntryGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChIpfixEntryGet: %d ", dev);

        /* Verifying values */
        isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another ipfixEntry than was set: %d", dev);

        /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E],
                     entryIndex[max - 1] */
        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

        ipfixEntry.timeStamp             = BIT_16 - 1;
        ipfixEntry.packetCount           = BIT_30 - 1;
        ipfixEntry.byteCount.l[0]        = 100;
        ipfixEntry.byteCount.l[1]        = BIT_4  - 1;
        ipfixEntry.dropCounter           = BIT_30 - 1;
        ipfixEntry.randomOffset          = BIT_31;
        ipfixEntry.lastSampledValue.l[0] = 100;
        ipfixEntry.lastSampledValue.l[1] = BIT_4  - 1;
        ipfixEntry.samplingWindow.l[0]   = 100;
        ipfixEntry.samplingWindow.l[1]   = BIT_4  - 1;
        ipfixEntry.samplingAction        = CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;
        ipfixEntry.logSamplingRange      = BIT_6 - 1;
        ipfixEntry.randomFlag            = CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E;
        ipfixEntry.samplingMode          = CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E;
        ipfixEntry.cpuSubCode            = BIT_2 - 1;

        /* get current stage 1 policer counting mode */
        st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* set stage 1 policer counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage,
                               CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */

        cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixEntryGet: %d ", dev);

        /* Verifying values */
        isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another ipfixEntry than was set: %d", dev);

        /* restore counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                     entryIndex[max - 1] */
        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        /* get current egress stage policer counting mode */
        st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* set egress stage policer counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage,
                               CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

        ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
        ipfixEntry.samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E;

        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixEntryGet: %d ", dev);

        /* Verifying values */
        isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another ipfixEntry than was set: %d", dev);

        /*
            1.3. Call api with wrong stage [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixEntrySet
                            (dev, stage, entryIndex, &ipfixEntry),
                            stage);

        /*
            1.4. Call with wrong entryIndex [max].
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

        /*
            1.5. Call with wrong ipfixEntry.timeStamp = BIT_16.
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.timeStamp = BIT_16;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.timeStamp = BIT_16 - 1;

        /*
            1.6. Call with wrong ipfixEntry.packetCount = BIT_30;
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.packetCount = BIT_30;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.packetCount = BIT_30 - 1;

        /*
            1.7. Call with wrong ipfixEntry.byteCount.l[1] = BIT_4;
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.byteCount.l[1] = BIT_4;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.byteCount.l[1] = BIT_4  - 1;

        /*
            1.8. Call with wrong ipfixEntry.dropCounter = BIT_30;
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.dropCounter = BIT_30;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.dropCounter = BIT_30 - 1;

        /*
            1.9. Call with wrong ipfixEntry.lastSampledValue.l[1] = BIT_4;
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.lastSampledValue.l[1] = BIT_4;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.lastSampledValue.l[1] = BIT_4  - 1;

        /*
            1.10. Call with wrong ipfixEntry.samplingWindow.l[1] = BIT_4;
            Expected: NOT GT_OK.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.samplingWindow.l[1] = BIT_4;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.samplingWindow.l[1] = BIT_4  - 1;

        /*
            1.11. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
            Expected: NOT GT_OK with random flag.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.randomFlag = CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E;
        ipfixEntry.logSamplingRange = BIT_6;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.logSamplingRange = BIT_6 - 1;

        /*
            1.12. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
            Expected: GT_OK with deterministic flag.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.randomFlag = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
        ipfixEntry.logSamplingRange = BIT_6;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.logSamplingRange = BIT_6 - 1;

        /*
            1.13. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
            Expected: NOT GT_OK with alarm sampling action.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
        ipfixEntry.cpuSubCode = BIT_2;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.cpuSubCode = BIT_2 - 1;

        /*
            1.14. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
            Expected: NOT GT_OK too.
        */
        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;
        ipfixEntry.cpuSubCode = BIT_2;

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ipfixEntry.cpuSubCode = BIT_2 - 1;

        /*
            1.15. Call api with wrong ipfixEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ipfixEntryPtr = NULL", dev);

        /*
            1.16. Call with wrong entryIndex[max].
            Expected: NOT GT_OK.
        */
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

        /*
            1.17. Call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E].
                   samplingAction[CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E].
            Expected: GT_OK for Falcon.
        */
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            entryIndex = 0;

            ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;
            ipfixEntry.cpuSubCode = BIT_2 - 1;

            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
            st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another ipfixEntry than was set: %d", dev);
        }

        /*
            1.18. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] and mode disable.
            Expected: GT_OUT_OF_RANGE for AC5P.
                  Call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E], mode disable and timestamp = BIT16.
            Expected: GT_OUT_OF_RANGE for AC5P.
                  Same as above with timestamp BIT16-1 and pha metadata mode = 4.
            Expected: GT_BAD_PARAM for AC5P.
                  Same as above with pha metadata mode disable and last packet command = 0[FORWARD].
            Expected: GT_BAD_PARAM for AC5P.
                  Same as above with lastPacketCommand CPSS_PACKET_CMD_MIRROR_TO_CPU_E and lastCpuOrDropCode CPSS_NET_FIRST_USER_DEFINED_E - 1.
            Expected: GT_BAD_PARAM for AC5P.
        */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
        {
            /*
                1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                      CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                      CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                   entryIndex[0 / max - 1],

                   ipfixEntry{timeStamp                    [0 / BIT_16 - 1];
                              packetCount                  [0 / BIT_30 - 1];
                              byteCount.l[0]               [0 / 100];
                              byteCount.l[1]               [0 / BIT_4  - 1];
                              dropCounter                  [0 / BIT_30 - 1];
                              randomOffset                 [0 / BIT_31];
                              lastSampledValue.l[0]        [0 / 100];
                              lastSampledValue.l[1]        [0 / BIT_4  - 1];
                              samplingWindow.l[0]          [0 / 100];
                              samplingWindow.l[1]          [0 / BIT_4  - 1];
                              samplingAction               [CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E/
                                                            CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E];
                              logSamplingRange             [0 / BIT_6 - 1];
                              randomFlag                   [CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E/
                                                            CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E];
                              samplingMode                 [CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E];
                                                            CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E];
                              cpuSubCode                   [0 / BIT_2 - 1];
                              firstPacketsCounter          [0 / BIT_31];
                              numberOfFirstPacketsToMirror [0 / BIT_31];
                              phaMetadataMode              [CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E/
                                                            CPSS_DXCH_IPFIX_PHA_METADATA_MODE_RANDOM_E];
                              firstTimestamp               [0 / BIT_16 - 1];
                              firstTimestampValid          [GT_FALSE / GT_TRUE];
                              lastPacketCommand            [CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_DROP_SOFT_E];
                              lastCpuOrDropCode            [CPSS_NET_FIRST_USER_DEFINED_E / CPSS_NET_LAST_USER_DEFINED_E];}
                    Expected: GT_OK.
            */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            entryIndex = 0;

            ipfixEntry.timeStamp                    = 0;
            ipfixEntry.packetCount                  = 0;
            ipfixEntry.byteCount.l[0]               = 0;
            ipfixEntry.byteCount.l[1]               = 0;
            ipfixEntry.dropCounter                  = 0;
            ipfixEntry.randomOffset                 = 0;
            ipfixEntry.lastSampledValue.l[0]        = 0;
            ipfixEntry.lastSampledValue.l[1]        = 0;
            ipfixEntry.samplingWindow.l[0]          = 0;
            ipfixEntry.samplingWindow.l[1]          = 0;
            ipfixEntry.samplingAction               = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
            ipfixEntry.logSamplingRange             = 0;
            ipfixEntry.randomFlag                   = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
            ipfixEntry.samplingMode                 = CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E;
            ipfixEntry.cpuSubCode                   = 0;
            ipfixEntry.firstPacketsCounter          = 0;
            ipfixEntry.numberOfFirstPacketsToMirror = 0;
            ipfixEntry.phaMetadataMode              = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E;
            ipfixEntry.firstTimestamp               = 0;
            ipfixEntry.firstTimestampValid          = GT_FALSE;
            ipfixEntry.lastPacketCommand            = CPSS_PACKET_CMD_FORWARD_E;
            ipfixEntry.lastCpuOrDropCode            = CPSS_NET_FIRST_USER_DEFINED_E;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChIpfixEntryGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
            st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpfixEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);

            /* 1.3. call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E],
                         entryIndex[max - 1] */
            if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            }

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

            /* Only one fields of MUXed fields to be set */
            ipfixEntry.timeStamp                    = BIT_16 - 1;
            ipfixEntry.packetCount                  = BIT_30 - 1;
            ipfixEntry.byteCount.l[0]               = 100;
            ipfixEntry.byteCount.l[1]               = BIT_4  - 1;
            ipfixEntry.dropCounter                  = BIT_30 - 1;
            ipfixEntry.randomOffset                 = 0;
            ipfixEntry.lastSampledValue.l[0]        = 0;
            ipfixEntry.lastSampledValue.l[1]        = 0;
            ipfixEntry.samplingWindow.l[0]          = 0;
            ipfixEntry.samplingWindow.l[1]          = 0;
            ipfixEntry.samplingAction               = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
            ipfixEntry.logSamplingRange             = 0;
            ipfixEntry.randomFlag                   = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
            ipfixEntry.samplingMode                 = CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E;
            ipfixEntry.cpuSubCode                   = 0;
            ipfixEntry.firstPacketsCounter          = BIT_31;
            ipfixEntry.numberOfFirstPacketsToMirror = BIT_31;
            ipfixEntry.phaMetadataMode              = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E;
            ipfixEntry.firstTimestamp               = BIT_16 - 1;
            ipfixEntry.firstTimestampValid          = GT_TRUE;
            ipfixEntry.lastPacketCommand            = CPSS_PACKET_CMD_DROP_SOFT_E;
            ipfixEntry.lastCpuOrDropCode            = CPSS_NET_LAST_USER_DEFINED_E;

            /* get current stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* set stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage,
                                   CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.4. call get func again, 1.2. */

            cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
            st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);

            /* Only one fields of MUXed fields to be set */
            ipfixEntry.timeStamp                    = BIT_16 - 1;
            ipfixEntry.packetCount                  = BIT_30 - 1;
            ipfixEntry.byteCount.l[0]               = 100;
            ipfixEntry.byteCount.l[1]               = BIT_4  - 1;
            ipfixEntry.dropCounter                  = BIT_30 - 1;
            ipfixEntry.randomOffset                 = BIT_31;
            ipfixEntry.lastSampledValue.l[0]        = 100;
            ipfixEntry.lastSampledValue.l[1]        = BIT_4 - 1;
            ipfixEntry.samplingWindow.l[0]          = 100;
            ipfixEntry.samplingWindow.l[1]          = BIT_4 - 1;
            ipfixEntry.samplingAction               = CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;
            ipfixEntry.logSamplingRange             = BIT_6 - 1;
            ipfixEntry.randomFlag                   = CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E;
            ipfixEntry.samplingMode                 = CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E;
            ipfixEntry.cpuSubCode                   = BIT_2 - 1;
            ipfixEntry.firstPacketsCounter          = 0;
            ipfixEntry.numberOfFirstPacketsToMirror = 0;
            ipfixEntry.phaMetadataMode              = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E;
            ipfixEntry.firstTimestamp               = 0;
            ipfixEntry.firstTimestampValid          = GT_FALSE;
            ipfixEntry.lastPacketCommand            = CPSS_PACKET_CMD_FORWARD_E;
            ipfixEntry.lastCpuOrDropCode            = CPSS_NET_FIRST_USER_DEFINED_E;

            /* get current stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* set stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage,
                                   CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.4. call get func again, 1.2. */

            cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
            st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);

            /* restore counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* 1.5. call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                         entryIndex[max - 1] */
            if(CHECK_EPLR_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            }

            /* get current egress stage policer counting mode */
            st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* set egress stage policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage,
                                   CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

            ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
            ipfixEntry.samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E;
            ipfixEntry.phaMetadataMode = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_COUNTER_E;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.2. */
            cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));
            st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);

            /* 1.6 call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E],
                    and phaMetadataMode[CPSS_DXCH_IPFIX_PHA_METADATA_MODE_COUNTER_E]
                Expected: GT_BAD_PARAM. */
            ipfixEntry.phaMetadataMode       = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_COUNTER_E;
            ipfixEntry.samplingMode          = CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E;
            stage                            = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.7. call with wrong ipfixEntry.firstTimestamp BIT_16
                Expected: GT_BAD_PARAM. */
            stage                            = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            ipfixEntry.firstTimestamp        = BIT_16;
            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            /* 1.8. call with wrong ipfixEntry.phaMetadataMode
             *   CPSS_DXCH_IPFIX_PHA_METADATA_MODE_RANDOM_E + 1
                Expected: GT_BAD_PARAM. */
            ipfixEntry.firstTimestamp         = BIT_16 - 1;
            ipfixEntry.phaMetadataMode        = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_RANDOM_E + 1;
            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.9. call with wrong ipfixEntry.lastPacketCommand
             *   CPSS_PACKET_CMD_BRIDGE_E
                Expected: GT_BAD_PARAM. */
            ipfixEntry.phaMetadataMode        = CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E;
            ipfixEntry.lastPacketCommand      = CPSS_PACKET_CMD_BRIDGE_E;
            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.10. call with wrong ipfixEntry.lastCpuOrDropCode
             *   CPSS_NET_FIRST_USER_DEFINED_E - 1
                Expected: GT_BAD_PARAM. */
            ipfixEntry.lastPacketCommand      = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            ipfixEntry.lastCpuOrDropCode      = CPSS_NET_FIRST_USER_DEFINED_E - 1;
            st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            ipfixEntry.lastCpuOrDropCode      = CPSS_NET_FIRST_USER_DEFINED_E;

            /* restore counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixEntryGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1.1 Call with stage CPSS_DXCH_POLICER_STAGE_INGRESS_0_E
                    reset CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E,
    Expected: GT_OK.

    1.1.2 Call with stage CPSS_DXCH_POLICER_STAGE_INGRESS_0_E
                    reset CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E.

    for SIP 6.10 and above Expected: GT_OK.
    for other devices Expected: GT_BAD_PARAM.

    1.1.3 Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                           CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                    reset CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E,
    Expected: GT_OK.

    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong ipfixEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                           entryIndex = 0;
    CPSS_DXCH_IPFIX_RESET_MODE_ENT   reset = CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E;
    CPSS_DXCH_IPFIX_ENTRY_STC        ipfixEntryGet;
    CPSS_DXCH_POLICER_COUNTING_MODE_ENT saveCountMode;

    cpssOsBzero((GT_VOID*)&ipfixEntryGet, sizeof(ipfixEntryGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* SIP6_10 has lastCpuOrDropCode field where 0 is invalid value so setting
         * it as per allowed enum.
         */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
        {
            ipfixEntryGet.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E;
        }

        /*
           1.1.1 call with stage CPSS_DXCH_POLICER_STAGE_INGRESS_0_E
                           reset CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E
                 Expected: GT_OK.
        */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
           1.1.2 Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]
                           reset [CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E].

                for SIP 6.10      Expected: GT_OK.
                for other devices Expected: GT_BAD_PARAM.
        */

        reset = CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E;
        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, stage);
        }

        /*
            1.1.3 Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                   CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                            reset  CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E,
            Expected: GT_OK.
        */

        reset = CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E;
        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }
        /* get current stage 1 policer counting mode */
        st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* set stage 1 policer counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage,
                                            CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* retrieve back stage 1 policer counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        /* get current egress stage policer counting mode */
        st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /* set stage egress policer counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage,
                                            CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);


        st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntrySet(dev, stage, entryIndex, &ipfixEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixEntryGet
                            (dev, stage, entryIndex, reset, &ipfixEntryGet),
                            stage);

        /*
            1.3. Call with wrong mode pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, stage, reset);

        /* retrieve back egress stage policer counting mode */
        st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixEntryGet(dev, stage, entryIndex, reset, &ipfixEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimerGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_TIMER_STC       *timerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimerGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong timer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_TIMER_STC       timer;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixTimerGet(dev, stage, &timer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixTimerGet(dev, stage, &timer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixTimerGet(dev, stage, &timer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixTimerGet
                            (dev, stage, &timer),
                            stage);

        /*
            1.3. Call with wrong timer pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixTimerGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimerGet(dev, stage, &timer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimerGet(dev, stage, &timer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimestampUploadSet
(
    IN GT_U8                                 devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    IN CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    *uploadPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimestampUploadSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E].
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    upload;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    uploadGet;

    GT_BOOL isEqual = GT_FALSE;

    cpssOsBzero((GT_VOID*)&upload, sizeof(upload));
    cpssOsBzero((GT_VOID*)&uploadGet, sizeof(uploadGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                                  upload - zeroes,
            and upload.uploadMode = CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E;
                upload.timer.nanoSecondTimer = 1000;
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]*/
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                     (GT_VOID*) &(uploadGet),
                                     sizeof(upload))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another upload than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                     (GT_VOID*) &(uploadGet),
                                     sizeof(upload))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another upload than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                     (GT_VOID*) &(uploadGet),
                                     sizeof(upload))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another upload than was set: %d", dev);


        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            and upload.uploadMode = CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E;
                upload.timer.nanoSecondTimer = 1000;
            Expected: GT_OK.
        */

        upload.uploadMode = CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E;
        upload.timer.nanoSecondTimer = 1000;

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]*/
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                     (GT_VOID*) &(uploadGet),
                                     sizeof(upload))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another upload than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                     (GT_VOID*) &(uploadGet),
                                     sizeof(upload))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another upload than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixTimestampUploadGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                     (GT_VOID*) &(uploadGet),
                                     sizeof(upload))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another upload than was set: %d", dev);

        /* check if WA for nanoseconds is enabled */
        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_XCAT_TOD_NANO_SEC_SET_WA_E))
        {
            /* set nanosecond to value that is not a multiply of 20 */
            upload.timer.nanoSecondTimer = 1010;

            st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            upload.timer.nanoSecondTimer = 1000;
            st = cpssDxChIpfixTimestampUploadGet(dev, stage, &uploadGet);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(upload),
                                         (GT_VOID*) &(uploadGet),
                                         sizeof(upload))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                         "get another upload than was set: %d", dev);
        }

        /*
            1.3. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixTimestampUploadSet
                            (dev, stage, &upload),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimestampUploadSet(dev, stage, &upload);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimestampUploadGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC   *uploadPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimestampUploadGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong upload [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC  upload;

    cpssOsBzero((GT_VOID*)&upload, sizeof(upload));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &upload);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixTimestampUploadGet
                            (dev, stage, &upload),
                            stage);

        /*
            1.3. Call with wrong upload pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixTimestampUploadGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimestampUploadGet(dev, stage, &upload);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimestampUploadGet(dev, stage, &upload);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimestampUploadStatusGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *uploadStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimestampUploadStatusGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with non NULL uploadStatusPtr.
    Expected: GT_OK.
    1.2. Call with wrong uploadStatusPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     uploadStatus;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non NULL uploadStatusPtr.
            Expected: GT_OK.
        */

        st = cpssDxChIpfixTimestampUploadStatusGet(dev, &uploadStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong upload pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixTimestampUploadStatusGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimestampUploadStatusGet(dev, &uploadStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimestampUploadStatusGet(dev, &uploadStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimestampUploadTrigger
(
    IN GT_U8    devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimestampUploadTrigger)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with non NULL dev.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non NULL dev.
            Expected: GT_OK.
        */

        st = cpssDxChIpfixTimestampUploadTrigger(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimestampUploadTrigger(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimestampUploadTrigger(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixWraparoundConfSet
(
    IN GT_U8                                 devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    IN CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC   *confPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixWraparoundConfSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E].
                          and conf - zeroes or
        conf.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
        conf.dropThreshold = 10;
        conf.packetThreshold = 100;
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC   conf;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC   confGet;

    GT_BOOL isEqual = GT_FALSE;

    cpssOsBzero((GT_VOID*)&conf, sizeof(conf));
    cpssOsBzero((GT_VOID*)&confGet, sizeof(confGet));

    /* In order for wraparound mechanizm to work well byte threshold must be */
    /* at least 0x40000. */
    conf.byteThreshold.l[0] = 0x40000;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
                                  and conf - zeroes or
                conf.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
                conf.dropThreshold = 10;
                conf.packetThreshold = 100;
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]*/
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &confGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(conf),
                                     (GT_VOID*) &(confGet),
                                     sizeof(conf))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another conf than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &confGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(conf),
                                     (GT_VOID*) &(confGet),
                                     sizeof(conf))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another conf than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &confGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(conf),
                                     (GT_VOID*) &(confGet),
                                     sizeof(conf))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another conf than was set: %d", dev);


        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                                  and conf - zeroes or
                conf.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
                conf.dropThreshold = 10;
                conf.packetThreshold = 100;
            Expected: GT_OK.
        */

        conf.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
        conf.dropThreshold = 10;
        conf.packetThreshold = 100;

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] and
            conf.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
            conf.dropThreshold = 10;
            conf.packetThreshold = 100; */
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &confGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(conf),
                                     (GT_VOID*) &(confGet),
                                     sizeof(conf))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another conf than was set: %d", dev);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &confGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(conf),
                                     (GT_VOID*) &(confGet),
                                     sizeof(conf))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another conf than was set: %d", dev);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call cpssDxChIpfixWraparoundConfGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &confGet);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(conf),
                                     (GT_VOID*) &(confGet),
                                     sizeof(conf))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                  "get another conf than was set: %d", dev);

        /*
            1.3. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixWraparoundConfSet
                            (dev, stage, &conf),
                            stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixWraparoundConfSet(dev, stage, &conf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixWraparoundConfGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC *confPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixWraparoundConfGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong conf [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC  conf;

    cpssOsBzero((GT_VOID*)&conf, sizeof(conf));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }

        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &conf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixWraparoundConfGet
                            (dev, stage, &conf),
                            stage);

        /*
            1.3. Call with wrong conf pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixWraparoundConfGet(dev, stage, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixWraparoundConfGet(dev, stage, &conf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixWraparoundConfGet(dev, stage, &conf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixWraparoundStatusGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                          CPSS_DXCH_POLICER_STAGE_EGRESS_E],
               and reset [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with wrong enum values stage.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32  startIndex = 0;
    GT_U32  endIndex = 0;
    GT_BOOL reset = GT_TRUE;
    GT_U32  bmpPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                  CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                  CPSS_DXCH_POLICER_STAGE_EGRESS_E].
            Expected: GT_OK.
        */

        /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
        stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

        st = cpssDxChIpfixWraparoundStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
            stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
        }

        st = cpssDxChIpfixWraparoundStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        if(CHECK_EPLR_EXISTS_MAC(dev))
        {
            /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E] and reset [GT_FALSE]*/
            stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
        }
        reset  = GT_FALSE;

        st = cpssDxChIpfixWraparoundStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

        /*
            1.2. Call with wrong enum values stage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixWraparoundStatusGet
                            (dev, stage, startIndex, endIndex, reset, &bmpPtr),
                            stage);

        /*
            1.3. Call with wrong bmp pointer [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpfixWraparoundStatusGet(dev, stage, startIndex,
                                         endIndex, reset, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, stage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixWraparoundStatusGet(dev, stage, startIndex,
                                         endIndex, reset, &bmpPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixWraparoundStatusGet(dev, stage, startIndex,
                                     endIndex, reset, &bmpPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPortGroupAgingStatusGet
(
    IN  GT_U8                             dev,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage,
    IN  GT_U32                            startIndex,
    IN  GT_U32                            endIndex,
    IN  GT_BOOL                           reset,
    OUT GT_U32                            *bmpPtr
)
*/

#define IPFIX_BC2_A0_MEM_SIZE_CNS _2K
#define IPFIX_BC2_B0_MEM_SIZE_CNS _8K
#define IPFIX_BC3_A0_MEM_SIZE_CNS _16K
#define IPFIX_AC5P_MEM_SIZE_CNS _64K


/* calculate IPFIX memory size in words for biggest memory of IPFIX - AC5P */
#define  IPFIX_BMP_SIZE_CNS (sizeof(GT_U32) * (IPFIX_AC5P_MEM_SIZE_CNS/32))

UTF_TEST_CASE_MAC(cpssDxChIpfixPortGroupAgingStatusGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChXcat and above)
    1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                           CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                           CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                   startIndex[0 / max - 1],
                   endIndex  [0 / max - 1],
                   reset[GT_TRUE / GT_FALSE / GT_TRUE],
    Expected: GT_OK.
    1.1.2. Call api with wrong stage [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.3. Call api with wrong bmpPtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call with wrong startIndex[max].
    Expected: NOT GT_OK.
    1.1.5. Call with wrong endIndex[max].
    Expected: NOT GT_OK.
    1.1.6. Call with endIndex more than startIndex.
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;

    GT_U32    portGroupId;

    GT_U8                             dev;
    GT_PORT_GROUPS_BMP                portGroupsBmp = 1;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                            startIndex = 0;
    GT_U32                            endIndex = 0;
    GT_BOOL                           reset = GT_FALSE;
    GT_U32                           *bmpPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    bmpPtr = cpssOsMalloc(IPFIX_BMP_SIZE_CNS);
    if (bmpPtr == NULL)
    {
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)bmpPtr,
                                        "cpssOsMalloc: Memory allocation error.");
        return;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                       CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                       CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                               startIndex[0 / max - 1],
                               endIndex  [0 / max - 1],
                               reset[GT_TRUE / GT_FALSE / GT_TRUE],
                Expected: GT_OK.
            */
            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E],
                         startIndex[0],
                         endIndex[0],
                         reset[GT_TRUE] */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            startIndex = 0;
            endIndex   = 0;
            reset = GT_TRUE;

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E],
                         startIndex[0],
                         endIndex[max - 1],
                         reset[GT_FALSE] */

            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

            if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            }

            startIndex = 0;
            endIndex   = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (endIndex >= IPFIX_BC2_A0_MEM_SIZE_CNS)
                {
                    endIndex = IPFIX_BC2_A0_MEM_SIZE_CNS -1;
                }
            }

            if(endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS && !PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC(endIndex, endIndex,
                                    "endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS");
            }
            reset = GT_FALSE;

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                         startIndex[0],
                         endIndex[max - 1],
                         reset[GT_TRUE] */
            if(CHECK_EPLR_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            }

            startIndex = 0;
            endIndex   = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (endIndex >= IPFIX_BC2_A0_MEM_SIZE_CNS)
                {
                    endIndex = IPFIX_BC2_A0_MEM_SIZE_CNS -1;
                }
            }

            reset = GT_TRUE;

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong stage [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPortGroupAgingStatusGet
                                (dev, portGroupsBmp, stage,
                                startIndex, endIndex, reset, bmpPtr),
                                stage);

            /*
                1.1.3. Call api with wrong bmpPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpfixPortGroupAgingStatusGet(dev,  portGroupsBmp,
                                  stage, startIndex, endIndex, reset, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bmpPtr = NULL", dev);

            /*
                1.1.4. Call with wrong startIndex[max].
                Expected: NOT GT_OK.
            */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            startIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            startIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (startIndex >= IPFIX_BC2_A0_MEM_SIZE_CNS)
                {
                    startIndex = IPFIX_BC2_A0_MEM_SIZE_CNS -1;
                }
            }
            if(endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS && !PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC(startIndex, startIndex,
                    "startIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS");
            }

            /*
                1.1.5. Call with wrong endIndex[max].
                Expected: NOT GT_OK.
            */
            endIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            endIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (endIndex >= IPFIX_BC2_A0_MEM_SIZE_CNS)
                {
                    endIndex = IPFIX_BC2_A0_MEM_SIZE_CNS -1;
                }
            }
            if(endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS && !PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC(endIndex, endIndex,
                    "endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS");
            }

            /*
                1.1.6. Call with endIndex more than startIndex.
                Expected: NOT GT_OK.
            */
            startIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];
            endIndex   = 0;

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            startIndex = 0;
            endIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (endIndex >= IPFIX_BC2_A0_MEM_SIZE_CNS)
                {
                    endIndex = IPFIX_BC2_A0_MEM_SIZE_CNS -1;
                }
            }
            if(endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS && !PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC(endIndex, endIndex,
                    "endIndex >= IPFIX_BC2_B0_MEM_SIZE_CNS");
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixPortGroupAgingStatusGet(dev,  portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpfixPortGroupAgingStatusGet(dev,  portGroupsBmp,
                              stage, startIndex, endIndex, reset, bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    startIndex = 0;
    endIndex = 0;
    reset = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPortGroupAgingStatusGet(dev,  portGroupsBmp,
                              stage, startIndex, endIndex, reset, bmpPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPortGroupAgingStatusGet(dev,  portGroupsBmp,
                          stage, startIndex, endIndex, reset, bmpPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    cpssOsFree(bmpPtr);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPortGroupAlarmEventsGet
(
    IN  GT_U8                             dev,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage,
    OUT GT_U32                            eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                            *eventsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixPortGroupAlarmEventsGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChXcat and above)
    1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                           CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                           CPSS_DXCH_POLICER_STAGE_EGRESS_E],
    Expected: GT_OK.
    1.1.2. Call api with wrong stage [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.3. Call api with wrong eventsArr [NULL].
    Expected: NOT GT_OK.
    1.1.4. Call api with wrong eventsNumPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8                             dev;
    GT_PORT_GROUPS_BMP                portGroupsBmp = 1;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                            eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] = {0};
    GT_U32                            eventsNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                       CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                       CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                Expected: GT_OK.
            */

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

            st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;

            st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E] */
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;

            st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong stage [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPortGroupAlarmEventsGet
                                (dev, portGroupsBmp, stage,
                                eventsArr, &eventsNum),
                                stage);

            /*
                1.1.3. Call api with wrong eventsArr [NULL].
                Expected: NOT GT_OK.
            */
            st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                                                stage, NULL, &eventsNum);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.4. Call api with wrong eventsNumPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                                                stage, eventsArr, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, eventsNumPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPortGroupAlarmEventsGet(dev, portGroupsBmp,
                            stage, eventsArr, &eventsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPortGroupEntryGet
(
    IN  GT_U8                             dev,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage,
    IN  GT_U32                            entryIndex,
    IN  GT_BOOL                           reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC         *ipfixEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixPortGroupEntryGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChXcat and above)
    1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                           CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                           CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                   entryIndex[0 / max-1],
                   reset[GT_TRUE / GT_FALSE / GT_TRUE],
    Expected: GT_OK.
    1.1.2. Call api with wrong stage [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.3. Call api with wrong ipfixEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call with wrong entryIndex[max].
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8                             dev;
    GT_PORT_GROUPS_BMP                portGroupsBmp = 1;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                            entryIndex = 0;
    GT_BOOL                           reset = GT_FALSE;
    CPSS_DXCH_IPFIX_ENTRY_STC         ipfixEntry;

    CPSS_DXCH_POLICER_COUNTING_MODE_ENT saveCountMode;

    /* Zero structures first */
    cpssOsBzero((GT_VOID*) &ipfixEntry, sizeof(ipfixEntry));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* SIP6_10 has lastCpuOrDropCode field where 0 is invalid value so setting
         * it as per allowed enum.
         */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
        {
            ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E;
        }
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                       CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                       CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                               entryIndex[0 / max-1],
                               reset[GT_TRUE / GT_FALSE / GT_TRUE],
                Expected: GT_OK.
            */

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E],
                         entryIndex[0],
                         reset[GT_TRUE] */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            entryIndex = 0;
            reset = GT_TRUE;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                      stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                   stage, entryIndex, reset, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E],
                         entryIndex[max - 1],
                         reset[GT_FALSE] */
            if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
                stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            }

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

            /* get current stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* set stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage,
                                 CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            reset = GT_FALSE;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                      stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                   stage, entryIndex, reset, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                         entryIndex[max - 1],
                         reset[GT_TRUE] */

            /* retrieve back stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            if(CHECK_EPLR_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            }

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            reset = GT_TRUE;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                      stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                   stage, entryIndex, reset, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);


            /*
                1.1.2. Call api with wrong stage [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPortGroupEntryGet
                                (dev, portGroupsBmp, stage,
                                entryIndex, reset, &ipfixEntry),
                                stage);

            /*
                1.1.3. Call api with wrong ipfixEntryPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                       stage, entryIndex, reset, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, ipfixEntryPtr = NULL", dev);

            /*
                1.1.4. Call with wrong entryIndex[max].
                Expected: NOT GT_OK.
            */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                   stage, entryIndex, reset, &ipfixEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, stage);

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        entryIndex = 0;
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                   stage, entryIndex, reset, &ipfixEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                           stage, entryIndex, reset, &ipfixEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    entryIndex = 0;
    reset = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                       stage, entryIndex, reset, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                       stage, entryIndex, reset, &ipfixEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPortGroupEntrySet
(
    IN  GT_U8                             dev,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage,
    IN  GT_U32                            entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC         *ipfixEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixPortGroupEntrySet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChXcat and above)
    1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                           CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                           CPSS_DXCH_POLICER_STAGE_EGRESS_E],
       entryIndex[0 / max - 1],
       ipfixEntry{timeStamp             [0 / BIT_16 - 1];
                  packetCount           [0 / BIT_30 - 1];
                  byteCount.l[0]        [0 / 100];
                  byteCount.l[1]        [0 / BIT_4  - 1];
                  dropCounter           [0 / BIT_30 - 1];
                  randomOffset          [0 / BIT_31];
                  lastSampledValue.l[0] [0 / 100];
                  lastSampledValue.l[1] [0 / BIT_4  - 1];
                  samplingWindow.l[0]   [0 / 100];
                  samplingWindow.l[1]   [0 / BIT_4  - 1];
                  samplingAction        [CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E/
                                         CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E];
                  logSamplingRange      [0 / BIT_6 - 1];
                  randomFlag            [CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E/
                                         CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E];
                  samplingMode          [CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E /
                                         CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E /
                                         CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E];
                  cpuSubCode            [0 / BIT_2 - 1] }
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpfixPortGroupEntryGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong stage [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong entryIndex [max].
    Expected: NOT GT_OK.
    1.1.5. Call with wrong ipfixEntry.timeStamp = BIT_16.
    Expected: NOT GT_OK.
    1.1.6. Call with wrong ipfixEntry.packetCount = BIT_30;
    Expected: NOT GT_OK.
    1.1.7. Call with wrong ipfixEntry.byteCount.l[1] = BIT_4;
    Expected: NOT GT_OK.
    1.1.8. Call with wrong ipfixEntry.dropCounter = BIT_30;
    Expected: NOT GT_OK.
    1.1.9. Call with wrong ipfixEntry.lastSampledValue.l[1] = BIT_4;
    Expected: NOT GT_OK.
    1.1.10. Call with wrong ipfixEntry.samplingWindow.l[1] = BIT_4;
    Expected: NOT GT_OK.
    1.1.11. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
    Expected: NOT GT_OK with random flag.
    1.1.12. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
    Expected: GT_OK with deterministic flag.
    1.1.13. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
    Expected: NOT GT_OK with alarm sampling action.
    1.1.14. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
    Expected: GT_OK with not alarm sampling action (not relevant).
    1.1.15. Call api with wrong ipfixEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;
    GT_BOOL   isEqual = GT_FALSE;

    GT_U8                             dev;
    GT_PORT_GROUPS_BMP                portGroupsBmp = 1;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                            entryIndex = 0;
    CPSS_DXCH_IPFIX_ENTRY_STC         ipfixEntry;
    CPSS_DXCH_IPFIX_ENTRY_STC         ipfixEntryGet;
    GT_BOOL                           reset = GT_FALSE;

    CPSS_DXCH_POLICER_COUNTING_MODE_ENT saveCountMode;

    /* Zero structures first */
    cpssOsBzero((GT_VOID*) &ipfixEntry, sizeof(ipfixEntry));
    cpssOsBzero((GT_VOID*) &ipfixEntryGet, sizeof(ipfixEntryGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                       CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                       CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                               entryIndex[0 / max - 1],

                   ipfixEntry{timeStamp             [0 / BIT_16 - 1];
                              packetCount           [0 / BIT_30 - 1];
                              byteCount.l[0]        [0 / 100];
                              byteCount.l[1]        [0 / BIT_4  - 1];
                              dropCounter           [0 / BIT_30 - 1];
                              randomOffset          [0 / BIT_31];
                              lastSampledValue.l[0] [0 / 100];
                              lastSampledValue.l[1] [0 / BIT_4  - 1];
                              samplingWindow.l[0]   [0 / 100];
                              samplingWindow.l[1]   [0 / BIT_4  - 1];
                              samplingAction        [CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E/
                                                     CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E];
                              logSamplingRange      [0 / BIT_6 - 1];
                              randomFlag            [CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E/
                                                     CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E];
                              samplingMode          [CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E /
                                                     CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E /
                                                     CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E];
                              cpuSubCode            [0 / BIT_2 - 1] }
                Expected: GT_OK.
            */

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E],
                         entryIndex[0] */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            entryIndex = 0;

            ipfixEntry.timeStamp             = 0;
            ipfixEntry.packetCount           = 0;
            ipfixEntry.byteCount.l[0]        = 0;
            ipfixEntry.byteCount.l[1]        = 0;
            ipfixEntry.dropCounter           = 0;
            ipfixEntry.randomOffset          = 0;
            ipfixEntry.lastSampledValue.l[0] = 0;
            ipfixEntry.lastSampledValue.l[1] = 0;
            ipfixEntry.samplingWindow.l[0]   = 0;
            ipfixEntry.samplingWindow.l[1]   = 0;
            ipfixEntry.samplingAction        = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
            ipfixEntry.logSamplingRange      = 0;
            ipfixEntry.randomFlag            = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
            ipfixEntry.samplingMode          = CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E;
            ipfixEntry.cpuSubCode            = 0;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                      stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChIpfixPortGroupEntryGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                               stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpfixPortGroupEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E],
                         entryIndex[max - 1] */
            if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            }

            /* get current stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* set stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage,
                                   CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

            ipfixEntry.timeStamp             = BIT_16 - 1;
            ipfixEntry.packetCount           = BIT_30 - 1;
            ipfixEntry.byteCount.l[0]        = 100;
            ipfixEntry.byteCount.l[1]        = BIT_4  - 1;
            ipfixEntry.dropCounter           = BIT_30 - 1;
            ipfixEntry.randomOffset          = 0xFFFFFFFF;
            ipfixEntry.lastSampledValue.l[0] = 0xAAAAAAAA;
            ipfixEntry.lastSampledValue.l[1] = BIT_4  - 1;
            ipfixEntry.samplingWindow.l[0]   = 100;
            ipfixEntry.samplingWindow.l[1]   = BIT_4  - 1;
            ipfixEntry.samplingAction        = CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;
            ipfixEntry.logSamplingRange      = BIT_6 - 1;
            ipfixEntry.randomFlag            = CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E;
            ipfixEntry.samplingMode          = CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E;
            ipfixEntry.cpuSubCode            = BIT_2 - 1;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                                   stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpfixPortGroupEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);


            /* retrieve back stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                         entryIndex[max - 1] */
            if(CHECK_EPLR_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            }

            /* get current stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeGet(dev, stage, &saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            /* set stage 1 policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage,
                                   CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

            ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
            ipfixEntry.samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E;

            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                    stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChIpfixPortGroupEntryGet(dev, portGroupsBmp,
                               stage, entryIndex, reset, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpfixPortGroupEntryGet: %d ", dev);

            /* Verifying values */
            isEqual = prvUtfCompareIpfixEntries(&ipfixEntry, &ipfixEntryGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                      "get another ipfixEntry than was set: %d", dev);

            /*
                1.1.3. Call api with wrong stage [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPortGroupEntrySet
                                (dev, portGroupsBmp, stage,
                                entryIndex, &ipfixEntry),
                                stage);

            /*
                1.1.4. Call with wrong entryIndex [max].
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;

            /*
                1.1.5. Call with wrong ipfixEntry.timeStamp = BIT_16.
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.timeStamp = BIT_16;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.timeStamp = BIT_16 - 1;

            /*
                1.1.6. Call with wrong ipfixEntry.packetCount = BIT_30;
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.packetCount = BIT_30;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.packetCount = BIT_30 - 1;

            /*
                1.1.7. Call with wrong ipfixEntry.byteCount.l[1] = BIT_4;
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.byteCount.l[1] = BIT_4;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.byteCount.l[1] = BIT_4  - 1;

            /*
                1.1.8. Call with wrong ipfixEntry.dropCounter = BIT_30;
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.dropCounter = BIT_30;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.dropCounter = BIT_30 - 1;

            /*
                1.1.9. Call with wrong ipfixEntry.lastSampledValue.l[1] = BIT_4;
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.lastSampledValue.l[1] = BIT_4;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.lastSampledValue.l[1] = BIT_4  - 1;

            /*
                1.1.10. Call with wrong ipfixEntry.samplingWindow.l[1] = BIT_4;
                Expected: NOT GT_OK.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.samplingWindow.l[1] = BIT_4;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.samplingWindow.l[1] = BIT_4  - 1;

            /*
                1.1.11. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
                Expected: NOT GT_OK with random flag.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.randomFlag = CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E;
            ipfixEntry.logSamplingRange = BIT_6;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.logSamplingRange = BIT_6 - 1;

            /*
                1.1.12. Call with wrong ipfixEntry.logSamplingRange = BIT_6;
                Expected: GT_OK with deterministic flag.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.randomFlag = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
            ipfixEntry.logSamplingRange = BIT_6;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.logSamplingRange = BIT_6 - 1;

            /*
                1.1.13. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
                Expected: NOT GT_OK with alarm sampling action.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
            ipfixEntry.cpuSubCode = BIT_2;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.cpuSubCode = BIT_2 - 1;

            /*
                1.1.14. Call with wrong ipfixEntry.cpuSubCode = BIT_2;
                Expected: NOT GT_OK too.
            */
            st = cpssDxChPolicerCountingWriteBackCacheFlush(dev,stage);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);

            ipfixEntry.samplingAction = CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;
            ipfixEntry.cpuSubCode = BIT_2;

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                                                stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            ipfixEntry.cpuSubCode = BIT_2 - 1;

            /*
                1.1.15. Call api with wrong ipfixEntryPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp, stage, entryIndex, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ipfixEntryPtr = NULL", dev);

            /* retrieve back egress stage policer counting mode */
            st = cpssDxChPolicerCountingModeSet(dev, stage, saveCountMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stage);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */

        stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        entryIndex = 0;
        ipfixEntry.timeStamp             = 0;
        ipfixEntry.packetCount           = 0;
        ipfixEntry.byteCount.l[0]        = 0;
        ipfixEntry.byteCount.l[1]        = 0;
        ipfixEntry.dropCounter           = 0;
        ipfixEntry.randomOffset          = 0;
        ipfixEntry.lastSampledValue.l[0] = 0;
        ipfixEntry.lastSampledValue.l[1] = 0;
        ipfixEntry.samplingWindow.l[0]   = 0;
        ipfixEntry.samplingWindow.l[1]   = 0;
        ipfixEntry.samplingAction        = CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E;
        ipfixEntry.logSamplingRange      = 0;
        ipfixEntry.randomFlag            = CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
        ipfixEntry.samplingMode          = CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E;
        ipfixEntry.cpuSubCode            = 0;

        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                            stage, entryIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                            stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    entryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                            stage, entryIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPortGroupEntrySet(dev, portGroupsBmp,
                            stage, entryIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPortGroupWraparoundStatusGet
(
    IN  GT_U8                             dev,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage,
    IN  GT_U32                            startIndex,
    IN  GT_U32                            endIndex,
    IN  GT_BOOL                           reset,
    OUT GT_U32                            *bmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixPortGroupWraparoundStatusGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChXcat and above)
    1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                           CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                           CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                   startIndex[0 / 0],
                   endIndex[0 / max - 1],
                   reset[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call api with wrong stage [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.3. Call api with wrong startIndex [max].
    Expected: NOT GT_OK.
    1.1.4. Call api with wrong endIndex [max].
    Expected: NOT GT_OK.
    1.1.5. Call api with endindex less than start startIndex [max], endIndex[0].
    Expected: NOT GT_OK.
    1.1.6. Call api with wrong bmpPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8                             dev;
    GT_PORT_GROUPS_BMP                portGroupsBmp = 1;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT  stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    GT_U32                            startIndex = 0;
    GT_U32                            endIndex = 0;
    GT_BOOL                           reset = GT_FALSE;
    GT_U32                           *bmpPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    bmpPtr = cpssOsMalloc(IPFIX_BMP_SIZE_CNS);
    if (bmpPtr == NULL)
    {
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)bmpPtr,
                                        "cpssOsMalloc: Memory allocation error.");
        return;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E /
                                       CPSS_DXCH_POLICER_STAGE_INGRESS_1_E /
                                       CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                               startIndex[0 / 0],
                               endIndex[0 / max - 1],
                               reset[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E],
                         startIndex[0],
                         endIndex[0],
                         reset[GT_TRUE] */
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            startIndex = 0;
            endIndex = 0;
            reset = GT_TRUE;

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E],
                         startIndex[0],
                         endIndex[max - 1],
                         reset[GT_FALSE] */
            if(CHECK_IPLR_SECOND_STAGE_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            }

            startIndex = 0;
            endIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (endIndex >= _2K)
                {
                    endIndex = _2K -1;
                }
            }

            reset = GT_FALSE;

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with stage[CPSS_DXCH_POLICER_STAGE_EGRESS_E],
                         startIndex[0],
                         endIndex[max - 1],
                         reset[GT_FALSE] */
            if(CHECK_EPLR_EXISTS_MAC(dev))
            {
                /*call with stage [CPSS_DXCH_POLICER_STAGE_EGRESS_E]*/
                stage  = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            }

            endIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (endIndex >= _2K)
                {
                    endIndex = _2K -1;
                }
            }

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong stage [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPortGroupWraparoundStatusGet
                                (dev, portGroupsBmp, stage,
                                startIndex, endIndex, reset, bmpPtr),
                                stage);

            /*
                1.1.3. Call api with wrong startIndex [max].
                Expected: NOT GT_OK.
            */
            startIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            startIndex = 0;

            /*
                1.1.4. Call api with wrong endIndex [max].
                Expected: NOT GT_OK.
            */
            endIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage];

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            endIndex = 0;

            /*
                1.1.5. Call api with endindex less than start startIndex [max], endIndex[0].
                Expected: NOT GT_OK.
            */
            startIndex = PRV_CPSS_DXCH_PP_MAC(dev)->policer.countingMemSize[stage] - 1;
            if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
            {
                if (startIndex >= _2K)
                {
                    startIndex = _2K -1;
                }
            }
            endIndex = 0;

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            startIndex = 0;

            /*
                1.1.6. Call api with wrong bmpPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bmpPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                      stage, startIndex, endIndex, reset, bmpPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                                  stage, startIndex, endIndex, reset, bmpPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    startIndex = 0;
    endIndex = 0;
    reset = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                              stage, startIndex, endIndex, reset, bmpPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPortGroupWraparoundStatusGet(dev, portGroupsBmp,
                              stage, startIndex, endIndex, reset, bmpPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    cpssOsFree(bmpPtr);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimestampToCpuEnableGet
(
    IN  GT_U8    dev,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimestampToCpuEnableGet)
{
/*
    ITERATE_DEVICES(Lion and above)
    1.1. Call with not null enablePtr.
    Expected: GT_OK.
    1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8    dev;
    GT_BOOL  enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpfixTimestampToCpuEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixTimestampToCpuEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimestampToCpuEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimestampToCpuEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixTimestampToCpuEnableSet
(
    IN  GT_U8   dev,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixTimestampToCpuEnableSet)
{
/*
    ITERATE_DEVICES(Lion and above)
    1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixTimestampToCpuEnableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_BOOL enable = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_TRUE / GT_FALSE],
            Expected: GT_OK.
        */

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChIpfixTimestampToCpuEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChIpfixTimestampToCpuEnableGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChIpfixTimestampToCpuEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChIpfixTimestampToCpuEnableGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChIpfixTimestampToCpuEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChIpfixTimestampToCpuEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChIpfixTimestampToCpuEnableGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);
    }

    /* restore correct values */
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixTimestampToCpuEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixTimestampToCpuEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixBaseFlowIdSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32   baseFlowId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixBaseFlowIdSet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with baseFlowId [0 / 64K-1]
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixBaseFlowIdSet with 0 baseFlowId.
    Expected: GT_OK and the same baseFlowId as was set.
    1.3. Call with out of range baseFlowId [64K]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32   baseFlowId    = 0;
    GT_U32   baseFlowIdGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with baseFlowId [0 / 64K].
            Expected: GT_OK.
        */
        /* iterate with baseFlowId = 0 */
        baseFlowId = 0;

        st = cpssDxChIpfixBaseFlowIdSet(dev, plrStage, baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseFlowId);

        /*
            1.2. Call cpssDxChIpfixBaseFlowIdGet with not NULL baseFlowIdPtr.
            Expected: GT_OK and the same baseFlowId as was set.
        */
        st = cpssDxChIpfixBaseFlowIdGet(dev, plrStage, &baseFlowIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixBaseFlowIdGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseFlowId, baseFlowIdGet,
                   "got another baseFlowId then was set: %d", dev);

        /* iterate with baseFlowId = 64K-1 */
        baseFlowId = 65535;

        st = cpssDxChIpfixBaseFlowIdSet(dev, plrStage, baseFlowId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseFlowId);

        /*
            1.2. Call cpssDxChIpfixBaseFlowIdGet with not NULL baseFlowIdPtr.
            Expected: GT_OK and the same baseFlowId as was set.
        */
        st = cpssDxChIpfixBaseFlowIdGet(dev, plrStage, &baseFlowIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixBaseFlowIdGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseFlowId, baseFlowIdGet,
                   "got another baseFlowId then was set: %d", dev);

        /*
           1.3. Call with wrong enum values enable.
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixBaseFlowIdSet
            (dev, plrStage, baseFlowId),
            plrStage);

        /*
           1.4. Call with out of range baseFlowId [64K]
           Expected: NOT GT_OK.
        */
        baseFlowId = BIT_16;
        st = cpssDxChIpfixBaseFlowIdSet(dev, plrStage, baseFlowId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, baseFlowId);
    }

    baseFlowId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixBaseFlowIdSet(dev, plrStage, baseFlowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixBaseFlowIdSet(dev, plrStage, baseFlowId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixBaseFlowIdGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          *baseFlowIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixBaseFlowIdGet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with not NULL baseFlowIdPtr.
    Expected: GT_OK.
    1.2. Call function with baseFlowIdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32  baseFlowIdPtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL mruSizePtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpfixBaseFlowIdGet(dev, plrStage, &baseFlowIdPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with wrong enum values enable.
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixBaseFlowIdGet
            (dev, plrStage, &baseFlowIdPtr),
            plrStage);

        /*
            1.3. Call function with baseFlowIdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixBaseFlowIdGet(dev, plrStage, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, baseFlowIdPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixBaseFlowIdGet(dev, plrStage, &baseFlowIdPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixBaseFlowIdGet(dev, plrStage, &baseFlowIdPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixFirstPacketConfigSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC  *firstPacketCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFirstPacketConfigSet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with packetCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E]
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixFirsPacketConfigSet with CPSS_PACKET_CMD_MIRROR_TO_CPU_E packetCmd.
    Expected: GT_OK and the same packetCmd as was set.
    1.3. Call cpssDxChIpfixFirsPacketConfigSet with valid cpuCode.
    Expected: GT_OK and the same cpuCode as was set.
    1.4. Call with out of range packetCmd
    Expected: NOT GT_OK.
    1.5. Call with out of range cpuCode
    Expected: NOT GT_OK.
    1.6. Call with out of range plrStage
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC firstPacketCfg;
    CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC firstPacketCfgGet;

    cpssOsMemSet(&firstPacketCfg, 0, sizeof(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC));
    cpssOsMemSet(&firstPacketCfgGet, 0, sizeof(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        firstPacketCfg.packetCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        firstPacketCfg.cpuCode   = CPSS_NET_FIRST_USER_DEFINED_E;
        /*
            1.1. Call function with packetCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E].
            Expected: GT_OK.
        */
        /* iterate with packetCmd  = CPSS_PACKET_CMD_MIRROR_TO_CPU_E and cpuCode = CPSS_NET_PCL_MIRRORED_TO_ANALYZER_E */
        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChIpfixFirstPacketConfigSet with not NULL firstPacketCfgGet.
            Expected: GT_OK and the same packetCmd and cpuCode as was set.
        */
        st = cpssDxChIpfixFirstPacketConfigGet(dev, plrStage, &firstPacketCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixFirstPacketConfigGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(firstPacketCfg.packetCmd, firstPacketCfgGet.packetCmd,
                   "got another packetCmd then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(firstPacketCfg.cpuCode, firstPacketCfgGet.cpuCode,
                   "got another cpuCode then was set: %d", dev);

        /* iterate with  packetCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E */
        firstPacketCfg.packetCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call cpssDxChIpfixFirstPacketConfigGet with not NULL firstPacketCfgGet.
            Expected: GT_OK and the same packetCmd and cpuode as was set.
        */
        st = cpssDxChIpfixFirstPacketConfigGet(dev, plrStage, &firstPacketCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixFirstPacketConfigGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(firstPacketCfg.packetCmd, firstPacketCfgGet.packetCmd,
                   "got another packetCmd then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(firstPacketCfg.cpuCode, firstPacketCfgGet.cpuCode,
                   "got another cpuCode then was set: %d", dev);

        /*
           1.4. Call with out of range packetCmd
           Expected: GT_BAD_PARAM.
        */
        firstPacketCfg.packetCmd = CPSS_PACKET_CMD_BRIDGE_E;

        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.5. Call with out of range cpuCode
           Expected: GT_BAD_PARAM.
        */
        firstPacketCfg.packetCmd = CPSS_PACKET_CMD_FORWARD_E;
        firstPacketCfg.cpuCode = CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E + 1;

        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.6. Call with out of range packetCmd
           Expected: GT_BAD_PARAM.
        */
        firstPacketCfg.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.7. Call function with wrong enum values packetCmd.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixFirstPacketConfigSet
            (dev, plrStage, &firstPacketCfg),
            firstPacketCfg.packetCmd);

        /*
            1.8. Call function with wrong enum values cpuCode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixFirstPacketConfigSet
            (dev, plrStage, &firstPacketCfg),
            firstPacketCfg.cpuCode);

        /*
            1.9. Call function with wrong enum values plrStage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixFirstPacketConfigSet
            (dev, plrStage, &firstPacketCfg),
            plrStage);

        /*
            1.10. Call function with NULL value for firstPacketCfgPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    firstPacketCfg.packetCmd = CPSS_PACKET_CMD_FORWARD_E;
    firstPacketCfg.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixFirstPacketConfigSet(dev, plrStage, &firstPacketCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixFirstPacketConfigGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC *firstPacketCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFirstPacketConfigGet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with not NULL firstPacketCfgPtr.
    Expected: GT_OK.
    1.2. Call function with firstPacketCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC firstPacketCfgGet;

    cpssOsMemSet(&firstPacketCfgGet, 0, sizeof(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL firstPacketCfgGetPtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpfixFirstPacketConfigGet(dev, plrStage, &firstPacketCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with firstPacketCfgGetPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixFirstPacketConfigGet(dev, plrStage, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, firstPacketCfgGetPtr = NULL", dev);

        /*
            1.3. Call function with wrong enum values plrStage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixFirstPacketConfigGet
            (dev, plrStage, &firstPacketCfgGet),
            plrStage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixFirstPacketConfigGet(dev, plrStage, &firstPacketCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixFirstPacketConfigGet(dev, plrStage, &firstPacketCfgGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPhaMetadataCounterMaskSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U64   phaCounterMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixPhaMetadataCounterMaskSet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with phaCounterMask [0x0 / 0xFFFFFFFFFFFF]
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixPhaMetadataCounterMaskSet with 0x0 phaCounterMask.
    Expected: GT_OK and the same phaCounterMask. as was set.
    1.3. Call with out of range phaCounterMask. [0x1FFFFFFFFFFFF]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U64   phaCounterMask;
    GT_U64   phaCounterMaskGet;
    phaCounterMask.l[0] = 0x0;
    phaCounterMask.l[1] = 0x0;
    phaCounterMaskGet.l[0] = 0x0;
    phaCounterMaskGet.l[1] = 0x0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with phaCounterMask [0x0 / 0xFFFFFFFFFFFF].
            Expected: GT_OK.
        */
        /* iterate with phaCounterMask = 0x0 */
        phaCounterMask.l[0] = 0x0;
        phaCounterMask.l[1] = 0x0;

        st = cpssDxChIpfixPhaMetadataCounterMaskSet(dev, plrStage, phaCounterMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChIpfixPhaMetadataCounterMaskGet with not NULL phaCounterMaskPtr.
            Expected: GT_OK and the same phaCounterMask as was set.
        */
        st = cpssDxChIpfixPhaMetadataCounterMaskGet(dev, plrStage, &phaCounterMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixPhaMetadataCounterMaskGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(phaCounterMask.l[0], phaCounterMaskGet.l[0],
                   "got another phaCounterMask then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(phaCounterMask.l[1], phaCounterMaskGet.l[1],
                   "got another phaCounterMask then was set: %d", dev);

        /* iterate with phaCounterMask = 0xFFFFFFFFFFFF */
        phaCounterMask.l[0] = 0xFFFFFFFF;
        phaCounterMask.l[1] = 0xFFFF;

        st = cpssDxChIpfixPhaMetadataCounterMaskSet(dev, plrStage, phaCounterMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChIpfixPhaMetadataCounterMaskGet with not NULL phaCounterMaskPtr.
            Expected: GT_OK and the same phaCounterMask as was set.
        */
        st = cpssDxChIpfixPhaMetadataCounterMaskGet(dev, plrStage, &phaCounterMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixPhaMetadataCounterMaskGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(phaCounterMask.l[0], phaCounterMaskGet.l[0],
                   "got another phaCounterMask then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(phaCounterMask.l[1], phaCounterMaskGet.l[1],
                   "got another phaCounterMask then was set: %d", dev);

        /*
           1.3. Call with out of range phaCounterMask [0x1FFFFFFFFFFFF]
           Expected: NOT GT_OK.
        */
        phaCounterMask.l[0] = 0xFFFFFFFF;
        phaCounterMask.l[1] = 0x1FFFF;
        st = cpssDxChIpfixPhaMetadataCounterMaskSet(dev, plrStage, phaCounterMask);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call function with wrong enum values plrStage.
            Expected: GT_BAD_PARAM.
        */
        phaCounterMask.l[0] = 0xFFFFFFFF;
        phaCounterMask.l[1] = 0xFFFF;
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPhaMetadataCounterMaskSet
            (dev, plrStage, phaCounterMask),
            plrStage);
    }

    phaCounterMask.l[0] = 0x0;
    phaCounterMask.l[1] = 0x0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPhaMetadataCounterMaskSet(dev, plrStage, phaCounterMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPhaMetadataCounterMaskSet(dev, plrStage, phaCounterMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixPhaMetadataCounterMaskGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U64                          *phaCounterMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixPhaMetadataCounterMaskGet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with not NULL phaCounterMaskPtr.
    Expected: GT_OK.
    1.2. Call function with phaCounterMaskPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U64  phaCounterMaskPtr;
    phaCounterMaskPtr.l[0] = 0;
    phaCounterMaskPtr.l[1] = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL phaCounterMaskPtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpfixPhaMetadataCounterMaskGet(dev, plrStage, &phaCounterMaskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with phaCounterMaskPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixPhaMetadataCounterMaskGet(dev, plrStage, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, phaCounterMaskPtr = NULL", dev);

        /*
            1.3. Call function with wrong enum values plrStage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixPhaMetadataCounterMaskGet
            (dev, plrStage, &phaCounterMaskPtr),
            plrStage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixPhaMetadataCounterMaskGet(dev, plrStage, &phaCounterMaskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixPhaMetadataCounterMaskGet(dev, plrStage, &phaCounterMaskPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixFirstPacketsMirrorEnableSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           ipfixEntryNum,
    IN GT_BOOL                          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFirstPacketsMirrorEnableSet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with ipfixEntryNum [0 / 64K-1]
    Expected: GT_OK.
    1.2. Call cpssDxChIpfixFirstPacketsMirrorEnableSet with 0 ipfixEntryNum.
    Expected: GT_OK and the same ipfixEntryNum as was set.
    1.3. Call with out of range ipfixEntryNum [64K]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32   ipfixEntryNum    = 0;
    GT_BOOL  enable = GT_TRUE;
    GT_BOOL  enableGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ipfixEntryNum [0 / 64K].
            Expected: GT_OK.
        */
        /* iterate with ipfixEntryNum = 0 */
        ipfixEntryNum = 0;

        st = cpssDxChIpfixFirstPacketsMirrorEnableSet(dev, plrStage, ipfixEntryNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ipfixEntryNum);

        /*
            1.2. Call cpssDxChIpfixFirstPacketsMirrorEnableGet with not NULL enableGetPtr.
            Expected: GT_OK and the same baseFlowId as was set.
        */
        st = cpssDxChIpfixFirstPacketsMirrorEnableGet(dev, plrStage, ipfixEntryNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixFirstPacketsMirrorEnableGet: %d", dev);

        /* iterate with ipfixEntryNum = max-1 */
        ipfixEntryNum = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.plrInfo.countingEntriesNum - 1;

        st = cpssDxChIpfixFirstPacketsMirrorEnableSet(dev, plrStage, ipfixEntryNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ipfixEntryNum);

        /*
            1.2. Call cpssDxChIpfixFirstPacketsMirrorEnableGet with not NULL enablePtr.
            Expected: GT_OK and the same ipfixEntryNum as was set.
        */
        st = cpssDxChIpfixFirstPacketsMirrorEnableGet(dev, plrStage, ipfixEntryNum, &enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpfixFirstPacketsMirrorEnableGet: %d", dev);

        /*
            1.3. Call function with wrong enum values plrStage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixFirstPacketsMirrorEnableSet
            (dev, plrStage, ipfixEntryNum, enable),
            plrStage);

        /*
           1.4. Call with out of range ipfixEntryNum [max]
           Expected: NOT GT_OK.
        */
        ipfixEntryNum = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.plrInfo.countingEntriesNum;
        st = cpssDxChIpfixFirstPacketsMirrorEnableSet(dev, plrStage, ipfixEntryNum, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ipfixEntryNum);
    }

    ipfixEntryNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixFirstPacketsMirrorEnableSet(dev, plrStage, ipfixEntryNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixFirstPacketsMirrorEnableSet(dev, plrStage, ipfixEntryNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixFirstPacketsMirrorEnableGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                          ipfixEntryNum,
    OUT GT_BOOL                         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFirstPacketsMirrorEnableGet)
{
/*
    ITERATE_DEVICES (AC5P)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32   ipfixEntryNum = 0;
    GT_BOOL  enablePtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpfixFirstPacketsMirrorEnableGet(dev, plrStage, ipfixEntryNum, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpfixFirstPacketsMirrorEnableGet(dev, plrStage, ipfixEntryNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);

        /*
            1.3. Call function with wrong enum values plrStage.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpfixFirstPacketsMirrorEnableGet
            (dev, plrStage, ipfixEntryNum, &enablePtr),
            plrStage);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT2_E | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpfixFirstPacketsMirrorEnableGet(dev, plrStage, ipfixEntryNum, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixFirstPacketsMirrorEnableGet(dev, plrStage, ipfixEntryNum, &enablePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChIpFix suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChIpFix)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixAgingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixAgingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixAgingStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixAlarmEventsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixCpuCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixDropCountModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixDropCountModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimestampUploadSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimestampUploadGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimestampUploadStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimestampUploadTrigger)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixWraparoundConfSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixWraparoundConfGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixWraparoundStatusGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPortGroupAgingStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPortGroupAlarmEventsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPortGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPortGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPortGroupWraparoundStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimestampToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixTimestampToCpuEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixBaseFlowIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixBaseFlowIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFirstPacketConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFirstPacketConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPhaMetadataCounterMaskSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixPhaMetadataCounterMaskGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFirstPacketsMirrorEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFirstPacketsMirrorEnableGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChIpFix)

