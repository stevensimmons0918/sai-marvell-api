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
* @file prvTgfIpv6UcRoutingAddManyIpLpmMng.c
*
* @brief IPV6 UC Routing when filling the Lpm using cpssDxChIpLpmIpv6UcPrefixAddManyByOctet.
*
* @version   1
********************************************************************************
*/

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
#include <ip/prvTgfIpv6UcRoutingAddMany.h>
#include <ip/prvTgfIpv6UcRoutingAddManyIpLpmMng.h>
#include <ip/prvTgfIpv4UcRoutingAddManyIpLpmMng.h>
#include <trunk/prvTgfTrunk.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* number of prefixed actually added and need to be deleted */
static GT_U32  numOfExistingPrefixesToDel=0;
static GT_U32  numOfExistingPrefixesAddForBulk=0;

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 70313
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 69885 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS 70313

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 302077
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 302333 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS     302077

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 21582
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 21245 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS     21582

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 103677
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 103933 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS         103677

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
/* expected value when SWAP is used at the beginning of memory allocation     --> 1955
   expected value when SWAP is used before allocating a new memory  block     --> 1533
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 1955
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 1533 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS     1955

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
/* expected value when SWAP is used at the beginning of memory allocation     --> 28067
   expected value when SWAP is used before allocating a new memory  block     --> 27901
   expected value when 2 SWAP areas is in used                                --> 27389
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 27901 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS     27389

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS    30889

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0  */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS        144125


#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS      764705
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_ALLOW_SHARING_CNS          1678079
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS      3080
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_ALLOW_SHARING_CNS          42495
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_NOT_ALLOW_SHARING_CNS                   383731 /*with merge*/ /* 380659 - without merge */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_ALLOW_SHARING_CNS                       723720

