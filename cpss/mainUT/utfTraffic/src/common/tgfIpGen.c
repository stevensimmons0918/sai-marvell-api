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
* @file tgfIpGen.c
*
* @brief Generic API implementation for IP
*
* @version   96
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfIpGen.h>
#include <common/tgfLpmGen.h>
#include <gtOs/gtOsMem.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* default Virtual Router ID */
static GT_U32        prvTgfDefVrId = 0;

/* default LPM DB and Virtual Router configuration */
static struct
{
    GT_BOOL                                     isSupportIpv4Uc;
    GT_BOOL                                     isSupportIpv6Uc;
    GT_BOOL                                     isSupportIpv4Mc;
    GT_BOOL                                     isSupportIpv6Mc;
    GT_BOOL                                     isSupportFcoe;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv6McRouteLttEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defFcoeNextHopInfo;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    GT_BOOL                                     partitionEnable;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    GT_BOOL                                     isTcamManagerInternal;
} prvTgfDefLpmDBVrCfg;

/* Flag whether to make IP validation (default: GT_TRUE) */
static GT_BOOL prvTgfIpValidityCheckFlag = GT_TRUE;


/******************************************************************************\
 *                             Define section                                 *
\******************************************************************************/

/* total number of counters */
#define PRV_TGF_MAX_COUNTER_NUM_CNS    35

/* max counter field's name length */
#define PRV_TGF_MAX_FIELD_NAME_LEN_CNS 25

/* max number of LPM blocks on eArch architecture */
#define PRV_TGF_MAX_NUM_OF_LPM_BLOCKS_CNS    20

/* return number of LPM lines in ram according to number of prefixes supported.
   We use the rule of average 10Bytes for Ipv4 prefix and 20Bytes for Ipv6 prefix.
   each line holds 4 byte so one prefix hold 10 Bytes = 2.5 Lines. */
#define PRV_TGF_LPM_RAM_GET_NUM_OF_LINES_MAC(_numOfPrefixes)        \
    ((GT_U32)(_numOfPrefixes*2.5))

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY

#define ipv6_is_bigger_or_equal_MAC( IPv6_addr1_PTR , IPv6_addr2_PTR)                 \
    (cpssOsMemCmp(IPv6_addr1_PTR, IPv6_addr2_PTR, 16) >= 0 )

#define ipv6_is_smaller_or_equal_MAC( IPv6_addr1_PTR , IPv6_addr2_PTR)                 \
    (cpssOsMemCmp(IPv6_addr1_PTR, IPv6_addr2_PTR, 16) <= 0 )

typedef union
{
    GT_U32 Word;
    GT_U8  Bytes[4];
} CPSS_DXCH_IP_CONVERT_UNT;

GT_U8 IPv4Step[4]  = {1,1,1,1};
GT_U8 IPv6Step[16]  = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
GT_U8 srcIPv4Step[4]  = {1,1,1,1};
GT_U8 srcIPv6Step[16]  = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
GT_U32 IPv4PrefixLength = 32;
GT_U32 IPv6PrefixLength = 128;
GT_U32 srcIPv4PrefixLength = 32;
GT_U32 srcIPv6PrefixLength = 128;

/* flag to be used when we want to change the way many prefixes are
   added to the device
   GT_TRUE  : defragmentation is done - using shrink and merge mechanism
   GT_FALSE : defragmentation is NOT done */
GT_BOOL defragmentationEnableFlag = GT_TRUE;



/**
* @internal outputIPAddress function
* @endinternal
*
* @brief   Print Ip address
*
* @param[in] ipAddrPtr                - ip address
* @param[in] bytes2Print              - amount of printed bytes
* @param[in] isIpv4                   - Ipv4 or Ipv6 address
*/
static GT_VOID outputIPAddress
(
    IN GT_U8 *ipAddrPtr,
    IN GT_U32 bytes2Print,
    IN GT_BOOL isIpv4
)
{
    while(bytes2Print > 1)
    {
        if (GT_TRUE == isIpv4)
        {
            cpssOsPrintf("%03d.%03d.", *ipAddrPtr, *(ipAddrPtr+1));
        }
        else
        {
            cpssOsPrintf("%02X%02X:", *ipAddrPtr, *(ipAddrPtr+1));
        }

        ipAddrPtr+=2;
        bytes2Print-=2;
    }

    if (bytes2Print > 0)
    {
        if (GT_TRUE == isIpv4)
        {
            cpssOsPrintf("%03d", *ipAddrPtr);
        }
        else
        {
            cpssOsPrintf("%02X", *ipAddrPtr);
        }
    }
}

/**
* @internal prvCpssDxChIpLpmDbgConvertRouteType function
* @endinternal
*
* @brief   Convert CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E to
*         CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E for RAM based LPM whenever input
*         type is CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E and numOfPaths is 0
* @param[in] lpmDBId                  - LPM DB id
* @param[in,out] ipLttEntryPtr            - (pointer to) the original IP LTT entry
* @param[in,out] ipLttEntryPtr            - (pointer to) the changed IP LTT entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if can't find the lpm DB
* @retval GT_BAD_PTR               - on null pointer or if shadowType is uninitialized
*/
static GT_STATUS prvCpssDxChIpLpmDbgConvertRouteType
(
    IN    GT_U32                          lpmDBId,
    INOUT CPSS_DXCH_IP_LTT_ENTRY_STC      *ipLttEntryPtr
)
{
    GT_STATUS                                       status;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              memoryCfg;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    tcamLpmManagerCapcityCfg;
    GT_PTR                                          tcamManagerHandler;

    memoryCfg.tcamDbCfg.indexesRangePtr = &indexesRange;
    memoryCfg.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
    memoryCfg.tcamDbCfg.tcamManagerHandlerPtr = &tcamManagerHandler;
    status = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &memoryCfg);
    if (status != GT_OK)
    {
        return status;
    }

    if (ipLttEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E) &&
        (ipLttEntryPtr->routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E) &&
        (ipLttEntryPtr->numOfPaths == 0))
    {
        ipLttEntryPtr->routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E) &&
        (ipLttEntryPtr->routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) &&
        (ipLttEntryPtr->numOfPaths == 0))
    {
        ipLttEntryPtr->routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }


    return GT_OK;
}


/**
* @internal internal_cpssDxChIpLpmDbgIpv4SetStep function
* @endinternal
*
* @brief   This function set the step size for each of the 4 IPv4 octets, and the prefix length.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] stepArr[4]               - step size array for IPv4 UC Destination or IPv4 MC Group
* @param[in] prefixLength             - prefix length for IPv4 UC Destination or IPv4 MC Group
* @param[in] srcStepArr[4]            - step size array for IPv4 source
* @param[in] srcPrefixLen             - prefix length for IPv4 source
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
*/
GT_STATUS tgfCpssDxChIpLpmDbgIpv4SetStep
(
    IN GT_U8  stepArr[4],
    IN GT_U32 prefixLength,
    IN GT_U8  srcStepArr[4],
    IN GT_U32 srcPrefixLen
)
{
    GT_U32 i;
    if (stepArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i = 0; i < 4; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32 || srcStepArr[i] < 1 || srcStepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (prefixLength > 32 || srcPrefixLen > 32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    IPv4PrefixLength = prefixLength;
    srcIPv4PrefixLength = srcPrefixLen;

    for (i = 0; i < 4; i++)
    {
        IPv4Step[i] = stepArr[i];
        srcIPv4Step[i] = srcStepArr[i];
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6SetStep function
* @endinternal
*
* @brief   This function set the step size for each of the 16 IPv6 octets, and the prefix length.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] stepArr[16]              - step size array for IPv6 UC Destination or IPv6 MC Group
* @param[in] prefixLength             - prefix length for IPv6 UC Destination or IPv6 MC Group
* @param[in] srcStepArr[16]           - step size for IPv6 source
* @param[in] srcPrefixLen             - prefix length for IPv6 source
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
*/
GT_STATUS tgfCpssDxChIpLpmDbgIpv6SetStep
(
    IN GT_U8  stepArr[16],
    IN GT_U32 prefixLength,
    IN GT_U8 srcStepArr[16],
    IN GT_U32 srcPrefixLen
)
{
    GT_U32 i;
    if (stepArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i = 0; i < 16; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32 || srcStepArr[i] < 1 || srcStepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (prefixLength > 128 || srcPrefixLen > 128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    IPv6PrefixLength = prefixLength;
    srcIPv6PrefixLength = srcPrefixLen;

    for (i = 0; i < 16; i++)
    {
        IPv6Step[i] = stepArr[i];
        srcIPv6Step[i] = srcStepArr[i];
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr        - (pointer to)the first group address to add
* @param[in] srcStartIpAddr        - (pointer to)the first source address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrAddedPtr    - points to the last group prefix successfully
*                                      added (NULL to ignore)
* @param[out] srcLastIpAddrAddedPtr    - points to the last source prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *grpStartIpAddr,
    IN  GT_IPV6ADDR *srcStartIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_IPV6ADDR *grpLastIpAddrAddedPtr,
    OUT GT_IPV6ADDR *srcLastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             grpCurrentIpAddr;
    GT_IPV6ADDR                             srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPV6ADDR                             grpLastIpAddrAdded;
    GT_IPV6ADDR                             srcLastIpAddrAdded;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_BOOL                                 defragmentationEnable=defragmentationEnableFlag;
    GT_U32                                  srcIPv6Val[16] = { 0 };
    GT_U32                                  grpIPv6Val[16] = { 0 };

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT         protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT memoryCfg;

    cpssOsMemSet(octetsOrderToChangeArray, 0, sizeof(octetsOrderToChangeArray));

    rc = cpssDxChIpLpmDBConfigGet(lpmDbId, &shadowType, &protocolStack, &memoryCfg);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E) ||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E) )
    {
        mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
        mcRouteLttEntry.priority = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E;
    }

    mcRouteLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    mcRouteLttEntry.numOfPaths          = 0;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
        octetsOrderToChangeArray[4] = grpFifthOctetToChange;
        octetsOrderToChangeArray[5] = grpSixthOctetToChange;
        octetsOrderToChangeArray[6] = grpSeventhOctetToChange;
        octetsOrderToChangeArray[7] = grpEighthOctetToChange;
        octetsOrderToChangeArray[8] = grpNinthOctetToChange;
        octetsOrderToChangeArray[9] = grpTenthOctetToChange;
        octetsOrderToChangeArray[10] = grpElevenOctetToChange;
        octetsOrderToChangeArray[11] = grpTwelveOctetToChange;
        octetsOrderToChangeArray[12] = grpThirteenOctetToChange;
        octetsOrderToChangeArray[13] = grpFourteenOctetToChange;
        octetsOrderToChangeArray[14] = grpFifteenOctetToChange;
        octetsOrderToChangeArray[15] = grpSixteenOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
        octetsOrderToChangeArray[4] = srcFifthOctetToChange;
        octetsOrderToChangeArray[5] = srcSixthOctetToChange;
        octetsOrderToChangeArray[6] = srcSeventhOctetToChange;
        octetsOrderToChangeArray[7] = srcEighthOctetToChange;
        octetsOrderToChangeArray[8] = srcNinthOctetToChange;
        octetsOrderToChangeArray[9] = srcTenthOctetToChange;
        octetsOrderToChangeArray[10] = srcElevenOctetToChange;
        octetsOrderToChangeArray[11] = srcTwelveOctetToChange;
        octetsOrderToChangeArray[12] = srcThirteenOctetToChange;
        octetsOrderToChangeArray[13] = srcFourteenOctetToChange;
        octetsOrderToChangeArray[14] = srcFifteenOctetToChange;
        octetsOrderToChangeArray[15] = srcSixteenOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            cpssOsPrintf("octetToChange can not be bigger then 15\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(&grpCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&srcCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        grpCurrentIpAddr.u32Ip[j]      = grpStartIpAddr->u32Ip[j];
        grpLastIpAddrAdded.u32Ip[j]    = grpStartIpAddr->u32Ip[j];

        srcCurrentIpAddr.u32Ip[j]      = srcStartIpAddr->u32Ip[j];
        srcLastIpAddrAdded.u32Ip[j]    = srcStartIpAddr->u32Ip[j];
    }

   for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF
           and the src prefix is not a MC prefix */
        if((grpCurrentIpAddr.arIP[0] == 0xFF)&&
           (srcCurrentIpAddr.arIP[0] != 0xFF))
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv6McEntryAdd(lpmDbId,
                                            vrId,
                                            &grpCurrentIpAddr,
                                            IPv6PrefixLength,
                                            &srcCurrentIpAddr,
                                            srcIPv6PrefixLength,
                                            &mcRouteLttEntry,
                                            override,
                                            defragmentationEnable);

            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;

            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" is not a valid MC address\n");

             /* print result */
            cpssOsPrintf("\nIpv6McPrefixAddMany results:");
            if (numOfPrefixesAdded == 0)
            {
                cpssOsPrintf("\n    No prefixes were added at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
                cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
                cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

            }

            /* update output parameters */
            if (numOfPrefixesAddedPtr != NULL)
                *numOfPrefixesAddedPtr = numOfPrefixesAdded;
            if (grpLastIpAddrAddedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    grpLastIpAddrAddedPtr->u32Ip[j] = grpLastIpAddrAdded.u32Ip[j];
                }
            }

            if (srcLastIpAddrAddedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    srcLastIpAddrAddedPtr->u32Ip[j] = srcLastIpAddrAdded.u32Ip[j];
                }
            }

            /* Return to default */
            for (i = 0; i < 16; i++) {
                IPv6Step[i] = 1;
                srcIPv6Step[i] = 1;
            }
            IPv6PrefixLength = 128;
            srcIPv6PrefixLength = 128;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        for(j=0;j<4;j++)
        {
            grpLastIpAddrAdded.u32Ip[j]    = grpCurrentIpAddr.u32Ip[j];
            srcLastIpAddrAdded.u32Ip[j]    = srcCurrentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            srcIPv6Val[j]    = srcCurrentIpAddr.arIP[j];
            grpIPv6Val[j]    = grpCurrentIpAddr.arIP[j];
        }

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv6Val[srcFirstOctetToChange] + srcIPv6Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv6Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv6Val[srcSecondOctetToChange] + srcIPv6Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv6Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv6Val[srcThirdOctetToChange] + srcIPv6Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv6Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv6Val[srcFourthOctetToChange] + srcIPv6Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv6Step[srcFourthOctetToChange];
                        else
                        {
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] = 0;
                            if (srcIPv6Val[srcFifthOctetToChange] + srcIPv6Step[srcFifthOctetToChange] <= 255)
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] += srcIPv6Step[srcFifthOctetToChange];
                            else
                            {
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] = 0;
                                if (srcIPv6Val[srcSixthOctetToChange] + srcIPv6Step[srcSixthOctetToChange] <= 255)
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] += srcIPv6Step[srcSixthOctetToChange];
                                else
                                {
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] = 0;
                                    if (srcIPv6Val[srcSeventhOctetToChange] + srcIPv6Step[srcSeventhOctetToChange] <= 255)
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] += srcIPv6Step[srcSeventhOctetToChange];
                                    else
                                    {
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] = 0;
                                        if (srcIPv6Val[srcEighthOctetToChange] + srcIPv6Step[srcEighthOctetToChange] <= 255)
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] += srcIPv6Step[srcEighthOctetToChange];
                                        else
                                        {
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] = 0;
                                            if (srcIPv6Val[srcNinthOctetToChange] + srcIPv6Step[srcNinthOctetToChange] <= 255)
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] += srcIPv6Step[srcNinthOctetToChange];
                                            else
                                            {
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] = 0;
                                                if (srcIPv6Val[srcTenthOctetToChange] + srcIPv6Step[srcTenthOctetToChange] <= 255)
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] += srcIPv6Step[srcTenthOctetToChange];
                                                else
                                                {
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] = 0;
                                                    if (srcIPv6Val[srcElevenOctetToChange] + srcIPv6Step[srcElevenOctetToChange] <= 255)
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] += srcIPv6Step[srcElevenOctetToChange];
                                                    else
                                                    {
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] = 0;
                                                        if (srcIPv6Val[srcTwelveOctetToChange] + srcIPv6Step[srcTwelveOctetToChange] <= 255)
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] += srcIPv6Step[srcTwelveOctetToChange];
                                                        else
                                                        {
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] = 0;
                                                            if (srcIPv6Val[srcThirteenOctetToChange] + srcIPv6Step[srcThirteenOctetToChange] <= 255)
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] += srcIPv6Step[srcThirteenOctetToChange];
                                                            else
                                                            {
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] = 0;
                                                                if (srcIPv6Val[srcFourteenOctetToChange] + srcIPv6Step[srcFourteenOctetToChange] <= 255)
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] += srcIPv6Step[srcFourteenOctetToChange];
                                                                else
                                                                {
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] = 0;
                                                                    if (srcIPv6Val[srcFifteenOctetToChange] + srcIPv6Step[srcFifteenOctetToChange] <= 255)
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] += srcIPv6Step[srcFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] = 0;
                                                                        if (srcIPv6Val[srcSixteenOctetToChange] + srcIPv6Step[srcSixteenOctetToChange] <= 255)
                                                                            srcCurrentIpAddr.arIP[srcSixteenOctetToChange] += srcIPv6Step[srcSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv6Val[grpFirstOctetToChange] + IPv6Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv6Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv6Val[grpSecondOctetToChange] + IPv6Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv6Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv6Val[grpThirdOctetToChange] + IPv6Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv6Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv6Val[grpFourthOctetToChange] + IPv6Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv6Step[grpFourthOctetToChange];
                        else
                        {
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] = 0;
                            if (grpIPv6Val[grpFifthOctetToChange] + IPv6Step[grpFifthOctetToChange] <= 255)
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] += IPv6Step[grpFifthOctetToChange];
                            else
                            {
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] = 0;
                                if (grpIPv6Val[grpSixthOctetToChange] + IPv6Step[grpSixthOctetToChange] <= 255)
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] += IPv6Step[grpSixthOctetToChange];
                                else
                                {
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] = 0;
                                    if (grpIPv6Val[grpSeventhOctetToChange] + IPv6Step[grpSeventhOctetToChange] <= 255)
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] += IPv6Step[grpSeventhOctetToChange];
                                    else
                                    {
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] = 0;
                                        if (grpIPv6Val[grpEighthOctetToChange] + IPv6Step[grpEighthOctetToChange] <= 255)
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] += IPv6Step[grpEighthOctetToChange];
                                        else
                                        {
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] = 0;
                                            if (grpIPv6Val[grpNinthOctetToChange] + IPv6Step[grpNinthOctetToChange] <= 255)
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] += IPv6Step[grpNinthOctetToChange];
                                            else
                                            {
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] = 0;
                                                if (grpIPv6Val[grpTenthOctetToChange] + IPv6Step[grpTenthOctetToChange] <= 255)
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] += IPv6Step[grpTenthOctetToChange];
                                                else
                                                {
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] = 0;
                                                    if (grpIPv6Val[grpElevenOctetToChange] + IPv6Step[grpElevenOctetToChange] <= 255)
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] += IPv6Step[grpElevenOctetToChange];
                                                    else
                                                    {
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] = 0;
                                                        if (grpIPv6Val[grpTwelveOctetToChange] + IPv6Step[grpTwelveOctetToChange] <= 255)
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] += IPv6Step[grpTwelveOctetToChange];
                                                        else
                                                        {
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] = 0;
                                                            if (grpIPv6Val[grpThirteenOctetToChange] + IPv6Step[grpThirteenOctetToChange] <= 255)
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] += IPv6Step[grpThirteenOctetToChange];
                                                            else
                                                            {
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] = 0;
                                                                if (grpIPv6Val[grpFourteenOctetToChange] + IPv6Step[grpFourteenOctetToChange] <= 255)
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] += IPv6Step[grpFourteenOctetToChange];
                                                                else
                                                                {
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] = 0;
                                                                    if (grpIPv6Val[grpFifteenOctetToChange] + IPv6Step[grpFifteenOctetToChange] <= 255)
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] += IPv6Step[grpFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] = 0;
                                                                        if (grpIPv6Val[grpSixteenOctetToChange] + IPv6Step[grpSixteenOctetToChange] <= 255)
                                                                            grpCurrentIpAddr.arIP[grpSixteenOctetToChange] += IPv6Step[grpSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6McPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (grpLastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            grpLastIpAddrAddedPtr->u32Ip[j] = grpLastIpAddrAdded.u32Ip[j];
        }
    }

    if (srcLastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            srcLastIpAddrAddedPtr->u32Ip[j] = srcLastIpAddrAdded.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}



