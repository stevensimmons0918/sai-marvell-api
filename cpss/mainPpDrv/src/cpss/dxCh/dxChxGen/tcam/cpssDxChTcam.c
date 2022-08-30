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
* @file cpssDxChTcam.c
*
* @brief CPSS TCAM implementation.
*
* @version   11
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcamLog.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_PORT_DIR_DXCH_TCAM_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChTcamDir._var,_value)

#define PRV_SHARED_PORT_DIR_DXCH_TCAM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChTcamDir._var)

/* there are 3K entries in floor
   NOTE: currently this number is not 'per device'
*/
#define NUM_RULES_IN_FLOOR_MAC(_devNum)     CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS

/* macro to roundup a number that is divided so :
    for example :
    3 / 2 is round up to 2 and not round down to 1
*/
#define ROUNDUP_DIV_MAC(_number , _divider)             \
    (((_number) + ((_divider)-1)) / (_divider))

/* the number of rules derived from the 'max' of 'TTI'/'PCL' rules defined */
/* as we need to support the Ironman-S that hold no TTI connection to the TCAM ! */
#define NUM_TCAM_RULES_MAC(_devNum) \
    ((PCL_NUM_RULES_MAX_AS_TTI_AUTO_CALC != PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policyTcamRaws) ? \
        (MAX(                           \
         PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policyTcamRaws , \
         PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm)):    \
         PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm)

/* get the number of floors needed for the number of rules in the tcam */
#define CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(_devNum)                          \
    ROUNDUP_DIV_MAC(                                                        \
        NUM_TCAM_RULES_MAC(_devNum),                                        \
        NUM_RULES_IN_FLOOR_MAC(_devNum))

/**
* @internal prvCpssDxChTcamRuleIndexToEntryNumber function
* @endinternal
*
* @brief   Converts TCAM rule global index to entry number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] entryNumberXPtr          - (pointer to) entry number in TCAM for X format
* @param[out] entryNumberYPtr          - (pointer to) entry number in TCAM for Y format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamRuleIndexToEntryNumber
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT GT_U32                              *entryNumberXPtr,
    OUT GT_U32                              *entryNumberYPtr
)
{
    GT_U32  floorNum;           /* floor number in TCAM         */
    GT_U32  bankNum;            /* bank number in TCAM          */
    GT_U32  lineIndex;          /* index in bank                */
    GT_U32  mod;
    GT_U32  entryNumber;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(entryNumberXPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryNumberYPtr);

    floorNum = (GT_U32)(index / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    mod = (GT_U32)(index % CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    lineIndex = (GT_U32)(mod / CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);
    bankNum = (GT_U32)(mod % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);

    /* Entry Number = {floor_num[3:0], bank_index[7:0], bank_num[3:0]} */
    entryNumber = ((floorNum & 0xF) << 12) | ((lineIndex & 0xFF) << 4) | (bankNum & 0xF);

    /* Entry Number X = {floor_num[3:0], bank_index[7:0], bank_num[3:0], 0} */
    *entryNumberXPtr  =  entryNumber << 1;

    /* Entry Number Y = {floor_num[3:0], bank_index[7:0], bank_num[3:0], 1} */
    *entryNumberYPtr  =  (entryNumber << 1) | 1;

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamRuleIndexToSip5HwEntryNumber function
* @endinternal
*
* @brief   Converts TCAM rule global index to HW X entry number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] index                    - global line  in TCAM. Index may be more than
*                                      real TCAM size.
*
* @param[out] entryNumberXPtr          - (pointer to) entry number in TCAM for X format
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDxChTcamRuleIndexToSip5HwEntryNumber
(
    IN  GT_U32                              index,
    OUT GT_U32                              *entryNumberXPtr
)
{
    GT_U32  floorNum;           /* floor number in TCAM         */
    GT_U32  bankNum;            /* bank number in TCAM          */
    GT_U32  lineIndex;          /* index in bank                */
    GT_U32  mod;
    GT_U32  entryNumber;

    floorNum = (GT_U32)(index / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    mod = (GT_U32)(index % CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    lineIndex = (GT_U32)(mod / CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);
    bankNum = (GT_U32)(mod % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);

    /* Entry Number = {floor_num[3:0], bank_index[7:0], bank_num[3:0]} */
    entryNumber = ((floorNum) << 12) | ((lineIndex & 0xFF) << 4) | (bankNum & 0xF);

    /* Entry Number X = {floor_num[3:0], bank_index[7:0], bank_num[3:0], 0} */
    *entryNumberXPtr  =  entryNumber << 1;

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamSip5HwEntryToRuleIndexConvert function
* @endinternal
*
* @brief   Converts SIP5 TCAM HW entry index to global rule index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] hwIndex                  - entry HW number in TCAM
*
* @param[out] ruleIndexPtr             - (pointer to) global rule index in TCAM.
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDxChTcamSip5HwEntryToRuleIndexConvert
(
    IN  GT_U32                              hwIndex,
    OUT GT_U32                              *ruleIndexPtr
)
{
    GT_U32      tcamBank;  /* TCAM bank number */
    GT_U32      tcamRow;   /* TCAM row number */
    GT_U32      tcamFloor; /* TCAM floor number */

    /* HW Entry Number = {floor_num[3:0], array_addr[7:0], bank[3:0], XY} */
    tcamBank = ((hwIndex >> 1) & 0xF);
    tcamRow =  ((hwIndex >> 5) & 0xFF);
    tcamFloor = hwIndex >> 13;

    *ruleIndexPtr = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS * tcamFloor +
                    CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * tcamRow + tcamBank;

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamRuleRangeToHwSpaceConvert function
* @endinternal
*
* @brief   Converts range of TCAM rules to HW space range.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] firstRuleIndex           - global first rule index in TCAM.
* @param[in] numOfRules               - number of TCAM rules
*
* @param[out] firstHwEntryIndexPtr     - (pointer to) first HW entry number in TCAM for X format
* @param[out] hwEntriesNumberPtr       - (pointer to) number of HW entries. HW Entries number
*                                      may be more than TCAM real size. The caller function
*                                      must to take care of index wraparound.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamRuleRangeToHwSpaceConvert
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              firstRuleIndex,
    IN  GT_U32                              numOfRules,
    OUT GT_U32                              *firstHwEntryIndexPtr,
    OUT GT_U32                              *hwEntriesNumberPtr
)
{
    GT_STATUS rc;
    GT_U32    nextHwEntryIndex; /* index of next after range HW entry */
    GT_U32    tmp; /* TMP variable */

    /* each logical entry has two HW (one X one Y). Some holes are in HW range also.
      HW Entry Number = {floor_num[3:0], array_addr[7:0], bank[3:0], XY},
      where the banks 12-15 at each floor do not exists. */

    /* convert to X HW entry index */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, firstRuleIndex, firstHwEntryIndexPtr, &tmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* use special function without rule index check to calculate number of HW X/Y entires */
    rc = prvCpssDxChTcamRuleIndexToSip5HwEntryNumber(numOfRules + firstRuleIndex, &nextHwEntryIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* HW Entries number may be more than TCAM real size.
       the caller function must to take care of index wraparound. */
    *hwEntriesNumberPtr = nextHwEntryIndex - (*firstHwEntryIndexPtr);
    return GT_OK;
}

/**
* @internal prvCpssDxChTcamRuleIndexCheck_stateErrorLog function
* @endinternal
*
* @brief   Checks if an gloabl TCAM index is valid for a TCAM rule's size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - size of rule
* @param[in] doCpssErrorLog           - indication if 'fail' should cause 'CPSS error log'
*                                       as the caller may treat the fail as valid state !!!
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChTcamRuleIndexCheck_stateErrorLog
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_BOOL                             doCpssErrorLog
)
{
    GT_U32 ruleColumn;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);

    ruleColumn = (GT_U32)(index % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);

    switch (ruleSize)
    {
    case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* ruleColumn must be odd becase there is only one action entry per pair of even and odd rules */
            if ((ruleColumn % 2) != 1)
            {
                if(doCpssErrorLog)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[10B] , ruleColumn[%d] must be odd",index,ruleColumn);
                }
                else
                {
                    return /* not error for the CPSS LOG */ GT_BAD_PARAM;
                }
            }
            break;
        }

        /* do nothing - rule can start in each bank */
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
        /* ruleColumn must divide by 2 */
        if ((ruleColumn % 2) != 0)
        {
            if(doCpssErrorLog)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[20B] , ruleColumn[%d] must divide by 2",index,ruleColumn);
            }
            else
            {
                return /* not error for the CPSS LOG */ GT_BAD_PARAM;
            }
        }
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
        /* ruleColumn must divide by 3 */
        if ((ruleColumn % 3) != 0)
        {
            if(doCpssErrorLog)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[30B] , ruleColumn[%d] must divide by 3",index,ruleColumn);
            }
            else
            {
                return /* not error for the CPSS LOG */ GT_BAD_PARAM;
            }
        }
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
    case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
    case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
        if ((PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            && (ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_40_B_E))
        {
            /* ruleColumn must divide by 4 */
            if ((ruleColumn % 4) != 0)
            {
                if(doCpssErrorLog)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(
                        GT_BAD_PARAM, "index[%d] , ruleSize[40B] , ruleColumn[%d] must divide by 4",index,ruleColumn);
                }
                else
                {
                    return /* not error for the CPSS LOG */ GT_BAD_PARAM;
                }
            }
        }
        else if ((ruleColumn % 6) != 0)
        {
            /* ruleColumn must divide by 6 */
            if(doCpssErrorLog)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    GT_BAD_PARAM, "index[%d] , ruleSize[40B/50B/60B] , ruleColumn[%d] must divide by 6",index,ruleColumn);
            }
            else
            {
                return /* not error for the CPSS LOG */ GT_BAD_PARAM;
            }
        }
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
        /* ruleColumn must divide by 12 */
        if ((ruleColumn % 12) != 0)
        {
            if(doCpssErrorLog)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[80B] , ruleColumn[%d] must divide by 12",index,ruleColumn);
            }
            else
            {
                return /* not error for the CPSS LOG */ GT_BAD_PARAM;
            }
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamRuleIndexCheck function
* @endinternal
*
* @brief   Checks if an gloabl TCAM index is valid for a TCAM rule's size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamRuleIndexCheck
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize
)
{
    return prvCpssDxChTcamRuleIndexCheck_stateErrorLog(devNum,index,ruleSize,GT_TRUE/* CPSS error log needed */);
}


