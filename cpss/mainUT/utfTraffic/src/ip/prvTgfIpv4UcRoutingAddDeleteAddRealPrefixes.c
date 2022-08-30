/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfIpv4UcRoutingAddDeleteRealPrefixes.c
*
* DESCRIPTION:
*       IPV4 UC Routing adding, deleting and the adding again prefixes into the Lpm
*       using real costumer prefixes defined in a file.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpv4UcRoutingAddMany.h>
#include <trunk/prvTgfTrunk.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <ip/prvTgfIpv4UcRoutingAddManyIpLpmMng.h>

extern GT_U8 prefixesOctetsAndLenRepresentation[];
extern GT_U32 prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet(GT_VOID);
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* number of prefixed actually added and need to be deleted */
static GT_U32       numOfExistingPrefixesToDel=0;
static GT_U32       numOfElementsInDelArray=0;
static GT_U32       numOfElementsInSendTrafficArray=0;

/* number of prefixes expected to be added in a device with 320K/120K LPM lines
   blocksAllocationMethod = do_not_share/share and PBR = 0/not 0
   and we add 100,000 from the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS       100000

#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_CNS 494314
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_CNS 15560
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_CNS 287448

#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_CNS 100000

#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_AC5P_CNS 211579

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add, delete, add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationAndTrafficGenerate(GT_VOID)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32                                  i=0;
    GT_IPADDR                               ipPrefixAddr;
    GT_U32                                  prefixLen=0;
    GT_U32                                  numOfPrefixesAdded=0;
    GT_U32                                  numOfPrefixesExpected=0;
    GT_U32                                  numOfPacketsSent=0;
    GT_U32                                  numOfElementsInArray=0;
    GT_U16                                  nextHopVlanId;
    GT_U8                                   tagArray[] = {0, 0, 0, 1};
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;

    GT_BOOL                                 defragmentationEnable=GT_TRUE;
    GT_U32                                  numEntries = 0;
    PRV_TGF_IP_CAPACITY_TESTS_ENT           testNum = PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigGet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  prvTgfIpLpmIpv4UcPrefixAdd\n "
                     "This function tries to add many IPv4 Unicast prefixes according to file ====\n");

   /* fill a nexthope info for the prefix routeEntryBaseIndex=port_2 */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsExpectedValueGet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, &numOfPrefixesExpected);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsForCurrentTestGet Failed");

        numOfElementsInArray=(prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet()/5);
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
#ifdef GM_USED /*GM do not support new memory banks logic */
        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_CNS;
        numOfElementsInArray = numOfPrefixesExpected;
#else

#if 0
    /* once the prefixesOctetsAndLenRepresentation will be updated to have more entries to fit the bigger size of banks supported in falcon
       then the following code will be correct. meanwhile we will be expected numOfElementsInArray */
        switch (ramDbCfg.sharedMemCnfg)
        {
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_CNS;
            break;
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_CNS;
            break;
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_CNS;
            break;
        default:
            rc = GT_NOT_SUPPORTED;
            break;
        }
#endif
        numOfElementsInArray=(prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet()/5);
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_AC5P_CNS;
        }
        else
        {
            numOfPrefixesExpected = numOfElementsInArray;/* should be removed once more prefixes in supported in prefixesOctetsAndLenRepresentation*/
        }
