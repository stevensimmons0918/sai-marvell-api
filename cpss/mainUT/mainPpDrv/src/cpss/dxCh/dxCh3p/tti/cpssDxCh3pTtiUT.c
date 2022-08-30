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
* @file cpssDxCh3pTtiUT.c
*
* @brief Unit tests for cpssDxCh3pTti, that provides
* CPSS tunnel termination declerations.
*
*
* @version   1.3
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxCh3p/tti/cpssDxCh3pTti.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
/* Default valid value for port id */
#define TTI_VALID_PHY_PORT_CNS   0

/* Invalid enum */
#define TTI_INVALID_ENUM_CNS     0x5AAAAAA5

/* Internal functions forward declaration */
/* Set Pattern to default values */
static void prvSet_Default_Pattern(OUT CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC *patternPtr);

/* Set Action to default values */
static void prvSet_Default_Action(OUT CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC *actionPtr);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiPortLookupEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               port,
    IN  CPSS_DXCH3P_TTI_KEY_TYPE_ENT        keyType,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiPortLookupEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DXCH3P)
    1.1.1. Call with keyType [CPSS_DXCH3P_TTI_KEY_IPV4_E /
                              CPSS_DXCH3P_TTI_KEY_MPLS_E ],
                     enable[GT_FALSE / GT_BOOL].
    Expected: GT_OK.
    1.1.2. Call cpssDxCh3pTtiPortLookupEnableGet with not NULL enablePtr
                                                      and other params from 1.1.1.
    Expected: GT_OK and same enable as was set
    1.1.3. Call with out of range keyType[0x5AAAAAA5]
                     and other params from 1.1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8       port;

    CPSS_DXCH3P_TTI_KEY_TYPE_ENT    keyType   = CPSS_DXCH3P_TTI_KEY_IPV4_E;
    GT_BOOL                         enable    = GT_FALSE;
    GT_BOOL                         enableGet = GT_FALSE;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {

            /*
                1.1.1. Call with keyType [CPSS_DXCH3P_TTI_KEY_IPV4_E /
                                          CPSS_DXCH3P_TTI_KEY_MPLS_E ],
                                 enable[GT_FALSE / GT_BOOL].
                Expected: GT_OK.
            */
            /* iterate with keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E */
            keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;
            enable  = GT_FALSE;

            st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, keyType, enable);

            /*
                1.1.2. Call cpssDxCh3pTtiPortLookupEnableGet with not NULL enablePtr
                                                                  and other params from 1.1.1.
                Expected: GT_OK and same enable as was set
            */
            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enableGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiPortLookupEnableGet: %d, %d, %d",
                                         dev, port, keyType);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d",
                                         dev, port);

            /* iterate with keyType = CPSS_DXCH3P_TTI_KEY_MPLS_E */
            keyType = CPSS_DXCH3P_TTI_KEY_MPLS_E;
            enable  = GT_TRUE;

            st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, keyType, enable);

            /*
                1.1.2. Call cpssDxCh3pTtiPortLookupEnableGet with not NULL enablePtr
                                                                  and other params from 1.1.1.
                Expected: GT_OK and same enable as was set
            */
            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enableGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiPortLookupEnableGet: %d, %d, %d",
                                         dev, port, keyType);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d",
                                         dev, port);

            /*
                1.1.3. Call with out of range keyType[0x5AAAAAA5]
                                 and other params from 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            keyType = TTI_INVALID_ENUM_CNS;

            st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, keyType);
        }

        keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;
        enable  = GT_FALSE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;
    enable  = GT_FALSE;
    port = TTI_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiPortLookupEnableSet(dev, port, keyType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiPortLookupEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               port,
    IN  CPSS_DXCH3P_TTI_KEY_TYPE_ENT        keyType,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiPortLookupEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DXCH3P)
    1.1.1. Call with keyType [CPSS_DXCH3P_TTI_KEY_IPV4_E /
                              CPSS_DXCH3P_TTI_KEY_MPLS_E ]
                     and not NULL  enablePtr.
    Expected: GT_OK.
    1.1.2. Call with out of range keyType [0x5AAAAAA5]
                     and not NULL  enablePtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with enablePtr [NULL]
                     and other param from 1.1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8       port;

    CPSS_DXCH3P_TTI_KEY_TYPE_ENT    keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;
    GT_BOOL                         enable  = GT_FALSE;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with keyType [CPSS_DXCH3P_TTI_KEY_IPV4_E /
                                          CPSS_DXCH3P_TTI_KEY_MPLS_E ]
                                 and not NULL  enablePtr.
                Expected: GT_OK.
            */
            /* iterate with keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E */
            keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;

            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, keyType);

            /* iterate with keyType = CPSS_DXCH3P_TTI_KEY_MPLS_E */
            keyType = CPSS_DXCH3P_TTI_KEY_MPLS_E;

            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, keyType);

            /*
                1.1.2. Call with out of range keyType [0x5AAAAAA5]
                                 and not NULL  enablePtr.
                Expected: GT_BAD_PARAM.
            */
            keyType = TTI_INVALID_ENUM_CNS;

            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, keyType);

            keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;

            /*
                1.1.3. Call with enablePtr [NULL]
                                 and other param from 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;
    port    = TTI_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiPortLookupEnableGet(dev, port, keyType, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiCapwapRuleSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  pclCapwapRuleIndex,
    IN  CPSS_DXCH3P_TTI_KEY_TYPE_ENT            keyType,
    IN  CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC *patternPtr,
    IN  CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC *maskPtr,
    IN  CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC       *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiCapwapRuleSet)
{
/*
    ITERATE_DEVICES (DXCH3P)
    {keyType=CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E}
    1.1. Call with pclCapwapRuleIndex[0],
                   keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                   patternPtr{ pclId[0],
                               localDevSrcIsTrunk[GT_FALSE],
                               localDevSrcPortTrunk[0],
                               vid[100],
                               srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
                               destIp{ ipv4Addr{ arIP[192.168.0.2] } },
                               udpSrcPort[9001],
                               udpDestPort[9002],
                               isDtlsEncripted[GT_TRUE],
                               capwapHeaderWord0[0xFFFFFFFF],
                               macAddr{ arEther[10, 20, 30, 40, 50, 60] },
                               frameControlToDS[0],
                               frameControlFromDS[0],
                               frameControlType[0],
                               frameControlSubtype[0]
                               isProtected[GT_TRUE]},
                   maskPtr[all bits set to 1],
                   actionPtr{
                            tunnelTerminate[GT_FALSE],
                            vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
                            vid[100],
                            rssiUpdateEnable[GT_FALSE],
                            bridgeAutoLearnEnable[GT_FALSE],
                            naToCPUEnable[GT_FALSE],
                            unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
                            qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
                            qosAtributes{qosProfile[0], up[0]} ,
                            qosTidMappingProfileTable[0],
                            srcVirtualPortAssignment{ srcDevId[0],
                                                      srcPortId[0] },
                            localSwitchingEnable[GT_FALSE],
                            arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
                            ieeeResMcCmdProfile[0],
                            srcId[31] }.
    Expected: GT_OK.
    1.2. Call cpssDxCh3pTtiCapwapRuleGet with not NULL patternPtr, maskPtr, actionPtr
                                              and other params from 1.1.
    Expected: GT_OK.
    1.3. Call with out of range keyType [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with keyType [CPSS_DXCH3P_TTI_KEY_MPLS_E]
                   and other params from 1.1. (not supported)
    Expected: NOT GT_OK.
    1.5. Call with patternPtr ->localDevSrcIsTrunk [GT_FALSE]
                   and localDevSrcPortTrunk [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with patternPtr ->localDevSrcIsTrunk [GT_TRUE]
                   and patternPtr ->localDevSrcPortTrunk [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   and other params from 1.1.
    Expected: GT_OK.
    1.7. Call with out of range patternPtr -> vid [4096]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with out of range actionPtr-> passengerPacketType [0x5AAAAAA5]
                    action.tunnelTerminate [GT_TRUE]
                    and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call with out of range actionPtr-> passengerPacketType [0x5AAAAAA5]
                    action.tunnelTerminate [GT_FALSE]
                    and other params from 1.1.
    Expected: GT_OK. (not relevant)
    1.10. Call with out of range actionPtr-> vlanAssignment [0x5AAAAAA5]
                    and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.11. Call with out of range actionPtr-> vid [4096]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.12. Call with out of range actionPtr-> unknownSACmd [0x5AAAAAA5]
                    and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.13. Call with not valid actionPtr-> unknownSACmd [CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.14. Call with out of range actionPtr-> qosMode [0x5AAAAAA5]
                    and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.15. Call with out of range actionPtr-> qosTidMappingProfileTable [8]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.16. Call with out of range actionPtr-> srcVirtualPortAssignment.
                    srcDevId [PRV_CPSS_MAX_PP_DEVICES_CNS]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.17. Call with out of range actionPtr-> srcVirtualPortAssignment.
                    srcPortId [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.18. Call with not valid actionPtr-> arpCmd [CPSS_DXCH3P_PACKET_CMD_ROUTE_E]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.19. Call with out of range actionPtr-> arpCmd [0x5AAAAAA5]
                    and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.20. Call with out of range actionPtr-> srcId [32]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.21. Call with patternPtr [NULL]
                    and other params from 1.18.
    Expected: GT_BAD_PTR.
    1.22. Call with maskPtr [NULL]
                    and other params from 1.18.
    Expected: GT_BAD_PTR.
    1.23. Call with actionPtr [NULL]
                    and other params from 1.18.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                                  pclCapwapRuleIndex = 0;
    GT_BOOL                                 isEqual = GT_FALSE;
    CPSS_DXCH3P_TTI_KEY_TYPE_ENT            keyType;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC pattern;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC mask;
    CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC       action;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC patternGet;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC maskGet;
    CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC       actionGet;


    keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

    cpssOsBzero((GT_VOID*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
    cpssOsBzero((GT_VOID*) &action, sizeof(action));
    cpssOsBzero((GT_VOID*) &patternGet, sizeof(patternGet));
    cpssOsBzero((GT_VOID*) &maskGet, sizeof(maskGet));
    cpssOsBzero((GT_VOID*) &actionGet, sizeof(actionGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            {keyType=CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E}
            1.1. Call with pclCapwapRuleIndex[0],
                           keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                           patternPtr{ pclId[0],
                                       localDevSrcIsTrunk[GT_FALSE],
                                       localDevSrcPortTrunk[0],
                                       vid[0],
                                       srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
                                       destIp{ ipv4Addr{ arIP[192.168.0.2] } },
                                       udpSrcPort[9001],
                                       udpDestPort[9002],
                                       isDtlsEncripted[GT_TRUE],
                                       capwapHeaderWord0[0xFFFFFFFF],
                                       macAddr{ arEther[10, 20, 30, 40, 50, 60] },
                                       frameControlToDS[0],
                                       frameControlFromDS[0],
                                       frameControlType[0],
                                       frameControlSubtype[0]
                                       isProtected[GT_TRUE]},
                           maskPtr[all bits set to 1],
                           actionPtr{
                                    tunnelTerminate[GT_FALSE],
                                    vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
                                    vid[100],
                                    rssiUpdateEnable[GT_FALSE],
                                    bridgeAutoLearnEnable[GT_FALSE],
                                    naToCPUEnable[GT_FALSE],
                                    unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
                                    qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
                                    qosAtributes{qosProfile[0],up[0]},
                                    qosTidMappingProfileTable[0],
                                    srcVirtualPortAssignment{ srcDevId[0],
                                                              srcPortId[0] },
                                    localSwitchingEnable[GT_FALSE],
                                    arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
                                    ieeeResMcCmdProfile[0],
                                    srcId[31] }.
            Expected: GT_OK.
        */
        pclCapwapRuleIndex = 0;

        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        prvSet_Default_Pattern(&pattern);
        prvSet_Default_Action(&action);

        cpssOsBzero((GT_VOID*) &(mask), sizeof(mask));

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pclCapwapRuleIndex, keyType);

        /*
            1.2. Call cpssDxCh3pTtiCapwapRuleGet with not NULL patternPtr, maskPtr, actionPtr
                                                      and other params from 1.1.
            Expected: GT_OK.
        */
        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &patternGet, &maskGet, &actionGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiCapwapRuleGet: %d, %d, %d",
                                     dev, keyType, pclCapwapRuleIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &pattern, (GT_VOID*) &patternGet, sizeof(pattern) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another pattern than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mask, (GT_VOID*) &maskGet, sizeof(mask) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another mask than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &action, (GT_VOID*) &actionGet, sizeof(action) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another action than was set: %d", dev);

        /*
            1.3. Call with out of range keyType [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        keyType = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, keyType = %d", dev, keyType);

        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        /*
            1.4. Call with keyType [CPSS_DXCH3P_TTI_KEY_MPLS_E]
                           and other params from 1.1. (not supported)
            Expected: NOT GT_OK.
        */
        keyType = CPSS_DXCH3P_TTI_KEY_MPLS_E;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, keyType = %d", dev, keyType);

        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        /*
            1.5. Call with patternPtr ->localDevSrcIsTrunk [GT_FALSE]
                           and localDevSrcPortTrunk [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        pattern.localDevSrcIsTrunk   = GT_FALSE;
        pattern.localDevSrcPortTrunk = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, patternPtr->localDevSrcIsTrunk = %d, patternPtr->localDevSrcPortTrunk = %d",
                                                    dev, pattern.localDevSrcIsTrunk,
                                                    pattern.localDevSrcPortTrunk);

        pattern.localDevSrcIsTrunk   = GT_FALSE;
        pattern.localDevSrcPortTrunk = TTI_VALID_PHY_PORT_CNS;

        /*
            1.6. Call with patternPtr ->localDevSrcIsTrunk [GT_TRUE]
                           and patternPtr ->localDevSrcPortTrunk [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           and other params from 1.1.
            Expected: GT_OK.
        */
        pattern.localDevSrcIsTrunk   = GT_TRUE;
        pattern.localDevSrcPortTrunk = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, patternPtr->localDevSrcIsTrunk = %d, patternPtr->localDevSrcPortTrunk = %d",
                                                dev, pattern.localDevSrcIsTrunk,
                                                pattern.localDevSrcPortTrunk);

        pattern.localDevSrcIsTrunk   = GT_FALSE;
        pattern.localDevSrcPortTrunk = TTI_VALID_PHY_PORT_CNS;

        /*
            1.7. Call with out of range patternPtr -> vid [4096]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        pattern.vid = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, patternPtr->vid = %d", dev, pattern.vid);

        pattern.vid = 100;

        /*
            1.8. Call with out of range actionPtr-> passengerPacketType [0x5AAAAAA5]
                            action.tunnelTerminate [GT_TRUE]
                            and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        action.tunnelTerminate     = GT_TRUE;
        action.passengerPacketType = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "%d, actionPtr->tunnelTerminate = %d, actionPtr->passengerPacketType = %d",
                                     dev, action.tunnelTerminate, action.passengerPacketType);

        action.tunnelTerminate     = GT_FALSE;
        action.passengerPacketType = CPSS_DXCH3P_TTI_PASSENGER_CAPWAP_802_3_E;

        /*
            1.10. Call with out of range actionPtr-> vlanAssignment [0x5AAAAAA5]
                            and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        action.vlanAssignment = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, actionPtr->vlanAssignment = %d",
                                     dev, action.vlanAssignment);

        action.vlanAssignment = CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E;

        /*
            1.11. Call with out of range actionPtr-> vid [4096]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.vid = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->vid = %d",
                                         dev, action.vid);

        action.vid = 100;

        /*
            1.12. Call with out of range actionPtr-> unknownSACmd [0x5AAAAAA5]
                            and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        action.unknownSACmd = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, actionPtr->unknownSACmd = %d",
                                     dev, action.unknownSACmd);

        action.unknownSACmd = CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E;

        /*
            1.13. Call with not valid actionPtr-> unknownSACmd [CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.unknownSACmd = CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->unknownSACmd = %d",
                                     dev, action.unknownSACmd);

        action.unknownSACmd = CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E;

        /*
            1.14. Call with out of range actionPtr-> qosMode [0x5AAAAAA5]
                            and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        action.qosMode = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, actionPtr->qosMode = %d",
                                     dev, action.qosMode);

        action.qosMode = CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E;

        /*
            1.15. Call with out of range actionPtr-> qosTidMappingProfileTable [8]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.qosTidMappingProfileTable = 8;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->qosTidMappingProfileTable = %d",
                                         dev, action.qosTidMappingProfileTable);

        action.qosTidMappingProfileTable = 7;

        /*
            1.16. Call with out of range actionPtr-> srcVirtualPortAssignment.
                            srcDevId [PRV_CPSS_MAX_PP_DEVICES_CNS]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.srcVirtualPortAssignment.srcDevId = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->srcVirtualPortAssignment.srcDevId = %d",
                                         dev, action.srcVirtualPortAssignment.srcDevId);

        action.srcVirtualPortAssignment.srcDevId = 0;

        /*
            1.17. Call with out of range actionPtr-> srcVirtualPortAssignment.
                            srcPortId [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.srcVirtualPortAssignment.srcPortId = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->srcVirtualPortAssignment.srcPortId = %d",
                                         dev, action.srcVirtualPortAssignment.srcPortId);

        action.srcVirtualPortAssignment.srcPortId = 0;

        /*
            1.18. Call with not valid actionPtr->arpCmd [CPSS_DXCH3P_PACKET_CMD_ROUTE_E]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.arpCmd = CPSS_DXCH3P_PACKET_CMD_ROUTE_E;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->arpCmd = %d",
                                         dev, action.arpCmd);

        action.arpCmd = CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E;

        /*
            1.19. Call with out of range actionPtr->arpCmd [0x5AAAAAA5]
                            and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        action.arpCmd = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, actionPtr->arpCmd = %d",
                                                       dev, action.arpCmd);

        action.arpCmd = CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E;

        /*
            1.20. Call with out of range actionPtr-> srcId [32]
                    and other params from 1.1.
            Expected: NOT GT_OK.
        */
        action.srcId = 32;

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actionPtr->srcId = %d",
                                                     dev, action.srcId);

        action.srcId = 31;

        /*
            1.21. Call with patternPtr [NULL]
                            and other params from 1.18.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, NULL, &mask, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, patternPtr = NULL", dev);

        /*
            1.22. Call with maskPtr [NULL]
                            and other params from 1.18.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, NULL, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.23. Call with actionPtr [NULL]
                            and other params from 1.18.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actionPtr = NULL", dev);
    }

    pclCapwapRuleIndex = 0;

    keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

    prvSet_Default_Pattern(&pattern);
    prvSet_Default_Action(&action);

    cpssOsBzero((GT_VOID*) &(mask), sizeof(mask));
    mask.pclId = 0x000003ff;
    mask.localDevSrcIsTrunk = 0x00000001;
    mask.localDevSrcPortTrunk = 0x000000ff;
    mask.vid = 0x0fff;
    mask.srcIp.ipv4Addr.u32Ip = 0xffffffff;
    mask.destIp.ipv4Addr.u32Ip = 0xffffffff;
    mask.udpSrcPort = 0x0000ffff;
    mask.udpDestPort = 0x0000ffff;
    mask.isDtlsEncripted = 0x00000001;
    mask.capwapHeaderWord0 = 0xffffffff;
    mask.macAddr.arEther[0] = 0xff;
    mask.macAddr.arEther[1] = 0xff;
    mask.macAddr.arEther[2] = 0xff;
    mask.macAddr.arEther[3] = 0xff;
    mask.macAddr.arEther[4] = 0xff;
    mask.macAddr.arEther[5] = 0xff;
    mask.frameControlToDS = 0x00000001;
    mask.frameControlFromDS = 0x00000001;
    mask.frameControlType = 0x00000003;
    mask.frameControlSubtype = 0x0000000f;
    mask.isProtected = 0x00000001;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiCapwapRuleGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH3P_TTI_KEY_TYPE_ENT             keyType,
    IN  GT_U32                                   pclCapwapRuleIndex,
    OUT CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC  *patternPtr,
    OUT CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC  *maskPtr,
    OUT CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC        *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiCapwapRuleGet)
{
/*
    ITERATE_DEVICES (DXCH3P)
    1.1. Set Rule for testing. Call cpssDxCh3pTtiCapwapRuleSet with
                   pclCapwapRuleIndex[0],
                   keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                   patternPtr{ pclId[0],
                               localDevSrcIsTrunk[GT_FALSE],
                               localDevSrcPortTrunk[0],
                               vid[0],
                               srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
                               destIp{ ipv4Addr{ arIP[192.168.0.2] } },
                               udpSrcPort[9001],
                               udpDestPort[9002],
                               isDtlsEncripted[GT_TRUE],
                               capwapHeaderWord0[0xFFFFFFFF],
                               macAddr{ arEther[10, 20, 30, 40, 50, 60] },
                               frameControlToDS[0],
                               frameControlFromDS[0],
                               frameControlType[0],
                               frameControlSubtype[0]
                               isProtected[GT_TRUE]},
                   maskPtr[all bits set to 1],
                   actionPtr{
                            tunnelTerminate[GT_FALSE],
                            vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
                            vid[100],
                            rssiUpdateEnable[GT_FALSE],
                            bridgeAutoLearnEnable[GT_FALSE],
                            naToCPUEnable[GT_FALSE],
                            unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
                            qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
                            qosAtributes{qosProfile[0],up[0]},
                            qosTidMappingProfileTable[0],
                            srcVirtualPortAssignment{ srcDevId[0],
                                                      srcPortId[0] },
                            localSwitchingEnable[GT_FALSE],
                            arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
                            ieeeResMcCmdProfile[0],
                            srcId[31] }.
    Expected: GT_OK.
    1.2. Call with keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                    pclCapwapRuleIndex[0]
                    and not NULL patternPtr, maskPtr, actionPtr.
    Expected: GT_OK.
    1.3. Call with wrong keyType[CPSS_DXCH3P_TTI_KEY_IPV4_E]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with patternPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with maskPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with actionPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH3P_TTI_KEY_TYPE_ENT             keyType;
    GT_U32                                   pclCapwapRuleIndex;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC  pattern;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC  mask;
    CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC        action;


    keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

    pclCapwapRuleIndex = 0;

    cpssOsBzero((GT_VOID*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
    cpssOsBzero((GT_VOID*) &action, sizeof(action));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Set Rule for testing. Call cpssDxCh3pTtiCapwapRuleSet with
                           pclCapwapRuleIndex[0],
                           keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                           patternPtr{ pclId[0],
                                       localDevSrcIsTrunk[GT_FALSE],
                                       localDevSrcPortTrunk[0],
                                       vid[0],
                                       srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
                                       destIp{ ipv4Addr{ arIP[192.168.0.2] } },
                                       udpSrcPort[9001],
                                       udpDestPort[9002],
                                       isDtlsEncripted[GT_TRUE],
                                       capwapHeaderWord0[0xFFFFFFFF],
                                       macAddr{ arEther[10, 20, 30, 40, 50, 60] },
                                       frameControlToDS[0],
                                       frameControlFromDS[0],
                                       frameControlType[0],
                                       frameControlSubtype[0]
                                       isProtected[GT_TRUE]},
                           maskPtr[all bits set to 1],
                           actionPtr{
                                    tunnelTerminate[GT_FALSE],
                                    vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
                                    vid[100],
                                    rssiUpdateEnable[GT_FALSE],
                                    bridgeAutoLearnEnable[GT_FALSE],
                                    naToCPUEnable[GT_FALSE],
                                    unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
                                    qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
                                    qosAtributes{ qosProfile[0], up[0]} ,
                                    qosTidMappingProfileTable[0],
                                    srcVirtualPortAssignment{ srcDevId[0],
                                                              srcPortId[0] },
                                    localSwitchingEnable[GT_FALSE],
                                    arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
                                    ieeeResMcCmdProfile[0],
                                    srcId[31] }.
            Expected: GT_OK.
        */
        pclCapwapRuleIndex = 0;

        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        prvSet_Default_Pattern(&pattern);
        prvSet_Default_Action(&action);

        cpssOsMemSet((GT_VOID*) &(mask), 0xFF, sizeof(mask));

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiCapwapRuleSet: %d, %d, %d",
                                     dev, pclCapwapRuleIndex, keyType);

        /*
            1.2. Call with keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                            pclCapwapRuleIndex[0]
                            and not NULL patternPtr, maskPtr, actionPtr.
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        cpssOsBzero((GT_VOID*) &pattern, sizeof(pattern));
        cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
        cpssOsBzero((GT_VOID*) &action, sizeof(action));

        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, keyType, pclCapwapRuleIndex);

        /*
            1.3. Call with wrong keyType[CPSS_DXCH3P_TTI_KEY_IPV4_E]
                            and other params from 1.2.
            Expected: NOT GT_OK.
        */
        keyType = CPSS_DXCH3P_TTI_KEY_IPV4_E;

        cpssOsBzero((GT_VOID*) &pattern, sizeof(pattern));
        cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
        cpssOsBzero((GT_VOID*) &action, sizeof(action));

        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, &mask, &action);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, keyType);

        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        /*
            1.4. Call with patternPtr [NULL]
                           and other params from 1.2.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, NULL, &mask, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, patternPtr = NULL|", dev);

        /*
            1.5. Call with maskPtr [NULL]
                           and other params from 1.2.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, NULL, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.6. Call with actionPtr [NULL]
                           and other params from 1.2.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, &mask, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actionPtr = NULL", dev);
    }

    keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV6_E;
    pclCapwapRuleIndex = 0;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, &mask, &action);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiRuleCapwapActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              pclCapwapRuleIndex,
    IN  CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC   *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiRuleCapwapActionUpdate)
{
/*
    ITERATE_DEVICES (DXCH3P)
    1.1. Set Rule for testing. Call cpssDxCh3pTtiCapwapRuleSet with
                   pclCapwapRuleIndex[0],
                   keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                   patternPtr{ pclId[0],
                               localDevSrcIsTrunk[GT_FALSE],
                               localDevSrcPortTrunk[0],
                               vid[100],
                               srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
                               destIp{ ipv4Addr{ arIP[192.168.0.2] } },
                               udpSrcPort[9001],
                               udpDestPort[9002],
                               isDtlsEncripted[GT_TRUE],
                               capwapHeaderWord0[0xFFFFFFFF],
                               macAddr{ arEther[10, 20, 30, 40, 50, 60] },
                               frameControlToDS[0],
                               frameControlFromDS[0],
                               frameControlType[0],
                               frameControlSubtype[0]
                               isProtected[GT_TRUE]},
                   maskPtr[all bits set to 1],
                   actionPtr{
                            tunnelTerminate[GT_FALSE],
                            vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
                            vid[100],
                            rssiUpdateEnable[GT_FALSE],
                            bridgeAutoLearnEnable[GT_FALSE],
                            naToCPUEnable[GT_FALSE],
                            unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
                            qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
                            qosAtributes{qosProfile[0],up[0]} ,
                            qosTidMappingProfileTable[0],
                            srcVirtualPortAssignment{ srcDevId[0],
                                                      srcPortId[0] },
                            localSwitchingEnable[GT_FALSE],
                            arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
                            ieeeResMcCmdProfile[0],
                            srcId[31] }.
    Expected: GT_OK.
    1.2. Call with pclCapwapRuleIndex[0],
                   actionPtr{
                            tunnelTerminate[GT_TRUE],
                            passengerPacketType[CPSS_DXCH3P_TTI_PASSENGER_CAPWAP_802_11_E],
                            vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_MAC_SA_BASED_E], vid[100],
                            rssiUpdateEnable[GT_TRUE],
                            bridgeAutoLearnEnable[GT_TRUE],
                            naToCPUEnable[GT_TRUE],
                            unknownSACmd[CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E],
                            qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_UP_BASED_E],
                            qosAtributes{qosProfile[0], up[0]} ,
                            qosTidMappingProfileTable[7],
                            srcVirtualPortAssignment{ srcDevId[0],
                                                      srcPortId[0] },
                            localSwitchingEnable[GT_TRUE],
                            arpCmd[CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E],
                            ieeeResMcCmdProfile[0],
                            srcId[31] }
    Expected: GT_OK.
    1.3. Call cpssDxCh3pTtiCapwapRuleGet with keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                                              pclCapwapRuleIndex[0],
                                              and not NULL patternPtr, maskPtr, actionPtr.
    Expected: GT_OK and the same action as was set in 1.2.
    1.4. Call with actionPtr[NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL                                  isEqual = GT_FALSE;
    CPSS_DXCH3P_TTI_KEY_TYPE_ENT             keyType;
    GT_U32                                   pclCapwapRuleIndex = 0;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC  pattern;
    CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC  mask;
    CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC        action;
    CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC        actionGet;


    keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

    cpssOsBzero((GT_VOID*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));
    cpssOsBzero((GT_VOID*) &action, sizeof(action));
    cpssOsBzero((GT_VOID*) &actionGet, sizeof(actionGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Set Rule for testing. Call cpssDxCh3pTtiCapwapRuleSet with
                           pclCapwapRuleIndex[0],
                           keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                           patternPtr{ pclId[0],
                                       localDevSrcIsTrunk[GT_FALSE],
                                       localDevSrcPortTrunk[0],
                                       vid[0],
                                       srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
                                       destIp{ ipv4Addr{ arIP[192.168.0.2] } },
                                       udpSrcPort[9001],
                                       udpDestPort[9002],
                                       isDtlsEncripted[GT_TRUE],
                                       capwapHeaderWord0[0xFFFFFFFF],
                                       macAddr{ arEther[10, 20, 30, 40, 50, 60] },
                                       frameControlToDS[0],
                                       frameControlFromDS[0],
                                       frameControlType[0],
                                       frameControlSubtype[0]
                                       isProtected[GT_TRUE]},
                           maskPtr[all bits set to 1],
                           actionPtr{
                                    tunnelTerminate[GT_FALSE],
                                    vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
                                    vid[100],
                                    rssiUpdateEnable[GT_FALSE],
                                    bridgeAutoLearnEnable[GT_FALSE],
                                    naToCPUEnable[GT_FALSE],
                                    unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
                                    qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
                                    qosAtributes{qosProfile[0],up[0]},
                                    qosTidMappingProfileTable[0],
                                    srcVirtualPortAssignment{ srcDevId[0],
                                                              srcPortId[0] },
                                    localSwitchingEnable[GT_FALSE],
                                    arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
                                    ieeeResMcCmdProfile[0],
                                    srcId[31] }.
            Expected: GT_OK.
        */
        pclCapwapRuleIndex = 0;

        keyType = CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E;

        prvSet_Default_Pattern(&pattern);
        prvSet_Default_Action(&action);

        cpssOsMemSet((GT_VOID*) &(mask), 0xFF, sizeof(mask));

        st = cpssDxCh3pTtiCapwapRuleSet(dev, pclCapwapRuleIndex, keyType, &pattern, &mask, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiCapwapRuleSet: %d, %d, %d",
                                     dev, pclCapwapRuleIndex, keyType);
        /*
            1.2. Call with pclCapwapRuleIndex[0],
                           actionPtr{
                                    tunnelTerminate[GT_TRUE],
                                    passengerPacketType[CPSS_DXCH3P_TTI_PASSENGER_CAPWAP_802_11_E],
                                    vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_MAC_SA_BASED_E], vid[100],
                                    rssiUpdateEnable[GT_TRUE],
                                    bridgeAutoLearnEnable[GT_TRUE],
                                    naToCPUEnable[GT_TRUE],
                                    unknownSACmd[CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E],
                                    qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_UP_BASED_E],
                                    qosAtributes{qosProfile[0], up[7]},
                                    qosTidMappingProfileTable[7],
                                    srcVirtualPortAssignment{ srcDevId[0x1f],
                                                              srcPortId[0x3f] },
                                    localSwitchingEnable[GT_TRUE],
                                    arpCmd[CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E],
                                    ieeeResMcCmdProfile[0],
                                    srcId[31] }
            Expected: GT_OK.
        */
        action.tunnelTerminate = GT_TRUE;
        action.passengerPacketType = CPSS_DXCH3P_TTI_PASSENGER_CAPWAP_802_11_E; /* not relevant in this case */
        action.vlanAssignment  = CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_MAC_SA_BASED_E;
        action.vid = 100;
        action.rssiUpdateEnable = GT_TRUE;
        action.bridgeAutoLearnEnable = GT_TRUE;
        action.naToCPUEnable = GT_TRUE;
        action.unknownSACmd = CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E;
        action.qosMode = CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_UP_BASED_E;
        action.qosAtributes.qosProfile = 0;
        action.qosAtributes.up = 7;
        action.qosTidMappingProfileTable = 7;
        action.srcVirtualPortAssignment.srcDevId = 0x1f;
        action.srcVirtualPortAssignment.srcPortId = 0x3f;
        action.localSwitchingEnable = GT_TRUE;
        action.arpCmd = CPSS_DXCH3P_PACKET_CMD_MIRROR_TO_CPU_E;
        action.ieeeResMcCmdProfile = 0;
        action.srcId = 31;

        st = cpssDxCh3pTtiRuleCapwapActionUpdate(dev, pclCapwapRuleIndex, &action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pclCapwapRuleIndex);

        /*
            1.3. Call cpssDxCh3pTtiCapwapRuleGet with keyType[CPSS_DXCH3P_TTI_KEY_CAPWAP_OVER_IPV4_E],
                                                      pclCapwapRuleIndex[0],
                                                      and not NULL patternPtr, maskPtr, actionPtr.
            Expected: GT_OK and the same action as was set in 1.2.
        */
        st = cpssDxCh3pTtiCapwapRuleGet(dev, keyType, pclCapwapRuleIndex, &pattern, &mask, &actionGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiCapwapRuleGet: %d, %d, %d",
                                               dev, keyType, pclCapwapRuleIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &action, (GT_VOID*) &actionGet, sizeof(action) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another action than was set: %d", dev);

        /*
            1.4. Call with actionPtr[NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiRuleCapwapActionUpdate(dev, pclCapwapRuleIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actionPtr = NULL", dev);
    }

    pclCapwapRuleIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiRuleCapwapActionUpdate(dev, pclCapwapRuleIndex, &action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiRuleCapwapActionUpdate(dev, pclCapwapRuleIndex, &action);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH3P_TTI_EXCEPTION_ENT       exceptionType,
    IN  CPSS_DXCH3P_PACKET_CMD_ENT          command
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiExceptionCmdSet)
{
/*
    ITERATE_DEVICEX (DXCH3P)
    1.1. Call with exceptionType [CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                  CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E],
                   command [CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E /
                            CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E].
    Expected: GT_OK.
    1.2. Call cpssDxCh3pTtiExceptionCmdGet with not NULL commandPtr
                                                and other params from 1.1.
    Expected: GT_OK and command as was set.
    1.3. Call with command [CPSS_DXCH3P_PACKET_CMD_FORWARD_E] (not supported)
    Expected: NOT GT_OK.
    1.4. Call with out of range exceptionType [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range command [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH3P_TTI_EXCEPTION_ENT exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    CPSS_DXCH3P_PACKET_CMD_ENT    command       = CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E;
    CPSS_DXCH3P_PACKET_CMD_ENT    commandGet    = CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with exceptionType [CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                          CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E],
                           command [CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E /
                                    CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E].
            Expected: GT_OK.
        */
        /* iterate with exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E */
        exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
        command       = CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, exceptionType, command);

        /*
            1.2. Call cpssDxCh3pTtiExceptionCmdGet with not NULL commandPtr
                                                        and other params from 1.1.
            Expected: GT_OK and command as was set.
        */
        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiExceptionCmdGet: %d, %d",
                                                dev, exceptionType);

        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet, "got another command than was set: %d", dev);

        /* iterate with exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E */
        exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E;
        command       = CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E;

        st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, exceptionType, command);

        /*
            1.2. Call cpssDxCh3pTtiExceptionCmdGet with not NULL commandPtr
                                                        and other params from 1.1.
            Expected: GT_OK and command as was set.
        */
        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &commandGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTtiExceptionCmdGet: %d, %d",
                                                dev, exceptionType);

        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet, "got another command than was set: %d", dev);

        /*
            1.3. Call with command [CPSS_DXCH3P_PACKET_CMD_FORWARD_E] (not supported)
            Expected: NOT GT_OK.
        */
        command = CPSS_DXCH3P_PACKET_CMD_FORWARD_E;

        st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command = %d", dev, command);

        command = CPSS_DXCH3P_PACKET_CMD_DROP_HARD_E;

        /*
            1.4. Call with out of range exceptionType [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        exceptionType = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, exceptionType);

        exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;

        /*
            1.5. Call with out of range command [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        command = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, command = %d", dev, command);
    }

    exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    command       = CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiExceptionCmdSet(dev, exceptionType, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH3P_TTI_EXCEPTION_ENT       exceptionType,
    OUT CPSS_DXCH3P_PACKET_CMD_ENT          *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTtiExceptionCmdGet)
{
/*
    ITERATE_DEVICEX (DXCH3P)
    1.1. Call with exceptionType [CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                  CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E],
                                  not NULL commandPtr.
    Expected: GT_OK.
    1.2. Call with out of range exceptionType [0x5AAAAAA5]
                   and other param from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with commandPtr [NULL]
                   and other param from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH3P_TTI_EXCEPTION_ENT exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;
    CPSS_DXCH3P_PACKET_CMD_ENT    command       = CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with exceptionType [CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E /
                                          CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E],
                                          not NULL commandPtr.
            Expected: GT_OK.
        */
        /* iterate with exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E */
        exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;

        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, exceptionType);

        /* iterate with exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E */
        exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E;

        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, exceptionType);

        /*
            1.2. Call with out of range exceptionType [0x5AAAAAA5]
                           and other param from 1.1.
            Expected: GT_BAD_PARAM.
        */
        exceptionType = TTI_INVALID_ENUM_CNS;

        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, exceptionType);

        exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;

        /*
            1.3. Call with commandPtr [NULL]
                           and other param from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, commandPtr = NULL", dev);
    }

    exceptionType = CPSS_DXCH3P_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTtiExceptionCmdGet(dev, exceptionType, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxCh3pTti suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxCh3pTti)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiPortLookupEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiPortLookupEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiCapwapRuleSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiCapwapRuleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiRuleCapwapActionUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiExceptionCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTtiExceptionCmdGet)
UTF_SUIT_END_TESTS_MAC(cpssDxCh3pTti)

/*----------------------------------------------------------------------------*/
/* Set Pattern to default values */
static void prvSet_Default_Pattern
(
    OUT CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC *patternPtr
)
{
/*
patternPtr{ pclId[0],
           localDevSrcIsTrunk[GT_FALSE],
           localDevSrcPortTrunk[0],
           vid[100],
           srcIp{ ipv4Addr{ arIP[192.168.0.1] } },
           destIp{ ipv4Addr{ arIP[192.168.0.2] } },
           udpSrcPort[9001],
           udpDestPort[9002],
           isDtlsEncripted[GT_TRUE],
           capwapHeaderWord0[0xFFFFFFFF],
           macAddr{ arEther[10, 20, 30, 40, 50, 60] },
           frameControlToDS[1],
           frameControlFromDS[1],
           frameControlType[3],
           frameControlSubtype[15]
           isProtected[GT_TRUE] },
*/
    cpssOsBzero((GT_VOID*) patternPtr, sizeof(CPSS_DXCH3P_TTI_CAPWAP_OVER_IP_RULE_STC));

    patternPtr->pclId = 0;
    patternPtr->localDevSrcIsTrunk = GT_FALSE;
    patternPtr->localDevSrcPortTrunk = 0;
    patternPtr->vid = 100;

    patternPtr->srcIp.ipv4Addr.arIP[0] = 192;
    patternPtr->srcIp.ipv4Addr.arIP[1] = 168;
    patternPtr->srcIp.ipv4Addr.arIP[2] = 0;
    patternPtr->srcIp.ipv4Addr.arIP[3] = 1;

    patternPtr->destIp.ipv4Addr.arIP[0] = 192;
    patternPtr->destIp.ipv4Addr.arIP[1] = 168;
    patternPtr->destIp.ipv4Addr.arIP[2] = 0;
    patternPtr->destIp.ipv4Addr.arIP[3] = 2;

    patternPtr->udpSrcPort  = 8;
    patternPtr->udpDestPort = 24;
    patternPtr->isDtlsEncripted   = GT_TRUE;
    patternPtr->capwapHeaderWord0 = 0xFFFFFFFF;

    patternPtr->macAddr.arEther[0] = 0x10;
    patternPtr->macAddr.arEther[1] = 0x20;
    patternPtr->macAddr.arEther[2] = 0x30;
    patternPtr->macAddr.arEther[3] = 0x40;
    patternPtr->macAddr.arEther[4] = 0x50;
    patternPtr->macAddr.arEther[5] = 0x10;

    patternPtr->frameControlToDS    = 1;
    patternPtr->frameControlFromDS  = 1;
    patternPtr->frameControlType    = 3;
    patternPtr->frameControlSubtype = 15;
    patternPtr->isProtected         = GT_TRUE;
}