/**
* @internal prvCpssDxChTcamPortGroupRuleReadEntryCheckHa function
* @endinternal
*
* @brief   Reads the TCAM pattern/mask of single bank - supports HA and normal mode
*           If HA mode, ruleSize 7 is considered valid for ruleRead
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] haMode                   - HA mode enable if GT_TRUE(ruleSize 7 valid), else normal mode
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
* @param[out] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChTcamPortGroupRuleReadEntryCheckHa
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             haMode,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamEntryPatternPtr,
    OUT GT_U32                              *tcamEntryMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryMaskPtr);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read first bank to get the rule's length */
    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberX,
                                             entryDataX);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberY,
                                             entryDataY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* first bank size is used as validity for the whole rule.
       when X=0 and Y=1 results--> Match 0,
       when X=1 and Y=1 results--> Always Miss. */
    U32_GET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,value);
    *validPtr = !(BIT2BOOL_MAC(value));

    /* convert data from X/Y format to pattern/mask */
    for (ii = 0 ; ii <= 2 ; ii++)
    {
        entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
        entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
    }

    U32_GET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);


    if(haMode)
    {
        if (keySize == 6)
        {
            numOfBanks = 1;
            *ruleSizePtr = PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS;/* Invalid Tcam Rule Size */
        }
        else if (keySize == 7)
            numOfBanks = 8;
        else
            numOfBanks = keySize + 1;

        if(keySize!=6)
            PRV_CPSS_DXCH_TCAM_CONVERT_NUM_OF_BANKS_TO_RULE_SIZE_VAL_MAC(*ruleSizePtr,numOfBanks);
    }
    else
    {
        if (keySize == 6)
        {
            /* 70-bytes format not supported - it used only by HA as stamp */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
        }
        if (keySize == 7)
            numOfBanks = 8;
        else
            numOfBanks = keySize + 1;
        PRV_CPSS_DXCH_TCAM_CONVERT_NUM_OF_BANKS_TO_RULE_SIZE_VAL_MAC(*ruleSizePtr,numOfBanks);

        rc = prvCpssDxChTcamRuleIndexCheck_stateErrorLog(devNum,index,*ruleSizePtr,
            GT_FALSE /* no CPSS error log (as we ignore the fail) */);
        if(rc != GT_OK)
        {
            *validPtr = GT_FALSE;
            return GT_OK;
        }
    }

    /* copy first 64 bits of pattern/mask */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,32*ii,32,value);
        U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,32*ii,32,value);
    }
    ii = 2;
    /* copy next 16 bits of pattern/mask (total of 80 bits)*/
    U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,32*ii,16,value);
    U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,32*ii,16,value);

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleReadEntry function
* @endinternal
*
* @brief   Reads the TCAM pattern/mask of single bank
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
* @param[out] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamPortGroupRuleReadEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamEntryPatternPtr,
    OUT GT_U32                              *tcamEntryMaskPtr
)
{
    return prvCpssDxChTcamPortGroupRuleReadEntryCheckHa(devNum, portGroupsBmp, index, GT_FALSE,
                                                validPtr, ruleSizePtr, tcamEntryPatternPtr, tcamEntryMaskPtr);
}


/**
* @internal prvCpssDxChTcamPortGroupRuleInvalidateSingleEntry function
* @endinternal
*
* @brief   Writes the TCAM X-entry making a single bank invalid.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChTcamPortGroupRuleInvalidateSingleEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    /* entry to write (dataY)       */
    /* all bits of X-data ones, valid bit in Y-data of any rule is 1 - the rule will never match any key */
    GT_U32  entryDataX[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

    /* check parameters done by caller */

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)
         /* The bit#3 is used to manage validity of TCAM entry.
           The Y entry has constant bit #3 that is always 1 and assumed to be such without writting.
           The content of X entry bit #3 defines if entry also 1 and makes rule invalid. */
        rc =  prvCpssDxChPortGroupWriteTableEntry(
            devNum, portGroupId, CPSS_DXCH_SIP5_TABLE_TCAM_E, entryNumberX, entryDataX);
        if (rc != GT_OK)
        {
            return rc;
        }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleWriteSingleEntry function
* @endinternal
*
* @brief   Writes the TCAM mask and key of a single bank.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
* @param[in] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChTcamPortGroupRuleWriteSingleEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamEntryPatternPtr,
    IN  GT_U32                              *tcamEntryMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryMaskPtr);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,ruleSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,ruleSize);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_SET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
    U32_SET_FIELD_IN_ENTRY_MAC(entryMask,0,3,7);

    /* copy first 64 bits of pattern/mask */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
        U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
    }
    ii = 2;
    /* copy next 16 bits of pattern/mask (total of 80 bits)*/
    U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
    U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);

    /* convert pattern/mask to X/Y format */
    for (ii = 0; ii <= 2; ii++)
    {
        entryDataX[ii] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[ii], entryMask[ii]);
        entryDataY[ii] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[ii], entryMask[ii]);
    }

    /* first bank size is used as validity for the whole rule.
       when X=0 and Y=1 results--> Match 0,
       when X=1 and Y=1 results--> Always Miss. */
    U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,(1-BOOL2BIT_MAC(valid)));
    U32_SET_FIELD_IN_ENTRY_MAC(entryDataY,3,1,1); /* Y=1*/

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)
        /* The bit#3 is used to manage validity of TCAM entry.
           The Y entry has constant bit #3 that is always 1. The content of X entry
           bit #3 defines if entry valid or not. The X entry must be written after Y
           entry to ensure consistency of the whole X/Y entry.  */
        rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                 entryNumberY,
                                                 entryDataY);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                 entryNumberX,
                                                 entryDataX);
        if (rc != GT_OK)
        {
            return rc;
        }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)

    return rc;
}

