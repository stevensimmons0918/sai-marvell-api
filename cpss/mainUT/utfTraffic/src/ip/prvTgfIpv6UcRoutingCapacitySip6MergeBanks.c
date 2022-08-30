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
* @file prvTgfIpv6UcRoutingCapacitySip6MergeBanks.c
*
* @brief IPv6 LPM capacity test for merge banks
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <ip/prvTgfIpv6UcRoutingCapacitySip6MergeBanks.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_U32                      prvTgfLpmDBId = 0;

/**
 * @internal prvTgfIpv6UcRoutingCapacitySip6MergeBanksAddIpv6Prefixes function
 * @endinternal
 *
 * @brief  Add IPv6 prefixes increment last octet with step size 4.
 *
 * @param[in]  numPrefixes   - number of prefixes to add.
 * @param[in]  lastOct       - Octet which needs to be incremented.
 *
 */
static GT_VOID prvTgfIpv6UcRoutingCapacitySip6MergeBanksAddIpv6Prefixes
(
    IN GT_U32 numPrefixes,
    IN GT_U32 lastOct
)
{
    GT_STATUS                   rc = GT_OK;
    GT_IPV6ADDR                 ucIpv6Addr;
    GT_IPV6ADDR                 lastIpAddrAdded;
    GT_U32                      numOfPrefixesAdded   = 0;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    CPSS_UNICAST_MULTICAST_ENT  prefixType;
    GT_U32                      prefixLength;
    GT_U32                      srcPrefixLength;
    GT_U32                      totalNumIpUc;
    GT_U8                       stepArr[16];
    GT_U8                       srcStepArr[16];
    GT_U32                      i;
    GT_U32                      octArr[16];
    GT_U32                      temp;

    cpssOsMemSet(&ucIpv6Addr.arIP[0],0,sizeof(ucIpv6Addr.arIP));

    totalNumIpUc             = numPrefixes;
    protocolStack            = CPSS_IP_PROTOCOL_IPV6_E;
    prefixType               = CPSS_UNICAST_E;
    prefixLength             = (lastOct + 1) * 8;
    srcPrefixLength          = 0;
    ucIpv6Addr.arIP[0]       = 1;
    ucIpv6Addr.arIP[lastOct] = 1;

    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d\n", prvTgfDevNum);
    }

    for (i=0; i<16; i++)
    {
        stepArr[i] = 1;
        srcStepArr[i] = 1;
    }
    stepArr[lastOct] = 4;

    /* change global step size */
    rc = tgfCpssDxChIpLpmDbgIpv6SetStep(stepArr, prefixLength, srcStepArr, srcPrefixLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfCpssDxChIpLpmDbgIpv6SetStep: %d\n", prvTgfDevNum);

    temp = lastOct + 1;
    for (i=0; i<16; i++)
    {
        if (temp != 0)
        {
            temp --;
            octArr[i] = temp;
        }
        else
        {
            octArr[i] = i;
        }
    }

    /* add the prefixes */
    rc = prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &ucIpv6Addr, 55, totalNumIpUc,
                                               octArr[0], octArr[1], octArr[2], octArr[3], octArr[4], octArr[5],
                                               octArr[6], octArr[7], octArr[8], octArr[9], octArr[10], octArr[11],
                                               octArr[12], octArr[13], octArr[14], octArr[15],
                                               &lastIpAddrAdded, &numOfPrefixesAdded);

    if (rc == GT_OK)
    {
        cpssOsPrintf("Last ipv6 unicast  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "numOfPrefixesAdded = %d\n",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,lastIpAddrAdded.arIP[4]
                 ,lastIpAddrAdded.arIP[5]
                 ,lastIpAddrAdded.arIP[6]
                 ,lastIpAddrAdded.arIP[7]
                 ,lastIpAddrAdded.arIP[8]
                 ,lastIpAddrAdded.arIP[9]
                 ,lastIpAddrAdded.arIP[10]
                 ,lastIpAddrAdded.arIP[11]
                 ,lastIpAddrAdded.arIP[12]
                 ,lastIpAddrAdded.arIP[13]
                 ,lastIpAddrAdded.arIP[14]
                 ,lastIpAddrAdded.arIP[15]
                 ,numOfPrefixesAdded);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpv6UcRoutingCapacitySip6MergeBanksTest: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);

    /* check validity */
    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, protocolStack , prefixType, GT_TRUE);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmValidityCheck: %d", prvTgfDevNum);
    }
}

