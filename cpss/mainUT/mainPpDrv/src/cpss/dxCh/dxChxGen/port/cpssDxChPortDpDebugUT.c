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
* @file cpssDxChPortDpDebugUT.c
*
* @brief Unit tests for cpssDxChPortCpDebug.
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDpDebug.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*
GT_STATUS cpssDxChPortDpDebugCountersResetAndEnableSet
(
    IN GT_U8                         devNum,
    IN GT_PHYSICAL_PORT_NUM          unitRepresentingPort,
    IN GT_BOOL                       enable
);
*/

UTF_TEST_CASE_MAC(cpssDxChPortDpDebugCountersResetAndEnableSet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* correct parameters */
    enable    = GT_FALSE;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugCountersResetAndEnableSet(
                dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugCountersResetAndEnableSet(
                dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugCountersResetAndEnableSet(
            dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugCountersResetAndEnableSet(
            dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugCountersResetAndEnableSet(
            dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugCountersResetAndEnableSet(
        dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
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
*/
UTF_TEST_CASE_MAC(cpssDxChPortDpDebugRxToCpCountingConfigSet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      criteriaParamBmp;
    GT_BOOL     cutThroughPacket;
    GT_U32      rxToCpPriority;
    GT_BOOL     latencySensitive;
    GT_BOOL     headOrTailDispatch;
    GT_U32      criteriaParamBmpGet;
    GT_BOOL     cutThroughPacketGet;
    GT_U32      rxToCpPriorityGet;
    GT_BOOL     latencySensitiveGet;
    GT_BOOL     headOrTailDispatchGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* correct parameters */
    criteriaParamBmp    = 0x1F;
    cutThroughPacket    = GT_TRUE;
    rxToCpPriority      = 1;
    latencySensitive    = GT_FALSE;
    headOrTailDispatch  = GT_TRUE;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugRxToCpCountingConfigSet(
                dev, port, criteriaParamBmp, cutThroughPacket,
                rxToCpPriority, latencySensitive, headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* get and compare set-values and get-values */
            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmpGet, &cutThroughPacketGet,
                &rxToCpPriorityGet, &latencySensitiveGet, &headOrTailDispatchGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL0_PARAM_MAC(criteriaParamBmp, criteriaParamBmpGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(cutThroughPacket, cutThroughPacketGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(rxToCpPriority, rxToCpPriorityGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(latencySensitive, latencySensitiveGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(headOrTailDispatch, headOrTailDispatchGet);

            /* bad parameters not supported - any values converted to correct */
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugRxToCpCountingConfigSet(
                dev, port, criteriaParamBmp, cutThroughPacket,
                rxToCpPriority, latencySensitive, headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugRxToCpCountingConfigSet(
            dev, port, criteriaParamBmp, cutThroughPacket,
            rxToCpPriority, latencySensitive, headOrTailDispatch);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugRxToCpCountingConfigSet(
            dev, port, criteriaParamBmp, cutThroughPacket,
            rxToCpPriority, latencySensitive, headOrTailDispatch);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/


    port = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugRxToCpCountingConfigSet(
            dev, port, criteriaParamBmp, cutThroughPacket,
            rxToCpPriority, latencySensitive, headOrTailDispatch);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugRxToCpCountingConfigSet(
        dev, port, criteriaParamBmp, cutThroughPacket,
        rxToCpPriority, latencySensitive, headOrTailDispatch);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
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
*/
UTF_TEST_CASE_MAC(cpssDxChPortDpDebugRxToCpCountingConfigGet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      criteriaParamBmp;
    GT_BOOL     cutThroughPacket;
    GT_U32      rxToCpPriority;
    GT_BOOL     latencySensitive;
    GT_BOOL     headOrTailDispatch;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmp, &cutThroughPacket,
                &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* bad parameters - NULL pointers */
            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, NULL, &cutThroughPacket,
                &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmp, NULL,
                &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmp, &cutThroughPacket,
                NULL, &latencySensitive, &headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmp, &cutThroughPacket,
                &rxToCpPriority, NULL, &headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmp, &cutThroughPacket,
                &rxToCpPriority, &latencySensitive, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
                dev, port, &criteriaParamBmp, &cutThroughPacket,
                &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
            dev, port, &criteriaParamBmp, &cutThroughPacket,
            &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
            dev, port, &criteriaParamBmp, &cutThroughPacket,
            &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
            dev, port, &criteriaParamBmp, &cutThroughPacket,
            &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugRxToCpCountingConfigGet(
        dev, port, &criteriaParamBmp, &cutThroughPacket,
        &rxToCpPriority, &latencySensitive, &headOrTailDispatch);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortDpDebugRxToCpCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortDpDebugRxToCpCountValueGet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      countValue;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugRxToCpCountValueGet(
                dev, port, &countValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* bad parameters - NULL pointers */
            st = cpssDxChPortDpDebugRxToCpCountValueGet(
                dev, port, NULL);
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugRxToCpCountValueGet(
                dev, port, &countValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugRxToCpCountValueGet(
            dev, port, &countValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugRxToCpCountValueGet(
            dev, port, &countValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugRxToCpCountValueGet(
            dev, port, &countValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugRxToCpCountValueGet(
        dev, port, &countValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
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
*/
UTF_TEST_CASE_MAC(cpssDxChPortDpDebugCpToRxCountingConfigSet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      criteriaParamBmp;
    GT_BOOL     multicastPacket;
    GT_BOOL     cutThroughPacket;
    GT_BOOL     cutThroughTerminated;
    GT_BOOL     trunkatedHeader;
    GT_BOOL     dummyDescriptor;
    GT_BOOL     highPriorityPacket;
    GT_BOOL     contextDone;
    GT_BOOL     descriptorValid;
    GT_U32      criteriaParamBmpGet;
    GT_BOOL     multicastPacketGet;
    GT_BOOL     cutThroughPacketGet;
    GT_BOOL     cutThroughTerminatedGet;
    GT_BOOL     trunkatedHeaderGet;
    GT_BOOL     dummyDescriptorGet;
    GT_BOOL     highPriorityPacketGet;
    GT_BOOL     contextDoneGet;
    GT_BOOL     descriptorValidGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* correct parameters */
    criteriaParamBmp     = 0x11F; /* will be overridden */
    multicastPacket      = GT_FALSE;
    cutThroughPacket     = GT_TRUE;
    cutThroughTerminated = GT_FALSE;
    trunkatedHeader      = GT_TRUE;
    dummyDescriptor      = GT_FALSE;
    highPriorityPacket   = GT_TRUE;
    contextDone          = GT_FALSE;
    descriptorValid      = GT_TRUE;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
        {
            criteriaParamBmp     = 0x1FF;
        }
        else
        {
            /* bypassed highPriorityPacket, contextDone, descriptorValid */
            criteriaParamBmp     = 0x11F;
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugCpToRxCountingConfigSet(
                dev, port, criteriaParamBmp, multicastPacket, cutThroughPacket,
                cutThroughTerminated, trunkatedHeader, dummyDescriptor,
                highPriorityPacket, contextDone, descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* get and compare set-values and get-values */
            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmpGet, &multicastPacketGet, &cutThroughPacketGet,
                &cutThroughTerminatedGet, &trunkatedHeaderGet, &dummyDescriptorGet,
                &highPriorityPacketGet, &contextDoneGet, &descriptorValidGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL0_PARAM_MAC(criteriaParamBmp, criteriaParamBmpGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(multicastPacket, multicastPacketGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(cutThroughPacket, cutThroughPacketGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(cutThroughTerminated, cutThroughTerminatedGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(trunkatedHeader, trunkatedHeaderGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(dummyDescriptor, dummyDescriptorGet);
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(highPriorityPacket, highPriorityPacketGet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(contextDone, contextDoneGet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(descriptorValid, descriptorValidGet);
            }
            /* bad parameters not supported - any values converted to correct */
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugCpToRxCountingConfigSet(
                dev, port, criteriaParamBmp, multicastPacket, cutThroughPacket,
                cutThroughTerminated, trunkatedHeader, dummyDescriptor,
                highPriorityPacket, contextDone, descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugCpToRxCountingConfigSet(
            dev, port, criteriaParamBmp, multicastPacket, cutThroughPacket,
            cutThroughTerminated, trunkatedHeader, dummyDescriptor,
            highPriorityPacket, contextDone, descriptorValid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugCpToRxCountingConfigSet(
            dev, port, criteriaParamBmp, multicastPacket, cutThroughPacket,
            cutThroughTerminated, trunkatedHeader, dummyDescriptor,
            highPriorityPacket, contextDone, descriptorValid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/


    port = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugCpToRxCountingConfigSet(
            dev, port, criteriaParamBmp, multicastPacket, cutThroughPacket,
            cutThroughTerminated, trunkatedHeader, dummyDescriptor,
            highPriorityPacket, contextDone, descriptorValid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugCpToRxCountingConfigSet(
        dev, port, criteriaParamBmp, multicastPacket, cutThroughPacket,
        cutThroughTerminated, trunkatedHeader, dummyDescriptor,
        highPriorityPacket, contextDone, descriptorValid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortDpDebugCpToRxCountingConfigGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             unitRepresentingPort,
    OUT GT_U32                           *criteriaParamBmpPtr,
    OUT GT_BOOL                          *multicastPacketPtr,
    OUT GT_BOOL                          *cutThroughPacketPtr,
    OUT GT_BOOL                          *cutThroughTerminatedPtr,
    OUT GT_BOOL                          *trunkatedHeaderPtr,
    OUT GT_BOOL                          *dummyDescriptorPtr
    OUT GT_BOOL                          *highPriorityPacketPtr,
    OUT GT_BOOL                          *contextDonePtr,
    OUT GT_BOOL                          *descriptorValidPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortDpDebugCpToRxCountingConfigGet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      criteriaParamBmp;
    GT_BOOL     multicastPacket;
    GT_BOOL     cutThroughPacket;
    GT_BOOL     cutThroughTerminated;
    GT_BOOL     trunkatedHeader;
    GT_BOOL     dummyDescriptor;
    GT_BOOL     highPriorityPacket;
    GT_BOOL     contextDone;
    GT_BOOL     descriptorValid;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* bad parameters - NULL-pointers */
            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, NULL, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, NULL, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, NULL,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                NULL, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, NULL, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, NULL,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                NULL, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, NULL, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
                dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
                &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
                &highPriorityPacket, &contextDone, &descriptorValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
            dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
            &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
            &highPriorityPacket, &contextDone, &descriptorValid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
            dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
            &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
            &highPriorityPacket, &contextDone, &descriptorValid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/


    port = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
            dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
            &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
            &highPriorityPacket, &contextDone, &descriptorValid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugCpToRxCountingConfigGet(
        dev, port, &criteriaParamBmp, &multicastPacket, &cutThroughPacket,
        &cutThroughTerminated, &trunkatedHeader, &dummyDescriptor,
        &highPriorityPacket, &contextDone, &descriptorValid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortDpDebugCpToRxCountValueGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           unitRepresentingPort,
    OUT GT_U32                         *countValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortDpDebugCpToRxCountValueGet)
{
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      countValue;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortDpDebugCpToRxCountValueGet(
                dev, port, &countValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* bad parameters - NULL pointers */
            st = cpssDxChPortDpDebugCpToRxCountValueGet(
                dev, port, NULL);
        }

        /*
           For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortDpDebugCpToRxCountValueGet(
                dev, port, &countValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortDpDebugCpToRxCountValueGet(
            dev, port, &countValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortDpDebugCpToRxCountValueGet(
            dev, port, &countValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortDpDebugCpToRxCountValueGet(
            dev, port, &countValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortDpDebugCpToRxCountValueGet(
        dev, port, &countValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortDpDebug)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugCountersResetAndEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugRxToCpCountingConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugRxToCpCountingConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugRxToCpCountValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugCpToRxCountingConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugCpToRxCountingConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDpDebugCpToRxCountValueGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortDpDebug)