/**
* @internal prvCpssDxChTcamWriteDaemonWaWait function
* @endinternal
*
* @brief   Wait some time for daemon disable/enable WA.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] waitCount                - number of register reads count for wait operation
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTcamWriteDaemonWaWait
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              waitCount
)
{
    GT_U32      regAddr;  /* register address  */
    GT_U32      regData;  /* register data     */
    GT_U32      ii;       /* iterator          */
    GT_STATUS   rc;

    /* use readable register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.TCAMInterruptsSummaryMask;

    /* read register several times to make delay */
    for (ii = 0; ii < waitCount; ii++)
    {
        /*  read register */
        rc = prvCpssHwPpReadRegister (devNum,regAddr,&regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal debug_cpssDxChTcamWaWaitCountSet function
* @endinternal
*
* @brief   Debug function to set WA wait counts.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] waEnableCount            - count for enable daemon wait
* @param[in] waDisableCount           - count for disable daemon wait
*                                       none
*/
GT_VOID debug_cpssDxChTcamWaWaitCountSet
(
    IN  GT_U32 waEnableCount,
    IN  GT_U32 waDisableCount
)
{
    PRV_SHARED_PORT_DIR_DXCH_TCAM_SRC_GLOBAL_VAR_SET(prvTcamWaEnableCount, waEnableCount);
    PRV_SHARED_PORT_DIR_DXCH_TCAM_SRC_GLOBAL_VAR_SET(prvTcamWaDisableCount, waDisableCount);
}

/* count for enable daemon wait operation in parity daemon WA */
#define PRV_CPSS_DXCH_TCAM_WA_ENABLE_COUNT_CNS \
    PRV_SHARED_PORT_DIR_DXCH_TCAM_SRC_GLOBAL_VAR_GET(prvTcamWaEnableCount)

/* count for disable daemon wait operation in parity daemon WA */
#define PRV_CPSS_DXCH_TCAM_WA_DISABLE_COUNT_CNS \
    PRV_SHARED_PORT_DIR_DXCH_TCAM_SRC_GLOBAL_VAR_GET(prvTcamWaDisableCount)

/**
* @internal prvCpssDxChTcamWriteDaemonWaExecute function
* @endinternal
*
* @brief   Execute disable/enable parity daemon WA for TCAM write operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] disableDaemon            - 1 - disable daemon
*                                      0 - enable daemon
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTcamWriteDaemonWaExecute
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              disableDaemon
)
{
    GT_U32      regAddr;  /* register address  */
    GT_STATUS   rc;

    if (disableDaemon == 0)
    {
        /* wait some time to guranty that TCAM write updated parity DB also
           before enable daemon. */
        rc = prvCpssDxChTcamWriteDaemonWaWait(devNum, PRV_CPSS_DXCH_TCAM_WA_ENABLE_COUNT_CNS);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.parityDaemonRegs.parityDaemonCtrl;

    /*  write to daemon control bit: 0 - enable, 1 - disable */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, disableDaemon);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (disableDaemon)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamGlobalConfig;

        /*  increase daemon's priority  */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* wait some time to guranty that all daemon request were executed by TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaWait(devNum, PRV_CPSS_DXCH_TCAM_WA_DISABLE_COUNT_CNS);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*  restore daemon's priority  */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleInvalidateEntry function
* @endinternal
*
* @brief   Writes the TCAM X-data of a single bank making the rule invalid.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamPortGroupRuleInvalidateEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index
)
{
    GT_STATUS rc;               /* function return code         */
    GT_STATUS rc1;              /* function return code         */
    GT_BOOL   doWriteParityWa;  /* execute write TCAM WA        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    doWriteParityWa = PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                            PRV_CPSS_DXCH_BOBCAT2_TCAM_WRITE_PARITY_ERROR_WA_E);

    if (doWriteParityWa)
    {
        /* disable parity daemon before write to TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChTcamPortGroupRuleInvalidateSingleEntry(
        devNum, portGroupsBmp, index);

    if (doWriteParityWa)
    {
        /* enable parity daemon after write to TCAM */
        rc1 = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 0);
        if (rc1 != GT_OK)
        {
            /* return rc if it's not OK */
            return (rc != GT_OK)? rc : rc1;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleWriteEntry function
* @endinternal
*
* @brief   Writes the TCAM mask and key of a single bank.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
* @param[in] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamPortGroupRuleWriteEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamEntryPatternPtr,
    IN  GT_U32                              *tcamEntryMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_STATUS rc1;              /* function return code         */
    GT_BOOL   doWriteParityWa;  /* execute write TCAM WA        */

    doWriteParityWa = PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                            PRV_CPSS_DXCH_BOBCAT2_TCAM_WRITE_PARITY_ERROR_WA_E);

    if (doWriteParityWa)
    {
        /* disable parity daemon before write to TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChTcamPortGroupRuleWriteSingleEntry(devNum, portGroupsBmp,
                                                      index, valid, ruleSize,
                                                      tcamEntryPatternPtr,
                                                      tcamEntryMaskPtr);

    if (doWriteParityWa)
    {
        /* enable parity daemon after write to TCAM */
        rc1 = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 0);
        if (rc1 != GT_OK)
        {
            /* return rc if it's not OK */
            return (rc != GT_OK)? rc : rc1;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleWriteCheckHa function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line. - supports HA and Normal Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] haMode                   - HA mode enable if GT_TRUE(ruleSize 7 valid), else normal mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS prvCpssDxChTcamPortGroupRuleWriteCheckHa
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr,
    IN  GT_BOOL                             haMode
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  bank;               /* current bank written         */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  bankIdx;            /* bank iterator                */
    GT_U32  value;
    GT_U32  startBit;

    if(!haMode)
    {
        PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,ruleSize);
        PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks,ruleSize);
    }
    else
    {
        if(ruleSize == PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS)/* Invalid RuleSize */
            keySize = 6;
        else
            PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,ruleSize);
        numOfBanks = 1;/* update single bank */
    }

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* bank#0 validates whole entry. Therefore write banks in reverse
           order to ensure that bank#0 will be last. */
        bank = numOfBanks - 1;
        for (bankIdx = 0; bankIdx < numOfBanks; bankIdx++, bank--)
        {
             /* clear data */
            cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
            cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
            cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
            cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

            startBit = bank * 80;

            U32_SET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
            U32_SET_FIELD_IN_ENTRY_MAC(entryMask,0,3,7); /* excet match */

            /* copy first 64 bits of pattern/mask */
            for(ii = 0 ; ii < 2 ; ii++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),32,value);
                U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
                U32_GET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),32,value);
                U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
            }
            ii = 2;
            /* copy next 16 bits of pattern/mask (total of 80 bits)*/
            U32_GET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
            U32_GET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);

            /* convert pattern/mask to X/Y format */
            for (ii = 0; ii <= 2; ii++)
            {
                entryDataX[ii] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[ii], entryMask[ii]);
                entryDataY[ii] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[ii], entryMask[ii]);
            }

            /* first bank size is used as validity for the whole rule.
               when X=0 and Y=1 results--> Match 0,
               when X=1 and Y=1 results--> Always Miss. */
            if(bank == 0)
            {
                U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,(1-BOOL2BIT_MAC(valid)));
            }
            else
            {
                U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,0); /* X=0*/
            }

            U32_SET_FIELD_IN_ENTRY_MAC(entryDataY,3,1,1); /* Y=1*/

            /* The bit#3 is used to manage validity of TCAM entry.
               The Y entry has constant bit #3 that is always 1. The content of X entry
               bit #3 defines if entry valid or not. The X entry must be written after Y
               entry to ensure consistency of the whole X/Y entry.  */
            rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     entryNumberY + (2*bank),
                                                     entryDataY);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     entryNumberX + (2*bank),
                                                     entryDataX);
            if(rc != GT_OK)
            {
                return rc;
            }
       }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line. - supports HA and Normal Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS prvCpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
{
    return prvCpssDxChTcamPortGroupRuleWriteCheckHa(devNum, portGroupsBmp, index, valid, ruleSize,
                                                tcamPatternPtr, tcamMaskPtr, GT_FALSE);
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_STATUS rc1;              /* function return code         */
    GT_BOOL   doWriteParityWa;  /* execute write TCAM WA        */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    CPSS_NULL_PTR_CHECK_MAC(tcamPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,ruleSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    doWriteParityWa = PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                            PRV_CPSS_DXCH_BOBCAT2_TCAM_WRITE_PARITY_ERROR_WA_E);

    if (doWriteParityWa)
    {
        /* disable parity daemon before write to TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChTcamPortGroupRuleWrite(devNum, portGroupsBmp, index, valid,
                                           ruleSize, tcamPatternPtr, tcamMaskPtr);

    if (doWriteParityWa)
    {
        /* enable parity daemon after write to TCAM */
        rc1 = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 0);
        if (rc1 != GT_OK)
        {
            /* return rc if it's not OK */
            return (rc != GT_OK)? rc : rc1;
        }
    }

    return rc;
}

/**
* @internal cpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, valid, ruleSize, tcamPatternPtr, tcamMaskPtr));

    rc = internal_cpssDxChTcamPortGroupRuleWrite(devNum, portGroupsBmp, index, valid, ruleSize, tcamPatternPtr, tcamMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, valid, ruleSize, tcamPatternPtr, tcamMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleRead function
* @endinternal
*
* @brief   Reads the TCAM mask and key.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[out] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTcamPortGroupRuleRead
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamPatternPtr,
    OUT GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  bank;               /* current bank written         */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;              /* holds temporary value        */
    GT_U32  startBit;           /* start bit in output data     */
    GT_U32  keySize;            /* key size in HW data          */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize; /* SW value of rule size */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));

    /* read data from first bank*/
    rc = prvCpssDxChTcamPortGroupRuleReadEntry(devNum,portGroupsBmp,index,validPtr,ruleSizePtr,tcamPatternPtr,tcamMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,*ruleSizePtr);
    if (rc != GT_OK)
    {
        /* found rule size and rule index incompatible */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks,*ruleSizePtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* first bank was already read above. Read other banks. */
    for (bank = 1; bank < numOfBanks; bank++)
    {
        rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                 entryNumberX + (bank*2),
                                                 entryDataX);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                 entryNumberY + (bank*2),
                                                 entryDataY);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* convert data from X/Y format to pattern/mask */
        for (ii = 0 ; ii <= 2 ; ii++)
        {
            entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
            entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
        }

        /* check rule size and compare with first bank. Bank may be overridden
           by beggining of another rule. */
        U32_GET_FIELD_IN_ENTRY_MAC(entryKey, 0, 3, keySize);
        PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(ruleSize, keySize, GT_BAD_STATE);
        if (*ruleSizePtr != ruleSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        startBit = bank * 80;

        /* copy first 64 bits of pattern/mask */
        for(ii = 0 ; ii < 2 ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
            U32_SET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),32,value);
            U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
            U32_SET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),32,value);
        }
        ii = 2;
        /* copy next 16 bits of pattern/mask (total of 80 bits)*/
        U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
        U32_SET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),16,value);
        U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);
        U32_SET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),16,value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupRuleRead function
