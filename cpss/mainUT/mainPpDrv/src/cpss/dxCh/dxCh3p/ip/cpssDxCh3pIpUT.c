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
* @file cpssDxCh3pIpUT.c
*
* @brief Unit tests for cpssDxCh3pIp, that provides
* The CPSS DXCH3P Ip HW structures APIs
*
*
* @version   2.
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxCh3p/ip/cpssDxCh3pIp.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
/* Invalid enum */
#define IP_INVALID_ENUM_CNS         0x5AAAAAA5


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pIpMLLPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_IP_PAIR_READ_WRITE_FORM_ENT        mllPairWriteForm,
    IN CPSS_DXCH3P_IP_MLL_PAIR_STC                  *mllPairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pIpMLLPairWrite)
{
/*
    ITERATE_DEVICES (DxCh3p)
    {CPSS_DXCH_IP_PAIR_READ_WRITE_FIRST_MLL_ONLY_E}
    1.1. Call with mllPairEntryIndex [0],
                   mllPairWriteForm [CPSS_DXCH_IP_PAIR_READ_WRITE_FIRST_MLL_ONLY_E],
                   mllPairEntryPtr { firstMllNode{mllNode{
                                                         mllRPFFailCommand[CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                                         isTunnelStart[GT_TRUE],
                                                         nextHopVlanId[100],
                                                         nextHopTunnelPointer[0],
                                                         ttlHopLimitThreshold[0],
                                                         excludeSrcVlan[GT_FALSE],
                                                         last[GT_TRUE] },
                                       broadcastEnable[GT_FALSE],
                                       unregMulticastEnable[GT_FALSE],
                                       unknownUnicastEnable [GT_FALSE],
                                       isTagged[GT_FALSE]},
    Expected: GT_OK.
    1.2. Call cpssDxCh3pIpMLLPairRead with not NULL mllPairEntryPtr
                                           and other params from 1.1.
                                           ( and the same for 1.7.)
    Expected: GT_OK.
    1.3. Call with out of range mllPairEntryIndex [1024]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range mllPairWriteForm [0x5AAAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range mllPairEntryPtr->firstMllNode.mllNode.mllRPFFailCommand [0x5AAAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with mllPairEntryPtr->firstMllNode.mllNode.mllRPFFailCommand [CPSS_PACKET_CMD_FORWARD_E] (not supported)
                   and other params from 1.1.
    Expected: NOT GT_OK.
    {CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E }
    1.7. Call with mllPairEntryIndex [1023],
                   mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E],
                   mllPairEntryPtr { secondMllNode{ mllNode{
                                                            mllRPFFailCommand[CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                                            isTunnelStart[GT_FALSE],
                                                            nextHopInterface {type [CPSS_INTERFACE_TRUNK_E],
                                                                              trunkId [2] }
                                                            nextHopVlanId[100],
                                                            ttlHopLimitThreshold[0],
                                                            excludeSrcVlan[GT_FALSE],
                                                            last[GT_TRUE] },
                                                    broadcastEnable[GT_TRUE],
                                                    unregMulticastEnable[GT_TRUE],
                                                    unknownUnicastEnable [GT_TRUE],
                                                    isTagged[GT_TRUE]},
                                     nextPointer [0] }
    Expected: GT_OK.
    1.8. Call with out of range mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.type [0x5AAAAAAA5]
                   and other params from 1.7.
    Expected: GT_BAD_PARAM.
    1.9. Call with mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.type [CPSS_INTERFACE_VID_E]  (not supported)
                    mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.vlanId [100]
                    and other params from 1.7.
    Expected: NOT GT_OK.
    1.10. Call with mllPairEntryPtr->secondMllNode.mllNode.nextHopVlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (out of range)
                    and other params from 1.7.
    Expected: NOT GT_OK.
    1.11. Call with out of range mllPairEntryPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     isEqual = GT_FALSE;

    GT_U32                                      mllPairEntryIndex;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairWriteForm;
    CPSS_DXCH3P_IP_MLL_PAIR_STC                 mllPairEntry;
    CPSS_DXCH3P_IP_MLL_PAIR_STC                 mllPairEntryGet;


    mllPairEntryIndex = 0;
    mllPairWriteForm  = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

    cpssOsBzero((GT_VOID*) &mllPairEntry, sizeof(mllPairEntry));
    cpssOsBzero((GT_VOID*) &mllPairEntryGet, sizeof(mllPairEntryGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            {CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E}
            1.1. Call with mllPairEntryIndex [0],
                           mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E],
                           mllPairEntryPtr { firstMllNode{mllNode{
                                                                 mllRPFFailCommand[CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                                                 isTunnelStart[GT_TRUE],
                                                                 nextHopVlanId[100],
                                                                 nextHopTunnelPointer[0],
                                                                 ttlHopLimitThreshold[0],
                                                                 excludeSrcVlan[GT_FALSE],
                                                                 last[GT_FALSE] },
                                               broadcastEnable[GT_FALSE],
                                               unregMulticastEnable[GT_FALSE],
                                               unknownUnicastEnable [GT_FALSE],
                                               isTagged[GT_TRUE]},
            Expected: GT_OK.
        */
        mllPairEntryIndex = 0;
        mllPairWriteForm  = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
        mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand  = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mllPairEntry.secondMllNode.mllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mllPairEntry.firstMllNode.mllNode.isTunnelStart     = GT_TRUE;
        mllPairEntry.firstMllNode.mllNode.nextHopVlanId     = 100;
        mllPairEntry.firstMllNode.mllNode.nextHopTunnelPointer = 0;
        mllPairEntry.firstMllNode.mllNode.ttlHopLimitThreshold = 0;
        mllPairEntry.firstMllNode.mllNode.excludeSrcVlan    = GT_FALSE;
        mllPairEntry.firstMllNode.mllNode.last              = GT_TRUE;
        mllPairEntry.firstMllNode.broadcastEnable           = GT_FALSE;
        mllPairEntry.firstMllNode.unregMulticastEnable      = GT_FALSE;
        mllPairEntry.firstMllNode.unknownUnicastEnable      = GT_FALSE;
        mllPairEntry.firstMllNode.isTagged                  = GT_FALSE;
        mllPairEntry.nextPointer = 0;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllPairEntryIndex, mllPairWriteForm);

        /*
            1.2. Call cpssDxCh3pIpMLLPairRead with not NULL mllPairEntryPtr
                                                   and other params from 1.1.
                                                   ( and the same for 1.7.)
            Expected: GT_OK.
        */
        st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pIpMLLPairRead: %d, %d", dev, mllPairEntryIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mllPairEntry.firstMllNode,
                                     (GT_VOID*) &mllPairEntryGet.firstMllNode,
                                     sizeof(mllPairEntry.firstMllNode) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another mllPairEntryPtr->firstMllNode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.nextPointer, mllPairEntryGet.nextPointer,
                                     "got another mllPairEntry.nextPointer then was set: %d", dev);

        /*
            1.3. Call with out of range mllPairEntryIndex [1024]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        mllPairEntryIndex = 1024;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllPairEntryIndex);

        mllPairEntryIndex = 0;

        /*
            1.4. Call with out of range mllPairWriteForm [0x5AAAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        mllPairWriteForm = IP_INVALID_ENUM_CNS;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, mllPairWriteForm = %d", dev, mllPairWriteForm);

        mllPairWriteForm = 0;


        /*
            1.5. Call with out of range mllPairEntryPtr->firstMllNode.mllNode.mllRPFFailCommand [0x5AAAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand = IP_INVALID_ENUM_CNS;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, mllPairEntryPtr->firstMllNode.mllNode.mllRPFFailCommand = %d",
                                    dev, mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand);

        mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        /*
            1.6. Call with mllPairEntryPtr->firstMllNode.mllNode.mllRPFFailCommand [CPSS_PACKET_CMD_FORWARD_E]
                           and other params from 1.1. (not supported)
            Expected: NOT GT_OK.
        */
        mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, mllPairEntryPtr->firstMllNode.mllNode.mllRPFFailCommand = %d",
                                    dev, mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand);

        mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        /*
            {CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E }
            1.7. Call with mllPairEntryIndex [1023],
                           mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E],
                           mllPairEntryPtr { secondMllNode{ mllNode{
                                                                    mllRPFFailCommand[CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                                                    isTunnelStart[GT_FALSE],
                                                                    nextHopInterface {type [CPSS_INTERFACE_TRUNK_E],
                                                                                      trunkId [2] }
                                                                    nextHopVlanId[100],
                                                                    ttlHopLimitThreshold[0],
                                                                    excludeSrcVlan[GT_FALSE],
                                                                    last[GT_FALSE] },
                                                            broadcastEnable[GT_TRUE],
                                                            unregMulticastEnable[GT_TRUE],
                                                            unknownUnicastEnable [GT_TRUE],
                                                            isTagged[GT_TRUE]},
                                             nextPointer [0] }
            Expected: GT_OK.
        */
        mllPairEntryIndex = 1023;
        mllPairWriteForm  = CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;
        mllPairEntry.secondMllNode.mllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mllPairEntry.secondMllNode.mllNode.isTunnelStart     = GT_FALSE;
        mllPairEntry.secondMllNode.mllNode.nextHopInterface.type    = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.secondMllNode.mllNode.nextHopInterface.trunkId = 2;
        mllPairEntry.secondMllNode.mllNode.nextHopVlanId     = 100;
        mllPairEntry.secondMllNode.mllNode.nextHopTunnelPointer = 0;
        mllPairEntry.secondMllNode.mllNode.ttlHopLimitThreshold = 0;
        mllPairEntry.secondMllNode.mllNode.excludeSrcVlan    = GT_FALSE;
        mllPairEntry.secondMllNode.mllNode.last              = GT_TRUE;
        mllPairEntry.secondMllNode.broadcastEnable           = GT_TRUE;
        mllPairEntry.secondMllNode.unregMulticastEnable      = GT_TRUE;
        mllPairEntry.secondMllNode.unknownUnicastEnable      = GT_TRUE;
        mllPairEntry.secondMllNode.isTagged                  = GT_TRUE;
        mllPairEntry.nextPointer = 0;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllPairEntryIndex, mllPairWriteForm);

        /*
            1.2. Call cpssDxCh3pIpMLLPairRead with not NULL mllPairEntryPtr
                                                   and other params from 1.1.
                                                   ( and the same for 1.7.)
            Expected: GT_OK.
        */
        st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pIpMLLPairRead: %d, %d", dev, mllPairEntryIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mllPairEntry.secondMllNode,
                                     (GT_VOID*) &mllPairEntryGet.secondMllNode,
                                     sizeof(mllPairEntry.secondMllNode) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another mllPairEntryPtr->secondMllNode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.nextPointer, mllPairEntryGet.nextPointer,
                                     "got another mllPairEntry.nextPointer then was set: %d", dev);

        /*
            1.8. Call with out of range mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.type [0x5AAAAAAA5]
                           and other params from 1.7.
            Expected: GT_BAD_PARAM.
        */
        mllPairEntry.secondMllNode.mllNode.nextHopInterface.type = IP_INVALID_ENUM_CNS;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                     "%d, mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.type = %d",
                                     dev, mllPairEntry.secondMllNode.mllNode.nextHopInterface.type);

        mllPairEntry.secondMllNode.mllNode.nextHopInterface.type = CPSS_INTERFACE_TRUNK_E;

        /*
            1.9. Call with mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.type [CPSS_INTERFACE_VID_E](not supported),
                            mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.vlanId [100]
                            and other params from 1.7.
            Expected: NOT GT_OK.
        */
        mllPairEntry.secondMllNode.mllNode.nextHopInterface.type   = CPSS_INTERFACE_VID_E;
        mllPairEntry.secondMllNode.mllNode.nextHopInterface.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                     "%d, mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.type = %d, mllPairEntryPtr->secondMllNode.mllNode.nextHopInterface.vlanId = %d",
                                     dev, mllPairEntry.secondMllNode.mllNode.nextHopInterface.type, mllPairEntry.secondMllNode.mllNode.nextHopInterface.vlanId);

        /*
            1.10. Call with mllPairEntryPtr->secondMllNode.mllNode.nextHopVlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (out of range)
                            and other params from 1.7.
            Expected: NOT GT_OK.
        */
        mllPairEntry.secondMllNode.mllNode.nextHopVlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                         "%d, mllPairEntryPtr->secondMllNode.mllNode.nextHopVlanId = %d",
                                         dev, mllPairEntry.secondMllNode.mllNode.nextHopVlanId);

        mllPairEntry.secondMllNode.mllNode.nextHopVlanId = 100;

        /*
            1.11. Call with out of range mllPairEntryPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mllPairEntryPtr = NULL", dev);

    }

    mllPairEntryIndex = 0;
    mllPairWriteForm  = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.firstMllNode.mllNode.isTunnelStart     = GT_TRUE;
    mllPairEntry.firstMllNode.mllNode.nextHopVlanId     = 100;
    mllPairEntry.firstMllNode.mllNode.nextHopTunnelPointer = 0;
    mllPairEntry.firstMllNode.mllNode.ttlHopLimitThreshold = 0;
    mllPairEntry.firstMllNode.mllNode.excludeSrcVlan    = GT_FALSE;
    mllPairEntry.firstMllNode.mllNode.last              = GT_TRUE;
    mllPairEntry.firstMllNode.broadcastEnable           = GT_FALSE;
    mllPairEntry.firstMllNode.unregMulticastEnable      = GT_FALSE;
    mllPairEntry.firstMllNode.unknownUnicastEnable      = GT_FALSE;
    mllPairEntry.firstMllNode.isTagged                  = GT_FALSE;
    mllPairEntry.nextPointer = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pIpMLLPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pIpMLLPairRead
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    mllPairEntryIndex,
    OUT CPSS_DXCH3P_IP_MLL_PAIR_STC *mllPairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pIpMLLPairRead)
{
/*
    ITERATE_DEVICES (DxCh3p)
    1.1. Call with mllPairEntryIndex [1023]
                   and not NULL mllPairEntryPtr.
    Expected: GT_OK.
    1.2. Call with out of range mllPairEntryIndex [1024]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with out of range mllPairEntryPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                      mllPairEntryIndex = 0;
    CPSS_DXCH3P_IP_MLL_PAIR_STC mllPairEntry;


    cpssOsBzero((GT_VOID*) &mllPairEntry, sizeof(mllPairEntry));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mllPairEntryIndex [0 / 1023]
                           and not NULL mllPairEntryPtr.
            Expected: GT_OK.
        */
        mllPairEntryIndex = 1023;

        st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllPairEntryIndex);

        /*
            1.2. Call with out of range mllPairEntryIndex [1024]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        mllPairEntryIndex = 1024;

        st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllPairEntryIndex);

        mllPairEntryIndex = 1023;

        /*
            1.3. Call with out of range mllPairEntryPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, mllPairEntryPtr = NULL", dev);
    }

    mllPairEntryIndex = 1023;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pIpMLLPairRead(dev, mllPairEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill MLL_PAIR table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pFillMllPairTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size. numEntries[1024].
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_MLL_PAIR_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in MLL_PAIR table.
         Call cpssDxCh3pIpMLLPairWrite with mllPairEntryIndex [0..numEntries-1],
                                           mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E],
                                           mllPairEntryPtr { firstMllNode{mllNode{
                                                                                 mllRPFFailCommand[CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                                                                 isTunnelStart[GT_TRUE],
                                                                                 nextHopVlanId[100],
                                                                                 nextHopTunnelPointer[0],
                                                                                 ttlHopLimitThreshold[0],
                                                                                 excludeSrcVlan[GT_FALSE],
                                                                                 last[GT_TRUE] },
                                                               broadcastEnable[GT_FALSE],
                                                               unregMulticastEnable[GT_FALSE],
                                                               unknownUnicastEnable [GT_FALSE],
                                                               isTagged[GT_FALSE]},
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pIpMLLPairWrite with mllPairEntryIndex [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in MLL_PAIR table and compare with original.
         Call cpssDxCh3pIpMLLPairRead with not NULL mllPairEntryPtr and other params from 1.1.
    Expected: GT_OK.
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pIpMLLPairRead with mllPairEntryIndex [numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual    = GT_FALSE;
    GT_U32      numEntries = 0;
    GT_U32      iTemp      = 0;

    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT      mllPairWriteForm;
    CPSS_DXCH3P_IP_MLL_PAIR_STC                 mllPairEntry;
    CPSS_DXCH3P_IP_MLL_PAIR_STC                 mllPairEntryGet;


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    mllPairWriteForm  = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

    cpssOsBzero((GT_VOID*) &mllPairEntry, sizeof(mllPairEntry));
    cpssOsBzero((GT_VOID*) &mllPairEntryGet, sizeof(mllPairEntryGet));

    /* Fill the entry for MLL_PAIR table */
    mllPairWriteForm         = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.nextPointer = 0;
    mllPairEntry.firstMllNode.mllNode.mllRPFFailCommand    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.secondMllNode.mllNode.mllRPFFailCommand   = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.firstMllNode.mllNode.isTunnelStart        = GT_TRUE;
    mllPairEntry.firstMllNode.mllNode.nextHopVlanId        = 100;
    mllPairEntry.firstMllNode.mllNode.nextHopTunnelPointer = 0;
    mllPairEntry.firstMllNode.mllNode.ttlHopLimitThreshold = 0;
    mllPairEntry.firstMllNode.mllNode.excludeSrcVlan       = GT_FALSE;
    mllPairEntry.firstMllNode.mllNode.last                 = GT_TRUE;
    mllPairEntry.firstMllNode.broadcastEnable              = GT_FALSE;
    mllPairEntry.firstMllNode.unregMulticastEnable         = GT_FALSE;
    mllPairEntry.firstMllNode.unknownUnicastEnable         = GT_FALSE;
    mllPairEntry.firstMllNode.isTagged                     = GT_FALSE;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        numEntries = 1024;
        /*st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_MLL_PAIR_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);*/

        /* 1.2. Fill all entries in MLL_PAIR table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pIpMLLPairWrite(dev, iTemp, mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pIpMLLPairWrite: %d, %d, %d", dev, iTemp, mllPairWriteForm);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pIpMLLPairWrite(dev, numEntries, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pIpMLLPairWrite: %d, %d, %d",
                                     dev, numEntries, mllPairWriteForm);

        /* 1.4. Read all entries in MLL_PAIR table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pIpMLLPairRead(dev, iTemp, mllPairWriteForm, &mllPairEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pIpMLLPairRead: %d, %d", dev, iTemp);

            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mllPairEntry.firstMllNode,
                                         (GT_VOID*) &mllPairEntryGet.firstMllNode,
                                         sizeof(mllPairEntry.firstMllNode) )) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another mllPairEntryPtr->firstMllNode then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.nextPointer, mllPairEntryGet.nextPointer,
                                         "got another mllPairEntry.nextPointer then was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pIpMLLPairRead(dev, numEntries, mllPairWriteForm, &mllPairEntryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pIpMLLPairRead: %d, %d", dev, numEntries);
    }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxCh3pIp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxCh3pIp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pIpMLLPairWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pIpMLLPairRead)
    /* Tests for tables */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pFillMllPairTable)
UTF_SUIT_END_TESTS_MAC(cpssDxCh3pIp)

