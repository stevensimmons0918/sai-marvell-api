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
* @file cpssDxChIp.c
*
* @brief The CPSS DXCH Ip HW structures APIs
*
* @version   94
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIpLog.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Static function declarations */
static GT_STATUS prvCpssDxChIpHwFormat2MllStruct
(
    IN   GT_U32                 *hwData,
    IN   GT_BOOL                isFirstMll,
    OUT  CPSS_DXCH_IP_MLL_STC   *mllPairEntryPtr
);
static GT_STATUS prvCpssDxChIpMllStruct2HwFormat
(
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairWriteForm,
    IN  CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwData
);

static GT_STATUS sip5IpMllStruct2HwFormat/*sip5 : sw to hw */
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    IN  CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr,
    OUT GT_U32                                      *hwMaskPtr
);
static GT_STATUS sip5IpMllHw2StructFormat/*sip5 : hw to sw */
(
    IN  GT_U8                                        devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_IP_MLL_PAIR_STC                  *mllPairEntryPtr,
    IN GT_U32                                       *hwDataPtr
);

/* Convert hardware value to Aatribute tunnel start passenger type */
#define PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_ATTR_TUNNEL_START_PASS_TYPE_MAC(_attr, _val)  \
    switch (_val)                                                       \
    {                                                                   \
        case 0:                                                         \
            _attr = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;               \
            break;                                                      \
        case 1:                                                         \
            _attr = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;                     \
            break;                                                      \
        default:                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                         \
    }

#define CPSS_DXCH_SIZE_OF_MLL_NODE_IN_BYTE  (16)
#define CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD  (CPSS_DXCH_SIZE_OF_MLL_NODE_IN_BYTE >> 2)


/* number of bits in the Bobcat3 - MLL entry */
#define MAX_IP_MLL_ENTRY_BITS_SIZE_CNS       162


/* Bobcat2; Caelum; Bobcat3; Aldrin - number of word for the rule */
#define NUM_WORDS_RULE_CNS  8 /* need to be like TTI_RULE_SIZE_CNS in cpssDxChTti.c */

/* Bobcat2; Caelum; Bobcat3; Aldrin - macro assumption : value holds the 'value' and set it into hwFormatArray */
#define SIP5_IPVX_FIELD_VALUE_SET_MAC(_dev,_fieldName) \
    SIP5_IPVX_FIELD_SET_MAC(_dev,hwDataPtr,_fieldName,value)

/* Bobcat2; Caelum; Bobcat3; Aldrin - macro assumption : hwFormatArray holds the entry and macro fills the value with 'value'*/
#define SIP5_IPVX_FIELD_VALUE_GET_MAC(_dev,_fieldName) \
    SIP5_IPVX_FIELD_GET_MAC(_dev,hwDataPtr,_fieldName,value)

/**
* @internal prvCpssDxCh3Ipv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] routerTtiTcamColumn      - the TCAM column to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail.
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
static GT_STATUS prvCpssDxCh3Ipv4PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
{
    GT_U32  keyArr[NUM_WORDS_RULE_CNS];      /* TCAM key in hw format  */
    GT_U32  maskArr[NUM_WORDS_RULE_CNS];     /* TCAM mask in hw format */
    GT_U32  wordOffset;     /* word offset in hw format to set */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(prefixPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routerTtiTcamRow >= fineTuningPtr->tableSize.router)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (routerTtiTcamColumn >= 4)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (prefixPtr->isMcSource == GT_TRUE)
    {
        if (prefixPtr->mcGroupIndexRow >= fineTuningPtr->tableSize.router)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (prefixPtr->mcGroupIndexColumn >= 4)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (prefixPtr->vrId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /***********************************/
    /* convert to TCAM hardware format */
    /***********************************/

    /* clear hw data */
    cpssOsMemSet(keyArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);
    cpssOsMemSet(maskArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);

    /* set the ip address & mask*/
    switch (routerTtiTcamColumn)
    {
    case 0:
    case 2:
        wordOffset = (routerTtiTcamColumn == 0) ? 0 : 3;

        keyArr[wordOffset] = (prefixPtr->ipAddr.arIP[3]) |
                             (prefixPtr->ipAddr.arIP[2] << 8) |
                             (prefixPtr->ipAddr.arIP[1] << 16)|
                             (prefixPtr->ipAddr.arIP[0] << 24);

        maskArr[wordOffset] = (maskPtr->ipAddr.arIP[3]) |
                              (maskPtr->ipAddr.arIP[2] << 8) |
                              (maskPtr->ipAddr.arIP[1] << 16)|
                              (maskPtr->ipAddr.arIP[0] << 24);

        if (prefixPtr->isMcSource == GT_FALSE)
        {
            keyArr[wordOffset + 1] = (prefixPtr->vrId & 0xfff) |
                                     (0 << 15);     /* indicates not Mc source */

            maskArr[wordOffset + 1] = (maskPtr->vrId & 0xfff) |
                                      (1 << 15);    /* Mc source bit can't be masked */
        }
        else
        {
            keyArr[wordOffset + 1] = (prefixPtr->mcGroupIndexRow & 0x1fff) |
                                     ((prefixPtr->mcGroupIndexColumn & 0x3) << 13) |
                                     (1 << 15);     /* indicates Mc source */

            maskArr[wordOffset + 1] = (maskPtr->mcGroupIndexRow & 0x1fff) |
                                      ((maskPtr->mcGroupIndexColumn & 0x3) << 13) |
                                      (1 << 15);    /* Mc source bit can't be masked */
        }
        break;

    case 1:
    case 3:
        wordOffset = (routerTtiTcamColumn == 1) ? 1 : 4;

        keyArr[wordOffset] = (prefixPtr->ipAddr.arIP[3] << 16) |
                             (prefixPtr->ipAddr.arIP[2] << 24);

        keyArr[wordOffset + 1] = (prefixPtr->ipAddr.arIP[1]) |
                                 (prefixPtr->ipAddr.arIP[0] << 8);

        maskArr[wordOffset] = (maskPtr->ipAddr.arIP[3] << 16) |
                              (maskPtr->ipAddr.arIP[2] << 24);

        maskArr[wordOffset + 1] = (maskPtr->ipAddr.arIP[1]) |
                                  (maskPtr->ipAddr.arIP[0] << 8);

        if (prefixPtr->isMcSource == GT_FALSE)
        {
            keyArr[wordOffset + 1] |= ((prefixPtr->vrId & 0xfff) << 16) |
                                      (0 << 31);     /* indicates not Mc source */

            maskArr[wordOffset + 1] |= ((maskPtr->vrId & 0xfff) << 16) |
                                       (1 << 31);    /* Mc source bit can't be masked */
        }
        else
        {
            keyArr[wordOffset + 1] |= ((prefixPtr->mcGroupIndexRow & 0x1fff) << 16) |
                                      ((prefixPtr->mcGroupIndexColumn & 0x3) << 29) |
                                      (1 << 31);     /* indicates Mc source */

            maskArr[wordOffset + 1] |= ((maskPtr->mcGroupIndexRow & 0x1fff) << 16) |
                                       ((maskPtr->mcGroupIndexColumn & 0x3) << 29) |
                                       (1 << 31);    /* Mc source bit can't be masked */
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /*****************************/
    /* write the TCAM key & mask */
    /*****************************/

    return  prvCpssDxChRouterTunnelTermTcamKeyMaskWriteEntry(devNum,
                                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             routerTtiTcamRow,routerTtiTcamColumn,
                                                             keyArr,maskArr);
}


/**
* @internal internal_cpssDxChIpv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] routerTtiTcamColumn      - the TCAM column to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail.
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. IP prefix entries reside in router TCAM. The router TCAM is organized
*       in rows, where each row contains 4 column entries for Ch3 and 5 column
*       entries for Ch2. Each line can hold:
*       - 1 TTI (tunnel termination interface) entry
*       - 1 ipv6 address
*       - 4 (ch3)/ 5 (ch2) ipv4 addresses
*       Note that if a line is used for ipv6 address or TTI entry then
*       it can't be used for ipv4 addresses and if an entry in a line is used
*       for ipv4 address, then the other line entries must hold ipv4 addresses
*       as well.
*       2. The match for prefixes is done using a first match , thus if two prefixes
*       can match a packet (they have the same prefix , but diffrent prefix
*       length) and we want LPM to work on them we have to make sure to put
*       the longest prefix before the short one.
*       Search order is by columns, first column, then second and so on.
*       All and all default UC prefix 0.0.0.0/mask 0 must reside at the last
*       possible ipv4 prefix, and default MC prefix 224.0.0.0/ mask 240.0.0.0
*       must reside at the index before it.
*       3. In Ch2 MC source addresses can reside only on the second and third
*       columns (on other PP, there is no such restriction).
*       4. In Ch2 there is no support for multiple virtual routers. Therefore in
*       Ch2 vrId is ignored.
*
*/
static GT_STATUS internal_cpssDxChIpv4PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxCh3Ipv4PrefixSet(devNum,
                                     routerTtiTcamRow,routerTtiTcamColumn,
                                     prefixPtr,maskPtr);
}

/**
* @internal cpssDxChIpv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] routerTtiTcamColumn      - the TCAM column to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail.
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. IP prefix entries reside in router TCAM. The router TCAM is organized
*       in rows, where each row contains 4 column entries for Ch3 and 5 column
*       entries for Ch2. Each line can hold:
*       - 1 TTI (tunnel termination interface) entry
*       - 1 ipv6 address
*       - 4 (ch3)/ 5 (ch2) ipv4 addresses
*       Note that if a line is used for ipv6 address or TTI entry then
*       it can't be used for ipv4 addresses and if an entry in a line is used
*       for ipv4 address, then the other line entries must hold ipv4 addresses
*       as well.
*       2. The match for prefixes is done using a first match , thus if two prefixes
*       can match a packet (they have the same prefix , but diffrent prefix
*       length) and we want LPM to work on them we have to make sure to put
*       the longest prefix before the short one.
*       Search order is by columns, first column, then second and so on.
*       All and all default UC prefix 0.0.0.0/mask 0 must reside at the last
*       possible ipv4 prefix, and default MC prefix 224.0.0.0/ mask 240.0.0.0
*       must reside at the index before it.
*       3. In Ch2 MC source addresses can reside only on the second and third
*       columns (on other PP, there is no such restriction).
*       4. In Ch2 there is no support for multiple virtual routers. Therefore in
*       Ch2 vrId is ignored.
*
*/
GT_STATUS cpssDxChIpv4PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpv4PrefixSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerTtiTcamRow, routerTtiTcamColumn, prefixPtr, maskPtr));

    rc = internal_cpssDxChIpv4PrefixSet(devNum, routerTtiTcamRow, routerTtiTcamColumn, prefixPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerTtiTcamRow, routerTtiTcamColumn, prefixPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxCh3Ipv4PrefixGet function
* @endinternal
*
* @brief   Gets an ipv4 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
* @param[in] routerTtiTcamColumn      - the TCAM column to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
static GT_STATUS prvCpssDxCh3Ipv4PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
{
    GT_U32  keyArr[NUM_WORDS_RULE_CNS];          /* TCAM key in hw format            */
    GT_U32  maskArr[NUM_WORDS_RULE_CNS];         /* TCAM mask in hw format           */
    /* TCAM action (rule) in hw format  */
    GT_U32  actionArr[PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS];
    GT_U32  validArr[5];        /* TCAM line valid bits             */
    GT_U32  compareModeArr[5];  /* TCAM line compare mode           */
    GT_U32  wordOffset;         /* word offset in hw format to set  */
    GT_U32  retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routerTtiTcamRow >= fineTuningPtr->tableSize.router)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (routerTtiTcamColumn >= 4)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(prefixPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

     /* clear data */
    cpssOsMemSet(keyArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);
    cpssOsMemSet(maskArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);
    cpssOsMemSet(prefixPtr, 0, sizeof(CPSS_DXCH_IPV4_PREFIX_STC));
    cpssOsMemSet(maskPtr, 0, sizeof(CPSS_DXCH_IPV4_PREFIX_STC));

    /* read hw data */

    retVal = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                    routerTtiTcamRow,
                                                    &validArr[0],
                                                    &compareModeArr[0],
                                                    &keyArr[0],&maskArr[0],
                                                    &actionArr[0]);
    if (retVal != GT_OK)
        return retVal;

    /* convert from HW format to ip address & mask */
    /***********************************************/

    switch (routerTtiTcamColumn)
    {
    case 0:
    case 2:
        wordOffset = (routerTtiTcamColumn == 0) ? 0 : 3;

        prefixPtr->ipAddr.arIP[3] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset], 0,8);
        prefixPtr->ipAddr.arIP[2] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset], 8,8);
        prefixPtr->ipAddr.arIP[1] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset], 16,8);
        prefixPtr->ipAddr.arIP[0] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset], 24,8);

        maskPtr->ipAddr.arIP[3]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset], 0,8);
        maskPtr->ipAddr.arIP[2]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset], 8,8);
        maskPtr->ipAddr.arIP[1]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset], 16,8);
        maskPtr->ipAddr.arIP[0]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset], 24,8);

        prefixPtr->isMcSource = maskPtr->isMcSource =
            (((keyArr[wordOffset + 1] >> 15) & 0x1) == 0) ? GT_FALSE : GT_TRUE;

        if (prefixPtr->isMcSource == GT_FALSE)
        {
            prefixPtr->vrId = (keyArr[wordOffset + 1] & 0xfff);
            maskPtr->vrId =   (maskArr[wordOffset + 1] & 0xfff);
        }
        else
        {
            prefixPtr->mcGroupIndexRow =    ((keyArr[wordOffset + 1] & 0x1fff));
            prefixPtr->mcGroupIndexColumn = ((keyArr[wordOffset + 1] >> 13) & 0x3);
            maskPtr->mcGroupIndexRow =      ((maskArr[wordOffset + 1] & 0x1fff));
            maskPtr->mcGroupIndexColumn =   ((maskArr[wordOffset + 1] >> 13) & 0x3);
        }

        break;

    case 1:
    case 3:
        wordOffset = (routerTtiTcamColumn == 1) ? 1 : 4;

        prefixPtr->ipAddr.arIP[3] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset], 16,8);
        prefixPtr->ipAddr.arIP[2] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset], 24,8);

        prefixPtr->ipAddr.arIP[1] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset + 1], 0,8);
        prefixPtr->ipAddr.arIP[0] = (GT_U8)U32_GET_FIELD_MAC(keyArr[wordOffset + 1], 8,8);

        maskPtr->ipAddr.arIP[3]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset], 16,8);
        maskPtr->ipAddr.arIP[2]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset], 24,8);

        maskPtr->ipAddr.arIP[1]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset + 1], 0,8);
        maskPtr->ipAddr.arIP[0]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[wordOffset + 1], 8,8);

        prefixPtr->isMcSource = maskPtr->isMcSource =
            (((keyArr[wordOffset + 1] >> 31) & 0x1) == 0) ? GT_FALSE : GT_TRUE;

        if (prefixPtr->isMcSource == GT_FALSE)
        {
            prefixPtr->vrId = ((keyArr[wordOffset + 1] >> 16) & 0xfff);
            maskPtr->vrId =   ((maskArr[wordOffset + 1] >> 16) & 0xfff);
        }
        else
        {
            prefixPtr->mcGroupIndexRow =    ((keyArr[wordOffset + 1] >> 16) & 0x1fff);
            prefixPtr->mcGroupIndexColumn = ((keyArr[wordOffset + 1] >> 29) & 0x3);
            maskPtr->mcGroupIndexRow =      ((maskArr[wordOffset + 1] >> 16) & 0x1fff);
            maskPtr->mcGroupIndexColumn =   ((maskArr[wordOffset + 1] >> 29) & 0x3);
        }

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* entry holds valid IPv4 prefix if the followings applies:
         - the entry is valid
         - the compare mode of the entry is single compare mode */
    if ((validArr[routerTtiTcamColumn] == 1) && (compareModeArr[routerTtiTcamColumn] == 0))
        *validPtr = GT_TRUE;
    else
        *validPtr = GT_FALSE;

    return retVal;
}

/**
* @internal internal_cpssDxChIpv4PrefixGet function
* @endinternal
*
* @brief   Gets an ipv4 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
* @param[in] routerTtiTcamColumn      - the TCAM column to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
static GT_STATUS internal_cpssDxChIpv4PrefixGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn,
    OUT GT_BOOL     *validPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxCh3Ipv4PrefixGet(devNum,
                                     routerTtiTcamRow,routerTtiTcamColumn,
                                     validPtr,prefixPtr,maskPtr);
}

/**
* @internal cpssDxChIpv4PrefixGet function
* @endinternal
*
* @brief   Gets an ipv4 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
* @param[in] routerTtiTcamColumn      - the TCAM column to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
GT_STATUS cpssDxChIpv4PrefixGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn,
    OUT GT_BOOL     *validPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpv4PrefixGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerTtiTcamRow, routerTtiTcamColumn, validPtr, prefixPtr, maskPtr));

    rc = internal_cpssDxChIpv4PrefixGet(devNum, routerTtiTcamRow, routerTtiTcamColumn, validPtr, prefixPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerTtiTcamRow, routerTtiTcamColumn, validPtr, prefixPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpv4PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv4 UC or MC prefix in the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to invalidate
* @param[in] routerTtiTcamColumn      - the TCAM column to invalidate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
static GT_STATUS internal_cpssDxChIpv4PrefixInvalidate
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn
)
{
    GT_U32  retVal = GT_OK;         /* function return code */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);


    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routerTtiTcamRow >= fineTuningPtr->tableSize.router)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (routerTtiTcamColumn >= 4)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    retVal = prvCpssDxChRouterTunnelTermTcamInvalidateEntry(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,routerTtiTcamRow,routerTtiTcamColumn);
    return retVal;
}

