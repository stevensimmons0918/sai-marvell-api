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
* @file prvTgfIpCapacityTestsExpectedResults.h
*
* @brief Store expected results for capacity tests
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpCapacityTestsExpectedResultsh
#define __prvTgfIpCapacityTestsExpectedResultsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @enum PRV_TGF_IP_CAPACITY_TESTS_ENT
 *
 * @brief enumerator for each cpacity test
 */
typedef enum{
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddManyIpLpmMng_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddRealPrefixes_2_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4UcRoutingAddDeleteAddRealPrefixes_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv6UcRoutingAddManyIpLpmMng_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4Ipv6UcRoutingAddManyIpLpmMng_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_E,
    PRV_TGF_IP_CAPACITY_TESTS_tgfIpv4McRoutingAddRealPrefixes_2_E,
    PRV_TGF_IP_CAPACITY_TESTS__LAST_E
} PRV_TGF_IP_CAPACITY_TESTS_ENT;

/**
 * @struct PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC
 *
 * @brief A structure to hold expected values for different modes.
 */
typedef struct PRV_TGF_IP_CAPACITY_TESTS_TYPE_STCT {

    /** expected value for mode: 1. do not allow block sharing and
     *                           2. use max PBR entries
     */
    GT_U32      expAddShrNoPbrYes;

    /** expected value for mode: 1. allow block sharing and
     *                           2. use max PBR entries
     */
    GT_U32      expAddShrYesPbrYes;

    /** expected value for mode: 1. do not allow block sharing and
     *                           2. no PBR entries
     */
    GT_U32      expAddShrNoPbrNo;

    /** expected value for mode: 1. allow block sharing and
     *                           2. no PBR entries
     */
    GT_U32      expAddShrYesPbrNo;
} PRV_TGF_IP_CAPACITY_TESTS_TYPE_STC;


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
    IN  PRV_TGF_IP_CAPACITY_TESTS_ENT testNum
);

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
);

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
    IN  PRV_TGF_IP_CAPACITY_TESTS_ENT testNum,
    IN  GT_BOOL                       isSharingAllowed,
    IN  GT_BOOL                       isWithPbr,
    OUT GT_U32                        *expectedValPtr
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpCapacityTestsExpectedResultsh */
