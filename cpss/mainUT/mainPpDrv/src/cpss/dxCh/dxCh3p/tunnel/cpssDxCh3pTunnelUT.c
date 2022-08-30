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
* @file cpssDxCh3pTunnelUT.c
*
* @brief Unit tests for cpssDxCh3pTunnel, that provides
* CPSS Capwap tunnel declarations.
*
* @version   1.6
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxCh3p/tunnel/cpssDxCh3pTunnel.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
/* Invalid enum */
#define TUNNEL_INVALID_ENUM_CNS     0x5AAAAAA5

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTunnelStartEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_DXCH3P_TUNNEL_TYPE_ENT         tunnelType,
    IN  CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT *configPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTunnelStartEntrySet)
{
/*
    ITERATE_DEVICES (DxCh3p)

    {tunnelType=CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E}
    1.1. Call with routerArpTunnelStartLineIndex[0 / 1023],
                   tunnelType[CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E],
                   configPtr{ ipv4Cfg{
                                    tagEnable[GT_TRUE],
                                    vlanId[100 / 4095],
                                    upMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                    up[0 / 7],
                                    dscpMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                    dscp[0 / 63],
                                    macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                    dontFragmentFlag[GT_FALSE],
                                    ttl[0 / 255],
                                    autoTunnel[GT_FALSE / GT_TRUE],
                                    autoTunnelOffset[0 / 15],
                                    destIp{ arIP[192.168.0.1] },
                                    srcIp{ arIP[192.168.0.2] },
                                    cfi[0] } }
    Expected: GT_OK.
    1.2. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr and other params from 1.1.
                                                 (and the same for 1.14)
    Expected: GT_OK and same config as was set.
    1.3. Call with out of range routerArpTunnelStartLineIndex[1024]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range tunnelType [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with not valid tunnelType [CPSS_DXCH3P_TUNNEL_IP_OVER_X_E]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with configPtr-> ipv4Cfg.tagEnable[GT_TRUE],
                   configPtr-> ipv4Cfg.vlanId[4096]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with configPtr-> ipv4Cfg. tagEnable[GT_FALSE],
                   configPtr-> ipv4Cfg. vlanId[4096]
                   and other params from 1.1.
    Expected: GT_OK.
    1.8. Call with out of range configPtr-> ipv4Cfg. upMarkMode [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call with out of range configPtr-> ipv4Cfg. dscpMarkMode [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call with out of range configPtr-> ipv4Cfg. up [8]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.11. Call with out of range configPtr-> ipv4Cfg. dscp [64]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.12. Call with out of range configPtr-> ipv4Cfg. ttl [256]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.13. Call with out of range configPtr-> ipv4Cfg. autoTunnelOffset [16]
                    and other params from 1.1.
    Expected: NOT GT_OK.

    {tunnelType=CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E}
    1.14. Call with routerArpTunnelStartLineIndex[0 / 1023],
                    tunnelType[CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E],
                    configPtr{ mplsCfg{
                                        tagEnable[GT_TRUE],
                                        vlanId[100 / 4095],
                                        upMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                        up[0 / 7],
                                        macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                        numLabels[2], ttl[0 / 255], label1[1],
                                        exp1MarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                        exp1[0 / 7],
                                        label2[2],
                                        exp2MarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                        exp2[0 / 7],
                                        retainCRC[GT_TRUE],
                                        cfi[0] } }
    Expected: GT_OK.
    1.15. Call with configPtr-> mplsCfg. tagEnable[GT_TRUE],
                    configPtr-> mplsCfg. vlanId[4096]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.16. Call with configPtr-> mplsCfg. tagEnable[GT_FALSE],
                    configPtr-> mplsCfg. vlanId[4096]
                    and other params from 1.1.
    Expected: GT_OK.
    1.17. Call with out of range configPtr-> mplsCfg. upMarkMode [0x5AAAAAA5]
                    and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.18. Call with out of range configPtr-> mplsCfg. up [8]
                    and other params from 1.1.
    Expected: NOT GT_OK.
    1.19. Call with out of range configPtr-> mplsCfg. ttl [256]
                    and other params from 1.1.
    Expected: NOT GT_OK.

    {tunnelType=CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV4_E}
    1.20. Call with routerArpTunnelStartLineIndex[0 / 1023],
                    tunnelType[CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E],
                    configPtr{ ipCapwapCfg{ tunnelType [CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E],
                                            upMarkMode [CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                            up [0 / 7],
                                            tagEnable [GT_FALSE / GT_TRUE],
                                            vlanId [100 / 4095],
                                            ttl [0 / 255],
                                            capwapPayload [CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_802_3_FRAME_E / CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_NATIVE_WLAN_FRAME_E],
                                            wlanSpecInfoIsPresent [GT_FALSE / GT_TRUE],
                                            radioMacAddrIsPresent [GT_FALSE / GT_TRUE],
                                            macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                            wlanIdBitmap [100],
                                            dscp [63],
                                            dscpMarkMode [CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E / CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E],
                                            dontFragmentFlag [GT_FALSE / GT_TRUE],
                                            capwapVersion [0],
                                            capwapRid [0],
                                            capwapFlags [0],
                                            capwapWbid [0],
                                            enableWds [GT_FALSE / GT_TRUE],
                                            defaultEnable [GT_FALSE / GT_TRUE],
                                            mappingProfileTable [0],
                                            destIp {ipv6Addr {u32Ip {0, 0, 0, 0}}},
                                            srcIp {ipv6Addr {u32Ip {0, 0, 0, 0}}},
                                            udpSrcPort [0],
                                            udpDstPort [0],
                                            bssid { arEther[10, 20, 30, 40, 50, 60] },
                                            ra { arEther[10, 20, 30, 40, 50, 60] } }}
    Expected: GT_OK.
    1.21. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr.
    Expected: GT_OK and same config as was set.
    1.22. Call with tunnelType [0x5AAAAAA5] and other params from 1.20.
    Expected: GT_BAD_PARAM.
    1.23. Call with upMarkMode [0x5AAAAAA5] and other params from 1.20.
    Expected: GT_BAD_PARAM.
    1.24. Call with up [8] and other params from 1.20.
    Expected: NOT GT_OK.
    1.25. Call with vlanId [4096] and other params from 1.20.
    Expected: NOT GT_OK.
    1.26. Call with ttl [256] and other params from 1.20.
    Expected: NOT GT_OK.
    1.27. Call with capwapPayload [0x5AAAAAA5] and other params from 1.20.
    Expected: GT_BAD_PARAM.
    1.28. Call with wlanIdBitmap [BIT_16] and other params from 1.20.
    Expected: NOT GT_OK.
    1.29. Call with dscp [64] and other params from 1.20.
    Expected: NOT GT_OK.
    1.30. Call with dscpMarkMode [0x5AAAAAA5] and other params from 1.20.
    Expected: GT_BAD_PARAM.
    1.31. Call with capwapWbid [0x5AAAAAA5] and other params from 1.20.
    Expected: GT_BAD_PARAM.
    1.32. Call with mappingProfileTable [BIT_2] and other params from 1.20.
    Expected: NOT GT_OK.
    1.33. Call with udpSrcPort [BIT_16] and other params from 1.20.
    Expected: NOT GT_OK.
    1.34. Call with udpDstPort [BIT_16] and other params from 1.20.
    Expected: NOT GT_OK.

    1.35. Call with configPtr[NULL]
                    and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                              routerArpTunnelStartLineIndex;
    CPSS_DXCH3P_TUNNEL_TYPE_ENT         tunnelType;
    CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT config;
    CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT configGet;
    GT_BOOL                             isEqual;

    routerArpTunnelStartLineIndex = 0;

    tunnelType    = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;

    cpssOsBzero((GT_VOID*) &config, sizeof(config));
    cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            {tunnelType=CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E}
            1.1. Call with routerArpTunnelStartLineIndex[0 / 1023],
                           tunnelType[CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E],
                           configPtr{ ipv4Cfg{
                                            tagEnable[GT_TRUE],
                                            vlanId[100 / 4095],
                                            upMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                            up[0 / 7],
                                            dscpMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                            dscp[0 / 63],
                                            macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                            dontFragmentFlag[GT_FALSE],
                                            ttl[0 / 255],
                                            autoTunnel[GT_FALSE / GT_TRUE],
                                            autoTunnelOffset[0 / 15],
                                            destIp{ arIP[192.168.0.1] },
                                            srcIp{ arIP[192.168.0.2] },
                                            cfi[0] } }
            Expected: GT_OK.
        */
        /* iterate with routerArpTunnelStartLineIndex = 0 */
        routerArpTunnelStartLineIndex = 0;
        tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;
        config.ipv4Cfg.tagEnable = GT_TRUE;
        config.ipv4Cfg.vlanId = 100;
        config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up = 0;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp = 0;
        config.ipv4Cfg.macDa.arEther[0] = 10;
        config.ipv4Cfg.macDa.arEther[1] = 20;
        config.ipv4Cfg.macDa.arEther[2] = 30;
        config.ipv4Cfg.macDa.arEther[3] = 40;
        config.ipv4Cfg.macDa.arEther[4] = 50;
        config.ipv4Cfg.macDa.arEther[5] = 60;
        config.ipv4Cfg.dontFragmentFlag = GT_FALSE;
        config.ipv4Cfg.ttl = 0;
        config.ipv4Cfg.autoTunnel = GT_FALSE;
        config.ipv4Cfg.autoTunnelOffset = 0;
        config.ipv4Cfg.destIp.arIP[0] = 192;
        config.ipv4Cfg.destIp.arIP[1] = 168;
        config.ipv4Cfg.destIp.arIP[2] = 0;
        config.ipv4Cfg.destIp.arIP[3] = 1;
        config.ipv4Cfg.srcIp.arIP[0] = 192;
        config.ipv4Cfg.srcIp.arIP[1] = 168;
        config.ipv4Cfg.srcIp.arIP[2] = 0;
        config.ipv4Cfg.srcIp.arIP[3] = 2;
        config.ipv4Cfg.cfi = 0;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex, tunnelType);

        /*
            1.2. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr and other params from 1.1.
                                                         (and the same for 1.14)
            Expected: GT_OK and same config as was set.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &configGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                     dev, routerArpTunnelStartLineIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg, (GT_VOID*) &configGet.ipv4Cfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipv4Cfg than was set", dev);

        /* iterate with routerArpTunnelStartLineIndex = 1023 */
        routerArpTunnelStartLineIndex = 1023;
        tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;
        config.ipv4Cfg.tagEnable = GT_TRUE;
        config.ipv4Cfg.vlanId = 4095;
        config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up = 7;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp = 63;
        config.ipv4Cfg.macDa.arEther[0] = 10;
        config.ipv4Cfg.macDa.arEther[1] = 20;
        config.ipv4Cfg.macDa.arEther[2] = 30;
        config.ipv4Cfg.macDa.arEther[3] = 40;
        config.ipv4Cfg.macDa.arEther[4] = 50;
        config.ipv4Cfg.macDa.arEther[5] = 60;
        config.ipv4Cfg.dontFragmentFlag = GT_FALSE;
        config.ipv4Cfg.ttl = 255;
        config.ipv4Cfg.autoTunnel = GT_TRUE;
        config.ipv4Cfg.autoTunnelOffset = 15;
        config.ipv4Cfg.destIp.arIP[0] = 192;
        config.ipv4Cfg.destIp.arIP[1] = 168;
        config.ipv4Cfg.destIp.arIP[2] = 0;
        config.ipv4Cfg.destIp.arIP[3] = 1;
        config.ipv4Cfg.srcIp.arIP[0] = 192;
        config.ipv4Cfg.srcIp.arIP[1] = 168;
        config.ipv4Cfg.srcIp.arIP[2] = 0;
        config.ipv4Cfg.srcIp.arIP[3] = 2;
        config.ipv4Cfg.cfi = 0;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex, tunnelType);

        /*
            1.2. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr and other params from 1.1.
                                                         (and the same for 1.14)
            Expected: GT_OK and same config as was set.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &configGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                     dev, routerArpTunnelStartLineIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg, (GT_VOID*) &configGet.ipv4Cfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipv4Cfg than was set", dev);

        /*
            1.3. Call with out of range routerArpTunnelStartLineIndex[1024]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerArpTunnelStartLineIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArpAndTunnelStart;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex);

        routerArpTunnelStartLineIndex = 0;

        /*
            1.4. Call with out of range tunnelType [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        tunnelType = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, tunnelType = %d", dev, tunnelType);

        tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;

        /*
            1.5. Call with not valid tunnelType [CPSS_DXCH3P_TUNNEL_IP_OVER_X_E]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        tunnelType = CPSS_DXCH3P_TUNNEL_IP_OVER_X_E;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tunnelType = %d", dev, tunnelType);

        tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;

        /*
            1.6. Call with configPtr-> ipv4Cfg.vlanId[4096]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.ipv4Cfg.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, configPtr->ipv4Cfg.tagEnable = %d, configPtr->ipv4Cfg.vlanId = %d",
                                         dev, config.ipv4Cfg.tagEnable, config.ipv4Cfg.vlanId);

        config.ipv4Cfg.vlanId = 100;

        /*
            1.8. Call with out of range configPtr-> ipv4Cfg. upMarkMode [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        config.ipv4Cfg.upMarkMode = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv4Cfg.upMarkMode = %d",
                                     dev, config.ipv4Cfg.upMarkMode);

        config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;

        /*
            1.9. Call with out of range configPtr-> ipv4Cfg. dscpMarkMode [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        config.ipv4Cfg.dscpMarkMode = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv4Cfg.dscpMarkMode = %d",
                                     dev, config.ipv4Cfg.dscpMarkMode);

        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;

        /*
            1.10. Call with out of range configPtr-> ipv4Cfg. up [8]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.ipv4Cfg.up = 8;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->ipv4Cfg.up = %d",
                                         dev, config.ipv4Cfg.up);

        config.ipv4Cfg.up = 0;

        /*
            1.11. Call with out of range configPtr-> ipv4Cfg. dscp [64]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.ipv4Cfg.dscp = 64;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->ipv4Cfg.dscp = %d",
                                         dev, config.ipv4Cfg.dscp);

        config.ipv4Cfg.dscp = 0;

        /*
            1.12. Call with out of range configPtr-> ipv4Cfg. ttl [256]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.ipv4Cfg.ttl = 256;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->ipv4Cfg.ttl = %d",
                                         dev, config.ipv4Cfg.ttl);

        config.ipv4Cfg.ttl = 0;

        /*
            1.13. Call with out of range configPtr-> ipv4Cfg. autoTunnelOffset [16]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.ipv4Cfg.autoTunnelOffset = 16;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->ipv4Cfg.autoTunnelOffset = %d",
                                         dev, config.ipv4Cfg.autoTunnelOffset);

        config.ipv4Cfg.autoTunnelOffset = 0;

        /*
            {tunnelType=CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E}
            1.14. Call with routerArpTunnelStartLineIndex[0 / 1023],
                            tunnelType[CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E],
                            configPtr{ mplsCfg{
                                                tagEnable[GT_TRUE],
                                                vlanId[100 / 4095],
                                                upMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                                up[0 / 7],
                                                macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                                numLabels[2],
                                                ttl[0 / 255],
                                                label1[1],
                                                exp1MarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                                exp1[0 / 7],
                                                label2[2],
                                                exp2MarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                                exp2[0 / 7],
                                                retainCRC[GT_TRUE],
                                                cfi[0] } }
            Expected: GT_OK.
        */
        /* iterate with routerArpTunnelStartLineIndex = 0 */
        routerArpTunnelStartLineIndex = 0;
        tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E;
        config.mplsCfg.tagEnable = GT_TRUE;
        config.mplsCfg.vlanId = 100;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.up = 0;
        config.mplsCfg.macDa.arEther[0] = 10;
        config.mplsCfg.macDa.arEther[1] = 20;
        config.mplsCfg.macDa.arEther[2] = 30;
        config.mplsCfg.macDa.arEther[3] = 40;
        config.mplsCfg.macDa.arEther[4] = 50;
        config.mplsCfg.macDa.arEther[5] = 60;
        config.mplsCfg.numLabels = 2;
        config.mplsCfg.ttl = 0;
        config.mplsCfg.label1 = 1;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp1 = 0;
        config.mplsCfg.label2 = 2;
        config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp2 = 0;
        config.mplsCfg.retainCRC = GT_TRUE;
        config.mplsCfg.cfi = 0;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex, tunnelType);

        /* PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC
            1.2. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr and other params from 1.1.
                                                         (and the same for 1.14)
            Expected: GT_OK and same config as was set.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &configGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                     dev, routerArpTunnelStartLineIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg, (GT_VOID*) &configGet.ipv4Cfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipv4Cfg than was set", dev);

        /* iterate with routerArpTunnelStartLineIndex = 1023 */
        routerArpTunnelStartLineIndex = 1023;
        tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E;
        config.mplsCfg.tagEnable = GT_TRUE;
        config.mplsCfg.vlanId = 4095;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.up = 7;
        config.mplsCfg.macDa.arEther[0] = 10;
        config.mplsCfg.macDa.arEther[1] = 20;
        config.mplsCfg.macDa.arEther[2] = 30;
        config.mplsCfg.macDa.arEther[3] = 40;
        config.mplsCfg.macDa.arEther[4] = 50;
        config.mplsCfg.macDa.arEther[5] = 60;
        config.mplsCfg.numLabels = 2;
        config.mplsCfg.ttl = 255;
        config.mplsCfg.label1 = 1;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp1 = 7;
        config.mplsCfg.label2 = 2;
        config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp2 = 7;
        config.mplsCfg.retainCRC = GT_TRUE;
        config.mplsCfg.cfi = 0;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex, tunnelType);

        /* TTL_MAX_CNS
            1.2. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr and other params from 1.1.
                                                         (and the same for 1.14)
            Expected: GT_OK and same config as was set.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &configGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                     dev, routerArpTunnelStartLineIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg, (GT_VOID*) &configGet.ipv4Cfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipv4Cfg than was set", dev);

        /*
            1.15. Call with configPtr-> mplsCfg. tagEnable[GT_TRUE],
                            configPtr-> mplsCfg. vlanId[4096]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.mplsCfg.tagEnable = GT_TRUE;
        config.mplsCfg.vlanId    = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.tagEnable = %d, configPtr->mplsCfg.vlanId = %d",
                                         dev, config.mplsCfg.tagEnable, config.mplsCfg.vlanId);

        config.mplsCfg.tagEnable = GT_TRUE;
        config.mplsCfg.vlanId    = 100;

        /*
            1.17. Call with out of range configPtr-> mplsCfg. upMarkMode [0x5AAAAAA5]
                            and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        config.mplsCfg.upMarkMode = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.upMarkMode = %d",
                                     dev, config.mplsCfg.upMarkMode);

        config.mplsCfg.upMarkMode = 0;

        /*
            1.18. Call with out of range configPtr-> mplsCfg. up [8]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.mplsCfg.up = 8;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.up = %d",
                                     dev, config.mplsCfg.up);

        config.mplsCfg.up = 0;

        /*
            1.19. Call with out of range configPtr-> mplsCfg. ttl [256]
                            and other params from 1.1.
            Expected: NOT GT_OK.
        */
        config.mplsCfg.ttl = 256;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.ttl = %d",
                                     dev, config.mplsCfg.ttl);

        config.mplsCfg.ttl = 0;

        /*
            {tunnelType=CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV4_E}
            1.20. Call with routerArpTunnelStartLineIndex[0 / 1023],
                            tunnelType[CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E],
                            configPtr{ ipCapwapCfg{ tunnelType [CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E],
                                                    upMarkMode [CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                                    up [0 / 7],
                                                    tagEnable [GT_FALSE / GT_TRUE],
                                                    vlanId [100 / 4095],
                                                    ttl [0 / 255],
                                                    capwapPayload [CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_802_3_FRAME_E / CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_NATIVE_WLAN_FRAME_E],
                                                    wlanSpecInfoIsPresent [GT_FALSE / GT_TRUE],
                                                    radioMacAddrIsPresent [GT_FALSE / GT_TRUE],
                                                    macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                                    wlanIdBitmap [100],
                                                    dscp [63],
                                                    dscpMarkMode [CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E / CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E],
                                                    dontFragmentFlag [GT_FALSE / GT_TRUE],
                                                    capwapVersion [0],
                                                    capwapRid [0],
                                                    capwapFlags [0],
                                                    capwapWbid [0],
                                                    enableWds [GT_FALSE / GT_TRUE],
                                                    defaultEnable [GT_FALSE / GT_TRUE],
                                                    mappingProfileTable [0],
                                                    destIp {ipv6Addr {u32Ip {0, 0, 0, 0}}},
                                                    srcIp {ipv6Addr {u32Ip {0, 0, 0, 0}}},
                                                    udpSrcPort [0],
                                                    udpDstPort [0],
                                                    bssid { arEther[10, 20, 30, 40, 50, 60] },
                                                    ra { arEther[10, 20, 30, 40, 50, 60] } }}
            Expected: GT_OK.
        */
        /* iterate with routerArpTunnelStartLineIndex = 0 */
        routerArpTunnelStartLineIndex = 0;
        tunnelType = CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E;
        config.ipCapwapCfg.tunnelType = CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E;
        config.ipCapwapCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipCapwapCfg.up = 0;
        config.ipCapwapCfg.tagEnable = GT_FALSE;
        config.ipCapwapCfg.vlanId = 100;
        config.ipCapwapCfg.ttl = 0;
        config.ipCapwapCfg.capwapPayload = CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_802_3_FRAME_E;
        config.ipCapwapCfg.wlanSpecInfoIsPresent = GT_FALSE;
        config.ipCapwapCfg.radioMacAddrIsPresent = GT_FALSE;

        config.ipCapwapCfg.macDa.arEther[0] = 10;
        config.ipCapwapCfg.macDa.arEther[1] = 20;
        config.ipCapwapCfg.macDa.arEther[2] = 30;
        config.ipCapwapCfg.macDa.arEther[3] = 40;
        config.ipCapwapCfg.macDa.arEther[4] = 50;
        config.ipCapwapCfg.macDa.arEther[5] = 60;

        config.ipCapwapCfg.wlanIdBitmap = 100;
        config.ipCapwapCfg.dscp = 63;
        config.ipCapwapCfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipCapwapCfg.dontFragmentFlag = GT_FALSE;
        config.ipCapwapCfg.capwapVersion = 0;
        config.ipCapwapCfg.capwapRid = 0;
        config.ipCapwapCfg.capwapFlags = 0;
        config.ipCapwapCfg.capwapWbid = 0;
        config.ipCapwapCfg.enableWds = GT_FALSE;
        config.ipCapwapCfg.defaultEnable = GT_FALSE;
        config.ipCapwapCfg.mappingProfileTable = 0;

        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[0] = 0;
        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[1] = 0;
        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[2] = 0;
        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[3] = 0;

        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[0] = 0;
        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[1] = 0;
        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[2] = 0;
        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[3] = 0;

        config.ipCapwapCfg.udpSrcPort = 0;
        config.ipCapwapCfg.udpDstPort = 0;

        config.ipCapwapCfg.bssid.arEther[0] = 10;
        config.ipCapwapCfg.bssid.arEther[1] = 20;
        config.ipCapwapCfg.bssid.arEther[2] = 30;
        config.ipCapwapCfg.bssid.arEther[3] = 40;
        config.ipCapwapCfg.bssid.arEther[4] = 50;
        config.ipCapwapCfg.bssid.arEther[5] = 60;

        config.ipCapwapCfg.ra.arEther[0] = 10;
        config.ipCapwapCfg.ra.arEther[1] = 20;
        config.ipCapwapCfg.ra.arEther[2] = 30;
        config.ipCapwapCfg.ra.arEther[3] = 40;
        config.ipCapwapCfg.ra.arEther[4] = 50;
        config.ipCapwapCfg.ra.arEther[5] = 60;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex, tunnelType);

        /*
            1.21. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr.
            Expected: GT_OK and same config as was set.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &configGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                     dev, routerArpTunnelStartLineIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipCapwapCfg, (GT_VOID*) &configGet.ipCapwapCfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipCapwapCfg than was set", dev);

        /* iterate with routerArpTunnelStartLineIndex = 1023 */
        routerArpTunnelStartLineIndex = 0;
        tunnelType = CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E;
        config.ipCapwapCfg.tunnelType = CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E;
        config.ipCapwapCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.ipCapwapCfg.up = 7;
        config.ipCapwapCfg.tagEnable = GT_TRUE;
        config.ipCapwapCfg.vlanId = 4095;
        config.ipCapwapCfg.ttl = 255;
        config.ipCapwapCfg.capwapPayload = CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_NATIVE_WLAN_FRAME_E;
        config.ipCapwapCfg.wlanSpecInfoIsPresent = GT_TRUE;
        config.ipCapwapCfg.radioMacAddrIsPresent = GT_TRUE;

        config.ipCapwapCfg.macDa.arEther[0] = 10;
        config.ipCapwapCfg.macDa.arEther[1] = 20;
        config.ipCapwapCfg.macDa.arEther[2] = 30;
        config.ipCapwapCfg.macDa.arEther[3] = 40;
        config.ipCapwapCfg.macDa.arEther[4] = 50;
        config.ipCapwapCfg.macDa.arEther[5] = 60;

        config.ipCapwapCfg.wlanIdBitmap = 50;
        config.ipCapwapCfg.dscp = 0;
        config.ipCapwapCfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.ipCapwapCfg.dontFragmentFlag = GT_TRUE;
        config.ipCapwapCfg.capwapVersion = 0;
        config.ipCapwapCfg.capwapRid = 0;
        config.ipCapwapCfg.capwapFlags = 0;
        config.ipCapwapCfg.capwapWbid = 0;
        config.ipCapwapCfg.enableWds = GT_TRUE;
        config.ipCapwapCfg.defaultEnable = GT_TRUE;
        config.ipCapwapCfg.mappingProfileTable = 3;

        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[0] = 10;
        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[1] = 20;
        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[2] = 30;
        config.ipCapwapCfg.destIp.ipv6Addr.u32Ip[3] = 40;

        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[0] = 10;
        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[1] = 20;
        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[2] = 30;
        config.ipCapwapCfg.srcIp.ipv6Addr.u32Ip[3] = 40;

        config.ipCapwapCfg.udpSrcPort = 10;
        config.ipCapwapCfg.udpDstPort = 20;

        config.ipCapwapCfg.bssid.arEther[0] = 10;
        config.ipCapwapCfg.bssid.arEther[1] = 20;
        config.ipCapwapCfg.bssid.arEther[2] = 30;
        config.ipCapwapCfg.bssid.arEther[3] = 40;
        config.ipCapwapCfg.bssid.arEther[4] = 50;
        config.ipCapwapCfg.bssid.arEther[5] = 60;

        config.ipCapwapCfg.ra.arEther[0] = 10;
        config.ipCapwapCfg.ra.arEther[1] = 20;
        config.ipCapwapCfg.ra.arEther[2] = 30;
        config.ipCapwapCfg.ra.arEther[3] = 40;
        config.ipCapwapCfg.ra.arEther[4] = 50;
        config.ipCapwapCfg.ra.arEther[5] = 60;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex, tunnelType);

        /*
            1.21. Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr.
            Expected: GT_OK and same config as was set.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &configGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                     dev, routerArpTunnelStartLineIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipCapwapCfg, (GT_VOID*) &configGet.ipCapwapCfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipCapwapCfg than was set", dev);

        /*
            1.22. Call with tunnelType [0x5AAAAAA5] and other params from 1.20.
            Expected: GT_BAD_PARAM.
        */
        config.ipCapwapCfg.tunnelType = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, config.ipCapwapCfg.tunnelType = %d",
                                    dev, config.ipCapwapCfg.tunnelType);

        config.ipCapwapCfg.tunnelType = CPSS_DXCH3P_TUNNEL_CAPWAP_OVER_IPV6_E;

        /*
            1.23. Call with upMarkMode [0x5AAAAAA5] and other params from 1.20.
            Expected: GT_BAD_PARAM.
        */
        config.ipCapwapCfg.upMarkMode = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, config.ipCapwapCfg.upMarkMode = %d",
                                    dev, config.ipCapwapCfg.upMarkMode);

        config.ipCapwapCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

        /*
            1.24. Call with up [8] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.up = 8;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.up = %d",
                                         dev, config.ipCapwapCfg.up);

        config.ipCapwapCfg.up = 7;

        /*
            1.25. Call with vlanId [4096] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.vlanId = 4096;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.vlanId = %d",
                                         dev, config.ipCapwapCfg.vlanId);

        config.ipCapwapCfg.vlanId = 0;

        /*
            1.26. Call with ttl [256] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.ttl = 256;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.ttl = %d",
                                         dev, config.ipCapwapCfg.ttl);

        config.ipCapwapCfg.ttl = 0;

        /*
            1.27. Call with capwapPayload [0x5AAAAAA5] and other params from 1.20.
            Expected: GT_BAD_PARAM.
        */
        config.ipCapwapCfg.capwapPayload = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, config.ipCapwapCfg.capwapPayload = %d",
                                    dev, config.ipCapwapCfg.capwapPayload);

        config.ipCapwapCfg.capwapPayload = CPSS_DXCH3P_CAPWAP_GEN_PAYLOAD_FORMAT_802_3_FRAME_E;

        /*
            1.28. Call with wlanIdBitmap [BIT_16] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.wlanIdBitmap = BIT_16;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.wlanIdBitmap = %d",
                                         dev, config.ipCapwapCfg.wlanIdBitmap);

        config.ipCapwapCfg.wlanIdBitmap = 0;

        /*
            1.29. Call with dscp [64] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.dscp = 64;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.dscp = %d",
                                         dev, config.ipCapwapCfg.dscp);

        config.ipCapwapCfg.dscp = 0;

        /*
            1.30. Call with dscpMarkMode [0x5AAAAAA5] and other params from 1.20.
            Expected: GT_BAD_PARAM.
        */
        config.ipCapwapCfg.dscpMarkMode = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, config.ipCapwapCfg.dscpMarkMode = %d",
                                    dev, config.ipCapwapCfg.dscpMarkMode);

        config.ipCapwapCfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;

        /*
            1.31. Call with capwapWbid [0x5AAAAAA5] and other params from 1.20.
            Expected: GT_BAD_PARAM.
        */
        config.ipCapwapCfg.capwapWbid = TUNNEL_INVALID_ENUM_CNS;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, config.ipCapwapCfg.capwapWbid = %d",
                                    dev, config.ipCapwapCfg.capwapWbid);

        config.ipCapwapCfg.capwapWbid = 0;

        /*
            1.32. Call with mappingProfileTable [BIT_2] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.mappingProfileTable = BIT_2;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.mappingProfileTable = %d",
                                         dev, config.ipCapwapCfg.mappingProfileTable);

        config.ipCapwapCfg.mappingProfileTable = 0;

        /*
            1.33. Call with udpSrcPort [BIT_16] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.udpSrcPort = BIT_16;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.udpSrcPort = %d",
                                         dev, config.ipCapwapCfg.udpSrcPort);

        config.ipCapwapCfg.udpSrcPort = 0;

        /*
            1.34. Call with udpDstPort [BIT_16] and other params from 1.20.
            Expected: NOT GT_OK.
        */
        config.ipCapwapCfg.udpDstPort = BIT_16;

        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, config.ipCapwapCfg.udpDstPort = %d",
                                         dev, config.ipCapwapCfg.udpDstPort);

        config.ipCapwapCfg.udpDstPort = 0;

        /*
            1.35. Call with configPtr[NULL]
                            and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configPtr = NULL", dev);
    }

    routerArpTunnelStartLineIndex = 0;
    tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;
    config.ipv4Cfg.tagEnable = GT_TRUE;
    config.ipv4Cfg.vlanId = 100;
    config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.up = 0;
    config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.dscp = 0;
    config.ipv4Cfg.macDa.arEther[0] = 10;
    config.ipv4Cfg.macDa.arEther[1] = 20;
    config.ipv4Cfg.macDa.arEther[2] = 30;
    config.ipv4Cfg.macDa.arEther[3] = 40;
    config.ipv4Cfg.macDa.arEther[4] = 50;
    config.ipv4Cfg.macDa.arEther[5] = 60;
    config.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    config.ipv4Cfg.ttl = 0;
    config.ipv4Cfg.autoTunnel = GT_FALSE;
    config.ipv4Cfg.autoTunnelOffset = 0;
    config.ipv4Cfg.destIp.arIP[0] = 192;
    config.ipv4Cfg.destIp.arIP[1] = 168;
    config.ipv4Cfg.destIp.arIP[2] = 0;
    config.ipv4Cfg.destIp.arIP[3] = 1;
    config.ipv4Cfg.srcIp.arIP[0] = 192;
    config.ipv4Cfg.srcIp.arIP[1] = 168;
    config.ipv4Cfg.srcIp.arIP[2] = 0;
    config.ipv4Cfg.srcIp.arIP[3] = 2;
    config.ipv4Cfg.cfi = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTunnelStartEntrySet(dev, routerArpTunnelStartLineIndex, tunnelType, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pTunnelStartEntryGet
(
    IN   GT_U8                               devNum,
    IN   GT_U32                              routerArpTunnelStartLineIndex,
    IN   CPSS_DXCH3P_TUNNEL_TYPE_ENT         *tunnelTypePtr,
    OUT  CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT *configPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTunnelStartEntryGet)
{
/*
    ITERATE_DEVICES (DxCh3p)
    1.1. Call with routerArpTunnelStartLineIndex[0 / 1023],
                   tunnelTypePtr[CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E / CPSS_DXCH3P_TUNNEL_X_OVER_MPLS_E]
                   not NULL configPtr.
    Expected: GT_OK.
    1.2. Call with routerArpTunnelStartLineIndex[1024]
                   and other params from 1.1..
    Expected: NOT GT_OK.
    1.3. Call with out of range tunnelType [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with not valid tunnelType [CPSS_DXCH3P_TUNNEL_IP_OVER_X_E]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with configPtr[NULL]
                and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                              routerArpTunnelStartLineIndex;
    CPSS_DXCH3P_TUNNEL_TYPE_ENT         tunnelType;
    CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT config;


    routerArpTunnelStartLineIndex = 0;

    cpssOsBzero((GT_VOID*) &config, sizeof(config));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with routerArpTunnelStartLineIndex[0 / 1023],
                           not NULL tunnelTypePtr and configPtr.
            Expected: GT_OK.
        */
        /* iterate with routerArpTunnelStartLineIndex = 0 */
        routerArpTunnelStartLineIndex = 0;

        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex);

        /* iterate with routerArpTunnelStartLineIndex = 1023 */
        routerArpTunnelStartLineIndex = 1023;

        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex);

        /*
            1.2. Call with routerArpTunnelStartLineIndex[1024]
                           and other params from 1.1..
            Expected: NOT GT_OK.
        */
        routerArpTunnelStartLineIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArpAndTunnelStart;;

        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpTunnelStartLineIndex);

        routerArpTunnelStartLineIndex = 0;

        /*
            1.5. Call with configPtr[NULL]
                        and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configPtr = NULL", dev);
    }

    routerArpTunnelStartLineIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pTunnelStartEntryGet(dev, routerArpTunnelStartLineIndex, &tunnelType, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill tunnel start table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pTunnelFillTunnelStartTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_ARP_TUNNEL_START_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in TUNNEL_START table.
         Call cpssDxCh3pTunnelStartEntrySet with routerArpTunnelStartLineIndex[0..numEntries-1],
                                           tunnelType[CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E],
                                           configPtr{ ipv4Cfg{
                                                            tagEnable[GT_TRUE],
                                                            vlanId[100],
                                                            upMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                                            up[0],
                                                            dscpMarkMode[CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E],
                                                            dscp[0],
                                                            macDa{ arEther[10, 20, 30, 40, 50, 60] },
                                                            dontFragmentFlag[GT_FALSE],
                                                            ttl[0],
                                                            autoTunnel[GT_FALSE],
                                                            autoTunnelOffset[0],
                                                            destIp{ arIP[192.168.0.1] },
                                                            srcIp{ arIP[192.168.0.2] },
                                                            cfi[0] } }
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pTunnelStartEntrySet with routerArpTunnelStartLineIndex[numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in TUNNEL_START table and compare with original.
         Call cpssDxCh3pTunnelStartEntryGet with not NULL configPtr
                                                 and other params from 1.2.
    Expected: GT_OK and same config as was set.
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pTunnelStartEntryGet with routerArpTunnelStartLineIndex[numEntries]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual    = GT_FALSE;
    GT_U32      numEntries = 0;
    GT_U32      iTemp      = 0;

    CPSS_DXCH3P_TUNNEL_TYPE_ENT         tunnelType;
    CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT config;
    CPSS_DXCH3P_TUNNEL_START_CONFIG_UNT configGet;


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    tunnelType    = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;

    cpssOsBzero((GT_VOID*) &config, sizeof(config));
    cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

    /* Fill the entry for TUNNEL_START table */
    tunnelType = CPSS_DXCH3P_TUNNEL_X_OVER_IPV4_E;

    config.ipv4Cfg.tagEnable = GT_TRUE;
    config.ipv4Cfg.vlanId = 100;
    config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.up = 0;
    config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.dscp = 0;
    config.ipv4Cfg.macDa.arEther[0] = 10;
    config.ipv4Cfg.macDa.arEther[1] = 20;
    config.ipv4Cfg.macDa.arEther[2] = 30;
    config.ipv4Cfg.macDa.arEther[3] = 40;
    config.ipv4Cfg.macDa.arEther[4] = 50;
    config.ipv4Cfg.macDa.arEther[5] = 60;
    config.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    config.ipv4Cfg.ttl = 0;
    config.ipv4Cfg.autoTunnel = GT_FALSE;
    config.ipv4Cfg.autoTunnelOffset = 0;
    config.ipv4Cfg.destIp.arIP[0] = 192;
    config.ipv4Cfg.destIp.arIP[1] = 168;
    config.ipv4Cfg.destIp.arIP[2] = 0;
    config.ipv4Cfg.destIp.arIP[3] = 1;
    config.ipv4Cfg.srcIp.arIP[0] = 192;
    config.ipv4Cfg.srcIp.arIP[1] = 168;
    config.ipv4Cfg.srcIp.arIP[2] = 0;
    config.ipv4Cfg.srcIp.arIP[3] = 2;
    config.ipv4Cfg.cfi = 0;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_TUNNEL_START_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        /* 1.2. Fill all entries in TUNNEL_START table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pTunnelStartEntrySet(dev, iTemp, tunnelType, &config);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntrySet: %d, %d, %d",
                                         dev, iTemp, tunnelType);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pTunnelStartEntrySet(dev, numEntries, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntrySet: %d, %d, %d",
                                         dev, numEntries, tunnelType);

        /* 1.4. Read all entries in TUNNEL_START table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pTunnelStartEntryGet(dev, iTemp, &tunnelType, &configGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                         dev, iTemp);

            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg, (GT_VOID*) &configGet.ipv4Cfg, sizeof(config) )) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another configPtr->ipv4Cfg than was set", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pTunnelStartEntryGet(dev, numEntries, &tunnelType, &configGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pTunnelStartEntryGet: %d, %d",
                                         dev, numEntries);
    }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxCh3pTunnel suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxCh3pTunnel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTunnelStartEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTunnelStartEntryGet)
    /* Tests for Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pTunnelFillTunnelStartTable)
UTF_SUIT_END_TESTS_MAC(cpssDxCh3pTunnel)