/**
 * @internal prvTgfIpv6UcRoutingCapacitySip6MergeBanksDelIpv6Prefixes function
 * @endinternal
 *
 * @brief  Delete IPv6 prefixes with incrementing lastOct with step size 4.
 *
 * @param[in]  numPrefixes   - number of prefixes to add.
 * @param[in]  lastOct       - Octet which needs to be incremented.
 * @param[in]  delOct        - octet to be incremented during delete.
 * @param[in]  delStep       - increment step size for delOct
 * @param[in]  delOffset     - Offset to add to IPv6 start packet last octet.
 *
 */
static GT_VOID prvTgfIpv6UcRoutingCapacitySip6MergeBanksDelIpv6Prefixes
(
    IN GT_U32 numPrefixes,
    IN GT_U32 lastOct,
    IN GT_U32 delOct,
    IN GT_U32 delStep,
    IN GT_U32 delOffset
)
{
    GT_STATUS                   rc = GT_OK;
    GT_IPV6ADDR                 ucIpv6Addr;
    GT_IPV6ADDR                 lastIpAddrDeleted;
    GT_U32                      numOfPrefixesDeleted = 0;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    CPSS_UNICAST_MULTICAST_ENT  prefixType;
    GT_U32                      prefixLength;
    GT_U32                      srcPrefixLength;
    GT_U32                      totalNumIpUc;
    GT_U8                       stepArr[16];
    GT_U8                       srcStepArr[16];
    GT_U32                      i;
    GT_U32                      octArr[16];
    GT_U32                      temp;

    cpssOsMemSet(&ucIpv6Addr.arIP[0],0,sizeof(ucIpv6Addr.arIP));

    totalNumIpUc             = numPrefixes;
    protocolStack            = CPSS_IP_PROTOCOL_IPV6_E;
    prefixType               = CPSS_UNICAST_E;
    prefixLength             = (lastOct + 1) * 8;
    srcPrefixLength          = 0;
    ucIpv6Addr.arIP[0]       = 1;
    ucIpv6Addr.arIP[lastOct] = 1;
    ucIpv6Addr.arIP[delOct]  = delOffset;

    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d", prvTgfDevNum);
    }

    for (i=0; i<16; i++)
    {
        stepArr[i] = 1;
        srcStepArr[i] = 1;
    }
    stepArr[lastOct] = 4;
    stepArr[delOct]  = delStep;

    /* change global step size */
    rc = tgfCpssDxChIpLpmDbgIpv6SetStep(stepArr, prefixLength, srcStepArr, srcPrefixLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfCpssDxChIpLpmDbgIpv6SetStep: %d", prvTgfDevNum);

    temp = lastOct + 1;
    for (i=0; i<16; i++)
    {
        if (temp != 0)
        {
            temp --;
            octArr[i] = temp;
        }
        else
        {
            octArr[i] = i;
        }
    }

    /* add the prefixes */
    rc = prvTgfIpLpmIpv6UcPrefixDelManyByOctet(prvTgfLpmDBId, 0, &ucIpv6Addr, totalNumIpUc,
                                               octArr[0], octArr[1], octArr[2], octArr[3], octArr[4], octArr[5],
                                               octArr[6], octArr[7], octArr[8], octArr[9], octArr[10], octArr[11],
                                               octArr[12], octArr[13], octArr[14], octArr[15],
                                               &lastIpAddrDeleted, &numOfPrefixesDeleted);

    cpssOsPrintf("numberOfAddedPrefixes = %d \n",numOfPrefixesDeleted);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Last ipv6 unicast  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "numOfPrefixesDeleted = %d\n",
                  lastIpAddrDeleted.arIP[0]
                 ,lastIpAddrDeleted.arIP[1]
                 ,lastIpAddrDeleted.arIP[2]
                 ,lastIpAddrDeleted.arIP[3]
                 ,lastIpAddrDeleted.arIP[4]
                 ,lastIpAddrDeleted.arIP[5]
                 ,lastIpAddrDeleted.arIP[6]
                 ,lastIpAddrDeleted.arIP[7]
                 ,lastIpAddrDeleted.arIP[8]
                 ,lastIpAddrDeleted.arIP[9]
                 ,lastIpAddrDeleted.arIP[10]
                 ,lastIpAddrDeleted.arIP[11]
                 ,lastIpAddrDeleted.arIP[12]
                 ,lastIpAddrDeleted.arIP[13]
                 ,lastIpAddrDeleted.arIP[14]
                 ,lastIpAddrDeleted.arIP[15]
                 ,numOfPrefixesDeleted);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpv6UcRoutingCapacitySip6MergeBanksTest: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);

    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, protocolStack , prefixType, GT_TRUE);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmValidityCheck: %d", prvTgfDevNum);
    }
}