* @endinternal
*
* @brief   Reads the TCAM mask and key.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[out] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleRead
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamPatternPtr,
    OUT GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, validPtr, ruleSizePtr, tcamPatternPtr, tcamMaskPtr));

    rc = internal_cpssDxChTcamPortGroupRuleRead(devNum, portGroupsBmp, index, validPtr, ruleSizePtr, tcamPatternPtr, tcamMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, validPtr, ruleSizePtr, tcamPatternPtr, tcamMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the TCAM rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_U32  portGroupId;        /* the port group Id            */
    GT_STATUS rc;               /* function return code         */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_BOOL validFound;         /* is rule found valid          */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSizeFound; /* rule size found */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {

        rc = prvCpssDxChTcamPortGroupRuleReadEntry(devNum,1<<portGroupId,index,&validFound,&ruleSizeFound,entryKey,entryMask);
        if ((valid == GT_FALSE) && (rc == GT_BAD_SIZE))
        {
            /* the rule is place contains invalid data - nothing can be matched */
            return GT_OK;
        }
        if (rc != GT_OK)
        {
            return rc;
        }

        if (valid == validFound)
        {
            /* the rule is already in required valid state */
            return GT_OK;
        }

        rc = prvCpssDxChTcamPortGroupRuleWriteEntry(devNum,1<<portGroupId,index,valid,ruleSizeFound,entryKey,entryMask);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the TCAM rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleValidStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, valid));

    rc = internal_cpssDxChTcamPortGroupRuleValidStatusSet(devNum, portGroupsBmp, index, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, valid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   Gets validity of the TCAM rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleSizePtr);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read first bank to get the rule's length */
    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberX,
                                             entryDataX);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberY,
                                             entryDataY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* first bank size is used as validity for the whole rule.
       when X=0 and Y=1 results--> Match 0,
       when X=1 and Y=1 results--> Always Miss. */
    U32_GET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,value);
    *validPtr = !(BIT2BOOL_MAC(value));

    /* convert data from X/Y format to pattern/mask */
    for (ii = 0 ; ii <= 2 ; ii++)
    {
        entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
        entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
    }

    U32_GET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
    if (keySize == 7)
        numOfBanks = 8;
    else
        numOfBanks = keySize + 1;
    PRV_CPSS_DXCH_TCAM_CONVERT_NUM_OF_BANKS_TO_RULE_SIZE_VAL_MAC(*ruleSizePtr,numOfBanks);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,*ruleSizePtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   Gets validity of the TCAM rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleValidStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, validPtr, ruleSizePtr));

    rc = internal_cpssDxChTcamPortGroupRuleValidStatusGet(devNum, portGroupsBmp, index, validPtr, ruleSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, validPtr, ruleSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* clientGroupForRepresentativePortGroupSet
*
* DESCRIPTION:
*       Enable/Disable client per TCAM group for Representative Port Group.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X.
*
* NOT APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*       NOTE: Bobcat3 : The Tcam groups can not be configured as it is 'Hard Wired'
*
* INPUTS:
*       devNum            - the device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                         (APPLICABLE DEVICES Falcon)
*                         Bitmap must be set with at least one bit representing
*                         valid port group(s). If a bit of non valid port group
*                         is set then function returns GT_BAD_PARAM.
*                         value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                         3. see general note about portGroupsBmp in TCAM APIs for Falcon
*       hwClientId        - HW index for tcamClient.
*       tcamGroup         - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
*       representativePortGroupId - the representative port group Id (0,2,4,6).
*                               every 2 pipes share a TCAM
*       enable            - GT_TRUE: TCAM client is enabled
*                           GT_FALSE: TCAM client is disabled
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_OUT_OF_RANGE          - parameter not in valid range.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
/*static*/ GT_STATUS clientGroupForRepresentativePortGroupSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              hwClientId,
    IN  GT_U32                              tcamGroup,
    IN  GT_U32                              representativePortGroupId,
    IN  GT_BOOL                             enable
)
{
    GT_U32    ii;
    GT_STATUS rc;
    GT_U32    value;
    GT_U32    regAddr;          /* register address          */
    GT_U32    currentValueMask;
    GT_U32    currentPortGroupsBmp;
    GT_U32    numClientIds;     /* number of TCAM clients */
    GT_U32    valueMask;        /* support 2 pipes (Falcon) */
    GT_U32    numOfPipeOnTcam;
    GT_U32    numOfClientGroups;

    numOfClientGroups = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups;
    if (tcamGroup >= numOfClientGroups)
    {
        /* client group not supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(representativePortGroupId & 1)/* every 2 pipes share a TCAM */
    {
        /* only representatives port groups (0,2,4,6) supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numClientIds = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass ? 4 : 5;
    if (!PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* Bobcat2, Aldrin, Caelum, Aldrin2, AC5P */
        numOfPipeOnTcam = 1;
        valueMask = BIT_0;
    }
    else
    {
        /* Falcon */
        numOfPipeOnTcam = 2;
        valueMask = BIT_0 | (1 << numClientIds);
    }

    /* set the mask to proper client */
    currentValueMask = valueMask << hwClientId;
    /* caller already '>>' the portGroupsBmp */
    /* get proper port groups */
    currentPortGroupsBmp = portGroupsBmp & ((1<<numOfPipeOnTcam)-1);


    for(ii = 0 ; ii < numOfPipeOnTcam; ii++)
    {
        if(0 == (currentPortGroupsBmp & (1 << ii)))
        {
            /* remove the bit of this port group from the mask */
            currentValueMask &= ~(((1 << numClientIds) - 1) << (numClientIds*ii));
        }
    }

    /* first : disable the client on the rest of 4 tcam groups */
    value = 0;

    for(ii = 0 ; ii < numOfClientGroups ;ii++)
    {
        if(ii == tcamGroup)
        {
            /* will be configured after this loop */
            continue;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.groupClientEnable[ii];
        rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum,representativePortGroupId,regAddr,currentValueMask,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable/disable the client on the chosen tcam group */
    value = (enable == GT_TRUE) ? 0xFFFFFFFF : 0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.groupClientEnable[tcamGroup];
    rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum,representativePortGroupId,regAddr,currentValueMask,value);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}
/**
* @internal internal_cpssDxChTcamPortGroupClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamPortGroupClientGroupSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    IN  GT_U32                              tcamGroup,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    GT_U32    hwClientId;                   /* HW index for tcamClient   */
    GT_PORT_GROUPS_BMP client_portGroupsBmp = portGroupsBmp;/* the BMP of pipes that need clients */
    GT_U32  portGroupId;                    /* the port group Id            */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E);

    /* TCAM we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    /* clients we have per pipe we have for every 2 pipes */
    if (CPSS_PORT_GROUP_UNAWARE_MODE_CNS != portGroupsBmp)
    {
        /* the same configurations for both pipes using TCAM */
        client_portGroupsBmp = (portGroupsBmp | (portGroupsBmp << 1));
    }
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,client_portGroupsBmp);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,tcamGroup);

    PRV_CPSS_DXCH_TCAM_CONVERT_CLIENT_TO_CLIENT_ID_VAL_MAC(devNum, hwClientId, tcamClient);


    /* for AC5P only TCAM Client Groups 2-4 not dedicated to support IPCL clients */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        switch (tcamClient)
        {
            case CPSS_DXCH_TCAM_IPCL_0_E:
            case CPSS_DXCH_TCAM_IPCL_1_E:
            case CPSS_DXCH_TCAM_IPCL_2_E:
                if (tcamGroup > 2)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            default: break;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = clientGroupForRepresentativePortGroupSet(devNum,
            client_portGroupsBmp >> portGroupId,/* the BMP of clients */
            hwClientId,
            tcamGroup,
            portGroupId,/* the representative port group */
            enable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;

}

/**
* @internal cpssDxChTcamPortGroupClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupClientGroupSet

(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    IN  GT_U32                              tcamGroup,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupClientGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, tcamClient, tcamGroup, enable));

    rc = internal_cpssDxChTcamPortGroupClientGroupSet(devNum, portGroupsBmp, tcamClient, tcamGroup, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, tcamClient, tcamGroup, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal clientGroupForRepresentativePortGroupGet function
* @endinternal
*
* @brief   Get Enable/Disable TCAM group per client for Representative Port Group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] hwClientId               - HW index for tcamClient.
* @param[in] representativePortGroupId - the representative port group Id (0,2,4,6).
*                                      every 2 pipes share a TCAM
*
* @param[out] tcamGroupPtr             - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
*                                      relevant only when (enablePtr == GT_TRUE)
* @param[out] enablePtr                - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS clientGroupForRepresentativePortGroupGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              hwClientId,
    IN  GT_U32                              representativePortGroupId,
    OUT GT_U32                              *tcamGroupPtr,
    OUT  GT_BOOL                            *enablePtr
)
{
    GT_U32    ii;
    GT_STATUS rc;
    GT_U32    value;
    GT_U32    regAddr;          /* register address          */
    GT_U32    currentValueMask;
    GT_U32    currentPortGroupsBmp;
    GT_U32    numClientIds;     /* number of TCAM clients */
    GT_U32    valueMask;        /* support 2 pipes (Falcon) */
    GT_U32    numOfHits = 0;
    GT_U32    numOfPipeOnTcam;
    GT_U32    numOfClientGroups = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups;

    if(representativePortGroupId & 1)/* every 2 pipes share a TCAM */
    {
        /* only representatives port groups (0,2,4,6) supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *enablePtr = GT_FALSE;

    numClientIds = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass ? 4 : 5;
    if (!PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* Bobcat2, Aldrin, Caelum, Aldrin2, AC5P */
        numOfPipeOnTcam = 1;
        valueMask = BIT_0;
    }
    else
    {
        /* Falcon */
        numOfPipeOnTcam = 2;
        valueMask = BIT_0 | (1 << numClientIds);
    }

    /* set the mask to proper client */
    currentValueMask = valueMask << hwClientId;
    /* caller already '>>' the portGroupsBmp */
    /* get proper port groups */
    currentPortGroupsBmp = portGroupsBmp & ((1<<numOfPipeOnTcam)-1);

    for(ii = 0 ; ii < numOfPipeOnTcam; ii++)
    {
        if(0 == (currentPortGroupsBmp & (1 << ii)))
        {
            /* remove the bit of this port group from the mask */
            currentValueMask &= ~(((1 << numClientIds) - 1) << (numClientIds*ii));
        }
    }

    for(ii = 0 ; ii < numOfClientGroups ; ii++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.groupClientEnable[ii];
        rc = prvCpssHwPpPortGroupReadRegBitMask(devNum,representativePortGroupId,regAddr,currentValueMask,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(value == 0)
        {
            /* 'disabled' on all port groups */
        }
        else
        if(value == currentValueMask)
        {
            /* all the bits are '1'*/
            numOfHits ++;
            if(numOfHits > 1)
            {
                /* more than one hit ?! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            *tcamGroupPtr = ii;
            *enablePtr = GT_TRUE;
        }
        else
        {
            /* no all port groups 'agree' that this is the tcam group */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChTcamPortGroupClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChTcamPortGroupClientGroupGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    OUT GT_U32                              *tcamGroupPtr,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    hwClientId;                   /* HW index for tcamClient   */
    GT_U32    curr_tcamGroup=0;             /* tcam group from the current representative port group */
    GT_BOOL   curr_enable=GT_FALSE;         /* enable/disable from the current representative port group */
    GT_U32    numValuesGet = 0;             /* number of values read from the registers */
    GT_PORT_GROUPS_BMP client_portGroupsBmp = portGroupsBmp;/* the BMP of pipes that need clients */
    GT_U32  portGroupId;                    /* the port group Id            */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* TCAM we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    /* clients we have per pipe we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,client_portGroupsBmp);

    CPSS_NULL_PTR_CHECK_MAC(tcamGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /* clients mapping is fixed at HW, can not be changed */
        switch (tcamClient)
        {
            case CPSS_DXCH_TCAM_TTI_E:
                *tcamGroupPtr = 0;
                break;
            case CPSS_DXCH_TCAM_IPCL_0_E:
                *tcamGroupPtr = 1;
                break;
            case CPSS_DXCH_TCAM_IPCL_1_E:
                *tcamGroupPtr = 2;
                break;
            case CPSS_DXCH_TCAM_IPCL_2_E:
                *tcamGroupPtr = 3;
                break;
            case CPSS_DXCH_TCAM_EPCL_E:
                *tcamGroupPtr = 4;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *enablePtr = GT_TRUE;

        return GT_OK;
    }
    PRV_CPSS_DXCH_TCAM_CONVERT_CLIENT_TO_CLIENT_ID_VAL_MAC(devNum, hwClientId, tcamClient);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = clientGroupForRepresentativePortGroupGet(devNum,
            client_portGroupsBmp >> portGroupId,/* the BMP of clients */
            hwClientId,
            portGroupId,/* the representative port group */
            &curr_tcamGroup,&curr_enable);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(numValuesGet != 0)
        {
            /* we can compare current result with previous one */
            if(curr_tcamGroup != (*tcamGroupPtr))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Getting curr_tcamGroup[%d] (from port group[%d]) that differ from previous one[%d]",
                    curr_tcamGroup,portGroupId,(*tcamGroupPtr));
            }

            if(curr_enable != (*enablePtr))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Getting curr_enable[%d] (from port group[%d]) that differ from previous one[%d]",
                    curr_enable,portGroupId,(*enablePtr));
            }
        }

        *tcamGroupPtr = curr_tcamGroup;
        *enablePtr = curr_enable;

        numValuesGet++;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupClientGroupGet

(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    OUT GT_U32                              *tcamGroupPtr,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupClientGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, tcamClient, tcamGroupPtr, enablePtr));

    rc = internal_cpssDxChTcamPortGroupClientGroupGet(devNum, portGroupsBmp, tcamClient, tcamGroupPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, tcamClient, tcamGroupPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       (APPLICABLE DEVICES Falcon)
*                                       Bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
*                                       Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                       APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                       index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                       index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                       APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  GT_U32                           floorIndex,
    IN  CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_U32    regAddr;        /* register address  */
    GT_U32    ii;             /* index */
    GT_U32    numOfBlocks;    /* number of blocks in TCAM, 2 in Bobcat2, 6 in Caelum */
    GT_U32    blockId;        /* blockId */
    GT_U32    portGroupId;    /* the port group Id */
    GT_STATUS rc;             /* return code */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(
        devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    for (ii = 0; ii < numOfBlocks; ii++)
    {
        PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,floorInfoArr[ii].group);
        PRV_CPSS_DXCH_TCAM_HIT_NUM_CHECK_MAC(devNum,floorInfoArr[ii].hitNum);
    }

    if (floorIndex>=CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamHitNumAndGroupSelFloor[floorIndex];

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        if(! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /*  index 1 - means all banks at range 6-11 */
            rc = prvCpssHwPpPortGroupSetRegField(
                devNum, portGroupId, regAddr,3,3,floorInfoArr[1].group);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssHwPpPortGroupSetRegField(
                devNum, portGroupId, regAddr, 8, 2, floorInfoArr[1].hitNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Bank0 - means all banks at range 0-5 */

            rc = prvCpssHwPpPortGroupSetRegField(
                devNum, portGroupId, regAddr, 0, 3, floorInfoArr[0].group);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssHwPpPortGroupSetRegField(
                devNum, portGroupId, regAddr, 6, 2, floorInfoArr[0].hitNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            for(ii = 0 ; ii < numOfBlocks; ii++)
            {
                blockId = ii * (6 / numOfBlocks);

                rc = prvCpssHwPpPortGroupSetRegField(
                    devNum, portGroupId, regAddr, blockId*3, 3, floorInfoArr[ii].group);
                if (rc != GT_OK)
                {
                    return rc;
                }

                rc = prvCpssHwPpPortGroupSetRegField(
                    devNum, portGroupId, regAddr, 18 + blockId*2, 2, floorInfoArr[ii].hitNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       (APPLICABLE DEVICES Falcon)
*                                       Bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
*                                       Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                       APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                       index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                       index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                       APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  GT_U32                           floorIndex,
    IN  CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, floorIndex, floorInfoArr));

    rc = internal_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
        devNum, portGroupsBmp, floorIndex, floorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, floorIndex, floorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       (APPLICABLE DEVICES Falcon)
*                                       Bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
*                                       Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                       APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                       index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                       index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                       APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    IN CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    return cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, floorIndex, floorInfoArr);
}

/**
* @internal cpssDxChTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
*                                       Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                       APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                       index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                       index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                       APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    IN CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamIndexRangeHitNumAndGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, floorIndex, floorInfoArr));

    rc = internal_cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, floorIndex, floorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, floorIndex, floorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum          - the device number
* @param[in] portGroupsBmp   - bitmap of Port Groups.
*                              NOTEs:
*                              1. for non multi-port groups device this parameter is IGNORED.
*                              (APPLICABLE DEVICES Falcon)
*                              Bitmap must be set with at least one bit representing
*                              valid port group(s). If a bit of non valid port group
*                              is set then function returns GT_BAD_PARAM.
*                              value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                              2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex      - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  GT_U32                           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{

    GT_U32    regAddr;  /* register address  */
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    numOfBlocks;    /* number of blocks in TCAM, 2 in Bobcat2, 6 in Caelum */
    GT_U32    blockId;        /* blockId */
    GT_U32    portGroupId;    /* the port group Id  */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(
        devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    CPSS_NULL_PTR_CHECK_MAC(floorInfoArr);

    if (floorIndex>=CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamHitNumAndGroupSelFloor[floorIndex];
    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        rc = prvCpssHwPpPortGroupGetRegField(
            devNum, portGroupId, regAddr, 3, 3, &floorInfoArr[1].group);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpPortGroupGetRegField(
            devNum, portGroupId, regAddr, 8, 2, &floorInfoArr[1].hitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Bank0 - means all banks at range 0-5 */

        rc = prvCpssHwPpPortGroupGetRegField(
            devNum, portGroupId, regAddr, 0, 3, &floorInfoArr[0].group);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpPortGroupGetRegField(
            devNum, portGroupId, regAddr, 6, 2, &floorInfoArr[0].hitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        for(ii = 0 ; ii < numOfBlocks; ii++)
        {
            blockId = ii * (6 / numOfBlocks);

            rc = prvCpssHwPpPortGroupGetRegField(
                devNum, portGroupId, regAddr, blockId*3, 3, &floorInfoArr[ii].group);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssHwPpPortGroupGetRegField(
                devNum, portGroupId, regAddr, 18 + blockId*2, 2, &floorInfoArr[ii].hitNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum          - the device number
* @param[in] portGroupsBmp   - bitmap of Port Groups.
*                              NOTEs:
*                              1. for non multi-port groups device this parameter is IGNORED.
*                              (APPLICABLE DEVICES Falcon)
*                              Bitmap must be set with at least one bit representing
*                              valid port group(s). If a bit of non valid port group
*                              is set then function returns GT_BAD_PARAM.
*                              value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                              2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex      - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  GT_U32                           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, floorIndex, floorInfoArr));

    rc = internal_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
        devNum, portGroupsBmp, floorIndex, floorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, portGroupsBmp, devNum, floorIndex, floorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/

static GT_STATUS internal_cpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    return cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, floorIndex, floorInfoArr);
}

/**
* @internal cpssDxChTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamIndexRangeHitNumAndGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, floorIndex, floorInfoArr));

    rc = internal_cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, floorIndex, floorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvTcamDummyRead function
* @endinternal
*
* @brief   perform read of entry #0 from TCAM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS  prvTcamDummyRead
(
    IN GT_U8                    devNum,
    IN GT_PORT_GROUPS_BMP       portGroupsBmp
)
{
    GT_STATUS       rc;           /* return code */
    GT_U32          portGroupId;  /* port group id */
    GT_U32          entryData[3]; /* buffer for entry */
    GT_BOOL         isReadFromShadow = GT_FALSE; /* is read done from shadow */

    rc = prvCpssDxChTableReadFromShadowEnableGet(devNum,CPSS_DXCH_SIP5_TABLE_TCAM_E,&isReadFromShadow);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (isReadFromShadow)
    {
        /* need configure tables API to read from HW */
        rc = prvCpssDxChTableReadFromShadowEnableSet(devNum,CPSS_DXCH_SIP5_TABLE_TCAM_E,GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                 0,
                                                 entryData);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    if (isReadFromShadow)
    {
        /* restore configuration */
        rc = prvCpssDxChTableReadFromShadowEnableSet(devNum,CPSS_DXCH_SIP5_TABLE_TCAM_E,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamActiveBmpGet function
* @endinternal
*
* @brief   Function get bitmap of port groups with Activated TCAMs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum             - PP device number
* @param[out] activeTcamBmpPtr  - bitmap of Port Groups with Active TCAM.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*
*/
GT_STATUS prvCpssDxChTcamActiveBmpGet
(
    IN  GT_U8               devNum,
    OUT GT_PORT_GROUPS_BMP  *activeTcamBmpPtr
)
{
    GT_U32              portGroupId; /* the port group Id */
    GT_STATUS           rc;          /* return code       */
    GT_PORT_GROUPS_BMP  tcamBmp;     /* bitmap of port groups with TCAM */
    GT_U32              activeFloors; /* number of active floors */
    GT_PORT_GROUPS_BMP  tmpBmp;       /* bitmap of port groups */

    /* get bitmap with existing TCAMs */
    tcamBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(
        devNum, tcamBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    *activeTcamBmpPtr = 0;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, tcamBmp, portGroupId)
    {
        tmpBmp = (1 << portGroupId);
        rc = cpssDxChTcamPortGroupActiveFloorsGet(devNum, tmpBmp, &activeFloors);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (activeFloors)
        {
            *activeTcamBmpPtr |= tmpBmp;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, tcamBmp, portGroupId)

    return GT_OK;
}

/**
* @internal internal_cpssDxChTcamPortGroupActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamPortGroupActiveFloorsSet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    numOfActiveFloors
)
{
    GT_U32    regAddr;               /* register address  */
    GT_U32    portGroupId;           /* the port group Id */
    GT_U32    prevNumOfActiveFloors; /* previous number of active floors */
    GT_BOOL   tcamDaemonEnable;      /* status of TCAM Daemon */
    GT_BOOL   restartTcamDaemon;     /* restart TCAM daemon after active floors config */
    GT_STATUS rc;                    /* return code       */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(
        devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    if (numOfActiveFloors > CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* the feature is not applicable on emulator */
    if(cpssDeviceRunCheck_onEmulator())
    {
        return GT_OK;
    }

    rc = cpssDxChTcamPortGroupActiveFloorsGet(devNum, portGroupsBmp,&prevNumOfActiveFloors);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get TCAM daemon status */
    tcamDaemonEnable = PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityDaemonEnable;

    restartTcamDaemon = GT_FALSE;

    /* stop TCAM daemon before decrease number of active floors */
    if (prevNumOfActiveFloors && (prevNumOfActiveFloors > numOfActiveFloors))
    {
        if (tcamDaemonEnable == GT_TRUE)
        {
            /* disable TCAM daemon before change configurations */
            rc = prvCpssDxChDiagDataIntegrityPortGroupTcamParityDaemonEnableSet(devNum, portGroupsBmp, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* restart daemon if some floors are still active */
            restartTcamDaemon = numOfActiveFloors ? GT_TRUE : GT_FALSE;
        }

        /* Need to make sure that all daemon TCAM requests are completed.
           And for sip_6_10 device need to clean all internal buffers.
           Both actions are done by read dummy entry before shutdown floors. */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) || (tcamDaemonEnable == GT_TRUE))
        {
            rc = prvTcamDummyRead(devNum,portGroupsBmp);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else if ((0 == prevNumOfActiveFloors) && (numOfActiveFloors) &&
             (tcamDaemonEnable == GT_TRUE))
    {
        /* restart TCAM daemon if number of active floors changing from 0. */
        restartTcamDaemon = GT_TRUE;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamArrayConf.tcamActiveFloors;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(
            devNum, portGroupId, regAddr, 0, 8, numOfActiveFloors);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    if (restartTcamDaemon == GT_TRUE)
    {
        /* re-enable TCAM daemon after change configurations. */
        rc = prvCpssDxChDiagDataIntegrityPortGroupTcamParityDaemonEnableSet(devNum, portGroupsBmp, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupActiveFloorsSet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    numOfActiveFloors
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupActiveFloorsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, numOfActiveFloors));

    rc = internal_cpssDxChTcamPortGroupActiveFloorsSet(
        devNum, portGroupsBmp, numOfActiveFloors);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, numOfActiveFloors));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamActiveFloorsSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          numOfActiveFloors
)
{
    return  cpssDxChTcamPortGroupActiveFloorsSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, numOfActiveFloors);
}

/**
* @internal cpssDxChTcamActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamActiveFloorsSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          numOfActiveFloors
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamActiveFloorsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfActiveFloors));

    rc = internal_cpssDxChTcamActiveFloorsSet(devNum, numOfActiveFloors);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfActiveFloors));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamPortGroupActiveFloorsGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    OUT  GT_U32                   *numOfActiveFloorsPtr
)
{
    GT_U32    regAddr;     /* register address  */
    GT_U32    portGroupId; /* the port group Id */
    GT_STATUS rc;          /* return code       */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(numOfActiveFloorsPtr);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(
        devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamArrayConf.tcamActiveFloors;

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 8, numOfActiveFloorsPtr);

    return rc;
}

/**
* @internal cpssDxChTcamPortGroupActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupActiveFloorsGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    OUT  GT_U32                   *numOfActiveFloorsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupActiveFloorsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, numOfActiveFloorsPtr));

    rc = internal_cpssDxChTcamPortGroupActiveFloorsGet(
        devNum, portGroupsBmp, numOfActiveFloorsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ,numOfActiveFloorsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamActiveFloorsGet
(
    IN  GT_U8           devNum,
    OUT  GT_U32         *numOfActiveFloorsPtr
)
{
    return cpssDxChTcamPortGroupActiveFloorsGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, numOfActiveFloorsPtr);
}

/**
* @internal cpssDxChTcamActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamActiveFloorsGet
(
    IN  GT_U8           devNum,
    OUT  GT_U32         *numOfActiveFloorsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamActiveFloorsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfActiveFloorsPtr));

    rc = internal_cpssDxChTcamActiveFloorsGet(devNum, numOfActiveFloorsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfActiveFloorsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] size                     - key  in TCAM
* @param[in] tcamKeyArr               - (pointer to) key array in the TCAM
* @param[in] subKeyProfile            - TCAM profile ID
*                                       (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*                                       (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                              tcamKeyArr[],
    IN  GT_U32                              subKeyProfile
)
{

    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  regAddr;            /* register address             */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  keySize;            /* key size values              */
    GT_U32  keyChunk;           /* KeyChunk                     */
    GT_U32  offset;             /* offset in chunk register     */
    GT_U32  value;              /* Value of the keyChunk        */
    GT_U32  keyWritten;         /* rule written in bits         */
    GT_U32  length;
    GT_U32  minKeySize;         /* min ruleSize in bits         */
    GT_U32  groupIndex;         /* TCAM group number for MG register access */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyArr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum, group);

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize, size);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* TCAM profile ID is applicable only for SIP_10 and above devices */
        if (subKeyProfile >= BIT_6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        keyChunk = 0;
        offset = 0;
        keyWritten = 0;
        minKeySize = 80;
        length = 0;

        /* SIP_6_10 devices do not have group based indexing for MG access
         * registers and hence set it to 0
         */
        groupIndex = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 0 : group;

        /* write key size to register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeySizeAndTypeGroup[groupIndex];
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 4, 3, keySize);
        if(rc != GT_OK)
        {
            return rc;
        }

        if ((PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 8, 6, subKeyProfile);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* write key size to key array (key size is 4 bits at the begining of each 84 bits of key array)  */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[groupIndex][keyChunk];

        for ( ii = 0; ii <= keySize; ii++)
        {
            minKeySize = 80 * (ii + 1);
            if (!(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
            {
                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, offset%32, 3, keySize);
                if ( rc != GT_OK )
                {
                    return rc;
                }
                offset += 4;
                if (offset == 32 )
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[group][++keyChunk];
                }
            }
            /* Set 80 bits of key in the register */
            while ( keyWritten != minKeySize )
            {
                offset = offset%32;
                length = (minKeySize - keyWritten) > 32 ? (32 - offset) : minKeySize - keyWritten;
                U32_GET_FIELD_IN_ENTRY_MAC(tcamKeyArr, keyWritten, length, value);
                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, offset, length, value);
                if(rc != GT_OK)
                {
                    return rc;
                }
                keyWritten += length;
                if ( length + offset == 32 && keyWritten != 640)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[groupIndex][++keyChunk];
                }
                offset += length;
                value = 0;
            }
        }

        /* trigger request*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgCompRequest;
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, group, 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] size                     - key  in TCAM
* @param[in] tcamKeyArr               - (pointer to) key array in the TCAM
* @param[in] subKeyProfile            - TCAM profile ID
*                                       (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*                                       (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                              tcamKeyArr[],
    IN  GT_U32                              subKeyProfile
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupCpuLookupTriggerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile));

    rc = internal_cpssDxChTcamPortGroupCpuLookupTriggerSet(devNum, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
*
* @param[out] sizePtr                  - (pointer to) key size in TCAM
* @param[out] tcamKeyArr               - (pointer to) key array in the TCAM
* @param[out]  subKeyProfilePtr        - (pointer to) TCAM profile ID
*                                       (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - if in TCAM found rule with different sizes
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT       *sizePtr,
    OUT GT_U32                              tcamKeyArr[],
    OUT GT_U32                             *subKeyProfilePtr
)
{

    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  regAddr;            /* register address             */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  keySize = 0;        /* key size values              */
    GT_U32  keyChunk = 0;       /* KeyChunk                     */
    GT_U32  offset = 0;         /* offset in chunk register     */
    GT_U32  value;              /* Value of the keyChunk        */
    GT_U32  keyRead = 0;        /* key read in bits             */
    GT_U32  length = 0;
    GT_U32  minKeySize = 80;    /* min keySize in bits          */
    GT_U32  groupIndex;         /* TCAM group number for MG register access */
    GT_U32  subKeyProfile;      /* TCAM profile ID */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(sizePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyArr);
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(subKeyProfilePtr);
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,group);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* SIP_6_10 devices do not have group based indexing for MG access
     * registers and hence set it to 0
     */
    groupIndex = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 0 : group;

    /* Get the key size */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeySizeAndTypeGroup[groupIndex];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 4, 3, &keySize);
    if(rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC(*sizePtr,keySize);

    if ((PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 8, 6, &subKeyProfile);
        if(rc != GT_OK)
        {
            return rc;
        }
        *subKeyProfilePtr = subKeyProfile;
    }

    /* write key size to key array (key size is 4 bits at the begining of each 84 bits of key array) */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[groupIndex][keyChunk];

    for (ii = 0; ii <= keySize; ii++)
    {
        minKeySize = 80 * (ii + 1);
        if (!(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
        {
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset%32, 3, &keySize);
            if(rc != GT_OK)
            {
                return rc;
            }
            offset += 4;
            if (offset == 32 )
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[group][++keyChunk];
            }
        }
        /* Read 80 bits of key from the register */
        while(keyRead != minKeySize)
        {
            offset = offset%32;
            length = (minKeySize - keyRead) > 32 ? (32 - offset) : minKeySize - keyRead;
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset, length, &value);
            if(rc != GT_OK)
            {
                return rc;
            }
            U32_SET_FIELD_IN_ENTRY_MAC(tcamKeyArr, keyRead, length, value);

            keyRead += length;

            /* Get the address of next chunk */
            if( length + offset == 32 && keyRead != 640)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[groupIndex][++keyChunk];
            }
            offset += length;
            value = 0;
        }
    }

    /* read rest of 80 bits of key from the register */
    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
*
* @param[out] sizePtr                  - (pointer to) key size in TCAM
* @param[out] tcamKeyArr               - (pointer to) key array in the TCAM
* @param[out]  subKeyProfilePtr        - (pointer to) TCAM profile ID
*                                       (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - if in TCAM found rule with different sizes
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT       *sizePtr,
    OUT GT_U32                              tcamKeyArr[],
    OUT GT_U32                             *subKeyProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupCpuLookupTriggerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, group, sizePtr, tcamKeyArr, subKeyProfilePtr));

    rc = internal_cpssDxChTcamPortGroupCpuLookupTriggerGet(devNum, portGroupsBmp, group, sizePtr, tcamKeyArr, subKeyProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, group, sizePtr, tcamKeyArr, subKeyProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] hitNum                   - hit number in TCAM (APPLICABLE RANGES: 0..3)
* @param[in] portGroupsBmp            -
*
* @param[out] isValidPtr               - (pointer to) whether results are valid for prev trigger set.
* @param[out] isHitPtr                 - (pointer to) whether there was hit in the TCAM. Valid only when isValidPtr == GT_TRUE
* @param[out] hitIndexPtr              - (pointer to) global hit index in the TCAM. Valid only when isHitPtr == GT_TRUE
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if CPU lookup results are not valid
*/
static GT_STATUS internal_cpssDxChTcamCpuLookupResultsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  regAddr;            /* register address             */
    GT_U32  value;
    GT_U32  floorNum;           /* floor number in TCAM         */
    GT_U32  bankNum;            /* bank number in TCAM          */
    GT_U32  bankIndex;          /* index in bank                */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(isValidPtr);
    CPSS_NULL_PTR_CHECK_MAC(isHitPtr);
    CPSS_NULL_PTR_CHECK_MAC(hitIndexPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,group);
    PRV_CPSS_DXCH_TCAM_HIT_NUM_CHECK_MAC(devNum,hitNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* check trigget request*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgCompRequest;
    rc = prvCpssHwPpGetRegField(devNum,regAddr,group,1,&value);
    if(rc != GT_OK)
    {
        return rc;
    }
    *isValidPtr = (value == 1)? GT_FALSE : GT_TRUE;
    if (*isValidPtr == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgHitGroupHitNum[group][hitNum];
    rc = prvCpssHwPpGetRegField(devNum,regAddr,0,1,&value);
    if(rc != GT_OK)
    {
        return rc;
    }
    *isHitPtr = (BIT2BOOL_MAC(value));

    rc = prvCpssHwPpGetRegField(devNum,regAddr,1,16,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(&value,12,4,floorNum);
    U32_GET_FIELD_IN_ENTRY_MAC(&value,4,8,bankIndex);
    U32_GET_FIELD_IN_ENTRY_MAC(&value,0,4,bankNum);

    *hitIndexPtr = floorNum*CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS + bankIndex * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS + bankNum;

    return GT_OK;
}

/**
* @internal cpssDxChTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] hitNum                   - hit number in TCAM (APPLICABLE RANGES: 0..3)
* @param[in] portGroupsBmp            -
*
* @param[out] isValidPtr               - (pointer to) whether results are valid for prev trigger set.
* @param[out] isHitPtr                 - (pointer to) whether there was hit in the TCAM. Valid only when isValidPtr == GT_TRUE
* @param[out] hitIndexPtr              - (pointer to) global hit index in the TCAM. Valid only when isHitPtr == GT_TRUE
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if CPU lookup results are not valid
*/
GT_STATUS cpssDxChTcamCpuLookupResultsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamCpuLookupResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, group, hitNum, isValidPtr, isHitPtr, hitIndexPtr));

    rc = internal_cpssDxChTcamCpuLookupResultsGet(devNum, portGroupsBmp, group, hitNum, isValidPtr, isHitPtr, hitIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, group, hitNum, isValidPtr, isHitPtr, hitIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChTcamHaRuleRead function
* @endinternal
*
* @brief  read a single bank for the given index - supports invalid tcamRuleSize(7) - to support HA
*
* @note   APPLICABLE DEVICES:      Falcon;
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
* @param[out] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamHaRuleRead
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamEntryPatternPtr,
    OUT GT_U32                              *tcamEntryMaskPtr
)
{
    return prvCpssDxChTcamPortGroupRuleReadEntryCheckHa(devNum, portGroupsBmp, index, GT_TRUE,
                                                validPtr, ruleSizePtr, tcamEntryPatternPtr, tcamEntryMaskPtr);
}

/**
* @internal prvCpssDxChTcamHaRuleSizeSet function
* @endinternal
*
* @brief  writes rulesize only - content remains same
*
* @note   APPLICABLE DEVICES:      Falcon;
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTcamHaRuleSizeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_U32                                  index,
    IN  GT_U32                                  ruleSize
)
{
    GT_STATUS rc;               /* function return code         */
    GT_BOOL valid;
    GT_U32  mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32  pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        tcamRuleSize;

    /* rule read is perfomed to avoid the ruleWrite to change anything apart from invalidating ruleSize
        pattern, mask, valid bit all are kept same
     */
    rc = prvCpssDxChTcamHaRuleRead(devNum, portGroupsBmp, index, &valid, &tcamRuleSize, pattern, mask);
    if(rc != GT_OK )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTcamHaRuleRead failed\n");
    }

    return prvCpssDxChTcamPortGroupRuleWriteCheckHa(devNum, portGroupsBmp, index, valid, ruleSize,
                                                pattern, mask, GT_TRUE);
}