/**
* @internal cpssDxChIpv4PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv4 UC or MC prefix in the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to invalidate
* @param[in] routerTtiTcamColumn      - the TCAM column to invalidate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChIpv4PrefixInvalidate
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpv4PrefixInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerTtiTcamRow, routerTtiTcamColumn));

    rc = internal_cpssDxChIpv4PrefixInvalidate(devNum, routerTtiTcamRow, routerTtiTcamColumn);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerTtiTcamRow, routerTtiTcamColumn));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxCh3Ipv6PrefixSet function
* @endinternal
*
* @brief   Sets an ipv6 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
static GT_STATUS prvCpssDxCh3Ipv6PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
{
    GT_U32  keyArr[NUM_WORDS_RULE_CNS];   /* TCAM key in hw format  */
    GT_U32  maskArr[NUM_WORDS_RULE_CNS];  /* TCAM mask in hw format */
    GT_U32  retVal;      /* function return code   */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
   CPSS_NULL_PTR_CHECK_MAC(prefixPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routerTtiTcamRow >= fineTuningPtr->tableSize.router)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (prefixPtr->isMcSource == GT_TRUE)
    {
        if (prefixPtr->mcGroupIndexRow >= fineTuningPtr->tableSize.router)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (prefixPtr->vrId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /***********************************/
    /* convert to TCAM hardware format */
    /***********************************/

    /* clear hw data */
     cpssOsMemSet(keyArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);
     cpssOsMemSet(maskArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);

    /* set the ip address key */
    /**************************/

    keyArr[0] = (prefixPtr->ipAddr.arIP[15]) |
                (prefixPtr->ipAddr.arIP[14] << 8) |
                (prefixPtr->ipAddr.arIP[13] << 16)|
                (prefixPtr->ipAddr.arIP[12] << 24);

    keyArr[1] = (prefixPtr->ipAddr.arIP[11]) |
                (prefixPtr->ipAddr.arIP[10] << 8) |
                (prefixPtr->ipAddr.arIP[9]  << 16)|
                (prefixPtr->ipAddr.arIP[8]  << 24);

    keyArr[2] = (prefixPtr->ipAddr.arIP[7]) |
                (prefixPtr->ipAddr.arIP[6] << 8) |
                (prefixPtr->ipAddr.arIP[5] << 16)|
                (prefixPtr->ipAddr.arIP[4] << 24);

    keyArr[3] = (prefixPtr->ipAddr.arIP[3]) |
                (prefixPtr->ipAddr.arIP[2] << 8) |
                (prefixPtr->ipAddr.arIP[1] << 16)|
                (prefixPtr->ipAddr.arIP[0] << 24);

    /* set the ip address mask */
    /***************************/

    maskArr[0] = (maskPtr->ipAddr.arIP[15]) |
                 (maskPtr->ipAddr.arIP[14] << 8) |
                 (maskPtr->ipAddr.arIP[13] << 16)|
                 (maskPtr->ipAddr.arIP[12] << 24);

    maskArr[1] = (maskPtr->ipAddr.arIP[11]) |
                 (maskPtr->ipAddr.arIP[10] << 8) |
                 (maskPtr->ipAddr.arIP[9]  << 16)|
                 (maskPtr->ipAddr.arIP[8]  << 24);

    maskArr[2] = (maskPtr->ipAddr.arIP[7]) |
                 (maskPtr->ipAddr.arIP[6] << 8) |
                 (maskPtr->ipAddr.arIP[5] << 16)|
                 (maskPtr->ipAddr.arIP[4] << 24);

    maskArr[3] = (maskPtr->ipAddr.arIP[3]) |
                 (maskPtr->ipAddr.arIP[2] << 8) |
                 (maskPtr->ipAddr.arIP[1] << 16)|
                 (maskPtr->ipAddr.arIP[0] << 24);

    /* set the vr Id or MC Group Index */
    /***********************************/

    if (prefixPtr->isMcSource == GT_FALSE)
    {
        keyArr[4] = (prefixPtr->vrId & 0xfff) |
                    (0 << 15); /* indicates not Mc Source */

        maskArr[4] = (maskPtr->vrId & 0xfff) |
                      0xffff8000;
    }
    else
    {
        keyArr[4] = (prefixPtr->mcGroupIndexRow & 0x7fff) |
                    (1 << 15); /* indicate Mc source */

        maskArr[4] = (maskPtr->mcGroupIndexRow & 0x7fff) |
                     0xffffffff;
    }

    keyArr[5] = 0;

    maskArr[5] = 0xffffffff;


    /*****************************/
    /* write the TCAM key & mask */
    /*****************************/
    retVal =  prvCpssDxChRouterTunnelTermTcamKeyMaskWriteLine(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              routerTtiTcamRow,keyArr,maskArr);
    if (retVal != GT_OK)
        return retVal;

    return GT_OK;
}


/**
* @internal internal_cpssDxChIpv6PrefixSet function
* @endinternal
*
* @brief   Sets an ipv6 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
static GT_STATUS internal_cpssDxChIpv6PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxCh3Ipv6PrefixSet(devNum,routerTtiTcamRow, prefixPtr,maskPtr);
}

/**
* @internal cpssDxChIpv6PrefixSet function
* @endinternal
*
* @brief   Sets an ipv6 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
GT_STATUS cpssDxChIpv6PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpv6PrefixSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerTtiTcamRow, prefixPtr, maskPtr));

    rc = internal_cpssDxChIpv6PrefixSet(devNum, routerTtiTcamRow, prefixPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerTtiTcamRow, prefixPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxCh3Ipv6PrefixGet function
* @endinternal
*
* @brief   Gets an ipv6 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv6PrefixSet
*
*/
static GT_STATUS prvCpssDxCh3Ipv6PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
{
    GT_U32  keyArr[NUM_WORDS_RULE_CNS];          /* TCAM key in hw format            */
    GT_U32  maskArr[NUM_WORDS_RULE_CNS];         /* TCAM mask in hw format           */
    /* TCAM action (rule) in hw format  */
    GT_U32  actionArr[PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS];
    GT_U32  validArr[5];        /* TCAM line valid bits             */
    GT_U32  compareModeArr[5];  /* TCAM line compare mode           */
    GT_U32  retVal = GT_OK;     /* function return code             */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_U32  i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routerTtiTcamRow >= fineTuningPtr->tableSize.router)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(prefixPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

     /* clear hw data */
    cpssOsMemSet(keyArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);
    cpssOsMemSet(maskArr, 0, sizeof(GT_U32) * NUM_WORDS_RULE_CNS);
    /* clear output data */
    *validPtr = GT_FALSE;
    cpssOsMemSet(prefixPtr, 0, sizeof(CPSS_DXCH_IPV6_PREFIX_STC));
    cpssOsMemSet(maskPtr, 0, sizeof(CPSS_DXCH_IPV6_PREFIX_STC));

    retVal = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                    routerTtiTcamRow,
                                                    &validArr[0],
                                                    &compareModeArr[0],
                                                    &keyArr[0],
                                                    &maskArr[0],
                                                    &actionArr[0]);
    if (retVal != GT_OK)
        return retVal;

    /* convert from HW format to ip address */
    /****************************************/

    prefixPtr->ipAddr.arIP[15] = (GT_U8)U32_GET_FIELD_MAC(keyArr[0], 0,8);
    prefixPtr->ipAddr.arIP[14] = (GT_U8)U32_GET_FIELD_MAC(keyArr[0], 8,8);
    prefixPtr->ipAddr.arIP[13] = (GT_U8)U32_GET_FIELD_MAC(keyArr[0], 16,8);
    prefixPtr->ipAddr.arIP[12] = (GT_U8)U32_GET_FIELD_MAC(keyArr[0], 24,8);

    prefixPtr->ipAddr.arIP[11] = (GT_U8)U32_GET_FIELD_MAC(keyArr[1], 0,8);
    prefixPtr->ipAddr.arIP[10] = (GT_U8)U32_GET_FIELD_MAC(keyArr[1], 8,8);
    prefixPtr->ipAddr.arIP[9]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[1], 16,8);
    prefixPtr->ipAddr.arIP[8]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[1], 24,8);

    prefixPtr->ipAddr.arIP[7]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[2], 0,8);
    prefixPtr->ipAddr.arIP[6]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[2], 8,8);
    prefixPtr->ipAddr.arIP[5]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[2], 16,8);
    prefixPtr->ipAddr.arIP[4]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[2], 24,8);

    prefixPtr->ipAddr.arIP[3]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[3], 0,8);
    prefixPtr->ipAddr.arIP[2]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[3], 8,8);
    prefixPtr->ipAddr.arIP[1]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[3], 16,8);
    prefixPtr->ipAddr.arIP[0]  = (GT_U8)U32_GET_FIELD_MAC(keyArr[3], 24,8);

    /* convert from HW format to mask */
    /**********************************/

    maskPtr->ipAddr.arIP[15]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[0], 0,8);
    maskPtr->ipAddr.arIP[14]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[0], 8,8);
    maskPtr->ipAddr.arIP[13]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[0], 16,8);
    maskPtr->ipAddr.arIP[12]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[0], 24,8);

    maskPtr->ipAddr.arIP[11]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[1], 0,8);
    maskPtr->ipAddr.arIP[10]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[1], 8,8);
    maskPtr->ipAddr.arIP[9]    = (GT_U8)U32_GET_FIELD_MAC(maskArr[1], 16,8);
    maskPtr->ipAddr.arIP[8]    = (GT_U8)U32_GET_FIELD_MAC(maskArr[1], 24,8);

    maskPtr->ipAddr.arIP[7]    = (GT_U8)U32_GET_FIELD_MAC(maskArr[2], 0,8);
    maskPtr->ipAddr.arIP[6]    = (GT_U8)U32_GET_FIELD_MAC(maskArr[2], 8,8);
    maskPtr->ipAddr.arIP[5]    = (GT_U8)U32_GET_FIELD_MAC(maskArr[2], 16,8);
    maskPtr->ipAddr.arIP[4]    = (GT_U8)U32_GET_FIELD_MAC(maskArr[2], 24,8);

    maskPtr->ipAddr.arIP[3]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[3], 0,8);
    maskPtr->ipAddr.arIP[2]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[3], 8,8);
    maskPtr->ipAddr.arIP[1]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[3], 16,8);
    maskPtr->ipAddr.arIP[0]   = (GT_U8)U32_GET_FIELD_MAC(maskArr[3], 24,8);

    /* get the vr Id or MC Group Index */
    /***********************************/

    prefixPtr->isMcSource = maskPtr->isMcSource =
        (((keyArr[4] >> 15) & 0x1) == 1) ? GT_TRUE : GT_FALSE;

    if (prefixPtr->isMcSource == GT_FALSE)
    {
        prefixPtr->vrId = (keyArr[4] & 0xfff);
        maskPtr->vrId   = (maskArr[4] & 0xfff);
    }
    else
    {
        prefixPtr->mcGroupIndexRow = (keyArr[4] & 0x7fff);
        maskPtr->mcGroupIndexRow   = (maskArr[4] & 0x7fff);
    }

    /* line holds valid IPv6 prefix if the following applies:
         - all entries are valid
         - the compare mode or all entries is row compare
         - keyArr[5] bit 31 must be 0 (to indicate IPv6 entry and not TT entry) */
    *validPtr = GT_TRUE;
    for (i = 0 ; i < 4 ; i++)
    {
        /* if entry is not valid or is single compare mode, whole line is not valid */
        if ((validArr[i] == 0) || (compareModeArr[i] == 0))
        {
            *validPtr = GT_FALSE;
            break;
        }
    }
    /* if whole line is valid, verify it is indeed IPv6 entry and not TTI entry */
    if ((*validPtr == GT_TRUE) && (((keyArr[5] >> 31) & 0x1) != 0))
    {
        *validPtr = GT_FALSE;
    }

    return retVal;
}


/**
* @internal internal_cpssDxChIpv6PrefixGet function
* @endinternal
*
* @brief   Gets an ipv6 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv6PrefixSet
*
*/
static GT_STATUS internal_cpssDxChIpv6PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxCh3Ipv6PrefixGet(devNum,routerTtiTcamRow,
                                     validPtr,prefixPtr,maskPtr);
}

/**
* @internal cpssDxChIpv6PrefixGet function
* @endinternal
*
* @brief   Gets an ipv6 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv6PrefixSet
*
*/
GT_STATUS cpssDxChIpv6PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpv6PrefixGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerTtiTcamRow, validPtr, prefixPtr, maskPtr));

    rc = internal_cpssDxChIpv6PrefixGet(devNum, routerTtiTcamRow, validPtr, prefixPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerTtiTcamRow, validPtr, prefixPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpv6PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv6 UC or MC prefix in the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to invalidate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that invalidating ipv6 address (that takes whole TCAM line) will
*       result invalidating all columns in that line.
*
*/
static GT_STATUS internal_cpssDxChIpv6PrefixInvalidate
(
    IN  GT_U8           devNum,
    IN  GT_U32          routerTtiTcamRow
)
{
    GT_U32  retVal = GT_OK;         /* function return code */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routerTtiTcamRow >= fineTuningPtr->tableSize.router)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    retVal = prvCpssDxChRouterTunnelTermTcamInvalidateLine(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,routerTtiTcamRow);

    return retVal;
}

/**
* @internal cpssDxChIpv6PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv6 UC or MC prefix in the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to invalidate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that invalidating ipv6 address (that takes whole TCAM line) will
*       result invalidating all columns in that line.
*
*/
GT_STATUS cpssDxChIpv6PrefixInvalidate
(
    IN  GT_U8           devNum,
    IN  GT_U32          routerTtiTcamRow
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpv6PrefixInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerTtiTcamRow));

    rc = internal_cpssDxChIpv6PrefixInvalidate(devNum, routerTtiTcamRow);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerTtiTcamRow));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttEntryPtr              - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active or
*                                       invalid ipv6MCGroupScopeLevel value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Lookup translation table is shared with the Tunnel Termination (TT)
*       Table.
*       If the relevant index in the Router Tcam is in line used for tunnel
*       termination key then the indexes in the LTT/TT table will function as
*       Tunnel Termination Action, and you shouldn't write LTT entries in these
*       indexes. It works Vice-verse.
*       For Ipv6 Entries the relevant index is the line prefix (same index sent
*       as parameter to the ipv6 prefix set API). The other LTT/TT entries
*       related to that TCAM line are left unused.
*       FEr#2018 - if the PCL action uses redirection to Router Lookup Translation Table (LTT)
*       (policy based routing) or if the TTI action uses redirection to LTT
*       (TTI base routing), then the LTT index written in the PCL/TTI action is
*       restricted only to column 0 of the LTT row.
*       NOTE: - Since LTT entries can be used also for Router TCAM routing, this API is not
*       affected directly by this errata. However when the LTT should be pointed from
*       PCL action or LTT action, it should be set at column 0.
*       - The API affected by this errata are PCL rule set, PCL action update,
*       TTI rule set and TTI action update.
*
*/
static GT_STATUS internal_cpssDxChIpLttWrite
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          lttTtiRow,
    IN  GT_U32                          lttTtiColumn,
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC      *lttEntryPtr
)
{
    GT_U32 lltEntryHwFormat = 0;
    GT_U32 lltEntryHwFormatLength=0;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    /* ltt size is the same as router TCAM size */
    if (lttTtiRow >= PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (lttTtiColumn >= 4)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    CPSS_NULL_PTR_CHECK_MAC(lttEntryPtr);

    /* check ltt parameters */
    switch (lttEntryPtr->routeType)
    {
    case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
    case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        if (lttEntryPtr->numOfPaths > CPSS_DXCH_IP_MAX_ECMP_QOS_GROUP)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        if(lttEntryPtr->routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E)
        {
            if (lttEntryPtr->numOfPaths > CPSS_DXCH_IP_MAX_LION_QOS_GROUP)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            if (lttEntryPtr->numOfPaths > CPSS_DXCH_IP_MAX_LION_ECMP_GROUP)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }
    if (lttEntryPtr->routeEntryBaseIndex >= fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch(lttEntryPtr->ipv6MCGroupScopeLevel)
    {
    case CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E:
    case CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E:
    case CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E:
    case CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* build the 22 bits of the llt entry */
    if(lttEntryPtr->routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E)
        lltEntryHwFormat = 0x1;

    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        lltEntryHwFormat |= (lttEntryPtr->numOfPaths & 0x7) << 1;
        lltEntryHwFormat |= (BOOL2BIT_MAC(lttEntryPtr->ucRPFCheckEnable)) << 4;
        lltEntryHwFormat |= (lttEntryPtr->ipv6MCGroupScopeLevel & 0x3) << 5;
        lltEntryHwFormat |= (BOOL2BIT_MAC(lttEntryPtr->sipSaCheckMismatchEnable)) << 7;
        lltEntryHwFormat |= lttEntryPtr->routeEntryBaseIndex << 8;
        lltEntryHwFormatLength   = 22;
    }
    else
    {
        lltEntryHwFormat |= (lttEntryPtr->numOfPaths & 0x3F) << 1;
        lltEntryHwFormat |= (BOOL2BIT_MAC(lttEntryPtr->ucRPFCheckEnable)) << 7;
        lltEntryHwFormat |= (lttEntryPtr->ipv6MCGroupScopeLevel & 0x3) << 8;
        lltEntryHwFormat |= (BOOL2BIT_MAC(lttEntryPtr->sipSaCheckMismatchEnable)) << 10;
        lltEntryHwFormat |= lttEntryPtr->routeEntryBaseIndex << 11;
        lltEntryHwFormatLength   = 25;
    }

    return prvCpssDxChWriteTableEntryField(devNum,CPSS_DXCH3_LTT_TT_ACTION_E,
                                                 lttTtiRow,lttTtiColumn,0,
                                                 lltEntryHwFormatLength,
                                                 lltEntryHwFormat);
}

/**
* @internal cpssDxChIpLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttEntryPtr              - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active or
*                                       invalid ipv6MCGroupScopeLevel value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Lookup translation table is shared with the Tunnel Termination (TT)
*       Table.
*       If the relevant index in the Router Tcam is in line used for tunnel
*       termination key then the indexes in the LTT/TT table will function as
*       Tunnel Termination Action, and you shouldn't write LTT entries in these
*       indexes. It works Vice-verse.
*       For Ipv6 Entries the relevant index is the line prefix (same index sent
*       as parameter to the ipv6 prefix set API). The other LTT/TT entries
*       related to that TCAM line are left unused.
*       FEr#2018 - if the PCL action uses redirection to Router Lookup Translation Table (LTT)
*       (policy based routing) or if the TTI action uses redirection to LTT
*       (TTI base routing), then the LTT index written in the PCL/TTI action is
*       restricted only to column 0 of the LTT row.
*       NOTE: - Since LTT entries can be used also for Router TCAM routing, this API is not
*       affected directly by this errata. However when the LTT should be pointed from
*       PCL action or LTT action, it should be set at column 0.
*       - The API affected by this errata are PCL rule set, PCL action update,
*       TTI rule set and TTI action update.
*
*/
GT_STATUS cpssDxChIpLttWrite
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          lttTtiRow,
    IN  GT_U32                          lttTtiColumn,
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC      *lttEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLttWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lttTtiRow, lttTtiColumn, lttEntryPtr));

    rc = internal_cpssDxChIpLttWrite(devNum, lttTtiRow, lttTtiColumn, lttEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lttTtiRow, lttTtiColumn, lttEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpLttRead function
* @endinternal
*
* @brief   Reads a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
*
* @param[out] lttEntryPtr              - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpLttWrite
*
*/
static GT_STATUS internal_cpssDxChIpLttRead
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             lttTtiRow,
    IN  GT_U32                             lttTtiColumn,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC         *lttEntryPtr
)
{
    GT_U32      data = 0;
    GT_STATUS   retVal = GT_OK;
    GT_U32      ipv6MCGroupScopeLevelOffset=0;
    GT_U32      lltEntryHwFormatLength=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* ltt size is the same as router TCAM size */
    if (lttTtiRow >= PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (lttTtiColumn >= 4)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    CPSS_NULL_PTR_CHECK_MAC(lttEntryPtr);

    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        lltEntryHwFormatLength = 22;
    }
    else
    {
        lltEntryHwFormatLength = 25;
    }

    retVal = prvCpssDxChReadTableEntryField(devNum,CPSS_DXCH3_LTT_TT_ACTION_E,
                                            lttTtiRow,lttTtiColumn,0,lltEntryHwFormatLength,&data);
    if(data & 0x1)
        lttEntryPtr->routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E;
    else
        lttEntryPtr->routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;

    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        lttEntryPtr->numOfPaths               = U32_GET_FIELD_MAC(data, 1,3);
        lttEntryPtr->ucRPFCheckEnable         = BOOL2BIT_MAC(U32_GET_FIELD_MAC(data, 4,1));
        lttEntryPtr->sipSaCheckMismatchEnable = BOOL2BIT_MAC(U32_GET_FIELD_MAC(data, 7,1));
        lttEntryPtr->routeEntryBaseIndex      = U32_GET_FIELD_MAC(data, 8,13);
        ipv6MCGroupScopeLevelOffset           = 5;
    }
    else
    {
        lttEntryPtr->numOfPaths               = U32_GET_FIELD_MAC(data, 1,6);
        lttEntryPtr->ucRPFCheckEnable         = BOOL2BIT_MAC(U32_GET_FIELD_MAC(data, 7,1));
        lttEntryPtr->sipSaCheckMismatchEnable = BOOL2BIT_MAC(U32_GET_FIELD_MAC(data, 10,1));
        lttEntryPtr->routeEntryBaseIndex      = U32_GET_FIELD_MAC(data,11,13);
        ipv6MCGroupScopeLevelOffset           = 8;
    }

    switch(U32_GET_FIELD_MAC(data,ipv6MCGroupScopeLevelOffset,2))
    {
    case 0:
        lttEntryPtr->ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        break;
    case 1:
        lttEntryPtr->ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        break;
    case 2:
        lttEntryPtr->ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        break;
    case 3:
        lttEntryPtr->ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        break;
    default:
        break;
    }

    return (retVal);
}