/**
* @internal internal_cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to deleted
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of prefixes
*       that was added by cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_U32                                  numOfMcPrefixesNotDeleted = 0;
    GT_IPV6ADDR                             lastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  IPv6Val[16] = { 0 };

    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        currentIpAddr.u32Ip[j]        = startIpAddr->u32Ip[j];
        lastIpAddrDeleted.u32Ip[j]    = startIpAddr->u32Ip[j];
    }


    octetsOrderToChangeArray[0] = firstOctetToChange;
    octetsOrderToChangeArray[1] = secondOctetToChange;
    octetsOrderToChangeArray[2] = thirdOctetToChange;
    octetsOrderToChangeArray[3] = fourthOctetToChange;
    octetsOrderToChangeArray[4] = fifthOctetToChange;
    octetsOrderToChangeArray[5] = sixthOctetToChange;
    octetsOrderToChangeArray[6] = seventhOctetToChange;
    octetsOrderToChangeArray[7] = eighthOctetToChange;
    octetsOrderToChangeArray[8] = ninthOctetToChange;
    octetsOrderToChangeArray[9] = tenthOctetToChange;
    octetsOrderToChangeArray[10] = elevenOctetToChange;
    octetsOrderToChangeArray[11] = twelveOctetToChange;
    octetsOrderToChangeArray[12] = thirteenOctetToChange;
    octetsOrderToChangeArray[13] = fourteenOctetToChange;
    octetsOrderToChangeArray[14] = fifteenOctetToChange;
    octetsOrderToChangeArray[15] = sixteenOctetToChange;

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {

            /* delete the current prefix */
            rc = cpssDxChIpLpmIpv6UcPrefixDel(lpmDbId,
                                              vrId,
                                              &currentIpAddr,
                                              IPv6PrefixLength);

            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
            else
                numOfPrefixesDeleted++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotDeleted++;
        }

        for(j=0;j<4;j++)
        {
            lastIpAddrDeleted.u32Ip[j]    = currentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            IPv6Val[j]    = currentIpAddr.arIP[j];
        }

        /* advance to the next IP address */
        if (IPv6Val[firstOctetToChange] + IPv6Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv6Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv6Val[secondOctetToChange] + IPv6Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv6Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv6Val[thirdOctetToChange] + IPv6Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv6Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv6Val[fourthOctetToChange] + IPv6Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv6Step[fourthOctetToChange];
                    else
                    {
                        currentIpAddr.arIP[fourthOctetToChange] = 0;
                        if (IPv6Val[fifthOctetToChange] + IPv6Step[fifthOctetToChange] <= 255)
                            currentIpAddr.arIP[fifthOctetToChange] += IPv6Step[fifthOctetToChange];
                        else
                        {
                            currentIpAddr.arIP[fifthOctetToChange] = 0;
                            if (IPv6Val[sixthOctetToChange] + IPv6Step[sixthOctetToChange] <= 255)
                                currentIpAddr.arIP[sixthOctetToChange] += IPv6Step[sixthOctetToChange];
                            else
                            {
                                currentIpAddr.arIP[sixthOctetToChange] = 0;
                                if (IPv6Val[seventhOctetToChange] + IPv6Step[seventhOctetToChange] <= 255)
                                    currentIpAddr.arIP[seventhOctetToChange] += IPv6Step[seventhOctetToChange];
                                else
                                {
                                    currentIpAddr.arIP[seventhOctetToChange] = 0;
                                    if (IPv6Val[eighthOctetToChange] + IPv6Step[eighthOctetToChange] <= 255)
                                        currentIpAddr.arIP[eighthOctetToChange] += IPv6Step[eighthOctetToChange];
                                    else
                                    {
                                        currentIpAddr.arIP[eighthOctetToChange] = 0;
                                        if (IPv6Val[ninthOctetToChange] + IPv6Step[ninthOctetToChange] <= 255)
                                            currentIpAddr.arIP[ninthOctetToChange] += IPv6Step[ninthOctetToChange];
                                        else
                                        {
                                            currentIpAddr.arIP[ninthOctetToChange] = 0;
                                            if (IPv6Val[tenthOctetToChange] + IPv6Step[tenthOctetToChange] <= 255)
                                                currentIpAddr.arIP[tenthOctetToChange] += IPv6Step[tenthOctetToChange];
                                            else
                                            {
                                                currentIpAddr.arIP[tenthOctetToChange] = 0;
                                                if (IPv6Val[elevenOctetToChange] + IPv6Step[elevenOctetToChange] <= 255)
                                                    currentIpAddr.arIP[elevenOctetToChange] += IPv6Step[elevenOctetToChange];
                                                else
                                                {
                                                    currentIpAddr.arIP[elevenOctetToChange] = 0;
                                                    if (IPv6Val[twelveOctetToChange] + IPv6Step[twelveOctetToChange] <= 255)
                                                        currentIpAddr.arIP[twelveOctetToChange] += IPv6Step[twelveOctetToChange];
                                                    else
                                                    {
                                                        currentIpAddr.arIP[twelveOctetToChange] = 0;
                                                        if (IPv6Val[thirteenOctetToChange] + IPv6Step[thirteenOctetToChange] <= 255)
                                                            currentIpAddr.arIP[thirteenOctetToChange] += IPv6Step[thirteenOctetToChange];
                                                        else
                                                        {
                                                            currentIpAddr.arIP[thirteenOctetToChange] = 0;
                                                            if (IPv6Val[fourteenOctetToChange] + IPv6Step[fourteenOctetToChange] <= 255)
                                                                currentIpAddr.arIP[fourteenOctetToChange] += IPv6Step[fourteenOctetToChange];
                                                            else
                                                            {
                                                                currentIpAddr.arIP[fourteenOctetToChange] = 0;
                                                                if (IPv6Val[fifteenOctetToChange] + IPv6Step[fifteenOctetToChange] <= 255)
                                                                    currentIpAddr.arIP[fifteenOctetToChange] += IPv6Step[fifteenOctetToChange];
                                                                else
                                                                {
                                                                    currentIpAddr.arIP[fifteenOctetToChange] = 0;
                                                                    if (IPv6Val[sixteenOctetToChange] + IPv6Step[sixteenOctetToChange] <= 255)
                                                                        currentIpAddr.arIP[sixteenOctetToChange] += IPv6Step[sixteenOctetToChange];
                                                                    else
                                                                    {
                                                                        rc = GT_NO_MORE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixDelMany statistics\n");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("    No prefixes were Deleted : 0\n");
    }
    else
    {
        cpssOsPrintf("    Prefix Range :   ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Deleted Prefixes             : %d/%d\n", numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    MC prefixes not Deleted      : %d\n", numOfMcPrefixesNotDeleted);
        cpssOsPrintf("    Already deleted              : %d\n", numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    Total deleted plus not_found : %d/%d\n", (numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (lastIpAddrDeletedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            lastIpAddrDeletedPtr->u32Ip[j] = lastIpAddrDeleted.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr     - (pointer to)the first group address to delete
* @param[in] srcStartIpAddr     - (pointer to)the first source address to delete
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrDeletedPtr  - points to the last group prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] srcLastIpAddrDeletedPtr  - points to the last source prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be deleted. All the prefixes are deleted with exact match (prefix
*       length 128).
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *grpStartIpAddr,
    IN  GT_IPV6ADDR *srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_IPV6ADDR *grpLastIpAddrDeletedPtr,
    OUT GT_IPV6ADDR *srcLastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             grpCurrentIpAddr;
    GT_IPV6ADDR                             srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_IPV6ADDR                             grpLastIpAddrDeleted;
    GT_IPV6ADDR                             srcLastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  srcIPv6Val[16] = { 0 };
    GT_U32                                  grpIPv6Val[16] = { 0 };

    cpssOsMemSet(octetsOrderToChangeArray, 0, sizeof(octetsOrderToChangeArray));

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
        octetsOrderToChangeArray[4] = grpFifthOctetToChange;
        octetsOrderToChangeArray[5] = grpSixthOctetToChange;
        octetsOrderToChangeArray[6] = grpSeventhOctetToChange;
        octetsOrderToChangeArray[7] = grpEighthOctetToChange;
        octetsOrderToChangeArray[8] = grpNinthOctetToChange;
        octetsOrderToChangeArray[9] = grpTenthOctetToChange;
        octetsOrderToChangeArray[10] = grpElevenOctetToChange;
        octetsOrderToChangeArray[11] = grpTwelveOctetToChange;
        octetsOrderToChangeArray[12] = grpThirteenOctetToChange;
        octetsOrderToChangeArray[13] = grpFourteenOctetToChange;
        octetsOrderToChangeArray[14] = grpFifteenOctetToChange;
        octetsOrderToChangeArray[15] = grpSixteenOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
        octetsOrderToChangeArray[4] = srcFifthOctetToChange;
        octetsOrderToChangeArray[5] = srcSixthOctetToChange;
        octetsOrderToChangeArray[6] = srcSeventhOctetToChange;
        octetsOrderToChangeArray[7] = srcEighthOctetToChange;
        octetsOrderToChangeArray[8] = srcNinthOctetToChange;
        octetsOrderToChangeArray[9] = srcTenthOctetToChange;
        octetsOrderToChangeArray[10] = srcElevenOctetToChange;
        octetsOrderToChangeArray[11] = srcTwelveOctetToChange;
        octetsOrderToChangeArray[12] = srcThirteenOctetToChange;
        octetsOrderToChangeArray[13] = srcFourteenOctetToChange;
        octetsOrderToChangeArray[14] = srcFifteenOctetToChange;
        octetsOrderToChangeArray[15] = srcSixteenOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            cpssOsPrintf("octetToChange can not be bigger then 15\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(&grpCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&srcCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        grpCurrentIpAddr.u32Ip[j]      = grpStartIpAddr->u32Ip[j];
        grpLastIpAddrDeleted.u32Ip[j]    = grpStartIpAddr->u32Ip[j];

        srcCurrentIpAddr.u32Ip[j]      = srcStartIpAddr->u32Ip[j];
        srcLastIpAddrDeleted.u32Ip[j]    = srcStartIpAddr->u32Ip[j];
    }

   for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF
           and the src prefix is not a MC prefix */
        if((grpCurrentIpAddr.arIP[0] == 0xFF)&&
           (srcCurrentIpAddr.arIP[0] != 0xFF))
        {
            /* delete the current prefix */
            rc = cpssDxChIpLpmIpv6McEntryDel(lpmDbId,
                                            vrId,
                                            &grpCurrentIpAddr,
                                            IPv6PrefixLength,
                                            &srcCurrentIpAddr,
                                            srcIPv6PrefixLength);
            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
            else
                numOfPrefixesDeleted++;
            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv6McPrefixAddMany results:");
            if (numOfPrefixesDeleted == 0)
            {
                cpssOsPrintf("\n    No prefixes were deleted at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
                cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
                cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

            }

            /* update output parameters */
            if (numOfPrefixesDeletedPtr != NULL)
                *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
            if (grpLastIpAddrDeletedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    grpLastIpAddrDeletedPtr->u32Ip[j] = grpLastIpAddrDeleted.u32Ip[j];
                }
            }

            if (srcLastIpAddrDeletedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    srcLastIpAddrDeletedPtr->u32Ip[j] = srcLastIpAddrDeleted.u32Ip[j];
                }
            }

            /* Return to default */
            for (i = 0; i < 16; i++) {
                IPv6Step[i] = 1;
                srcIPv6Step[i] = 1;
            }
            IPv6PrefixLength = 128;
            srcIPv6PrefixLength = 128;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        for(j=0;j<4;j++)
        {
            grpLastIpAddrDeleted.u32Ip[j]    = grpCurrentIpAddr.u32Ip[j];
            srcLastIpAddrDeleted.u32Ip[j]    = srcCurrentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            srcIPv6Val[j]    = srcCurrentIpAddr.arIP[j];
            grpIPv6Val[j]    = grpCurrentIpAddr.arIP[j];
        }

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv6Val[srcFirstOctetToChange] + srcIPv6Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv6Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv6Val[srcSecondOctetToChange] + srcIPv6Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv6Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv6Val[srcThirdOctetToChange] + srcIPv6Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv6Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv6Val[srcFourthOctetToChange] + srcIPv6Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv6Step[srcFourthOctetToChange];
                        else
                        {
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] = 0;
                            if (srcIPv6Val[srcFifthOctetToChange] + srcIPv6Step[srcFifthOctetToChange] <= 255)
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] += srcIPv6Step[srcFifthOctetToChange];
                            else
                            {
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] = 0;
                                if (srcIPv6Val[srcSixthOctetToChange] + srcIPv6Step[srcSixthOctetToChange] <= 255)
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] += srcIPv6Step[srcSixthOctetToChange];
                                else
                                {
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] = 0;
                                    if (srcIPv6Val[srcSeventhOctetToChange] + srcIPv6Step[srcSeventhOctetToChange] <= 255)
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] += srcIPv6Step[srcSeventhOctetToChange];
                                    else
                                    {
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] = 0;
                                        if (srcIPv6Val[srcEighthOctetToChange] + srcIPv6Step[srcEighthOctetToChange] <= 255)
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] += srcIPv6Step[srcEighthOctetToChange];
                                        else
                                        {
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] = 0;
                                            if (srcIPv6Val[srcNinthOctetToChange] + srcIPv6Step[srcNinthOctetToChange] <= 255)
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] += srcIPv6Step[srcNinthOctetToChange];
                                            else
                                            {
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] = 0;
                                                if (srcIPv6Val[srcTenthOctetToChange] + srcIPv6Step[srcTenthOctetToChange] <= 255)
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] += srcIPv6Step[srcTenthOctetToChange];
                                                else
                                                {
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] = 0;
                                                    if (srcIPv6Val[srcElevenOctetToChange] + srcIPv6Step[srcElevenOctetToChange] <= 255)
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] += srcIPv6Step[srcElevenOctetToChange];
                                                    else
                                                    {
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] = 0;
                                                        if (srcIPv6Val[srcTwelveOctetToChange] + srcIPv6Step[srcTwelveOctetToChange] <= 255)
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] += srcIPv6Step[srcTwelveOctetToChange];
                                                        else
                                                        {
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] = 0;
                                                            if (srcIPv6Val[srcThirteenOctetToChange] + srcIPv6Step[srcThirteenOctetToChange] <= 255)
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] += srcIPv6Step[srcThirteenOctetToChange];
                                                            else
                                                            {
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] = 0;
                                                                if (srcIPv6Val[srcFourteenOctetToChange] + srcIPv6Step[srcFourteenOctetToChange] <= 255)
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] += srcIPv6Step[srcFourteenOctetToChange];
                                                                else
                                                                {
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] = 0;
                                                                    if (srcIPv6Val[srcFifteenOctetToChange] + srcIPv6Step[srcFifteenOctetToChange] <= 255)
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] += srcIPv6Step[srcFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] = 0;
                                                                        if (srcIPv6Val[srcSixteenOctetToChange] + srcIPv6Step[srcSixteenOctetToChange] <= 255)
                                                                            srcCurrentIpAddr.arIP[srcSixteenOctetToChange] += srcIPv6Step[srcSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv6Val[grpFirstOctetToChange] + IPv6Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv6Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv6Val[grpSecondOctetToChange] + IPv6Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv6Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv6Val[grpThirdOctetToChange] + IPv6Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv6Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv6Val[grpFourthOctetToChange] + IPv6Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv6Step[grpFourthOctetToChange];
                        else
                        {
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] = 0;
                            if (grpIPv6Val[grpFifthOctetToChange] + IPv6Step[grpFifthOctetToChange] <= 255)
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] += IPv6Step[grpFifthOctetToChange];
                            else
                            {
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] = 0;
                                if (grpIPv6Val[grpSixthOctetToChange] + IPv6Step[grpSixthOctetToChange] <= 255)
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] += IPv6Step[grpSixthOctetToChange];
                                else
                                {
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] = 0;
                                    if (grpIPv6Val[grpSeventhOctetToChange] + IPv6Step[grpSeventhOctetToChange] <= 255)
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] += IPv6Step[grpSeventhOctetToChange];
                                    else
                                    {
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] = 0;
                                        if (grpIPv6Val[grpEighthOctetToChange] + IPv6Step[grpEighthOctetToChange] <= 255)
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] += IPv6Step[grpEighthOctetToChange];
                                        else
                                        {
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] = 0;
                                            if (grpIPv6Val[grpNinthOctetToChange] + IPv6Step[grpNinthOctetToChange] <= 255)
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] += IPv6Step[grpNinthOctetToChange];
                                            else
                                            {
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] = 0;
                                                if (grpIPv6Val[grpTenthOctetToChange] + IPv6Step[grpTenthOctetToChange] <= 255)
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] += IPv6Step[grpTenthOctetToChange];
                                                else
                                                {
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] = 0;
                                                    if (grpIPv6Val[grpElevenOctetToChange] + IPv6Step[grpElevenOctetToChange] <= 255)
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] += IPv6Step[grpElevenOctetToChange];
                                                    else
                                                    {
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] = 0;
                                                        if (grpIPv6Val[grpTwelveOctetToChange] + IPv6Step[grpTwelveOctetToChange] <= 255)
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] += IPv6Step[grpTwelveOctetToChange];
                                                        else
                                                        {
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] = 0;
                                                            if (grpIPv6Val[grpThirteenOctetToChange] + IPv6Step[grpThirteenOctetToChange] <= 255)
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] += IPv6Step[grpThirteenOctetToChange];
                                                            else
                                                            {
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] = 0;
                                                                if (grpIPv6Val[grpFourteenOctetToChange] + IPv6Step[grpFourteenOctetToChange] <= 255)
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] += IPv6Step[grpFourteenOctetToChange];
                                                                else
                                                                {
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] = 0;
                                                                    if (grpIPv6Val[grpFifteenOctetToChange] + IPv6Step[grpFifteenOctetToChange] <= 255)
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] += IPv6Step[grpFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] = 0;
                                                                        if (grpIPv6Val[grpSixteenOctetToChange] + IPv6Step[grpSixteenOctetToChange] <= 255)
                                                                            grpCurrentIpAddr.arIP[grpSixteenOctetToChange] += IPv6Step[grpSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6McPrefixAddMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (grpLastIpAddrDeletedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            grpLastIpAddrDeletedPtr->u32Ip[j] = grpLastIpAddrDeleted.u32Ip[j];
        }
    }

    if (srcLastIpAddrDeletedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            srcLastIpAddrDeletedPtr->u32Ip[j] = srcLastIpAddrDeleted.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPV6ADDR                             lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_BOOL                                 defragmentationEnable=defragmentationEnableFlag;
    GT_U32                                  IPv6Val[16] = { 0 };

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT         protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT memoryCfg;


    rc = cpssDxChIpLpmDBConfigGet(lpmDbId, &shadowType, &protocolStack, &memoryCfg);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    if (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        currentIpAddr.u32Ip[j]     = startIpAddr->u32Ip[j];
        lastIpAddrAdded.u32Ip[j]    = startIpAddr->u32Ip[j];
    }


    octetsOrderToChangeArray[0] = firstOctetToChange;
    octetsOrderToChangeArray[1] = secondOctetToChange;
    octetsOrderToChangeArray[2] = thirdOctetToChange;
    octetsOrderToChangeArray[3] = fourthOctetToChange;
    octetsOrderToChangeArray[4] = fifthOctetToChange;
    octetsOrderToChangeArray[5] = sixthOctetToChange;
    octetsOrderToChangeArray[6] = seventhOctetToChange;
    octetsOrderToChangeArray[7] = eighthOctetToChange;
    octetsOrderToChangeArray[8] = ninthOctetToChange;
    octetsOrderToChangeArray[9] = tenthOctetToChange;
    octetsOrderToChangeArray[10] = elevenOctetToChange;
    octetsOrderToChangeArray[11] = twelveOctetToChange;
    octetsOrderToChangeArray[12] = thirteenOctetToChange;
    octetsOrderToChangeArray[13] = fourteenOctetToChange;
    octetsOrderToChangeArray[14] = fifteenOctetToChange;
    octetsOrderToChangeArray[15] = sixteenOctetToChange;

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {

            /* add the current prefix */
            rc = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDbId,
                                              vrId,
                                              &currentIpAddr,
                                              IPv6PrefixLength,
                                              &nextHopInfo,
                                              override,
                                              defragmentationEnable);
            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        for(j=0;j<4;j++)
        {
            lastIpAddrAdded.u32Ip[j]    = currentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            IPv6Val[j]    = currentIpAddr.arIP[j];
        }

        /* advance to the next IP address */
        if (IPv6Val[firstOctetToChange] + IPv6Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv6Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv6Val[secondOctetToChange] + IPv6Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv6Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv6Val[thirdOctetToChange] + IPv6Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv6Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv6Val[fourthOctetToChange] + IPv6Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv6Step[fourthOctetToChange];
                    else
                    {
                        currentIpAddr.arIP[fourthOctetToChange] = 0;
                        if (IPv6Val[fifthOctetToChange] + IPv6Step[fifthOctetToChange] <= 255)
                            currentIpAddr.arIP[fifthOctetToChange] += IPv6Step[fifthOctetToChange];
                        else
                        {
                            currentIpAddr.arIP[fifthOctetToChange] = 0;
                            if (IPv6Val[sixthOctetToChange] + IPv6Step[sixthOctetToChange] <= 255)
                                currentIpAddr.arIP[sixthOctetToChange] += IPv6Step[sixthOctetToChange];
                            else
                            {
                                currentIpAddr.arIP[sixthOctetToChange] = 0;
                                if (IPv6Val[seventhOctetToChange] + IPv6Step[seventhOctetToChange] <= 255)
                                    currentIpAddr.arIP[seventhOctetToChange] += IPv6Step[seventhOctetToChange];
                                else
                                {
                                    currentIpAddr.arIP[seventhOctetToChange] = 0;
                                    if (IPv6Val[eighthOctetToChange] + IPv6Step[eighthOctetToChange] <= 255)
                                        currentIpAddr.arIP[eighthOctetToChange] += IPv6Step[eighthOctetToChange];
                                    else
                                    {
                                        currentIpAddr.arIP[eighthOctetToChange] = 0;
                                        if (IPv6Val[ninthOctetToChange] + IPv6Step[ninthOctetToChange] <= 255)
                                            currentIpAddr.arIP[ninthOctetToChange] += IPv6Step[ninthOctetToChange];
                                        else
                                        {
                                            currentIpAddr.arIP[ninthOctetToChange] = 0;
                                            if (IPv6Val[tenthOctetToChange] + IPv6Step[tenthOctetToChange] <= 255)
                                                currentIpAddr.arIP[tenthOctetToChange] += IPv6Step[tenthOctetToChange];
                                            else
                                            {
                                                currentIpAddr.arIP[tenthOctetToChange] = 0;
                                                if (IPv6Val[elevenOctetToChange] + IPv6Step[elevenOctetToChange] <= 255)
                                                    currentIpAddr.arIP[elevenOctetToChange] += IPv6Step[elevenOctetToChange];
                                                else
                                                {
                                                    currentIpAddr.arIP[elevenOctetToChange] = 0;
                                                    if (IPv6Val[twelveOctetToChange] + IPv6Step[twelveOctetToChange] <= 255)
                                                        currentIpAddr.arIP[twelveOctetToChange] += IPv6Step[twelveOctetToChange];
                                                    else
                                                    {
                                                        currentIpAddr.arIP[twelveOctetToChange] = 0;
                                                        if (IPv6Val[thirteenOctetToChange] + IPv6Step[thirteenOctetToChange] <= 255)
                                                            currentIpAddr.arIP[thirteenOctetToChange] += IPv6Step[thirteenOctetToChange];
                                                        else
                                                        {
                                                            currentIpAddr.arIP[thirteenOctetToChange] = 0;
                                                            if (IPv6Val[fourteenOctetToChange] + IPv6Step[fourteenOctetToChange] <= 255)
                                                                currentIpAddr.arIP[fourteenOctetToChange] += IPv6Step[fourteenOctetToChange];
                                                            else
                                                            {
                                                                currentIpAddr.arIP[fourteenOctetToChange] = 0;
                                                                if (IPv6Val[fifteenOctetToChange] + IPv6Step[fifteenOctetToChange] <= 255)
                                                                    currentIpAddr.arIP[fifteenOctetToChange] += IPv6Step[fifteenOctetToChange];
                                                                else
                                                                {
                                                                    currentIpAddr.arIP[fifteenOctetToChange] = 0;
                                                                    if (IPv6Val[sixteenOctetToChange] + IPv6Step[sixteenOctetToChange] <= 255)
                                                                        currentIpAddr.arIP[sixteenOctetToChange] += IPv6Step[sixteenOctetToChange];
                                                                    else
                                                                    {
                                                                        rc = GT_NO_MORE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixAddMany statistics\n");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("    No prefixes were added : 0\n");
    }
    else
    {
        cpssOsPrintf("    Prefixes Range :    ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Added Prefixes                   : %d/%d\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    MC prefixes not added            : %d\n",numOfMcPrefixesNotAdded);
        cpssOsPrintf("    Already exists                   : %d\n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    Total added plus already existed : %d/%d\n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            lastIpAddrAddedPtr->u32Ip[j] = lastIpAddrAdded.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

/**
* @internal cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctetBulk function
* @endinternal
*
* @brief  This function tries to add many sequential IPv6 Unicast prefixes with bulk according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] sizeOfBulk               - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
* @param[out] numOfPrefixesAlreadyExistPtr - points to the nubmer of prefixes that were
*                                      not added since they are already defined (NULL to ignore)
* @param[out] numOfPrefixesNotAddedDueToOutOfPpMemPtr - points to the nubmer of prefixes that were
*                                      not added due to out of PP memory (NULL to ignore)
* @param[out] bulkTimePtr              - points to the time it takes for bulk operation*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctetBulk
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      sizeOfBulk,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr,
    OUT GT_U32      *numOfPrefixesAlreadyExistPtr,
    OUT GT_U32      *numOfPrefixesNotAddedDueToOutOfPpMemPtr,
    OUT GT_U32      *bulkTimePtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_U32                                  numOfPrefixesNotAddedDueToOutOfPpMem=0;
    GT_IPV6ADDR                             lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  IPv6Val[16] = { 0 };

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT         protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT memoryCfg;

    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC     *ipPrefixArrayPtr;
    GT_U32 startTime;
    GT_U32 endTime;

    rc = cpssDxChIpLpmDBConfigGet(lpmDbId, &shadowType, &protocolStack, &memoryCfg);
    if (rc != GT_OK)
    {
        return rc;
    }

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    if (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return rc;
    }

    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        currentIpAddr.u32Ip[j]     = startIpAddr->u32Ip[j];
        lastIpAddrAdded.u32Ip[j]    = startIpAddr->u32Ip[j];
    }


    octetsOrderToChangeArray[0] = firstOctetToChange;
    octetsOrderToChangeArray[1] = secondOctetToChange;
    octetsOrderToChangeArray[2] = thirdOctetToChange;
    octetsOrderToChangeArray[3] = fourthOctetToChange;
    octetsOrderToChangeArray[4] = fifthOctetToChange;
    octetsOrderToChangeArray[5] = sixthOctetToChange;
    octetsOrderToChangeArray[6] = seventhOctetToChange;
    octetsOrderToChangeArray[7] = eighthOctetToChange;
    octetsOrderToChangeArray[8] = ninthOctetToChange;
    octetsOrderToChangeArray[9] = tenthOctetToChange;
    octetsOrderToChangeArray[10] = elevenOctetToChange;
    octetsOrderToChangeArray[11] = twelveOctetToChange;
    octetsOrderToChangeArray[12] = thirteenOctetToChange;
    octetsOrderToChangeArray[13] = fourteenOctetToChange;
    octetsOrderToChangeArray[14] = fifteenOctetToChange;
    octetsOrderToChangeArray[15] = sixteenOctetToChange;

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            cpssOsFree(ipPrefixArrayPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsFree(ipPrefixArrayPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    for (i = 0 ; i < sizeOfBulk ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {
             /* add the current prefix to the bulk array */
            cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &currentIpAddr, sizeof(GT_IPV6ADDR));
            ipPrefixArrayPtr[i].vrId = vrId;
            ipPrefixArrayPtr[i].prefixLen = IPv6PrefixLength;
            cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
            ipPrefixArrayPtr[i].override = override;
            ipPrefixArrayPtr[i].returnStatus = GT_NOT_INITIALIZED;

            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        for(j=0;j<4;j++)
        {
            lastIpAddrAdded.u32Ip[j]    = currentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            IPv6Val[j]    = currentIpAddr.arIP[j];
        }

        /* advance to the next IP address */
        if (IPv6Val[firstOctetToChange] + IPv6Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv6Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv6Val[secondOctetToChange] + IPv6Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv6Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv6Val[thirdOctetToChange] + IPv6Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv6Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv6Val[fourthOctetToChange] + IPv6Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv6Step[fourthOctetToChange];
                    else
                    {
                        currentIpAddr.arIP[fourthOctetToChange] = 0;
                        if (IPv6Val[fifthOctetToChange] + IPv6Step[fifthOctetToChange] <= 255)
                            currentIpAddr.arIP[fifthOctetToChange] += IPv6Step[fifthOctetToChange];
                        else
                        {
                            currentIpAddr.arIP[fifthOctetToChange] = 0;
                            if (IPv6Val[sixthOctetToChange] + IPv6Step[sixthOctetToChange] <= 255)
                                currentIpAddr.arIP[sixthOctetToChange] += IPv6Step[sixthOctetToChange];
                            else
                            {
                                currentIpAddr.arIP[sixthOctetToChange] = 0;
                                if (IPv6Val[seventhOctetToChange] + IPv6Step[seventhOctetToChange] <= 255)
                                    currentIpAddr.arIP[seventhOctetToChange] += IPv6Step[seventhOctetToChange];
                                else
                                {
                                    currentIpAddr.arIP[seventhOctetToChange] = 0;
                                    if (IPv6Val[eighthOctetToChange] + IPv6Step[eighthOctetToChange] <= 255)
                                        currentIpAddr.arIP[eighthOctetToChange] += IPv6Step[eighthOctetToChange];
                                    else
                                    {
                                        currentIpAddr.arIP[eighthOctetToChange] = 0;
                                        if (IPv6Val[ninthOctetToChange] + IPv6Step[ninthOctetToChange] <= 255)
                                            currentIpAddr.arIP[ninthOctetToChange] += IPv6Step[ninthOctetToChange];
                                        else
                                        {
                                            currentIpAddr.arIP[ninthOctetToChange] = 0;
                                            if (IPv6Val[tenthOctetToChange] + IPv6Step[tenthOctetToChange] <= 255)
                                                currentIpAddr.arIP[tenthOctetToChange] += IPv6Step[tenthOctetToChange];
                                            else
                                            {
                                                currentIpAddr.arIP[tenthOctetToChange] = 0;
                                                if (IPv6Val[elevenOctetToChange] + IPv6Step[elevenOctetToChange] <= 255)
                                                    currentIpAddr.arIP[elevenOctetToChange] += IPv6Step[elevenOctetToChange];
                                                else
                                                {
                                                    currentIpAddr.arIP[elevenOctetToChange] = 0;
                                                    if (IPv6Val[twelveOctetToChange] + IPv6Step[twelveOctetToChange] <= 255)
                                                        currentIpAddr.arIP[twelveOctetToChange] += IPv6Step[twelveOctetToChange];
                                                    else
                                                    {
                                                        currentIpAddr.arIP[twelveOctetToChange] = 0;
                                                        if (IPv6Val[thirteenOctetToChange] + IPv6Step[thirteenOctetToChange] <= 255)
                                                            currentIpAddr.arIP[thirteenOctetToChange] += IPv6Step[thirteenOctetToChange];
                                                        else
                                                        {
                                                            currentIpAddr.arIP[thirteenOctetToChange] = 0;
                                                            if (IPv6Val[fourteenOctetToChange] + IPv6Step[fourteenOctetToChange] <= 255)
                                                                currentIpAddr.arIP[fourteenOctetToChange] += IPv6Step[fourteenOctetToChange];
                                                            else
                                                            {
                                                                currentIpAddr.arIP[fourteenOctetToChange] = 0;
                                                                if (IPv6Val[fifteenOctetToChange] + IPv6Step[fifteenOctetToChange] <= 255)
                                                                    currentIpAddr.arIP[fifteenOctetToChange] += IPv6Step[fifteenOctetToChange];
                                                                else
                                                                {
                                                                    currentIpAddr.arIP[fifteenOctetToChange] = 0;
                                                                    if (IPv6Val[sixteenOctetToChange] + IPv6Step[sixteenOctetToChange] <= 255)
                                                                        currentIpAddr.arIP[sixteenOctetToChange] += IPv6Step[sixteenOctetToChange];
                                                                    else
                                                                    {
                                                                        rc = GT_NO_MORE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* Add the entries using bulk API */
    /* call device specific API */
    startTime = cpssOsTime();
    rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDbId, sizeOfBulk, ipPrefixArrayPtr,GT_FALSE);/* bulk do not suppurt defrag */
    endTime = cpssOsTime();
    *bulkTimePtr = endTime-startTime;

    /* check how many entries were added in the bulk operation */
    numOfPrefixesAdded=0;
    numOfPrefixesAlreadyExist=0;
    for (i = 0 ; i < sizeOfBulk ; i++)
    {
        if(ipPrefixArrayPtr[i].returnStatus == GT_OK)
        {
            lastIpAddrAdded.u32Ip[0] = ipPrefixArrayPtr[i].ipAddr.u32Ip[0];
            lastIpAddrAdded.u32Ip[1] = ipPrefixArrayPtr[i].ipAddr.u32Ip[1];
            lastIpAddrAdded.u32Ip[2] = ipPrefixArrayPtr[i].ipAddr.u32Ip[2];
            lastIpAddrAdded.u32Ip[3] = ipPrefixArrayPtr[i].ipAddr.u32Ip[3];
            numOfPrefixesAdded++;
        }
        else
        {
            if (ipPrefixArrayPtr[i].returnStatus == GT_ALREADY_EXIST)
            {
                numOfPrefixesAlreadyExist++;
            }
            else{
                if (ipPrefixArrayPtr[i].returnStatus == GT_OUT_OF_PP_MEM)
                {
                    numOfPrefixesNotAddedDueToOutOfPpMem++;
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixAddMany statistics\n");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("    No prefixes were added : 0\n");
    }
    else
    {
        cpssOsPrintf("    Prefixes Range :    ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Added Prefixes                   : %d/%d\n",numOfPrefixesAdded,sizeOfBulk);
        cpssOsPrintf("    MC prefixes not added            : %d\n",numOfMcPrefixesNotAdded);
        cpssOsPrintf("    Already exists                   : %d\n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    Total added plus already existed : %d/%d\n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),sizeOfBulk);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),sizeOfBulk);
        cpssOsPrintf("    total number of prefixes added plus existed plus out_of_pp(%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded+numOfPrefixesNotAddedDueToOutOfPpMem),sizeOfBulk);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (numOfPrefixesNotAddedDueToOutOfPpMemPtr != NULL)
        *numOfPrefixesNotAddedDueToOutOfPpMemPtr = numOfPrefixesNotAddedDueToOutOfPpMem;
    if (numOfPrefixesAlreadyExistPtr != NULL)
        *numOfPrefixesAlreadyExistPtr = numOfPrefixesAlreadyExist;

    if (lastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            lastIpAddrAddedPtr->u32Ip[j] = lastIpAddrAdded.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    cpssOsFree(ipPrefixArrayPtr);
    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr    - (pointer to)the first group address to deleted
* @param[in] srcStartIpAddr    - (pointer to)the first source address to deleted
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrDeletedPtr  - points to the last group prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] srcLastIpAddrDeletedPtr  - points to the last source prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be deleted. All the prefixes are deleted with exact match (prefix
*       length 32).
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *grpStartIpAddr,
    IN  GT_IPADDR   *srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_IPADDR   *grpLastIpAddrDeletedPtr,
    OUT GT_IPADDR   *srcLastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPADDR                               grpCurrentIpAddr;
    GT_IPADDR                               srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_IPADDR                               grpLastIpAddrDeleted;
    GT_IPADDR                               srcLastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS];
    GT_U32                                  srcIPv4Val[4] = { 0 };
    GT_U32                                  grpIPv4Val[4] = { 0 };

    cpssOsMemSet(octetsOrderToChangeArray, 0, sizeof(octetsOrderToChangeArray));

    srcCurrentIpAddr.u32Ip                  = srcStartIpAddr->u32Ip;
    srcLastIpAddrDeleted.u32Ip              = srcStartIpAddr->u32Ip;

    grpCurrentIpAddr.u32Ip                  = grpStartIpAddr->u32Ip;
    grpLastIpAddrDeleted.u32Ip              = grpStartIpAddr->u32Ip;

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>3)
        {
            cpssOsPrintf("octetToChange can not be bigger then 3\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the grp prefix is a MC prefix - in the range 224/4 - 239/4
           therefore the first octate must start with 0xE
           and the src prefix is not a MC prefix */
        if(((grpCurrentIpAddr.arIP[0] & 0xF0) == 0xE0)&&
           ((srcCurrentIpAddr.arIP[0] & 0xF0) != 0xE0))
        {
            /* del the current prefix */
            rc = cpssDxChIpLpmIpv4McEntryDel(lpmDbId,
                                             vrId,
                                             &grpCurrentIpAddr,
                                             IPv4PrefixLength,
                                             &srcCurrentIpAddr,
                                             srcIPv4PrefixLength);
            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
             else
                numOfPrefixesDeleted++;

            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv4McPrefixDelMany results:");
            if (numOfPrefixesDeleted == 0)
            {
                cpssOsPrintf("\n    No prefixes were deleted at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
                cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
                cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

            }

            /* update output parameters */
            if (numOfPrefixesDeletedPtr != NULL)
                *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
            if (grpLastIpAddrDeletedPtr != NULL)
                grpLastIpAddrDeletedPtr->u32Ip = grpLastIpAddrDeleted.u32Ip;
            if (srcLastIpAddrDeletedPtr != NULL)
                srcLastIpAddrDeletedPtr->u32Ip = srcLastIpAddrDeleted.u32Ip;

            /* Return to default */
            for (i = 0; i < 4; i++) {
                IPv4Step[i] = 1;
                srcIPv4Step[i] = 1;
            }
            IPv4PrefixLength = 32;
            srcIPv4PrefixLength = 32;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        srcLastIpAddrDeleted.u32Ip                = srcCurrentIpAddr.u32Ip;
        grpLastIpAddrDeleted.u32Ip                = grpCurrentIpAddr.u32Ip;

        srcIPv4Val[3]=srcCurrentIpAddr.arIP[3];
        srcIPv4Val[2]=srcCurrentIpAddr.arIP[2];
        srcIPv4Val[1]=srcCurrentIpAddr.arIP[1];
        srcIPv4Val[0]=srcCurrentIpAddr.arIP[0];

        grpIPv4Val[3]=grpCurrentIpAddr.arIP[3];
        grpIPv4Val[2]=grpCurrentIpAddr.arIP[2];
        grpIPv4Val[1]=grpCurrentIpAddr.arIP[1];
        grpIPv4Val[0]=grpCurrentIpAddr.arIP[0];

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv4Val[srcFirstOctetToChange] + srcIPv4Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv4Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv4Val[srcSecondOctetToChange] + srcIPv4Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv4Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv4Val[srcThirdOctetToChange] + srcIPv4Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv4Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv4Val[srcFourthOctetToChange] + srcIPv4Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv4Step[srcFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv4Val[grpFirstOctetToChange] + IPv4Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv4Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv4Val[grpSecondOctetToChange] + IPv4Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv4Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv4Val[grpThirdOctetToChange] + IPv4Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv4Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv4Val[grpFourthOctetToChange] + IPv4Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv4Step[grpFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4McPrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (grpLastIpAddrDeletedPtr != NULL)
        grpLastIpAddrDeletedPtr->u32Ip = grpLastIpAddrDeleted.u32Ip;
    if (srcLastIpAddrDeletedPtr != NULL)
        srcLastIpAddrDeletedPtr->u32Ip = srcLastIpAddrDeleted.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr        - (pointer to)the first group address to add
* @param[in] srcStartIpAddr        - (pointer to)the first source address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrAddedPtr    - points to the last group prefix successfully
*                                      added (NULL to ignore)
* @param[out] srcLastIpAddrAddedPtr    - points to the last source prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *grpStartIpAddr,
    IN  GT_IPADDR   *srcStartIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_IPADDR   *grpLastIpAddrAddedPtr,
    OUT GT_IPADDR   *srcLastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPADDR                               grpCurrentIpAddr;
    GT_IPADDR                               srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPADDR                               grpLastIpAddrAdded;
    GT_IPADDR                               srcLastIpAddrAdded;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS];
    GT_BOOL                                 defragmentationEnable=defragmentationEnableFlag;
    GT_U32                                  srcIPv4Val[4] = { 0 };
    GT_U32                                  grpIPv4Val[4] = { 0 };
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT         protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT memoryCfg;

    cpssOsMemSet(octetsOrderToChangeArray, 0, sizeof(octetsOrderToChangeArray));

    rc = cpssDxChIpLpmDBConfigGet(lpmDbId, &shadowType, &protocolStack, &memoryCfg);
    if (rc != GT_OK)
    {
        return rc;
    }

    srcCurrentIpAddr.u32Ip                  = srcStartIpAddr->u32Ip;
    srcLastIpAddrAdded.u32Ip                = srcStartIpAddr->u32Ip;

    grpCurrentIpAddr.u32Ip                  = grpStartIpAddr->u32Ip;
    grpLastIpAddrAdded.u32Ip                = grpStartIpAddr->u32Ip;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E) ||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E) )
    {
        mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
        mcRouteLttEntry.priority = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E;
    }
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>3)
        {
            cpssOsPrintf("octetToChange can not be bigger then 3\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the grp prefix is a MC prefix - in the range 224/4 - 239/4
           therefore the first octate must start with 0xE
           and the src prefix is not a MC prefix */
        if(((grpCurrentIpAddr.arIP[0] & 0xF0) == 0xE0)&&
           ((srcCurrentIpAddr.arIP[0] & 0xF0) != 0xE0))
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv4McEntryAdd(lpmDbId,
                                             vrId,
                                             &grpCurrentIpAddr,
                                             IPv4PrefixLength,
                                             &srcCurrentIpAddr,
                                             srcIPv4PrefixLength,
                                             &mcRouteLttEntry,
                                             override,
                                             defragmentationEnable);

            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;

            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv4McPrefixAddMany results:");
            if (numOfPrefixesAdded == 0)
            {
                cpssOsPrintf("\n    No prefixes were added at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
                cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
                cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

            }

            /* update output parameters */
            if (numOfPrefixesAddedPtr != NULL)
                *numOfPrefixesAddedPtr = numOfPrefixesAdded;
            if (grpLastIpAddrAddedPtr != NULL)
                grpLastIpAddrAddedPtr->u32Ip = grpLastIpAddrAdded.u32Ip;
            if (srcLastIpAddrAddedPtr != NULL)
                srcLastIpAddrAddedPtr->u32Ip = srcLastIpAddrAdded.u32Ip;

            /* Return to default */
            for (i = 0; i < 4; i++) {
                IPv4Step[i] = 1;
                srcIPv4Step[i] = 1;
            }
            IPv4PrefixLength = 32;
            srcIPv4PrefixLength = 32;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        srcLastIpAddrAdded.u32Ip                = srcCurrentIpAddr.u32Ip;
        grpLastIpAddrAdded.u32Ip                = grpCurrentIpAddr.u32Ip;

        srcIPv4Val[3]=srcCurrentIpAddr.arIP[3];
        srcIPv4Val[2]=srcCurrentIpAddr.arIP[2];
        srcIPv4Val[1]=srcCurrentIpAddr.arIP[1];
        srcIPv4Val[0]=srcCurrentIpAddr.arIP[0];

        grpIPv4Val[3]=grpCurrentIpAddr.arIP[3];
        grpIPv4Val[2]=grpCurrentIpAddr.arIP[2];
        grpIPv4Val[1]=grpCurrentIpAddr.arIP[1];
        grpIPv4Val[0]=grpCurrentIpAddr.arIP[0];

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv4Val[srcFirstOctetToChange] + srcIPv4Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv4Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv4Val[srcSecondOctetToChange] + srcIPv4Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv4Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv4Val[srcThirdOctetToChange] + srcIPv4Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv4Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv4Val[srcFourthOctetToChange] + srcIPv4Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv4Step[srcFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv4Val[grpFirstOctetToChange] + IPv4Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv4Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv4Val[grpSecondOctetToChange] + IPv4Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv4Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv4Val[grpThirdOctetToChange] + IPv4Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv4Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv4Val[grpFourthOctetToChange] + IPv4Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv4Step[grpFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4McPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (grpLastIpAddrAddedPtr != NULL)
        grpLastIpAddrAddedPtr->u32Ip = grpLastIpAddrAdded.u32Ip;
    if (srcLastIpAddrAddedPtr != NULL)
        srcLastIpAddrAddedPtr->u32Ip = srcLastIpAddrAdded.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv4 Unicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfRetrievedPrefixes = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_IPADDR                               lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  tcamRowIndex=0;
    GT_U32                                  tcamColumnIndex=0;
    GT_U32                                  IPv4Val[4] = { 0 };

    currentIpAddr.u32Ip                     = startIpAddr->u32Ip;
    lastIpAddrAdded.u32Ip                   = startIpAddr->u32Ip;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < numOfPrefixesToGet ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* get the current prefix */
            rc =  cpssDxChIpLpmIpv4UcPrefixSearch(lpmDbId,
                                                 vrId,
                                                 &currentIpAddr,
                                                 IPv4PrefixLength,
                                                 &nextHopInfo,
                                                 &tcamRowIndex,
                                                 &tcamColumnIndex);
            if (rc != GT_OK)
                break;

            numOfRetrievedPrefixes++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        lastIpAddrAdded.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixGetMany results:");
    if (numOfRetrievedPrefixes == 0)
    {
        cpssOsPrintf("\n    No prefixes at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
    }

    /* update output parameters */
    if (numOfRetrievedPrefixesPtr != NULL)
        *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_U32                                  numOfMcPrefixesNotDeleted = 0;
    GT_IPADDR                               lastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  IPv4Val[4] = {0};

    currentIpAddr.u32Ip                     = startIpAddr->u32Ip;
    lastIpAddrDeleted.u32Ip                 = startIpAddr->u32Ip;

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* delete the current prefix */
            rc = cpssDxChIpLpmIpv4UcPrefixDel(lpmDbId,vrId,&currentIpAddr,IPv4PrefixLength);

            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
            else
                numOfPrefixesDeleted++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotDeleted++;
        }


        lastIpAddrDeleted.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were Deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were Deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) MC prefixes not Deleted in this range\n",numOfMcPrefixesNotDeleted);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (lastIpAddrDeletedPtr != NULL)
        lastIpAddrDeletedPtr->u32Ip = lastIpAddrDeleted.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctetBulk function
* @endinternal
*
* @brief  This function tries to delete many sequential IPv4 Unicast prefixes using bulk according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] sizeOfBulk               - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
* @param[out] bulkTimePtr              - points to the time it takes for bulk operation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctetBulk.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctetBulk
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      sizeOfBulk,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr,
    OUT GT_U32      *numOfPrefixesAlreadyDeletedPtr,
    OUT GT_U32      *bulkTimePtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_U32                                  numOfMcPrefixesNotDeleted = 0;
    GT_IPADDR                               lastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  IPv4Val[4] = {0};

    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC     *ipPrefixArrayPtr;
    GT_U32 startTime;
    GT_U32 endTime;

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

    currentIpAddr.u32Ip                     = startIpAddr->u32Ip;
    lastIpAddrDeleted.u32Ip                 = startIpAddr->u32Ip;

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < sizeOfBulk ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* add the current prefix to the bulk array */
            cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &currentIpAddr, sizeof(GT_IPADDR));
            ipPrefixArrayPtr[i].vrId = vrId;
            ipPrefixArrayPtr[i].prefixLen = IPv4PrefixLength;
            ipPrefixArrayPtr[i].returnStatus = GT_NOT_INITIALIZED;

            i++;
        }
        else
        {
            numOfMcPrefixesNotDeleted++;
        }


        lastIpAddrDeleted.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* delete the entries using bulk API */
    /* call device specific API */
    startTime = cpssOsTime();
    rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDbId, sizeOfBulk, ipPrefixArrayPtr);
    endTime = cpssOsTime();
    *bulkTimePtr = endTime-startTime;

    /* check how many entries were added in the bulk operation */
    numOfPrefixesDeleted=0;
    numOfPrefixesAlreadyDeleted=0;
    for (i = 0 ; i < sizeOfBulk ; i++)
    {
        if(ipPrefixArrayPtr[i].returnStatus == GT_OK)
        {
            lastIpAddrDeleted.u32Ip = ipPrefixArrayPtr[i].ipAddr.u32Ip;
            numOfPrefixesDeleted++;
        }
        else
        {
            if (ipPrefixArrayPtr[i].returnStatus == GT_NOT_FOUND)
            {
                numOfPrefixesAlreadyDeleted++;
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were Deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were Deleted (%d/%d)\n",numOfPrefixesDeleted,sizeOfBulk);
        cpssOsPrintf("    (%d) MC prefixes not Deleted in this range\n",numOfMcPrefixesNotDeleted);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),sizeOfBulk);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (numOfPrefixesAlreadyDeletedPtr != NULL)
        *numOfPrefixesAlreadyDeletedPtr = numOfPrefixesAlreadyDeleted;
    if (lastIpAddrDeletedPtr != NULL)
        lastIpAddrDeletedPtr->u32Ip = lastIpAddrDeleted.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPADDR                               lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 defragmentationEnable = defragmentationEnableFlag;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32 IPv4Val[4] = {0};

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT         protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT memoryCfg;


    rc = cpssDxChIpLpmDBConfigGet(lpmDbId, &shadowType, &protocolStack, &memoryCfg);
    if (rc != GT_OK)
    {
        return rc;
    }

    currentIpAddr.u32Ip                     = startIpAddr->u32Ip;
    lastIpAddrAdded.u32Ip                   = startIpAddr->u32Ip;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    if (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDbId,vrId,&currentIpAddr,
                                              IPv4PrefixLength,&nextHopInfo,
                                              override,defragmentationEnable);
            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;

            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }


        lastIpAddrAdded.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
        lastIpAddrAddedPtr->u32Ip = lastIpAddrAdded.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctetBulk function
* @endinternal
*
* @brief  This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*         the add is done using bulk API
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] sizeOfBulk               - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
* @param[out] numOfPrefixesAlreadyExistPtr - points to the nubmer of prefixes that were
*                                      not added since they are already defined (NULL to ignore)
* @param[out] numOfPrefixesNotAddedDueToOutOfPpMemPtr - points to the nubmer of prefixes that were
*                                      not added due to out of PP memory (NULL to ignore)
* @param[out] bulkTimePtr              - points to the time it takes for bulk operation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
static GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctetBulk
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      sizeOfBulk,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr,
    OUT GT_U32      *numOfPrefixesAlreadyExistPtr,
    OUT GT_U32      *numOfPrefixesNotAddedDueToOutOfPpMemPtr,
    OUT GT_U32      *bulkTimePtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_U32                                  numOfPrefixesNotAddedDueToOutOfPpMem=0;
    GT_IPADDR                               lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32 IPv4Val[4] = {0};

    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC     *ipPrefixArrayPtr;
    GT_U32 startTime;
    GT_U32 endTime;

     CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT         protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT memoryCfg;


    rc = cpssDxChIpLpmDBConfigGet(lpmDbId, &shadowType, &protocolStack, &memoryCfg);
    if (rc != GT_OK)
    {
        return rc;
    }

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

    currentIpAddr.u32Ip                     = startIpAddr->u32Ip;
    lastIpAddrAdded.u32Ip                   = startIpAddr->u32Ip;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    if (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        cpssOsFree(ipPrefixArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        cpssOsFree(ipPrefixArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        cpssOsFree(ipPrefixArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        cpssOsFree(ipPrefixArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < sizeOfBulk ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* add the current prefix to the bulk array */
            cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &currentIpAddr, sizeof(GT_IPADDR));
            ipPrefixArrayPtr[i].vrId = vrId;
            ipPrefixArrayPtr[i].prefixLen = IPv4PrefixLength;
            cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
            ipPrefixArrayPtr[i].override = override;
            ipPrefixArrayPtr[i].returnStatus = GT_NOT_INITIALIZED;

            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        lastIpAddrAdded.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* Add the entries using bulk API */
    /* call device specific API */
    startTime = cpssOsTime();
    rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDbId, sizeOfBulk, ipPrefixArrayPtr);
    endTime = cpssOsTime();
    *bulkTimePtr = endTime-startTime;

    /* check how many entries were added in the bulk operation */
    numOfPrefixesAdded=0;
    numOfPrefixesAlreadyExist=0;
    for (i = 0 ; i < sizeOfBulk ; i++)
    {
        if(ipPrefixArrayPtr[i].returnStatus == GT_OK)
        {
            lastIpAddrAdded.u32Ip = ipPrefixArrayPtr[i].ipAddr.u32Ip;
            numOfPrefixesAdded++;
        }
        else
        {
            if (ipPrefixArrayPtr[i].returnStatus == GT_ALREADY_EXIST)
            {
                numOfPrefixesAlreadyExist++;
            }
            else{
                if (ipPrefixArrayPtr[i].returnStatus == GT_OUT_OF_PP_MEM)
                {
                    numOfPrefixesNotAddedDueToOutOfPpMem++;
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddMany by bulk results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr->arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,sizeOfBulk);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),sizeOfBulk);
        cpssOsPrintf("    total number of prefixes added plus existed plus out_of_pp(%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded+numOfPrefixesNotAddedDueToOutOfPpMem),sizeOfBulk);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
        lastIpAddrAddedPtr->u32Ip = lastIpAddrAdded.u32Ip;
    if (numOfPrefixesNotAddedDueToOutOfPpMemPtr != NULL)
        *numOfPrefixesNotAddedDueToOutOfPpMemPtr = numOfPrefixesNotAddedDueToOutOfPpMem;
    if (numOfPrefixesAlreadyExistPtr != NULL)
        *numOfPrefixesAlreadyExistPtr = numOfPrefixesAlreadyExist;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    cpssOsFree(ipPrefixArrayPtr);
    return rc;
}

static GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    IN  GT_BOOL     defragmentationEnable,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_U32                                  counter = 0;
    GT_U32                                  maxCounterValue = 0;
    GT_U32                                  seed = 0;

    /* set specific seed for random generator */
    cpssOsSrand(seed);

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    currentIpAddr.u32Ip                     = startIpAddr->u32Ip;
    currentIpAddr.arIP[3] = (GT_U8)(cpssOsRand() % 256);
    currentIpAddr.arIP[2]= (GT_U8)(cpssOsRand() % 256);

    if (isWholeIpRandom == GT_TRUE)
    {
        currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
        currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
    }
    for (i = 0 ; i < numOfPrefixesToAdd ; i++)
    {
        rc = 100;
        counter = 0;
        while (rc != GT_OK)
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDbId,vrId,&currentIpAddr,
                                          IPv4PrefixLength,&nextHopInfo,
                                          override,defragmentationEnable);
            if (rc != GT_OK)
            {
                if (rc == GT_ALREADY_EXIST)
                {
                    if (counter > 20000)
                        break;
                    currentIpAddr.arIP[3] = (GT_U8)(cpssOsRand() % 256);
                    currentIpAddr.arIP[2]= (GT_U8)(cpssOsRand() % 256);

                    if (isWholeIpRandom == GT_TRUE)
                    {
                        currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
                        currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
                    }
                    counter++;
                    if (counter > maxCounterValue)
                        maxCounterValue = counter;
                }
                else
                {
                    break;
                }
            }
        }

        if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
            break;

        if (rc==GT_ALREADY_EXIST)
        {
            rc = GT_OK;
            numOfPrefixesAlreadyExist++;
        }
        else
            numOfPrefixesAdded++;

        if ((numOfPrefixesAdded % 50) == 0 )
        {
            currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
            currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
        }
        currentIpAddr.arIP[3] = (GT_U8)(cpssOsRand() % 256);
        currentIpAddr.arIP[2]= (GT_U8)(cpssOsRand() % 256);
        if (isWholeIpRandom == GT_TRUE)
        {
            currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
            currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
        }
    }
    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddManyRandom results:");
    if (rc != GT_OK)
    {
        cpssOsPrintf("\n error = %x occured on prefix\n",rc);
        outputIPAddress(currentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;

    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf(" were added %d prefixes from %d prefixes)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
    }
    cpssOsPrintf("maxCounterValue =%d\n",maxCounterValue);
    return rc;
}


/**
* @internal prvTgfConvertGenericToDxChIpUcRouteEntry function
* @endinternal
*
* @brief   Convert generic IP UC route entry into device specific IP UC route entry
*
* @param[in] ipUcRouteEntryPtr        - (pointer to) IP UC route entry
*
* @param[out] dxChIpUcRouteEntryPtr    - (pointer to) DxCh IP UC route entry
*
* @retval GT_OK                    -  on OK
* @retval GT_BAD_PARAM             -  Invalid device id
*/
static GT_STATUS prvTgfConvertGenericToDxChIpUcRouteEntry
(
    IN  PRV_TGF_IP_UC_ROUTE_ENTRY_STC            *ipUcRouteEntryPtr,
    OUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC   *dxChIpUcRouteEntryPtr
)
{
    GT_STATUS        rc = GT_OK; /* return code */

    /* convert IP UC route entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, cmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, unicastPacketSipFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ingressMirror);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, countSet);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, trapMirrorArpBcEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipAccessLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, dipAccessLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ICMPRedirectEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, scopeCheckingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, siteId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopVlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimitDecEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileMarkingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isTunnelStart);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isNat);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopARPPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopTunnelPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopNatPointer);

    dxChIpUcRouteEntryPtr->cpuCodeIdx = ipUcRouteEntryPtr->cpuCodeIndex;

    cpssOsMemCpy((GT_VOID*) &(dxChIpUcRouteEntryPtr->nextHopInterface),
                 (GT_VOID*) &(ipUcRouteEntryPtr->nextHopInterface),
                 sizeof(ipUcRouteEntryPtr->nextHopInterface));

    if( CPSS_INTERFACE_PORT_E == ipUcRouteEntryPtr->nextHopInterface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)ipUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum,
                                     &dxChIpUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum);
        if (rc != GT_OK)
        {
            /* ignore NOT OK.
              Some tests uses not existing devNum and it's OK for tests */
            rc = GT_OK;
        }
    }

    return rc;
}

/**
* @internal prvTgfConvertDxChToGenericIpUcRouteEntry function
* @endinternal
*
* @brief   Convert device specific IP UC route entry into generic IP UC route entry
*
* @param[in] dxChIpUcRouteEntryPtr    - (pointer to) DxCh IP UC route entry
*
* @param[out] ipUcRouteEntryPtr        - (pointer to) IP UC route entry
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericIpUcRouteEntry
(
    IN  CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC   *dxChIpUcRouteEntryPtr,
    OUT PRV_TGF_IP_UC_ROUTE_ENTRY_STC            *ipUcRouteEntryPtr
)
{
    /* convert IP UC route entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, cmd);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, unicastPacketSipFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ingressMirror);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, countSet);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, trapMirrorArpBcEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipAccessLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, dipAccessLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ICMPRedirectEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, scopeCheckingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, siteId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopVlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimitDecEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileMarkingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosPrecedence);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyDscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isTunnelStart);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isNat);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopARPPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopTunnelPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopNatPointer);

    ipUcRouteEntryPtr->cpuCodeIndex = dxChIpUcRouteEntryPtr->cpuCodeIdx;

    cpssOsMemCpy((GT_VOID*) &(ipUcRouteEntryPtr->nextHopInterface),
                 (GT_VOID*) &(dxChIpUcRouteEntryPtr->nextHopInterface),
                 sizeof(dxChIpUcRouteEntryPtr->nextHopInterface));

    if( CPSS_INTERFACE_PORT_E == dxChIpUcRouteEntryPtr->nextHopInterface.type )
    {
        prvUtfSwFromHwDeviceNumberGet(dxChIpUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum,
                                      &ipUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum);
    }
}

/**
* @internal prvTgfConvertGenericToDxChIpMcRouteEntry function
* @endinternal
*
* @brief   Convert generic IP MC route entry into device specific IP UC route entry
*
* @param[in] protocol                 -  ip  type
* @param[in] ipMcRouteEntryPtr        - (pointer to) IP MC route entry
*
* @param[out] dxChIpMcRouteEntryPtr    - (pointer to) DxCh IP MC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpMcRouteEntry
(
    IN CPSS_IP_PROTOCOL_STACK_ENT                protocol,
    IN  PRV_TGF_IP_MC_ROUTE_ENTRY_STC            *ipMcRouteEntryPtr,
    OUT CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC          *dxChIpMcRouteEntryPtr
)
{
    /* convert IP MC route entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, cmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, ttlHopLimitDecEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, ingressMirror);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, qosProfileMarkingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, qosProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, qosPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, modifyUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, modifyDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, countSet);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, multicastRPFCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, multicastRPFVlan);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, RPFFailCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, scopeCheckingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, siteId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, mtuProfileIndex);
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        dxChIpMcRouteEntryPtr->internalMLLPointer = ipMcRouteEntryPtr->ipv4MllPtr;

    }
    else
    {
        dxChIpMcRouteEntryPtr->externalMLLPointer = ipMcRouteEntryPtr->ipv6ExternalMllPtr;
        dxChIpMcRouteEntryPtr->internalMLLPointer = ipMcRouteEntryPtr->ipv6InternalMllPtr;
    }
    dxChIpMcRouteEntryPtr->cpuCodeIdx = ipMcRouteEntryPtr->cpuCodeIndex;

    /* convert mcRPFFailCmdMode into device specific format */
    switch (ipMcRouteEntryPtr->mcRPFFailCmdMode)
    {
        case PRV_TGF_IP_MC_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E:
            dxChIpMcRouteEntryPtr->multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
            break;

        case PRV_TGF_IP_MC_MLL_RPF_FAIL_COMMAND_MODE_E:
            dxChIpMcRouteEntryPtr->multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_MLL_RPF_FAIL_COMMAND_MODE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChRouteType function
* @endinternal
*
* @brief   Convert generic route type into device specific route type
*
* @param[in] routeType                - route type
*
* @param[out] dxChRouteTypePtr         - (pointer to) DxCh route type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChRouteType
(
    IN  PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT       routeType,
    OUT CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT    *dxChRouteTypePtr
)
{
    /* convert routeType into device specific format */
    switch (routeType)
    {
        case PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
            break;

        case PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E;
            break;

        case PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
            break;
        case PRV_TGF_IP_MULTIPATH_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericRouteType function
* @endinternal
*
* @brief   Convert device specific route type into generic route type
*
* @param[in] dxChRouteType            - DxCh route type
*
* @param[out] routeTypePtr             - (pointer to) route type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericRouteType
(
    IN  CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT     dxChRouteType,
    OUT PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT      *routeTypePtr
)
{
    /* convert routeType from device specific format */
    switch (dxChRouteType)
    {
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
            *routeTypePtr = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;

        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
            *routeTypePtr = PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;

        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E:
            *routeTypePtr = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChUcNextHopInfo function
* @endinternal
*
* @brief   Convert generic uc next hop info into DX specific uc next hop info
*
* @param[in] routingMode              - routing mode
* @param[in] defUcNextHopInfoPtr      - (pointer to)  generic uc next hop info
*
* @param[out] dxChDefUcNextHopInfoPtr  - (pointer to) DX uc next hop info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChUcNextHopInfo
(
    IN  PRV_TGF_IP_ROUTING_MODE_ENT            routingMode,
    IN  PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *defUcNextHopInfoPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChDefUcNextHopInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    switch (routingMode)
    {
    case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       pktCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       actionStop);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       bypassBridge);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       bypassIngressPipe);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       egressPolicy);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       ipclConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       pcl0_1OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       pcl1OverrideConfigIndex);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       cpuCode);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       mirrorTcpRstAndFinPacketsToCpu);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       mirrorToRxAnalyzerPort);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       ingressMirrorToAnalyzerIndex);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       enableMatchCount);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       matchCounterIndex);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       modifyUp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       modifyDscp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       profileAssignIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       profileIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       profilePrecedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect,
                                       redirectCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       routerLttIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       routerLttIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       vrfId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                       &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                       policerId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                       &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                       policerEnable);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       assignSourceId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       sourceIdValue);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       arpDaIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       bypassTTLCheck);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       decrementTTL);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       doIpUcRoute);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       icmpRedirectCheck);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       modifyVlan);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       nestedVlan);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       precedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       vlanId);
        break;
    case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
        /* set route type */
        rc = prvTgfConvertGenericToDxChRouteType(defUcNextHopInfoPtr->ipLttEntry.routeType,
                                                 &dxChDefUcNextHopInfoPtr->ipLttEntry.routeType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

            return rc;
        }
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       numOfPaths);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       routeEntryBaseIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       ucRPFCheckEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       sipSaCheckMismatchEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       ipv6MCGroupScopeLevel);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       priority);

        break;
    default:
            return GT_BAD_PARAM;
    }

    return rc;
}


/**
* @internal prvTgfConvertGenericToDxChMcLttEntryInfo function
* @endinternal
*
* @brief   Convert generic mc LTT info into DX specific mc LTT info
*
* @param[in] defMcRouteLttEntryPtr    - (pointer to) generic mc LTT info
*
* @param[out] dxChDefMcRouteLttEntryPtr - (pointer to) DX mc LTT info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChMcLttEntryInfo
(
    IN  PRV_TGF_IP_LTT_ENTRY_STC             *defMcRouteLttEntryPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC      *dxChDefMcRouteLttEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    /* set route type */
    rc = prvTgfConvertGenericToDxChRouteType(defMcRouteLttEntryPtr->routeType,
                                             &dxChDefMcRouteLttEntryPtr->routeType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert ltt entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, numOfPaths);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, routeEntryBaseIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ucRPFCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, sipSaCheckMismatchEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ipv6MCGroupScopeLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, priority);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, applyPbr);
    return rc;
}

/**
* @internal prvTgfConvertDxChToGenericMcLttEntryInfo function
* @endinternal
*
* @brief   Convert generic mc LTT info from DX specific mc LTT info
*
* @param[in] dxChDefMcRouteLttEntryPtr - (pointer to) DX mc LTT info
*
* @param[out] defMcRouteLttEntryPtr    - (pointer to) generic mc LTT info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericMcLttEntryInfo
(
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC      *dxChDefMcRouteLttEntryPtr,
    OUT PRV_TGF_IP_LTT_ENTRY_STC        *defMcRouteLttEntryPtr
)
{
    GT_STATUS rc = GT_OK;

    /* set route type */
    rc = prvTgfConvertDxChToGenericRouteType(dxChDefMcRouteLttEntryPtr->routeType,
                                             &(defMcRouteLttEntryPtr->routeType));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRouteType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert ltt entry into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, numOfPaths);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, routeEntryBaseIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ucRPFCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, sipSaCheckMismatchEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ipv6MCGroupScopeLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, priority);
    return rc;
}

/**
* @internal prvTgfConvertDxChToGenericUcNextHopInfo function
* @endinternal
*
* @brief   Convert generic UC next hop info from DX specific UC next hop info
*
* @param[in] routingMode              - routing mode
* @param[in] dxChDefUcNextHopInfoPtr  - (pointer to) DxCh UC next hop info
*
* @param[out] defUcNextHopInfoPtr      - (pointer to) generic uc next hop info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericUcNextHopInfo
(
    IN  PRV_TGF_IP_ROUTING_MODE_ENT            routingMode,
    IN  CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChDefUcNextHopInfoPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *defUcNextHopInfoPtr
)
{
    GT_STATUS rc = GT_OK;

    switch (routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           pktCmd);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           actionStop);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           bypassBridge);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           bypassIngressPipe);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           egressPolicy);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           ipclConfigIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           pcl0_1OverrideConfigIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           pcl1OverrideConfigIndex);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           cpuCode);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           mirrorTcpRstAndFinPacketsToCpu);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           mirrorToRxAnalyzerPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           ingressMirrorToAnalyzerIndex);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           enableMatchCount);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           matchCounterIndex);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           modifyUp);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           modifyDscp);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           profileAssignIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           profileIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           profilePrecedence);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect,
                                           redirectCmd);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           routerLttIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           routerLttIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           vrfId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                           &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                           policerId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                           &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                           policerEnable);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           assignSourceId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           sourceIdValue);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           arpDaIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           bypassTTLCheck);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           decrementTTL);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           doIpUcRoute);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           icmpRedirectCheck);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                           modifyVlan);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                           nestedVlan);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                           precedence);
            dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress.vlanId =
                (GT_U16)defUcNextHopInfoPtr->pclIpUcAction.vlan.vlanId;
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            /* set route type */
            rc = prvTgfConvertDxChToGenericRouteType(dxChDefUcNextHopInfoPtr->ipLttEntry.routeType,
                                                     &defUcNextHopInfoPtr->ipLttEntry.routeType);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRouteType FAILED, rc = [%d]", rc);

                return rc;
            }

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           numOfPaths);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           routeEntryBaseIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           ucRPFCheckEnable);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           sipSaCheckMismatchEnable);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           ipv6MCGroupScopeLevel);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           priority);

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChIpHeaderError function
* @endinternal
*
* @brief   Convert generic IP header error into device specific IP header error
*
* @param[in] ipErrorHeader            - IP error header
*
* @param[out] dxChIpErrorHeader        - (pointer to) DxCh IP error header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpHeaderError
(
    IN  PRV_TGF_IP_HEADER_ERROR_ENT         ipErrorHeader,
    OUT CPSS_DXCH_IP_HEADER_ERROR_ENT       *dxChIpErrorHeader
)
{
    /* convert error header into device specific format */
    switch (ipErrorHeader)
    {
        case PRV_TGF_IP_HEADER_ERROR_CHECKSUM_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
            break;

        case PRV_TGF_IP_HEADER_ERROR_VERSION_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT;
            break;

        case PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT;
            break;

        case PRV_TGF_IP_HEADER_ERROR_SIP_DIP_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChUnicastRpfType function
* @endinternal
*
* @brief   Convert generic uRPF type into device specific uRPF type
*
* @param[in] uRpfType                 - unicast RPF type
*
* @param[out] dxChURpfTypePtr          - (pointer to) DxCh unicast RPF type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChUnicastRpfType
(
    IN  PRV_TGF_IP_URPF_MODE_ENT            uRpfType,
    OUT CPSS_DXCH_IP_URPF_MODE_ENT         *dxChURpfTypePtr
)
{
    /* convert routeType into device specific format */
    switch (uRpfType)
    {
    case PRV_TGF_IP_URPF_DISABLE_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_DISABLE_MODE_E;
        break;
    case PRV_TGF_IP_URPF_VLAN_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_VLAN_MODE_E;
        break;
    case PRV_TGF_IP_URPF_PORT_TRUNK_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E;
        break;
    case PRV_TGF_IP_URPF_LOOSE_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_LOOSE_MODE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericUnicastRpfType function
* @endinternal
*
* @brief   Convert device specific unicast RPF type into generic unicast RPF type
*
* @param[in] dxChURpfType             - DxCh unicast RPF type
*
* @param[out] uRpfTypePtr              - (pointer to) unicast RPF type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericUnicastRpfType
(
    IN  CPSS_DXCH_IP_URPF_MODE_ENT          dxChURpfType,
    OUT PRV_TGF_IP_URPF_MODE_ENT           *uRpfTypePtr
)
{
    /* convert routeType from device specific format */
    switch (dxChURpfType)
    {
    case CPSS_DXCH_IP_URPF_DISABLE_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_DISABLE_MODE_E;
        break;

    case CPSS_DXCH_IP_URPF_VLAN_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_VLAN_MODE_E;
        break;

    case CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_PORT_TRUNK_MODE_E;
        break;

    case CPSS_DXCH_IP_URPF_LOOSE_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_LOOSE_MODE_E;
        break;

    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChUnicastRpfLooseModeType function
* @endinternal
*
* @brief   Convert generic uRPF loose mode type into device specific uRPF loose mode type
*
* @param[in] uRpfLooseModeType        - unicast RPF loose mode type
*
* @param[out] dxChURpfLooseModeTypePtr - (pointer to) DxCh unicast RPF loose mode type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChUnicastRpfLooseModeType
(
    IN  PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT    uRpfLooseModeType,
    OUT CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT  *dxChURpfLooseModeTypePtr
)
{
    /* convert routeType into device specific format */
    switch (uRpfLooseModeType)
    {
    case PRV_TGF_URPF_LOOSE_MODE_TYPE_0_E:
        *dxChURpfLooseModeTypePtr = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E;
        break;
    case PRV_TGF_URPF_LOOSE_MODE_TYPE_1_E:
        *dxChURpfLooseModeTypePtr = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_1_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfIpRoutingModeGet function
* @endinternal
*
* @brief   Return the current routing mode
*
* @param[out] routingModePtr           - the current routing mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpRoutingModeGet
(
    OUT PRV_TGF_IP_ROUTING_MODE_ENT *routingModePtr
)
{
#ifdef CHX_FAMILY
    if (prvUtfIsPbrModeUsed())
    {
        *routingModePtr = PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E;
    }
    else
        *routingModePtr = PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E;
#endif /* CHX_FAMILY */

    return GT_OK;
}

/**
* @internal prvTgfIpUcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpUcRouteEntriesWrite
(
    IN GT_U32                         baseRouteEntryIndex,
    IN PRV_TGF_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                         numOfRouteEntries
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
    GT_U32      entryIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *dxChRouteEntriesArray;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* allocate memory */
    dxChRouteEntriesArray = cpssOsMalloc(numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChRouteEntriesArray, 0,
                 numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* convert IP UC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        dxChRouteEntriesArray[entryIter].type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        /* convert IP UC route entry */
        rc = prvTgfConvertGenericToDxChIpUcRouteEntry(&routeEntriesArray[entryIter],
                                                 &(dxChRouteEntriesArray[entryIter].entry.regularEntry));

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(dxChRouteEntriesArray);

            return rc;
        }
    }

    /* prepare device iterator */
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(dxChRouteEntriesArray);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpUcRouteEntriesWrite(devNum, baseRouteEntryIndex, dxChRouteEntriesArray, numOfRouteEntries);
        if (GT_OK != rc)
        {
            /* give indication that test that use IP failed due to lack of support in the CPSS for {dev,port}/{trunk}*/
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
            {
                for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
                {
                    if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.type == CPSS_INTERFACE_PORT_E)
                    {
                        if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.devPort.portNum >= BIT_6 ||
                           dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.devPort.hwDevNum >= BIT_5)
                        {
                            PRV_UTF_LOG1_MAC(" [%s]    prvTgfIpUcRouteEntriesWrite : the E-Arch for port >= 64 / hwDevNum >= 32 not supported yet \n",utfTestNameGet());
                            break;
                        }
                    }
                    else if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.type == CPSS_INTERFACE_TRUNK_E)
                    {
                        if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.trunkId >= BIT_7)
                        {
                            PRV_UTF_LOG1_MAC(" [%s]   prvTgfIpUcRouteEntriesWrite : the E-Arch for trunkId >= 127 not supported yet \n",utfTestNameGet());
                            break;
                        }
                    }
                }
            }


            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpUcRouteEntriesWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    /* free allocated memory */
    cpssOsFree(dxChRouteEntriesArray);

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpUcRouteEntriesRead function
* @endinternal
*
* @brief   Reads an array of uc route entries from the hw
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to start reading
* @param[in,out] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array
* @param[in,out] routeEntriesArray        - the uc route entries array read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpUcRouteEntriesRead
(
    IN    GT_U8                          devNum,
    IN    GT_U32                         baseRouteEntryIndex,
    INOUT PRV_TGF_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN    GT_U32                         numOfRouteEntries
)
{
    GT_STATUS   rc        = GT_OK;
    GT_U32      entryIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *dxChRouteEntriesArray;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* allocate memory */
    dxChRouteEntriesArray = cpssOsMalloc(numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChRouteEntriesArray, 0,
                 numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* convert IP UC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        dxChRouteEntriesArray[entryIter].type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        /* convert IP UC route entry */
        rc = prvTgfConvertGenericToDxChIpUcRouteEntry(&routeEntriesArray[entryIter],
                                                 &(dxChRouteEntriesArray[entryIter].entry.regularEntry));

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(dxChRouteEntriesArray);

            return rc;
        }
    }

    /* call device specific API */
    rc = cpssDxChIpUcRouteEntriesRead(devNum, baseRouteEntryIndex, dxChRouteEntriesArray, numOfRouteEntries);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpUcRouteEntriesRead FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(dxChRouteEntriesArray);

        return rc;
    }

    /* convert IP UC route entry from device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        /* convert IP UC route entry */
        prvTgfConvertDxChToGenericIpUcRouteEntry(&(dxChRouteEntriesArray[entryIter].entry.regularEntry),
                                                 &routeEntriesArray[entryIter]);
    }

    /* free allocated memory */
    cpssOsFree(dxChRouteEntriesArray);

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterArpAddrWrite function
* @endinternal
*
* @brief   Write a ARP MAC address to the router ARP / Tunnel start Table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
GT_STATUS prvTgfIpRouterArpAddrWrite
(
    IN GT_U32                         routerArpIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpRouterArpAddrWrite(devNum, routerArpIndex, arpMacAddrPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpRouterArpAddrWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterArpAddrRead function
* @endinternal
*
* @brief   Read a ARP MAC address from the router ARP / Tunnel start Table
*
* @param[in] devNum                   - the device number
* @param[in] routerArpIndex           - The Arp Address index
*
* @param[out] arpMacAddrPtr            - the ARP MAC address to read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
GT_STATUS prvTgfIpRouterArpAddrRead
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        routerArpIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterArpAddrRead(devNum, routerArpIndex, arpMacAddrPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv4 prefix
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                               override,
    IN GT_BOOL                               defragmentationEnable
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC         tempSystemRecovery_Info;
    GT_BOOL                               managerHwWriteBlock;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    cpssOsMemSet(&dxChNextHopInfo,0,sizeof(dxChNextHopInfo));

    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            pclIpUcActionPtr = &nextHopInfoPtr->pclIpUcAction;

            rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo.pclIpUcAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            ipLttEntryPtr = &nextHopInfoPtr->ipLttEntry;

            /* set route type */
            rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo.ipLttEntry.routeType));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert ltt entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, numOfPaths);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, priority);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, applyPbr );

            break;

        default:

            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &dxChNextHopInfo, override, defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvTgfIpValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmActivityBitEnableGet function
* @endinternal
*
* @brief   Get status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmActivityBitEnableGet
(
    OUT GT_BOOL     *activityBitPtr
)
{
    return cpssDxChIpLpmActivityBitEnableGet(prvTgfDevNum, activityBitPtr);
}

/**
* @internal prvTgfIpLpmActivityBitEnableSet function
* @endinternal
*
* @brief   Set status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmActivityBitEnableSet
(
    IN  GT_BOOL     activityBit
)
{
    return cpssDxChIpLpmActivityBitEnableSet(prvTgfDevNum, activityBit);
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 UC prefix for a specific LPM DB
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixActivityStatusGet
(
    IN GT_U32       lpmDbId,
    IN GT_U32       vrId,
    IN GT_IPADDR    ipAddr,
    IN GT_U32       prefixLen,
    IN GT_BOOL      clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(
                lpmDbId,
                vrId,
                &ipAddr,
                prefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv4McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2; Bobcat2.
*
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv4McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   ipGroup,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPADDR   ipSrc,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv4McEntryActivityStatusGet(
                lpmDbId,
                vrId,
                &ipGroup,
                ipGroupPrefixLen,
                &ipSrc,
                ipSrcPrefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv6McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2; Bobcat2.
*
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv6McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR ipGroup,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPV6ADDR ipSrc,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv6McEntryActivityStatusGet(
                lpmDbId,
                vrId,
                &ipGroup,
                ipGroupPrefixLen,
                &ipSrc,
                ipSrcPrefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixActivityStatusGet
(
    IN GT_U32       lpmDbId,
    IN GT_U32       vrId,
    IN GT_IPV6ADDR  ipAddr,
    IN GT_U32       prefixLen,
    IN GT_BOOL      clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv6UcPrefixActivityStatusGet(
                lpmDbId,
                vrId,
                &ipAddr,
                prefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv4 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         vrId,
    IN GT_IPADDR                      ipAddr,
    IN GT_U32                         prefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;


#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv6 prefix
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If prefix length is too big.
* @retval GT_ERROR                 - If the vrId was not created yet.
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate TCAM memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId,
    IN GT_IPV6ADDR                           ipAddr,
    IN GT_U32                                prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                               override,
    IN GT_BOOL                               defragmentationEnable
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC         tempSystemRecovery_Info;
    GT_BOOL                               managerHwWriteBlock;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    cpssOsMemSet(&dxChNextHopInfo,0,sizeof(dxChNextHopInfo));

    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            pclIpUcActionPtr = &nextHopInfoPtr->pclIpUcAction;

            rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo.pclIpUcAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            ipLttEntryPtr = &nextHopInfoPtr->ipLttEntry;

            /* convert routeType into device specific format */
            rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo.ipLttEntry.routeType));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

                return rc;
            }
            /* convert ltt entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, numOfPaths);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, priority);


            break;

        default:

            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId,
                                        vrId,
                                        &ipAddr,
                                        prefixLen,
                                        &dxChNextHopInfo,
                                        override,
                                        defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv6 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         vrId,
    IN GT_IPV6ADDR                    ipAddr,
    IN GT_U32                         prefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6UcPrefixesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfCountersIpSet function
* @endinternal
*
* @brief   Set route entry mode and reset IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfCountersIpSet
(
    IN GT_U32                         portNum,
    IN GT_U32                         counterIndex
)
{
    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode       = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg;
    CPSS_DXCH_IP_COUNTER_SET_STC               ipCounters;

    cpssOsMemSet(&interfaceModeCfg, 0, sizeof(interfaceModeCfg));
    cpssOsMemSet(&ipCounters, 0, sizeof(ipCounters));
#endif /* CHX_FAMILY */

    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* set counter mode */
    cntSetMode = CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E;

    /* set ROUTE_ENTRY mode for IP counters */
    interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceModeCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceModeCfg.vlanMode         = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
    interfaceModeCfg.hwDevNum         = prvTgfDevsArray[portIter]; /* don't care */
    interfaceModeCfg.portTrunk.port   = portNum;
    interfaceModeCfg.portTrunk.trunk  = 0;
    interfaceModeCfg.vlanId           = 0;

    rc = cpssDxChIpCntSetModeSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex,
                                 cntSetMode, &interfaceModeCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSetModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* reset IP couters */
    rc = cpssDxChIpCntSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex, &ipCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfCountersIpVlanModeSet function
