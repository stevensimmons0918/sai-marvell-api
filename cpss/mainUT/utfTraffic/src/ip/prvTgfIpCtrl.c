
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
* @file prvTgfIpCtrl.c
*
* @brief Tests include the IPV4/V6 UC/MC ip control configs
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfBasicIpv4McRouting.h>
#include <gtOs/gtOsMem.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <bridge/prvTgfBrgGen.h>
#include <bridge/prvTgfBrgSrcId.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       0
#define PRV_TGF_TX_PORT_IDX_CNS       1
#define PRV_TGF_ROUTE_TARGET_EPG_CNS 0x10
#define PRV_TGF_ROUTER_NH_INDEX_CNS 1
static struct
{
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT nhMuxMode;
}prvTgfRestore;

/**
* @internal prvTgfIpNhMuxModeConfig function
* @endinternal
*
* @brief  Configure the VLANs and add port
*           enable IP UC on vlan and ingress port
*           Add mac2me entry
*           Add NH entry with EPG bits
*           Add EPCL rule to match target EPG and action to modify DSCP
*/
GT_VOID prvTgfIpNhMuxModeConfig()
{
    GT_STATUS rc;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC ipUcRouteEntry;

    /* Set configuration */
    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();
    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();
    /* Set FDB Route configuration */
    prvTgfFdbBasicIpv4UcRoutingRouteConfigurationSet(GT_FALSE);

    /*Config the NH mux mode to UC EPG*/
    rc = cpssDxChIpNhMuxModeGet(prvTgfDevNum, &prvTgfRestore.nhMuxMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpNhMuxModeGet");
    rc = cpssDxChIpNhMuxModeSet(prvTgfDevNum, CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpNhMuxModeSet");
    
    /* Set LPM Route configuration */
    prvTgfScalableSgtIpv4UcLpmRouteConfigSet();
    
    /*modify the route entry, update the target EPG value*/
    ipUcRouteEntry.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
    rc = cpssDxChIpUcRouteEntriesRead(prvTgfDevNum, PRV_TGF_ROUTER_NH_INDEX_CNS, &ipUcRouteEntry, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpUcRouteEntriesRead");

    ipUcRouteEntry.entry.regularEntry.targetEpg = PRV_TGF_ROUTE_TARGET_EPG_CNS;
    ipUcRouteEntry.entry.regularEntry.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    rc = cpssDxChIpUcRouteEntriesWrite(prvTgfDevNum, PRV_TGF_ROUTER_NH_INDEX_CNS, &ipUcRouteEntry, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpUcRouteEntriesRead");

    /*config the EPCL rule to match target EPG*/
    prvTgfBrgFdbEpgEpclConfig(0,PRV_TGF_ROUTE_TARGET_EPG_CNS);
}

/**
* @internal prvTgfIpNhMuxModeConfigRestore function
* @endinternal
*
* @brief  Restore all config done for target EPG from Router
*/
GT_VOID prvTgfIpNhMuxModeConfigRestore()
{
    GT_STATUS rc;
    /* Restore LPM configuration */
    prvTgfScalableSgtIpv4UcLpmRouteConfigReset();
    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();
    /* Restore routing configuration */
    prvTgfFdbBasicIpv4UcRoutingConfigurationRestore();
    /*restore EPG mux mode*/
    rc = cpssDxChIpNhMuxModeSet(prvTgfDevNum, prvTgfRestore.nhMuxMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpNhMuxModeSet");
    /*restore EPCL*/
    prvTgfBrgFdbEpgEpclConfigRestore();
}

/**
* @internal prvTgfIpNhMuxModeTraffic function
* @endinternal
*
* @brief   
*          Generate traffic and test the egress packet DSCP as set by 
*          the EPCL group ID indices.
*/
GT_VOID prvTgfIpNhMuxModeTraffic()
{
    prvTgfBrgFdbEpgTrafficGenerate();
}