/**
* @internal cpssDxChIpLttRead function
* @endinternal
*
* @brief   Reads a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
*
* @param[out] lttEntryPtr              - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpLttWrite
*
*/
GT_STATUS cpssDxChIpLttRead
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             lttTtiRow,
    IN  GT_U32                             lttTtiColumn,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC         *lttEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLttRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lttTtiRow, lttTtiColumn, lttEntryPtr));

    rc = internal_cpssDxChIpLttRead(devNum, lttTtiRow, lttTtiColumn, lttEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lttTtiRow, lttTtiColumn, lttEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpUcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to write the array
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to write)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case uRPF ECMP/QOS is globally enabled, then for ECMP/QOS block
*       with x route entries, additional route entry should be included after
*       the block with the uRPF route entry information.
*
*/
static GT_STATUS internal_cpssDxChIpUcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          baseRouteEntryIndex,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                          numOfRouteEntries
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 i;
    GT_U32 hwData[4];
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    CPSS_DXCH_TABLE_ENT tableType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(routeEntriesArray);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (baseRouteEntryIndex >= fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    if((baseRouteEntryIndex + numOfRouteEntries) > fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    retVal = prvCpssDxChIpUcRouteEntriesCheck(devNum,routeEntriesArray,numOfRouteEntries);
    if(retVal != GT_OK)
        return retVal;

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E;
    }
    else
    {
        tableType = CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E;
    }
    for (i = 0; i < numOfRouteEntries ; i++)
    {
        /* Convert route entry to HW format */
        /* currently only regular route entry is supported */
        retVal = prvCpssDxChIpConvertUcEntry2HwFormat(devNum,&routeEntriesArray[i],hwData);
        if(retVal != GT_OK)
            return retVal;

        /* Write to hw table */
        retVal =
            prvCpssDxChWriteTableEntry(devNum,
                                       tableType,
                                       baseRouteEntryIndex + i,
                                       hwData);
        if(retVal != GT_OK)
            return retVal;
    }

    return retVal;
}

/**
* @internal cpssDxChIpUcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to write the array
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to write)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case uRPF ECMP/QOS is globally enabled, then for ECMP/QOS block
*       with x route entries, additional route entry should be included after
*       the block with the uRPF route entry information.
*
*/
GT_STATUS cpssDxChIpUcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          baseRouteEntryIndex,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntriesArray[],
    IN GT_U32                          numOfRouteEntries
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpUcRouteEntriesWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseRouteEntryIndex, routeEntriesArray, numOfRouteEntries));

    rc = internal_cpssDxChIpUcRouteEntriesWrite(devNum, baseRouteEntryIndex, routeEntriesArray, numOfRouteEntries);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseRouteEntryIndex, routeEntriesArray, numOfRouteEntries));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpUcRouteEntriesRead function
* @endinternal
*
* @brief   Reads an array of uc route entries from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to start reading
* @param[in,out] routeEntriesArray        - the uc route entries array, for each element
*                                      set the uc route entry type to determine how
*                                      entry data will be interpreted
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to read)
* @param[in,out] routeEntriesArray        - the uc route entries array read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpUcRouteEntriesWrite
*
*/
static GT_STATUS internal_cpssDxChIpUcRouteEntriesRead
(
    IN    GT_U8                             devNum,
    IN    GT_U32                            baseRouteEntryIndex,
    INOUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC   *routeEntriesArray,
    IN    GT_U32                            numOfRouteEntries
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 i;
    GT_U32 hwData[4];
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    CPSS_DXCH_TABLE_ENT tableType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(routeEntriesArray);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (baseRouteEntryIndex >= fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    if((baseRouteEntryIndex + numOfRouteEntries) > fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E;
    }
    else
    {
        tableType = CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E;
    }
    /* Convert route entry to HW format */
    for (i = 0; i < numOfRouteEntries ; i++)
    {
        retVal =
            prvCpssDxChReadTableEntry(devNum,
                                      tableType,
                                      baseRouteEntryIndex + i,
                                      hwData);

        if(retVal != GT_OK)
            return retVal;

        retVal =
            prvCpssDxChIpConvertHwFormat2UcEntry(devNum,hwData,&routeEntriesArray[i]);

        if(retVal != GT_OK)
            return retVal;

    }

    return GT_OK;
}

/**
* @internal cpssDxChIpUcRouteEntriesRead function
* @endinternal
*
* @brief   Reads an array of uc route entries from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to start reading
* @param[in,out] routeEntriesArray        - the uc route entries array, for each element
*                                      set the uc route entry type to determine how
*                                      entry data will be interpreted
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to read)
* @param[in,out] routeEntriesArray        - the uc route entries array read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpUcRouteEntriesWrite
*
*/
GT_STATUS cpssDxChIpUcRouteEntriesRead
(
    IN    GT_U8                             devNum,
    IN    GT_U32                            baseRouteEntryIndex,
    INOUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC   *routeEntriesArray,
    IN    GT_U32                            numOfRouteEntries
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpUcRouteEntriesRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseRouteEntryIndex, routeEntriesArray, numOfRouteEntries));

    rc = internal_cpssDxChIpUcRouteEntriesRead(devNum, baseRouteEntryIndex, routeEntriesArray, numOfRouteEntries);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseRouteEntryIndex, routeEntriesArray, numOfRouteEntries));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpMcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes a MC route entry to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntryIndex          - the Index in the Route entries table
* @param[in] routeEntryPtr            - the MC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChIpMcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          routeEntryIndex,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
{
    GT_U32 hwData[4];
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    CPSS_DXCH_TABLE_ENT tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(routeEntryPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routeEntryIndex >= fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    retVal = prvCpssDxChIpMcRouteEntryCheck(devNum,routeEntryPtr);
    if(retVal != GT_OK)
        return retVal;

    /* Convert route entry to HW format */
    retVal = prvCpssDxChIpConvertMcEntry2HwFormat(devNum,routeEntryPtr,hwData);
    if(retVal != GT_OK)
        return retVal;

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E;
    }
    else
    {
        tableType = CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E;
    }

    /* Write to hw table */
    retVal = prvCpssDxChWriteTableEntry(devNum,
                                        tableType,
                                        routeEntryIndex,
                                        hwData);
    return retVal;
}

/**
* @internal cpssDxChIpMcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes a MC route entry to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntryIndex          - the Index in the Route entries table
* @param[in] routeEntryPtr            - the MC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          routeEntryIndex,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpMcRouteEntriesWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routeEntryIndex, routeEntryPtr));

    rc = internal_cpssDxChIpMcRouteEntriesWrite(devNum, routeEntryIndex, routeEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routeEntryIndex, routeEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpMcRouteEntriesRead function
* @endinternal
*
* @brief   Reads a MC route entry from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntryIndex          - the route entry index.
*
* @param[out] routeEntryPtr            - the MC route entry read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChIpMcRouteEntriesRead
(
    IN GT_U8                            devNum,
    IN GT_U32                           routeEntryIndex,
    OUT CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
{
    GT_U32 hwData[4];
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    CPSS_DXCH_TABLE_ENT tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(routeEntryPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (routeEntryIndex >= fineTuningPtr->tableSize.routerNextHop)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E;
    }
    else
    {
        tableType = CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E;
    }

    /* Write to hw table */
    retVal = prvCpssDxChReadTableEntry(devNum,
                                       tableType,
                                       routeEntryIndex,
                                       hwData);
    if(retVal != GT_OK)
        return retVal;

    /* Convert route entry to HW format */
    retVal = prvCpssDxChIpConvertHwFormat2McEntry(devNum,routeEntryPtr,hwData);

    return retVal;
}

/**
* @internal cpssDxChIpMcRouteEntriesRead function
* @endinternal
*
* @brief   Reads a MC route entry from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntryIndex          - the route entry index.
*
* @param[out] routeEntryPtr            - the MC route entry read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMcRouteEntriesRead
(
    IN GT_U8                            devNum,
    IN GT_U32                           routeEntryIndex,
    OUT CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpMcRouteEntriesRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routeEntryIndex, routeEntryPtr));

    rc = internal_cpssDxChIpMcRouteEntriesRead(devNum, routeEntryIndex, routeEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routeEntryIndex, routeEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpRouterNextHopTableAgeBitsEntryWrite function
* @endinternal
*
* @brief   set router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routerNextHopTableAgeBitsEntry - the router next hop table age bits
*                                      entry index. each entry is 32 Age
*                                      bits.
* @param[in] routerNextHopTableAgeBitsEntry - a 32 Age Bit map for route entries
*                                      <32routeEntriesIndexesArrayPtr>..
*                                      <32routeEntriesIndexesArrayPtr+31>
*                                      bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChIpRouterNextHopTableAgeBitsEntryWrite
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    IN  GT_U32  routerNextHopTableAgeBitsEntry
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   retVal = GT_OK;
    GT_U32      hwStep;         /* hw index step */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    /* the number of age bits is same as the number of route */
    /* entries, and each age bits entry holds 32 age bits    */
    if (routerNextHopTableAgeBitsEntryIndex >= fineTuningPtr->tableSize.routerNextHop / 32)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        retVal = prvCpssDxChWriteTableEntry(devNum,
                                            CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E,
                                            routerNextHopTableAgeBitsEntryIndex,
                                            &routerNextHopTableAgeBitsEntry);
    }
    else
    {
        hwStep = 0x04;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ucMcRouteEntriesAgeBitsBase +
                  routerNextHopTableAgeBitsEntryIndex * hwStep;

        retVal = prvCpssHwPpWriteRegister(devNum, regAddr, routerNextHopTableAgeBitsEntry);
    }

    return retVal;
}

/**
* @internal cpssDxChIpRouterNextHopTableAgeBitsEntryWrite function
* @endinternal
*
* @brief   set router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routerNextHopTableAgeBitsEntry - the router next hop table age bits
*                                      entry index. each entry is 32 Age
*                                      bits.
* @param[in] routerNextHopTableAgeBitsEntry - a 32 Age Bit map for route entries
*                                      <32routeEntriesIndexesArrayPtr>..
*                                      <32routeEntriesIndexesArrayPtr+31>
*                                      bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterNextHopTableAgeBitsEntryWrite
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    IN  GT_U32  routerNextHopTableAgeBitsEntry
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpRouterNextHopTableAgeBitsEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerNextHopTableAgeBitsEntryIndex, routerNextHopTableAgeBitsEntry));

    rc = internal_cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(devNum, routerNextHopTableAgeBitsEntryIndex, routerNextHopTableAgeBitsEntry);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerNextHopTableAgeBitsEntryIndex, routerNextHopTableAgeBitsEntry));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpRouterNextHopTableAgeBitsEntryRead function
* @endinternal
*
* @brief   read router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routerNextHopTableAgeBitsEntryIndex - the router next hop table age bits
*                                      entry index. each entry is 32 Age
*                                      bits.
*
* @param[out] routerNextHopTableAgeBitsEntryPtr - a 32 Age Bit map for route entries
*                                      <32routeEntriesIndexesArrayPtr>..
*                                      <32routeEntriesIndexesArrayPtr+31>
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChIpRouterNextHopTableAgeBitsEntryRead
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    OUT GT_U32  *routerNextHopTableAgeBitsEntryPtr
)
{
    GT_U32      regAddr = 0;        /* register address */
    GT_STATUS   retVal = GT_OK;
    GT_U32      hwStep;         /* hw index step */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_U32      portGroupId;    /* port group Id */
    GT_U32      tempRouterNextHopTableAgeBitsEntry=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(routerNextHopTableAgeBitsEntryPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    /* the number of age bits is same as the number of route */
    /* entries, and each age bits entry holds 32 age bits    */
    if (routerNextHopTableAgeBitsEntryIndex >= fineTuningPtr->tableSize.routerNextHop / 32)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    *routerNextHopTableAgeBitsEntryPtr=0;

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        hwStep = 0x04;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ucMcRouteEntriesAgeBitsBase +
                  routerNextHopTableAgeBitsEntryIndex * hwStep;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            retVal = prvCpssDxChPortGroupReadTableEntry(devNum, portGroupId,
                                                        CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E,
                                                        routerNextHopTableAgeBitsEntryIndex,
                                                        &tempRouterNextHopTableAgeBitsEntry);
        }
        else
        {
            retVal = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                         &tempRouterNextHopTableAgeBitsEntry);
        }
        if (retVal != GT_OK)
            return retVal;

        (*routerNextHopTableAgeBitsEntryPtr) |= tempRouterNextHopTableAgeBitsEntry;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return retVal;
}

/**
* @internal cpssDxChIpRouterNextHopTableAgeBitsEntryRead function
* @endinternal
*
* @brief   read router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routerNextHopTableAgeBitsEntryIndex - the router next hop table age bits
*                                      entry index. each entry is 32 Age
*                                      bits.
*
* @param[out] routerNextHopTableAgeBitsEntryPtr - a 32 Age Bit map for route entries
*                                      <32routeEntriesIndexesArrayPtr>..
*                                      <32routeEntriesIndexesArrayPtr+31>
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterNextHopTableAgeBitsEntryRead
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    OUT GT_U32  *routerNextHopTableAgeBitsEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpRouterNextHopTableAgeBitsEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerNextHopTableAgeBitsEntryIndex, routerNextHopTableAgeBitsEntryPtr));

    rc = internal_cpssDxChIpRouterNextHopTableAgeBitsEntryRead(devNum, routerNextHopTableAgeBitsEntryIndex, routerNextHopTableAgeBitsEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerNextHopTableAgeBitsEntryIndex, routerNextHopTableAgeBitsEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpMLLPairWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) pair entry to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the Mc lisk list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllPairWriteForm.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note MLL entries are two words long.
*       In Lion2 and above devices the whole entry is written when last word is
*       set.
*       In PP prior to lion The atomic hw write operation is done on one word.
*       Writing order of MLL parameters could be significant as some parameters
*       in word[1],[3] depend on parameters in word[0],[2].
*       Erroneous handling of mllRPFFailCommand/ nextHopTunnelPointer may
*       result with bad traffic. nextPointer may result with PP infinite loop.
*       cpssDxChIpMLLPairWrite handles the nextPointer parameter in the
*       following way:
*       if (last == 1) first set word[0] or word[2].
*       if (last == 0) first set word[3].
*
*/
static GT_STATUS internal_cpssDxChIpMLLPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_IP_MLL_PAIR_STC                    *mllPairEntryPtr
)
{
    GT_U32    hwAddr;
    GT_U32    hwData[CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD];
    GT_U32    hwMask[CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD];
    GT_U32    sip5HwData[BITS_TO_WORDS_MAC(MAX_IP_MLL_ENTRY_BITS_SIZE_CNS)];
    GT_U32    sip5HwMask[BITS_TO_WORDS_MAC(MAX_IP_MLL_ENTRY_BITS_SIZE_CNS)] = {0};
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_BOOL   isWriteOnLastWord = GT_FALSE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mllPairEntryPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* update the HW entry. Use prvCpssDxChWriteTableEntryMasked instead of
           "prvCpssDxChReadTableEntry, update it, prvCpssDxChWriteTableEntry.
           This allows update only necessary fields in Shadow DB table instead of
           overwriting full entry from HW. It prevents propagation of weak bit(s)
           from HW to Shadow. */
        retVal = sip5IpMllStruct2HwFormat(devNum,mllPairWriteForm,mllPairEntryPtr,
                                          sip5HwData, sip5HwMask);
        if(retVal != GT_OK)
        {
            return retVal;
        }

        retVal = prvCpssDxChWriteTableEntryMasked(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
                                                  mllPairEntryIndex,
                                                  sip5HwData, sip5HwMask);
        return retVal;
    }

    /*in here we need to add all devices where entry is written on last word*/
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        isWriteOnLastWord = GT_TRUE;
    }

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        switch(mllPairEntryPtr->firstMllNode.mllRPFFailCommand)
        {
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            case CPSS_PACKET_CMD_DROP_HARD_E:
            case CPSS_PACKET_CMD_DROP_SOFT_E:
            case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            case CPSS_PACKET_CMD_BRIDGE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        switch(mllPairEntryPtr->secondMllNode.mllRPFFailCommand)
        {
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            case CPSS_PACKET_CMD_DROP_HARD_E:
            case CPSS_PACKET_CMD_DROP_SOFT_E:
            case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            case CPSS_PACKET_CMD_BRIDGE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllTableBase +
             mllPairEntryIndex * 0x10;

    /*in case we are writing either first or second mll we need to read word 1 for tunnel start ptr*/
    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E))
    {
        retVal = prvCpssHwPpReadRam(devNum,
                                       hwAddr + 4,
                                       CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                       &hwData[1]);
    }

    retVal = prvCpssDxChIpMllStruct2HwFormat(mllPairWriteForm,mllPairEntryPtr,hwData);
    if(retVal != GT_OK)
        return retVal;


    cpssOsMemSet(hwMask, 0, sizeof(hwMask));
    if (isWriteOnLastWord)
    {
        switch(mllPairWriteForm)
        {
        case CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E:
            /*we need to modify only the first mll and we read the last word
              so it will not change*/
            retVal = prvCpssHwPpReadRam(devNum,
                                           hwAddr + 12,
                                           CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                           &hwData[3]);

            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/2,
                                            &hwData[0]);

            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 12,
                                            CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[3]);
            hwMask[0] = hwMask[3] = 0xFFFFFFFF;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            /*we need to write word 1 for second mll ts bits*/
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 4,
                                            CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[1]);

            /*we are writing both words and overwriting the last word*/
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 8,
                                            CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/2,
                                            &hwData[2]);
            hwMask[1] = hwMask[2] = 0xFFFFFFFF;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD,
                                            &hwData[0]);
            cpssOsMemSet(hwMask, 1, sizeof(hwMask));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch(mllPairWriteForm)
        {
        case CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E:
            if(mllPairEntryPtr->firstMllNode.last)
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/2,
                                                &hwData[0]);
            }
            else
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 4,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[1]);
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[0]);
            }
            hwMask[0] = hwMask[1] = 0xFFFFFFFF;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            if(mllPairEntryPtr->secondMllNode.last)
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 8,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/2,
                                                &hwData[2]);
            }
            else
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 12,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[3]);
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 8,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[2]);
            }
            /*we need to write word 1 anyhow for tunnel ptr bits*/
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 4,
                                            CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[1]);
            hwMask[2] = hwMask[3] = 0xFFFFFFFF;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
            if(mllPairEntryPtr->firstMllNode.last)
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD,
                                                &hwData[0]);
            }
            else if(mllPairEntryPtr->secondMllNode.last)
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 8,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/2,
                                                &hwData[2]);
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/2,
                                                &hwData[0]);
            }
            else
            {
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 12,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[3]);
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 8,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[2]);
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr + 4,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[1]);
                retVal = prvCpssHwPpWriteRam(devNum,
                                                hwAddr,
                                                CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD/4,
                                                &hwData[0]);
            }
            cpssOsMemSet(hwMask, 1, sizeof(hwMask));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(retVal != GT_OK)
    {
        return retVal;
    }

    return prvCpssDxChPortGroupShadowLineUpdateMasked(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_XCAT_TABLE_MLL_E, mllPairEntryIndex, &hwData[0], &hwMask[0]);
}

