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
* @file cpssDxCh3pBrgCapwapUT.c
*
* @brief Unit tests for cpssDxCh3pBrgCapwap, that provides
* Ingress MAC SA and Egress MAC DA tables facility CPSS DxCh3p API.
*
* @version   1.5
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxCh3p/bridge/cpssDxCh3pBrgCapwap.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
/* Invalid enum */
#define BRG_CAPWAP_INVALID_ENUM_CNS     0x5AAAAAA5


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pBrgCapwapHashCalc
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macAddrPtr,
    OUT GT_U32          *hashPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgCapwapHashCalc)
{
/*
    ITERATE_DEVICES (DXCH3P and above)
    1.1. Call function with macAddr { arEther [0x10, 0x20, 0x30, 0x40, 0x50, 0x60] /
                                               [0x00, 0x00, 0x00, 0x00, 0x00, 0x00]}
                            and not-NULL hashPtr
    Expected: GT_OK
    1.2. Call function with macAddrPtr [NULL]
                            and other param from 1.1.
    Expected: GT_BAD_PTR
    1.3. Call function with hashrPtr [NULL]
                            and other param from 1.1.
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_ETHERADDR    macAddr;
    GT_U32          hash = 0;


    cpssOsBzero((GT_VOID*) &macAddr, sizeof(macAddr));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with macAddr { arEther [0x10, 0x20, 0x30, 0x40, 0x50, 0x60]}
                                    and not-NULL hashPtr
            Expected: GT_OK
        */
        macAddr.arEther[0] = 0x10;
        macAddr.arEther[1] = 0x20;
        macAddr.arEther[2] = 0x30;
        macAddr.arEther[3] = 0x40;
        macAddr.arEther[4] = 0x50;
        macAddr.arEther[5] = 0x10;

        st = cpssDxCh3pBrgCapwapHashCalc(dev, &macAddr, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with macAddrPtr [NULL]
                                    and other param from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pBrgCapwapHashCalc(dev, NULL, &hash);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macAddrPtr = NULL", dev);

        /*
            1.3. Call function with hashrPtr [NULL]
                                    and other param from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pBrgCapwapHashCalc(dev, &macAddr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, hashPtr = NULL", dev);
    }

    macAddr.arEther[0] = 0x10;
    macAddr.arEther[1] = 0x20;
    macAddr.arEther[2] = 0x30;
    macAddr.arEther[3] = 0x40;
    macAddr.arEther[4] = 0x50;
    macAddr.arEther[5] = 0x10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pBrgCapwapHashCalc(dev, &macAddr, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pBrgCapwapHashCalc(dev, &macAddr, &hash);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pBrgCapwapMacEntryWrite
(
    IN GT_U8                                                   devNum,
    IN GT_U32                                                  index,
    IN CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC                 *macEntryPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgCapwapMacEntryWrite)
{
/*
    ITERATE_DEVICES (DXCH3P and above)
    1.1. Write new Mac Entry.
         Call function with index [0 / 0x3FFF],
                            macEntry {skip[GT_FALSE],
                                      macAddr{arEther[10, 20, 30, 40, 50, 60]},
                                      saPacketCmd[CPSS_PACKET_CMD_FORWARD_E /
                                                  CPSS_PACKET_CMD_DROP_HARD_E],
                                      saVlanId[100 / 4095]
                                      saRssiWeightProfile[0 / 3],
                                      saRssiAverage[0 / 3]
                                      saRedirectEnable[GT_TRUE / GT_FALSE],
                                      saEgressInterface{ VirtDevNum[dev],
                                                         VirtPortNum[0] },
                                      daPacketCmd [CPSS_PACKET_CMD_FORWARD_E /
                                                   CPSS_PACKET_CMD_DROP_HARD_E]
                                      daBasedVlanEnable[GT_TRUE / GT_FALSE],
                                      daVlanId[100]
                                      daQosEnable[GT_TRUE],
                                      daEgressMappingProfile[0]}
    Expected: GT_OK.
    1.2. Try to read created Mac Entry.
        Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                  and not-NULL macEntryPtr
                                                  and Index as in 1.1.
    Expected: GT_OK and macEntry as was set and valid[GT_TRUE].
    1.3. Call with macEntryPtr->saPacketCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                             CPSS_PACKET_CMD_BRIDGE_E] (not supported)
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range macEntryPtr->saPacketCmd [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range macEntryPtr->saVlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with out of range macEntryPtr->saRssiWeightProfile [4]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with macEntryPtr->daPacketCmd [CPSS_PACKET_CMD_ROUTE_E /
                                             CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E] (not supported)
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with out of range macEntryPtr->daPacketCmd [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call with out of range macEntryPtr->daVlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.10. Call with out of range index [0x4000]
                    and other param from 1.1.
    Expected: NOT GT_OK.
    1.11. Call with macEntryPtr [NULL]
                    and other param from 1.1.
    Expected: GT_BAD_PTR.
    1.12. Try to read not created Mac Entry.
          Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                  and not-NULL macEntryPtr
                                                  and Index[2].
    Expected: GT_OK and valid[GT_FALSE].
    1.13. Invalidate created Mac Entry.
          Call cpssDxCh3pBrgCapwapMacEntryInvalidate with index[0 / 0x3FFF].
    Expected: GT_OK.
    1.14. Try to read Invalidated Mac Entry.
          Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                    and not-NULL macEntryPtr,
                                                    index as in 1.1.
    Expected: GT_OK and valid [GT_FALSE].
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      index   = 0;
    GT_BOOL     valid   = GT_FALSE;
    GT_BOOL     isEqual = GT_FALSE;

    CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC macEntry;
    CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC macEntryGet;


    cpssOsBzero((GT_VOID*) &macEntry, sizeof(macEntry));
    cpssOsBzero((GT_VOID*) &macEntryGet, sizeof(macEntryGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Write new Mac Entry.
                 Call function with index [0 / 0x3FFF],
                                    macEntry {skip[GT_FALSE],
                                              macAddr{arEther[10, 20, 30, 40, 50, 60]},
                                              saPacketCmd[CPSS_PACKET_CMD_FORWARD_E /
                                                          CPSS_PACKET_CMD_DROP_HARD_E],
                                              saVlanId[100 / 4095]
                                              saRssiWeightProfile[0 / 3],
                                              saRssiAverage[0 / 3]
                                              saRedirectEnable[GT_TRUE / GT_FALSE],
                                              saEgressInterface{ VirtDevNum[dev],
                                                                 VirtPortNum[0] },
                                              daPacketCmd [CPSS_PACKET_CMD_FORWARD_E /
                                                           CPSS_PACKET_CMD_DROP_HARD_E]
                                              daBasedVlanEnable[GT_TRUE / GT_FALSE],
                                              daVlanId[100]
                                              daQosEnable[GT_TRUE],
                                              daEgressMappingProfile[0]}
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        macEntry.skip               = GT_FALSE;
        macEntry.macAddr.arEther[0] = 0x10;
        macEntry.macAddr.arEther[1] = 0x20;
        macEntry.macAddr.arEther[2] = 0x30;
        macEntry.macAddr.arEther[3] = 0x40;
        macEntry.macAddr.arEther[4] = 0x50;
        macEntry.macAddr.arEther[5] = 0x10;
        macEntry.saPacketCmd        = CPSS_PACKET_CMD_FORWARD_E;
        macEntry.saVlanId           = 100;

        macEntry.saRssiWeightProfile = 0;
        macEntry.saRssiAverage       = 0;
        macEntry.saRedirectEnable    = GT_TRUE;

        macEntry.saEgressInterface.VirtDevNum  = dev;
        macEntry.saEgressInterface.VirtPortNum = 0;

        macEntry.daPacketCmd            = CPSS_PACKET_CMD_FORWARD_E;
        macEntry.daBasedVlanEnable      = GT_TRUE;
        macEntry.daVlanId               = 100;
        macEntry.daQosEnable            = GT_TRUE;
        macEntry.daEgressMappingProfile = 0;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Try to read created Mac Entry.
                Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                          and not-NULL macEntryPtr
                                                          and Index as in 1.1.
            Expected: GT_OK and macEntry as was set and valid[GT_TRUE].
        */
        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d", dev, index);

        if(st == GT_OK)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, valid, "%d, valid = %d", dev, valid);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry,
                                         (GT_VOID*) &macEntryGet,
                                         sizeof(macEntry))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another macEntry as was set: %d", dev);
        }

        /* iterate with index = 0x3FFF */
        index = 0x3FFF;

        macEntry.skip        = GT_FALSE;
        macEntry.saPacketCmd = CPSS_PACKET_CMD_DROP_HARD_E;
        macEntry.saVlanId    = 4095;

        macEntry.saRssiWeightProfile = 3;
        macEntry.saRssiAverage       = 3;
        macEntry.saRedirectEnable    = GT_FALSE;

        macEntry.saEgressInterface.VirtDevNum  = dev;
        macEntry.saEgressInterface.VirtPortNum = 0;

        macEntry.daPacketCmd            = CPSS_PACKET_CMD_DROP_HARD_E;
        macEntry.daBasedVlanEnable      = GT_FALSE;
        macEntry.daVlanId               = 100;
        macEntry.daQosEnable            = GT_TRUE;
        macEntry.daEgressMappingProfile = 0;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Try to read created Mac Entry.
                Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                          and not-NULL macEntryPtr
                                                          and Index as in 1.1.
            Expected: GT_OK and macEntry as was set and valid [GT_TRUE].
        */
        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d", dev, index);

        if(st == GT_OK)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, valid, "%d, valid = %d", dev, valid);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry,
                                         (GT_VOID*) &macEntryGet,
                                         sizeof(macEntry))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another macEntry as was set: %d", dev);
        }

        /*
            1.3. Call with macEntryPtr->saPacketCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                                     CPSS_PACKET_CMD_BRIDGE_E] (not supported)
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        /* iterate with macEntry.saPacketCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
        macEntry.saPacketCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saPacketCmd = %d",
                                         dev, macEntry.saPacketCmd);

        /* iterate with macEntry.saPacketCmd = CPSS_PACKET_CMD_BRIDGE_E */
        macEntry.saPacketCmd = CPSS_PACKET_CMD_BRIDGE_E;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saPacketCmd = %d",
                                        dev, macEntry.saPacketCmd);

        /*
            1.4. Call with out of range macEntryPtr->saPacketCmd [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        macEntry.saPacketCmd = BRG_CAPWAP_INVALID_ENUM_CNS;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, macEntryPtr->saPacketCmd = %d",
                                        dev, macEntry.saPacketCmd);

        macEntry.saPacketCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.5. Call with out of range macEntryPtr->saVlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.saVlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saVlanId = %d",
                                         dev, macEntry.saVlanId);

        macEntry.saVlanId = 100;

        /*
            1.6. Call with out of range macEntryPtr->saRssiWeightProfile [4]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.saRssiWeightProfile = 4;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saRssiWeightProfile = %d",
                                         dev, macEntry.saRssiWeightProfile);

        macEntry.saRssiWeightProfile = 3;

        /*
            1.7. Call with macEntryPtr->daPacketCmd [CPSS_PACKET_CMD_ROUTE_E /
                                                     CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E] (not supported)
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        /* iterate with macEntry.daPacketCmd = CPSS_PACKET_CMD_ROUTE_E */
        macEntry.daPacketCmd = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daPacketCmd = %d",
                                         dev, macEntry.daPacketCmd);

        /* iterate with macEntry.daPacketCmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E */
        macEntry.daPacketCmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daPacketCmd = %d",
                                         dev, macEntry.daPacketCmd);

        macEntry.daPacketCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.8. Call with out of range macEntryPtr->daPacketCmd [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        macEntry.daPacketCmd = BRG_CAPWAP_INVALID_ENUM_CNS;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, macEntryPtr->daPacketCmd = %d",
                                     dev, macEntry.daPacketCmd);

        macEntry.daPacketCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.9. Call with out of range macEntryPtr->daVlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.daVlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daVlanId = %d",
                                         dev, macEntry.daVlanId);

        macEntry.daVlanId = 100;

        /*
            1.10. Call with out of range index [0x3FFF + 1]
                            and other param from 1.1.
            Expected: NOT GT_OK.
        */
        index = 0x4000;

        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 0x3FFF;

        /*
            1.11. Call with macEntryPtr [NULL]
                            and other param from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macEntryPtr = NULL", dev);

        /*
            1.12. Try to read not created Mac Entry.
                  Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                          and not-NULL macEntryPtr
                                                          and Index[2].
            Expected: GT_OK and valid[GT_FALSE].
        */
        index = 2;

        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d",
                                     dev, index);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, valid, "cpssDxCh3pBrgCapwapMacEntryRead: %d, valid = %d",
                                     dev, valid);

        /*
            1.13. Invalidate created Mac Entry.
                  Call cpssDxCh3pBrgCapwapMacEntryInvalidate with index[0 / 0x3FFF].
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryInvalidate: %d, %d",
                                     dev, index);

        /* iterate with index = 0x3FFF */
        index = 0x3FFF;

        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryInvalidate: %d, %d",
                                     dev, index);

        /*
            1.14. Try to read Invalidated Mac Entry.
                  Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                            and not-NULL macEntryPtr,
                                                            index as in 1.1.
            Expected: GT_OK and valid [GT_FALSE].
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid,&macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d",
                                     dev, index);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, valid, "cpssDxCh3pBrgCapwapMacEntryRead: %d, valid = %d",
                                     dev, valid);

        /* iterate with index = 0x3FFF */
        index = 0x3FFF;

        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid,&macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d",
                                     dev, index);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, valid, "cpssDxCh3pBrgCapwapMacEntryRead: %d, valid = %d",
                                     dev, valid);
    }

    index = 0;

    macEntry.skip = GT_FALSE;

    macEntry.macAddr.arEther[0] = 0x10;
    macEntry.macAddr.arEther[1] = 0x20;
    macEntry.macAddr.arEther[2] = 0x30;
    macEntry.macAddr.arEther[3] = 0x40;
    macEntry.macAddr.arEther[4] = 0x50;
    macEntry.macAddr.arEther[5] = 0x10;

    macEntry.saPacketCmd = CPSS_PACKET_CMD_FORWARD_E;
    macEntry.saVlanId    = 100;

    macEntry.saRssiWeightProfile = 0;
    macEntry.saRssiAverage       = 0;
    macEntry.saRedirectEnable    = GT_TRUE;

    macEntry.saEgressInterface.VirtDevNum  = 0;
    macEntry.saEgressInterface.VirtPortNum = 0;

    macEntry.daPacketCmd            = CPSS_PACKET_CMD_FORWARD_E;
    macEntry.daBasedVlanEnable      = GT_TRUE;
    macEntry.daVlanId               = 100;
    macEntry.daQosEnable            = GT_TRUE;
    macEntry.daEgressMappingProfile = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, index, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pBrgCapwapMacEntryRead