/**
 * @internal prvTgfIpv6UcRoutingCapacitySip6MergeBanks function
 * @endinternal
 *
 * @brief Run IPv6 LPM capacity test for merge banks.
 *        This is the test for defragmentation phase 4
 *        1. Use 20 small banks.
 *        2. Add Ipv6 prefixes of length 64 bits by incrementing octet = 7. (Step = 4) until LPM banks are full.
 *        3. Delete half of the prefixes by incrementing octet 6 (Step = 2) and incrementing octet 7 (Step = 4)
 *        4. Add Ipv6 prefixes of length 80 bits until LPM banks are full. Merge operation occurs here.
 *        5. Delete half of remaining prefixes of length 64 bits by incrementing octet 6 (Step = 4)
 *           and octet 7 (Step = 4).
 *        6. Add Ipv6 prefixes of length 96 bits until LPM banks are full. Merge operation occurs here.
 *        7. Run validity at each of the above steps.
 *        8. Flush all prefixes.
 */
GT_VOID prvTgfIpv6UcRoutingCapacitySip6MergeBanks
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      lastOct = 0;
    GT_U32                      numPrefixes = 0;

    /* Flush all IPv4 and IPv6 entries */
    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    /* 2. Add Ipv6 prefixes of length 64 bits by incrementing octet = 7. (Step = 4) until LPM banks are full. */
    lastOct = 7;
    numPrefixes = 64*256;
    cpssOsPrintf("Phase1: Adding Prefixes: length = %d, numberOfPrefixes = %d\n", (lastOct+1)*8, numPrefixes);
    prvTgfIpv6UcRoutingCapacitySip6MergeBanksAddIpv6Prefixes(numPrefixes, lastOct);

    /* 3. Delete half of the prefixes by incrementing octet 6 (Step = 2) and incrementing octet 7 (Step = 4) */
    lastOct = 7;
    numPrefixes = 32*256;
    cpssOsPrintf("Phase2: Deleting Prefixes: length = %d, numberOfPrefixes = %d\n", (lastOct+1)*8, numPrefixes);
    prvTgfIpv6UcRoutingCapacitySip6MergeBanksDelIpv6Prefixes(numPrefixes, lastOct, 6/*delOct*/, 2/*delStep*/, 0/*delOffset*/);

    /* 4. Add Ipv6 prefixes of length 80 bits until LPM banks are full. Merge operation occurs here. */
    lastOct = 9;
    numPrefixes = 32*256;
    cpssOsPrintf("Phase3: Adding Prefixes: length = %d, numberOfPrefixes = %d\n", (lastOct+1)*8, numPrefixes);
    prvTgfIpv6UcRoutingCapacitySip6MergeBanksAddIpv6Prefixes(numPrefixes, lastOct);

    /* 5. Delete half of remaining prefixes of length 64 bits by incrementing octet 6 (Step = 4)
     *    and octet 7 (Step = 4)
     */
    lastOct = 7;
    numPrefixes = 16*256;
    cpssOsPrintf("Phase4: Deleteing Prefixes: length = %d, numberOfPrefixes = %d\n", (lastOct+1)*8, numPrefixes);
    prvTgfIpv6UcRoutingCapacitySip6MergeBanksDelIpv6Prefixes(numPrefixes, lastOct, 6/*delOct*/, 4/*delStep*/, 1/*delOffset*/);

    /* 6. Add Ipv6 prefixes of length 96 bits until LPM banks are full. Merge operation occurs here.*/
    lastOct = 11;
    numPrefixes = 16*256;
    cpssOsPrintf("Phase5: Adding Prefixes: length = %d, numberOfPrefixes = %d\n", (lastOct+1)*8, numPrefixes);
    prvTgfIpv6UcRoutingCapacitySip6MergeBanksAddIpv6Prefixes(numPrefixes, lastOct);

    /* Flush all IPv4 and IPv6 prefixes */
    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);
    return;
}