/**
* @internal cpssDxChIpMLLPairWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) pair entry to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the Mc lisk list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllPairWriteForm.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note MLL entries are two words long.
*       In Lion2 and above devices the whole entry is written when last word is
*       set.
*       In PP prior to lion The atomic hw write operation is done on one word.
*       Writing order of MLL parameters could be significant as some parameters
*       in word[1],[3] depend on parameters in word[0],[2].
*       Erroneous handling of mllRPFFailCommand/ nextHopTunnelPointer may
*       result with bad traffic. nextPointer may result with PP infinite loop.
*       cpssDxChIpMLLPairWrite handles the nextPointer parameter in the
*       following way:
*       if (last == 1) first set word[0] or word[2].
*       if (last == 0) first set word[3].
*
*/
GT_STATUS cpssDxChIpMLLPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_IP_MLL_PAIR_STC                    *mllPairEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpMLLPairWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllPairEntryIndex, mllPairWriteForm, mllPairEntryPtr));

    rc = internal_cpssDxChIpMLLPairWrite(devNum, mllPairEntryIndex, mllPairWriteForm, mllPairEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllPairEntryIndex, mllPairWriteForm, mllPairEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpMLLPairRead function
* @endinternal
*
* @brief   Read a Mc Link List (MLL) pair entry from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the Mc lisk list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChIpMLLPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairReadForm,
    OUT CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr
)
{
    GT_U32    hwAddr;
    GT_U32    hwData[CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD];
    GT_U32    sip5HwData[BITS_TO_WORDS_MAC(MAX_IP_MLL_ENTRY_BITS_SIZE_CNS)];
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mllPairEntryPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        retVal = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
                                        mllPairEntryIndex,
                                        sip5HwData);
        if(retVal != GT_OK)
        {
            return retVal;
        }

        /* convert HW entry to SW format*/
        retVal = sip5IpMllHw2StructFormat(devNum,mllPairReadForm,mllPairEntryPtr,sip5HwData);
        return retVal;
    }

    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllTableBase +
             mllPairEntryIndex * 0x10;

    retVal = prvCpssHwPpReadRam(devNum, hwAddr,
                                    CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD, hwData);
    if(retVal != GT_OK)
        return retVal;

    /* read generic data */
    switch(mllPairReadForm)
    {
         case CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E:
            retVal = prvCpssDxChIpHwFormat2MllStruct(&hwData[0], GT_TRUE, &mllPairEntryPtr->firstMllNode);
            if(retVal != GT_OK)
                return retVal;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            retVal = prvCpssDxChIpHwFormat2MllStruct(&hwData[0], GT_FALSE, &mllPairEntryPtr->secondMllNode);
            if(retVal != GT_OK)
                return retVal;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
            retVal = prvCpssDxChIpHwFormat2MllStruct(&hwData[0], GT_TRUE, &mllPairEntryPtr->firstMllNode);
            if(retVal != GT_OK)
                return retVal;

            retVal = prvCpssDxChIpHwFormat2MllStruct(&hwData[0], GT_FALSE, &mllPairEntryPtr->secondMllNode);
            if(retVal != GT_OK)
                return retVal;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((mllPairReadForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E) ||
        (mllPairReadForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        /* Next MLL Pointer */
        mllPairEntryPtr->nextPointer = (GT_U16)U32_GET_FIELD_MAC(hwData[3], 20, 12);
    }

    return retVal;
}

/**
* @internal cpssDxChIpMLLPairRead function
* @endinternal
*
* @brief   Read a Mc Link List (MLL) pair entry from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the Mc lisk list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMLLPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairReadForm,
    OUT CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpMLLPairRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllPairEntryIndex, mllPairReadForm, mllPairEntryPtr));

    rc = internal_cpssDxChIpMLLPairRead(devNum, mllPairEntryIndex, mllPairReadForm, mllPairEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllPairEntryIndex, mllPairReadForm, mllPairEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpMLLLastBitWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) Last bit to the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] lastBit                  - the Last bit
* @param[in] mllEntryPart             - to which part of the mll Pair to write the Last bit
*                                      for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllEntryPart.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChIpMLLLastBitWrite
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllPairEntryIndex,
    IN GT_BOOL                                          lastBit,
    IN CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT    mllEntryPart
)
{
    GT_U32      hwAddr;
    GT_U32      hwData[CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD];
    GT_STATUS   retVal = GT_OK;
    GT_U32      hwWord;
    GT_U32      offsetInWord;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(mllEntryPart)
        {
            case CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E:
            case CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
                                        mllPairEntryIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        (mllEntryPart == CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E) ?
                                            SIP5_IP_MLL_TABLE_FIELDS_LAST_1_E :
                                            SIP5_IP_MLL_TABLE_FIELDS_LAST_0_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        BOOL2BIT_MAC(lastBit));
        return retVal;
    }

    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllTableBase +
             mllPairEntryIndex * 0x10;

    /* here we need to add all devices where entry write is triggered upon writing last word */
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        /* read the table entry from hardware */
        retVal = prvCpssHwPpReadRam(devNum,hwAddr,CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD,hwData);
        if (retVal != GT_OK)
            return retVal;

        switch(mllEntryPart)
        {
        case CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E:
            hwWord = 0;
            offsetInWord = 0;
            break;
        case CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E:
            hwWord = 2;
            offsetInWord = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwData[hwWord] &= (~(1 << offsetInWord));
        hwData[hwWord] |= (BOOL2BIT_MAC(lastBit) << offsetInWord);

        /* write the updated entry to the hardware */
        retVal = prvCpssHwPpWriteRam(devNum,hwAddr,CPSS_DXCH_SIZE_OF_MLL_NODE_IN_WORD,hwData);
        if (retVal != GT_OK)
            return retVal;
    }
    else
    {
        switch(mllEntryPart)
        {
        case CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E:
            retVal = prvCpssHwPpSetRegField(devNum,hwAddr,0,1,BOOL2BIT_MAC(lastBit));
            break;
        case CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E:
            retVal = prvCpssHwPpSetRegField(devNum,hwAddr+8,0,1,BOOL2BIT_MAC(lastBit));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return retVal;
}

/**
* @internal cpssDxChIpMLLLastBitWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) Last bit to the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] lastBit                  - the Last bit
* @param[in] mllEntryPart             - to which part of the mll Pair to write the Last bit
*                                      for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllEntryPart.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMLLLastBitWrite
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllPairEntryIndex,
    IN GT_BOOL                                          lastBit,
    IN CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT    mllEntryPart
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpMLLLastBitWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllPairEntryIndex, lastBit, mllEntryPart));

    rc = internal_cpssDxChIpMLLLastBitWrite(devNum, mllPairEntryIndex, lastBit, mllEntryPart);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllPairEntryIndex, lastBit, mllEntryPart));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpRouterArpAddrWrite function
* @endinternal
*
* @brief   write a ARP MAC address to the router ARP / Tunnel start / NAT table (for NAT capable devices)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - the device number
* @param[in] entryIndex            - The Arp Address index
* @param[in] arpMacAddrPtr         - the ARP MAC address to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel
*       start entries table and router ARP addresses table
*       reside at the same physical memory. See also
*       cpssDxChTunnelStartEntrySet and cpssDxChIpNatEntrySet
*
*       For xCat3; AC5; Lion2; devices, Each line can hold:
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       Tunnel start entry at index n and router ARP addresses at indexes 4n..4n+3 share the same memory.
*       For example NAT entry/tunnel start entry at index 100
*       and router ARP addresses at indexes 400..403 share the
*       same physical memory.
*
*       For Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman devices, Each line can hold:
*       - 2 NAT entries
*       - 1 CPSS_TUNNEL_GENERIC_E or CPSS_TUNNEL_GENERIC_IPV6_E tunnel start entry type
*       - 2 other tunnel start entries
*       - 8 router ARP addresses entries
*/
static GT_STATUS internal_cpssDxChIpRouterArpAddrWrite
(
    IN GT_U8                         devNum,
    IN GT_U32                        entryIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
)
{
    GT_U32  hwData[2];
    GT_U32  retVal;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(arpMacAddrPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (entryIndex >= fineTuningPtr->tableSize.routerArp)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    /* convert the ARP MAC address to hw format */
    hwData[0] =  arpMacAddrPtr->arEther[5] |
                 (arpMacAddrPtr->arEther[4] << 8) |
                 (arpMacAddrPtr->arEther[3] << 16)|
                 (arpMacAddrPtr->arEther[2] << 24);
    hwData[1] =  arpMacAddrPtr->arEther[1] |
                 (arpMacAddrPtr->arEther[0] << 8);


    retVal = prvCpssDxChHwHaTunnelStartArpNatTableEntryWrite(devNum,
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ARP_E,
                entryIndex,
                hwData);

    return retVal;
}

/**
* @internal cpssDxChIpRouterArpAddrWrite function
* @endinternal
*
* @brief   write a ARP MAC address to the router ARP / Tunnel start / NAT table (for NAT capable devices)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - the device number
* @param[in] entryIndex            - The Arp Address index
* @param[in] arpMacAddrPtr         - the ARP MAC address to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       See also cpssDxChTunnelStartEntrySet and cpssDxChIpNatEntrySet
*
*       For xCat3; AC5; Lion2; devices, Each line can hold:
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       Tunnel start entry at index n and router ARP addresses at indexes 4n..4n+3 share the same memory.
*       For example NAT entry/tunnel start entry at index 100
*       and router ARP addresses at indexes 400..403 share the
*       same physical memory.
*
*       For Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman devices, Each line can hold:
*       - 2 NAT entries
*       - 1 CPSS_TUNNEL_GENERIC_E or CPSS_TUNNEL_GENERIC_IPV6_E tunnel start entry type
*       - 2 other tunnel start entries
*       - 8 router ARP addresses entries
*/
GT_STATUS cpssDxChIpRouterArpAddrWrite
(
    IN GT_U8                         devNum,
    IN GT_U32                        entryIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpRouterArpAddrWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, arpMacAddrPtr));

    rc = internal_cpssDxChIpRouterArpAddrWrite(devNum, entryIndex, arpMacAddrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, arpMacAddrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpRouterArpAddrRead function
* @endinternal
*
* @brief   read a ARP MAC address from the router ARP / Tunnel start / NAT table (for NAT capable devices)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - the device number
* @param[in] entryIndex            - The Arp Address index
*
* @param[out] arpMacAddrPtr         - the ARP MAC addresses
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_OUT_OF_RANGE          - on entryIndex bigger then 4095 or
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       See also cpssDxChTunnelStartEntrySet and cpssDxChIpNatEntrySet
*
*       For xCat3; AC5; Lion2; devices, Each line can hold:
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       Tunnel start entry at index n and router ARP addresses at indexes 4n..4n+3 share the same memory.
*       For example NAT entry/tunnel start entry at index 100
*       and router ARP addresses at indexes 400..403 share the
*       same physical memory.
*
*       For Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman devices, Each line can hold:
*       - 2 NAT entries
*       - 1 CPSS_TUNNEL_GENERIC_E or CPSS_TUNNEL_GENERIC_IPV6_E tunnel start entry type
*       - 2 other tunnel start entries
*       - 8 router ARP addresses entries
*/
static GT_STATUS internal_cpssDxChIpRouterArpAddrRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       entryIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
)
{
    GT_U32  hwData[2];
    GT_U32  retVal;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(arpMacAddrPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if (entryIndex >= fineTuningPtr->tableSize.routerArp)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    retVal = prvCpssDxChHwHaTunnelStartArpNatTableEntryRead(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ARP_E,
                                        entryIndex,
                                        hwData);

    arpMacAddrPtr->arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwData[0],0,8);
    arpMacAddrPtr->arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwData[0],8,8);
    arpMacAddrPtr->arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwData[0],16,8);
    arpMacAddrPtr->arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwData[0],24,8);

    arpMacAddrPtr->arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwData[1],0,8);
    arpMacAddrPtr->arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwData[1],8,8);

    return retVal;
}