/**
* @internal internal_cpssDxChTcamLookupProfileCfgSet function
* @endinternal
*
* @brief   Sets Parallel lookups per Profile configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of port groups used for access to TCAMs.
*                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS used to specify all TCAMs.
*                                       The parameter reserved for future big devices.
*                                       For all devices having one TCAM unit the parameter ignored.
* @param[in] tcamProfileId            - TCAM Profile Id (APPLICABLE RANGES: 1..63).
*                                       TCAM Profile 0 has no configuration and implemented by passing super-key to all lookups.
* @param[in] tcamLookupProfileCfgPtr  - (pointer to) configuration per TCAM profile.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_OUT_OF_RANGE          - one of parameters value out-of-range
*/
static GT_STATUS internal_cpssDxChTcamLookupProfileCfgSet
(
    IN   GT_U8                                  devNum,
    IN   GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN   GT_U32                                 tcamProfileId,
    IN   CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    i;
    GT_U32    portGroupId;
    GT_U32    hwKeySize;
    GT_U32    hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
        | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);
    if ((tcamProfileId == 0) || (tcamProfileId > 63))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tcamProfileId out of range\n");
    }
    CPSS_NULL_PTR_CHECK_MAC(tcamLookupProfileCfgPtr);

    /* configuration valid checking */
    for (i = 0; (i < 4); i++)
    {
        /* if wrong returns GT_BAD_PARAM */
        PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(hwKeySize, tcamLookupProfileCfgPtr->subKey0_3_Sizes[i]);
    }
    for (i = 0; (i < 3); i++)
    {
        if (tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineIndexes[i] > 63)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "subKey1_3_MuxTableLineOffsets out of range\n");
        }
        if (tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineOffsets[i] > 5)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "subKey1_3_MuxTableLineOffsets out of range\n");
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* write key size to register */
        PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(hwKeySize, tcamLookupProfileCfgPtr->subKey0_3_Sizes[0]);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.subKey0Mapping[tcamProfileId - 1];
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 3, hwKeySize);
        if(rc != GT_OK)
        {
            return rc;
        }

        for (i = 0; (i < 3); i++)
        {
            PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(hwKeySize, tcamLookupProfileCfgPtr->subKey0_3_Sizes[i + 1]);
            hwValue = (hwKeySize
                | (tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineIndexes[i] << 3)
                | (tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineOffsets[i] << 9));
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.subKey1_3Mapping[tcamProfileId - 1][i];
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 12, hwValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamLookupProfileCfgSet function
* @endinternal
*
* @brief   Sets Parallel lookups per Profile configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] tcamProfileId            - TCAM Profile Id (APPLICABLE RANGES: 1..63).
*                                       TCAM Profile 0 has no configuration and implemented by passing super-key to all lookups.
* @param[in] tcamLookupProfileCfgPtr  - (pointer to) configuration per TCAM profile.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_OUT_OF_RANGE          - one of parameters value out-of-range
*/
GT_STATUS cpssDxChTcamLookupProfileCfgSet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tcamProfileId,
    IN   CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamLookupProfileCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcamProfileId, tcamLookupProfileCfgPtr));

    rc = internal_cpssDxChTcamLookupProfileCfgSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, tcamProfileId, tcamLookupProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcamProfileId, tcamLookupProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamLookupProfileCfgGet function