#endif
    }
    else
    {
        if(ramDbCfg.blocksSizeArray[0]==(_16K*4))
        {
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS;
        }
        else if (ramDbCfg.blocksSizeArray[0]==(_8K*4))
        {
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS/2;
        }
        else if (ramDbCfg.blocksSizeArray[0]==(_6K*4))
        {
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS/2;
        }
        else if (ramDbCfg.blocksSizeArray[0]==(_2K*4))
        {
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS/8;
        }
        else
        {
            /* the test do not supported at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test do not supported at the moment any other configuration: %d", prvTgfDevNum);
        }
        numOfElementsInArray = numOfPrefixesExpected;
    }


    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);

    /* start from index 0 in the prefixesOctetsAndLenRepresentation array */
    for(i=0;i<(numOfElementsInArray*5); i=i+5)
    {
        ipPrefixAddr.arIP[0]= prefixesOctetsAndLenRepresentation[i];
        ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];
        prefixLen = prefixesOctetsAndLenRepresentation[i+4];

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0,
                                        ipPrefixAddr, prefixLen, &nextHopInfo, GT_FALSE, defragmentationEnable);
        if ((rc!=GT_OK)&&(rc!=GT_ALREADY_EXIST))
        {
            cpssOsPrintf("\n could not add ipPrefixAddr = %d.%d.%d.%d/%d \n",ipPrefixAddr.arIP[0],
                   ipPrefixAddr.arIP[1],ipPrefixAddr.arIP[2],ipPrefixAddr.arIP[3],prefixLen);
            cpssOsPrintf("number of prefixes added: %d\n",numOfPrefixesAdded);
            break;
        }

        if(rc!=GT_ALREADY_EXIST)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
            numOfPrefixesAdded++;
        }

        /* check validiaty after every 10,000 prefixes */
        if (numOfPrefixesAdded%10000==0)
        {
            cpssOsPrintf(".");

            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }

    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsObservedValueSet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsObservedValueSet Failed");
    }

    if(numOfPrefixesAdded < numOfPrefixesExpected)
    {
        /* check if we get the correct number of prefixed added */
        UTF_VERIFY_EQUAL2_STRING_MAC( numOfPrefixesExpected, numOfPrefixesAdded,
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfPrefixesAdded,numOfPrefixesExpected);
    }

    numOfExistingPrefixesToDel = numOfPrefixesAdded;
    numOfElementsInDelArray = i/5;
    numOfElementsInSendTrafficArray = i/5;


    /* -------------------------------------------------------------------------
     * 2. send packet to ALL prefixes added
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = CPSS_IP_CNT_SET0_E;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;

    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[3];

    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* send traffic */
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
       /* since in SIP6 we have a big data list and a few configuration of the memory,
        the array contain multiple appearance of prefixes with different prefix length,
        we should send traffic according to the number of prefixes we added,
        by looking on the i we got to in previous add operation */
        numOfElementsInArray = numOfElementsInSendTrafficArray;
    }

    rc = cpssDxChCfgTableNumEntriesGet(prvTgfDevNum,CPSS_DXCH_CFG_TABLE_VLAN_E, &numEntries);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    if(numEntries > _4K) {
        numEntries = _4K;
    }


    for(i=0;i<(numOfElementsInArray*5);i=i+505)
    {
        ipPrefixAddr.arIP[0] = prefixesOctetsAndLenRepresentation[i];
        ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];

        /* AUTODOC: add UC route entry with nexthop VLAN i and nexthop port 3 */
        nextHopVlanId = (i % numEntries); /* 12 bits */
        if (nextHopVlanId == 5 || nextHopVlanId == 0) /* in vlan 5 the ports are untagged */
        {
            nextHopVlanId = 1;
        }
        routeEntriesArray[0].nextHopVlanId = nextHopVlanId;
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        rc = prvTgfBrgDefVlanEntryWithPortsSet(nextHopVlanId, prvTgfPortsArray + 3, NULL, tagArray + 3, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the added prefix */
        prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, nextHopVlanId);
        numOfPacketsSent++;
        cpssOsPrintf(".");
    }
    cpssOsPrintf("\n numOfPacketsSent=%d \n", numOfPacketsSent);
}