/**
* @internal cpssDxChIpRouterArpAddrRead function
* @endinternal
*
* @brief   read a ARP MAC address from the router ARP / Tunnel start / NAT table (for NAT capable devices)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - the device number
* @param[in] entryIndex            - The Arp Address index
*
* @param[out] arpMacAddrPtr         - the ARP MAC addresses
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_OUT_OF_RANGE          - on entryIndex bigger then 4095 or
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel
*       start entries table and router ARP addresses table
*       reside at the same physical memory. See also
*       cpssDxChTunnelStartEntrySet and cpssDxChIpNatEntrySet
*
*       For xCat3; AC5; Lion2; devices, Each line can hold:
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       Tunnel start entry at index n and router ARP addresses at indexes 4n..4n+3 share the same memory.
*       For example NAT entry/tunnel start entry at index 100
*       and router ARP addresses at indexes 400..403 share the
*       same physical memory.
*
*       For Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman devices, Each line can hold:
*       - 2 NAT entries
*       - 1 CPSS_TUNNEL_GENERIC_E or CPSS_TUNNEL_GENERIC_IPV6_E tunnel start entry type
*       - 2 other tunnel start entries
*       - 8 router ARP addresses entries
*/
GT_STATUS cpssDxChIpRouterArpAddrRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       entryIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpRouterArpAddrRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, arpMacAddrPtr));

    rc = internal_cpssDxChIpRouterArpAddrRead(devNum, entryIndex, arpMacAddrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, arpMacAddrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChIpConvertUcEntry2HwFormat function
* @endinternal
*
* @brief   This function converts a given ip uc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] ipUcRouteEntryPtr        - The entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertUcEntry2HwFormat
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *ipUcRouteEntryPtr,
    OUT GT_U32 *hwDataPtr
)
{
    GT_U8         packetCmd;        /* packet command */
    GT_U8         cpuCodeIdx;       /* cpu code index */
    GT_U8         modifyDscp;       /* modify packet Dscp command */
    GT_U8         modifyUp;         /* modify packet user priority field */
    GT_HW_DEV_NUM targHwDev;        /* target HW device */
    GT_PORT_NUM   targPort;         /* target port   */
    GT_U8         counterSet;
    GT_U32        value;            /* used for sip5 macro */
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC  *routeEntry = NULL;
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT  muxMode;
    GT_STATUS     rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* clear hw data */
    cpssOsMemSet(hwDataPtr, 0, sizeof(GT_U32) * 4);

    switch (ipUcRouteEntryPtr->type)
    {
    case CPSS_DXCH_IP_UC_ROUTE_ENTRY_E:

        routeEntry = &ipUcRouteEntryPtr->entry.regularEntry;   /* to make the code more readable */

        if (routeEntry->ttlHopLimitDecEnable && routeEntry->ttlHopLimDecOptionsExtChkByPass)
        {
            /* It is not legal to configure bypass the TTL/Hop-Limit Check and to enable the
            TTL/Hop-Limit to be decremented. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(routeEntry->cmd)
        {
            case CPSS_PACKET_CMD_ROUTE_E:
                packetCmd= 0;
                break;
            case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
                packetCmd= 1;
                break;
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                packetCmd= 2;
                break;
            case CPSS_PACKET_CMD_DROP_HARD_E:
                packetCmd= 3;
                break;
            case CPSS_PACKET_CMD_DROP_SOFT_E:
                packetCmd= 4;
                break;
            case CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E:
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    packetCmd = 7;
                    break;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(routeEntry->cpuCodeIdx)
        {
            case CPSS_DXCH_IP_CPU_CODE_IDX_0_E:
                cpuCodeIdx= 0;
                break;
            case CPSS_DXCH_IP_CPU_CODE_IDX_1_E:
                cpuCodeIdx= 1;
                break;
            case CPSS_DXCH_IP_CPU_CODE_IDX_2_E:
                cpuCodeIdx= 2;
                break;
            case CPSS_DXCH_IP_CPU_CODE_IDX_3_E:
                cpuCodeIdx= 3;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(routeEntry->modifyDscp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
                modifyDscp= 0;
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                modifyDscp= 2;
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                modifyDscp= 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(routeEntry->modifyUp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
                modifyUp= 0;
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                modifyUp= 2;
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                modifyUp= 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        counterSet = (GT_U8)((routeEntry->countSet == CPSS_IP_CNT_NO_SET_E) ?
                                            7 : routeEntry->countSet);

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            value = packetCmd;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E);

            value = routeEntry->ttlHopLimDecOptionsExtChkByPass;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E);

            value = cpuCodeIdx;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E);

            if (routeEntry->ingressMirror)
            {
                value = routeEntry->ingressMirrorToAnalyzerIndex + 1;
            }
            else
            {
                value = 0;
            }
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E);

            /* Get next hop muxing mode */
            rc = cpssDxChIpNhMuxModeGet(devNum, &muxMode);
            if (rc != GT_OK)
            {
                return rc;
            }
            switch (muxMode)
            {
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_QOS_E:
                {
                    value = routeEntry->qosPrecedence;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E);

                    value = routeEntry->qosProfileMarkingEnable;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E);

                    value = routeEntry->qosProfileIndex;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E);

                    value = modifyDscp;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E);

                    value = modifyUp;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E);
                }
                break;
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_VID1_E:
                {
                    value = routeEntry->nextHopVlanId1;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_VID1_E);
                }
                break;
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_E:
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E:
                {
                    value = routeEntry->targetEpg;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP6_10_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_EPG_E);
                }
                break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            value = routeEntry->nextHopVlanId;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_EVLAN_E);

            value = routeEntry->ttlHopLimitDecEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E);

            value = routeEntry->siteId;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_DEST_SITE_ID_E);

            value = routeEntry->scopeCheckingEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_SCOPE_CHECK_EN_E);

            value = routeEntry->ICMPRedirectEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_E);

            value = counterSet;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E);

            value = routeEntry->mtuProfileIndex;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E);

            value = routeEntry->trapMirrorArpBcEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E);

            value = routeEntry->appSpecificCpuCodeEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E);

            value = routeEntry->dipAccessLevel;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DIP_ACCESS_LEVEL_E);

            value = routeEntry->sipAccessLevel;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_ACCESS_LEVEL_E);

            value = routeEntry->unicastPacketSipFilterEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_FILTER_EN_E);
        }
        else
        {
            hwDataPtr[0] |= ((packetCmd & 0x7)                                              |
                     ((routeEntry->ttlHopLimDecOptionsExtChkByPass & 0x1) << 3) |
                     ((cpuCodeIdx & 0x3) << 4)                                      |
                     ((routeEntry->ingressMirror & 0x1) << 6)                   |
                     ((routeEntry->qosPrecedence & 0x1) << 7)                   |
                     ((routeEntry->qosProfileMarkingEnable & 0x1) << 8)         |
                     ((routeEntry->qosProfileIndex & 0x7F) << 9)                |
                     ((modifyDscp & 0x3) << 16)                                     |
                     ((modifyUp & 0x3) << 18)                                       |
                     ((routeEntry->nextHopVlanId & 0xFFF) << 20));
            hwDataPtr[1] |= ((routeEntry->ttlHopLimitDecEnable & 0x1) << 18);
            hwDataPtr[2] |= ((routeEntry->siteId & 0x1)                               |
                             ((routeEntry->scopeCheckingEnable & 0x1) << 1)           |
                             ((routeEntry->ICMPRedirectEnable & 0x1) << 2)            |
                             ((counterSet & 0x7) << 3)                                |
                             ((routeEntry->mtuProfileIndex & 0x7) << 6)               |
                             ((routeEntry->trapMirrorArpBcEnable & 0x1) << 9)         |
                             ((routeEntry->appSpecificCpuCodeEnable & 0x1) << 10)     |
                             ((routeEntry->dipAccessLevel & 0x7) << 11)               |
                             ((routeEntry->sipAccessLevel & 0x7) << 14)               |
                             ((routeEntry->unicastPacketSipFilterEnable & 0x1) << 17));
        }

        if ( ( (routeEntry->cmd == CPSS_PACKET_CMD_ROUTE_E) || (routeEntry->cmd == CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E) )
             || ( (routeEntry->cmd == CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E) && (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) ) )
        {
            /* outlif */
            if ((routeEntry->nextHopInterface.type == CPSS_INTERFACE_VIDX_E)||
                (routeEntry->nextHopInterface.type == CPSS_INTERFACE_VID_E))
            {
                if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    value = 1;  /* use VIDX */
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E);

                    if(routeEntry->nextHopInterface.type == CPSS_INTERFACE_VID_E)
                    {
                        value = 0xFFF;
                    }
                    else
                    {
                        value = routeEntry->nextHopInterface.vidx;
                    }
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EVIDX_E);
                }
                else
                {
                    hwDataPtr[1] |= 0x1; /* use VIDX */
                    if(routeEntry->nextHopInterface.type == CPSS_INTERFACE_VID_E)
                    {
                        hwDataPtr[1] |= (0xFFF << 1);
                    }
                    else
                    {
                        hwDataPtr[1] |= (routeEntry->nextHopInterface.vidx & 0xFFF) << 1;
                    }
                }
            }
            else if (routeEntry->nextHopInterface.type == CPSS_INTERFACE_TRUNK_E)
            {
                /* TRUNK */
                if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    value = 0;  /* use VIDX */
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E);

                    value = 1;  /* target is trunk */
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_TRUNK_E);

                    value = routeEntry->nextHopInterface.trunkId;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_TRUNK_ID_E);
                }
                else
                {
                    hwDataPtr[1] |= (1 << 1) | /* target is trunk */
                        ((routeEntry->nextHopInterface.trunkId & 0x7F) << 6);
                }
            }
            else
            {
                 if (routeEntry->nextHopInterface.type == CPSS_INTERFACE_PORT_E)
                 {
                    /* PORT */
                    targHwDev  = (PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                        routeEntry->nextHopInterface.devPort.hwDevNum,
                        routeEntry->nextHopInterface.devPort.portNum));
                    targPort = (PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                        routeEntry->nextHopInterface.devPort.hwDevNum,
                        routeEntry->nextHopInterface.devPort.portNum));
                    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                    {
                        value = 0;  /* use VIDX */
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E);

                        value = 0;  /* target is trunk */
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_TRUNK_E);

                        value = targHwDev;
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_DEV_E);

                        value = targPort;
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_EPORT_E);
                    }
                    else
                    {
                        hwDataPtr[1] |= ((targPort & 0x3F) << 2) | ((targHwDev & 0x1F) << 8);
                    }
                 }
                 else
                 {
                      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                 }
            }

            if (routeEntry->isTunnelStart == GT_FALSE)  /* Link Layer */
            {
                /* ARP MAC DA Pointer */
                if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    value = 0;  /* start of tunnel */
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E);

                    value = routeEntry->nextHopARPPointer;
                    SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_PTR_E);
                }
                else
                {
                    hwDataPtr[1] |= ((routeEntry->nextHopARPPointer & 0x1FFF) << 19);
                    hwDataPtr[2] |= (((routeEntry->nextHopARPPointer >> 13) & 0x1) << 19);
                }
            }
            else /* tunnel start */
            {
                if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    /* for bobcat2 B0 and above NAT is supported */
                    if ((PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))&&
                        (routeEntry->isNat == GT_TRUE))
                    {
                        value = 1;  /* start of tunnel */
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E);

                        value = 1;  /* tunnel start pointer in HW is actually a NAT pointer */
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TS_IS_NAT_E);

                        value = routeEntry->nextHopNatPointer;
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E);

                    }
                    else
                    {
                        value = 1;  /* start of tunnel */
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E);

                        value = routeEntry->nextHopTunnelPointer;
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E);

                        /* force the <tunnel type> to be 'other' (not 'Ethernet') */
                        value = 1;  /* <tunnel type> to be 'other' */
                        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TUNNEL_TYPE_E);
                    }
                }
                else
                {
                    hwDataPtr[1] |=
                        (((routeEntry->isTunnelStart & 0x1) << 17) |
                         (0x1 << 31) | /* <tunnel type> to be 'other' */
                         ((routeEntry->nextHopTunnelPointer & 0xFFF) << 19));
                }
            }
        }

        break;

    case CPSS_DXCH_IP_UC_ECMP_RPF_E:

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            hwDataPtr[0] |= (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[0]) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[1] << 13) |
                        ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[2] & 0x3f) << 26);

            hwDataPtr[1] |= ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[2] & 0x1fc0) >> 6) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[3] << 7) |
                        ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[4] & 0xfff) << 20);

            hwDataPtr[2] |= ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[4] & 0x1000) >> 12) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[5] << 1) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[6] << 14) |
                        ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[7] & 0x1f) << 27);

            hwDataPtr[3] |= ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[7] & 0x1fe0) >> 5);
        }
        else
        {
            hwDataPtr[0] |= (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[0]) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[1] << 12) |
                        ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[2] & 0xff) << 24);

            hwDataPtr[1] |= ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[2] & 0xf00) >> 8) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[3] << 4) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[4] << 16) |
                        ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[5] & 0xf) << 28);

            hwDataPtr[2] |= ((ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[5] & 0xff0) >> 4) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[6] << 8) |
                        (ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[7] << 20);

            /* hwDataPtr[3] = 0; */
        }

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChIpConvertHwFormat2UcEntry function
* @endinternal
*
* @brief   This function converts a given ip uc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] hwDataPtr                - The entry in the HW format representation.
* @param[in,out] ipUcRouteEntryPtr        - Set the route entry type to determine how hw data will be
*                                      interpreted
* @param[in,out] ipUcRouteEntryPtr        - The entry to be converted.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertHwFormat2UcEntry
(
    IN  GT_U8                               devNum,
    IN  GT_U32 *hwDataPtr,
    INOUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *ipUcRouteEntryPtr
)
{
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC     *regularEntry = NULL;
    GT_U32 value, cmd, cpuCodeIdx, modifyDscp = 0, modifyUp = 0;
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT  muxMode;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* clear output data */
    cpssOsMemSet(&ipUcRouteEntryPtr->entry, 0, sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT));

    switch (ipUcRouteEntryPtr->type)
    {
    case CPSS_DXCH_IP_UC_ROUTE_ENTRY_E:

        regularEntry = &ipUcRouteEntryPtr->entry.regularEntry; /* to make the code more readable... */

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E);
            cmd = value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E);
            cpuCodeIdx = value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E);
            regularEntry->ttlHopLimDecOptionsExtChkByPass = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E);
            if (value)
            {
                regularEntry->ingressMirrorToAnalyzerIndex = value - 1;
                regularEntry->ingressMirror = GT_TRUE;
            }
            else
            {
                regularEntry->ingressMirror = GT_FALSE;
            }

            /* Get next hop muxing mode */
            rc = cpssDxChIpNhMuxModeGet(devNum, &muxMode);
            if (rc != GT_OK)
            {
                return rc;
            }
            switch (muxMode)
            {
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_QOS_E:
                {
                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E);
                    regularEntry->qosPrecedence = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)value;

                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E);
                    regularEntry->qosProfileMarkingEnable = BIT2BOOL_MAC(value);

                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E);
                    regularEntry->qosProfileIndex = value;

                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E);
                    modifyDscp = value;

                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E);
                    modifyUp = value;
                }
                break;
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_VID1_E:
                {
                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_VID1_E);
                    regularEntry->nextHopVlanId1 = (GT_U16)value;
                }
                break;
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_E:
                case CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E:
                {
                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP6_10_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_EPG_E);
                    regularEntry->targetEpg = (GT_U16)value;
                }
                break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_EVLAN_E);
            regularEntry->nextHopVlanId = (GT_U16)value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E);
            regularEntry->ttlHopLimitDecEnable = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_DEST_SITE_ID_E);
            regularEntry->siteId = (CPSS_IP_SITE_ID_ENT)value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_SCOPE_CHECK_EN_E);
            regularEntry->scopeCheckingEnable = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_E);
            regularEntry->ICMPRedirectEnable = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E);
            if (value == 7)
            {
                regularEntry->countSet = CPSS_IP_CNT_NO_SET_E;
            }
            else
            {
                regularEntry->countSet = (CPSS_IP_CNT_SET_ENT)value;
            }

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E);
            regularEntry->mtuProfileIndex = value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E);
            regularEntry->trapMirrorArpBcEnable = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E);
            regularEntry->appSpecificCpuCodeEnable = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DIP_ACCESS_LEVEL_E);
            regularEntry->dipAccessLevel = value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_ACCESS_LEVEL_E);
            regularEntry->sipAccessLevel = value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_FILTER_EN_E);
            regularEntry->unicastPacketSipFilterEnable = BIT2BOOL_MAC(value);
        }
        else
        {
            cmd = U32_GET_FIELD_MAC(hwDataPtr[0],0,3);
            cpuCodeIdx = U32_GET_FIELD_MAC(hwDataPtr[0],4,2);
            modifyDscp = U32_GET_FIELD_MAC(hwDataPtr[0],16,2);
            modifyUp = U32_GET_FIELD_MAC(hwDataPtr[0],18,2);
            regularEntry->ttlHopLimDecOptionsExtChkByPass = U32_GET_FIELD_MAC(hwDataPtr[0],3,1);
            regularEntry->ingressMirror                   = U32_GET_FIELD_MAC(hwDataPtr[0],6,1);
            regularEntry->qosPrecedence                   = U32_GET_FIELD_MAC(hwDataPtr[0],7,1);
            regularEntry->qosProfileMarkingEnable         = U32_GET_FIELD_MAC(hwDataPtr[0],8,1);
            regularEntry->qosProfileIndex                 = U32_GET_FIELD_MAC(hwDataPtr[0],9,7);
            regularEntry->nextHopVlanId           = (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[0],20,12);
            regularEntry->ttlHopLimitDecEnable = U32_GET_FIELD_MAC(hwDataPtr[1],18,1);
            regularEntry->siteId                       = U32_GET_FIELD_MAC(hwDataPtr[2],0,1);
            regularEntry->scopeCheckingEnable          = U32_GET_FIELD_MAC(hwDataPtr[2],1,1);
            regularEntry->ICMPRedirectEnable           = U32_GET_FIELD_MAC(hwDataPtr[2],2,1);
            regularEntry->countSet                     = U32_GET_FIELD_MAC(hwDataPtr[2],3,3);
            if (regularEntry->countSet == 7)
            {
                regularEntry->countSet = CPSS_IP_CNT_NO_SET_E;
            }
            regularEntry->mtuProfileIndex              = U32_GET_FIELD_MAC(hwDataPtr[2],6,3);
            regularEntry->trapMirrorArpBcEnable        = U32_GET_FIELD_MAC(hwDataPtr[2],9,1);
            regularEntry->appSpecificCpuCodeEnable     = U32_GET_FIELD_MAC(hwDataPtr[2],10,1);
            regularEntry->dipAccessLevel               = U32_GET_FIELD_MAC(hwDataPtr[2],11,3);
            regularEntry->sipAccessLevel               = U32_GET_FIELD_MAC(hwDataPtr[2],14,3);
            regularEntry->unicastPacketSipFilterEnable = U32_GET_FIELD_MAC(hwDataPtr[2],17,1);
        }

        switch(cmd)
        {
            case 0:
                regularEntry->cmd = CPSS_PACKET_CMD_ROUTE_E;
                break;
            case 1:
                regularEntry->cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
                break;
            case 2:
                regularEntry->cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                break;
            case 3:
                regularEntry->cmd = CPSS_PACKET_CMD_DROP_HARD_E;
                break;
            case 4:
                regularEntry->cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                break;
            case 7:
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    regularEntry->cmd = CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E;
                    break;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(cpuCodeIdx)
        {
            case 0:
                regularEntry->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                break;
            case 1:
                regularEntry->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_1_E;
                break;
            case 2:
                regularEntry->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_2_E;
                break;
            case 3:
                regularEntry->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_3_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(modifyDscp)
        {
            case 0:
                regularEntry->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                break;
            case 1:
                regularEntry->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                break;
            case 2:
                regularEntry->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(modifyUp)
        {
            case 0:
                regularEntry->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                break;
            case 1:
                regularEntry->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                break;
            case 2:
                regularEntry->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if ( ( (regularEntry->cmd == CPSS_PACKET_CMD_ROUTE_E) || (regularEntry->cmd == CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E) )
             || ( (regularEntry->cmd == CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E) && (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) ) )
        {
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E);
                if (value == 1)
                {
                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EVIDX_E);
                    if(value==0xFFF)
                    {
                        /* VID */
                        regularEntry->nextHopInterface.type = CPSS_INTERFACE_VID_E;
                        regularEntry->nextHopInterface.vlanId = 0xFFF;
                    }
                    else
                    {
                        /* VIDX */
                        regularEntry->nextHopInterface.type = CPSS_INTERFACE_VIDX_E;
                        regularEntry->nextHopInterface.vidx = (GT_U16)value;
                    }
                }
                else
                {
                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_TRUNK_E);
                    if (value == 1)
                    {
                        /* TRUNK */
                        regularEntry->nextHopInterface.type = CPSS_INTERFACE_TRUNK_E;
                        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_TRUNK_ID_E);
                        regularEntry->nextHopInterface.trunkId = (GT_TRUNK_ID)value;
                    }
                    else
                    {
                        /* PORT */
                        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_DEV_E);
                        regularEntry->nextHopInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)value;
                        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_EPORT_E);
                        regularEntry->nextHopInterface.devPort.portNum = (GT_PORT_NUM)value;
                        PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(regularEntry->nextHopInterface));
                    }
                }

                SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E);
                if (value == 1)
                {
                     /* for bobcat2 B0 and above NAT is supported */
                    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
                    {
                        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TS_IS_NAT_E);
                        if(value == 1)
                        {
                            regularEntry->isTunnelStart = GT_TRUE;
                            regularEntry->isNat = GT_TRUE;
                            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E);
                            regularEntry->nextHopNatPointer = value;
                        }
                        else
                        {
                            regularEntry->isTunnelStart = GT_TRUE;
                            regularEntry->isNat = GT_FALSE;
                            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E);
                            regularEntry->nextHopTunnelPointer = value;
                        }
                    }
                    else
                    {
                        regularEntry->isTunnelStart = GT_TRUE;
                        regularEntry->isNat = GT_FALSE;
                        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E);
                        regularEntry->nextHopTunnelPointer = value;
                    }
                }
                else
                {
                    regularEntry->isTunnelStart = GT_FALSE;
                    regularEntry->isNat = GT_FALSE;
                    SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_PTR_E);
                    regularEntry->nextHopARPPointer = value;
                }
            }
            else /* not eArch*/
            {
                if (U32_GET_FIELD_MAC(hwDataPtr[1],0,1) == 1)
                {
                    regularEntry->nextHopInterface.type    = CPSS_INTERFACE_VIDX_E;
                    regularEntry->nextHopInterface.vidx    =
                        (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[1],1,13);
                }
                else if(U32_GET_FIELD_MAC(hwDataPtr[1],1,1) == 1)
                {
                    regularEntry->nextHopInterface.type    = CPSS_INTERFACE_TRUNK_E;
                    regularEntry->nextHopInterface.trunkId =
                        (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[1],6,7);
                }
                else
                {
                    regularEntry->nextHopInterface.type    = CPSS_INTERFACE_PORT_E;
                    regularEntry->nextHopInterface.devPort.hwDevNum =
                        U32_GET_FIELD_MAC(hwDataPtr[1],8,5);
                    regularEntry->nextHopInterface.devPort.portNum =
                        U32_GET_FIELD_MAC(hwDataPtr[1],2,6);
                    PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(regularEntry->nextHopInterface));
                }

                if(U32_GET_FIELD_MAC(hwDataPtr[1],17,1) == 0)
                {
                    regularEntry->isTunnelStart = GT_FALSE;
                        regularEntry->nextHopARPPointer =
                            U32_GET_FIELD_MAC(hwDataPtr[1],19,13) | (U32_GET_FIELD_MAC(hwDataPtr[2],19,1) << 13);
                }
                else
                {
                    regularEntry->isTunnelStart = GT_TRUE;
                    regularEntry->nextHopTunnelPointer = U32_GET_FIELD_MAC(hwDataPtr[1],19,12);
                }
            }
        }

        break;

    case CPSS_DXCH_IP_UC_ECMP_RPF_E:

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[0] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[0],0,13));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[1] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[0],13,13));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[2] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[0],26,6) | (U32_GET_FIELD_MAC(hwDataPtr[1],0,7) << 6));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[3] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[1],7,13));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[4] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[1],20,12) | (U32_GET_FIELD_MAC(hwDataPtr[2],0,1) << 12));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[5] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[2],1,13));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[6] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[2],14,13));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[7] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[2],27,5) | (U32_GET_FIELD_MAC(hwDataPtr[3],0,8) << 5));
        }
        else
        {
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[0] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[0],0,12));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[1] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[0],12,12));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[2] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[0],24,8) | (U32_GET_FIELD_MAC(hwDataPtr[1],0,4) << 8));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[3] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[1],4,12));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[4] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[1],16,12));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[5] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[1],28,4) | (U32_GET_FIELD_MAC(hwDataPtr[2],0,8) << 4));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[6] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[2],8,12));
            ipUcRouteEntryPtr->entry.ecmpRpfCheck.vlanArray[7] =
                (GT_U16)(U32_GET_FIELD_MAC(hwDataPtr[2],20,12));
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIpConvertMcEntry2HwFormat function
* @endinternal
*
* @brief   This function converts a given ip mc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] ipMcRouteEntryPtr        - The entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertMcEntry2HwFormat
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *ipMcRouteEntryPtr,
    OUT GT_U32 *hwDataPtr
)
{
    GT_U8         packetCmd;        /* packet command   */
    GT_U8         cpuCodeIdx;       /* cpu code index   */
    GT_U8         rpfFailCmd = 0;       /* RPF Fail command */
    GT_U8         counterSet;       /* counter Set */
    GT_U8         modifyDscp;       /* modify DSCP*/
    GT_U8         modifyUp;         /* modify UP */
    GT_U32        value;            /* used for sip5 macro */
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT  muxMode;
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* clear hw data */
    cpssOsMemSet(hwDataPtr, 0, sizeof(GT_U32) * 4);

    switch(ipMcRouteEntryPtr->cmd)
    {
        case CPSS_PACKET_CMD_ROUTE_E:
            packetCmd = 0;
            break;
        case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
            packetCmd = 1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            packetCmd = 2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            packetCmd = 3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            packetCmd = 4;
            break;
        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            packetCmd = 5;
            break;
        case CPSS_PACKET_CMD_BRIDGE_E:
            packetCmd = 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(ipMcRouteEntryPtr->cpuCodeIdx)
    {
         case CPSS_DXCH_IP_CPU_CODE_IDX_0_E:
             cpuCodeIdx = 0;
             break;
         case CPSS_DXCH_IP_CPU_CODE_IDX_1_E:
             cpuCodeIdx = 1;
             break;
         case CPSS_DXCH_IP_CPU_CODE_IDX_2_E:
             cpuCodeIdx = 2;
             break;
         case CPSS_DXCH_IP_CPU_CODE_IDX_3_E:
             cpuCodeIdx = 3;
             break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(ipMcRouteEntryPtr->modifyDscp)
    {
        case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            modifyDscp = 0;
            break;
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            modifyDscp = 2;
            break;
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            modifyDscp = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(ipMcRouteEntryPtr->modifyUp)
    {
        case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            modifyUp = 0;
            break;
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            modifyUp = 2;
            break;
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            modifyUp = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(ipMcRouteEntryPtr->multicastRPFCheckEnable == GT_TRUE)
    {
        switch(ipMcRouteEntryPtr->RPFFailCommand)
        {
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            rpfFailCmd = 2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            rpfFailCmd = 3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            rpfFailCmd = 4;
            break;
        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            rpfFailCmd = 5;
            break;
        case CPSS_PACKET_CMD_BRIDGE_E:
            rpfFailCmd = 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* default value for rpfFailsCmd is CPSS_PACKET_CMD_TRAP_TO_CPU_E */
         rpfFailCmd = 2;
    }

    counterSet = (GT_U8)((ipMcRouteEntryPtr->countSet == CPSS_IP_CNT_NO_SET_E) ?
                                                7 : ipMcRouteEntryPtr->countSet);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (ipMcRouteEntryPtr->ttlHopLimitDecEnable && ipMcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass)
        {
            /* It is not legal to configure bypass the TTL/Hop-Limit Check and to enable the
            TTL/Hop-Limit to be decremented. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        value = packetCmd;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E);

        value = ipMcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E);

        value = cpuCodeIdx;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E);

        if (ipMcRouteEntryPtr->ingressMirror)
        {
            value = ipMcRouteEntryPtr->ingressMirrorToAnalyzerIndex + 1;
        }
        else
        {
            value = 0;
        }
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E);

        /* Get next hop muxing mode */
        rc = cpssDxChIpNhMuxModeGet(devNum, &muxMode);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(muxMode == CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E)
        {
            value = ipMcRouteEntryPtr->targetEpg;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP6_10_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_EPG_E);
        }
        else
        {
            value = ipMcRouteEntryPtr->qosPrecedence;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E);

            value = ipMcRouteEntryPtr->qosProfileMarkingEnable;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E);

            value = ipMcRouteEntryPtr->qosProfileIndex;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E);
        }

        value = modifyDscp;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E);

        value = modifyUp;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E);

        if (ipMcRouteEntryPtr->multicastIngressVlanCheck == CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E)
        {
            value = ipMcRouteEntryPtr->multicastRPFVlan;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_E);
        }
        else    /* CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E */
        {
            value = ipMcRouteEntryPtr->multicastRPFRoutingSharedTreeIndex;
            SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_MRST_INDEX_E);
        }

        value = (ipMcRouteEntryPtr->externalMLLPointer & 0x1FFF);
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_E);

        value = (ipMcRouteEntryPtr->externalMLLPointer >> 13);
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_MSB_BITS_15_13_E);

        value = rpfFailCmd;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_FAIL_CMD_E);

        value = ipMcRouteEntryPtr->multicastRPFFailCommandMode;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_RPF_FAIL_CMD_MODE_E);

        value = ipMcRouteEntryPtr->ttlHopLimitDecEnable;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E);

        value = ipMcRouteEntryPtr->internalMLLPointer;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INTERNAL_MLL_PTR_E);

        value = ipMcRouteEntryPtr->siteId;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_DEST_SITE_ID_E);

        value = ipMcRouteEntryPtr->scopeCheckingEnable;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_SCOPE_CHECK_EN_E);

        value = counterSet;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E);

        value = ipMcRouteEntryPtr->mtuProfileIndex;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E);

        value = ipMcRouteEntryPtr->appSpecificCpuCodeEnable;
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E);

        if (ipMcRouteEntryPtr->multicastRPFCheckEnable)
        {
            if (ipMcRouteEntryPtr->multicastIngressVlanCheck == CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E)
            {
                value = 1;
            }
            else    /* CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E */
            {
                value = 2;
            }
        }
        else
        {
            value = 0;
        }
        SIP5_IPVX_FIELD_VALUE_SET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_E);
    }
    else
    {
        hwDataPtr[0] |= ((packetCmd & 0x7)                                            |
                    ((ipMcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass & 0x1) << 3)|
                    ((cpuCodeIdx & 0x3) << 4)                                     |
                    ((ipMcRouteEntryPtr->ingressMirror & 0x1) << 6)                  |
                    ((ipMcRouteEntryPtr->qosPrecedence & 0x1) << 7)                  |
                    ((ipMcRouteEntryPtr->qosProfileMarkingEnable & 0x1) << 8)        |
                    ((ipMcRouteEntryPtr->qosProfileIndex & 0x7F) << 9)               |
                    ((modifyDscp & 0x3) << 16)                    |
                    ((modifyUp & 0x3) << 18)                      |
                    ((ipMcRouteEntryPtr->multicastRPFVlan & 0xFFF) << 20));
        hwDataPtr[1] |= ((ipMcRouteEntryPtr->externalMLLPointer & 0x1FFF)                |
                    ((rpfFailCmd & 0x7) << 14)                                    |
                    ((ipMcRouteEntryPtr->multicastRPFFailCommandMode & 0x1) << 17)   |
                    ((ipMcRouteEntryPtr->ttlHopLimitDecEnable & 0x1) << 18)          |
                    ((ipMcRouteEntryPtr->internalMLLPointer & 0x1FFF) << 19));
        hwDataPtr[2] |= ((ipMcRouteEntryPtr->siteId & 0x1)                                |
                     ((ipMcRouteEntryPtr->scopeCheckingEnable & 0x1) << 1)            |
                     ((counterSet & 0x7) << 3)                                     |
                     ((ipMcRouteEntryPtr->mtuProfileIndex & 0x7) << 6)                |
                     ((ipMcRouteEntryPtr->appSpecificCpuCodeEnable & 0x1) << 10)      |
                     ((ipMcRouteEntryPtr->multicastRPFCheckEnable & 0x1) << 18));

        /* hwDataPtr[3] = 0; */
    }

   return GT_OK;
}