/**
 * @internal prvTgfIpv6UcRoutingCapacitySip6MergeBanksWithShrink function
 * @endinternal
 *
 * @brief Run IPv6 LPM capacity test for merge banks with shrink.
 *        This is the test for defragmentation phase 4
 *        1. Use 20 small banks.
 *        2. Add Ipv6 prefixes with last octet = 1 and by incrementing last octet (Step = 4) until LPM banks are full.
 *        3. Delete half of the prefixes by incrementing last octet (Step = 4) and incrementing last octet (Step = 8)
 *        4. Repeat step 2 and step 3
 *        7. Run validity at each of the above steps.
 *        8. Flush all prefixes.
 */
GT_VOID prvTgfIpv6UcRoutingCapacitySip6MergeBanksWithShrink
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      lastOct = 0;
    GT_U32                      numPrefixes = 0;
    GT_U32                      i = 0;
    (void)rc;

    /* Flush all IPv4 and IPv6 entries */
    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    for (i = 1; i < 8; i+=2)
    {
        /* 2. Add Ipv6 prefixes of length 64 bits by incrementing octet = 7. (Step = 4) until LPM banks are full. */
        lastOct = i;
        numPrefixes = 64*256;
        cpssOsPrintf("Phase%d: Adding Prefixes: length = %d, numberOfPrefixes = %d\n", i, (lastOct+1)*8, numPrefixes);
        prvTgfIpv6UcRoutingCapacitySip6MergeBanksAddIpv6Prefixes(numPrefixes, lastOct);

        /* 3. Delete half of the prefixes by incrementing octet 6 (Step = 2) and incrementing octet 7 (Step = 4) */
        lastOct = i;
        numPrefixes = 32*256;
        cpssOsPrintf("Phase%d: Deleting Prefixes: length = %d, numberOfPrefixes = %d\n", i+1, (lastOct+1)*8, numPrefixes);
        prvTgfIpv6UcRoutingCapacitySip6MergeBanksDelIpv6Prefixes(numPrefixes, lastOct, lastOct, 8/*delStep*/, 0/*delOffset*/);
    }

    /* Flush all IPv4 and IPv6 prefixes */
    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);
}

/**
 * @internal prvTgfIpv6McRoutingCapacitySip6DeleteAndRefill function
 * @endinternal
 *
 * @brief Run IPv6 LPM capacity test.
 *
 */