(
    IN  GT_U8                                                   devNum,
    IN  GT_U32                                                  index,
    OUT GT_BOOL                                                 *validPtr,
    OUT CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC                 *macEntryPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgCapwapMacEntryRead)
{
/*
    ITERATE_DEVICES (DXCH3P and above)
    1.1. Call function with index [0 / 0x3FFF],
                            not-NULL validPtr,
                            not-NULL macEntryPtr
    Expected: GT_OK.
    1.2. Call function with out of range index [0x3FFF+1]
                            and other params from 1.1.
    Expected: GT_OK
    1.3. Call function with validPtr [NULL]
                            and other params from 1.1.
    Expected: GT_BAD_PTR
    1.4. Call function with macEntryPtr [NULL]
                            and other params from 1.1.
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      index = 0;
    GT_BOOL     valid = GT_FALSE;

    CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC macEntry;


    cpssOsBzero((GT_VOID*) &macEntry, sizeof(macEntry));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0 / 0x3FFF],
                                    not-NULL validPtr,
                                    not-NULL macEntryPtr
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* iterate with index = 0x3FFF */
        index = 0x3FFF;

        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call function with out of range index [0x3FFF+1]
                                    and other params from 1.1.
            Expected: NOT GT_OK
        */
        index = 0x4000;

        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 0;

        /*
            1.3. Call function with validPtr [NULL]
                                    and other params from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, NULL, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%s, validPtr = NULL", dev);

        /*
            1.4. Call function with macEntryPtr [NULL]
                                    and other params from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%s, macEntryPtr = NULL", dev);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pBrgCapwapMacEntryRead(dev, index, &valid, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pBrgCapwapMacEntryInvalidate
(
    IN GT_U8       devNum,
    IN GT_U32      index
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgCapwapMacEntryInvalidate)
{
/*
    ITERATE_DEVICES (DXCH3P and above)
    1.1. Call function with index [0 / 0x3FFF]
    Expected: GT_OK.
    1.1. Call function with out of range index [0x3FFF+1]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      index = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0 / 0x3FFF].
            Expected: GT_OK.
        */
        /* iterate with index = 0 */
        index = 0;

        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* iterate with index = 0x3FFF */
        index = 0x3FFF;

        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.1. Call function with index [0x3FFF+1].
            Expected: NOT GT_OK.
        */
        index = 0x4000;

        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill CAPWAP MAC table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgCapwapFillCapwapMacTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_FDB_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in CAPWAP MAC address table.
         Call cpssDxCh3pBrgCapwapMacEntryWrite index [0...numEntries-1],
                            macEntry {skip[GT_FALSE],
                                      macAddr{arEther[10, 20, 30, 40, 50, 60]},
                                      saPacketCmd[CPSS_PACKET_CMD_FORWARD_E],
                                      saVlanId[100]
                                      saRssiWeightProfile[3],
                                      saRssiAverage[3]
                                      saRedirectEnable[GT_TRUE],
                                      saEgressInterface{ VirtDevNum[dev],
                                                         VirtPortNum[0] },
                                      daPacketCmd [CPSS_PACKET_CMD_FORWARD_E]
                                      daBasedVlanEnable[GT_TRUE],
                                      daVlanId[100]
                                      daQosEnable[GT_TRUE],
                                      daEgressMappingProfile[0]}
    Expected: GT_OK.

    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pBrgCapwapMacEntryWrite index [numEntries],
    Expected: NOT GT_OK.
    1.4. Read all entries in CAPWAP MAC address table and compare with original.
         Call cpssDxCh3pBrgCapwapMacEntryRead with not-NULL validPtr
                                                   and not-NULL macEntryPtr
                                                   and Index as in 1.2.
    Expected: GT_OK and macEntry as was set and valid[GT_TRUE].
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pBrgCapwapMacEntryRead with index [numEntries].
    Expected: NOT GT_OK.
    1.6. Delete all entries in CAPWAP MAC address table.
         Call cpssDxCh3pBrgCapwapMacEntryInvalidate with index as in 1.2.
    Expected: GT_OK.
    1.7. Try to delete entry with index out of range.
         Call cpssDxCh3pBrgCapwapMacEntryInvalidate with index [numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries = 0;
    GT_U32      iTemp      = 0;
    GT_BOOL     valid      = GT_FALSE;
    GT_BOOL     isEqual    = GT_FALSE;

    CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC macEntry;
    CPSS_DXCH3P_BRIDGE_CAPWAP_MAC_ENTRY_STC macEntryGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &macEntry, sizeof(macEntry));
    cpssOsBzero((GT_VOID*) &macEntryGet, sizeof(macEntryGet));

    /* Fill the entry for CAPWAP MAC address table */
    macEntry.skip               = GT_FALSE;
    macEntry.macAddr.arEther[0] = 0x10;
    macEntry.macAddr.arEther[1] = 0x20;
    macEntry.macAddr.arEther[2] = 0x30;
    macEntry.macAddr.arEther[3] = 0x40;
    macEntry.macAddr.arEther[4] = 0x50;
    macEntry.macAddr.arEther[5] = 0x10;
    macEntry.saPacketCmd        = CPSS_PACKET_CMD_FORWARD_E;
    macEntry.saVlanId           = 100;

    macEntry.saRssiWeightProfile = 0;
    macEntry.saRssiAverage       = 0;
    macEntry.saRedirectEnable    = GT_TRUE;

    macEntry.daPacketCmd            = CPSS_PACKET_CMD_FORWARD_E;
    macEntry.daBasedVlanEnable      = GT_TRUE;
    macEntry.daVlanId               = 100;
    macEntry.daQosEnable            = GT_TRUE;
    macEntry.daEgressMappingProfile = 0;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Fill the saEgressInterface of entry */
        macEntry.saEgressInterface.VirtDevNum  = dev;
        macEntry.saEgressInterface.VirtPortNum = 0;

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_FDB_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        /* 1.2. Fill all entries in CAPWAP MAC address table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            /* make every entry unique */
            macEntry.macAddr.arEther[2] = (GT_U8)(iTemp % 255);
            macEntry.macAddr.arEther[3] = (GT_U8)(iTemp % 255);
            macEntry.macAddr.arEther[4] = (GT_U8)(iTemp % 255);
            macEntry.macAddr.arEther[5] = (GT_U8)(iTemp % 255);

            st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, iTemp, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryWrite: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pBrgCapwapMacEntryWrite(dev, numEntries, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in CAPWAP MAC address table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            /* restore unique entry before compare */
            macEntry.macAddr.arEther[2] = (GT_U8)(iTemp % 255);
            macEntry.macAddr.arEther[3] = (GT_U8)(iTemp % 255);
            macEntry.macAddr.arEther[4] = (GT_U8)(iTemp % 255);
            macEntry.macAddr.arEther[5] = (GT_U8)(iTemp % 255);

            st = cpssDxCh3pBrgCapwapMacEntryRead(dev, iTemp, &valid, &macEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d", dev, iTemp);

            if(st == GT_OK)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, valid, "%d, valid = %d", dev, valid);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry,
                                             (GT_VOID*) &macEntryGet,
                                             sizeof(macEntry))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another macEntry as was set: %d", dev);
            }
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pBrgCapwapMacEntryRead(dev, numEntries, &valid, &macEntryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryRead: %d, %d", dev, numEntries);

        /* 1.6. Delete all entries in CAPWAP MAC address table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, iTemp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryInvalidate: %d, %d", dev, iTemp);
        }

        /* 1.7. Try to delete entry with index out of range. */
        st = cpssDxCh3pBrgCapwapMacEntryInvalidate(dev, numEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgCapwapMacEntryInvalidate: %d, %d", dev, numEntries);
    }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxCh3pBrgCapwap suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxCh3pBrgCapwap)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgCapwapHashCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgCapwapMacEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgCapwapMacEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgCapwapMacEntryInvalidate)
    /* Tests for table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgCapwapFillCapwapMacTable)
UTF_SUIT_END_TESTS_MAC(cpssDxCh3pBrgCapwap)