* @endinternal
*
* @brief   Gets Parallel lookups per Profile configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of port groups used for access to TCAMs.
*                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS used to specify all TCAMs.
*                                       The parameter reserved for future big devices.
*                                       For all devices having one TCAM unit the parameter ignored.
* @param[in] tcamProfileId            - TCAM Profile Id (APPLICABLE RANGES: 1..63).
*                                       TCAM Profile 0 has no configuration and implemented by passing super-key to all lookups.
* @param[out] tcamLookupProfileCfgPtr - (pointer to) configuration per TCAM profile.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_BAD_STATE             - one HW fields out of range
*/
static GT_STATUS internal_cpssDxChTcamLookupProfileCfgGet
(
    IN   GT_U8                                  devNum,
    IN   GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN   GT_U32                                 tcamProfileId,
    OUT  CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    i;
    GT_U32    portGroupId;
    GT_U32    hwKeySize;
    GT_U32    hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
        | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);
    if ((tcamProfileId == 0) || (tcamProfileId > 63))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tcamProfileId out of range\n");
    }
    CPSS_NULL_PTR_CHECK_MAC(tcamLookupProfileCfgPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.subKey0Mapping[tcamProfileId - 1];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 3, &hwKeySize);
    if(rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(
        tcamLookupProfileCfgPtr->subKey0_3_Sizes[0], hwKeySize, GT_BAD_STATE);

    for (i = 0; (i < 3); i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.subKey1_3Mapping[tcamProfileId - 1][i];
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 12, &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }
        hwKeySize = (hwValue & 0x7);
        PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(
            tcamLookupProfileCfgPtr->subKey0_3_Sizes[i + 1], hwKeySize, GT_BAD_STATE);
        tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineIndexes[i] = ((hwValue >> 3) & 0x3F);
        if (tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineIndexes[i] > 63)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "subKey1_3_MuxTableLineOffsets out of range\n");
        }
        tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineOffsets[i] = ((hwValue >> 9) & 0x7);
        if (tcamLookupProfileCfgPtr->subKey1_3_MuxTableLineOffsets[i] > 5)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "subKey1_3_MuxTableLineOffsets out of range\n");
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamLookupProfileCfgGet function
* @endinternal
*
* @brief   Gets Parallel lookups per Profile configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] tcamProfileId            - TCAM Profile Id (APPLICABLE RANGES: 1..63).
*                                       TCAM Profile 0 has no configuration and implemented by passing super-key to all lookups.
* @param[out] tcamLookupProfileCfgPtr - (pointer to) configuration per TCAM profile.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_BAD_STATE             - one HW fields out of range
*/
GT_STATUS cpssDxChTcamLookupProfileCfgGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tcamProfileId,
    OUT  CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamLookupProfileCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcamProfileId, tcamLookupProfileCfgPtr));

    rc = internal_cpssDxChTcamLookupProfileCfgGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, tcamProfileId, tcamLookupProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcamProfileId, tcamLookupProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamLookupMuxTableLineSet function
* @endinternal
*
* @brief   Sets configuration of TCAM sub-key Mux Table Entry.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of port groups used for access to TCAMs.
*                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS used to specify all TCAMs.
*                                       The parameter reserved for future big devices.
*                                       For all devices having one TCAM unit the parameter ignored.
* @param[in] entryIndex              - TCAM sub-key Mux Table Entry index (APPLICABLE RANGES: 1..47).
*                                       Index 0 used for truncating super-key without muxing.
* @param[in] tcamLookupProfileCfgPtr - (pointer to) configuration of TCAM sub-key Mux Table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_OUT_OF_RANGE          - one of parameters value out-of-range
*/
static GT_STATUS internal_cpssDxChTcamLookupMuxTableLineSet
(
    IN   GT_U8                                         devNum,
    IN   GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN   GT_U32                                        entryIndex,
    IN   CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupProfileCfgPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    i;
    GT_U32    n;
    GT_U32    portGroupId;
    GT_U32    hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
        | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);
    if ((entryIndex == 0) || (entryIndex > 47))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "entryIndex out of range\n");
    }
    CPSS_NULL_PTR_CHECK_MAC(tcamLookupProfileCfgPtr);

    for (i = 0; (i < 30); i++)
    {
        /* max - 80 bytes - 40 2-bytes units */
        if (tcamLookupProfileCfgPtr->superKeyTwoByteUnitsOffsets[i] >= 40)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "superKeyTwoByteUnitsOffsets out of range\n");
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        for (n = 0; (n < 6); n++)
        {
            hwValue = 0;
            for (i = 0; (i < 5); i++)
            {
                hwValue |= (tcamLookupProfileCfgPtr->superKeyTwoByteUnitsOffsets[(n * 5) + i] << (6 * i));
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.keyMuxCfgTable[entryIndex - 1][n];
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 30, hwValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamLookupMuxTableLineSet function
* @endinternal
*
* @brief   Sets configuration of TCAM sub-key Mux Table Entry.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                        - the device number
* @param[in] entryIndex                    - TCAM sub-key Mux Table Entry index (APPLICABLE RANGES: 1..47).
*                                            Index 0 used for truncating super-key without muxing.
* @param[in] tcamLookupMuxTableLineCfgPtr  - (pointer to) configuration of TCAM sub-key Mux Table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_OUT_OF_RANGE          - one of parameters value out-of-range
*/
GT_STATUS cpssDxChTcamLookupMuxTableLineSet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        entryIndex,
    IN   CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamLookupMuxTableLineSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, tcamLookupMuxTableLineCfgPtr));

    rc = internal_cpssDxChTcamLookupMuxTableLineSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, entryIndex, tcamLookupMuxTableLineCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, tcamLookupMuxTableLineCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamLookupMuxTableLineGet function
* @endinternal
*
* @brief   Gets configuration of TCAM sub-key Mux Table Entry.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                         - the device number
* @param[in] portGroupsBmp                  - bitmap of port groups used for access to TCAMs.
*                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS used to specify all TCAMs.
*                                             The parameter reserved for future big devices.
*                                             For all devices having one TCAM unit the parameter ignored.
* @param[in] entryIndex                    - TCAM sub-key Mux Table Entry index (APPLICABLE RANGES: 1..47).
*                                             Index 0 used for truncating super-key without muxing.
* @param[out] tcamLookupMuxTableLineCfgPtr - (pointer to) configuration of TCAM sub-key Mux Table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
*/
static GT_STATUS internal_cpssDxChTcamLookupMuxTableLineGet
(
    IN   GT_U8                                         devNum,
    IN   GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN   GT_U32                                        entryIndex,
    OUT  CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    i;
    GT_U32    n;
    GT_U32    portGroupId;
    GT_U32    hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
        | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);
    if ((entryIndex == 0) || (entryIndex > 47))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "entryIndex out of range\n");
    }
    CPSS_NULL_PTR_CHECK_MAC(tcamLookupMuxTableLineCfgPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    for (n = 0; (n < 6); n++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.keyMuxCfgTable[entryIndex - 1][n];
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 30, &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }
        for (i = 0; (i < 5); i++)
        {
            tcamLookupMuxTableLineCfgPtr->superKeyTwoByteUnitsOffsets[(n * 5) + i] = ((hwValue >> (6 * i)) & 0x3F);
        }
    }
    for (i = 0; (i < 30); i++)
    {
        /* max - 80 bytes - 40 2-bytes units */
        if (tcamLookupMuxTableLineCfgPtr->superKeyTwoByteUnitsOffsets[i] >= 40)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "superKeyTwoByteUnitsOffsets out of range\n");
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamLookupMuxTableLineGet function
* @endinternal
*
* @brief   Gets configuration of TCAM sub-key Mux Table Entry.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                        - the device number
* @param[in] entryIndex                    - TCAM sub-key Mux Table Entry index (APPLICABLE RANGES: 1..47).
*                                            Index 0 used for truncating super-key without muxing.
* @param[out] tcamLookupMuxTableLineCfgPtr - (pointer to) configuration of TCAM sub-key Mux Table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
*/
GT_STATUS cpssDxChTcamLookupMuxTableLineGet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        entryIndex,
    OUT  CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamLookupMuxTableLineGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, tcamLookupMuxTableLineCfgPtr));

    rc = internal_cpssDxChTcamLookupMuxTableLineGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, entryIndex, tcamLookupMuxTableLineCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, tcamLookupMuxTableLineCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