#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_NOT_ALLOW_SHARING_CNS                       100000
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_ALLOW_SHARING_CNS                           100000
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_NOT_ALLOW_SHARING_CNS                     274848
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_ALLOW_SHARING_CNS                         520456


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationAndTrafficGenerate
(
        PRV_TGF_IP_CAPACITY_TESTS_ENT testNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR   startIpAddr;
    GT_U32      numOfPrefixesToAdd;
    GT_U32      firstOctetToChange;
    GT_U32      secondOctetToChange;
    GT_U32      thirdOctetToChange;
    GT_U32      forthOctetToChange;
    GT_U32      fifthOctetToChange;
    GT_U32      sixthOctetToChange;
    GT_U32      seventhOctetToChange;
    GT_U32      eighthOctetToChange;
    GT_U32      ninthOctetToChange;
    GT_U32      tenthOctetToChange;
    GT_U32      elevenOctetToChange;
    GT_U32      twelveOctetToChange;
    GT_U32      thirteenOctetToChange;
    GT_U32      fourteenOctetToChange;
    GT_U32      fifteenOctetToChange;
    GT_U32      sixteenOctetToChange;
    GT_IPV6ADDR   lastIpAddrAdded;
    GT_U32      numOfPrefixesAdded;
    GT_U32      numOfPrefixesExpected;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32 i;
    GT_U32 startTime;
    GT_U32 endTime;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  cpssDxChIpLpmIpv6UcPrefixAddManyByOctet\n "
                     "This function tries to add many sequential IPv6 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully added. ====\n");


    numOfPrefixesToAdd=2000000;
    numOfPrefixesExpected=2000000;
    firstOctetToChange = 15;
    secondOctetToChange = 14;
    thirdOctetToChange = 13;
    forthOctetToChange = 12;
    fifthOctetToChange = 11;
    sixthOctetToChange = 10;
    seventhOctetToChange = 9;
    eighthOctetToChange = 8;
    ninthOctetToChange = 7;
    tenthOctetToChange = 6;
    elevenOctetToChange = 5;
    twelveOctetToChange = 4;
    thirteenOctetToChange = 3;
    fourteenOctetToChange = 2;
    fifteenOctetToChange = 1;
    sixteenOctetToChange = 0;

    startIpAddr.arIP[0]=1;
    for (i=1; i<15; i++)
    {
        startIpAddr.arIP[i] = 0;
    }
    startIpAddr.arIP[15]=1;

    startTime = cpssOsTime();
    rc = prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &startIpAddr, prvTgfRouteEntryBaseIndex,
                                               numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                                               thirdOctetToChange,forthOctetToChange, fifthOctetToChange,
                                               sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                                               ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                                               twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                               fifteenOctetToChange, sixteenOctetToChange, &lastIpAddrAdded,
                                               &numOfPrefixesAdded);
    endTime = cpssOsTime();
    cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet of numOfPrefixesAdded=%d takes %d sec \n",numOfPrefixesAdded,endTime-startTime);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAddManyByOctet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("\n==== HW Octet to Block Mapping after adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage after adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsExpectedValueGet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, &numOfPrefixesExpected);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsForCurrentTestGet Failed");
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_NOT_ALLOW_SHARING_CNS;
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_NOT_ALLOW_SHARING_CNS;
                }
                else
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_NOT_ALLOW_SHARING_CNS;
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
        else
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_ALLOW_SHARING_CNS;
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_ALLOW_SHARING_CNS;
                }
                else
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_ALLOW_SHARING_CNS;
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_6K*4):
                /* each block memory has 6K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS;
                break;
            case (_16K*4):
                /* each block memory has 16K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS;
                break;
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS;
                break;
            case (_13K/2):
                /* Empirically seen at AC3X-48 */
                numOfPrefixesExpected = 1582;
                break;
            case (_8K*4):
                /* each block memory has 8K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_6K*4):
                /* each block memory has 6K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS;
                break;
            case (_16K*4):
                /* each block memory has 16K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS;
                break;
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS;
                break;
            case (_13K/2):
                /* Empirically seen at AC3X-48 */
                numOfPrefixesExpected = 23293;
                break;
            case (_8K*4):
                /* each block memory has 8K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }

    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsObservedValueSet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsObservedValueSet Failed");
    }

    if (numOfPrefixesAdded < numOfPrefixesExpected)
    {
        /* check if we get the correct number of prefixed added */
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, numOfPrefixesAdded,
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfPrefixesAdded,numOfPrefixesExpected);

    }

    numOfExistingPrefixesToDel = numOfPrefixesAdded;
    numOfExistingPrefixesAddForBulk = numOfPrefixesAdded;

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. send packet to the first and last prefix added
     */

    /* AUTODOC: send packet to the last first added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(startIpAddr);

    /* AUTODOC: send packet to the last prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(lastIpAddrAdded);

}

/**
* @internal prvTgfIpLpmMngIpv6UcRoutingAddManyByBulkConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration by bulk and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv6UcRoutingAddManyByBulkConfigurationAndTrafficGenerate
(
        PRV_TGF_IP_CAPACITY_TESTS_ENT testNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR   startIpAddr;
    GT_U32      numOfPrefixesToAdd;
    GT_U32      firstOctetToChange;
    GT_U32      secondOctetToChange;
    GT_U32      thirdOctetToChange;
    GT_U32      forthOctetToChange;
    GT_U32      fifthOctetToChange;
    GT_U32      sixthOctetToChange;
    GT_U32      seventhOctetToChange;
    GT_U32      eighthOctetToChange;
    GT_U32      ninthOctetToChange;
    GT_U32      tenthOctetToChange;
    GT_U32      elevenOctetToChange;
    GT_U32      twelveOctetToChange;
    GT_U32      thirteenOctetToChange;
    GT_U32      fourteenOctetToChange;
    GT_U32      fifteenOctetToChange;
    GT_U32      sixteenOctetToChange;
    GT_IPV6ADDR   lastIpAddrAdded;
    GT_U32      numOfPrefixesAdded;
    GT_U32      numOfPrefixesExpected;
    GT_U32      numOfPrefixesAlreadyExist;
    GT_U32      numOfPrefixesNotAddedDueToOutOfPpMem;
    GT_U32      bulkTime;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32 i;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  cpssDxChIpLpmIpv6UcPrefixAddManyByOctet\n "
                     "This function tries to add many sequential IPv6 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully added. ====\n");


    numOfPrefixesToAdd=2000000;
    numOfPrefixesExpected=2000000;
    firstOctetToChange = 15;
    secondOctetToChange = 14;
    thirdOctetToChange = 13;
    forthOctetToChange = 12;
    fifthOctetToChange = 11;
    sixthOctetToChange = 10;
    seventhOctetToChange = 9;
    eighthOctetToChange = 8;
    ninthOctetToChange = 7;
    tenthOctetToChange = 6;
    elevenOctetToChange = 5;
    twelveOctetToChange = 4;
    thirteenOctetToChange = 3;
    fourteenOctetToChange = 2;
    fifteenOctetToChange = 1;
    sixteenOctetToChange = 0;

    startIpAddr.arIP[0]=1;
    for (i=1; i<15; i++)
    {
        startIpAddr.arIP[i] = 0;
    }
    startIpAddr.arIP[15]=1;

    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsExpectedValueGet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, &numOfPrefixesExpected);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsForCurrentTestGet Failed");
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_NOT_ALLOW_SHARING_CNS;
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_NOT_ALLOW_SHARING_CNS;
                }
                else
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_NOT_ALLOW_SHARING_CNS;
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
        else
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_ALLOW_SHARING_CNS;
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_ALLOW_SHARING_CNS;
                }
                else
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_ALLOW_SHARING_CNS;
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_6K*4):
                /* each block memory has 6K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS;
                break;
            case (_16K*4):
                /* each block memory has 16K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS;
                break;
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS;
                break;
            case (_13K/2):
                /* Empirically seen at AC3X-48 */
                numOfPrefixesExpected = 1582;
                break;
            case (_8K*4):
                /* each block memory has 8K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_6K*4):
                /* each block memory has 6K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS;
                break;
            case (_16K*4):
                /* each block memory has 16K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS;
                break;
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS;
                break;
            case (_13K/2):
                /* Empirically seen at AC3X-48 */
                numOfPrefixesExpected = 23293;
                break;
            case (_8K*4):
                /* each block memory has 8K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }

    if (numOfExistingPrefixesAddForBulk!=0)
    {
       numOfPrefixesToAdd = numOfExistingPrefixesAddForBulk;
    }
    rc = prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk(prvTgfLpmDBId, 0, &startIpAddr, prvTgfRouteEntryBaseIndex,
                                               numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                                               thirdOctetToChange,forthOctetToChange, fifthOctetToChange,
                                               sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                                               ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                                               twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                               fifteenOctetToChange, sixteenOctetToChange, &lastIpAddrAdded,
                                               &numOfPrefixesAdded,&numOfPrefixesAlreadyExist,&numOfPrefixesNotAddedDueToOutOfPpMem,&bulkTime);
    cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk of numOfPrefixesAdded=%d takes %d sec \n",numOfPrefixesAdded,bulkTime);
    cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk of numOfPrefixesAlreadyExist=%d takes %d sec \n",numOfPrefixesAlreadyExist,bulkTime);
    cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk of numOfPrefixesNotAddedDueToOutOfPpMem=%d takes %d sec \n",numOfPrefixesNotAddedDueToOutOfPpMem,bulkTime);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("\n==== HW Octet to Block Mapping after adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage after adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsObservedValueSet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsObservedValueSet Failed");
    }

    if (numOfPrefixesAdded < numOfPrefixesAdded+numOfPrefixesAlreadyExist+numOfPrefixesNotAddedDueToOutOfPpMem)
    {
        /* check if we get the correct number of prefixed added */
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, (numOfPrefixesAdded+numOfPrefixesAlreadyExist+numOfPrefixesNotAddedDueToOutOfPpMem),
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 (numOfPrefixesAdded+numOfPrefixesAlreadyExist+numOfPrefixesNotAddedDueToOutOfPpMem),numOfPrefixesExpected);
    }

    numOfExistingPrefixesToDel = numOfPrefixesAdded+numOfPrefixesNotAddedDueToOutOfPpMem;/* need to delete entryies even after we get the first fail on not exist*/

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. send packet to the first and last prefix added
     */

    /* AUTODOC: send packet to the last first added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(startIpAddr);

    /* AUTODOC: send packet to the last prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(lastIpAddrAdded);

    numOfExistingPrefixesAddForBulk = 0;

}

/**
* @internal prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationRestore
(
        PRV_TGF_IP_CAPACITY_TESTS_ENT testNum
)
{
    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR   startIpAddr;
    GT_U32      firstOctetToChange;
    GT_U32      secondOctetToChange;
    GT_U32      thirdOctetToChange;
    GT_U32      forthOctetToChange;
    GT_U32      fifthOctetToChange;
    GT_U32      sixthOctetToChange;
    GT_U32      seventhOctetToChange;
    GT_U32      eighthOctetToChange;
    GT_U32      ninthOctetToChange;
    GT_U32      tenthOctetToChange;
    GT_U32      elevenOctetToChange;
    GT_U32      twelveOctetToChange;
    GT_U32      thirteenOctetToChange;
    GT_U32      fourteenOctetToChange;
    GT_U32      fifteenOctetToChange;
    GT_U32      sixteenOctetToChange;
    GT_IPV6ADDR   lastIpAddrDeleted;
    GT_U32      numOfPrefixesDeleted;
    GT_U32      numOfPrefixesExpected=0;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32 i;

     rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  cpssDxChIpLpmIpv6UcPrefixDelManyByOctet\n "
                     "This function tries to delete many sequential IPv6 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully deleted. ====\n");

    firstOctetToChange = 15;
    secondOctetToChange = 14;
    thirdOctetToChange = 13;
    forthOctetToChange = 12;
    fifthOctetToChange = 11;
    sixthOctetToChange = 10;
    seventhOctetToChange = 9;
    eighthOctetToChange = 8;
    ninthOctetToChange = 7;
    tenthOctetToChange = 6;
    elevenOctetToChange = 5;
    twelveOctetToChange = 4;
    thirteenOctetToChange = 3;
    fourteenOctetToChange = 2;
    fifteenOctetToChange = 1;
    sixteenOctetToChange = 0;

    startIpAddr.arIP[0] = 1;
    for (i=1; i<15; i++)
    {
        startIpAddr.arIP[i] = 0;
    }
    startIpAddr.arIP[15]=1;

    rc = prvTgfIpLpmIpv6UcPrefixDelManyByOctet(prvTgfLpmDBId, 0, &startIpAddr,numOfExistingPrefixesToDel,
                                               firstOctetToChange, secondOctetToChange,
                                               thirdOctetToChange, forthOctetToChange, fifthOctetToChange,
                                               sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                                               ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                                               twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                               fifteenOctetToChange, sixteenOctetToChange,
                                               &lastIpAddrDeleted, &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDelManyByOctet: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCapacityTestsExpectedResultsExpectedValueGet(testNum, ramDbCfg.blocksAllocationMethod,
                                                                GT_TRUE /*with PBR */, &numOfPrefixesExpected);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCapacityTestsExpectedResultsForCurrentTestGet Failed");
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_NOT_ALLOW_SHARING_CNS;
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_NOT_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_NOT_ALLOW_SHARING_CNS;
                }
                else
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_NOT_ALLOW_SHARING_CNS;
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
        else
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_FALCON_GM_ALLOW_SHARING_CNS;
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E: /* 28 banks with 14K lines, 2 banks with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MAX_L3_MIN_L2_NO_EM_FALCON_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MIN_L3_MAX_L2_NO_EM_FALCON_ALLOW_SHARING_CNS;
                break;
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:/* 24 banks with 7K lines, 6 banks with 640 lines */
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_AC5P_ALLOW_SHARING_CNS;
                }
                else
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_BASELINE_TEST_EXECUTION_MID_L3_FALCON_ALLOW_SHARING_CNS;
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_6K*4):
                /* each block memory has 6K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS;
                break;
            case (_16K*4):
                /* each block memory has 16K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS;
                break;
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS;
                break;
            case (_13K/2):
                /* Empirically seen at AC3X-48 */
                numOfPrefixesExpected = 30;
                break;
            case (_8K*4):
                /* each block memory has 8K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_6K*4):
                /* each block memory has 6K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS;
                break;
            case (_16K*4):
                /* each block memory has 16K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS;
                break;
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS;
                break;
            case (_13K/2):
                /* Empirically seen at AC3X-48 */
                numOfPrefixesExpected = 23293;
                break;
            case (_8K*4):
                /* each block memory has 8K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }
    if (numOfExistingPrefixesToDel < numOfPrefixesExpected)
    {
        /* check if we get the correct number of prefixed deleted */
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, numOfExistingPrefixesToDel,
                                      "======= numOfExistingPrefixesToDel[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                     numOfExistingPrefixesToDel,numOfPrefixesExpected);
    }

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* change blocks allocation method and reset LPM DB */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

}