GT_VOID prvTgfIpv6McRoutingCapacitySip6DeleteAndRefill
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_IPV6ADDR                 grpStartIpAddr;
    GT_IPV6ADDR                 grpLastIpAddrAdded;
    GT_IPV6ADDR                 srcStartIpAddr;
    GT_IPV6ADDR                 srcLastIpAddrAdded;
    GT_U32                      numOfPrefixesAdded   = 0;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    CPSS_UNICAST_MULTICAST_ENT  prefixType;
    GT_U32                      prefixLength;
    GT_U32                      srcPrefixLength;
    GT_U32                      totalNumIpMc;
    GT_U8                       stepArr[16];
    GT_U8                       srcStepArr[16];
    GT_U32                      i;

    prvTgfIpValidityCheckEnable(GT_FALSE);

    /* Flush all IPv4 and IPv6 entries */
    PRV_UTF_LOG0_MAC("\n==== Flushing IPv6 Mc Prefixes \n");
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsMemSet(&grpStartIpAddr.arIP[0],0,sizeof(grpStartIpAddr.arIP));
    cpssOsMemSet(&grpLastIpAddrAdded.arIP[0],0,sizeof(grpLastIpAddrAdded.arIP));
    cpssOsMemSet(&srcStartIpAddr.arIP[0],0,sizeof(srcStartIpAddr.arIP));
    cpssOsMemSet(&srcLastIpAddrAdded.arIP[0],0,sizeof(srcLastIpAddrAdded.arIP));

    protocolStack            = CPSS_IP_PROTOCOL_IPV6_E;
    prefixType               = CPSS_MULTICAST_E;
    prefixLength             = 128;
    srcPrefixLength          = 128;

    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d\n", prvTgfDevNum);
    }

    for (i=0; i<16; i++)
    {
        stepArr[i]    = 1;
        srcStepArr[i] = 1;
    }

    /* change global step size */
    rc = tgfCpssDxChIpLpmDbgIpv6SetStep(stepArr, prefixLength, srcStepArr, srcPrefixLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfCpssDxChIpLpmDbgIpv6SetStep: %d\n", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("\n==== Add IPv6 MC Prefixes \n");
    /* Add IPv6 635392 MC prefixes by incrementing sip by 1
       start dip="FFFF:0000:0000:0000:0000:0000:0022:2222", sip="0000:1111:0000:2222:0000:1111:0000:2222" */

    grpStartIpAddr.arIP[0]  = 0xFF;
    grpStartIpAddr.arIP[1]  = 0xFF;
    grpStartIpAddr.arIP[2]  = 0x00;
    grpStartIpAddr.arIP[3]  = 0x00;
    grpStartIpAddr.arIP[4]  = 0x00;
    grpStartIpAddr.arIP[5]  = 0x00;
    grpStartIpAddr.arIP[6] = 0x00;
    grpStartIpAddr.arIP[7] = 0x00;
    grpStartIpAddr.arIP[8] = 0x00;
    grpStartIpAddr.arIP[9] = 0x00;
    grpStartIpAddr.arIP[10]= 0x00;
    grpStartIpAddr.arIP[11]= 0x00;
    grpStartIpAddr.arIP[12]= 0x00;
    grpStartIpAddr.arIP[13]= 0x22;
    grpStartIpAddr.arIP[14]= 0x22;
    grpStartIpAddr.arIP[15]= 0x22;

    srcStartIpAddr.arIP[0]  = 0x00;
    srcStartIpAddr.arIP[1]  = 0x00;
    srcStartIpAddr.arIP[2]  = 0x11;
    srcStartIpAddr.arIP[3]  = 0x11;
    srcStartIpAddr.arIP[4]  = 0x00;
    srcStartIpAddr.arIP[5]  = 0x00;
    srcStartIpAddr.arIP[6] = 0x22;
    srcStartIpAddr.arIP[7] = 0x22;
    srcStartIpAddr.arIP[8] = 0x00;
    srcStartIpAddr.arIP[9] = 0x00;
    srcStartIpAddr.arIP[10]= 0x11;
    srcStartIpAddr.arIP[11]= 0x11;
    srcStartIpAddr.arIP[12]= 0x00;
    srcStartIpAddr.arIP[13]= 0x00;
    srcStartIpAddr.arIP[14]= 0x22;
    srcStartIpAddr.arIP[15]= 0x22;

    totalNumIpMc = 635392;

    /* add the prefixes */
    rc = prvTgfIpLpmIpv6McPrefixAddManyByOctet(prvTgfLpmDBId, 0, &grpStartIpAddr, &srcStartIpAddr,
                                               55, totalNumIpMc, GT_FALSE /*changeGrpAddr*/,
                                               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                               GT_TRUE /*changeSrcAddr*/,
                                               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                               &grpLastIpAddrAdded, &srcLastIpAddrAdded, &numOfPrefixesAdded);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Last ipv6 grpAddrAded  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "Last ipv6 srcAddrAded  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "numOfPrefixesAdded = %d\n",
                  grpLastIpAddrAdded.arIP[0], grpLastIpAddrAdded.arIP[1], grpLastIpAddrAdded.arIP[2], grpLastIpAddrAdded.arIP[3]
                 ,grpLastIpAddrAdded.arIP[4],grpLastIpAddrAdded.arIP[5],grpLastIpAddrAdded.arIP[6],grpLastIpAddrAdded.arIP[7]
                 ,grpLastIpAddrAdded.arIP[8],grpLastIpAddrAdded.arIP[9],grpLastIpAddrAdded.arIP[10],grpLastIpAddrAdded.arIP[11]
                 ,grpLastIpAddrAdded.arIP[12],grpLastIpAddrAdded.arIP[13],grpLastIpAddrAdded.arIP[14],grpLastIpAddrAdded.arIP[15]
                 ,srcLastIpAddrAdded.arIP[0],srcLastIpAddrAdded.arIP[1],srcLastIpAddrAdded.arIP[2],srcLastIpAddrAdded.arIP[3]
                 ,srcLastIpAddrAdded.arIP[4],srcLastIpAddrAdded.arIP[5],srcLastIpAddrAdded.arIP[6],srcLastIpAddrAdded.arIP[7]
                 ,srcLastIpAddrAdded.arIP[8],srcLastIpAddrAdded.arIP[9],srcLastIpAddrAdded.arIP[10],srcLastIpAddrAdded.arIP[11]
                 ,srcLastIpAddrAdded.arIP[12],srcLastIpAddrAdded.arIP[13],srcLastIpAddrAdded.arIP[14],srcLastIpAddrAdded.arIP[15]
                 ,numOfPrefixesAdded);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpv6McRoutingCapacitySip6DeleteAndRefill: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("\n==== Delete half IPv6 MC Prefixes \n");
    /* Delete 317696 MC prefixes, , incrementing sip by 1
    start with dip="FFFF:0000:0000:0000:0000:0000:0022:2222", sip="0000:1111:0000:2222:0000:1111:0004:FB22" */

    grpStartIpAddr.arIP[0] = 0xFF;
    grpStartIpAddr.arIP[1] = 0xFF;
    grpStartIpAddr.arIP[2] = 0x00;
    grpStartIpAddr.arIP[3] = 0x00;
    grpStartIpAddr.arIP[4] = 0x00;
    grpStartIpAddr.arIP[5] = 0x00;
    grpStartIpAddr.arIP[6] = 0x00;
    grpStartIpAddr.arIP[7] = 0x00;
    grpStartIpAddr.arIP[8] = 0x00;
    grpStartIpAddr.arIP[9] = 0x00;
    grpStartIpAddr.arIP[10] = 0x00;
    grpStartIpAddr.arIP[11] = 0x00;
    grpStartIpAddr.arIP[12] = 0x00;
    grpStartIpAddr.arIP[13] = 0x22;
    grpStartIpAddr.arIP[14] = 0x22;
    grpStartIpAddr.arIP[15] = 0x22;

    srcStartIpAddr.arIP[0] = 0x00;
    srcStartIpAddr.arIP[1] = 0x00;
    srcStartIpAddr.arIP[2] = 0x11;
    srcStartIpAddr.arIP[3] = 0x11;
    srcStartIpAddr.arIP[4] = 0x00;
    srcStartIpAddr.arIP[5] = 0x00;
    srcStartIpAddr.arIP[6] = 0x22;
    srcStartIpAddr.arIP[7] = 0x22;
    srcStartIpAddr.arIP[8] = 0x00;
    srcStartIpAddr.arIP[9] = 0x00;
    srcStartIpAddr.arIP[10]= 0x11;
    srcStartIpAddr.arIP[11]= 0x11;
    srcStartIpAddr.arIP[12]= 0x00;
    srcStartIpAddr.arIP[13]= 0x04;
    srcStartIpAddr.arIP[14]= 0xFB;
    srcStartIpAddr.arIP[15]= 0x22;

    totalNumIpMc = 317696;

    rc = prvTgfIpLpmIpv6McPrefixDelManyByOctet(prvTgfLpmDBId, 0, &grpStartIpAddr, &srcStartIpAddr,
                                               totalNumIpMc, GT_FALSE /*changeGrpAddr*/,
                                               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                               GT_TRUE /*changeSrcAddr*/,
                                               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                               &grpLastIpAddrAdded, &srcLastIpAddrAdded, &numOfPrefixesAdded);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Last ipv6 grpAddrAded  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "Last ipv6 srcAddrAded  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "numOfPrefixesAdded = %d\n",
                  grpLastIpAddrAdded.arIP[0], grpLastIpAddrAdded.arIP[1], grpLastIpAddrAdded.arIP[2], grpLastIpAddrAdded.arIP[3]
                 ,grpLastIpAddrAdded.arIP[4],grpLastIpAddrAdded.arIP[5],grpLastIpAddrAdded.arIP[6],grpLastIpAddrAdded.arIP[7]
                 ,grpLastIpAddrAdded.arIP[8],grpLastIpAddrAdded.arIP[9],grpLastIpAddrAdded.arIP[10],grpLastIpAddrAdded.arIP[11]
                 ,grpLastIpAddrAdded.arIP[12],grpLastIpAddrAdded.arIP[13],grpLastIpAddrAdded.arIP[14],grpLastIpAddrAdded.arIP[15]
                 ,srcLastIpAddrAdded.arIP[0],srcLastIpAddrAdded.arIP[1],srcLastIpAddrAdded.arIP[2],srcLastIpAddrAdded.arIP[3]
                 ,srcLastIpAddrAdded.arIP[4],srcLastIpAddrAdded.arIP[5],srcLastIpAddrAdded.arIP[6],srcLastIpAddrAdded.arIP[7]
                 ,srcLastIpAddrAdded.arIP[8],srcLastIpAddrAdded.arIP[9],srcLastIpAddrAdded.arIP[10],srcLastIpAddrAdded.arIP[11]
                 ,srcLastIpAddrAdded.arIP[12],srcLastIpAddrAdded.arIP[13],srcLastIpAddrAdded.arIP[14],srcLastIpAddrAdded.arIP[15]
                 ,numOfPrefixesAdded);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpv6McRoutingCapacitySip6DeleteAndRefill: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("\n==== Add deleted half IPv6 MC Prefixes \n");
    /* Add 317696 MC prefixes, , incrementing sip by 1
    start with dip="FFFF:0000:0000:0000:0000:0000:0022:2222", sip="0000:1111:0000:2222:0000:1111:0004:FB22" */

    grpStartIpAddr.arIP[0]  = 0xFF;
    grpStartIpAddr.arIP[1]  = 0xFF;
    grpStartIpAddr.arIP[2]  = 0x00;
    grpStartIpAddr.arIP[3]  = 0x00;
    grpStartIpAddr.arIP[4]  = 0x00;
    grpStartIpAddr.arIP[5]  = 0x00;
    grpStartIpAddr.arIP[6] = 0x00;
    grpStartIpAddr.arIP[7] = 0x00;
    grpStartIpAddr.arIP[8] = 0x00;
    grpStartIpAddr.arIP[9] = 0x00;
    grpStartIpAddr.arIP[10]= 0x00;
    grpStartIpAddr.arIP[11]= 0x00;
    grpStartIpAddr.arIP[12]= 0x00;
    grpStartIpAddr.arIP[13]= 0x22;
    grpStartIpAddr.arIP[14]= 0x22;
    grpStartIpAddr.arIP[15]= 0x22;

    srcStartIpAddr.arIP[0]  = 0x00;
    srcStartIpAddr.arIP[1]  = 0x00;
    srcStartIpAddr.arIP[2]  = 0x11;
    srcStartIpAddr.arIP[3]  = 0x11;
    srcStartIpAddr.arIP[4]  = 0x00;
    srcStartIpAddr.arIP[5]  = 0x00;
    srcStartIpAddr.arIP[6] = 0x22;
    srcStartIpAddr.arIP[7] = 0x22;
    srcStartIpAddr.arIP[8] = 0x00;
    srcStartIpAddr.arIP[9] = 0x00;
    srcStartIpAddr.arIP[10]= 0x11;
    srcStartIpAddr.arIP[11]= 0x11;
    srcStartIpAddr.arIP[12]= 0x00;
    srcStartIpAddr.arIP[13]= 0x04;
    srcStartIpAddr.arIP[14]= 0xFB;
    srcStartIpAddr.arIP[15]= 0x22;

    totalNumIpMc = 317696;

    rc = prvTgfIpLpmIpv6McPrefixAddManyByOctet(prvTgfLpmDBId, 0, &grpStartIpAddr, &srcStartIpAddr,
                                               55, totalNumIpMc, GT_FALSE /*changeGrpAddr*/,
                                               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                               GT_TRUE /*changeSrcAddr*/,
                                               15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                               &grpLastIpAddrAdded, &srcLastIpAddrAdded, &numOfPrefixesAdded);

    if (rc == GT_OK)
    {
        cpssOsPrintf("Last ipv6 grpAddrAded  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "Last ipv6 srcAddrAded  = %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                     "numOfPrefixesAdded = %d\n",
                  grpLastIpAddrAdded.arIP[0], grpLastIpAddrAdded.arIP[1], grpLastIpAddrAdded.arIP[2], grpLastIpAddrAdded.arIP[3]
                 ,grpLastIpAddrAdded.arIP[4],grpLastIpAddrAdded.arIP[5],grpLastIpAddrAdded.arIP[6],grpLastIpAddrAdded.arIP[7]
                 ,grpLastIpAddrAdded.arIP[8],grpLastIpAddrAdded.arIP[9],grpLastIpAddrAdded.arIP[10],grpLastIpAddrAdded.arIP[11]
                 ,grpLastIpAddrAdded.arIP[12],grpLastIpAddrAdded.arIP[13],grpLastIpAddrAdded.arIP[14],grpLastIpAddrAdded.arIP[15]
                 ,srcLastIpAddrAdded.arIP[0],srcLastIpAddrAdded.arIP[1],srcLastIpAddrAdded.arIP[2],srcLastIpAddrAdded.arIP[3]
                 ,srcLastIpAddrAdded.arIP[4],srcLastIpAddrAdded.arIP[5],srcLastIpAddrAdded.arIP[6],srcLastIpAddrAdded.arIP[7]
                 ,srcLastIpAddrAdded.arIP[8],srcLastIpAddrAdded.arIP[9],srcLastIpAddrAdded.arIP[10],srcLastIpAddrAdded.arIP[11]
                 ,srcLastIpAddrAdded.arIP[12],srcLastIpAddrAdded.arIP[13],srcLastIpAddrAdded.arIP[14],srcLastIpAddrAdded.arIP[15]
                 ,numOfPrefixesAdded);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpv6McRoutingCapacitySip6DeleteAndRefill: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint: %d", prvTgfDevNum);

    /* check validity */
    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, protocolStack , prefixType, GT_TRUE);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmValidityCheck: %d", prvTgfDevNum);
    }


    /* Flush all IPv4 and IPv6 prefixes */
    PRV_UTF_LOG0_MAC("\n==== Flushing IPv6 Mc Prefixes \n");
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

}