/*----------------------------------------------------------------------------*/
/* Set Action to default values */
static void prvSet_Default_Action
(
    OUT CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC *actionPtr
)
{
/*
actionPtr{
        tunnelTerminate[GT_FALSE],
        actionPtr->passengerPacketType = CPSS_DXCH3P_TTI_PASSENGER_CAPWAP_802_3_E; (not relevant in this case)
        vlanAssignment[CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E],
        vid[100],
        rssiUpdateEnable[GT_FALSE],
        bridgeAutoLearnEnable[GT_FALSE],
        naToCPUEnable[GT_FALSE],
        unknownSACmd[CPSS_DXCH3P_PACKET_CMD_FORWARD_E],
        qosMode[CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E],
        qosAtributes{ qosProfile[0], up[0]},
        qosTidMappingProfileTable[0],
        srcVirtualPortAssignment{ srcDevId[0],
                                  srcPortId[0] },
        localSwitchingEnable[GT_FALSE],
        arpCmd[CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E],
        ieeeResMcCmdProfile[0],
        srcId[31] }.
*/
    cpssOsBzero((GT_VOID*) actionPtr, sizeof(CPSS_DXCH3P_TTI_CAPWAP_ACTION_STC));

    actionPtr->tunnelTerminate = GT_FALSE;
    actionPtr->passengerPacketType = CPSS_DXCH3P_TTI_PASSENGER_CAPWAP_802_3_E; /* not relevant in this case */
    actionPtr->vlanAssignment  = CPSS_DXCH3P_TTI_CAPWAP_VLAN_ASSIGNMENT_BSSID_E;
    actionPtr->vid = 100;
    actionPtr->rssiUpdateEnable = GT_FALSE;
    actionPtr->bridgeAutoLearnEnable = GT_FALSE;
    actionPtr->naToCPUEnable = GT_FALSE;
    actionPtr->unknownSACmd = CPSS_DXCH3P_PACKET_CMD_FORWARD_E;
    actionPtr->qosMode = CPSS_DXCH3P_TTI_CAPWAP_QOS_MODE_TID_BASED_E;
    actionPtr->qosAtributes.qosProfile = 0;
    actionPtr->qosAtributes.up = 0;
    actionPtr->qosTidMappingProfileTable = 0;
    actionPtr->srcVirtualPortAssignment.srcDevId = 0;
    actionPtr->srcVirtualPortAssignment.srcPortId = 0;
    actionPtr->localSwitchingEnable = GT_FALSE;
    actionPtr->arpCmd = CPSS_DXCH3P_PACKET_CMD_TRAP_TO_CPU_E;
    actionPtr->ieeeResMcCmdProfile = 0;
    actionPtr->srcId = 31;
}