/**
* @internal prvCpssDxChIpConvertHwFormat2McEntry function
* @endinternal
*
* @brief   This function converts a given ip mc entry to the HW format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] hwDataPtr                - The mc entry in the HW format to be converted..
*
* @param[out] ipMcRouteEntryPtr        - The mc entry data.
*
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssDxChIpConvertHwFormat2McEntry
(
    IN   GT_U8                           devNum,
    OUT  CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *ipMcRouteEntryPtr,
    IN   GT_U32 *hwDataPtr
)
{
    GT_U32 value, cmd, cpuCodeIdx, qosPrecedence=0, modifyDscp, modifyUp,
           RPFFailCommand, multicastRPFFailCommandMode, siteId, countSet;
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT  muxMode=CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_QOS_E;
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E);
        cmd = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E);
        cpuCodeIdx = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E);
        ipMcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass = BIT2BOOL_MAC(value);

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E);
        if (value)
        {
            ipMcRouteEntryPtr->ingressMirrorToAnalyzerIndex = value - 1;
            ipMcRouteEntryPtr->ingressMirror = GT_TRUE;
        }
        else
        {
            ipMcRouteEntryPtr->ingressMirror = GT_FALSE;
        }

        /* Get next hop muxing mode */
        rc = cpssDxChIpNhMuxModeGet(devNum, &muxMode);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(muxMode == CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E)
        {
            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP6_10_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_EPG_E);
            ipMcRouteEntryPtr->targetEpg = value;
        }
        else
        {
            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E);
            qosPrecedence = value;

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E);
            ipMcRouteEntryPtr->qosProfileMarkingEnable = BIT2BOOL_MAC(value);

            SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E);
            ipMcRouteEntryPtr->qosProfileIndex = value;
        }

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E);
        modifyDscp = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E);
        modifyUp = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_E);
        if (value == 0)
        {
            ipMcRouteEntryPtr->multicastRPFCheckEnable = GT_FALSE;
        }
        else
        {
            ipMcRouteEntryPtr->multicastRPFCheckEnable = GT_TRUE;
            if (value == 1)
            {
                ipMcRouteEntryPtr->multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E;
                SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_E);
                ipMcRouteEntryPtr->multicastRPFVlan = (GT_U16)value;
            }
            else    /* value == 2 */
            {
                ipMcRouteEntryPtr->multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E;
                SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_MRST_INDEX_E);
                ipMcRouteEntryPtr->multicastRPFRoutingSharedTreeIndex = (GT_U8)value;
            }
        }

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_E);
        ipMcRouteEntryPtr->externalMLLPointer = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_MSB_BITS_15_13_E);
        ipMcRouteEntryPtr->externalMLLPointer |= (value << 13);

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_RPF_FAIL_CMD_MODE_E);
        multicastRPFFailCommandMode = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E);
        ipMcRouteEntryPtr->ttlHopLimitDecEnable = BIT2BOOL_MAC(value);

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INTERNAL_MLL_PTR_E);
        ipMcRouteEntryPtr->internalMLLPointer = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_DEST_SITE_ID_E);
        siteId = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_SCOPE_CHECK_EN_E);
        ipMcRouteEntryPtr->scopeCheckingEnable = BIT2BOOL_MAC(value);

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E);
        ipMcRouteEntryPtr->mtuProfileIndex = value;

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E);
        ipMcRouteEntryPtr->appSpecificCpuCodeEnable = BIT2BOOL_MAC(value);

        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E);
        countSet = value;
    }
    else
    {
        cmd = U32_GET_FIELD_MAC(hwDataPtr[0],0,3);
        cpuCodeIdx = U32_GET_FIELD_MAC(hwDataPtr[0],4,2);
        ipMcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[0],3,1));
        ipMcRouteEntryPtr->ingressMirror                   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[0],6,1));
        qosPrecedence = U32_GET_FIELD_MAC(hwDataPtr[0],7,1);
        ipMcRouteEntryPtr->qosProfileMarkingEnable         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[0],8,1));
        ipMcRouteEntryPtr->qosProfileIndex                 = U32_GET_FIELD_MAC(hwDataPtr[0],9,7);
        ipMcRouteEntryPtr->multicastRPFVlan        = (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[0],20,12);
        modifyDscp = U32_GET_FIELD_MAC(hwDataPtr[0],16,2);
        modifyUp = U32_GET_FIELD_MAC(hwDataPtr[0],18,2);
        ipMcRouteEntryPtr->multicastRPFCheckEnable  = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[2],18,1));
        ipMcRouteEntryPtr->externalMLLPointer = U32_GET_FIELD_MAC(hwDataPtr[1],0,13);
        multicastRPFFailCommandMode = U32_GET_FIELD_MAC(hwDataPtr[1],17,1);
        ipMcRouteEntryPtr->ttlHopLimitDecEnable            = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[1],18,1));
        ipMcRouteEntryPtr->internalMLLPointer              = U32_GET_FIELD_MAC(hwDataPtr[1],19,13);
        siteId = U32_GET_FIELD_MAC(hwDataPtr[2],0,1);
        ipMcRouteEntryPtr->scopeCheckingEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[2],1,1));
        ipMcRouteEntryPtr->mtuProfileIndex = U32_GET_FIELD_MAC(hwDataPtr[2],6,3);
        ipMcRouteEntryPtr->appSpecificCpuCodeEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[2],10,1));
        countSet = U32_GET_FIELD_MAC(hwDataPtr[2],3,3);
    }


    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        SIP5_IPVX_FIELD_VALUE_GET_MAC(devNum,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_FAIL_CMD_E);
        RPFFailCommand = value;
    }
    else
    {
        RPFFailCommand = U32_GET_FIELD_MAC(hwDataPtr[1],14,3);
    }
    switch(RPFFailCommand)
    {
    case 2:
        ipMcRouteEntryPtr->RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        break;
    case 3:
        ipMcRouteEntryPtr->RPFFailCommand = CPSS_PACKET_CMD_DROP_HARD_E;
        break;
    case 4:
        ipMcRouteEntryPtr->RPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
        break;
    case 5:
        ipMcRouteEntryPtr->RPFFailCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
        break;
    case 6:
        ipMcRouteEntryPtr->RPFFailCommand = CPSS_PACKET_CMD_BRIDGE_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }


    switch(cmd)
    {
        case 0:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_ROUTE_E;
            break;
        case 1:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;
        case 2:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        case 5:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case 6:
            ipMcRouteEntryPtr->cmd = CPSS_PACKET_CMD_BRIDGE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(cpuCodeIdx)
    {
         case 0:
             ipMcRouteEntryPtr->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
             break;
         case 1:
             ipMcRouteEntryPtr->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_1_E;
             break;
         case 2:
             ipMcRouteEntryPtr->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_2_E;
             break;
         case 3:
             ipMcRouteEntryPtr->cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_3_E;
             break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(!PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) ||
        (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) && (muxMode != CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E)))
    {
        switch(qosPrecedence)
        {
            case 0:
                ipMcRouteEntryPtr->qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
                break;
            case 1:
                ipMcRouteEntryPtr->qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    switch(modifyDscp)
    {
        case 0:
            ipMcRouteEntryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case 1:
            ipMcRouteEntryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        case 2:
            ipMcRouteEntryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
        case 3:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(modifyUp)
    {
        case 0:
            ipMcRouteEntryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case 1:
            ipMcRouteEntryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        case 2:
            ipMcRouteEntryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
        case 3:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(multicastRPFFailCommandMode)
    {
        case 0:
            ipMcRouteEntryPtr->multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
            break;
        case 1:
            ipMcRouteEntryPtr->multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_MLL_RPF_FAIL_COMMAND_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(siteId)
    {
        case 0:
            ipMcRouteEntryPtr->siteId = CPSS_IP_SITE_ID_INTERNAL_E;
            break;
        case 1:
            ipMcRouteEntryPtr->siteId = CPSS_IP_SITE_ID_EXTERNAL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(countSet)
    {
        case 0:
            ipMcRouteEntryPtr->countSet = CPSS_IP_CNT_SET0_E;
            break;
        case 1:
            ipMcRouteEntryPtr->countSet = CPSS_IP_CNT_SET1_E;
            break;
        case 2:
            ipMcRouteEntryPtr->countSet = CPSS_IP_CNT_SET2_E;
            break;
        case 3:
            ipMcRouteEntryPtr->countSet = CPSS_IP_CNT_SET3_E;
            break;
        case 4:
            ipMcRouteEntryPtr->countSet = CPSS_IP_CNT_NO_SET_E;
            break;
        case 7:
            ipMcRouteEntryPtr->countSet = CPSS_IP_CNT_NO_SET_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIpUcRouteEntriesCheck function
* @endinternal
*
* @brief   Check validity of the route entry parametrers, in all entries
*         of routeEntriesArray.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array.
*
* @retval GT_OK                    - on all valid parameters.
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvCpssDxChIpUcRouteEntriesCheck
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                          numOfRouteEntries
)
{
    GT_U32 i,j;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC  *routeEntry = NULL;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_PORT_NUM   portTrunk;/*tmp port/trunk*/
    GT_HW_DEV_NUM devHwTmp;/*tmp HW device*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(routeEntriesArray);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    for (i = 0; i<numOfRouteEntries; i++)
    {
        switch (routeEntriesArray[i].type)
        {
        case CPSS_DXCH_IP_UC_ROUTE_ENTRY_E:

            routeEntry = &routeEntriesArray[i].entry.regularEntry; /* to make the code more readable... */

            switch(routeEntry->cmd)
            {
            case CPSS_PACKET_CMD_ROUTE_E:
            case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            case CPSS_PACKET_CMD_DROP_HARD_E:
            case CPSS_PACKET_CMD_DROP_SOFT_E:
                break;
            case CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E:
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    break;
                }
                GT_ATTR_FALLTHROUGH;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(routeEntry->cpuCodeIdx)
            {
            case CPSS_DXCH_IP_CPU_CODE_IDX_0_E:
            case CPSS_DXCH_IP_CPU_CODE_IDX_1_E:
            case CPSS_DXCH_IP_CPU_CODE_IDX_2_E:
            case CPSS_DXCH_IP_CPU_CODE_IDX_3_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(routeEntry->qosPrecedence)
            {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(routeEntry->modifyUp)
            {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(routeEntry->modifyDscp)
            {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(routeEntry->countSet)
            {
            case CPSS_IP_CNT_SET0_E:
            case CPSS_IP_CNT_SET1_E:
            case CPSS_IP_CNT_SET2_E:
            case CPSS_IP_CNT_SET3_E:
            case CPSS_IP_CNT_NO_SET_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(routeEntry->siteId)
            {
            case CPSS_IP_SITE_ID_INTERNAL_E:
            case CPSS_IP_SITE_ID_EXTERNAL_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(routeEntry->nextHopVlanId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                if (routeEntry->nextHopVlanId1 > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
            }

            switch(routeEntry->nextHopInterface.type)
            {
            case CPSS_INTERFACE_PORT_E:
                PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(routeEntry->nextHopInterface.devPort.hwDevNum,
                                                       routeEntry->nextHopInterface.devPort.portNum);
                devHwTmp = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                    routeEntry->nextHopInterface.devPort.hwDevNum,
                    routeEntry->nextHopInterface.devPort.portNum);
                portTrunk = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                    routeEntry->nextHopInterface.devPort.hwDevNum,
                    routeEntry->nextHopInterface.devPort.portNum);
                if (portTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                if (devHwTmp > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                break;
            case CPSS_INTERFACE_TRUNK_E:
                if (routeEntry->nextHopInterface.trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                break;
            case CPSS_INTERFACE_VIDX_E:
                if (routeEntry->nextHopInterface.vidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                break;
            case CPSS_INTERFACE_VID_E:
                /* in this case we will set vidx to be 0xFFF and useVidx=GT_TRUE */
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if (routeEntry->qosProfileIndex >= BIT_7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if(routeEntry->mtuProfileIndex > 7)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

            /* Tunnel pointer and ARP pointer reside on the same HW bits,
               the meaning of those bits is determine by those bits          */
            if(routeEntry->isTunnelStart == GT_TRUE)
            {
                 if ((PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))&&
                     (routeEntry->isNat == GT_TRUE))
                 {
                     if (routeEntry->nextHopNatPointer >= fineTuningPtr->tableSize.tunnelStart)
                         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                 }
                 else
                 {
                     if (routeEntry->nextHopTunnelPointer >= fineTuningPtr->tableSize.tunnelStart)
                         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                 }
            }
            else/* ARP */
            {
                /* ARP pointer is relevant only if packet */
                /* command is route or route and mirror   */
        if ( ( (routeEntry->cmd == CPSS_PACKET_CMD_ROUTE_E) || (routeEntry->cmd == CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E) )
             || ( (routeEntry->cmd == CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E) && (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) ) )
                {
                    if (routeEntry->nextHopARPPointer >= fineTuningPtr->tableSize.routerArp)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }

            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                if ((routeEntry->ingressMirror == GT_TRUE) &&
                    (routeEntry->ingressMirrorToAnalyzerIndex > PRV_CPSS_DXCH_IP_MIRROR_ANALYZER_MAX_INDEX_CNS))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
            }

            break;

        case CPSS_DXCH_IP_UC_ECMP_RPF_E:

            for (j = 0 ; j < 8 ; j++)
                {
                if (routeEntriesArray[i].entry.ecmpRpfCheck.vlanArray[j] > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIpMcRouteEntryCheck function
* @endinternal
*
* @brief   Check validity of the route entry parametrers, in all entries
*         of routeEntriesArray.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routeEntryPtr            - the Mc route entry
*
* @retval GT_OK                    - on all valid parameters.
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
*/
GT_STATUS prvCpssDxChIpMcRouteEntryCheck
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
{
    switch(routeEntryPtr->cmd)
    {
    case CPSS_PACKET_CMD_ROUTE_E:
    case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
    case CPSS_PACKET_CMD_DROP_HARD_E:
    case CPSS_PACKET_CMD_DROP_SOFT_E:
    case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
    case CPSS_PACKET_CMD_BRIDGE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(routeEntryPtr->cpuCodeIdx)
    {
    case CPSS_DXCH_IP_CPU_CODE_IDX_0_E:
    case CPSS_DXCH_IP_CPU_CODE_IDX_1_E:
    case CPSS_DXCH_IP_CPU_CODE_IDX_2_E:
    case CPSS_DXCH_IP_CPU_CODE_IDX_3_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(routeEntryPtr->qosPrecedence)
    {
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(routeEntryPtr->modifyUp)
    {
    case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
    case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
    case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(routeEntryPtr->modifyDscp)
    {
    case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
    case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
    case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(routeEntryPtr->countSet)
    {
    case CPSS_IP_CNT_SET0_E:
    case CPSS_IP_CNT_SET1_E:
    case CPSS_IP_CNT_SET2_E:
    case CPSS_IP_CNT_SET3_E:
    case CPSS_IP_CNT_NO_SET_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(routeEntryPtr->siteId)
    {
    case CPSS_IP_SITE_ID_INTERNAL_E:
    case CPSS_IP_SITE_ID_EXTERNAL_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(routeEntryPtr->multicastRPFCheckEnable)
    {
        switch(routeEntryPtr->multicastRPFFailCommandMode)
        {
        case CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E:
        case CPSS_DXCH_IP_MULTICAST_MLL_RPF_FAIL_COMMAND_MODE_E:

            switch(routeEntryPtr->RPFFailCommand)
            {
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            case CPSS_PACKET_CMD_DROP_SOFT_E:
            case CPSS_PACKET_CMD_DROP_HARD_E:
            case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            case CPSS_PACKET_CMD_BRIDGE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if((routeEntryPtr->multicastRPFCheckEnable == GT_TRUE) &&
        (routeEntryPtr->multicastRPFVlan > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(routeEntryPtr->mtuProfileIndex > 7)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if ((routeEntryPtr->ingressMirror == GT_TRUE) &&
            (routeEntryPtr->ingressMirrorToAnalyzerIndex > PRV_CPSS_DXCH_IP_MIRROR_ANALYZER_MAX_INDEX_CNS))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        switch (routeEntryPtr->multicastIngressVlanCheck)
        {
            case CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E:
            case CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (routeEntryPtr->multicastRPFRoutingSharedTreeIndex > PRV_CPSS_DXCH_IP_SHARED_TREE_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIpMllStruct2HwFormat function
* @endinternal
*
* @brief   This function converts a given ip Mll struct to the hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - (points to ) the entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS prvCpssDxChIpMllStruct2HwFormat
(
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairWriteForm,
    IN  CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr
)
{
    GT_U32 rpfFailCommand;
    GT_U32 tunnelStartPassengerType = 0;
    GT_U32 hwPort,hwDev;

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        switch(mllPairEntryPtr->firstMllNode.mllRPFFailCommand)
        {
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                rpfFailCommand = 2;
                break;
            case CPSS_PACKET_CMD_DROP_HARD_E:
                rpfFailCommand = 3;
                break;
            case CPSS_PACKET_CMD_DROP_SOFT_E:
                rpfFailCommand = 4;
                break;
            case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
                rpfFailCommand = 5;
                break;
            case CPSS_PACKET_CMD_BRIDGE_E:
                rpfFailCommand = 6;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwDataPtr[0] = (BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.last)                |
                     (BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.isTunnelStart) << 1)|
                     ((mllPairEntryPtr->firstMllNode.nextHopVlanId & 0xFFF) << 3)|
                     ((rpfFailCommand & 0x7) << 29));

        switch(mllPairEntryPtr->firstMllNode.nextHopInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                    mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum,
                    mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.portNum);
                hwDev =  PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                    mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum,
                    mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.portNum);
                hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                    mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum,
                    mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.portNum);
                hwDataPtr[0] |= ((0x0 << 15) | (0x0 << 16)                                                      |
                              ((hwPort & 0x3F) << 17)|
                              ((hwDev & 0x1F) << 23));
                break;
            case CPSS_INTERFACE_TRUNK_E:
                hwDataPtr[0] |= ((0x0 << 15) | (0x1 << 16) |
                              ((mllPairEntryPtr->firstMllNode.nextHopInterface.trunkId & 0x7F) << 21));
                break;
            case CPSS_INTERFACE_VIDX_E:
                hwDataPtr[0] |= ((0x1 << 15) |
                             ((mllPairEntryPtr->firstMllNode.nextHopInterface.vidx & 0xFFF) << 16));

                break;
            case CPSS_INTERFACE_VID_E:
                hwDataPtr[0] |= ((0x1 << 15) | (0xFFF << 16));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        }

        /* tunnelStartPassengerType should be handled only in supported devices and
           when isTunnelStart is GT_TRUE*/
        if (mllPairEntryPtr->firstMllNode.isTunnelStart == GT_TRUE)
        {
             switch(mllPairEntryPtr->firstMllNode.tunnelStartPassengerType)
            {
                case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                    tunnelStartPassengerType = 0;
                    break;
                case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                    tunnelStartPassengerType = 1;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /*in case we dont write whole entry we need to save 2 bits from word 1
          for the tunnel pointer of second mll*/
        if (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E)
            mllPairEntryPtr->secondMllNode.nextHopTunnelPointer = U32_GET_FIELD_MAC(hwDataPtr[1],22,2) << 10;

        /*we will use |= so the second mll bits we have read before the function will not be overide*/
        hwDataPtr[1] = ((mllPairEntryPtr->firstMllNode.nextHopTunnelPointer & 0x3FF)       |
                       ((tunnelStartPassengerType & 0x1) << 10)                            |
                       ((mllPairEntryPtr->firstMllNode.ttlHopLimitThreshold & 0xFF) << 11) |
                       (BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.excludeSrcVlan) << 19));


        /*we write to word 1 also the second mll save value we extracted*/
        hwDataPtr[1] |= (mllPairEntryPtr->firstMllNode.nextHopTunnelPointer & 0xC00) << 10;
        hwDataPtr[1] |= (mllPairEntryPtr->secondMllNode.nextHopTunnelPointer & 0xC00) << 12;
     }

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        switch(mllPairEntryPtr->secondMllNode.mllRPFFailCommand)
        {
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                rpfFailCommand = 2;
                break;
            case CPSS_PACKET_CMD_DROP_HARD_E:
                rpfFailCommand = 3;
                break;
            case CPSS_PACKET_CMD_DROP_SOFT_E:
                rpfFailCommand = 4;
                break;
            case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
                rpfFailCommand = 5;
                break;
            case CPSS_PACKET_CMD_BRIDGE_E:
                rpfFailCommand = 6;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwDataPtr[2] = (BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.last)                 |
                     (BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.isTunnelStart) << 1) |
                     ((mllPairEntryPtr->secondMllNode.nextHopVlanId & 0xFFF) << 3) |
                     ((rpfFailCommand & 0x7) << 29));

        switch(mllPairEntryPtr->secondMllNode.nextHopInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                    mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum,
                    mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.portNum);
                hwDev =  PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                    mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum,
                    mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.portNum);
                hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                    mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum,
                    mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.portNum);
                hwDataPtr[2] |= ((0x0 << 15) | (0x0 << 16)                                                       |
                              ((hwPort & 0x3F) << 17)|
                              ((hwDev & 0x1F) << 23));
                break;
            case CPSS_INTERFACE_TRUNK_E:
                hwDataPtr[2] |= ((0x0 << 15) | (0x1 << 16) |
                              ((mllPairEntryPtr->secondMllNode.nextHopInterface.trunkId & 0x7F) << 21));
                break;
            case CPSS_INTERFACE_VIDX_E:
                hwDataPtr[2] |= ((0x1 << 15) |
                             ((mllPairEntryPtr->secondMllNode.nextHopInterface.vidx & 0xFFF) << 16));

                break;
            case CPSS_INTERFACE_VID_E:
                hwDataPtr[2] |= ((0x1 << 15) | (0xFFF << 16));
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        }

        /* tunnelStartPassengerType should be handled only in supported devices and
           when isTunnelStart is GT_TRUE*/
        if (mllPairEntryPtr->secondMllNode.isTunnelStart == GT_TRUE)
        {
             switch(mllPairEntryPtr->secondMllNode.tunnelStartPassengerType)
            {
                case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                    tunnelStartPassengerType = 0;
                    break;
                case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                    tunnelStartPassengerType = 1;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        hwDataPtr[3] = ((mllPairEntryPtr->secondMllNode.nextHopTunnelPointer & 0x3FF)       |
                       ((tunnelStartPassengerType & 0x1) << 10)                             |
                       ((mllPairEntryPtr->secondMllNode.ttlHopLimitThreshold & 0xFF) << 11) |
                       (BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.excludeSrcVlan) << 19)  |
                       ((mllPairEntryPtr->nextPointer & 0xFFF) << 20));

        /*we write to word 1 only to relevant mll and we dont modify what we red for the other mll*/
        switch (mllPairWriteForm)
        {
            case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
                /*need to clean old garbage value and then set this bit*/
                U32_SET_FIELD_MAC(hwDataPtr[1],22,2,0);
                hwDataPtr[1] |= (mllPairEntryPtr->secondMllNode.nextHopTunnelPointer & 0xC00) << 12;
                break;
            case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
                /*nothing to do we changed it in first if*/
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIpHwFormat2MllStruct function
* @endinternal
*
* @brief   This function converts a given ip Mll struct to the hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] hwDataPtr                - The entry in the HW format representation.
* @param[in] isFirstMll               - if the MLL analyzed is the first MLL in the pair
*
* @param[out] mllEntryPtr              - The entry to be converted.
*
* @retval GT_OK                    - on all valid parameters.
* @retval GT_BAD_STATE             - on invalid hardware value read
*/
static GT_STATUS prvCpssDxChIpHwFormat2MllStruct
(
    IN   GT_U32                 *hwDataPtr,
    IN   GT_BOOL                isFirstMll,
    OUT  CPSS_DXCH_IP_MLL_STC   *mllEntryPtr
)
{
    GT_U32 useVidx0;
    GT_U32 targetIsTrunk;

    GT_U32 relativeLocation = 0;

    if (isFirstMll == GT_FALSE)
    {
        relativeLocation = 2;
    }

    /* clear output data */
    cpssOsMemSet(mllEntryPtr, 0, sizeof(CPSS_DXCH_IP_MLL_STC));

    mllEntryPtr->last          = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],0,1));
    mllEntryPtr->isTunnelStart = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],1,1));
    mllEntryPtr->nextHopVlanId = (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],3,12);

    useVidx0      = U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],15,1);
    targetIsTrunk = U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],16,1);

    if(useVidx0 == 1)
    {
        mllEntryPtr->nextHopInterface.type = CPSS_INTERFACE_VIDX_E;
        mllEntryPtr->nextHopInterface.vidx =
            (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],16,12);

        if(mllEntryPtr->nextHopInterface.vidx == 0xFFF)
        {
            mllEntryPtr->nextHopInterface.type = CPSS_INTERFACE_VID_E;
            mllEntryPtr->nextHopInterface.vlanId = 0xFFF;
        }
    }
    else if(targetIsTrunk == 1)
    {
        mllEntryPtr->nextHopInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllEntryPtr->nextHopInterface.trunkId =
            (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],21,7);
    }
    else
    {
        mllEntryPtr->nextHopInterface.type = CPSS_INTERFACE_PORT_E;
        mllEntryPtr->nextHopInterface.devPort.portNum =
            (GT_U8)U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],17,6);
        mllEntryPtr->nextHopInterface.devPort.hwDevNum  =
            (GT_U8)U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],23,5);
        PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(mllEntryPtr->nextHopInterface));
    }

    switch(U32_GET_FIELD_MAC(hwDataPtr[relativeLocation],29,3))
    {
        case 2:
            mllEntryPtr->mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            mllEntryPtr->mllRPFFailCommand = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            mllEntryPtr->mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        case 5:
            mllEntryPtr->mllRPFFailCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case 6:
            mllEntryPtr->mllRPFFailCommand = CPSS_PACKET_CMD_BRIDGE_E;
            break;
        case 0:/* reset value of HW */
            mllEntryPtr->mllRPFFailCommand = CPSS_PACKET_CMD_FORWARD_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    mllEntryPtr->nextHopTunnelPointer = U32_GET_FIELD_MAC(hwDataPtr[relativeLocation+1],0,10);
    mllEntryPtr->ttlHopLimitThreshold = (GT_U16)U32_GET_FIELD_MAC(hwDataPtr[relativeLocation+1],11,8);
    mllEntryPtr->excludeSrcVlan       = U32_GET_FIELD_MAC(hwDataPtr[relativeLocation+1],19,1);

    if (isFirstMll)
    {
        mllEntryPtr->nextHopTunnelPointer |= U32_GET_FIELD_MAC(hwDataPtr[1],20,2) << 10;
    }
    else
    {
        mllEntryPtr->nextHopTunnelPointer |= U32_GET_FIELD_MAC(hwDataPtr[1],22,2) << 10;
    }

    /* tunnelStartPassengerType should be handled only in supported devices and
       when isTunnelStart is GT_TRUE*/
    if (mllEntryPtr->isTunnelStart == GT_TRUE)
    {
        PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_ATTR_TUNNEL_START_PASS_TYPE_MAC(
                mllEntryPtr->tunnelStartPassengerType,
                (U32_GET_FIELD_MAC(hwDataPtr[relativeLocation+1],10,1)));
    }

    return GT_OK;
}

/**
* @internal sip5IpMllStruct2HwFormatSubSection function
* @endinternal
*
* @brief   SIP5 : This function converts a given ip Mll struct to the hardware format.
*         convert only sub section (0 or 1)
*         for section 1 , it also set the 'next MLL'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] mllPairEntryPtr          - (points to ) the entry to be converted.
* @param[in] sectionIndex             - section index 0 or 1.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
* @param[out] hwMaskPtr                - the mask for hwDataPtr. Bits of HW entry fields
*                                      updated in the function are raised. Other mask's
*                                      bits are not affected
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5IpMllStruct2HwFormatSubSection
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr,
    OUT GT_U32                                      *hwMaskPtr,
    IN  GT_U32                                       sectionIndex
)
{
    GT_U32  value;/*tmp value to set to HW*/
    CPSS_DXCH_IP_MLL_STC *currSwPtr;/*pointer to current SW entry (in each HW line there are 2 entries)*/
    SIP5_IP_MLL_TABLE_FIELDS_ENT fieldId; /* IP MLL table field name */

    /* pointer to current SW info */
    currSwPtr = sectionIndex ? &mllPairEntryPtr->secondMllNode :
                               &mllPairEntryPtr->firstMllNode ;

    if(sectionIndex)
    {
        fieldId = SIP5_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR_E;
        value = mllPairEntryPtr->nextPointer;
        SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
    }

    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_LAST_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_LAST_0_E);
    value = currSwPtr->last;
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0_E);
    switch(currSwPtr->mllRPFFailCommand)
    {
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            value = 2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            value = 3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            value = 4;
            break;
        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            value = 5;
            break;
        case CPSS_PACKET_CMD_BRIDGE_E:
            value = 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);


    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_0_E);
    value = currSwPtr->nextHopVlanId;
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_0_E);
    value = ((currSwPtr->nextHopInterface.type == CPSS_INTERFACE_VIDX_E) ||
             (currSwPtr->nextHopInterface.type == CPSS_INTERFACE_VID_E)) ? 1 : 0;
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    if(value)
    {
        if(currSwPtr->nextHopInterface.type == CPSS_INTERFACE_VID_E)
        {
            value = 0xFFF;
        }
        else
        {
            value = currSwPtr->nextHopInterface.vidx;
        }

        fieldId = (sectionIndex ?
                   SIP5_IP_MLL_TABLE_FIELDS_EVIDX_1_E :
                   SIP5_IP_MLL_TABLE_FIELDS_EVIDX_0_E);
        SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    }
    else
    {
        fieldId = (sectionIndex ?
                   SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E :
                   SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E);
        value = (currSwPtr->nextHopInterface.type == CPSS_INTERFACE_TRUNK_E) ? 1 :0;
        SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);


        if(currSwPtr->nextHopInterface.type == CPSS_INTERFACE_TRUNK_E)
        {
            fieldId = (sectionIndex ?
                       SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E :
                       SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E);
            value = currSwPtr->nextHopInterface.trunkId;
            SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
        }
        else
        if(currSwPtr->nextHopInterface.type == CPSS_INTERFACE_PORT_E)
        {
            fieldId = (sectionIndex ?
                       SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_1_E :
                       SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_0_E);
            value = currSwPtr->nextHopInterface.devPort.portNum;
            SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);


            fieldId = (sectionIndex ?
                       SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_1_E :
                       SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_0_E);
            value = currSwPtr->nextHopInterface.devPort.hwDevNum;
            SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0_E);
    value = BOOL2BIT_MAC(currSwPtr->isTunnelStart);
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    if(value)
    {
        fieldId = (sectionIndex ?
                   SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1_E :
                   SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0_E);
        value = currSwPtr->nextHopTunnelPointer;
        SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

        fieldId = (sectionIndex ?
                   SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1_E :
                   SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0_E);
        switch(currSwPtr->tunnelStartPassengerType)
        {
            case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                value = 0;
                break;
            case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                value = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    }

    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0_E);
    value = currSwPtr->ttlHopLimitThreshold;
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);


    fieldId = (sectionIndex ?
               SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1_E :
               SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0_E);
    value = BOOL2BIT_MAC(currSwPtr->excludeSrcVlan);
    SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    return GT_OK;
}

/**
* @internal sip5IpMllHw2StructFormatSubSection function
* @endinternal
*
* @brief   SIP5 : This function converts a given ip Mll hardware format to struct (SW) .
*         convert only sub section (0 or 1)
*         for section 1 , it also set the 'next MLL'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwDataPtr                - The entry in the HW format , the need to be converted to SW.
* @param[in] sectionIndex             - section index 0 or 1.
*
* @param[out] mllPairEntryPtr          - (points to ) the converted entry.
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5IpMllHw2StructFormatSubSection
(
    IN GT_U8                                        devNum,
    OUT  CPSS_DXCH_IP_MLL_PAIR_STC                  *mllPairEntryPtr,
    IN GT_U32                                       *hwDataPtr,
    IN  GT_U32                                       sectionIndex
)
{
    GT_U32  value;/*tmp value to get from HW*/
    CPSS_DXCH_IP_MLL_STC *currSwPtr;/*pointer to current SW entry (in each HW line there are 2 entries)*/

    /* pointer to current SW info */
    currSwPtr = sectionIndex ? &mllPairEntryPtr->secondMllNode :
                               &mllPairEntryPtr->firstMllNode ;

    if(sectionIndex)
    {
        SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            SIP5_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR_E
            ,value);
        mllPairEntryPtr->nextPointer = (GT_U16)value;
    }

    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_LAST_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_LAST_0_E
        ,value);
    currSwPtr->last = BIT2BOOL_MAC(value);

    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0_E
        ,value);
    switch(value)
    {
        case 2:
            currSwPtr->mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            currSwPtr->mllRPFFailCommand = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            currSwPtr->mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        case 5:
            currSwPtr->mllRPFFailCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case 6:
            currSwPtr->mllRPFFailCommand = CPSS_PACKET_CMD_BRIDGE_E;
            break;
        case 0:/* reset value of HW */
            currSwPtr->mllRPFFailCommand = CPSS_PACKET_CMD_FORWARD_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_0_E
        ,value);
    currSwPtr->nextHopVlanId = (GT_U16)value;


    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_0_E
        ,value);

    if(value)
    {
        SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_IP_MLL_TABLE_FIELDS_EVIDX_1_E :
                SIP5_IP_MLL_TABLE_FIELDS_EVIDX_0_E
            ,value);

        if(value != 0xFFF)
        {
            currSwPtr->nextHopInterface.type = CPSS_INTERFACE_VIDX_E;
            currSwPtr->nextHopInterface.vidx = (GT_U16)value;
        }
        else
        {
            currSwPtr->nextHopInterface.type = CPSS_INTERFACE_VID_E;
            currSwPtr->nextHopInterface.vlanId = 0xFFF;
        }
    }
    else
    {
        SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E :
                SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E
            ,value);

        if(value)
        {
            currSwPtr->nextHopInterface.type = CPSS_INTERFACE_TRUNK_E;
            SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                sectionIndex ?
                    SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E :
                    SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E
                ,value);
            currSwPtr->nextHopInterface.trunkId = (GT_TRUNK_ID)value;
        }
        else
        {
            currSwPtr->nextHopInterface.type = CPSS_INTERFACE_PORT_E;

            SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                sectionIndex ?
                    SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_1_E :
                    SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_0_E
                ,value);
            currSwPtr->nextHopInterface.devPort.portNum = value;

            SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                sectionIndex ?
                    SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_1_E :
                    SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_0_E
                ,value);
            currSwPtr->nextHopInterface.devPort.hwDevNum = value;
        }
    }

    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0_E
        ,value);
    currSwPtr->isTunnelStart = BIT2BOOL_MAC(value);

    if(value)
    {
        SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1_E :
                SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0_E
            ,value);
        currSwPtr->nextHopTunnelPointer = value;

        SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1_E :
                SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0_E
            ,value);

        PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_ATTR_TUNNEL_START_PASS_TYPE_MAC(
                currSwPtr->tunnelStartPassengerType,
                value);
    }

    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0_E
        ,value);
    currSwPtr->ttlHopLimitThreshold = (GT_U16)value;

    SIP5_IP_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1_E :
            SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0_E
        ,value);
    currSwPtr->excludeSrcVlan = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal sip5IpMllStruct2HwFormat function