/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] deleteFromTop            - delete the prefixes from bottom/top
*                                       None
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationRestore
(
    IN GT_BOOL deleteFromTop
)
{
    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    GT_STATUS   rc = GT_OK;
    GT_U32       i=0;
    GT_IPADDR  ipPrefixAddr;
    GT_U32     prefixLen=0;
    GT_U32     numOfPrefixesDeleted=0;
    GT_U32     numOfPacketsSent=0;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32     numOfElementsInArray=0;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    GT_U8                                   tagArray[] = {0, 0, 0, 1};
    GT_U16                                  nextHopVlanId;
    GT_U32                                  numEntries = 0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                               tcamRowIndex;
    GT_U32                               tcamColumnIndex;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigGet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  prvTgfIpLpmIpv4UcPrefixDel\n "
                     "This function tries to delete IPv4 Unicast prefixes added according to a file ====\n");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        numOfElementsInArray = numOfElementsInDelArray;
    }
    else
    {
        if(ramDbCfg.blocksSizeArray[0]==(_16K*4))
        {
            numOfElementsInArray = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS;
        }
        else if (ramDbCfg.blocksSizeArray[0]==(_8K*4))
        {
            numOfElementsInArray = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS/2;
        }
        else if (ramDbCfg.blocksSizeArray[0]==(_6K*4))
        {
            numOfElementsInArray = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS/2;
        }
        else if (ramDbCfg.blocksSizeArray[0]==(_2K*4))
        {
            numOfElementsInArray = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_CNS/8;
        }
        else
        {
            /* the test do not supported at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test do not supported at the moment any other configuration: %d", prvTgfDevNum);
        }
    }

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = CPSS_IP_CNT_SET0_E;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;

    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[3];

    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* disable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_FALSE);

    if (deleteFromTop == GT_TRUE)
    {
        rc = cpssDxChCfgTableNumEntriesGet(prvTgfDevNum, CPSS_DXCH_CFG_TABLE_VLAN_E, &numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

        if(numEntries > _4K) {
            numEntries = _4K;
        }

        /* start from index 0 in the prefixesOctetsAndLenRepresentation array */
        /* send traffic to 1000 prefixes only */
        for (i=0; i<(numOfElementsInArray*5); i= i+5)
        {
            ipPrefixAddr.arIP[0] = prefixesOctetsAndLenRepresentation[i];
            ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
            ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
            ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];
            prefixLen = prefixesOctetsAndLenRepresentation[i+4];

            /* send trafic */
            if (i%100==0)
            {
                /* AUTODOC: add UC route entry with nexthop VLAN i and nexthop port 3 */
                nextHopVlanId = (i % numEntries); /* 12 bits */
                if (nextHopVlanId == 5 || nextHopVlanId == 0) /* in vlan 5 the ports are untagged */
                {
                    nextHopVlanId = 1;
                }
                routeEntriesArray[0].nextHopVlanId = nextHopVlanId;
                rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

                /* AUTODOC: create VLAN with tagged port 3 */
                rc = prvTgfBrgDefVlanEntryWithPortsSet(nextHopVlanId, prvTgfPortsArray + 3, NULL, tagArray + 3, 1);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

                if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                {
                    /* since the prvTgfIpv4UcRoutingAddRealPrefixes_data file contain prefixes that appear more
                       then once we can have a case that we get to a prefix that was already deleted,
                       so the traffic will fail. in this case we need to check if the prefix exist before sending the traffic  */

                    rc = prvTgfIpLpmIpv4UcPrefixSearch(prvTgfLpmDBId, 0, ipPrefixAddr, prefixLen, &nextHopInfo,&tcamRowIndex,&tcamColumnIndex);

                    if (rc==GT_OK)
                    {
                        /* AUTODOC: send packet to the added prefix */
                        prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, nextHopVlanId);
                        numOfPacketsSent++;
                        cpssOsPrintf(".");
                    }
                }
                else
                {
                    /* AUTODOC: send packet to the added prefix */
                    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, nextHopVlanId);
                    numOfPacketsSent++;
                    cpssOsPrintf(".");
                }

                /* untag */
                (void)prvTgfBrgDefVlanEntryWithPortsSet(nextHopVlanId, prvTgfPortsArray + 3, NULL, tagArray, 1);
                if(nextHopVlanId != 1)
                {
                    /* remove unneeded vlan */
                    (void)prvTgfBrgVlanMemberRemove(prvTgfDevNum, nextHopVlanId, prvTgfPortsArray[3]);
                    (void)prvTgfBrgVlanEntryInvalidate(nextHopVlanId);
                }
            }

            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipPrefixAddr, prefixLen);

            if((rc!=GT_OK)&&(rc!=GT_NOT_FOUND))
            {
                cpssOsPrintf("\n could not delete ipPrefixAddr = %d.%d.%d.%d/%d \n",ipPrefixAddr.arIP[0],
                   ipPrefixAddr.arIP[1],ipPrefixAddr.arIP[2],ipPrefixAddr.arIP[3],prefixLen);
                cpssOsPrintf("number of prefixes deleted: %d\n",numOfPrefixesDeleted);
                break;
            }
            if (rc!=GT_NOT_FOUND)
            {
                numOfPrefixesDeleted++;
            }

            /* check validiaty after every 10,000 of the prefixes added */
            if (i%10000==0)
            {
                /* enable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_TRUE);

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                 /* disable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_FALSE);
            }
        }
    }
    else
    {
        rc = cpssDxChCfgTableNumEntriesGet(prvTgfDevNum, CPSS_DXCH_CFG_TABLE_VLAN_E, &numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

        if(numEntries > _4K) {
            numEntries = _4K;
        }

        /* start from index 100,000 in the prefixesOctetsAndLenRepresentation array */
        for (i = (numOfElementsInArray*5 - 5); ; i = i - 5)
        {
            ipPrefixAddr.arIP[0] = prefixesOctetsAndLenRepresentation[i];
            ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
            ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
            ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];
            prefixLen = prefixesOctetsAndLenRepresentation[i+4];

            if (i%100==0)
            {
                /* AUTODOC: add UC route entry with nexthop VLAN i and nexthop port 3 */
                nextHopVlanId = (i % numEntries); /* 12 bits */
                if (nextHopVlanId == 5 || nextHopVlanId == 0) /* in vlan 5 the ports are untagged */
                {
                    nextHopVlanId = 1;
                }
                routeEntriesArray[0].nextHopVlanId = nextHopVlanId;
                rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

                /* AUTODOC: create VLAN with tagged port 3 */
                rc = prvTgfBrgDefVlanEntryWithPortsSet(nextHopVlanId, prvTgfPortsArray + 3, NULL, tagArray + 3, 1);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

                if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                {
                    /* since the prvTgfIpv4UcRoutingAddRealPrefixes_data file contain prefixes that appear more
                       then once we can have a case that we get to a prefix that was already deleted,
                       so the traffic will fail. in this case we need to check if the prefix exist before sending the traffic  */

                    rc = prvTgfIpLpmIpv4UcPrefixSearch(prvTgfLpmDBId, 0, ipPrefixAddr, prefixLen, &nextHopInfo,&tcamRowIndex,&tcamColumnIndex);

                    if (rc==GT_OK)
                    {
                        /* AUTODOC: send packet to the added prefix */
                        prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, nextHopVlanId);
                        numOfPacketsSent++;
                        cpssOsPrintf(".");
                    }
                }
                else
                {
                    /* AUTODOC: send packet to the added prefix */
                    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, nextHopVlanId);
                    numOfPacketsSent++;
                    cpssOsPrintf(".");
                }

                /* untag */
                (void)prvTgfBrgDefVlanEntryWithPortsSet(nextHopVlanId, prvTgfPortsArray + 3, NULL, tagArray, 1);
                if(nextHopVlanId != 1)
                {
                    /* remove unneeded vlan */
                    (void)prvTgfBrgVlanMemberRemove(prvTgfDevNum, nextHopVlanId, prvTgfPortsArray[3]);
                    (void)prvTgfBrgVlanEntryInvalidate(nextHopVlanId);
                }
            }

            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipPrefixAddr, prefixLen);
            if((rc!=GT_OK)&&(rc!=GT_NOT_FOUND))
            {
                cpssOsPrintf("\n could not delete ipPrefixAddr = %d.%d.%d.%d/%d \n",ipPrefixAddr.arIP[0],
                   ipPrefixAddr.arIP[1],ipPrefixAddr.arIP[2],ipPrefixAddr.arIP[3],prefixLen);
                cpssOsPrintf("number of prefixes deleted: %d\n",numOfPrefixesDeleted);
                break;
            }
            if (rc!=GT_NOT_FOUND)
            {
                numOfPrefixesDeleted++;
            }

            /* check validiaty after every 10,000 of the prefixes added */
            if (i%10000==0)
            {
                /* enable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_TRUE);

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                 /* disable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_FALSE);
            }
            /* end of the loop */
            if(i==0)
                break;

        }
    }

    cpssOsPrintf("\n numOfPacketsSent=%d \n",numOfPacketsSent);

    if(numOfPrefixesDeleted < numOfExistingPrefixesToDel)
    {
        /* check if we get the correct number of prefixed deleted */
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfExistingPrefixesToDel , numOfPrefixesDeleted,
                                  "======= numOfExistingPrefixesToDel[%d] is different then numOfPrefixesDeleted value [%d] =======\n",
                                 numOfExistingPrefixesToDel,numOfPrefixesDeleted);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

     /* enable LPM validity check */
     prvTgfIpValidityCheckEnable(GT_TRUE);
}