* @endinternal
*
* @brief   Set route entry VLAN interface mode and reset IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfCountersIpVlanModeSet
(
    IN GT_U32                         vlanId,
    IN GT_U32                         counterIndex
)
{
    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode       = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg;
    CPSS_DXCH_IP_COUNTER_SET_STC               ipCounters;

    cpssOsMemSet(&interfaceModeCfg, 0, sizeof(interfaceModeCfg));
    cpssOsMemSet(&ipCounters, 0, sizeof(ipCounters));

    /* set ROUTE_ENTRY mode for IP counters */
    interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceModeCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceModeCfg.vlanMode         = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E;
    interfaceModeCfg.hwDevNum         = prvTgfDevsArray[portIter]; /* don't care */
    interfaceModeCfg.portTrunk.port   = 0;  /* don't care */
    interfaceModeCfg.portTrunk.trunk  = 0;  /* don't care */
    interfaceModeCfg.vlanId           = (GT_U16)vlanId;

    rc = cpssDxChIpCntSetModeSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex,
                                 cntSetMode, &interfaceModeCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSetModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* reset IP couters */
    rc = cpssDxChIpCntSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex, &ipCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfCountersIpGet function
* @endinternal
*
* @brief   This function gets the values of the various counters in the IP
*         Router Management Counter-Set.
* @param[in] devNum                   - device number
* @param[in] counterIndex             - counter index
* @param[in] enablePrint              - Enable/Disable output log
*
* @param[out] ipCountersPtr            - (pointer to) IP counters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfCountersIpGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterIndex,
    IN  GT_BOOL                       enablePrint,
    OUT PRV_TGF_IP_COUNTER_SET_STC   *ipCountersPtr
)
{
    GT_STATUS   rc       = GT_OK;
    GT_BOOL     isZero   = GT_TRUE;
    GT_U32      cntIter  = 0;
    GT_U32      cntNext  = 0;
    GT_U32      cntCount = 0;
    GT_U8       ipCountersName[][PRV_TGF_MAX_FIELD_NAME_LEN_CNS] =
        {"inUcPkts", "inMcPkts", "inUcNonRoutedExcpPkts", "inUcNonRoutedNonExcpPkts",
         "inMcNonRoutedExcpPkts", "inMcNonRoutedNonExcpPkts", "inUcTrappedMirrorPkts",
         "inMcTrappedMirrorPkts", "mcRfpFailPkts", "outUcRoutedPkts"};
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_COUNTER_SET_STC dxChIpCounters;
    cpssOsMemSet(&dxChIpCounters, 0, sizeof(dxChIpCounters));
#endif /* CHX_FAMILY */


    /* get counter count */
    cntCount = sizeof(ipCountersName) / sizeof(ipCountersName[0]);

#ifdef CHX_FAMILY
    /* get counters */
    rc = cpssDxChIpCntGet(devNum, (CPSS_IP_CNT_SET_ENT) counterIndex, &dxChIpCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert counters from cpss */
    ipCountersPtr->inUcPkts = dxChIpCounters.inUcPkts;
    ipCountersPtr->inMcPkts = dxChIpCounters.inMcPkts;
    ipCountersPtr->inUcNonRoutedExcpPkts = dxChIpCounters.inUcNonRoutedExcpPkts;
    ipCountersPtr->inUcNonRoutedNonExcpPkts = dxChIpCounters.inUcNonRoutedNonExcpPkts;
    ipCountersPtr->inMcNonRoutedExcpPkts = dxChIpCounters.inMcNonRoutedExcpPkts;
    ipCountersPtr->inMcNonRoutedNonExcpPkts = dxChIpCounters.inMcNonRoutedNonExcpPkts;
    ipCountersPtr->inUcTrappedMirrorPkts = dxChIpCounters.inUcTrappedMirrorPkts;
    ipCountersPtr->inMcTrappedMirrorPkts = dxChIpCounters.inMcTrappedMirrorPkts;
    ipCountersPtr->mcRfpFailPkts = dxChIpCounters.mcRfpFailPkts;
    ipCountersPtr->outUcRoutedPkts = dxChIpCounters.outUcRoutedPkts;

#endif /* CHX_FAMILY */

    if (GT_TRUE == enablePrint)
    {
        /* print all not zero IP counters values */
        isZero = GT_TRUE;
        for (cntIter = 0; cntIter < cntCount; cntIter++)
        {
            /* next ip counter from CPSS_DXCH_IP_COUNTER_SET_STC */
            cntNext = *(cntIter + (GT_U32*) ipCountersPtr);

            if (cntNext != 0)
            {
                isZero = GT_FALSE;
                PRV_UTF_LOG2_MAC("  %s = %d\n", &ipCountersName[cntIter], cntNext);
            }
        }

        if (GT_TRUE == isZero)
        {
            PRV_UTF_LOG0_MAC("  All IP Counters are Zero\n");
        }

        PRV_UTF_LOG0_MAC("\n");
    }

    return rc;
}

/**
* @internal prvTgfIpPortRoutingEnable function
* @endinternal
*
* @brief   Enable multicast/unicast IPv4/v6 routing on a port
*
* @param[in] portIndex                - index of port to enable
* @param[in] ucMcEnable               - routing type to enable Unicast/Multicast
* @param[in] protocol                 - what type of traffic to enable ipv4 or ipv6 or both
* @param[in] enable                   -  IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpPortRoutingEnable
(
    IN GT_U8                          portIndex,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN GT_BOOL                        enable
)
{
    PRV_TGF_PORT_INDEX_CHECK_MAC(portIndex);

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRoutingEnable(prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
                                       ucMcEnable, protocol, enable);
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpPortRoutingEnableGet function
* @endinternal
*
* @brief   Get status of multicast/unicast IPv4/v6 routing on a port.
*
* @param[out] enablePtr                - (pointer to)enable IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note CPSS_IP_PROTOCOL_IPV4V6_E is not supported in this get API.
*       Can not get both values for ipv4 and ipv6 in the same get.
*
*/
GT_STATUS prvTgfIpPortRoutingEnableGet
(
    IN GT_U8                          portIndex,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    OUT GT_BOOL                       *enablePtr
)
{
    PRV_TGF_PORT_INDEX_CHECK_MAC(portIndex);

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRoutingEnableGet(prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
                                       ucMcEnable, protocol, enablePtr);
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpVlanRoutingEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 multicast/unicast Routing on Vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfIpVlanRoutingEnable
(
    IN GT_U16                         vlanId,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN GT_BOOL                        enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        switch (ucMcEnable)
        {
            case CPSS_IP_UNICAST_E:
                /* call device specific API */
                rc = cpssDxChBrgVlanIpUcRouteEnable(devNum, vlanId, protocol, enable);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanIpUcRouteEnable FAILED, rc = [%d]", rc);

                    rc1 = rc;
                }

                break;

            case CPSS_IP_MULTICAST_E:
                /* call device specific API */
                rc = cpssDxChBrgVlanIpMcRouteEnable(devNum, vlanId, protocol, enable);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanIpMcRouteEnable FAILED, rc = [%d]", rc);

                    rc1 = rc;
                }

                break;

            default:
                rc1 = GT_BAD_PARAM;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmVirtualRouterAdd function
* @endinternal
*
* @brief   This function adds a virtual router in system for specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterAdd
(
    IN GT_U32                                lpmDbId,
    IN GT_U32                                vrId,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defFcoeNextHopInfoPtr
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;

    cpssOsMemSet(&vrConfigInfo, 0, sizeof(vrConfigInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    if (NULL != defIpv4UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv4UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv4UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
    }
    if (NULL != defIpv6UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv6UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv6UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
    }

    if (NULL != defIpv4McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv4McRouteLttEntryPtr,
                                                 &vrConfigInfo.defIpv4McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv4Mc = GT_TRUE;
    }
    if (NULL != defIpv6McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv6McRouteLttEntryPtr,
                                                     &vrConfigInfo.defIpv6McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv6Mc = GT_TRUE;
    }
    if (NULL != defFcoeNextHopInfoPtr)
    {
       rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                    defFcoeNextHopInfoPtr,
                                                    &vrConfigInfo.defaultFcoeForwardingNextHopInfo);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportFcoe = GT_TRUE;
    }

    /* call device specific API */
    return cpssDxChIpLpmVirtualRouterAdd(lpmDbId, vrId,&vrConfigInfo);
}

/**
* @internal prvTgfIpLpmVirtualRouterAddDefault function
* @endinternal
*
* @brief   This function adds the default virtual router in the default LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterAddDefault
(
    GT_VOID
)
{
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC             defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC             defIpv6McRouteLttEntry;

    cpssOsMemSet(&defIpv4UcNextHopInfo,0,sizeof(defIpv4UcNextHopInfo));
    cpssOsMemSet(&defIpv6UcNextHopInfo,0,sizeof(defIpv6UcNextHopInfo));
    cpssOsMemSet(&defIpv4McRouteLttEntry,0,sizeof(defIpv4McRouteLttEntry));
    cpssOsMemSet(&defIpv6McRouteLttEntry,0,sizeof(defIpv6McRouteLttEntry));

    defIpv4UcNextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv6UcNextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
    defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv4McRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv4McRouteLttEntry.numOfPaths = 0;
    defIpv4McRouteLttEntry.routeEntryBaseIndex = 1;
    defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv6McRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv6McRouteLttEntry.numOfPaths = 0;
    defIpv6McRouteLttEntry.routeEntryBaseIndex = 1;
    defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    return prvTgfIpLpmVirtualRouterAdd(0, 0,
                                       &defIpv4UcNextHopInfo,
                                       &defIpv6UcNextHopInfo,
                                       &defIpv4McRouteLttEntry,
                                       &defIpv6McRouteLttEntry,
                                       NULL);
}

/**
* @internal prvTgfIpLpmVirtualRouterSharedAdd function
* @endinternal
*
* @brief   This function adds shared virtual router in system for specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterSharedAdd
(
    IN GT_U32                                lpmDbId,
    IN GT_U32                                vrId,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defFcoeNextHopInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
#endif /* CHX_FAMILY */

    cpssOsMemSet(&vrConfigInfo,0,sizeof(vrConfigInfo));

#ifdef CHX_FAMILY

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    if (NULL != defIpv4UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv4UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv4UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
    }
    if (NULL != defIpv6UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv6UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv6UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
    }

    if (NULL != defIpv4McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv4McRouteLttEntryPtr,
                                                 &vrConfigInfo.defIpv4McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv4Mc = GT_TRUE;
    }
    if (NULL != defIpv6McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv6McRouteLttEntryPtr,
                                                     &vrConfigInfo.defIpv6McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv6Mc = GT_TRUE;
    }
    if (NULL != defFcoeNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defFcoeNextHopInfoPtr,
                                                     &vrConfigInfo.defaultFcoeForwardingNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportFcoe = GT_TRUE;
    }

    /* call device specific API */
    return cpssDxChIpLpmVirtualRouterSharedAdd(lpmDbId, vrId, &vrConfigInfo);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmVirtualRouterDel function
* @endinternal
*
* @brief   This function removes a virtual router in system for a specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
*/
GT_STATUS prvTgfIpLpmVirtualRouterDel
(
    IN GT_U32                         lpmDbId,
    IN GT_U32                         vrId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmVirtualRouterDelDefault function
* @endinternal
*
* @brief   This function deletes the default virtual router in the default LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
*/
GT_STATUS prvTgfIpLpmVirtualRouterDelDefault
(
    GT_VOID
)
{
    return prvTgfIpLpmVirtualRouterDel(0, 0);
}

/**
* @internal prvTgfIpLpmVirtualRouterGet function
* @endinternal
*
* @brief   This function gets the virtual router in system for specific LPM DB
*
* @param[out] isSupportIpv4Uc          - (pointer to) whether VR support UC Ipv4
* @param[out] defIpv4UcNextHopInfoPtr  - (pointer to) ipv4 uc next hop info
* @param[out] isSupportIpv6Uc          - (pointer to) whether VR support UC Ipv6
* @param[out] defIpv6UcNextHopInfoPtr  - (pointer to) ipv6 uc next hop info
* @param[out] isSupportIpv4Mc          - (pointer to) whether VR support MC Ipv4
* @param[out] defIpv4McRouteLttEntryPtr - (pointer to) ipv4 mc LTT entry info
* @param[out] isSupportIpv6Mc          - (pointer to) whether VR support MC Ipv6
* @param[out] defIpv6McRouteLttEntryPtr - (pointer to) ipv6 mc LTT entry info
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterGet
(
    IN  GT_U32                                lpmDbId,
    IN  GT_U32                                vrId,
    OUT GT_BOOL                              *isSupportIpv4Uc,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    OUT GT_BOOL                              *isSupportIpv6Uc,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    OUT GT_BOOL                              *isSupportIpv4Mc,
    OUT PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    OUT GT_BOOL                              *isSupportIpv6Mc,
    OUT PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                       rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC  vrConfigInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    cpssOsMemSet(&vrConfigInfo,0,sizeof(vrConfigInfo));

    CPSS_NULL_PTR_CHECK_MAC(defIpv4UcNextHopInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(defIpv6UcNextHopInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(defIpv4McRouteLttEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(defIpv6McRouteLttEntryPtr);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpLpmVirtualRouterGet(lpmDbId, vrId, &vrConfigInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);
        return rc;
    }

    *isSupportIpv4Uc = vrConfigInfo.supportIpv4Uc;
    *isSupportIpv6Uc = vrConfigInfo.supportIpv6Uc;
    *isSupportIpv4Mc = vrConfigInfo.supportIpv4Mc;
    *isSupportIpv6Mc = vrConfigInfo.supportIpv6Mc;

    /* convert generic UC next hop info from DX specific UC next hop info */
    if (GT_TRUE == vrConfigInfo.supportIpv4Uc)
    {
        rc = prvTgfConvertDxChToGenericUcNextHopInfo(routingMode,
                                                     &vrConfigInfo.defIpv4UcNextHopInfo,
                                                     defIpv4UcNextHopInfoPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    /* convert generic UC next hop info from DX specific UC next hop info */
    if (GT_TRUE == vrConfigInfo.supportIpv6Uc)
    {
        rc = prvTgfConvertDxChToGenericUcNextHopInfo(routingMode,
                                                     &vrConfigInfo.defIpv6UcNextHopInfo,
                                                     defIpv6UcNextHopInfoPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    /* convert generic mc LTT info from DX specific mc LTT info */
    if (GT_TRUE == vrConfigInfo.supportIpv4Mc)
    {
        rc = prvTgfConvertDxChToGenericMcLttEntryInfo(&vrConfigInfo.defIpv4McRouteLttEntry,
                                                      defIpv4McRouteLttEntryPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericMcLttEntryInfo FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    /* convert generic mc LTT info from DX specific mc LTT info */
    if (GT_TRUE == vrConfigInfo.supportIpv6Mc)
    {
       rc = prvTgfConvertDxChToGenericMcLttEntryInfo(&vrConfigInfo.defIpv6McRouteLttEntry,
                                                     defIpv6McRouteLttEntryPtr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
    }

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpLttWrite
(
    IN GT_U32                         lttTtiRow,
    IN GT_U32                         lttTtiColumn,
    IN PRV_TGF_IP_LTT_ENTRY_STC      *lttEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChLttEntry;

    cpssOsMemSet(&dxChLttEntry,0,sizeof(dxChLttEntry));


    /* set LTT entry */
    dxChLttEntry.numOfPaths = lttEntryPtr->numOfPaths;
    dxChLttEntry.routeEntryBaseIndex = lttEntryPtr->routeEntryBaseIndex;
    dxChLttEntry.ucRPFCheckEnable = lttEntryPtr->ucRPFCheckEnable;
    dxChLttEntry.sipSaCheckMismatchEnable = lttEntryPtr->sipSaCheckMismatchEnable;
    dxChLttEntry.ipv6MCGroupScopeLevel = lttEntryPtr->ipv6MCGroupScopeLevel;

    /* set route type */
    rc = prvTgfConvertGenericToDxChRouteType(lttEntryPtr->routeType, &(dxChLttEntry.routeType));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRouteType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpLttWrite(devNum, lttTtiRow, lttTtiColumn, &dxChLttEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLttWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv4PrefixSet
(
    IN GT_U32                         routerTtiTcamRow,
    IN GT_U32                         routerTtiTcamColumn,
    IN PRV_TGF_IPV4_PREFIX_STC       *prefixPtr,
    IN PRV_TGF_IPV4_PREFIX_STC       *maskPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    CPSS_DXCH_IPV4_PREFIX_STC   dxChPrefix;
    CPSS_DXCH_IPV4_PREFIX_STC   dxChMask;

    cpssOsMemSet(&dxChPrefix,0,sizeof(dxChPrefix));
    cpssOsMemSet(&dxChMask,0,sizeof(dxChMask));


    /* convert Ipv4 prefix entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, mcGroupIndexRow);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, mcGroupIndexColumn);

    /* convert Ipv4 mask entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, mcGroupIndexRow);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, mcGroupIndexColumn);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv4PrefixSet(devNum, routerTtiTcamRow, routerTtiTcamColumn,
                                   &dxChPrefix, &dxChMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv4PrefixSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv6PrefixSet function
* @endinternal
*
* @brief   Set an ipv6 UC or MC prefix to the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv6PrefixSet
(
    IN GT_U32                  routerTtiTcamRow,
    IN PRV_TGF_IPV6_PREFIX_STC *prefixPtr,
    IN PRV_TGF_IPV6_PREFIX_STC *maskPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                     devNum  = 0;
    GT_STATUS                 rc, rc1 = GT_OK;
    CPSS_DXCH_IPV6_PREFIX_STC dxChPrefix;
    CPSS_DXCH_IPV6_PREFIX_STC dxChMask;

    cpssOsMemSet(&dxChPrefix,0,sizeof(dxChPrefix));
    cpssOsMemSet(&dxChMask,0,sizeof(dxChMask));

    /* convert Ipv6 prefix entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, mcGroupIndexRow);

    /* convert Ipv6 mask entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, mcGroupIndexRow);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv6PrefixSet(devNum, routerTtiTcamRow,
                                   &dxChPrefix, &dxChMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv6PrefixSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv4PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv4 UC or MC prefix in the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv4PrefixInvalidate
(
    IN GT_U32                         routerTtiTcamRow,
    IN GT_U32                         routerTtiTcamColumn
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv4PrefixInvalidate(devNum, routerTtiTcamRow, routerTtiTcamColumn);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv4PrefixInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv6PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv6 UC or MC prefix in the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv6PrefixInvalidate
(
    IN GT_U32 routerTtiTcamRow
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv6PrefixInvalidate(devNum, routerTtiTcamRow);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv6PrefixInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpMcRouteEntriesWrite function
* @endinternal
*
* @brief   Write an array of MC route entries to hw
*
* @param[in] ucMcBaseIndex            - base Index in the Route entries table
* @param[in] numOfEntries             - number of route entries to write
* @param[in] protocol                 - ip  type
* @param[in] entriesArrayPtr          - MC route entry array to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfIpMcRouteEntriesWrite
(
    IN GT_U32                         ucMcBaseIndex,
    IN GT_U32                         numOfEntries,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN PRV_TGF_IP_MC_ROUTE_ENTRY_STC *entriesArrayPtr
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
    GT_U32      entryIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *dxChRouteEntriesArray;
#endif /* CHX_FAMILY */

    switch (protocol) {
    case CPSS_IP_PROTOCOL_IPV4_E:
    case CPSS_IP_PROTOCOL_IPV6_E:
        break;
    default:
        return GT_BAD_PARAM;
    }
    for (entryIter = 0; entryIter < numOfEntries; entryIter++)
    {
        if (protocol == CPSS_IP_PROTOCOL_IPV6_E)
        {
            if (entriesArrayPtr[entryIter].ipv6InternalMllPtr != entriesArrayPtr[entryIter].ipv6ExternalMllPtr + 1 )
            {
              rc = GT_BAD_PARAM;
              PRV_UTF_LOG1_MAC("[TGF]: ipv6InternalMllPtr should be equal to ipv6ExternalMllPtr + 1 to support EXMXPM dit internal/external functionality , rc = [%d]", rc);
            }
        }
    }

#ifdef CHX_FAMILY
    /* allocate memory */
    dxChRouteEntriesArray = cpssOsMalloc(numOfEntries * sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChRouteEntriesArray, 0,
                 numOfEntries * sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    /* convert IP MC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfEntries; entryIter++)
    {
        /* convert IP MC route entry */
        rc = prvTgfConvertGenericToDxChIpMcRouteEntry(protocol,
                                                      &entriesArrayPtr[entryIter],
                                                      &(dxChRouteEntriesArray[entryIter]));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpMcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(dxChRouteEntriesArray);

            return rc;
        }
    }

    /* prepare device iterator */
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(dxChRouteEntriesArray);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (entryIter = 0; entryIter < numOfEntries; entryIter++)
        {
            /* call device specific API */
            rc = cpssDxChIpMcRouteEntriesWrite(devNum, ucMcBaseIndex + entryIter, &(dxChRouteEntriesArray[entryIter]));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpMcRouteEntriesWrite FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
    }

    /* free allocated memory */
    cpssOsFree(dxChRouteEntriesArray);

    return rc1;
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpMllPairWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) pair entry to hw
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllPairWriteForm.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpMllPairWrite
(
    IN GT_U32                                   mllPairEntryIndex,
    IN PRV_TGF_PAIR_READ_WRITE_FORM_ENT         mllPairWriteForm,
    IN PRV_TGF_IP_MLL_PAIR_STC                  *mllPairEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT   dxChMllPairWriteForm;
    CPSS_DXCH_IP_MLL_PAIR_STC            dxChMllPairEntry;

    cpssOsMemSet(&dxChMllPairEntry,0,sizeof(dxChMllPairEntry));


    /* convert mllPairWriteForm into device specific format */
    switch (mllPairWriteForm)
    {
        case PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E:
            dxChMllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
            break;

        case PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            dxChMllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;
            break;

        case PRV_TGF_PAIR_READ_WRITE_WHOLE_E:
            dxChMllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mllPairEntryPtr->firstMllNode into device specific format */
    dxChMllPairEntry.firstMllNode.mllRPFFailCommand = mllPairEntryPtr->firstMllNode.mllRPFFailCommand;
    dxChMllPairEntry.firstMllNode.isTunnelStart = mllPairEntryPtr->firstMllNode.isTunnelStart;
    dxChMllPairEntry.firstMllNode.nextHopVlanId = mllPairEntryPtr->firstMllNode.nextHopVlanId;
    dxChMllPairEntry.firstMllNode.nextHopTunnelPointer = mllPairEntryPtr->firstMllNode.nextHopTunnelPointer;

    cpssOsMemCpy((GT_VOID*) &(dxChMllPairEntry.firstMllNode.nextHopInterface),
                 (GT_VOID*) &(mllPairEntryPtr->firstMllNode.nextHopInterface),
                  sizeof(mllPairEntryPtr->firstMllNode.nextHopInterface));

    /* convert firstMllNode.tunnelStartPassengerType into device specific format */
    switch (mllPairEntryPtr->firstMllNode.tunnelStartPassengerType)
    {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChMllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChMllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    dxChMllPairEntry.firstMllNode.ttlHopLimitThreshold = mllPairEntryPtr->firstMllNode.ttlHopLimitThreshold;
    dxChMllPairEntry.firstMllNode.excludeSrcVlan = mllPairEntryPtr->firstMllNode.excludeSrcVlan;
    dxChMllPairEntry.firstMllNode.last = mllPairEntryPtr->firstMllNode.last;

    /* convert mllPairEntryPtr->secondMllNode into device specific format */
    dxChMllPairEntry.secondMllNode.mllRPFFailCommand = mllPairEntryPtr->secondMllNode.mllRPFFailCommand;
    dxChMllPairEntry.secondMllNode.isTunnelStart = mllPairEntryPtr->secondMllNode.isTunnelStart;
    dxChMllPairEntry.secondMllNode.nextHopVlanId = mllPairEntryPtr->secondMllNode.nextHopVlanId;
    dxChMllPairEntry.secondMllNode.nextHopTunnelPointer = mllPairEntryPtr->secondMllNode.nextHopTunnelPointer;

    cpssOsMemCpy((GT_VOID*) &(dxChMllPairEntry.secondMllNode.nextHopInterface),
                     (GT_VOID*) &(mllPairEntryPtr->secondMllNode.nextHopInterface),
                      sizeof(mllPairEntryPtr->secondMllNode.nextHopInterface));

    /* convert secondMllNode.tunnelStartPassengerType into device specific format */
    switch (mllPairEntryPtr->secondMllNode.tunnelStartPassengerType)
    {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChMllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChMllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    dxChMllPairEntry.secondMllNode.ttlHopLimitThreshold = mllPairEntryPtr->secondMllNode.ttlHopLimitThreshold;
    dxChMllPairEntry.secondMllNode.excludeSrcVlan = mllPairEntryPtr->secondMllNode.excludeSrcVlan;
    dxChMllPairEntry.secondMllNode.last = mllPairEntryPtr->secondMllNode.last;

    dxChMllPairEntry.nextPointer = mllPairEntryPtr->nextPointer;

    if(mllPairWriteForm == PRV_TGF_PAIR_READ_WRITE_WHOLE_E ||
        mllPairWriteForm == PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E)
    {
        if( CPSS_INTERFACE_PORT_E == mllPairEntryPtr->firstMllNode.nextHopInterface.type )
        {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH
            rc = prvUtfHwDeviceNumberGet((GT_U8)mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum,
                                         &dxChMllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

                return rc;
            }
        }
    }

    if(mllPairWriteForm == PRV_TGF_PAIR_READ_WRITE_WHOLE_E ||
        mllPairWriteForm ==
        PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E)
    {
        if( CPSS_INTERFACE_PORT_E == mllPairEntryPtr->secondMllNode.nextHopInterface.type )
        {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

            rc = prvUtfHwDeviceNumberGet((GT_U8)mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum,
                                         &dxChMllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

                return rc;
            }
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpMLLPairWrite(devNum, mllPairEntryIndex, dxChMllPairWriteForm, &dxChMllPairEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpMLLPairWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBDevListGet function
* @endinternal
*
* @brief   This function retrieves the list of devices in an existing LPM DB.
*
* @param[in] lpmDbId                      - the LPM DB id.
* @param[in,out] numOfDevsPtr             - (pointer to) the size of devListArray
* @param[in,out] numOfDevsPtr             - (pointer to) the number of devices retreived
*
* @param[out] devListArray[]           - array of device ids in the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to
*                                       the device list
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The application should allocate memory for the device list array.
*       If the function returns GT_OK, then number of devices holds
*       the number of devices the function filled.
*       If the function returns GT_BAD_SIZE, then the memory allocated by the
*       application to the device list is not enough. In this case the
*       number of devices will hold the size of array needed.
*
*/
GT_STATUS prvTgfIpLpmDBDevListGet
(
    IN    GT_U32                        lpmDbId,
    INOUT GT_U32                        *numOfDevsPtr,
    OUT   GT_U8                         devListArray[]
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmDBDevListGet(lpmDbId, numOfDevsPtr, devListArray);
#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(numOfDevsPtr);
    TGF_PARAM_NOT_USED(devListArray);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */
}
/**
* @internal prvTgfIpLpmDBDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing LPM DB
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] devList[]                - the array of device ids to add to the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfIpLpmDBDevListAdd
(
    IN GT_U32                         lpmDBId,
    IN GT_U8                          devList[],
    IN GT_U32                         numOfDevs
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBDevsListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing LPM DB. this remove will
*         invoke a hot sync removal of the devices.
*
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] devListArr[]             - the array of device ids to remove from the
*                                      LPM DB.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvTgfIpLpmDBDevsListRemove
(
    IN GT_U32   lpmDBId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmDBDevsListRemove(lpmDBId, devListArr, numOfDevs);
#else
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(devListArr);
    TGF_PARAM_NOT_USED(numOfDevs);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDBDevListAddDefault function
* @endinternal
*
* @brief   This function adds a default device to an existing LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfIpLpmDBDevListAddDefault
(
    GT_VOID
)
{
    return prvTgfIpLpmDBDevListAdd(0,&prvTgfDevNum,1);
}

/**
* @internal prvTgfIpLpmIpv4McEntryAdd function
* @endinternal
*
* @brief   Add IP MC route for a particular/all source and group address
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual private network identifier
* @param[in] ipGroup                  - IP MC group address
* @param[in] ipGroupPrefixLen         - number of bits that are actual valid in ipGroup
* @param[in] ipSrc                    - root address for source base multi tree protocol
* @param[in] ipSrcPrefixLen           - number of bits that are actual valid in ipSrc
* @param[in] mcRouteLttEntryPtr       - LTT entry pointing to the MC route entry
* @param[in] override                 - whether to  an mc Route pointer
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if one of prefixes' lengths is too big
* @retval GT_ERROR                 - if the virtual router does not exist
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpLpmIpv4McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
)
{

    GT_STATUS                   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    cpssOsMemSet(&dxChMcRouteLttEntry,0,sizeof(dxChMcRouteLttEntry));

    /* set LTT entry */
    rc = prvTgfConvertGenericToDxChMcLttEntryInfo(mcRouteLttEntryPtr, &dxChMcRouteLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen, &dxChMcRouteLttEntry,
                                     override, defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmIpv4McEntryDel
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the
*         information associated with it.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixSearch
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPADDR                            ipAddr,
    IN  GT_U32                               prefixLen,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId,
                                        &ipAddr, prefixLen,
                                        &dxChnextHopInfo,
                                        tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - The prefix length of the found entry.
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this UC prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this UC prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixGet
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPADDR                            ipAddr,
    OUT GT_U32                               *prefixLenPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId,
                                      &ipAddr, prefixLenPtr,
                                      &dxChnextHopInfo,
                                      tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4McEntrySearch function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - The ip Group prefix len.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamGroupColumnIndexPtr  - pointer to TCAM group column  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
* @param[out] tcamSrcColumnIndexPtr    - pointer to TCAM source column  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
*/
GT_STATUS prvTgfIpLpmIpv4McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPADDR                   ipGroup,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPADDR                   ipSrc,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    OUT GT_U32                      *tcamGroupRowIndexPtr,
    OUT GT_U32                      *tcamGroupColumnIndexPtr,
    OUT GT_U32                      *tcamSrcRowIndexPtr,
    OUT GT_U32                      *tcamSrcColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry;

    cpssOsMemSet(&dxChMcRouteLttEntry,0,sizeof(dxChMcRouteLttEntry));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId,
                                        &ipGroup, ipGroupPrefixLen,
                                        &ipSrc, ipSrcPrefixLen,
                                        &dxChMcRouteLttEntry,
                                        tcamGroupRowIndexPtr, tcamGroupColumnIndexPtr,
                                        tcamSrcRowIndexPtr, tcamSrcColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert LTT entry */
    return prvTgfConvertDxChToGenericMcLttEntryInfo(&dxChMcRouteLttEntry, mcRouteLttEntryPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the next
*         hop pointer associated with it and TCAM prefix index.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixSearch
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPV6ADDR                          ipAddr,
    IN  GT_U32                               prefixLen,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId,
                                        &ipAddr, prefixLen,
                                        &dxChnextHopInfo,
                                        tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - The prefix length of the found entry.
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixGet
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPV6ADDR                          ipAddr,
    OUT GT_U32                               *prefixLenPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId,
                                      &ipAddr, prefixLenPtr,
                                      &dxChnextHopInfo,
                                      tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6McEntrySearch function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
*/
GT_STATUS prvTgfIpLpmIpv6McEntrySearch
(
    IN  GT_U32                   lpmDBId,
    IN  GT_U32                   vrId,
    IN  GT_IPV6ADDR              ipGroup,
    IN  GT_U32                   ipGroupPrefixLen,
    IN  GT_IPV6ADDR              ipSrc,
    IN  GT_U32                   ipSrcPrefixLen,
    OUT PRV_TGF_IP_LTT_ENTRY_STC *mcRouteLttEntryPtr,
    OUT GT_U32                   *tcamGroupRowIndexPtr,
    OUT GT_U32                   *tcamSrcRowIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry;

    cpssOsMemSet(&dxChMcRouteLttEntry,0,sizeof(dxChMcRouteLttEntry));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId,
                                        &ipGroup, ipGroupPrefixLen,
                                        &ipSrc, ipSrcPrefixLen,
                                        &dxChMcRouteLttEntry,
                                        tcamGroupRowIndexPtr,
                                        tcamSrcRowIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert LTT entry */
    return prvTgfConvertDxChToGenericMcLttEntryInfo(&dxChMcRouteLttEntry, mcRouteLttEntryPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6McEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from a particular
*         source and addressed to a particular IP multicast group address for a
*         specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteLttEntryPtr       - the LTT entry pointing to the MC route entry
*                                      associated with this MC route.
* @param[in] override                 - weather to  the mcRoutePointerPtr for the
*                                      given prefix
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big.
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented .
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note to override the default mc route use ipGroup = ipGroupPrefixLen = 0.
*
*/
GT_STATUS prvTgfIpLpmIpv6McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
)
{
    GT_STATUS                   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    cpssOsMemSet(&dxChMcRouteLttEntry,0,sizeof(dxChMcRouteLttEntry));

    /* set LTT entry */
    dxChMcRouteLttEntry.numOfPaths = mcRouteLttEntryPtr->numOfPaths;
    dxChMcRouteLttEntry.routeEntryBaseIndex = mcRouteLttEntryPtr->routeEntryBaseIndex;
    dxChMcRouteLttEntry.ucRPFCheckEnable = mcRouteLttEntryPtr->ucRPFCheckEnable;
    dxChMcRouteLttEntry.sipSaCheckMismatchEnable = mcRouteLttEntryPtr->sipSaCheckMismatchEnable;
    dxChMcRouteLttEntry.ipv6MCGroupScopeLevel = mcRouteLttEntryPtr->ipv6MCGroupScopeLevel;

    /* set route type */
    rc = prvTgfConvertGenericToDxChRouteType(mcRouteLttEntryPtr->routeType,
                                             &(dxChMcRouteLttEntry.routeType));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

        return rc;
    }
     /* call device specific API */
    rc =  cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen, &dxChMcRouteLttEntry,
                                     override, defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv4McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4McEntriesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

}



/**
* @internal prvTgfIpLpmIpv6McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmIpv6McEntryDel
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrc,
    IN GT_U32                       ipSrcPrefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

}





/**
* @internal prvTgfIpLpmIpv6McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv6McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6McEntriesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpSetMllCntInterface function
* @endinternal
*
* @brief   Sets a mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpSetMllCntInterface
(
    IN GT_U32                                    mllCntSet,
    IN PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC  dxChInterfaceCfg;

    /* reset variable */
    cpssOsMemSet(&dxChInterfaceCfg, 0, sizeof(dxChInterfaceCfg));

    /* convert Port/Trunk mode into device specific format */
    switch (interfaceCfgPtr->portTrunkCntMode)
    {
        case PRV_TGF_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
            break;

        case PRV_TGF_IP_PORT_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E;
            dxChInterfaceCfg.portTrunk.port   = interfaceCfgPtr->portTrunk.port;
            break;

        case PRV_TGF_IP_TRUNK_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
            dxChInterfaceCfg.portTrunk.trunk  = interfaceCfgPtr->portTrunk.trunk;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert the counter Set vlan mode into device specific format */
    switch (interfaceCfgPtr->vlanMode)
    {
        case PRV_TGF_IP_DISREGARD_VLAN_CNT_MODE_E:
            dxChInterfaceCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
            break;

        case PRV_TGF_IP_USE_VLAN_CNT_MODE_E:
            dxChInterfaceCfg.vlanMode = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E;
            dxChInterfaceCfg.vlanId   = interfaceCfgPtr->vlanId;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert other fields */
    dxChInterfaceCfg.ipMode = interfaceCfgPtr->ipMode;
    dxChInterfaceCfg.hwDevNum = interfaceCfgPtr->devNum;
    rc = prvUtfHwDeviceNumberGet(interfaceCfgPtr->devNum,&(dxChInterfaceCfg.hwDevNum));
    if (rc != GT_OK)
    {
        return rc;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpSetMllCntInterface(devNum, mllCntSet, &dxChInterfaceCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpSetMllCntInterface FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpMllCntGet function
* @endinternal
*
* @brief   Get the mll counter.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - the mll counter set out of the 2
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of routed IP Multicast packets Duplicated by the
*                                      MLL Engine and transmitted via this interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllCntGet(devNum, mllCntSet, mllOutMCPktsPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpMllCntSet function
* @endinternal
*
* @brief   set an mll counter.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - the mll counter set out of the 2
* @param[in] mllOutMCPkts             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllCntSet
(
    IN GT_U8    devNum,
    IN GT_U32   mllCntSet,
    IN GT_U32   mllOutMCPkts
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllCntSet(devNum, mllCntSet, mllOutMCPkts);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpMllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the MLL priority queues.
*         A silent drop is a drop that is applied to a replica of the packet that
*         was previously replicated in the TTI.
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllSilentDropCntGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *silentDropPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllSilentDropCntGet(devNum, silentDropPktsPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(silentDropPktsPtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfIpMtuProfileSet function
* @endinternal
*
* @brief   Sets the next hop interface MTU profile limit value.
*
* @param[in] devNum                   - the device number
* @param[in] mtu                      - the mtu profile index.  CHX_FAMILY (0..7)
*                                      EXMXPM_FAMILY (0..15)
* @param[in] mtu                      - the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMtuProfileSet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    IN GT_U32 mtu
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMtuProfileSet(devNum, mtuProfileIndex, mtu);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpMtuProfileGet function
* @endinternal
*
* @brief   Gets the next hop interface MTU profile limit value.
*
* @param[in] devNum                   - device number
* @param[in] mtuProfileIndex          - the mtu profile index.  CHX_FAMILY (0..7)
*                                      EXMXPM_FAMILY (0..15)
*
* @param[out] mtuPtr                   - points to the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mtuProfileIndex,
    OUT GT_U32  *mtuPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMtuProfileGet(devNum, mtuProfileIndex, mtuPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv6AddrPrefixScopeSet function
* @endinternal
*
* @brief   Defines a prefix of a scope type.
*
* @param[in] devNum                   - the device number
* @param[in] prefix                   - an IPv6 address prefix
* @param[in] prefixLen                - length of the prefix (APPLICABLE RANGES: 0..16)
* @param[in] addressScope             - type of the address scope spanned by the prefix
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configures an entry in the prefix look up table
*
*/
GT_STATUS prvTgfIpv6AddrPrefixScopeSet
(
    IN  GT_U8                           devNum,
    IN  GT_IPV6ADDR                     prefix,
    IN  GT_U32                          prefixLen,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope,
    IN  GT_U32                          prefixScopeIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6AddrPrefixScopeSet(devNum, &prefix, prefixLen, addressScope, prefixScopeIndex);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv6AddrPrefixScopeGet function
* @endinternal
*
* @brief   Get a prefix of a scope type.
*
* @param[in] devNum                   - the device number
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @param[out] prefixPtr                - an IPv6 address prefix
* @param[out] prefixLenPtr             - length of the prefix
* @param[out] addressScopePtr          - type of the address scope spanned by the prefix
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfIpv6AddrPrefixScopeGet
(
    IN  GT_U8                           devNum,
    OUT GT_IPV6ADDR                     *prefixPtr,
    OUT GT_U32                          *prefixLenPtr,
    OUT CPSS_IPV6_PREFIX_SCOPE_ENT      *addressScopePtr,
    IN  GT_U32                          prefixScopeIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6AddrPrefixScopeGet(devNum, prefixPtr, prefixLenPtr, addressScopePtr, prefixScopeIndex);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpExceptionCommandSet function
* @endinternal
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - Exception type
* @param[in] protocolStack            - whether to set for ipv4/v6 or both
* @param[in] packetType               - Packet type. Valid values:
*                                      CPSS_IP_UNICAST_E
*                                      CPSS_IP_MULTICAST_E
* @param[in] exceptionCmd             - Exception command
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpExceptionCommandSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType,
    IN PRV_TGF_IP_EXC_CMD_ENT               exceptionCmd
)
{

#ifdef CHX_FAMILY

    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT         exceptionTypeDx;
    CPSS_PACKET_CMD_ENT                     exceptionCmdDx;

    /* convert exceptionType into device specific format */
    switch(packetType)
    {
        case CPSS_IP_UNICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_TTL_EXCEED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E;
                break;
            case PRV_TGF_IP_EXCP_RPF_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_SIP_SA_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
             break;
        }
        case CPSS_IP_MULTICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_MC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
            break;
        }
        default:
            return GT_BAD_PARAM;
    }

    /* convert exceptionCmd into device specific format */
    switch(exceptionCmd)
    {
        case PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E:
            exceptionCmdDx= CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DROP_HARD_E:
            exceptionCmdDx= CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DROP_SOFT_E:
            exceptionCmdDx= CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_E:
            exceptionCmdDx= CPSS_PACKET_CMD_ROUTE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E:
            exceptionCmdDx= CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;
        case PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E:
            exceptionCmdDx= CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case PRV_TGF_IP_EXC_CMD_BRIDGE_E:
            exceptionCmdDx= CPSS_PACKET_CMD_BRIDGE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_NONE_E:
            exceptionCmdDx= CPSS_PACKET_CMD_NONE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_MIRROR_TO_CPU_E:
            exceptionCmdDx= CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case PRV_TGF_IP_EXC_CMD_FORWARD_E:
            exceptionCmdDx= CPSS_PACKET_CMD_FORWARD_E;
            break;
        case PRV_TGF_IP_EXC_CMD_LOOPBACK_E:
            exceptionCmdDx= CPSS_PACKET_CMD_LOOPBACK_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DEFAULT_ROUTE_ENTRY_E:
            exceptionCmdDx= CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpExceptionCommandSet(devNum, exceptionTypeDx, protocolStack,exceptionCmdDx);
#endif /* CHX_FAMILY */


}

/**
* @internal prvTgfIpExceptionCommandGet function
* @endinternal
*
* @brief   Get a specific exception command.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - Exception type
* @param[in] protocolStack            - IP protocol to set for
* @param[in] packetType               - Packet type. Valid values:
*                                      CPSS_IP_UNICAST_E
*                                      CPSS_IP_MULTICAST_E
*
* @param[out] exceptionCmdPtr          - points to exception command
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpExceptionCommandGet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType,
    OUT PRV_TGF_IP_EXC_CMD_ENT              *exceptionCmdPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT         exceptionTypeDx;
    CPSS_PACKET_CMD_ENT                     exceptionCmdDx;
    GT_STATUS                               rc;

    /* convert exceptionType into device specific format */
    switch(packetType)
    {
        case CPSS_IP_UNICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_TTL_EXCEED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E;
                break;
            case PRV_TGF_IP_EXCP_RPF_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_SIP_SA_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
             break;
        }
        case CPSS_IP_MULTICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_MC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
             break;
        }
            default:
                return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpExceptionCommandGet(devNum, exceptionTypeDx, protocolStack, &exceptionCmdDx);
    switch(exceptionCmdDx)
    {
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DROP_HARD_E;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DROP_SOFT_E;
            break;
        case CPSS_PACKET_CMD_ROUTE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_E;
            break;
        case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E;
            break;
        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case CPSS_PACKET_CMD_BRIDGE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_BRIDGE_E;
            break;
        case CPSS_PACKET_CMD_NONE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_NONE_E;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_MIRROR_TO_CPU_E;
            break;
        case CPSS_PACKET_CMD_FORWARD_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_FORWARD_E;
            break;
        case CPSS_PACKET_CMD_LOOPBACK_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_LOOPBACK_E;
            break;
        case CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DEFAULT_ROUTE_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv6McScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Multicast scope commands.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[in] mllSelectionRule         - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpv6McScopeCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN GT_BOOL                          borderCrossed,
    IN CPSS_PACKET_CMD_ENT              scopeCommand,
    IN CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6McScopeCommandSet(devNum, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommand, mllSelectionRule);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpv6McScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Multicast scope commands.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
*
* @param[out] scopeCommandPtr          - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[out] mllSelectionRulePtr      - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfIpv6McScopeCommandGet
(
    IN  GT_U8                            devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN  GT_BOOL                          borderCrossed,
    OUT CPSS_PACKET_CMD_ENT              *scopeCommandPtr,
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT *mllSelectionRulePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6McScopeCommandGet(devNum, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommandPtr, mllSelectionRulePtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterMacSaBaseSet function
* @endinternal
*
* @brief   Sets 40 MSBs of Router MAC SA Base address on specified device.
*
* @param[in] devNum                   - the device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaBaseSet(devNum, macPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterMacSaBaseGet function
* @endinternal
*
* @brief   Gets 40 MSBs of Router MAC SA Base address on specified device.
*
* @param[in] devNum                   - the device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaBaseGet(devNum, macPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterMacSaModifyEnable function
* @endinternal
*
* @brief   Per Egress port bit Enable Routed packets MAC SA Modification
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] enable                   - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaModifyEnable
(
    IN  GT_U8                      devNum,
    IN  GT_U32                      portNum,
    IN  GT_BOOL                    enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaModifyEnable(
        devNum, portNum, enable);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterMacSaModifyEnableGet function
* @endinternal
*
* @brief   Per Egress port bit Get Routed packets MAC SA Modification State
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
*
* @param[out] enablePtr                - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaModifyEnableGet
(
   IN  GT_U8                       devNum,
   IN  GT_PHYSICAL_PORT_NUM        portNum,
   OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaModifyEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpPortRouterMacSaLsbModeSet function
* @endinternal
*
* @brief   Sets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] saLsbMode                - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*       The least significant bytes are set by:
*       Port mode is set by cpssDxChIpRouterPortMacSaLsbSet().
*       Vlan mode is set by cpssDxChIpRouterVlanMacSaLsbSet().
*
*/
GT_STATUS prvTgfIpPortRouterMacSaLsbModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRouterMacSaLsbModeSet(
        devNum, portNum, saLsbMode);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpPortRouterMacSaLsbModeGet function
* @endinternal
*
* @brief   Gets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] saLsbModePtr             - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 40 most significant bytes are get by cpssDxChIpRouterMacSaBaseGet().
*       The least significant bytes are get by:
*       Port mode is get by cpssDxChIpRouterPortMacSaLsbGet().
*       Vlan mode is get by cpssDxChIpRouterVlanMacSaLsbGet().
*
*/
GT_STATUS prvTgfIpPortRouterMacSaLsbModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT    *saLsbModePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRouterMacSaLsbModeGet(
        devNum, portNum, saLsbModePtr);
#endif /* CHX_FAMILY */

}



/**
* @internal prvTgfIpRouterGlobalMacSaSet function
* @endinternal
*
* @brief   Sets full 48-bit Router MAC SA in Global MAC SA table.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.
* @param[in] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterGlobalMacSaSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerMacSaIndex,
    IN  GT_ETHERADDR                *macSaAddrPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterGlobalMacSaSet(
        devNum, routerMacSaIndex & 0xFF, macSaAddrPtr);
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpRouterGlobalMacSaGet function
* @endinternal
*
* @brief   Gets full 48-bit Router MAC SA from Global MAC SA table.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.
*
* @param[out] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterGlobalMacSaIndexSet.
*
*/
GT_STATUS prvTgfIpRouterGlobalMacSaGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       routerMacSaIndex,
    OUT GT_ETHERADDR *macSaAddrPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex, macSaAddrPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterPortGlobalMacSaIndexSet function
* @endinternal
*
* @brief   Set router mac sa index refered to global MAC SA table.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] routerMacSaIndex         - global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterPortGlobalMacSaIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               routerMacSaIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterPortGlobalMacSaIndexSet(
        devNum, portNum, routerMacSaIndex & 0xFF);
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpRouterPortGlobalMacSaIndexGet function
* @endinternal
*
* @brief   Get router mac sa index refered to global MAC SA table.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] routerMacSaIndexPtr      - (pointer to) global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterPortGlobalMacSaIndexGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *routerMacSaIndexPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum, routerMacSaIndexPtr);
#endif /* CHX_FAMILY */

}



/**
* @internal prvTgfIpRouterPortMacSaLsbSet function
* @endinternal
*
* @brief   Sets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - Eggress Port number
* @param[in] saMac                    - The 8 bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to Eggress Port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterPortMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN GT_U8   saMac
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterPortMacSaLsbSet(
        devNum, portNum, saMac);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpRouterVlanMacSaLsbSet function
* @endinternal
*
* @brief   Sets the LSBs of Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - the device number
* @param[in] vlan                     - VLAN Id
* @param[in] saMac                    - The Least Significant bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterVlanMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlan,
    IN GT_U32  saMac
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterVlanMacSaLsbSet(
        devNum, vlan, saMac);
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpRouterNextHopTableAgeBitsEntryRead function
* @endinternal
*
* @brief   read router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @param[out] activityBitPtr           - (pointer to) Age bit value of requested Next-hop entry.
*                                      field. Range 0..1.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvTgfIpRouterNextHopTableAgeBitsEntryRead
(
    GT_U32     *activityBitPtr
)
{
#ifdef CHX_FAMILY
    GT_U32      rc = GT_OK;
    GT_U32      routerNextHopTableAgeBitsEntryIndex;
    GT_U32      routerNextHopTableAgeBitsEntryPtr;
    GT_U8       bitOffset;

    routerNextHopTableAgeBitsEntryIndex = prvTgfRouteEntryBaseIndex / 32;
    bitOffset = (GT_U8)(prvTgfRouteEntryBaseIndex % 32);

    /* call device specific API */
    rc = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(prvTgfDevNum,
                                                      routerNextHopTableAgeBitsEntryIndex,
                                                      &routerNextHopTableAgeBitsEntryPtr);
    if(rc != GT_OK)
        return rc;

    *activityBitPtr = ((routerNextHopTableAgeBitsEntryPtr >> bitOffset) & 0x1);

    return rc;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBCapacityGet function
* @endinternal
*
* @brief   This function gets the current LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[out] partitionEnablePtr       - GT_TRUE:  the TCAM is partitioned according
*                                      to the capacityCfgPtr, any unused TCAM entries
*                                      were allocated to IPv4 UC entries.
*                                      GT_FALSE: TCAM entries are allocated on demand
*                                      while entries are guaranteed as specified
*                                      in capacityCfgPtr.
* @param[out] tcamLpmManagerCapcityCfgPtr - the current capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means current
*                                      prefixes partition, when this was set to
*                                      GT_FALSE this means the current guaranteed
*                                      prefixes allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration.
*/
GT_STATUS prvTgfIpLpmDBCapacityGet
(
    IN  GT_U32                                     lpmDBId,
    OUT GT_BOOL                                    *partitionEnablePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));

    /* call device specific API */
    rc = cpssDxChIpLpmDBCapacityGet(lpmDBId,
                                    partitionEnablePtr,
                                    &dxChIndexesRange,
                                    &dxChTcamLpmManagerCapcityCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBCapacityGet FAILED, rc = [%d]", rc);
    }

    /* convert indexesRangePtr into device specific format */
    indexesRangePtr->firstIndex = dxChIndexesRange.firstIndex;
    indexesRangePtr->lastIndex  = dxChIndexesRange.lastIndex;

    /* convert the counter Set vlan mode into device specific format */
    tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes;

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBCapacityUpdate function
* @endinternal
*
* @brief   This function updates the initial LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[in] tcamLpmManagerCapcityCfgPtr - the new capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means new prefixes
*                                      partition, when this was set to GT_FALSE
*                                      this means the new prefixes guaranteed
*                                      allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration
*
* @note This API is relevant only when using TCAM Manager. This API is used for
*       updating only the capacity configuration of the LPM. for updating the
*       lines reservation for the TCAM Manger use cpssDxChTcamManagerRangeUpdate.
*
*/
GT_STATUS prvTgfIpLpmDBCapacityUpdate
(
    IN GT_U32                                     lpmDBId,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));

    /* convert indexesRangePtr into device specific format */
    dxChIndexesRange.firstIndex = indexesRangePtr->firstIndex;
    dxChIndexesRange.lastIndex  = indexesRangePtr->lastIndex;

    /* convert the counter Set vlan mode into device specific format */
    dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes;

    /* call device specific API */
    rc = cpssDxChIpLpmDBCapacityUpdate(lpmDBId,
                                       &dxChIndexesRange,
                                       &dxChTcamLpmManagerCapcityCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBConfigGet function
* @endinternal
*
* @brief   This function retrieves configuration of the LPM DB.
*
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] shadowTypePtr            - (pointer to) shadow type
* @param[out] protocolStackPtr         - (pointer to) protocol stack this LPM DB support
* @param[out] indexesRangePtr          - (pointer to) range of TCAM indexes availble for this LPM DB
* @param[out] partitionEnablePtr       - GT_TRUE:  TCAM is partitioned to different prefix types
*                                      GT_FALSE: TCAM entries are allocated on demand
* @param[out] tcamLpmManagerCapcityCfgPtr - (pointer to) capacity configuration
* @param[out] isTcamManagerInternalPtr - GT_TRUE:  TCAM manager is used and created
*                                      internally by the LPM DB
*                                      GT_FALSE: otherwise
* @param[out] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PTR               - NULL pointer.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmDBConfigGet
(
    IN  GT_U32                                         lpmDBId,
    OUT PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT               *shadowTypePtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT                    *protocolStackPtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    OUT GT_BOOL                                       *partitionEnablePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr,
    OUT GT_BOOL                                       *isTcamManagerInternalPtr,
    OUT GT_VOID                                       *tcamManagerHandlerPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                    rc = GT_OK;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &dxChIndexesRange;
    lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &dxChTcamLpmManagerCapcityCfg;
    lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = tcamManagerHandlerPtr;
    /* call device specific API */
    rc = cpssDxChIpLpmDBConfigGet(lpmDBId, &dxChShadowType, protocolStackPtr,
                                  &lpmMemoryConfig);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert shadowType from device specific format */
    switch (dxChShadowType)
    {
        case CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_TCAM_ROUTER_BASED_SHADOW_E;
            break;

        case CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E;
            break;

        case CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_RAM_SHADOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert indexesRangePtr from device specific format */
    indexesRangePtr->firstIndex = dxChIndexesRange.firstIndex;
    indexesRangePtr->lastIndex  = dxChIndexesRange.lastIndex;

    *partitionEnablePtr = lpmMemoryConfig.tcamDbCfg.partitionEnable;

    /* convert tcamLpmManagerCapcityCfgPtr from device specific format */
    tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes;

    *isTcamManagerInternalPtr = (tcamManagerHandlerPtr ? GT_FALSE : GT_TRUE);

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmRamDBConfigGet function
* @endinternal
*
* @brief   This function retrieves configuration of the
*           RAM LPM DB
*
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] shadowTypePtr            - (pointer to) shadow type
* @param[out] protocolStackPtr         - (pointer to) protocol stack this LPM DB support
* @param[out] ramDbCfgPtr              - (pointer to)the ram memory configuration for this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PTR               - NULL pointer.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmRamDBConfigGet
(
    IN  GT_U32                                         lpmDBId,
    OUT PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT               *shadowTypePtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT                    *protocolStackPtr,
    OUT PRV_TGF_LPM_RAM_CONFIG_STC                    *ramDbCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                    rc = GT_OK;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    GT_U32                                       i=0;

    /* reset variables */
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    /* call device specific API */
    rc = cpssDxChIpLpmDBConfigGet(lpmDBId, &dxChShadowType, protocolStackPtr,
                                  &lpmMemoryConfig);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert shadowType from device specific format */
    switch (dxChShadowType)
    {
        case CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E:
    case CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E:
            return GT_BAD_VALUE;
        case CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E:
        case CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_RAM_SHADOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    ramDbCfgPtr->numOfBlocks=lpmMemoryConfig.ramDbCfg.numOfBlocks;
    cpssOsMemCpy(ramDbCfgPtr->blocksSizeArray,lpmMemoryConfig.ramDbCfg.blocksSizeArray, sizeof(ramDbCfgPtr->blocksSizeArray));
    ramDbCfgPtr->maxNumOfPbrEntries=lpmMemoryConfig.ramDbCfg.maxNumOfPbrEntries;

    switch (lpmMemoryConfig.ramDbCfg.blocksAllocationMethod)
    {
        case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
            ramDbCfgPtr->blocksAllocationMethod =PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
            break;
        case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
            ramDbCfgPtr->blocksAllocationMethod =PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
            break;
        default:
            return GT_BAD_VALUE;
    }
    switch (lpmMemoryConfig.ramDbCfg.lpmMemMode)
    {
        case CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E:
            ramDbCfgPtr->lpmMemMode =PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
            break;
        case CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:
            ramDbCfgPtr->lpmMemMode =PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
            break;
        default:
            return GT_BAD_VALUE;
    }

    ramDbCfgPtr->lpmRamConfigInfoNumOfElements = lpmMemoryConfig.ramDbCfg.lpmRamConfigInfoNumOfElements;
    for (i=0; i<lpmMemoryConfig.ramDbCfg.lpmRamConfigInfoNumOfElements; i++)
    {
        ramDbCfgPtr->lpmRamConfigInfo[i].devType = lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].devType;

        switch (lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg)
        {
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg  = PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg  = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E;
                break;
            default:
                return GT_BAD_VALUE;
        }
    }

    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBCreate function
* @endinternal
*
* @brief   This function creates an LPM DB for a shared LPM managment.
*
* @param[in] lpmDBId                  - LPM DB id.
* @param[in] shadowType               - type of shadow to maintain
* @param[in] protocolStack            - type of protocol stack this LPM DB support
* @param[in] indexesRangePtr          - (pointer to) range of TCAM indexes
* @param[in] partitionEnable          - GT_TRUE:  partition the TCAM range according to the
*                                      capacityCfgPtr, any unused TCAM entries will
*                                      be allocated to IPv4 UC entries
*                                      GT_FALSE: allocate TCAM entries on demand while
*                                      guarantee entries as specified in capacityCfgPtr
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to) capacity configuration
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmDBCreate
(
    IN GT_U32                                      lpmDBId,
    IN PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN GT_BOOL                                     partitionEnable,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr,
    IN GT_VOID                                    *tcamManagerHandlerPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    /* convert shadowType into device specific format */
    switch (shadowType)
    {
        case PRV_TGF_IP_TCAM_ROUTER_BASED_SHADOW_E:
            dxChShadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;

            break;

        case PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E:
            dxChShadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E;
            break;

        case PRV_TGF_IP_RAM_SHADOW_E:
            dxChShadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert indexesRangePtr into device specific format */
    dxChIndexesRange.firstIndex = indexesRangePtr->firstIndex;
    dxChIndexesRange.lastIndex  = indexesRangePtr->lastIndex;
    lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &dxChIndexesRange;

    /* convert tcamLpmManagerCapcityCfgPtr into device specific format */
    dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes;
    lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &dxChTcamLpmManagerCapcityCfg;

    lpmMemoryConfig.tcamDbCfg.partitionEnable = partitionEnable;
    lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = tcamManagerHandlerPtr;

    /* call device specific API */
    return cpssDxChIpLpmDBCreate(lpmDBId, dxChShadowType, protocolStack,
                                 &lpmMemoryConfig);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmRamDBCreate function
* @endinternal
*
* @brief   This function creates an RAM LPM DB for a shared LPM managment.
*
* @param[in] lpmDBId                  - LPM DB id.
* @param[in] protocolStack            - type of protocol stack this LPM DB support
* @param[in] ramDbCfgPtr              - (pointer to)the ram memory configuration for this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmRamDBCreate
(
    IN GT_U32                                      lpmDBId,
    IN CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    PRV_TGF_LPM_RAM_CONFIG_STC                     *ramDbCfgPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    GT_U32                                       i;

    /* reset variables */
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        dxChShadowType = CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E;
        lpmMemoryConfig.ramDbCfg.maxNumOfPbrEntries = ramDbCfgPtr->maxNumOfPbrEntries;
        lpmMemoryConfig.ramDbCfg.lpmRamConfigInfoNumOfElements=ramDbCfgPtr->lpmRamConfigInfoNumOfElements;

        for(i=0;i<ramDbCfgPtr->lpmRamConfigInfoNumOfElements;i++)
        {
             lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].devType = ramDbCfgPtr->lpmRamConfigInfo[i].devType;

            if ((!CPSS_AC5X_ALL_DEVICES_CHECK_MAC(ramDbCfgPtr->lpmRamConfigInfo[i].devType)) &&
                (!CPSS_AC5P_98DX4504_DEVICES_CHECK_MAC(ramDbCfgPtr->lpmRamConfigInfo[i].devType)) &&
                (!CPSS_QUIRKS_DXCH_HARRIER_PEX_DEVICES_CHECK_MAC(ramDbCfgPtr->lpmRamConfigInfo[i].devType)) &&
                (!CPSS_QUIRKS_DXCH_IRONMAN_PEX_DEVICES_CHECK_MAC(ramDbCfgPtr->lpmRamConfigInfo[i].devType)))
            {
                switch (ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg)
                {
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;
                    break;
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;
                    break;
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
                    break;
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E;
                    break;
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E;
                    break;
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E;
                    break;
                case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E:
                    lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[i].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E;
                    break;
                default:
                    return GT_BAD_PARAM;
                }
            }
        }
    }
    else
    {
        dxChShadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    }
    if (!(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
        lpmMemoryConfig.ramDbCfg.numOfBlocks = ramDbCfgPtr->numOfBlocks;
        for (i=0;i<lpmMemoryConfig.ramDbCfg.numOfBlocks;i++)
        {
            lpmMemoryConfig.ramDbCfg.blocksSizeArray[i] = ramDbCfgPtr->blocksSizeArray[i];
        }
    }

    switch (ramDbCfgPtr->blocksAllocationMethod)
    {
    case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
        break;
    case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch (ramDbCfgPtr->lpmMemMode)
    {
    case PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E:
        lpmMemoryConfig.ramDbCfg.lpmMemMode= CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        break;
    case PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:
        lpmMemoryConfig.ramDbCfg.lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpLpmDBCreate(lpmDBId, dxChShadowType, protocolStack,
                                 &lpmMemoryConfig);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculates the default RAM LPM DB configuration for LPM managment.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvTgfIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                        devNum,
    OUT PRV_TGF_LPM_RAM_CONFIG_STC   *ramDbCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    GT_U32    i = 0;
    CPSS_DXCH_LPM_RAM_CONFIG_STC appDemoRamDbCfg;

    CPSS_NULL_PTR_CHECK_MAC(ramDbCfgPtr);
    cpssOsMemSet(&appDemoRamDbCfg, 0, sizeof(appDemoRamDbCfg));

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvWrAppFalconIpLpmRamDefaultConfigCalc(devNum,
                                                    PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.sharedTableMode,
                                                    PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.maxNumOfPbrEntries,
                                                    &appDemoRamDbCfg);
    }
    else
    {
        rc = prvWrAppBc2IpLpmRamDefaultConfigCalc(devNum,
                                                 PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.maxNumOfPbrEntries,
                                                 &appDemoRamDbCfg);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        ramDbCfgPtr->maxNumOfPbrEntries = appDemoRamDbCfg.maxNumOfPbrEntries;
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=appDemoRamDbCfg.lpmRamConfigInfoNumOfElements;

        for(i=0;i<appDemoRamDbCfg.lpmRamConfigInfoNumOfElements;i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].devType = appDemoRamDbCfg.lpmRamConfigInfo[i].devType;

            if ((!CPSS_AC5X_ALL_DEVICES_CHECK_MAC(appDemoRamDbCfg.lpmRamConfigInfo[i].devType)) &&
                (!CPSS_AC5P_98DX4504_DEVICES_CHECK_MAC(appDemoRamDbCfg.lpmRamConfigInfo[i].devType)) &&
                (!CPSS_QUIRKS_DXCH_HARRIER_PEX_DEVICES_CHECK_MAC(appDemoRamDbCfg.lpmRamConfigInfo[i].devType)) &&
                (!CPSS_QUIRKS_DXCH_IRONMAN_PEX_DEVICES_CHECK_MAC(appDemoRamDbCfg.lpmRamConfigInfo[i].devType)))
            {
                switch (appDemoRamDbCfg.lpmRamConfigInfo[i].sharedMemCnfg)
                {
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E;
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E;
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E;
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E:
                    ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E;
                    break;
                default:
                    return GT_BAD_PARAM;
                }
             }
        }
    }
    else
    {
        ramDbCfgPtr->numOfBlocks = appDemoRamDbCfg.numOfBlocks;
        for (i=0;i<ramDbCfgPtr->numOfBlocks;i++)
        {
            ramDbCfgPtr->blocksSizeArray[i] = appDemoRamDbCfg.blocksSizeArray[i];
        }
    }
    switch (appDemoRamDbCfg.blocksAllocationMethod)
    {
    case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
        ramDbCfgPtr->blocksAllocationMethod = PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
        break;
    case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
        ramDbCfgPtr->blocksAllocationMethod = PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch (appDemoRamDbCfg.lpmMemMode)
    {
    case CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E:
        ramDbCfgPtr->lpmMemMode = PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        break;
    case CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:
        ramDbCfgPtr->lpmMemMode = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpLpmRamDBCreateDefault function
* @endinternal
*
* @brief   This function creates a default RAM LPM DB for a shared LPM managment.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmRamDBCreateDefault
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    cpssOsMemSet(&ramDbCfg, 0, sizeof(ramDbCfg));

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvTgfIpLpmRamDBCreate(0,CPSS_IP_PROTOCOL_IPV4V6_E,&ramDbCfg);
}

/**
* @internal prvTgfIpLpmDBDelete function
* @endinternal
*
* @brief   This function deletes LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvTgfIpLpmDBDelete
(
    IN GT_U32           lpmDBId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmDBDelete(lpmDBId);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDBDeleteDefault function
* @endinternal
*
* @brief   This function deletes default LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvTgfIpLpmDBDeleteDefault
(
    GT_VOID
)
{
    return prvTgfIpLpmDBDelete(0);
}

/**
* @internal prvTgfIpLpmDBExtTcamManagerAdd function
* @endinternal
*
* @brief   This function add external TCAM manager into default LPM DB
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] tcamManagerRangePtr      - (pointer to) allocated TCAM range
*
* @param[out] tcamManagerHandlerPtrPtr - (pointer to) pointer of the created TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmDBExtTcamManagerAdd
(
    IN  GT_U32                                lpmDBId,
    IN  PRV_TGF_TCAM_MANAGER_RANGE_STC       *tcamManagerRangePtr,
    OUT GT_VOID                             **tcamManagerHandlerPtrPtr
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TCAM_MANAGER_TCAM_TYPE_ENT  tcamType;
    GT_PTR                              defTcamManagerHandlerPtr;


    /* retrieves configuration of the LPM DB */
    rc = prvTgfIpLpmDBConfigGet(lpmDBId,
                                &(prvTgfDefLpmDBVrCfg.shadowType),
                                &(prvTgfDefLpmDBVrCfg.protocolStack),
                                &(prvTgfDefLpmDBVrCfg.indexesRange),
                                &(prvTgfDefLpmDBVrCfg.partitionEnable),
                                &(prvTgfDefLpmDBVrCfg.tcamLpmManagerCapcityCfg),
                                &(prvTgfDefLpmDBVrCfg.isTcamManagerInternal),
                                &defTcamManagerHandlerPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(lpmDBId, prvTgfDefVrId,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete Virtual Router */
    rc = prvTgfIpLpmVirtualRouterDel(lpmDBId, prvTgfDefVrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterDel FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete default LPM DB */
    rc = prvTgfIpLpmDBDelete(lpmDBId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBDelete FAILED, rc = [%d]", rc);

        return rc;
    }

    /* init check for memory leaks */
    osMemStartHeapAllocationCounter();

    /* set TCAM type */
    tcamType = (PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E == prvTgfDefLpmDBVrCfg.shadowType) ?
                PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E :
                PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;

    /* create TCAM maneger */
    rc = prvTgfTcamManagerCreate(tcamType, tcamManagerRangePtr, tcamManagerHandlerPtrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerCreate FAILED, rc = [%d]", rc);

        return rc;
    }

    /* create new LPM DB with external TCAM manager */
    rc = prvTgfIpLpmDBCreate(lpmDBId,
                             prvTgfDefLpmDBVrCfg.shadowType,
                             prvTgfDefLpmDBVrCfg.protocolStack,
                             &(prvTgfDefLpmDBVrCfg.indexesRange),
                             prvTgfDefLpmDBVrCfg.partitionEnable,
                             &(prvTgfDefLpmDBVrCfg.tcamLpmManagerCapcityCfg),
                             *tcamManagerHandlerPtrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCreate FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add device to LPM DB */
    rc = prvTgfIpLpmDBDevListAdd(lpmDBId, &prvTgfDevNum, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBDevListAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add Virtual Router */
    rc = prvTgfIpLpmVirtualRouterAdd(lpmDBId, prvTgfDefVrId,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportFcoe) ?
                                      &(prvTgfDefLpmDBVrCfg.defFcoeNextHopInfo) : NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
}

/**
* @internal prvTgfIpLpmDBExtTcamManagerRemove function
* @endinternal
*
* @brief   This function remove external TCAM manager from default LPM DB
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmDBExtTcamManagerRemove
(
    IN  GT_U32                                lpmDBId,
    IN  GT_VOID                              *tcamManagerHandlerPtr
)
{
    GT_STATUS       rc, rc1 = GT_OK;
    GT_U32          usedMem = 0;


    /* delete Virtual Router */
    rc = prvTgfIpLpmVirtualRouterDel(lpmDBId, prvTgfDefVrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterDel FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete LPM DB */
    rc = prvTgfIpLpmDBDelete(lpmDBId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBDelete FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete TCAM Manager handler */
    rc = prvTgfTcamManagerDelete(tcamManagerHandlerPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerDelete FAILED, rc = [%d]", rc);

        return rc;
    }

    /* check for memory leaks */
    usedMem = osMemGetHeapAllocationCounter();

    rc1 = (0 == usedMem) ? GT_OK : GT_FAIL;
    if (GT_OK != rc1)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Memory leak detected: %d bytes \n", usedMem);
    }

    /* restore default LPM DB */
    rc = prvTgfIpLpmDBCreate(lpmDBId,
                             prvTgfDefLpmDBVrCfg.shadowType,
                             prvTgfDefLpmDBVrCfg.protocolStack,
                             &(prvTgfDefLpmDBVrCfg.indexesRange),
                             prvTgfDefLpmDBVrCfg.partitionEnable,
                             &(prvTgfDefLpmDBVrCfg.tcamLpmManagerCapcityCfg),
                             NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCreate FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add device to LPM DB */
    rc = prvTgfIpLpmDBDevListAdd(lpmDBId, &prvTgfDevNum, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add Virtual Router */
    rc = prvTgfIpLpmVirtualRouterAdd(lpmDBId, prvTgfDefVrId,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportFcoe) ?
                                      &(prvTgfDefLpmDBVrCfg.defFcoeNextHopInfo) : NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    /* init check for memory leaks */
    osMemStartHeapAllocationCounter();

    return (GT_OK == rc) ? rc1 : rc;
}

/**
* @internal prvTgfIpLpmDBWithTcamCapacityUpdate function
* @endinternal
*
* @brief   This function updates LPM DB allocation and reserved TCAM lines.
*
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
* @param[in] lpmDBId                  - LPM DB id
* @param[in] indexesRangePtr          - (pointer to) range of TCAM indexes
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to) new capacity configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources
*/
GT_STATUS prvTgfIpLpmDBWithTcamCapacityUpdate
(
    IN GT_VOID                                    *tcamManagerHandlerPtr,
    IN GT_U32                                      lpmDBId,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_BOOL                                     partitionEnable = GT_FALSE;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmMngrCapcityCfg;
    PRV_TGF_TCAM_MANAGER_RANGE_STC              tcamMngrRange;


    /* clear entries */
    cpssOsMemSet(&indexesRange,          0, sizeof(indexesRange));
    cpssOsMemSet(&tcamLpmMngrCapcityCfg, 0, sizeof(tcamLpmMngrCapcityCfg));
    cpssOsMemSet(&tcamMngrRange, 0, sizeof(tcamMngrRange));

    if (indexesRangePtr->firstIndex > indexesRangePtr->lastIndex)
        return GT_BAD_PARAM;

    /* get LpmDB capacity */
    rc = prvTgfIpLpmDBCapacityGet(lpmDBId,
                                  &partitionEnable,
                                  &indexesRange,
                                  &tcamLpmMngrCapcityCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCapacityGet FAILED, rc = [%d]", rc);

        return rc;
    }

    if ((indexesRange.lastIndex - indexesRange.firstIndex) >
        (indexesRangePtr->lastIndex - indexesRangePtr->firstIndex)) /* shrink */
    {
        /* update LpmDB capacity */
        rc = prvTgfIpLpmDBCapacityUpdate(lpmDBId,
                                         indexesRangePtr,
                                         tcamLpmManagerCapcityCfgPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);

            return rc;
        }

        /* update TCAM ranges */
        tcamMngrRange.firstLine = indexesRangePtr->firstIndex;
        tcamMngrRange.lastLine  = indexesRangePtr->lastIndex;

        rc = prvTgfTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                          &tcamMngrRange,
                                          PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerRangeUpdate FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    else /* enlarge */
    {
        /* update TCAM ranges */
        tcamMngrRange.firstLine = indexesRangePtr->firstIndex;
        tcamMngrRange.lastLine  = indexesRangePtr->lastIndex;

        rc = prvTgfTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                          &tcamMngrRange,
                                          PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerRangeUpdate FAILED, rc = [%d]", rc);

            return rc;
        }

        /* update LpmDB capacity */
        rc = prvTgfIpLpmDBCapacityUpdate(lpmDBId,
                                         indexesRangePtr,
                                         tcamLpmManagerCapcityCfgPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    return rc;
}


/**
* @internal prvTgfIpValidityCheck function
* @endinternal
*
* @brief   Check Patricia trie validity for specific LPM DB Id and virtual router Id
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
*                                      entryType   - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
*/
GT_STATUS prvTgfIpValidityCheck
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType

)
{
    if (GT_FALSE == prvTgfIpValidityCheckFlag)
    {
        /* do not perform IP validation */
        return GT_OK;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)) || (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
        return prvTgfIpLpmValidityCheck(lpmDBId, vrId, protocol, prefixType, GT_TRUE);
    }
    else
    {
        return cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, protocol, prefixType);
    }
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpValidityCheckEnable function
* @endinternal
*
* @brief   This function enable/disable Patricia trie validity check.
*
* @param[in] validityFlag             - flag whether to check (GT_TRUE) Patricia trie validity
*                                       None.
*/
GT_VOID prvTgfIpValidityCheckEnable
(
    IN GT_BOOL validityFlag
)
{
    prvTgfIpValidityCheckFlag = validityFlag;
}

/**
* @internal prvTgfIpLpmValidityCheck function
* @endinternal
*
* @brief   Check IP LPM Shadow validity for specific LPM DB Id and virtual router Id
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
*/
GT_STATUS prvTgfIpLpmValidityCheck
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_BOOL                    returnOnFailure

)
{
    GT_STATUS rc = GT_OK;
    if (GT_FALSE == prvTgfIpValidityCheckFlag)
    {
        /* do not perform IP validation */
        return GT_OK;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChIpLpmDbgShadowValidityCheck(lpmDBId, vrId, protocol, prefixType, returnOnFailure);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgShadowValidityCheck FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChIpLpmDbgHwValidation(prvTgfDevNum, vrId, protocol, prefixType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwValidation FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(prvTgfDevNum, lpmDBId, vrId, protocol, prefixType,returnOnFailure);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwShadowSyncValidityCheck FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom function
* @endinternal
*
* @brief   This function tries to add many random IPv4 Unicast prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to) the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] isWholeIpRandom          - GT_TRUE: all IP octets calculated by cpssOsRand
*                                      GT_FALSE: only 2 LSB octets calculated by cpssOsRand
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    IN  GT_BOOL     defragmentationEnable,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      numOfPrefixesAdded=0;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom(lpmDbId,
                                                    vrId,
                                                    startIpAddr,
                                                    routeEntryBaseMemAddr,
                                                    numOfPrefixesToAdd,
                                                    isWholeIpRandom,
                                                    defragmentationEnable,
                                                    &numOfPrefixesAdded);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        rc = GT_OK;
        *numOfPrefixesAddedPtr= numOfPrefixesAdded;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(isWholeIpRandom);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmDbgPrefixLengthSet function
* @endinternal
*
* @brief   set prefix length used in addManyByOctet APIs
*
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] prefixLength             - prefix length
* @param[in] srcPrefixLength          - src prefix length
*/
GT_U32 prvTgfIpLpmDbgPrefixLengthSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_U32                     prefixLength,
    IN GT_U32                     srcPrefixLength
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     switch (protocolStack)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
    case CPSS_IP_PROTOCOL_FCOE_E:
        if (prefixType==CPSS_UNICAST_E)
        {
            IPv4PrefixLength = prefixLength;
        }
        else
        {
            IPv4PrefixLength = prefixLength;
            srcIPv4PrefixLength = srcPrefixLength;
        }

        break;
    case CPSS_IP_PROTOCOL_IPV6_E:
        if (prefixType==CPSS_UNICAST_E)
        {
            IPv6PrefixLength = prefixLength;
        }
        else
        {
            IPv6PrefixLength = prefixLength;
            srcIPv6PrefixLength = srcPrefixLength;
        }
        break;
    default:
        rc = GT_BAD_PARAM;
        break;
    }
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvCpssDxChIpLpmDbgPrefixLengthSet FAILED, rc = [%d]", rc);

        return rc;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixType);
    TGF_PARAM_NOT_USED(prefixLength);
    TGF_PARAM_NOT_USED(srcPrefixLength);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to) the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet(lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                                                     numOfPrefixesToAdd, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,lastIpAddrAddedPtr,
                                                     numOfPrefixesAddedPtr);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixAddManyByOctet FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixAddManyByOctetBulk function
* @endinternal
*
* @brief  This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*         the add is done using bulk API
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] sizeOfBulk               - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
* @param[out] numOfPrefixesAlreadyExistPtr - points to the nubmer of prefixes that were
*                                      not added since they are already defined (NULL to ignore)
* @param[out] numOfPrefixesNotAddedDueToOutOfPpMemPtr - points to the nubmer of prefixes that were
*                                      not added due to out of PP memory (NULL to ignore)
* @param[out] bulkTimePtr              - points to the time it takes for bulk operation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAddManyByOctetBulk
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      sizeOfBulk,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr,
    OUT GT_U32      *numOfPrefixesAlreadyExistPtr,
    OUT GT_U32      *numOfPrefixesNotAddedDueToOutOfPpMemPtr,
    OUT GT_U32      *bulkTimePtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctetBulk(lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                                                     sizeOfBulk, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,lastIpAddrAddedPtr,
                                                     numOfPrefixesAddedPtr,numOfPrefixesAlreadyExistPtr,
                                                     numOfPrefixesNotAddedDueToOutOfPpMemPtr,bulkTimePtr);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctetBulk FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(sizeOfBulk);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAlreadyExistPtr);
    TGF_PARAM_NOT_USED(umOfPrefixesNotAddedDueToOutOfPpMemPtr);
    TGF_PARAM_NOT_USED(bulkTimePtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to) the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet(lpmDbId, vrId, startIpAddr, numOfPrefixesToDel, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,lastIpAddrDeletedPtr,numOfPrefixesDeletedPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet FAILED, rc = [%d]", rc);
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixDelManyByOctetBulk function
* @endinternal
*
* @brief  This function tries to delete many sequential IPv4 Unicast prefixes using bulk according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] sizeOfBulk               - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
* @param[out] numOfPrefixesAlreadyDeletedPtr  - points to the nubmer of prefixes that were
*                                      already deleted (NULL to ignore)
* @param[out] bulkTimePtr              - points to the time it takes for bulk operation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctetBulk.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDelManyByOctetBulk
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      sizeOfBulk,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr,
    OUT GT_U32      *numOfPrefixesAlreadyDeletedPtr,
    OUT GT_U32      *bulkTimePtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctetBulk(lpmDbId, vrId, startIpAddr, sizeOfBulk, firstOctetToChange,secondOctetToChange,
                                                         thirdOctetToChange,forthOctetToChange,lastIpAddrDeletedPtr,
                                                         numOfPrefixesDeletedPtr,numOfPrefixesAlreadyDeletedPtr,bulkTimePtr);
    if ((GT_OK != rc)&&(rc != GT_NOT_FOUND))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctetBulk FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAlreadyDeletedPtr);
    TGF_PARAM_NOT_USED(bulkTimePtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv4 Unicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to) the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *startIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet(lpmDbId, vrId, startIpAddr,
                                                     numOfPrefixesToGet, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,numOfRetrievedPrefixesPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet FAILED, rc = [%d]", rc);
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(numOfRetrievedPrefixesPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4McPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr        - (pointer to)the first group address to add
* @param[in] srcStartIpAddr        - (pointer to)the first source address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrAddedPtr    - points to the last group prefix successfully
*                                      added (NULL to ignore)
* @param[out] srcLastIpAddrAddedPtr    - points to the last source prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv4McPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *grpStartIpAddr,
    IN  GT_IPADDR   *srcStartIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_IPADDR   *grpLastIpAddrAddedPtr,
    OUT GT_IPADDR   *srcLastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet(lpmDbId,vrId,grpStartIpAddr,srcStartIpAddr,routeEntryBaseMemAddr,numOfPrefixesToAdd,
                                                     changeGrpAddr,grpFirstOctetToChange,grpSecondOctetToChange,grpThirdOctetToChange,grpFourthOctetToChange,
                                                     changeSrcAddr,srcFirstOctetToChange,srcSecondOctetToChange,srcThirdOctetToChange,srcFourthOctetToChange,
                                                     grpLastIpAddrAddedPtr,srcLastIpAddrAddedPtr,numOfPrefixesAddedPtr);

    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixAddManyByOctet FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(grpStartIpAddr);
    TGF_PARAM_NOT_USED(srcStartIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(changeGrpAddr);
    TGF_PARAM_NOT_USED(grpFirstOctetToChange);
    TGF_PARAM_NOT_USED(grpSecondOctetToChange);
    TGF_PARAM_NOT_USED(grpThirdOctetToChange);
    TGF_PARAM_NOT_USED(grpFourthOctetToChange);
    TGF_PARAM_NOT_USED(changeSrcAddr);
    TGF_PARAM_NOT_USED(srcFirstOctetToChange);
    TGF_PARAM_NOT_USED(srcSecondOctetToChange);
    TGF_PARAM_NOT_USED(srcThirdOctetToChange);
    TGF_PARAM_NOT_USED(srcFourthOctetToChange);
    TGF_PARAM_NOT_USED(grpLastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(srcLastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*

* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr    - (pointer to)the first group address to deleted
* @param[in] srcStartIpAddr    - (pointer to)the first source address to deleted
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrDeletedPtr  - points to the last group prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] srcLastIpAddrDeletedPtr  - points to the last source prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be deleted. All the prefixes are deleted with exact match (prefix
*       length 32).
*
*/
GT_STATUS prvTgfIpLpmIpv4McPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *grpStartIpAddr,
    IN  GT_IPADDR   *srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_IPADDR   *grpLastIpAddrDeletedPtr,
    OUT GT_IPADDR   *srcLastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet(lpmDbId, vrId, grpStartIpAddr,srcStartIpAddr, numOfPrefixesToDel,
                                                     changeGrpAddr,grpFirstOctetToChange,grpSecondOctetToChange,grpThirdOctetToChange,
                                                     grpFourthOctetToChange,changeSrcAddr,srcFirstOctetToChange,srcSecondOctetToChange,
                                                     srcThirdOctetToChange,srcFourthOctetToChange,grpLastIpAddrDeletedPtr,srcLastIpAddrDeletedPtr,
                                                     numOfPrefixesDeletedPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet FAILED, rc = [%d]", rc);
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(grpStartIpAddr);
    TGF_PARAM_NOT_USED(srcStartIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(changeGrpAddr);
    TGF_PARAM_NOT_USED(grpFirstOctetToChange);
    TGF_PARAM_NOT_USED(grpSecondOctetToChange);
    TGF_PARAM_NOT_USED(grpThirdOctetToChange);
    TGF_PARAM_NOT_USED(grpFourthOctetToChange);
    TGF_PARAM_NOT_USED(changeSrcAddr);
    TGF_PARAM_NOT_USED(srcFirstOctetToChange);
    TGF_PARAM_NOT_USED(srcSecondOctetToChange);
    TGF_PARAM_NOT_USED(srcThirdOctetToChange);
    TGF_PARAM_NOT_USED(srcFourthOctetToChange);
    TGF_PARAM_NOT_USED(grpLastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(srcLastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to) the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet(lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                                                      numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                                                      thirdOctetToChange, forthOctetToChange, fifthOctetToChange,
                                                      sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                                                      ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                                                      twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                                      fifteenOctetToChange, sixteenOctetToChange, lastIpAddrAddedPtr,
                                                      numOfPrefixesAddedPtr);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(fifthOctetToChange);
    TGF_PARAM_NOT_USED(sixthOctetToChange);
    TGF_PARAM_NOT_USED(seventhOctetToChange);
    TGF_PARAM_NOT_USED(eighthOctetToChange);
    TGF_PARAM_NOT_USED(ninthOctetToChange);
    TGF_PARAM_NOT_USED(tenthOctetToChange);
    TGF_PARAM_NOT_USED(elevenOctetToChange);
    TGF_PARAM_NOT_USED(twelveOctetToChange);
    TGF_PARAM_NOT_USED(thirteenOctetToChange);
    TGF_PARAM_NOT_USED(fourteenOctetToChange);
    TGF_PARAM_NOT_USED(fifteenOctetToChange);
    TGF_PARAM_NOT_USED(sixteenOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk function
* @endinternal
*
* @brief  This function tries to add many sequential IPv6 Unicast prefixes with bulk according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to)the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] sizeOfBulk               - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
* @param[out] numOfPrefixesAlreadyExistPtr - points to the nubmer of prefixes that were
*                                      not added since they are already defined (NULL to ignore)
* @param[out] numOfPrefixesNotAddedDueToOutOfPpMemPtr - points to the nubmer of prefixes that were
*                                      not added due to out of PP memory (NULL to ignore)
* @param[out] bulkTimePtr              - points to the time it takes for bulk operation*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr,
    OUT GT_U32      *numOfPrefixesAlreadyExistPtr,
    OUT GT_U32      *numOfPrefixesNotAddedDueToOutOfPpMemPtr,
    OUT GT_U32      *bulkTimePtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctetBulk(lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                                                      numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                                                      thirdOctetToChange, forthOctetToChange, fifthOctetToChange,
                                                      sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                                                      ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                                                      twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                                      fifteenOctetToChange, sixteenOctetToChange, lastIpAddrAddedPtr,
                                                      numOfPrefixesAddedPtr,numOfPrefixesAlreadyExistPtr,numOfPrefixesNotAddedDueToOutOfPpMemPtr,bulkTimePtr);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctetBulk FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(fifthOctetToChange);
    TGF_PARAM_NOT_USED(sixthOctetToChange);
    TGF_PARAM_NOT_USED(seventhOctetToChange);
    TGF_PARAM_NOT_USED(eighthOctetToChange);
    TGF_PARAM_NOT_USED(ninthOctetToChange);
    TGF_PARAM_NOT_USED(tenthOctetToChange);
    TGF_PARAM_NOT_USED(elevenOctetToChange);
    TGF_PARAM_NOT_USED(twelveOctetToChange);
    TGF_PARAM_NOT_USED(thirteenOctetToChange);
    TGF_PARAM_NOT_USED(fourteenOctetToChange);
    TGF_PARAM_NOT_USED(fifteenOctetToChange);
    TGF_PARAM_NOT_USED(sixteenOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAlreadyExistPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesNotAddedDueToOutOfPpMemPtr);
    TGF_PARAM_NOT_USED(bulkTimePtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - (pointer to) the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to deleted
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of prefixes
*       that was added by cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet(lpmDbId, vrId, startIpAddr, numOfPrefixesToDel, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange, forthOctetToChange, fifthOctetToChange, sixthOctetToChange,
                                                     seventhOctetToChange, eighthOctetToChange, ninthOctetToChange, tenthOctetToChange,
                                                     elevenOctetToChange, twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                                     fifteenOctetToChange, sixteenOctetToChange,lastIpAddrDeletedPtr,numOfPrefixesDeletedPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet FAILED, rc = [%d]", rc);
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(fifthOctetToChange);
    TGF_PARAM_NOT_USED(sixthOctetToChange);
    TGF_PARAM_NOT_USED(seventhOctetToChange);
    TGF_PARAM_NOT_USED(eighthOctetToChange);
    TGF_PARAM_NOT_USED(ninthOctetToChange);
    TGF_PARAM_NOT_USED(tenthOctetToChange);
    TGF_PARAM_NOT_USED(elevenOctetToChange);
    TGF_PARAM_NOT_USED(twelveOctetToChange);
    TGF_PARAM_NOT_USED(thirteenOctetToChange);
    TGF_PARAM_NOT_USED(fourteenOctetToChange);
    TGF_PARAM_NOT_USED(fifteenOctetToChange);
    TGF_PARAM_NOT_USED(sixteenOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6McPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr        - (pointer to)the first group address to add
* @param[in] srcStartIpAddr        - (pointer to)the first source address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrAddedPtr    - points to the last group prefix successfully
*                                      added (NULL to ignore)
* @param[out] srcLastIpAddrAddedPtr    - points to the last source prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv6McPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *grpStartIpAddr,
    IN  GT_IPV6ADDR *srcStartIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_IPV6ADDR *grpLastIpAddrAddedPtr,
    OUT GT_IPV6ADDR *srcLastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS       rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet(lpmDbId,vrId,grpStartIpAddr,srcStartIpAddr,routeEntryBaseMemAddr,numOfPrefixesToAdd,
                                                     changeGrpAddr,grpFirstOctetToChange,grpSecondOctetToChange,grpThirdOctetToChange,
                                                     grpFourthOctetToChange,grpFifthOctetToChange,grpSixthOctetToChange,grpSeventhOctetToChange,
                                                     grpEighthOctetToChange,grpNinthOctetToChange,grpTenthOctetToChange,grpElevenOctetToChange,
                                                     grpTwelveOctetToChange,grpThirteenOctetToChange,grpFourteenOctetToChange,grpFifteenOctetToChange,
                                                     grpSixteenOctetToChange,changeSrcAddr,srcFirstOctetToChange,srcSecondOctetToChange,srcThirdOctetToChange,
                                                     srcFourthOctetToChange,srcFifthOctetToChange,srcSixthOctetToChange,srcSeventhOctetToChange,srcEighthOctetToChange,
                                                     srcNinthOctetToChange,srcTenthOctetToChange,srcElevenOctetToChange,srcTwelveOctetToChange,srcThirteenOctetToChange,
                                                     srcFourteenOctetToChange,srcFifteenOctetToChange,srcSixteenOctetToChange,
                                                     grpLastIpAddrAddedPtr,srcLastIpAddrAddedPtr,numOfPrefixesAddedPtr);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet FAILED, rc = [%d]", rc);
    }
    else
    {
        rc = GT_OK;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(grpStartIpAddr);
    TGF_PARAM_NOT_USED(srcStartIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(changeGrpAddr);
    TGF_PARAM_NOT_USED(grpFirstOctetToChange);
    TGF_PARAM_NOT_USED(grpSecondOctetToChange);
    TGF_PARAM_NOT_USED(grpThirdOctetToChange);
    TGF_PARAM_NOT_USED(grpFourthOctetToChange);
    TGF_PARAM_NOT_USED(grpFifthOctetToChange);
    TGF_PARAM_NOT_USED(grpSixthOctetToChange);
    TGF_PARAM_NOT_USED(grpSeventhOctetToChange);
    TGF_PARAM_NOT_USED(grpEighthOctetToChange);
    TGF_PARAM_NOT_USED(grpNinthOctetToChange);
    TGF_PARAM_NOT_USED(grpTenthOctetToChange);
    TGF_PARAM_NOT_USED(grpElevenOctetToChange);
    TGF_PARAM_NOT_USED(grpTwelveOctetToChange);
    TGF_PARAM_NOT_USED(grpThirteenOctetToChange);
    TGF_PARAM_NOT_USED(grpFourteenOctetToChange);
    TGF_PARAM_NOT_USED(grpFifteenOctetToChange);
    TGF_PARAM_NOT_USED(grpSixteenOctetToChange);
    TGF_PARAM_NOT_USED(changeSrcAddr);
    TGF_PARAM_NOT_USED(srcFirstOctetToChange);
    TGF_PARAM_NOT_USED(srcSecondOctetToChange);
    TGF_PARAM_NOT_USED(srcThirdOctetToChange);
    TGF_PARAM_NOT_USED(srcFourthOctetToChange);
    TGF_PARAM_NOT_USED(srcFifthOctetToChange);
    TGF_PARAM_NOT_USED(srcSixthOctetToChange);
    TGF_PARAM_NOT_USED(srcSeventhOctetToChange);
    TGF_PARAM_NOT_USED(srcEighthOctetToChange);
    TGF_PARAM_NOT_USED(srcNinthOctetToChange);
    TGF_PARAM_NOT_USED(srcTenthOctetToChange);
    TGF_PARAM_NOT_USED(srcElevenOctetToChange);
    TGF_PARAM_NOT_USED(srcTwelveOctetToChange);
    TGF_PARAM_NOT_USED(srcThirteenOctetToChange);
    TGF_PARAM_NOT_USED(srcFourteenOctetToChange);
    TGF_PARAM_NOT_USED(srcFifteenOctetToChange);
    TGF_PARAM_NOT_USED(srcSixteenOctetToChange);
    TGF_PARAM_NOT_USED(grpLastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(srcLastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6McPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr     - (pointer to)the first group address to delete
* @param[in] srcStartIpAddr     - (pointer to)the first source address to delete
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrDeletedPtr  - points to the last group prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] srcLastIpAddrDeletedPtr  - points to the last source prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be deleted. All the prefixes are deleted with exact match (prefix
*       length 128).
*
*/
GT_STATUS prvTgfIpLpmIpv6McPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *grpStartIpAddr,
    IN  GT_IPV6ADDR *srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_IPV6ADDR *grpLastIpAddrDeletedPtr,
    OUT GT_IPV6ADDR *srcLastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet(lpmDbId,vrId,grpStartIpAddr,srcStartIpAddr,numOfPrefixesToDel,
                                                     changeGrpAddr,grpFirstOctetToChange,grpSecondOctetToChange,grpThirdOctetToChange,
                                                     grpFourthOctetToChange,grpFifthOctetToChange,grpSixthOctetToChange,grpSeventhOctetToChange,
                                                     grpEighthOctetToChange,grpNinthOctetToChange,grpTenthOctetToChange,grpElevenOctetToChange,
                                                     grpTwelveOctetToChange,grpThirteenOctetToChange,grpFourteenOctetToChange,grpFifteenOctetToChange,
                                                     grpSixteenOctetToChange,changeSrcAddr,srcFirstOctetToChange,srcSecondOctetToChange,srcThirdOctetToChange,
                                                     srcFourthOctetToChange,srcFifthOctetToChange,srcSixthOctetToChange,srcSeventhOctetToChange,srcEighthOctetToChange,
                                                     srcNinthOctetToChange,srcTenthOctetToChange,srcElevenOctetToChange,srcTwelveOctetToChange,srcThirteenOctetToChange,
                                                     srcFourteenOctetToChange,srcFifteenOctetToChange,srcSixteenOctetToChange,
                                                     grpLastIpAddrDeletedPtr,srcLastIpAddrDeletedPtr,numOfPrefixesDeletedPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet FAILED, rc = [%d]", rc);
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(grpStartIpAddr);
    TGF_PARAM_NOT_USED(srcStartIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(changeGrpAddr);
    TGF_PARAM_NOT_USED(grpFirstOctetToChange);
    TGF_PARAM_NOT_USED(grpSecondOctetToChange);
    TGF_PARAM_NOT_USED(grpThirdOctetToChange);
    TGF_PARAM_NOT_USED(grpFourthOctetToChange);
    TGF_PARAM_NOT_USED(grpFifthOctetToChange);
    TGF_PARAM_NOT_USED(grpSixthOctetToChange);
    TGF_PARAM_NOT_USED(grpSeventhOctetToChange);
    TGF_PARAM_NOT_USED(grpEighthOctetToChange);
    TGF_PARAM_NOT_USED(grpNinthOctetToChange);
    TGF_PARAM_NOT_USED(grpTenthOctetToChange);
    TGF_PARAM_NOT_USED(grpElevenOctetToChange);
    TGF_PARAM_NOT_USED(grpTwelveOctetToChange);
    TGF_PARAM_NOT_USED(grpThirteenOctetToChange);
    TGF_PARAM_NOT_USED(grpFourteenOctetToChange);
    TGF_PARAM_NOT_USED(grpFifteenOctetToChange);
    TGF_PARAM_NOT_USED(grpSixteenOctetToChange);
    TGF_PARAM_NOT_USED(changeSrcAddr);
    TGF_PARAM_NOT_USED(srcFirstOctetToChange);
    TGF_PARAM_NOT_USED(srcSecondOctetToChange);
    TGF_PARAM_NOT_USED(srcThirdOctetToChange);
    TGF_PARAM_NOT_USED(srcFourthOctetToChange);
    TGF_PARAM_NOT_USED(srcFifthOctetToChange);
    TGF_PARAM_NOT_USED(srcSixthOctetToChange);
    TGF_PARAM_NOT_USED(srcSeventhOctetToChange);
    TGF_PARAM_NOT_USED(srcEighthOctetToChange);
    TGF_PARAM_NOT_USED(srcNinthOctetToChange);
    TGF_PARAM_NOT_USED(srcTenthOctetToChange);
    TGF_PARAM_NOT_USED(srcElevenOctetToChange);
    TGF_PARAM_NOT_USED(srcTwelveOctetToChange);
    TGF_PARAM_NOT_USED(srcThirteenOctetToChange);
    TGF_PARAM_NOT_USED(srcFourteenOctetToChange);
    TGF_PARAM_NOT_USED(srcFifteenOctetToChange);
    TGF_PARAM_NOT_USED(srcSixteenOctetToChange);
    TGF_PARAM_NOT_USED(grpLastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(srcLastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixBulkAdd function
* @endinternal
*
* @brief   Add bulk of IPv4 UC prefixes
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixBulkAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                *vrId,
    IN GT_IPADDR                             *ipAddr,
    IN GT_U32                                *prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    IN GT_BOOL                               *override,
    IN GT_U32                                sizeOfBulk
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));


    dxChNextHopInfo = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    if (dxChNextHopInfo == NULL)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(dxChNextHopInfo,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                pclIpUcActionPtr = &nextHopInfoPtr[i].pclIpUcAction;

                rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo[i].pclIpUcAction);
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);
                    return rc;
                }

                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                ipLttEntryPtr = &nextHopInfoPtr[i].ipLttEntry;

                /* set route type */
                rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo[i].ipLttEntry.routeType));
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);
                    return rc;
                }

                /* convert ltt entry into device specific format */
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, numOfPaths);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);

                break;

            default:
                cpssOsFree(ipPrefixArrayPtr);
                cpssOsFree(dxChNextHopInfo);
                return GT_BAD_PARAM;
        }
        cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopInfo,
                     &dxChNextHopInfo[i],
                     sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        ipPrefixArrayPtr[i].override = override[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        cpssOsFree(dxChNextHopInfo);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixAddBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    cpssOsFree(dxChNextHopInfo);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv4UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing IPv4 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixBulkDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         *vrId,
    IN GT_IPADDR                      *ipAddr,
    IN GT_U32                         *prefixLen,
    IN GT_U32                         sizeOfBulk
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixDelBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;


#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixBulkAdd function
* @endinternal
*
* @brief   Add bulk of IPv6 UC prefixes
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixBulkAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                *vrId,
    IN GT_IPV6ADDR                           *ipAddr,
    IN GT_U32                                *prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    IN GT_BOOL                               *override,
    IN GT_BOOL                               defragmentationEnable,
    IN GT_U32                                sizeOfBulk
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    dxChNextHopInfo = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    if (dxChNextHopInfo == NULL)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return GT_OUT_OF_CPU_MEM;
    }
    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPV6ADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                pclIpUcActionPtr = &nextHopInfoPtr[i].pclIpUcAction;

                rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo[i].pclIpUcAction);
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);
                    return rc;
                }

                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                ipLttEntryPtr = &nextHopInfoPtr[i].ipLttEntry;

                /* set route type */
                rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo[i].ipLttEntry.routeType));
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);
                    return rc;
                }

                /* convert ltt entry into device specific format */
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, numOfPaths);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, priority);

                break;

            default:
                cpssOsFree(ipPrefixArrayPtr);
                cpssOsFree(dxChNextHopInfo);
                return GT_BAD_PARAM;
        }
        cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopInfo,
                     &dxChNextHopInfo[i],
                     sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        ipPrefixArrayPtr[i].override = override[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr, defragmentationEnable);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        cpssOsFree(dxChNextHopInfo);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6UcPrefixAddBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    cpssOsFree(dxChNextHopInfo);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpLpmIpv6UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing IPv6 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixBulkDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         *vrId,
    IN GT_IPV6ADDR                    *ipAddr,
    IN GT_U32                         *prefixLen,
    IN GT_U32                         sizeOfBulk
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPV6ADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6UcPrefixDelBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;


#endif /* CHX_FAMILY */

}



/**
* @internal prvTgfIpv4PbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] pbrConfigPtr             - pbr configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfIpv4PbrConfigurationSet
(
    IN PRV_TGF_IP_ROUTE_CNFG_STC *pbrConfigPtr
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_ETHERADDR                            arpMacAddr;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    cpssOsMemSet(&nextHopInfo,0,sizeof(nextHopInfo));

    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[pbrConfigPtr->sendPortNum],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,/* PBR must be in lookup 1 ! */
        PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfPclDefPortInitExt1 FAILED, rc = [%d],devNum = [%d]", rc,pbrConfigPtr->devNum);
        return rc;
    }

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(pbrConfigPtr->sendVlanId, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfIpVlanRoutingEnable FAILED, rc = [%d],devNum = [%d], portNum = [%d]", rc,pbrConfigPtr->devNum,prvTgfPortsArray[pbrConfigPtr->sendPortNum]);
        return rc;
    }

    if (pbrConfigPtr->isArpNeeded == GT_TRUE)
    {
        /* write ARP MAC address to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, pbrConfigPtr->prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfIpRouterArpAddrWrite(pbrConfigPtr->routerArpIndex, &arpMacAddr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpRouterArpAddrWrite FAILED, rc = [%d],devNum = [%d]", rc,pbrConfigPtr->devNum);
            return rc;
        }
    }

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = pbrConfigPtr->nextHopVlanId;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = pbrConfigPtr->devNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[pbrConfigPtr->nextHopPortNum];
    if (pbrConfigPtr->isTunnelStart == GT_TRUE)
    {
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType = PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr = pbrConfigPtr->tunnelStartIndex;
    }
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, pbrConfigPtr->dstIpAddr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(pbrConfigPtr->lpmDbId,0, ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpLpmIpv4UcPrefixAdd FAILED, rc = [%d],devNum = [%d]", rc,pbrConfigPtr->devNum);
    }
    return rc;
}


/**
* @internal prvTgfIpv4LttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] lttConfigPtr             - ltt routing configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfIpv4LttRouteConfigurationSet
(
    IN PRV_TGF_IP_ROUTE_CNFG_STC *lttConfigPtr
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_ETHERADDR                            arpMacAddr;


    cpssOsMemSet(&nextHopInfo,0,sizeof(nextHopInfo));

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(lttConfigPtr->sendPortNum, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfIpPortRoutingEnable FAILED, rc = [%d],devNum = [%d], portNum = [%d]",
                          rc,lttConfigPtr->devNum,prvTgfPortsArray[lttConfigPtr->sendPortNum]);
        return rc;
    }

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(lttConfigPtr->sendVlanId, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfIpPortRoutingEnable FAILED, rc = [%d],devNum = [%d], portNum = [%d]",
                          rc,lttConfigPtr->devNum,prvTgfPortsArray[lttConfigPtr->sendPortNum]);
        return rc;
    }

    if (lttConfigPtr->isArpNeeded == GT_TRUE)
    {
        /* write ARP MAC address to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, lttConfigPtr->prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfIpRouterArpAddrWrite(lttConfigPtr->routerArpIndex, &arpMacAddr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpRouterArpAddrWrite FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
            return rc;
        }
    }

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

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
    regularEntryPtr->countSet                   = lttConfigPtr->countSet;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = lttConfigPtr->isTunnelStart;
    regularEntryPtr->nextHopVlanId              = lttConfigPtr->nextHopVlanId;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = lttConfigPtr->devNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[lttConfigPtr->nextHopPortNum];
    regularEntryPtr->nextHopARPPointer          = lttConfigPtr->routerArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = lttConfigPtr->tunnelStartIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(lttConfigPtr->routeEntryBaseIndex, routeEntriesArray, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpUcRouteEntriesWrite FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
        return rc;
    }

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(lttConfigPtr->devNum, lttConfigPtr->routeEntryBaseIndex, routeEntriesArray, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpUcRouteEntriesRead FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
        return rc;
    }

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = lttConfigPtr->routeEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, lttConfigPtr->dstIpAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(lttConfigPtr->lpmDbId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpLpmIpv4UcPrefixAdd FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
    }

    return rc;
}

/**
* @internal prvTgfIpHeaderErrorMaskSet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
* @param[in] mask                     - GT_TRUE:  the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS prvTgfIpHeaderErrorMaskSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_HEADER_ERROR_ENT          ipHeaderErrorType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT           prefixType,
    IN GT_BOOL                              mask
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_HEADER_ERROR_ENT           dxChIpHeaderErrorType;
    GT_STATUS                               rc;

    /* convert TTI header error type into device specific format */
    rc = prvTgfConvertGenericToDxChIpHeaderError(ipHeaderErrorType, &dxChIpHeaderErrorType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpHeaderErrorMaskSet(devNum, dxChIpHeaderErrorType, protocolStack, prefixType, mask);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ipHeaderErrorType);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixType);
    TGF_PARAM_NOT_USED(mask);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpHeaderErrorMaskGet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
*
* @param[out] maskPtr                  - GT_TRUE: mask the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS prvTgfIpHeaderErrorMaskGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_IP_HEADER_ERROR_ENT         ipHeaderErrorType,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN  CPSS_UNICAST_MULTICAST_ENT          prefixType,
    OUT GT_BOOL                             *maskPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_HEADER_ERROR_ENT           dxChIpHeaderErrorType;
    GT_STATUS                               rc;

    /* convert TTI header error type into device specific format */
    rc = prvTgfConvertGenericToDxChIpHeaderError(ipHeaderErrorType, &dxChIpHeaderErrorType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpHeaderErrorMaskGet(devNum, dxChIpHeaderErrorType, protocolStack, prefixType, maskPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ipHeaderErrorType);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixType);
    TGF_PARAM_NOT_USED(maskPtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
* @param[in] modifyEnable             - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 modifyEnable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    if(!PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        PRV_UTF_LOG3_MAC("prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet : (%d,%d,%d) =======\n" ,
                            devNum ,
                            portNum,
                            modifyEnable);
        rc = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(devNum,portNum,modifyEnable);
    }

    return rc;
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(modifyEnable);
    return GT_NOT_IMPLEMENTED;
#endif

}

/**
* @internal prvTgfIpArpBcModeSet function
* @endinternal
*
* @brief   set a arp broadcast mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpArpBcModeSet
(
    IN CPSS_PACKET_CMD_ENT arpBcMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpArpBcModeSet(devNum, arpBcMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChIpArpBcModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpArpBcModeGet function
* @endinternal
*
* @brief   get a arp broadcast mode.
*
* @param[out] arpBcModePtr             - the arp broadcast command. Possible Commands:
*                                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpArpBcModeGet
(
    OUT CPSS_PACKET_CMD_ENT *arpBcModePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    return cpssDxChIpArpBcModeGet(devNum, arpBcModePtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfIpEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN PRV_TGF_IP_ECMP_ENTRY_STC    *ecmpEntryPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_ECMP_ENTRY_STC cpssEcmpEntry;

    cpssOsMemSet(&cpssEcmpEntry,0,sizeof(cpssEcmpEntry));

    cpssEcmpEntry.numOfPaths = ecmpEntryPtr->numOfPaths;
    cpssEcmpEntry.randomEnable = ecmpEntryPtr->randomEnable;
    cpssEcmpEntry.routeEntryBaseIndex = ecmpEntryPtr->routeEntryBaseIndex;
    switch (ecmpEntryPtr->multiPathMode) {
    case PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E:
        cpssEcmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        break;
    case PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E:
        cpssEcmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &cpssEcmpEntry);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ecmpEntryIndex);
    TGF_PARAM_NOT_USED(ecmpEntryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT PRV_TGF_IP_ECMP_ENTRY_STC    *ecmpEntryPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_ECMP_ENTRY_STC cpssEcmpEntry;
    GT_STATUS                   rc;

    cpssOsMemSet(&cpssEcmpEntry,0,sizeof(cpssEcmpEntry));

    /* call device specific API */
    rc = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &cpssEcmpEntry);
    if (rc == GT_OK)
    {
        ecmpEntryPtr->numOfPaths = cpssEcmpEntry.numOfPaths;
        ecmpEntryPtr->randomEnable = cpssEcmpEntry.randomEnable;
        ecmpEntryPtr->routeEntryBaseIndex = cpssEcmpEntry.routeEntryBaseIndex;
        switch (cpssEcmpEntry.multiPathMode) {
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
            ecmpEntryPtr->multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
            ecmpEntryPtr->multiPathMode = PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;
        default:
            return GT_BAD_STATE;
        }
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ecmpEntryIndex);
    TGF_PARAM_NOT_USED(ecmpEntryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpRouterSourceIdSet function
* @endinternal
*
* @brief   set the router source id assignment
*
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
* @param[in] sourceId                 - the assigned source id.
* @param[in] sourceIdMask             - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterSourceIdSet
(
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpRouterSourceIdSet(devNum, ucMcSet, sourceId, sourceIdMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChIpRouterSourceIdSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

}


/**
* @internal prvTgfIpRouterSourceIdGet function
* @endinternal
*
* @brief   get the router source id assignment
*
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
*
* @param[out] sourceIdPtr              - the assigned source id.
* @param[out] sourceIdMaskPtr          - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterSourceIdGet
(
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChIpRouterSourceIdGet(prvTgfDevNum, ucMcSet, sourceIdPtr, sourceIdMaskPtr);
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
* @param[in] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS prvTgfLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN PRV_TGF_LPM_LEAF_ENTRY_STC               *leafPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leafEntry;

    cpssOsMemSet(&leafEntry,0,sizeof(leafEntry));

    /* call device specific API */
    switch (leafPtr->entryType)
    {
        case PRV_TGF_LPM_REGULAR_NODE_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_ECMP_ENTRY_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_QOS_ENTRY_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    leafEntry.index = leafPtr->index;
    leafEntry.ucRPFCheckEnable = leafPtr->ucRPFCheckEnable;
    leafEntry.sipSaCheckMismatchEnable = leafPtr->sipSaCheckMismatchEnable;
    leafEntry.ipv6MCGroupScopeLevel = leafPtr->ipv6MCGroupScopeLevel;
    leafEntry.priority = leafPtr->priority;
    return cpssDxChLpmLeafEntryWrite(devNum, leafIndex, &leafEntry);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(leafIndex);
    TGF_PARAM_NOT_USED(leafPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS prvTgfLpmLeafEntryRead
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    OUT PRV_TGF_LPM_LEAF_ENTRY_STC              *leafPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leafEntry;
    GT_STATUS                   rc;

    /* call device specific API */
    rc = cpssDxChLpmLeafEntryRead(devNum, leafIndex, &leafEntry);
    if (rc == GT_OK)
    {
        switch (leafEntry.entryType)
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_REGULAR_NODE_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_ECMP_ENTRY_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_QOS_ENTRY_PTR_TYPE_E;
            break;
        default:
            return GT_BAD_PARAM;
        }

        leafPtr->index = leafEntry.index;
        leafPtr->ucRPFCheckEnable = leafEntry.ucRPFCheckEnable;
        leafPtr->sipSaCheckMismatchEnable = leafEntry.sipSaCheckMismatchEnable;
        leafPtr->ipv6MCGroupScopeLevel = leafEntry.ipv6MCGroupScopeLevel;
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(leafIndex);
    TGF_PARAM_NOT_USED(leafPtr);
    return GT_BAD_STATE;
#endif
}
/**
* @internal prvTgfIpMllMultiTargetShaperBaselineSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline,
*         the respective packet is not served.
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes
*                                      (APPLICABLE RANGES: 0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU (the maximum expected packet size in the system).
*       2. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       cpssDxChIpMllMultiTargetShaperMtuSet.
*
*/
GT_STATUS   prvTgfIpMllMultiTargetShaperBaselineSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseline
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperBaselineSet(devNum, baseline);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(baseline);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperBaselineGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective packet
*         is not served.
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperBaselineGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *baselinePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperBaselineGet(devNum, baselinePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(baselinePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperMtuSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
* @param[in] devNum                   - physical device number
* @param[in] mtu                      - MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mtu
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperMtuSet
(
    IN GT_U8   devNum,
    IN GT_U32  mtu
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperMtuSet(devNum, mtu);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mtu);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperMtuGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
* @param[in] devNum                   - physical device number
*
* @param[out] mtuPtr                   -  pointer to MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperMtuGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mtuPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperMtuGet(devNum, mtuPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mtuPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperTokenBucketModeSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Mode Mll shaper.
* @param[in] devNum                   - device number.
* @param[in] tokenBucketMode          -   Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperTokenBucketModeSet
(
    IN  GT_U8                                        devNum,
    IN  CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketMode
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(devNum, tokenBucketMode);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperTokenBucketModeGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Mode Mll shaper.
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketModePtr       - (pointer to)Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperTokenBucketModeGet
(
    IN  GT_U8                                        devNum,
    OUT CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            *tokenBucketModePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(devNum, tokenBucketModePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketModePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpUcRpfModeSet function
* @endinternal
*
* @brief   Defines the uRPF check mode for a given VID.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
* @param[in] uRpfMode                 - unicast RPF mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, vid or uRpfMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS prvTgfIpUcRpfModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    IN  PRV_TGF_IP_URPF_MODE_ENT        uRpfMode
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_URPF_MODE_ENT          dxChIpVlanUrpfModeType;
    GT_STATUS                           rc;

    /* convert IP Vlan URPF Mode type into device specific format */
    rc = prvTgfConvertGenericToDxChUnicastRpfType(uRpfMode, &dxChIpVlanUrpfModeType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpUcRpfModeSet(devNum, vid, dxChIpVlanUrpfModeType);

    /* call device specific API */
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(uRpfMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpUcRpfModeGet function
* @endinternal
*
* @brief   Read uRPF check mode for a given VID.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
*
* @param[out] uRpfModePtr              -  (pointer to) unicast RPF mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS prvTgfIpUcRpfModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    OUT PRV_TGF_IP_URPF_MODE_ENT        *uRpfModePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_URPF_MODE_ENT          dxChIpVlanUrpfModeType;
    GT_STATUS                           rc;

    /* call device specific API */
    rc = cpssDxChIpUcRpfModeGet(devNum, vid, &dxChIpVlanUrpfModeType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert IP Vlan URPF Mode type from device specific format */
    rc = prvTgfConvertDxChToGenericUnicastRpfType(dxChIpVlanUrpfModeType, uRpfModePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

    /* call device specific API */
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(uRpfModePtr);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfIpUrpfLooseModeTypeSet function
* @endinternal
*
* @brief   This function set type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3; AC5x ; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum                   -  the device number
* @param[in] looseModeType            - value of urpf loose mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpUrpfLooseModeTypeSet
(
    IN  GT_U8                            devNum,
    IN  PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT looseModeType
)
{
    #ifdef CHX_FAMILY
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT  dxChUrpfLooseModeType;
    GT_STATUS                           rc;

    /* convert URPF Loose Mode type into device specific format */
    rc = prvTgfConvertGenericToDxChUnicastRpfLooseModeType(looseModeType, &dxChUrpfLooseModeType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpUrpfLooseModeTypeSet(devNum, dxChUrpfLooseModeType);

    /* call device specific API */
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(uRpfMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperEnableSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Enable/Disable Token Bucket rate shaping.
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvTgfIpMllMultiTargetShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperEnableGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Enable/Disable Token Bucket rate shaping status
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - Pointer to Token Bucket rate shaping status.
*                                      - GT_TRUE, enable Shaping
*                                      - GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfIpMllMultiTargetShaperConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper.
* @param[in] devNum                   - device number.
* @param[in] maxBucketSize            - Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 means 4k and 0xFFF means 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       see:
*       cpssDxChIpMllMultiTargetShaperMtuSet
*       cpssDxChIpMllMultiTargetShaperBaselineSet.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperConfigurationSet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       maxBucketSize,
    INOUT GT_U32                                     *maxRatePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperConfigurationSet(devNum,
                                                          maxBucketSize,
                                                          maxRatePtr);
#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxBucketSize);
    TGF_PARAM_NOT_USED(maxRatePtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfIpMllMultiTargetShaperConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper.
* @param[in] devNum                   - device number.
*
* @param[out] maxBucketSizePtr         - (pointer to) Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 equal 4k all 12'bFFF equal 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperConfigurationGet
(
    IN  GT_U8                                        devNum,
    OUT GT_U32                                       *maxBucketSizePtr,
    OUT GT_U32                                       *maxRatePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperConfigurationGet(devNum,
                                                          maxBucketSizePtr,
                                                          maxRatePtr);
#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxBucketSizePtr);
    TGF_PARAM_NOT_USED(maxRatePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperIntervalConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper Interval.
* @param[in] devNum                   - device number.
* @param[in] tokenBucketIntervalSlowUpdateRatio - Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[in] tokenBucketUpdateInterval -   Defines the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[in] tokenBucketIntervalUpdateRatio - Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperIntervalConfigurationSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          tokenBucketIntervalSlowUpdateRatio,
    IN  GT_U32                                          tokenBucketUpdateInterval,
    IN  PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT     tokenBucketIntervalUpdateRatio
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   dxchTokenBucketIntervalUpdateRatio;
    switch(tokenBucketIntervalUpdateRatio)
    {
    case PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
        dxchTokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;
        break;
    case PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
        dxchTokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(devNum,
                                                          tokenBucketIntervalSlowUpdateRatio,
                                                          tokenBucketUpdateInterval,
                                                          dxchTokenBucketIntervalUpdateRatio);
#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketIntervalSlowUpdateRatio);
    TGF_PARAM_NOT_USED(tokenBucketUpdateInterval);
    TGF_PARAM_NOT_USED(tokenBucketIntervalUpdateRatio);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperIntervalConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper Interval.
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketIntervalSlowUpdateRatioPtr - (pointer to) Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[out] tokenBucketUpdateIntervalPtr -   (pointer to) the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[out] tokenBucketIntervalUpdateRatioPtr - (pointer to) Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperIntervalConfigurationGet
(
    IN   GT_U8                                           devNum,
    OUT  GT_U32                                          *tokenBucketIntervalSlowUpdateRatioPtr,
    OUT  GT_U32                                          *tokenBucketUpdateIntervalPtr,
    OUT  PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT     *tokenBucketIntervalUpdateRatioPtr
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   dxchTokenBucketIntervalUpdateRatio;


    /* call device specific API */
    rc = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(devNum,
                                                          tokenBucketIntervalSlowUpdateRatioPtr,
                                                          tokenBucketUpdateIntervalPtr,
                                                          &dxchTokenBucketIntervalUpdateRatio);

    if(rc != GT_OK)
        return rc;

    switch(dxchTokenBucketIntervalUpdateRatio)
    {
    case CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
        *tokenBucketIntervalUpdateRatioPtr = PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;
        break;
    case CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
        *tokenBucketIntervalUpdateRatioPtr = PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
        break;
    default:
        return GT_BAD_VALUE;
    }

    return rc;

#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketIntervalSlowUpdateRatioPtr);
    TGF_PARAM_NOT_USED(tokenBucketUpdateIntervalPtr);
    TGF_PARAM_NOT_USED(tokenBucketIntervalUpdateRatioPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpPortSipSaEnableSet function
* @endinternal
*
* @brief   Enable SIP/SA check for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
* @param[in] enable                   - GT_FALSE: disable SIP/SA check on the port
*                                      GT_TRUE:  enable SIP/SA check on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS prvTgfIpPortSipSaEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPortSipSaEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortSipSaEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpPortSipSaEnableGet function
* @endinternal
*
* @brief   Return the SIP/SA check status for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
*
* @param[out] enablePtr                - GT_FALSE: SIP/SA check on the port is disabled
*                                      GT_TRUE:  SIP/SA check on the port is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS prvTgfIpPortSipSaEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPortSipSaEnableGet(devNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortSipSaEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpEcmpHashNumBitsSet function
* @endinternal
*
* @brief   Set the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism
* @param[in] devNum                   - the device number
* @param[in] startBit                 - the index of the first bit that is needed by the L3
*                                      ECMP hash mechanism (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - the number of bits that is needed by the L3 ECMP hash
*                                      mechanism (APPLICABLE RANGES: 1..16)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong startBit or numOfBits
*
* @note startBit is relevant only when randomEnable field in the entry is set to
*       GT_FALSE.
*       startBit + numOfBits must not exceed 32.
*
*/
GT_STATUS prvTgfIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpEcmpHashNumBitsSet(devNum, startBit, numOfBits);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpEcmpHashNumBitsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(startBit);
    TGF_PARAM_NOT_USED(numOfBits);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfIpEcmpHashNumBitsGet function
* @endinternal
*
* @brief   Get the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism
* @param[in] devNum                   - the device number
*
* @param[out] startBitPtr              - (pointer to) the index of the first bit that is needed
*                                      by the L3 ECMP hash mechanism
* @param[out] numOfBitsPtr             - (pointer to) the number of bits that is needed by the
*                                      L3 ECMP hash mechanism
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note startBit is relevant only when randomEnable field in the entry is set to
*       GT_FALSE.
*
*/
GT_STATUS prvTgfIpEcmpHashNumBitsGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpEcmpHashNumBitsGet(devNum, startBitPtr, numOfBitsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpEcmpHashNumBitsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(startBitPtr);
    TGF_PARAM_NOT_USED(numOfBitsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet function
* @endinternal
*
* @brief   Enable/disable bypassing the router triggering requirements for policy
*         based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  bypassing the router triggering requirements for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet function
* @endinternal
*
* @brief   Get the enabling status of bypassing the router triggering requirements
*         for policy based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - the router triggering requirements enabling status for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}


#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChIpNatRouteEntry function
* @endinternal
*
* @brief   Convert generic IP NAT route entry into device specific entry
*
* @param[in] devNum                   - physical device number
* @param[in] natType                  - NAT type
* @param[in] prvTgfNatEntryPtr        - (pointer to) IP NAT route entry
*
* @param[out] dxChIpNatRouteEntryPtr   - (pointer to) DxCh IP NAT route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpNatRouteEntry
(
    IN  GT_U8                         devNum,
    IN  PRV_TGF_IP_NAT_TYPE_ENT       natType,
    IN  PRV_TGF_IP_NAT_ENTRY_UNT      *prvTgfNatEntryPtr,
    OUT CPSS_DXCH_IP_NAT_ENTRY_UNT    *dxChIpNatRouteEntryPtr
)
{
    switch(natType)
    {
        case PRV_TGF_IP_NAT_TYPE_NAT44_E:
             /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), macDa);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyDip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newDip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifySip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newSip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpDstPort);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpDstPort);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpSrcPort);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpSrcPort);
            break;
        case PRV_TGF_IP_NAT_TYPE_NAT66_E:
             /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), macDa);
            switch (prvTgfNatEntryPtr->nat66Entry.modifyCommand)
            {
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_WO_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_WO_E;
                        break;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_WO_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_WO_E;
                        break;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                default:
                    return GT_BAD_PARAM;
            }
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), address);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), prefixSize);
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericIpNatRouteEntry function
* @endinternal
*
* @brief   Convert device specific IP NAT route entry into generic entry
*
* @param[in] devNum                   - physical device number
* @param[in] natType                  - NAT type
* @param[in] dxChIpNatRouteEntryPtr   - (pointer to) DxCh IP NAT route entry
*
* @param[out] prvTgfNatEntryPtr        - (pointer to) IP NAT route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericIpNatRouteEntry
(
    IN  GT_U8                        devNum,
    IN  PRV_TGF_IP_NAT_TYPE_ENT      natType,
    IN  CPSS_DXCH_IP_NAT_ENTRY_UNT   *dxChIpNatRouteEntryPtr,
    OUT PRV_TGF_IP_NAT_ENTRY_UNT     *prvTgfNatEntryPtr
)
{
    switch(natType)
    {
        case PRV_TGF_IP_NAT_TYPE_NAT44_E:
            /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), macDa);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyDip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newDip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifySip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newSip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpDstPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpDstPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpSrcPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpSrcPort);
            break;
        case PRV_TGF_IP_NAT_TYPE_NAT66_E:

            /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), macDa);
            switch (dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand)
            {
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_WO_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_WO_E;
                        break;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_WO_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_WO_E;
                        break;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                default:
                    return GT_BAD_PARAM;
            }
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), address);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), prefixSize);
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /* CHX_FAMILY */


/**
* @internal prvTgfIpNatEntrySet function
* @endinternal
*
* @brief   Set a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] natIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
* @param[in] natType                  - type of the NAT
* @param[in] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table, Tunnel start entries table and router ARP addresses
*       table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS prvTgfIpNatEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              natIndex,
    IN  PRV_TGF_IP_NAT_TYPE_ENT             natType,
    IN  PRV_TGF_IP_NAT_ENTRY_UNT           *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_IP_NAT_TYPE_ENT        natTypeCpss;
    CPSS_DXCH_IP_NAT_ENTRY_UNT  entryCpss;


    switch(natType)
    {
        case PRV_TGF_IP_NAT_TYPE_NAT44_E:
            natTypeCpss = CPSS_IP_NAT_TYPE_NAT44_E;
            break;
        case PRV_TGF_IP_NAT_TYPE_NAT66_E:
            natTypeCpss = CPSS_IP_NAT_TYPE_NAT66_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    prvTgfConvertGenericToDxChIpNatRouteEntry(devNum, natType, entryPtr, &entryCpss);

    /* call device specific API */
    rc = cpssDxChIpNatEntrySet(devNum, natIndex, natTypeCpss, &entryCpss);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpNatEntrySet FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(natIndex);
    TGF_PARAM_NOT_USED(natType);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpNatEntryGet function
* @endinternal
*
* @brief   Get a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] natIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
*
* @param[out] natTypePtr               - (pointer to) type of the NAT
* @param[out] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table, Tunnel start entries table and router ARP addresses
*       table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS prvTgfIpNatEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             natIndex,
    OUT  PRV_TGF_IP_NAT_TYPE_ENT            *natTypePtr,
    OUT  PRV_TGF_IP_NAT_ENTRY_UNT           *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_IP_NAT_TYPE_ENT        natTypeCpss;
    CPSS_DXCH_IP_NAT_ENTRY_UNT  entryCpss;

    /* call device specific API */
    rc = cpssDxChIpNatEntryGet(devNum, natIndex, &natTypeCpss, &entryCpss);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpNatEntryGet FAILED, rc = [%d]", rc);
    }

    switch(natTypeCpss)
    {
        case CPSS_IP_NAT_TYPE_NAT44_E:
            *natTypePtr = PRV_TGF_IP_NAT_TYPE_NAT44_E;
            break;
        case CPSS_IP_NAT_TYPE_NAT66_E:
            *natTypePtr = PRV_TGF_IP_NAT_TYPE_NAT66_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    prvTgfConvertDxChToGenericIpNatRouteEntry(devNum, *natTypePtr, &entryCpss, entryPtr);

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(natIndex);
    TGF_PARAM_NOT_USED(natType);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerBlockPrint
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwOctetPerBlockPrint(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDbgHwOctetPerBlockPrint FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrint
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwOctetPerProtocolPrint(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDbgHwOctetPerProtocolPrint FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwBlockInfoPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwBlockInfoPrint
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwBlockInfoPrint(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDbgHwBlockInfoPrint FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc =
        cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllBridgeEnable function
* @endinternal
*
* @brief   enable/disable MLL based bridging.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; AC5; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] mllBridgeEnable          - enable /disable MLL based bridging.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllBridgeEnable
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    mllBridgeEnable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpMllBridgeEnable(devNum, mllBridgeEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpMllBridgeEnable FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mllBridgeEnable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpBridgeServiceEnable function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; AC5; Lion; Lion2; Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
* @param[in] enableService            - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpBridgeServiceEnable
(
    IN  GT_U8                                           devNum,
    IN  PRV_TGF_IP_BRG_SERVICE_ENT                      bridgeService,
    IN  PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT  enableDisableMode,
    IN  GT_BOOL                                         enableService
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_BRG_SERVICE_ENT service;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT mode;
    switch(bridgeService)
    {
    case PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_HEADER_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_FILTER_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_LAST_E:
        service = CPSS_DXCH_IP_BRG_SERVICE_LAST_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch(enableDisableMode)
    {
    case PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return cpssDxChIpBridgeServiceEnable(devNum, service, mode, enableService);
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(bridgeService);
    TGF_PARAM_NOT_USED(enableDisableMode);
    TGF_PARAM_NOT_USED(enableService);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpBridgeServiceEnableGet function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; AC5; Lion; Lion2; Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
*
* @param[out] enableServicePtr         - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpBridgeServiceEnableGet
(
    IN  GT_U8                                           devNum,
    IN  PRV_TGF_IP_BRG_SERVICE_ENT                      bridgeService,
    IN  PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT  enableDisableMode,
    OUT GT_BOOL                                         *enableServicePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_BRG_SERVICE_ENT service;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT mode;
    switch(bridgeService)
    {
    case PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_HEADER_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_FILTER_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_LAST_E:
        service = CPSS_DXCH_IP_BRG_SERVICE_LAST_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch(enableDisableMode)
    {
    case PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return cpssDxChIpBridgeServiceEnableGet(devNum, service, mode,
        enableServicePtr);
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(bridgeService);
    TGF_PARAM_NOT_USED(enableDisableMode);
    TGF_PARAM_NOT_USED(enableServicePtr);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfIpPortFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Enable FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - the port to enable on
* @param[in] enable                   -  FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpPortFcoeForwardingEnableSet
(
    IN GT_PORT_NUM                      portNum,
    IN GT_BOOL                          enable
)
{
    GT_STATUS   rc;

    rc = cpssDxChIpPortFcoeForwardingEnableSet(prvTgfDevNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortFcoeForwardingEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfIpPortFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Get status of FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to)enable FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpPortFcoeForwardingEnableGet
(
    IN  GT_PORT_NUM                      portNum,
    OUT GT_BOOL                          *enablePtr
)
{
    GT_STATUS   rc;

    rc = cpssDxChIpPortFcoeForwardingEnableGet(prvTgfDevNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortFcoeForwardingEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfIpLpmFcoePrefixAdd function
* @endinternal
*
* @brief   This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvTgfIpLpmFcoePrefixAdd
(
    IN  GT_U32                                      lpmDBId,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    IN  CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT      *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN  GT_BOOL                                     defragmentationEnable
)
{
    GT_STATUS   rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    rc = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId,
                                    &fcoeAddr, prefixLen, nextHopInfoPtr, override, defragmentationEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmFcoePrefixAdd FAILED, rc = [%d]", rc);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvTgfIpValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_FCOE_E, CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }

    return rc;
}

/**
* @internal prvTgfIpLpmFcoePrefixDel function
* @endinternal
*
* @brief   Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvTgfIpLpmFcoePrefixDel
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 fcoeAddr,
    IN  GT_U32                                  prefixLen
)
{
    GT_STATUS   rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;
    rc = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmFcoePrefixDel FAILED, rc = [%d]", rc);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) &&
        (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvTgfIpValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_FCOE_E, CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }

    return rc;
}

/**
* @internal prvTgfIpFdbRoutePrefixLenSet function
* @endinternal
*
* @brief   set the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
* @param[in] prefixLen                - The number of bits that
*                                       are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpFdbRoutePrefixLenSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U32                       prefixLen
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbRoutePrefixLenSet(devNum, protocolStack, prefixLen);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixLen);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpFdbRoutePrefixLenGet function
* @endinternal
*
* @brief   get the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
*
* @param[out] prefixLen                - The number of bits that
*                                       are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_VALUE             - on bad output value
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpFdbRoutePrefixLenGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_U32                      *prefixLenPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbRoutePrefixLenGet(devNum, protocolStack, prefixLenPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixLenPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpFdbUnicastRouteForPbrEnableSet function
* @endinternal
*
* @brief   Enable/Disable FDB Unicast routing for PBR (Policy Based
*          Routed) packets
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FDB
*                                       routing for PBR packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS prvTgfIpFdbUnicastRouteForPbrEnableSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
)
{
    #ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbUnicastRouteForPbrEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpFdbUnicastRouteForPbrEnableGet function
* @endinternal
*
* @brief   Return if FDB Unicast routing for PBR value (Policy Based
*          Routed) packets is enabled
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr               - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS prvTgfIpFdbUnicastRouteForPbrEnableGet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbUnicastRouteForPbrEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfLpmRamSyncSwHwForHa function
* @endinternal
*
* @brief  Update SW Shadow with relevant data from HW, and
*         allocate DMM memory according to HW memory.
*         Relevant for HA process
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvTgfLpmRamSyncSwHwForHa
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return prvCpssDxChLpmRamSyncSwHwForHa();
#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpv6Convert function
* @endinternal
*
* @brief   Converts TGF_IPV6_ADDR to GT_IPV6ADDR
*
* @param[in] utfIpv6Ptr               - ptr to MAC address in UTF format
*
* @param[out] cpssIpv6Ptr              - ptr to MAC address in CPSS format
*                                       None
*/
GT_VOID prvTgfIpv6Convert
(
    IN  TGF_IPV6_ADDR *utfIpv6Ptr,
    OUT GT_IPV6ADDR   *cpssIpv6Ptr
)
{
    GT_U32 ii;
    GT_U32 idx = 0;
    for(ii = 0; ii < 8; ii++)
    {
        cpssIpv6Ptr->arIP[idx++] = ((*utfIpv6Ptr)[ii] >> 8) & 0xFF;
        cpssIpv6Ptr->arIP[idx++] = ((*utfIpv6Ptr)[ii] >> 0) & 0xFF;
    }
}