* @endinternal
*
* @brief   SIP5 : This function converts a given ip Mll struct to the hardware format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mllPairForm              - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - (points to ) the entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
* @param[out] hwMaskPtr                - the mask for hwDataPtr. Bits of HW entry fields
*                                      updated in the function are raised. Other mask's
*                                      bits are not affected
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5IpMllStruct2HwFormat
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    IN  CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr,
    OUT GT_U32                                      *hwMaskPtr
)
{
    GT_STATUS rc = GT_BAD_PARAM;/* indication that none of the cases used */

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 0 */
        rc = sip5IpMllStruct2HwFormatSubSection(
            devNum, mllPairEntryPtr, hwDataPtr, hwMaskPtr, 0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 1 + next pointer */
        rc = sip5IpMllStruct2HwFormatSubSection(
            devNum, mllPairEntryPtr, hwDataPtr, hwMaskPtr, 1);
    }

    return rc;
}


/**
* @internal sip5IpMllHw2StructFormat function
* @endinternal
*
* @brief   SIP5 : This function converts a given ip Mll hardware format to struct (SW) .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mllPairForm              - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] hwDataPtr                - The entry in the HW format , the need to be converted to SW.
*
* @param[out] mllPairEntryPtr          - (points to ) the converted entry.
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5IpMllHw2StructFormat
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    OUT  CPSS_DXCH_IP_MLL_PAIR_STC                  *mllPairEntryPtr,
    IN GT_U32                                       *hwDataPtr
)
{
    GT_STATUS rc = GT_BAD_PARAM;/* indication that none of the cases used */

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 0 */
        rc = sip5IpMllHw2StructFormatSubSection(devNum,
                mllPairEntryPtr,hwDataPtr,0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 1 + next pointer */
        rc = sip5IpMllHw2StructFormatSubSection(devNum,
                mllPairEntryPtr,hwDataPtr,1);
    }

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
static GT_STATUS internal_cpssDxChIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)
{
    PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC    lpmEcmpEntry;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ecmpEntryPtr);

    lpmEcmpEntry.randomEnable = ecmpEntryPtr->randomEnable;
    lpmEcmpEntry.numOfPaths = ecmpEntryPtr->numOfPaths;
    lpmEcmpEntry.nexthopBaseIndex = ecmpEntryPtr->routeEntryBaseIndex;
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        switch (ecmpEntryPtr->multiPathMode) {
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
            lpmEcmpEntry.multiPathMode = PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E;
            break;
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
            lpmEcmpEntry.multiPathMode = PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return prvCpssDxChLpmHwEcmpEntryWrite(devNum, ecmpEntryIndex, &lpmEcmpEntry);
}

