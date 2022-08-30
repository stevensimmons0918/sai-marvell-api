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
* @file prvTgfIpCapacityTestsExpectedResults.c
*
* @brief Store expected results for capacity tests
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

#include <common/tgfCommon.h>
#include <common/tgfIpGen.h>

#include <ip/prvTgfIpCapacityTestsExpectedResults.h>

/* Ironman-L : Array of expected values for the capacity tests */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC ironmanlCapacityTestsExpectedResults[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {49697, 44808, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{19375, 19059, 21407, 21407}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{19468, 19543, 21556, 21591}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{19375, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{47368, 44808, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{49542, 49415, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{6076, 6076, 7530, 7943}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{6076, 6076, 7924, 8012}
};

/* Ironman-L : Array of expected values for the capacity tests */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC ironmanlCapacityTestsExpectedResultsNonBaseline[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {49697, 44808, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{19375, 19059, 21407, 21407}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{19468, 19543, 21556, 21591}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{47368, 44808, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{49542, 49415, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{6076, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{6076, 0, 0, 0}
};


/* AC5X : Array of expected values for the capacity tests */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC ac5xCapacityTestsExpectedResults[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {111530, 120063, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{45688, 50972, 47629, 52921}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{47640, 51227, 50020, 52979}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{45688, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{57302, 116479, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{105502, 116743, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{16513, 16979, 17279, 18229}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{16513, 17006, 17285, 18254}
};

/* AC5X : Array of expected values for the capacity tests */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC ac5xCapacityTestsExpectedResultsNonBaseline[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {111530, 120831, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{26615, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{57377, 119807, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{109810, 121001, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{13308, 0, 0, 0}
};


/* AC5P : Array of expected values for the capacity tests */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC ac5pCapacityTestsExpectedResults[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{211613, 228105, 220562, 234722}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{214058, 230215, 221318, 237702}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{0, 0, 0, 0}
};

/* AC5P : Array of expected values for the capacity tests */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC ac5pCapacityTestsExpectedResultsNonBaseline[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{26615, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{0, 0, 0, 0}
};


/* Array of observed values for the capacity tests to store */
static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC tgfIpCapacityTestsObservedResults[] = {
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E */
     {0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E */
    ,{0, 0, 0, 0}
    /* PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E */
    ,{0, 0, 0, 0}
};

/**
 * @internal prvTgfIpCapacityTestsNameGet function
 * @endinternal
 *
 * @param [in] testNum - test enum value
 *
 * @return GT_CHAR* - Test name.
 */
static GT_CHAR* prvTgfIpCapacityTestsNameGet
(
    IN  PRV_TGF_IP_CAPACITY_TESTS_ENT testNum
)
{
    switch(testNum)
    {
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E          :
            return "tgfIpv4UcRoutingAddManyIpLpmMng";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E          :
            return "tgfIpv4UcRoutingAddRealPrefixes";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E        :
            return "tgfIpv4UcRoutingAddRealPrefixes_2 - ContinueAfterFail";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E :
            return "tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E          :
            return "tgfIpv6UcRoutingAddManyIpLpmMng_E";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E      :
            return "tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E          :
            return "tgfIpv4McRoutingAddRealPrefixes_E";
        case PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E        :
            return "tgfIpv4McRoutingAddRealPrefixes_2 - ContinueAfterFail";
        default:
            return "";
    }
    return "";
}

static PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC  *prvTgfIpExpectedResultsArrayGet
(
    IN  PRV_TGF_IP_CAPACITY_TESTS_ENT       testNum
)
{
    if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
        {
            return &ironmanlCapacityTestsExpectedResults[testNum];
        }
        else if (PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
        {
            return &ac5xCapacityTestsExpectedResults[testNum];
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            return &ac5pCapacityTestsExpectedResults[testNum];
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
        {
            return &ironmanlCapacityTestsExpectedResultsNonBaseline[testNum];
        }
        else if (PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum))
        {
            return  &ac5xCapacityTestsExpectedResultsNonBaseline[testNum];
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            return  &ac5pCapacityTestsExpectedResultsNonBaseline[testNum];
        }
        else
        {
            return NULL;
        }
    }

    return GT_OK;
}

/**
 * @internal prvTgfIpCapacityTestsResultsCurrentDump function
 * @endinternal
 *
 * @brief This function prints expected and observed values
 *        for the current capacity test
 *
 * @param[in] testNum  - test enum value.
 *
 * @return GT_OK - on success
 *         GT_FAIL - otherwise
 */
GT_STATUS prvTgfIpCapacityTestsResultsCurrentDump
(
    PRV_TGF_IP_CAPACITY_TESTS_ENT testNum
)
{
    PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC *testExpResult = NULL;
    PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC *testObsResult;

    if (testNum >= PRV_TGF_IP_CAPACITY_TESTS__LAST_E)
    {
        return GT_FAIL;
    }

    testExpResult = prvTgfIpExpectedResultsArrayGet(testNum);
    if (testExpResult == NULL)
    {
        return GT_FAIL;;
    }

    testObsResult = &tgfIpCapacityTestsObservedResults[testNum];

    cpssOsPrintf("Result for %s\n", prvTgfIpCapacityTestsNameGet(testNum));
    cpssOsPrintf("              Shr=N,Pbr=Y  Shr=Y,Pbr=Y  Shr=N,Pbr=N  Shr=Y,Pbr=N\n");
    cpssOsPrintf("    Expected: %8d      %8d      %8d      %8d\n",
                 testExpResult->expAddShrNoPbrYes, testExpResult->expAddShrYesPbrYes,
                 testExpResult->expAddShrNoPbrNo,  testExpResult->expAddShrYesPbrNo);
    cpssOsPrintf("    Observed: %8d      %8d      %8d      %8d\n\n",
                 testObsResult->expAddShrNoPbrYes, testObsResult->expAddShrYesPbrYes,
                 testObsResult->expAddShrNoPbrNo,  testObsResult->expAddShrYesPbrNo);
    return GT_OK;
}

/**
 * @internal prvTgfIpCapacityTestsResultsDumpReport function
 *
 * @brief This function prints expected and observed values
 *        for the all capacity tests
 *
 * @return GT_OK   - on success
 *         GT_FAIL - on failure.
 */
GT_STATUS prvTgfIpCapacityTestsResultsDumpReport
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_IP_CAPACITY_TESTS_ENT i;

    for (i=0; i < PRV_TGF_IP_CAPACITY_TESTS__LAST_E; i++)
    {
        rc = prvTgfIpCapacityTestsResultsCurrentDump(i);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal prvTgfCapacityTestsExpectedResultsExpectedValueGet function
 * @endinternal
 *
 * @brief Get expected value for the current test and test mode.
 *
 * @param[in] testNum          - capacity test enum value
 * @param[in] isSharingAllowed - GT_TRUE/GT_FALSE is sharing of banks allowed
 * @param[in] isWithPbr        - GT_TRUE/GT_FALSE - with/without pbr entries
 * @param[out] expectedValuePtr - pointer to expected value
 *
 * @return GT_OK   - on success
 *         GT_FAIL - on failure
 */

GT_STATUS prvTgfCapacityTestsExpectedResultsExpectedValueGet
(
    PRV_TGF_IP_CAPACITY_TESTS_ENT testNum,
    GT_BOOL                       isSharingAllowed,
    GT_BOOL                       isWithPbr,
    GT_U32                        *expectedValPtr
)
{
    PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC *testExpResult = NULL;

    if (testNum >= PRV_TGF_IP_CAPACITY_TESTS__LAST_E)
    {
        return GT_FAIL;
    }

    testExpResult = prvTgfIpExpectedResultsArrayGet(testNum);
    if (testExpResult == NULL)
    {
        return GT_FAIL;;
    }

    if ((isSharingAllowed == GT_FALSE) && (isWithPbr == GT_TRUE))
    {
        *expectedValPtr = testExpResult->expAddShrNoPbrYes;
    }
    else if ((isSharingAllowed == GT_TRUE) && (isWithPbr == GT_TRUE))
    {
        *expectedValPtr = testExpResult->expAddShrYesPbrYes;
    }
    else if ((isSharingAllowed == GT_FALSE) && (isWithPbr == GT_FALSE))
    {
        *expectedValPtr = testExpResult->expAddShrNoPbrNo;
    }
    else if ((isSharingAllowed == GT_TRUE) && (isWithPbr == GT_FALSE))
    {
        *expectedValPtr = testExpResult->expAddShrYesPbrNo;
    }
    else
    {
        return GT_FAIL;
    }
    return GT_OK;
}

/**
 * @internal prvTgfCapacityTestsExpectedResultsObservedValueSet function
 * @endinternal
 *
 * @brief Store observed value for the current test and test mode.
 *
 * @param[in] testNum          - capacity test enum value
 * @param[in] isSharingAllowed - GT_TRUE/GT_FALSE is sharing of banks allowed
 * @param[in] isWithPbr        - GT_TRUE/GT_FALSE - with/without pbr entries
 * @param[in] observedVal      - observed value
 *
 * @return GT_OK   - on success
 *         GT_FAIL - on failure
 */
GT_STATUS prvTgfCapacityTestsExpectedResultsObservedValueSet
(
    PRV_TGF_IP_CAPACITY_TESTS_ENT testNum,
    GT_BOOL                       isSharingAllowed,
    GT_BOOL                       isWithPbr,
    GT_U32                        observedVal
)
{
    PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC *testObsResult;

    if (testNum >= PRV_TGF_IP_CAPACITY_TESTS__LAST_E)
    {
        return GT_FAIL;
    }

    testObsResult = &tgfIpCapacityTestsObservedResults[testNum];

    if ((isSharingAllowed == GT_FALSE) && (isWithPbr == GT_TRUE))
    {
        testObsResult->expAddShrNoPbrYes = observedVal;
    }
    else if ((isSharingAllowed == GT_TRUE) && (isWithPbr == GT_TRUE))
    {
        testObsResult->expAddShrYesPbrYes = observedVal;
    }
    else if ((isSharingAllowed == GT_FALSE) && (isWithPbr == GT_FALSE))
    {
        testObsResult->expAddShrNoPbrNo = observedVal;
    }
    else if ((isSharingAllowed == GT_TRUE) && (isWithPbr == GT_FALSE))
    {
        testObsResult->expAddShrYesPbrNo = observedVal;
    }
    else
    {
        return GT_FAIL;
    }
    return GT_OK;
}