/**
* @internal cpssDxChIpEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
GT_STATUS cpssDxChIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ecmpEntryIndex, ecmpEntryPtr));

    rc = internal_cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, ecmpEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ecmpEntryIndex, ecmpEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
static GT_STATUS internal_cpssDxChIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)
{
    PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC    lpmEcmpEntry;
    GT_STATUS                           rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ecmpEntryPtr);

    rc = prvCpssDxChLpmHwEcmpEntryRead(devNum, ecmpEntryIndex, &lpmEcmpEntry);
    if (rc == GT_OK)
    {
        ecmpEntryPtr->randomEnable = lpmEcmpEntry.randomEnable;
        ecmpEntryPtr->numOfPaths = lpmEcmpEntry.numOfPaths;
        ecmpEntryPtr->routeEntryBaseIndex = lpmEcmpEntry.nexthopBaseIndex;
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
        {
            switch (lpmEcmpEntry.multiPathMode) {
            case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E:
                ecmpEntryPtr->multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
                break;
            case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E:
                ecmpEntryPtr->multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    return rc;
}

/**
* @internal cpssDxChIpEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
GT_STATUS cpssDxChIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ecmpEntryIndex, ecmpEntryPtr));

    rc = internal_cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, ecmpEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ecmpEntryIndex, ecmpEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpHashNumBitsSet function
* @endinternal
*
* @brief   Set the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism. Those parameters are used to select the ECMP member
*         within the ECMP block according to one of the following formulas:
*         - when randomEnable bit in the ECMP entry is GT_FALSE:
*         ECMP member offset =
*         ((size of ECMP block) (the value received between [startBit] and
*         [startBit + numOfBits - 1])) >> numOfBits
*         - when randomEnable bit in the ECMP entry is GT_TRUE:
*         ECMP member offset =
*         ((size of ECMP block) (random seed value)) >> numOfBits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
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
*       startBit and numOfBits values may also be changed by
*       cpssDxChTrunkHashNumBitsSet
*
*/
static GT_STATUS internal_cpssDxChIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return cpssDxChTrunkHashNumBitsSet(devNum,
                                               CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E,
                                               startBit,
                                               numOfBits);
}

/**
* @internal cpssDxChIpEcmpHashNumBitsSet function
* @endinternal
*
* @brief   Set the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism. Those parameters are used to select the ECMP member
*         within the ECMP block according to one of the following formulas:
*         - when randomEnable bit in the ECMP entry is GT_FALSE:
*         ECMP member offset =
*         ((size of ECMP block) (the value received between [startBit] and
*         [startBit + numOfBits - 1])) >> numOfBits
*         - when randomEnable bit in the ECMP entry is GT_TRUE:
*         ECMP member offset =
*         ((size of ECMP block) (random seed value)) >> numOfBits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
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
*       startBit and numOfBits values may also be changed by
*       cpssDxChTrunkHashNumBitsSet
*
*/
GT_STATUS cpssDxChIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpHashNumBitsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startBit, numOfBits));

    rc = internal_cpssDxChIpEcmpHashNumBitsSet(devNum, startBit, numOfBits);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startBit, numOfBits));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpHashNumBitsGet function
* @endinternal
*
* @brief   Get the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism. Those parameters are used to select the ECMP member
*         within the ECMP block according to one of the following formulas:
*         - when randomEnable bit in the ECMP entry is GT_FALSE:
*         ECMP member offset =
*         ((size of ECMP block) (the value received between [startBit] and
*         [startBit + numOfBits - 1])) >> numOfBits
*         - when randomEnable bit in the ECMP entry is GT_TRUE:
*         ECMP member offset =
*         ((size of ECMP block) (random seed value)) >> numOfBits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
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
static GT_STATUS internal_cpssDxChIpEcmpHashNumBitsGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(startBitPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfBitsPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChTrunkHashBitsSelectionGet(devNum,
                                                    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E,
                                                    startBitPtr,
                                                    numOfBitsPtr);
}

/**
* @internal cpssDxChIpEcmpHashNumBitsGet function
* @endinternal
*
* @brief   Get the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism. Those parameters are used to select the ECMP member
*         within the ECMP block according to one of the following formulas:
*         - when randomEnable bit in the ECMP entry is GT_FALSE:
*         ECMP member offset =
*         ((size of ECMP block) (the value received between [startBit] and
*         [startBit + numOfBits - 1])) >> numOfBits
*         - when randomEnable bit in the ECMP entry is GT_TRUE:
*         ECMP member offset =
*         ((size of ECMP block) (random seed value)) >> numOfBits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
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
GT_STATUS cpssDxChIpEcmpHashNumBitsGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpHashNumBitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startBitPtr, numOfBitsPtr));

    rc = internal_cpssDxChIpEcmpHashNumBitsGet(devNum, startBitPtr, numOfBitsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startBitPtr, numOfBitsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpHashSeedValueSet function
* @endinternal
*
* @brief   Set the "seed value" parameter used by the L3 ECMP hash mechanism
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] seed                     - the " value" parameter used by the L3 ECMP hash
*                                      (APPLICABLE RANGES: 1..0xffffffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong seed
*
* @note The "seed value" parameter is relevant only for ECMP entries that are
*       configured with randomEnable == GT_TRUE.
*       0 is illegal seed value.
*
*/
static GT_STATUS internal_cpssDxChIpEcmpHashSeedValueSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       seed
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (seed == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* set the seed value */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.ECMPRoutingConfig.ECMPSeed;
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, seed);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* in order that the new seed value will be uploaded we need to set
       <Seed Load Enable> to 1 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.ECMPRoutingConfig.ECMPConfig;
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
}

/**
* @internal cpssDxChIpEcmpHashSeedValueSet function
* @endinternal
*
* @brief   Set the "seed value" parameter used by the L3 ECMP hash mechanism
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] seed                     - the " value" parameter used by the L3 ECMP hash
*                                      (APPLICABLE RANGES: 1..0xffffffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong seed
*
* @note The "seed value" parameter is relevant only for ECMP entries that are
*       configured with randomEnable == GT_TRUE.
*       0 is illegal seed value.
*
*/
GT_STATUS cpssDxChIpEcmpHashSeedValueSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       seed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpHashSeedValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, seed));

    rc = internal_cpssDxChIpEcmpHashSeedValueSet(devNum, seed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, seed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpHashSeedValueGet function
* @endinternal
*
* @brief   Get the "seed value" parameter used by the L3 ECMP hash mechanism
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] seedPtr                  - (pointer to) the "seed value" parameter used by the L3
*                                      ECMP hash
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The "seed value" parameter is relevant only for ECMP entries that are
*       configured with randomEnable == GT_TRUE.
*
*/
static GT_STATUS internal_cpssDxChIpEcmpHashSeedValueGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *seedPtr
)
{
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(seedPtr);

    /* get the seed value */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.ECMPRoutingConfig.ECMPSeed;
    return prvCpssHwPpReadRegister(devNum, regAddr, seedPtr);
}

/**
* @internal cpssDxChIpEcmpHashSeedValueGet function
* @endinternal
*
* @brief   Get the "seed value" parameter used by the L3 ECMP hash mechanism
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] seedPtr                  - (pointer to) the "seed value" parameter used by the L3
*                                      ECMP hash
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The "seed value" parameter is relevant only for ECMP entries that are
*       configured with randomEnable == GT_TRUE.
*
*/
GT_STATUS cpssDxChIpEcmpHashSeedValueGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *seedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpHashSeedValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, seedPtr));

    rc = internal_cpssDxChIpEcmpHashSeedValueGet(devNum, seedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, seedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet function
* @endinternal
*
* @brief   Function to get the number of ECMP indirect table 'logical' entries
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   -  the device number
*
* @retval The number of ECMP indirect table 'logical' entries.
*
* @note none.
*
*/
GT_U32  prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(
    IN  GT_U8                       devNum
)
{
    GT_U32  numLines;
    if(!PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        return 0;
    }

    numLines = PRV_TABLE_INFO_PTR_GET_MAC(devNum,
            CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E)->maxNumOfEntries;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* 4 entries in HW line */
        numLines *= 4;
    }

    return numLines;
}

/**
* @internal internal_cpssDxChIpEcmpIndirectNextHopEntrySet function
* @endinternal
*
* @brief   This function sets ECMP/QOS indirect table entry
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] indirectIndex            - index of ECMP/QOS indirect table
* @param[in] nextHopIndex             - index of the next hope at Next Hope Table
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS internal_cpssDxChIpEcmpIndirectNextHopEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U32                       indirectIndex,
    IN GT_U32                       nextHopIndex
)
{
    GT_STATUS   rc;
    GT_U32  hwData;
    GT_U32  maxNumOfIndirectEntries;
    GT_U32  maxNumOfNextHopEntries;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E);

    maxNumOfIndirectEntries = prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(devNum);

    /* there are only 15 bits the field*/
    maxNumOfNextHopEntries = MIN(BIT_15, PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop);

    if (nextHopIndex >= maxNumOfNextHopEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "nextHopIndex[0x%x] must be in range[0..0x%x]",
            nextHopIndex,
            maxNumOfNextHopEntries-1);
    }

    if (indirectIndex >= maxNumOfIndirectEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryIndex [0x%x] must be in range[0..0x%x]",
            indirectIndex,
            maxNumOfIndirectEntries -1);
    }

    hwData = nextHopIndex;

    rc = prvCpssDxChWriteTableEntry(devNum,
         CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
         indirectIndex,
         &hwData);

    return rc;
}

/**
* @internal cpssDxChIpEcmpIndirectNextHopEntrySet function
* @endinternal
*
* @brief   This function set ECMP/QOS indirect table entry
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] indirectIndex            - index of ECMP/QOS indirect table
* @param[in] nextHopIndex             - address of the next hope at Next Hope Table
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChIpEcmpIndirectNextHopEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U32                       indirectIndex,
    IN GT_U32                       nextHopIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpIndirectNextHopEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, indirectIndex,nextHopIndex));

    rc = internal_cpssDxChIpEcmpIndirectNextHopEntrySet(devNum, indirectIndex,nextHopIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, indirectIndex,nextHopIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpEcmpIndirectNextHopEntryGet function
* @endinternal
*
* @brief   This function get ECMP/QOS indirect table entry
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] indirectIndex            - index of ECMP/QOS indirect table
* @param[out] nextHopIndexPtr         - address of the next hope at Next Hope Table
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS internal_cpssDxChIpEcmpIndirectNextHopEntryGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       indirectIndex,
    IN GT_U32                       *nextHopIndexPtr
)
{
    GT_STATUS   rc;
    GT_U32      maxNumOfIndirectEntries;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E);

    maxNumOfIndirectEntries = prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(devNum);

    if (indirectIndex >= maxNumOfIndirectEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryIndex [0x%x] must be in range[0..0x%x]",
            indirectIndex,
            maxNumOfIndirectEntries -1);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
         CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
         indirectIndex,
         nextHopIndexPtr);

    return rc;
}

/**
* @internal cpssDxChIpEcmpIndirectNextHopEntryGet function
* @endinternal
*
* @brief   This function get ECMP/QOS indirect table entry
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] indirectIndex            - index of ECMP/QOS indirect table
* @param[out] nextHopIndexPtr         - address of the next hope at Next Hope Table
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChIpEcmpIndirectNextHopEntryGet
(
    IN GT_U8                         devNum,
    IN GT_U32                        indirectIndex,
    OUT GT_U32                       *nextHopIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpEcmpIndirectNextHopEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, indirectIndex,nextHopIndexPtr));

    rc = internal_cpssDxChIpEcmpIndirectNextHopEntryGet(devNum, indirectIndex,nextHopIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, indirectIndex,nextHopIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




