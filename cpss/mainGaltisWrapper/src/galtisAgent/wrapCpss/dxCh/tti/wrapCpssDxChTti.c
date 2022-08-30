/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file wrapCpssDxChTti.c
*
* @brief DxCh TTI wrappers
*
*
* @version   77
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>

#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#ifndef CPSS_APP_PLATFORM_REFERENCE
extern GT_U32 appDemoDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
);

extern GT_VOID appDemoDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useAppdemoOffset
);

extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

extern GT_U32 appDemoDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);
#endif
/* support for multi port groups */

/**
* @internal ttiMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs
*         with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void ttiMultiPortGroupsBmpGet
(
    IN   GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    /* default */
    *enablePtr  = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return;
    }

    utilMultiPortGroupsBmpGet(devNum, enablePtr, portGroupsBmpPtr);
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiMacToMeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChTtiMacToMeSet(
            devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);
    }
    else
    {
        return cpssDxChTtiPortGroupMacToMeSet(
            devNum, pgBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiMacToMeGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          entryIndex,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChTtiMacToMeGet(
            devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);
    }
    else
    {
        return cpssDxChTtiPortGroupMacToMeGet(
            devNum, pgBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiMacModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChTtiMacModeSet(
            devNum, keyType, macMode);
    }
    else
    {
        return cpssDxChTtiPortGroupMacModeSet(
            devNum, pgBmp, keyType, macMode);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiMacModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChTtiMacModeGet(
            devNum, keyType, macModePtr);
    }
    else
    {
        return cpssDxChTtiPortGroupMacModeGet(
            devNum, pgBmp, keyType, macModePtr);
    }
}

extern GT_U32          wrCpssDxChVirtualTcam_useVTcamApi;
extern GT_U32          wrCpssDxChVirtualTcam_vTcamMngId;
extern GT_U32          wrCpssDxChVirtualTcam_vTcamId;
extern GT_U32          wrCpssDxChVirtualTcam_vTcamRulePrio;

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         keyType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_STC            *actionPtr,
    IN  GT_BOOL                             absoluteTcamIndexing
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        ruleType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;

        ruleAttributes.priority = wrCpssDxChVirtualTcam_vTcamRulePrio;
        ruleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
        ruleType.rule.tti.ruleFormat = keyType;
        ruleData.valid = GT_TRUE;
        ruleData.rule.tti.actionPtr = actionPtr;
        ruleData.rule.tti.patternPtr = patternPtr;
        ruleData.rule.tti.maskPtr = maskPtr;

        return cpssDxChVirtualTcamRuleWrite(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            routerTtiTcamRow,
                                            &ruleAttributes,
                                            &ruleType,
                                            &ruleData);
    }
    else
    {
        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

        ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        if (absoluteTcamIndexing == GT_FALSE)
        {
            appDemoDxChNewTtiTcamSupportSet(GT_TRUE);
            routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
        }

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChTtiRuleSet(
                devNum, routerTtiTcamRow, keyType,
                patternPtr, maskPtr, actionPtr);
        }
        else
        {
            return cpssDxChTtiPortGroupRuleSet(
                devNum, pgBmp, routerTtiTcamRow, keyType,
                patternPtr, maskPtr, actionPtr);
        }
    }
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(routerTtiTcamRow);
    GT_UNUSED_PARAM(keyType);
    GT_UNUSED_PARAM(patternPtr);
    GT_UNUSED_PARAM(maskPtr);
    GT_UNUSED_PARAM(actionPtr);
    GT_UNUSED_PARAM(absoluteTcamIndexing);
    return GT_OK;
#endif
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         keyType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    OUT CPSS_DXCH_TTI_ACTION_STC            *actionPtr,
    IN  GT_BOOL                             absoluteTcamIndexing
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        ruleType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;

        ruleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
        ruleType.rule.tti.ruleFormat = keyType;

        ruleData.rule.tti.actionPtr = actionPtr;
        ruleData.rule.tti.patternPtr = patternPtr;
        ruleData.rule.tti.maskPtr = maskPtr;

        return cpssDxChVirtualTcamRuleRead(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            routerTtiTcamRow,
                                            &ruleType,
                                            &ruleData);
    }
    else
    {
        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

        ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        if (absoluteTcamIndexing == GT_FALSE)
        {
            appDemoDxChNewTtiTcamSupportSet(GT_TRUE);
            routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
        }

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChTtiRuleGet(
                devNum, routerTtiTcamRow, keyType,
                patternPtr, maskPtr, actionPtr);
        }
        else
        {
            return cpssDxChTtiPortGroupRuleGet(
                devNum, pgBmp, routerTtiTcamRow, keyType,
                patternPtr, maskPtr, actionPtr);
        }
    }
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(routerTtiTcamRow);
    GT_UNUSED_PARAM(keyType);
    GT_UNUSED_PARAM(patternPtr);
    GT_UNUSED_PARAM(maskPtr);
    GT_UNUSED_PARAM(actionPtr);
    GT_UNUSED_PARAM(absoluteTcamIndexing);
    return GT_OK;
#endif
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  CPSS_DXCH_TTI_ACTION_STC            *actionPtr,
    IN  GT_BOOL                             absoluteTcamIndexing,
    IN  GT_BOOL                             isNewIndex
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;
        GT_STATUS rc;

        actionType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

        actionData.action.tti.actionPtr = actionPtr;

        if (isNewIndex)
        {
            /* whole rule must be created before update action */
            return GT_OK;
        }

        rc = cpssDxChVirtualTcamRuleActionUpdate(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            routerTtiTcamRow,
                                            &actionType,
                                            &actionData);
        if (rc == GT_NOT_FOUND)
        {
            /* ignore NOT_FOUND case. Rule was not created yet. */
            rc = GT_OK;
        }

        return rc;
    }
    else
    {
        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

        ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        if (absoluteTcamIndexing == GT_FALSE)
        {
            appDemoDxChNewTtiTcamSupportSet(GT_TRUE);
            routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
        }

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChTtiRuleActionUpdate(
                devNum, routerTtiTcamRow, actionPtr);
        }
        else
        {
            return cpssDxChTtiPortGroupRuleActionUpdate(
                devNum, pgBmp, routerTtiTcamRow, actionPtr);
        }
    }
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(routerTtiTcamRow);
    GT_UNUSED_PARAM(actionPtr);
    GT_UNUSED_PARAM(absoluteTcamIndexing);
    GT_UNUSED_PARAM(isNewIndex);
    return GT_OK;
#endif
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    IN  GT_BOOL                             valid,
    IN  GT_BOOL                             absoluteTcamIndexing
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (absoluteTcamIndexing == GT_FALSE)
    {
        appDemoDxChNewTtiTcamSupportSet(GT_TRUE);
        routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
    }

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChTtiRuleValidStatusSet(
            devNum, routerTtiTcamRow, valid);
    }
    else
    {
        return cpssDxChTtiPortGroupRuleValidStatusSet(
            devNum, pgBmp, routerTtiTcamRow, valid);
    }
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(routerTtiTcamRow);
    GT_UNUSED_PARAM(valid);
    GT_UNUSED_PARAM(absoluteTcamIndexing);
    return GT_OK;
#endif
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChTtiRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerTtiTcamRow,
    OUT GT_BOOL                             *validPtr,
    IN  GT_BOOL                             absoluteTcamIndexing
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ttiMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (absoluteTcamIndexing == GT_FALSE)
    {
        appDemoDxChNewTtiTcamSupportSet(GT_TRUE);
        routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
    }

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChTtiRuleValidStatusGet(
            devNum, routerTtiTcamRow, validPtr);
    }
    else
    {
        return cpssDxChTtiPortGroupRuleValidStatusGet(
            devNum, pgBmp, routerTtiTcamRow, validPtr);
    }
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(routerTtiTcamRow);
    GT_UNUSED_PARAM(validPtr);
    GT_UNUSED_PARAM(absoluteTcamIndexing);
    return GT_OK;
#endif
}


/******************Table:cpssDxChTtiMacToMe*****************************************/

static GT_U8 macToMeTableIndex;

/**
* @internal wrCpssDxChTtiMacToMeSet function
* @endinternal
*
* @brief   This function sets the TTI MacToMe relevant Mac address and Vlan.
*         if a match is found, an internal flag is set. The MAC2ME flag
*         is optionally used as part of the TT lookup trigger for IPV4 or MPLS traffic.
*         Note: if the packet is TT and the pasenger is Ethernet, another MAC2ME
*         lookup is performed and the internal flag is set accordingly.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiMacToMeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mac2MeIndex;
    CPSS_DXCH_TTI_MAC_VLAN_STC macVlan;
    CPSS_DXCH_TTI_MAC_VLAN_STC mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mac2MeIndex=(GT_U8)inFields[0];
    galtisMacAddr(&macVlan.mac,(GT_U8*)inFields[1]);
    macVlan.vlanId=(GT_U16)inFields[2];
    galtisMacAddr(&mask.mac,(GT_U8*)inFields[3]);
    mask.vlanId=(GT_U16)inFields[4];

    /* use only mac and vlan fields */
    cmdOsMemSet(&interfaceInfo, 0, sizeof(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC));
    interfaceInfo.includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiMacToMeSet(devNum, mac2MeIndex, &macVlan, &mask,
                                           &interfaceInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMacToMe_1Set function
* @endinternal
*
* @brief   function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMacToMe_1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mac2MeIndex;
    CPSS_DXCH_TTI_MAC_VLAN_STC macVlan;
    CPSS_DXCH_TTI_MAC_VLAN_STC mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mac2MeIndex=(GT_U8)inFields[0];
    galtisMacAddr(&macVlan.mac,(GT_U8*)inFields[1]);
    macVlan.vlanId=(GT_U16)inFields[2];
    galtisMacAddr(&mask.mac,(GT_U8*)inFields[3]);
    mask.vlanId=(GT_U16)inFields[4];
    interfaceInfo.includeSrcInterface =
        (CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT)inFields[5];
    interfaceInfo.srcHwDevice = (GT_HW_DEV_NUM)inFields[6];
    interfaceInfo.srcIsTrunk = (GT_BOOL)inFields[7];
    interfaceInfo.srcPortTrunk = (GT_U32)inFields[8];

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiMacToMeSet(devNum, mac2MeIndex, &macVlan, &mask,
                                           &interfaceInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMacToMeGetEntry function
* @endinternal
*
* @brief   This function sets the TTI MacToMe relevant Mac address and Vlan.
*         if a match is found, an internal flag is set. The MAC2ME flag
*         is optionally used as part of the TT lookup trigger for IPV4 or MPLS traffic.
*         Note: if the packet is TT and the pasenger is Ethernet, another MAC2ME
*         lookup is performed and the internal flag is set accordingly.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiMacToMeGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;

    CPSS_DXCH_TTI_MAC_VLAN_STC macVlan;
    CPSS_DXCH_TTI_MAC_VLAN_STC mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC interfaceInfo;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (macToMeTableIndex>7)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* initialize macVlan and mask */
    cmdOsMemSet(&macVlan , 0, sizeof(macVlan));
    cmdOsMemSet(&mask , 0, sizeof(mask));

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiMacToMeGet(devNum, macToMeTableIndex,
                                           &interfaceInfo, &macVlan, &mask);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0]= macToMeTableIndex;
    inFields[2]=macVlan.vlanId;
    inFields[4]=mask.vlanId;


    fieldOutput("%d%6b%d%6b%d", inFields[0],macVlan.mac.arEther,inFields[2],mask.mac.arEther,inFields[4]);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMacToMe_1GetEntry function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMacToMe_1GetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U8 lastMacToMeEntryIndex;
    GT_U8 firstEntry, numOfEntries;

    CPSS_DXCH_TTI_MAC_VLAN_STC macVlan;
    CPSS_DXCH_TTI_MAC_VLAN_STC mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC interfaceInfo;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    firstEntry = (GT_U8)inArgs[1];
    numOfEntries = (GT_U8)inArgs[2];

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        lastMacToMeEntryIndex = 127;
    }
    else
    {
        lastMacToMeEntryIndex = 7;
    }

    if (macToMeTableIndex == 0)
    {
        macToMeTableIndex = firstEntry;
    }

    if (firstEntry + numOfEntries - 1 < lastMacToMeEntryIndex)
    {
        lastMacToMeEntryIndex = firstEntry + numOfEntries - 1;
    }

    if (macToMeTableIndex > lastMacToMeEntryIndex)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* initialize macVlan and mask */
    cmdOsMemSet(&macVlan , 0, sizeof(macVlan));
    cmdOsMemSet(&mask , 0, sizeof(mask));
    cmdOsMemSet(&interfaceInfo, 0, sizeof(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC));

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiMacToMeGet(devNum, macToMeTableIndex,
                                           &interfaceInfo, &macVlan, &mask);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = macToMeTableIndex;
    inFields[2] = macVlan.vlanId;
    inFields[4] = mask.vlanId;
    inFields[5] = interfaceInfo.includeSrcInterface;
    inFields[6] = interfaceInfo.srcHwDevice;
    inFields[7] = interfaceInfo.srcIsTrunk;
    inFields[8] = interfaceInfo.srcPortTrunk;

    fieldOutput("%d%6b%d%6b%d%d%d%d%d", inFields[0], macVlan.mac.arEther,
                inFields[2], mask.mac.arEther, inFields[4], inFields[5],
                inFields[6], inFields[7], inFields[8]);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**********************************************************************************/

static CMD_STATUS wrCpssDxChTtiMacToMeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    macToMeTableIndex=0;

    return wrCpssDxChTtiMacToMeGetEntry(inArgs,inFields,numFields,outArgs);
}

/**********************************************************************************/

static CMD_STATUS wrCpssDxChTtiMacToMeGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    macToMeTableIndex++;

    return wrCpssDxChTtiMacToMeGetEntry(inArgs,inFields,numFields,outArgs);

}

/**********************************************************************************/

static CMD_STATUS wrCpssDxChTtiMacToMe_1GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    macToMeTableIndex=0;

    return wrCpssDxChTtiMacToMe_1GetEntry(inArgs,inFields,numFields,outArgs);
}

/**********************************************************************************/

static CMD_STATUS wrCpssDxChTtiMacToMe_1GetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    macToMeTableIndex++;

    return wrCpssDxChTtiMacToMe_1GetEntry(inArgs,inFields,numFields,outArgs);

}

/**
* @internal wrCpssDxChTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port.
*
* @note   APPLICABLE DEVICES:      All DXCH devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortLookupEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    CPSS_DXCH_TTI_KEY_TYPE_ENT keyType;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortLookupEnableSet(devNum, port, keyType, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortLookupEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         TTI lookup for the specified key type.
*
* @note   APPLICABLE DEVICES:      All DXCH devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id, port or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortLookupEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    CPSS_DXCH_TTI_KEY_TYPE_ENT keyType;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortLookupEnableGet(devNum, port, keyType, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortIpv4OnlyTunneledEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only tunneled packets received on port.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortIpv4OnlyTunneledEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortIpv4OnlyMac2MeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortIpv4OnlyMac2MeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortIpv4OnlyMac2MeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortIpv4OnlyMac2MeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIpv4McEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for IPv4 multicast
*         (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiIpv4McEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiIpv4McEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIpv4McEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of TTI lookup for
*         IPv4 multicast (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiIpv4McEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiIpv4McEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortMplsOnlyMac2MeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MPLS TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortMplsOnlyMac2MeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortMplsOnlyMac2MeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MPLS TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiPortMplsOnlyMac2MeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortMimOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MIM TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortMimOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MIM TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPortMimOnlyMacToMeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/*********************************/
/* cpssDxChTtiRuleAction Table */
/*********************************/

/* table cpssDxChTtiRuleAction global variables */
#define ACTION_TABLE_SIZE 1024

/* Database to hold TTI actions that has been set.
   Non valid entries maintain link list of free table entries */
typedef struct
{
    GT_BOOL                         valid;          /* weather this entry is valid           */
    CPSS_DXCH_TTI_ACTION_STC        actionEntry;    /* action data                           */
    GT_U32                          ruleIndex;      /* rule index associated with the action */
    GT_U8                           devNum;         /* device associated with the action     */
    GT_U32                          nextFree;       /* in case not valid, next free entry    */
}ttiActionDB;

static ttiActionDB ttiActionDatabase[ACTION_TABLE_SIZE];

/* index of first free TTI action entry in the action table */
static GT_U32   firstFree = 0;
/* weather TTI action was used before or not (used to invalidate all entries
   on first time the table is used) */
static GT_BOOL  firstRun = GT_TRUE;
/* indicated invalid rule entry */
#define INVALID_RULE_INDEX 0x7FFFFFF0
/* index to go over TTI actions */
static  GT_U32   ttiActionGetIndex;

/* Database to hold TTI Multi action that has been set.*/

typedef struct
{
    GT_U8                           devNum;         /* device associated with the action     */
    GT_U32                          numOfRulesToAdd;/* number of Rules and Actions to add    */
    GT_U32                          ruleIndex;      /* rule index associated with the action */
    GT_U32                          ruleIndexDelta; /* delta between 2 indexes               */

    CPSS_DXCH_TTI_ACTION_STC      actionEntry;    /* action data - only type2 is supported for multi action set                           */
    GT_PORT_NUM                     portNum_0;      /* port interface to use when (ruleIndex % 4 == 0)                                      */
    GT_PORT_NUM                     portNum_1;      /* port interface to use when (ruleIndex % 4 == 1)                                      */
    GT_PORT_NUM                     portNum_2;      /* port interface to use when (ruleIndex % 4 == 2)                                      */
    GT_PORT_NUM                     portNum_3;      /* port interface to use when (ruleIndex % 4 == 3)                                      */
}ttiMultiActionDB;

static CPSS_DXCH_TTI_RULE_UNT       ethRuleMaskData;
static CPSS_DXCH_TTI_RULE_UNT       ethRulePatternData;
static ttiMultiActionDB             ttiMultiActionDatabaseParamsForEthRule;

/* holding keyTypes UDB10, UDB20, UDB30 */
static CPSS_DXCH_TTI_RULE_UNT       udbRuleMaskDataArray[3];
static CPSS_DXCH_TTI_RULE_UNT       udbRulePatternDataArray[3];
static ttiMultiActionDB             ttiMultiActionDatabaseParamsForUdbRuleArray[3];

/**
* @internal ttiActionDatabaseGet function
* @endinternal
*
*/
static void ttiActionDatabaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  ruleIndex,
    OUT GT_U32                  *tableIndex
)
{
    GT_U32              index = 0;
    GT_BOOL             entryFound = GT_FALSE;

    while ((!entryFound) && (index < ACTION_TABLE_SIZE))
    {
        /* get next valid entry */
        while ((ttiActionDatabase[index].valid == GT_FALSE) && (index < ACTION_TABLE_SIZE))
            index++;
        /* check if the entry match the device number and rule index */
        if ((index < ACTION_TABLE_SIZE) &&
            (ttiActionDatabase[index].ruleIndex == ruleIndex) &&
            (ttiActionDatabase[index].devNum == devNum))
        {
            entryFound = GT_TRUE;
        }
        else
        {
            index++;
        }
    }

    if (entryFound)
    {
        *tableIndex = index;
    }
    else
    {
        *tableIndex = INVALID_RULE_INDEX;
    }
}

/**
* @internal ttiActionDatabaseInit function
* @endinternal
*
*/
static CMD_STATUS ttiActionDatabaseInit(void)
{
    GT_U32      index = 0;

    firstFree = 0;

    while (index < ACTION_TABLE_SIZE-1)
    {
        ttiActionDatabase[index].valid = GT_FALSE;
        ttiActionDatabase[index].nextFree = index + 1;
        index++;
    }
    ttiActionDatabase[index].nextFree = INVALID_RULE_INDEX;
    ttiActionDatabase[index].valid = GT_FALSE;

    return CMD_OK;
}

/*******************************************************************************/
static CMD_STATUS wrCpssDxChTtiRuleActionDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32                              ruleIndex;
    GT_U8                               devNum;
    GT_U32                              index;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum =  (GT_U8)inFields[0];
    ruleIndex = (GT_U32)inFields[1];

    ttiActionDatabaseGet(devNum, ruleIndex, &index);

    /* the rule is not found */
    if (index == INVALID_RULE_INDEX)
    {
        galtisOutput(outArgs, GT_NO_SUCH, "%d", -1);
        return CMD_AGENT_ERROR;
    }

    ttiActionDatabase[index].valid = GT_FALSE;
    ttiActionDatabase[index].nextFree = firstFree;
    firstFree = index;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************/
static CMD_STATUS wrCpssDxChTtiRuleActionClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    ttiActionDatabaseInit();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/***********************************/
/* cpssDxChTtiRuleAction_1 Table */
/***********************************/

/**
* @internal wrCpssDxChTtiRuleAction_1Update function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_1Update
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result = GT_OK;
    GT_U8                               devNum;
    GT_U32                              ruleIndex;
    GT_U32                              index;
    CPSS_DXCH_TTI_ACTION_STC            *actionPtr;
    GT_BOOL                             isNewIndex = GT_FALSE;
    ttiActionDB                         ttiTempAction;

    GT_UNUSED_PARAM(numFields);
    cpssOsMemSet(&ttiTempAction, 0, sizeof(ttiTempAction));

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inFields[0];
    ruleIndex = (GT_U32)inFields[1];
    if (ruleIndex > 3071)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Rule index has to be in range [0..3071].\n");

        return CMD_AGENT_ERROR;
    }

    /* in case it is first usage of TTI action database, initialize it */
    if (firstRun)
    {
        ttiActionDatabaseInit();
        firstRun = GT_FALSE;
    }

    /* look for the action in the TTI action database */
    ttiActionDatabaseGet(devNum, ruleIndex, &index);

    /* the action wasn't in the TTI action database */
    if (index == INVALID_RULE_INDEX)
    {
        if (firstFree == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
        firstFree = ttiActionDatabase[firstFree].nextFree;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&ttiTempAction,&ttiActionDatabase[index],sizeof(ttiTempAction));
    }

    /* set action entry in TTI action database as valid */
    ttiActionDatabase[index].valid      = GT_TRUE;
    ttiActionDatabase[index].ruleIndex  = ruleIndex;
    ttiActionDatabase[index].devNum     = devNum;

    actionPtr = &(ttiActionDatabase[index].actionEntry);

    /* fill action fields */
    actionPtr->tunnelTerminate                 = (GT_BOOL)inFields[2];
    actionPtr->ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[3];
    actionPtr->tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[4];
    actionPtr->copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[5];
    actionPtr->mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[6];
    actionPtr->mplsTtl                         = (GT_U32)inFields[7];
    actionPtr->enableDecrementTtl              = (GT_BOOL)inFields[8];
    actionPtr->command                         = (CPSS_PACKET_CMD_ENT)inFields[9];
    actionPtr->redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[10];
    actionPtr->egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[11];
    actionPtr->egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[12];
    actionPtr->egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[13];
    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(actionPtr->egressInterface.devPort.hwDevNum, actionPtr->egressInterface.devPort.portNum);
    actionPtr->egressInterface.trunkId         = (GT_TRUNK_ID)inFields[14];
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->egressInterface.trunkId);
    actionPtr->egressInterface.vidx            = (GT_U16)inFields[15];
    actionPtr->egressInterface.vlanId          = (GT_U16)inFields[16];
    actionPtr->arpPtr                          = (GT_U32)inFields[17];
    actionPtr->tunnelStart                     = (GT_BOOL)inFields[18];
    actionPtr->tunnelStartPtr                  = (GT_U32)inFields[19];
    actionPtr->routerLttPtr                    = (GT_U32)inFields[20];
    actionPtr->vrfId                           = (GT_U32)inFields[21];
    actionPtr->sourceIdSetEnable               = (GT_BOOL)inFields[22];
    actionPtr->sourceId                        = (GT_U32)inFields[23];
    actionPtr->tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[24];
    actionPtr->tag0VlanId                      = (GT_U16)inFields[25];
    actionPtr->tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[26];
    actionPtr->tag1VlanId                      = (GT_U16)inFields[27];
    actionPtr->tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[28];
    actionPtr->nestedVlanEnable                = (GT_BOOL)inFields[29];
    actionPtr->bindToPolicerMeter              = (GT_BOOL)inFields[30];
    actionPtr->bindToPolicer                   = (GT_BOOL)inFields[31];
    actionPtr->policerIndex                    = (GT_U32)inFields[32];
    actionPtr->qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[33];
    actionPtr->keepPreviousQoS                 = (GT_BOOL)inFields[34];
    actionPtr->trustUp                         = (GT_BOOL)inFields[35];
    actionPtr->trustDscp                       = (GT_BOOL)inFields[36];
    actionPtr->trustExp                        = (GT_BOOL)inFields[37];
    actionPtr->qosProfile                      = (GT_U32)inFields[38];
    actionPtr->modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[39];
    actionPtr->tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[40];
    actionPtr->modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[41];
    actionPtr->tag0Up                          = (GT_U32)inFields[42];
    actionPtr->tag1Up                          = (GT_U32)inFields[43];
    actionPtr->remapDSCP                       = (GT_BOOL)inFields[44];
    actionPtr->pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[45];
    actionPtr->pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[46];
    actionPtr->pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[47];
    actionPtr->iPclConfigIndex                 = (GT_U32)inFields[48];
    actionPtr->mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[49];
    actionPtr->userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[50];
    actionPtr->bindToCentralCounter            = (GT_BOOL)inFields[51];
    actionPtr->centralCounterIndex             = (GT_U32)inFields[52];
    actionPtr->vntl2Echo                       = (GT_BOOL)inFields[53];
    actionPtr->bridgeBypass                    = (GT_BOOL)inFields[54];
    actionPtr->ingressPipeBypass               = (GT_BOOL)inFields[55];
    actionPtr->actionStop                      = (GT_BOOL)inFields[56];

    if (actionPtr->tunnelStart == GT_FALSE)
    {
        actionPtr->modifyMacDa = GT_TRUE;
        actionPtr->modifyMacSa = GT_TRUE;
    }
    else
    {
        actionPtr->modifyMacDa = GT_FALSE;
        actionPtr->modifyMacSa = GT_FALSE;
    }
       
    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiRuleActionUpdate(devNum,ruleIndex,actionPtr,GT_FALSE,isNewIndex);
    
    /* if the rule action didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            ttiActionDatabase[index].valid = GT_FALSE;
            ttiActionDatabase[index].nextFree = firstFree;
            firstFree = index;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&ttiActionDatabase[index],&ttiTempAction,sizeof(ttiTempAction));
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_1Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_ACTION_STC    *actionPtr;
    GT_HW_DEV_NUM               tempHwDev;    /* used for port,dev converting */
    GT_PORT_NUM                 tempPort;   /* used for port,dev converting */

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((ttiActionGetIndex < ACTION_TABLE_SIZE) &&
           (ttiActionDatabase[ttiActionGetIndex].valid == GT_FALSE))
        ttiActionGetIndex++;

    if (ttiActionGetIndex == ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    actionPtr = &(ttiActionDatabase[ttiActionGetIndex].actionEntry);

    inFields[0]  = ttiActionDatabase[ttiActionGetIndex].devNum;
    inFields[1]  = ttiActionDatabase[ttiActionGetIndex].ruleIndex;
    inFields[2]  = actionPtr->tunnelTerminate;
    inFields[3]  = actionPtr->ttPassengerPacketType;
    inFields[4]  = actionPtr->tsPassengerPacketType;
    inFields[5]  = actionPtr->copyTtlExpFromTunnelHeader;
    inFields[6]  = actionPtr->mplsCommand;
    inFields[7]  = actionPtr->mplsTtl;
    inFields[8]  = actionPtr->enableDecrementTtl;
    inFields[9]  = actionPtr->command;
    inFields[10] = actionPtr->redirectCommand;
    inFields[11] = actionPtr->egressInterface.type;
    tempHwDev      = actionPtr->egressInterface.devPort.hwDevNum;
    tempPort     = actionPtr->egressInterface.devPort.portNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
    inFields[12] = tempHwDev;
    inFields[13] = tempPort;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(actionPtr->egressInterface.trunkId);
    inFields[14] = actionPtr->egressInterface.trunkId;
    inFields[15] = actionPtr->egressInterface.vidx;
    inFields[16] = actionPtr->egressInterface.vlanId;
    inFields[17] = actionPtr->arpPtr;
    inFields[18] = actionPtr->tunnelStart;
    inFields[19] = actionPtr->tunnelStartPtr;
    inFields[20] = actionPtr->routerLttPtr;
    inFields[21] = actionPtr->vrfId;
    inFields[22] = actionPtr->sourceIdSetEnable;
    inFields[23] = actionPtr->sourceId;
    inFields[24] = actionPtr->tag0VlanCmd;
    inFields[25] = actionPtr->tag0VlanId;
    inFields[26] = actionPtr->tag1VlanCmd;
    inFields[27] = actionPtr->tag1VlanId;
    inFields[28] = actionPtr->tag0VlanPrecedence;
    inFields[29] = actionPtr->nestedVlanEnable;
    inFields[30] = actionPtr->bindToPolicerMeter;
    inFields[31] = actionPtr->bindToPolicer;
    inFields[32] = actionPtr->policerIndex;
    inFields[33] = actionPtr->qosPrecedence;
    inFields[34] = actionPtr->keepPreviousQoS;
    inFields[35] = actionPtr->trustUp;
    inFields[36] = actionPtr->trustDscp;
    inFields[37] = actionPtr->trustExp;
    inFields[38] = actionPtr->qosProfile;
    inFields[39] = actionPtr->modifyTag0Up;
    inFields[40] = actionPtr->tag1UpCommand;
    inFields[41] = actionPtr->modifyDscp;
    inFields[42] = actionPtr->tag0Up;
    inFields[43] = actionPtr->tag1Up;
    inFields[44] = actionPtr->remapDSCP;
    inFields[45] = actionPtr->pcl0OverrideConfigIndex;
    inFields[46] = actionPtr->pcl0_1OverrideConfigIndex;
    inFields[47] = actionPtr->pcl1OverrideConfigIndex;
    inFields[48] = actionPtr->iPclConfigIndex;
    inFields[49] = actionPtr->mirrorToIngressAnalyzerEnable;
    inFields[50] = actionPtr->userDefinedCpuCode;
    inFields[51] = actionPtr->bindToCentralCounter;
    inFields[52] = actionPtr->centralCounterIndex;
    inFields[53] = actionPtr->vntl2Echo;
    inFields[54] = actionPtr->bridgeBypass;
    inFields[55] = actionPtr->ingressPipeBypass;
    inFields[56] = actionPtr->actionStop;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],   inFields[2],   inFields[3],
                inFields[4],  inFields[5],   inFields[6],   inFields[7],
                inFields[8],  inFields[9],   inFields[10],  inFields[11],
                inFields[12], inFields[13],  inFields[14],  inFields[15],
                inFields[16], inFields[17],  inFields[18],  inFields[19],
                inFields[20], inFields[21],  inFields[22],  inFields[23],
                inFields[24], inFields[25],  inFields[26],  inFields[27],
                inFields[28], inFields[29],  inFields[30],  inFields[31],
                inFields[32], inFields[33],  inFields[34],  inFields[35],
                inFields[36], inFields[37],  inFields[38],  inFields[39],
                inFields[40], inFields[41],  inFields[42],  inFields[43],
                inFields[44], inFields[45],  inFields[46],  inFields[47],
                inFields[48], inFields[49],  inFields[50],  inFields[51],
                inFields[52], inFields[53],  inFields[54],  inFields[55],
                inFields[56]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", 1);

    ttiActionGetIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_1GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_1GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ttiActionGetIndex = 0;

    return wrCpssDxChTtiRuleAction_1Get(inArgs,inFields,numFields,outArgs);
}

/*************************/
/* cpssDxChTtiRule Table */
/*************************/

/* tables cpssExMxPmTtiRule global variables */

static  CPSS_DXCH_TTI_RULE_UNT          maskData;
static  CPSS_DXCH_TTI_RULE_UNT          patternData;
static  CPSS_DXCH_TTI_RULE_TYPE_ENT     ruleFormat;
static  GT_BOOL                         mask_set = GT_FALSE;    /* is mask set   */
static  GT_BOOL                         pattern_set = GT_FALSE; /* is pattern set*/
static  GT_U32                          mask_ruleIndex = 0;
static  GT_U32                          pattern_ruleIndex = 0;

/* variables for TTI rule table entries get */

/* table to hold the rule type of each added rule */
static  CPSS_DXCH_TTI_RULE_TYPE_ENT     ttiRuleFormat[36864];
/* table to hold the action type for each rule entry set (used in rule entry get) */

static  GT_U32                          ruleIndex;
static  GT_U32                          ruleIndexMaxGet;
static  GT_BOOL                         isMask;

/* convert trunk fields of test for mask ,pattern of cpss */
static void ttiRuleTrunkInfoFromTestToCpss
(
    INOUT CPSS_DXCH_TTI_RULE_COMMON_STC *commonPtr,
    IN GT_U32                  srcPortTrunk,
    IN GT_U32                  dsaSrcPortTrunk
)
{
    CPSS_DXCH_TTI_RULE_COMMON_STC *patternCommonPtr =
        &patternData.ipv4.common;/* all tti rules starts with the 'common'*/
    CPSS_DXCH_TTI_RULE_COMMON_STC *maskCommonPtr =
        &maskData.ipv4.common;/* all tti rules starts with the 'common'*/
    GT_TRUNK_ID trunkId;

    /*srcPortTrunk*/
    {
        commonPtr->srcPortTrunk = srcPortTrunk;

        trunkId = (GT_TRUNK_ID)commonPtr->srcPortTrunk;
        if(commonPtr == maskCommonPtr)
        {
            /* we do the mask now */
            gtPclMaskTrunkConvertFromTestToCpss(
                maskCommonPtr->srcIsTrunk,
                patternCommonPtr->srcIsTrunk,
                &trunkId);
        }
        else
        {
            /* we do the pattern now */
            gtPclPatternTrunkConvertFromTestToCpss(
                maskCommonPtr->srcIsTrunk,
                patternCommonPtr->srcIsTrunk,
                &trunkId);
        }

        commonPtr->srcPortTrunk = trunkId;
    }

    /*dsaSrcPortTrunk*/
    {
        commonPtr->dsaSrcPortTrunk = dsaSrcPortTrunk;

        trunkId = (GT_TRUNK_ID)commonPtr->dsaSrcPortTrunk;
        if(commonPtr == maskCommonPtr)
        {
            /* we do the mask now */
            gtPclMaskTrunkConvertFromTestToCpss(
                maskCommonPtr->dsaSrcIsTrunk,
                patternCommonPtr->dsaSrcIsTrunk,
                &trunkId);
        }
        else
        {
            /* we do the pattern now */
            gtPclPatternTrunkConvertFromTestToCpss(
                maskCommonPtr->dsaSrcIsTrunk,
                patternCommonPtr->dsaSrcIsTrunk,
                &trunkId);
        }

        commonPtr->dsaSrcPortTrunk = trunkId;
    }


    return;
}

/* convert trunk fields of test for mask ,pattern of cpss */
static void ttiRuleTrunkInfoFromTestToCpss_1
(
    IN GT_U32       pattern
)
{
    if(pattern == 0)/*mask*/
    {
        if(pattern_set == GT_FALSE)
        {
            return;
        }
        /* we do mask now and pattern already done*/
        /* we can get full info */
    }
    else
    {
        if(pattern_set == GT_FALSE)
        {
            return;
        }

        /* we do pattern now and mask already done*/
        /* we can get full info */
    }

    ttiRuleTrunkInfoFromTestToCpss(
    &maskData.ipv4.common, /* all keys start with common */
    maskData.ipv4.common.srcPortTrunk,
    maskData.ipv4.common.dsaSrcPortTrunk);

    ttiRuleTrunkInfoFromTestToCpss(
    &patternData.ipv4.common, /* all keys start with common */
    patternData.ipv4.common.srcPortTrunk,
    patternData.ipv4.common.dsaSrcPortTrunk);
}


/* convert trunk fields of mask ,pattern back to test value */
static void ttiRuleTrunkInfoFromCpssToTest
(
    INOUT CPSS_DXCH_TTI_RULE_COMMON_STC *commonPtr
)
{
    CPSS_DXCH_TTI_RULE_COMMON_STC *patternCommonPtr =
        &patternData.ipv4.common;/* all tti rules starts with the 'common'*/
    CPSS_DXCH_TTI_RULE_COMMON_STC *maskCommonPtr =
        &maskData.ipv4.common;/* all tti rules starts with the 'common'*/
    GT_TRUNK_ID trunkId;

    {
        trunkId = (GT_TRUNK_ID)commonPtr->srcPortTrunk;
        if(commonPtr == maskCommonPtr)
        {
            /* we do the mask now */
            gtPclMaskTrunkConvertFromCpssToTest(
                maskCommonPtr->srcIsTrunk,
                patternCommonPtr->srcIsTrunk,
                &trunkId);
        }
        else
        {
            /* we do the pattern now */
            gtPclPatternTrunkConvertFromCpssToTest(
                maskCommonPtr->srcIsTrunk,
                patternCommonPtr->srcIsTrunk,
                &trunkId);
        }

        commonPtr->srcPortTrunk = trunkId;
    }

    {
        trunkId = (GT_TRUNK_ID)commonPtr->dsaSrcPortTrunk;
        if(commonPtr == maskCommonPtr)
        {
            /* we do the mask now */
            gtPclMaskTrunkConvertFromCpssToTest(
                maskCommonPtr->dsaSrcIsTrunk,
                patternCommonPtr->dsaSrcIsTrunk,
                &trunkId);
        }
        else
        {
            /* we do the pattern now */
            gtPclPatternTrunkConvertFromCpssToTest(
                maskCommonPtr->dsaSrcIsTrunk,
                patternCommonPtr->dsaSrcIsTrunk,
                &trunkId);
        }

        commonPtr->dsaSrcPortTrunk = trunkId;
    }


    return;
}


/* convert trunk fields of test for mask ,pattern of cpss */
static void ttiRuleTrunkInfoFromCpssToTest_1
(
    void
)
{
    ttiRuleTrunkInfoFromCpssToTest(
    &maskData.ipv4.common);

    ttiRuleTrunkInfoFromCpssToTest(
    &patternData.ipv4.common);
}


/**
* @internal wrCpssDxChTtiRule_KEY_IPV4_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_KEY_IPV4_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(outArgs);

    ruleFormat = CPSS_DXCH_TTI_KEY_IPV4_E;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr        = &maskData;
        mask_ruleIndex = (GT_U32)inFields[0];
    }
    else /* pattern */
    {
        rulePtr           = &patternData;
        pattern_ruleIndex = (GT_U32)inFields[0];
    }

    rulePtr->ipv4.common.pclId           = (GT_U32)inFields[2];
    rulePtr->ipv4.common.srcIsTrunk      = (GT_U32)inFields[3];
    rulePtr->ipv4.common.srcPortTrunk    = (GT_U32)inFields[4];

    if (rulePtr->ipv4.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->ipv4.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        rulePtr->ipv4.common.srcPortTrunk = (GT_U32)port;
    }

    galtisMacAddr(&rulePtr->ipv4.common.mac,(GT_U8*)inFields[5]);
    rulePtr->ipv4.common.vid             = (GT_U16)inFields[6];
    rulePtr->ipv4.common.isTagged        = (GT_BOOL)inFields[7];
    rulePtr->ipv4.tunneltype             = (GT_U32)inFields[8];
    galtisIpAddr(&rulePtr->ipv4.srcIp,(GT_U8*)inFields[9]);
    galtisIpAddr(&rulePtr->ipv4.destIp,(GT_U8*)inFields[10]);
    rulePtr->ipv4.isArp                  = (GT_BOOL)inFields[11];
    rulePtr->ipv4.common.dsaSrcIsTrunk   = (GT_BOOL)inFields[12];
    rulePtr->ipv4.common.dsaSrcPortTrunk =(GT_U32)inFields[13];
    rulePtr->ipv4.common.dsaSrcDevice    = (GT_U32)inFields[14];

    /* convert the trunk fields */
    ttiRuleTrunkInfoFromTestToCpss_1((GT_U32)inFields[1]);

    if (inFields[1] == 0) /* mask */
    {
        mask_set       = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_set       = GT_TRUE;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_KEY_MPLS_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_KEY_MPLS_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(outArgs);

    ruleFormat = CPSS_DXCH_TTI_KEY_MPLS_E;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr        = &maskData;
        mask_ruleIndex = (GT_U32)inFields[0];
    }
    else /* pattern */
    {
        rulePtr           = &patternData;
        pattern_ruleIndex = (GT_U32)inFields[0];
    }

    rulePtr->mpls.common.pclId           = (GT_U32)inFields[2];
    rulePtr->mpls.common.srcIsTrunk      = (GT_U32)inFields[3];
    rulePtr->mpls.common.srcPortTrunk    = (GT_U32)inFields[4];

    if (rulePtr->mpls.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mpls.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        rulePtr->mpls.common.srcPortTrunk = (GT_U32)port;
    }

    galtisMacAddr(&rulePtr->mpls.common.mac,(GT_U8*)inFields[5]);
    rulePtr->mpls.common.vid             = (GT_U16)inFields[6];
    rulePtr->mpls.common.isTagged        = (GT_BOOL)inFields[7];
    rulePtr->mpls.label0                 = (GT_U32)inFields[8];
    rulePtr->mpls.exp0                   = (GT_U32)inFields[9];
    rulePtr->mpls.label1                 = (GT_U32)inFields[10];
    rulePtr->mpls.exp1                   = (GT_U32)inFields[11];
    rulePtr->mpls.label2                 = (GT_U32)inFields[12];
    rulePtr->mpls.exp2                   = (GT_U32)inFields[13];
    rulePtr->mpls.numOfLabels            = (GT_U32)inFields[14];
    rulePtr->mpls.protocolAboveMPLS      = (GT_U32)inFields[15];
    rulePtr->mpls.common.dsaSrcIsTrunk   = (GT_BOOL)inFields[16];
    rulePtr->mpls.common.dsaSrcPortTrunk = (GT_U32)inFields[17];
    rulePtr->mpls.common.dsaSrcDevice    = (GT_U32)inFields[18];

    /* convert the trunk fields */
    ttiRuleTrunkInfoFromTestToCpss_1((GT_U32)inFields[1]);

    if (inFields[1] == 0) /* mask */
    {
        mask_set       = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_set       = GT_TRUE;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_KEY_ETH_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_KEY_ETH_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(outArgs);

    ruleFormat = CPSS_DXCH_TTI_KEY_ETH_E;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr        = &maskData;
        mask_ruleIndex = (GT_U32)inFields[0];
    }
    else /* pattern */
    {
        rulePtr           = &patternData;
        pattern_ruleIndex = (GT_U32)inFields[0];
    }

    rulePtr->eth.common.pclId           = (GT_U32)inFields[2];
    rulePtr->eth.common.srcIsTrunk      = (GT_U32)inFields[3];
    rulePtr->eth.common.srcPortTrunk    = (GT_U32)inFields[4];

    if (rulePtr->eth.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->eth.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        rulePtr->eth.common.srcPortTrunk = (GT_U32)port;
    }

    galtisMacAddr(&rulePtr->eth.common.mac,(GT_U8*)inFields[5]);
    rulePtr->eth.common.vid             = (GT_U16)inFields[6];
    rulePtr->eth.common.isTagged        = (GT_BOOL)inFields[7];
    rulePtr->eth.up0                    = (GT_U32)inFields[8];
    rulePtr->eth.cfi0                   = (GT_U32)inFields[9];
    rulePtr->eth.isVlan1Exists          = (GT_BOOL)inFields[10];
    rulePtr->eth.vid1                   = (GT_U16)inFields[11];
    rulePtr->eth.up1                    = (GT_U32)inFields[12];
    rulePtr->eth.cfi1                   = (GT_U32)inFields[13];
    rulePtr->eth.etherType              = (GT_U32)inFields[14];
    rulePtr->eth.macToMe                = (GT_BOOL)inFields[15];
    rulePtr->eth.common.dsaSrcIsTrunk   = inFields[16];
    rulePtr->eth.common.dsaSrcPortTrunk = (GT_U32)inFields[17];
    rulePtr->eth.common.dsaSrcDevice    = (GT_U32)inFields[18];

    /* convert the trunk fields */
    ttiRuleTrunkInfoFromTestToCpss_1((GT_U32)inFields[1]);

    if (inFields[1] == 0) /* mask */
    {
        mask_set       = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_set       = GT_TRUE;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleSetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;


    switch (inArgs[1])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleSetNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result = GT_OK;
    GT_U8                       devNum;
    GT_U32                      ruleIndex;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    GT_U32                      actionIndex;

    CPSS_DXCH_TTI_ACTION_STC    *actionPtr;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Check Validity */
    if (inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 0) && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 1) && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inFields[0];

    switch (inArgs[1])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }


    /* Get the action */
    ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

    if ((mask_set && pattern_set) && (actionIndex != INVALID_RULE_INDEX))
    {
        actionPtr = &(ttiActionDatabase[actionIndex].actionEntry);

        keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
        ttiRuleFormat[ruleIndex] = keyType;

        /* call cpss api function */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,ruleIndex,keyType,&patternData,&maskData, actionPtr,GT_FALSE);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleEndSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    if (mask_set || pattern_set)
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_KEY_IPV4_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_KEY_IPV4_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_IPV4_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->ipv4.common.pclId;
    inFields[3]  = rulePtr->ipv4.common.srcIsTrunk;
    inFields[4]  = rulePtr->ipv4.common.srcPortTrunk;

    if (rulePtr->ipv4.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->ipv4.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->ipv4.common.vid;
    inFields[7]  = rulePtr->ipv4.common.isTagged;
    inFields[8]  = rulePtr->ipv4.tunneltype;
    inFields[11] = rulePtr->ipv4.isArp;
    inFields[12] = rulePtr->ipv4.common.dsaSrcIsTrunk;
    inFields[13] = rulePtr->ipv4.common.dsaSrcPortTrunk;
    inFields[14] = rulePtr->ipv4.common.dsaSrcDevice;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%4b%4b%d%d%d%d",
                inFields[0], inFields[1],  inFields[2], inFields[3],
                inFields[4], rulePtr->ipv4.common.mac.arEther, inFields[6],
                inFields[7], inFields[8], rulePtr->ipv4.srcIp.arIP,
                rulePtr->ipv4.destIp.arIP, inFields[11], inFields[12],inFields[13],inFields[14]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_KEY_MPLS_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_KEY_MPLS_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_MPLS_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->mpls.common.pclId;
    inFields[3]  = rulePtr->mpls.common.srcIsTrunk;
    inFields[4]  = rulePtr->mpls.common.srcPortTrunk;

    if (rulePtr->mpls.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mpls.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->mpls.common.vid;
    inFields[7]  = rulePtr->mpls.common.isTagged;
    inFields[8]  = rulePtr->mpls.label0;
    inFields[9]  = rulePtr->mpls.exp0 ;
    inFields[10] = rulePtr->mpls.label1;
    inFields[11] = rulePtr->mpls.exp1;
    inFields[12] = rulePtr->mpls.label2;
    inFields[13] = rulePtr->mpls.exp2;
    inFields[14] = rulePtr->mpls.numOfLabels;
    inFields[15] = rulePtr->mpls.protocolAboveMPLS;
    inFields[16] = rulePtr->mpls.common.dsaSrcIsTrunk;
    inFields[17] = rulePtr->mpls.common.dsaSrcPortTrunk;
    inFields[18] = rulePtr->mpls.common.dsaSrcDevice;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->mpls.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11], inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;

}

/**
* @internal wrCpssDxChTtiRule_KEY_ETH_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_KEY_ETH_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_ETH_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->eth.common.pclId;
    inFields[3]  = rulePtr->eth.common.srcIsTrunk;
    inFields[4]  = rulePtr->eth.common.srcPortTrunk;

    if (rulePtr->eth.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->eth.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->eth.common.vid;
    inFields[7]  = rulePtr->eth.common.isTagged;
    inFields[8]  = rulePtr->eth.up0;
    inFields[9]  = rulePtr->eth.cfi0;
    inFields[10] = rulePtr->eth.isVlan1Exists;
    inFields[11] = rulePtr->eth.vid1;
    inFields[12] = rulePtr->eth.up1;
    inFields[13] = rulePtr->eth.cfi1;
    inFields[14] = rulePtr->eth.etherType;
    inFields[15] = rulePtr->eth.macToMe;
    inFields[16] = rulePtr->eth.common.dsaSrcIsTrunk;
    inFields[17] = rulePtr->eth.common.dsaSrcPortTrunk;
    inFields[18] = rulePtr->eth.common.dsaSrcDevice;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->eth.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11],inFields[12],inFields[13], inFields[14],
                    inFields[15],inFields[16],inFields[17],inFields[18]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiRuleGet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_U32                      actionIndex;

    /* if isMask == GT_TRUE it is mask's turn (the first one to get), so there need to refresh the data
    else the data is updated and it is pattern's turn (was updated earlier)*/
    if (isMask)
    {
        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];
        keyType = inArgs[1];

        actionIndex = INVALID_RULE_INDEX;
        for (; actionIndex == INVALID_RULE_INDEX && ruleIndex < 3072; ruleIndex++)
            ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

        if (ruleIndex == 3072)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_OK, "%d", -1);

            return CMD_OK;
        }

        if (actionIndex == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_OK, "%d", -1);

            return CMD_OK;
        }

        ruleIndex--;

        /* call cpss api function */
        result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,keyType,&patternData,&maskData, &action,GT_FALSE);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }
    }

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_KEY_ETH_Get(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (isMask)
        ruleIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleGetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ruleIndex = 0;

    /* first to get is mask */
    isMask = GT_TRUE;

    return wrCpssDxChTtiRuleGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_IPV4_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_IPV4_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* IPv4 key in TtiRule_1 table is the same as in TtiRule table */
    return wrCpssDxChTtiRule_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_MPLS_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_MPLS_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* MPLS key in TtiRule_1 table is the same as in TtiRule table */
    return wrCpssDxChTtiRule_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_ETH_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_ETH_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(outArgs);

    ruleFormat = CPSS_DXCH_TTI_KEY_ETH_E;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr        = &maskData;
        mask_ruleIndex = (GT_U32)inFields[0];
    }
    else /* pattern */
    {
        rulePtr           = &patternData;
        pattern_ruleIndex = (GT_U32)inFields[0];
    }

    rulePtr->eth.common.pclId           = (GT_U32)inFields[2];
    rulePtr->eth.common.srcIsTrunk      = (GT_U32)inFields[3];
    rulePtr->eth.common.srcPortTrunk    = (GT_U32)inFields[4];

    if (rulePtr->eth.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->eth.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        rulePtr->eth.common.srcPortTrunk = (GT_U32)port;
    }

    galtisMacAddr(&rulePtr->eth.common.mac,(GT_U8*)inFields[5]);
    rulePtr->eth.common.vid             = (GT_U16)inFields[6];
    rulePtr->eth.common.isTagged        = (GT_BOOL)inFields[7];
    rulePtr->eth.up0                    = (GT_U32)inFields[8];
    rulePtr->eth.cfi0                   = (GT_U32)inFields[9];
    rulePtr->eth.isVlan1Exists          = (GT_BOOL)inFields[10];
    rulePtr->eth.vid1                   = (GT_U16)inFields[11];
    rulePtr->eth.up1                    = (GT_U32)inFields[12];
    rulePtr->eth.cfi1                   = (GT_U32)inFields[13];
    rulePtr->eth.etherType              = (GT_U32)inFields[14];
    rulePtr->eth.macToMe                = (GT_BOOL)inFields[15];
    rulePtr->eth.common.dsaSrcIsTrunk   = inFields[16];
    rulePtr->eth.common.dsaSrcPortTrunk = (GT_U32)inFields[17];
    rulePtr->eth.common.dsaSrcDevice    = (GT_U32)inFields[18];
    rulePtr->eth.srcId                  = (GT_U32)inFields[19];
    rulePtr->eth.dsaQosProfile          = (GT_U32)inFields[20];

    /* convert the trunk fields */
    ttiRuleTrunkInfoFromTestToCpss_1((GT_U32)inFields[1]);


    if (inFields[1] == 0) /* mask */
    {
        mask_set       = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_set       = GT_TRUE;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_MIM_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_MIM_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(outArgs);

    ruleFormat = CPSS_DXCH_TTI_KEY_MIM_E;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr        = &maskData;
        mask_ruleIndex = (GT_U32)inFields[0];
    }
    else /* pattern */
    {
        rulePtr           = &patternData;
        pattern_ruleIndex = (GT_U32)inFields[0];
    }

    rulePtr->mim.common.pclId           = (GT_U32)inFields[2];
    rulePtr->mim.common.srcIsTrunk      = (GT_BOOL)inFields[3];
    rulePtr->mim.common.srcPortTrunk    = (GT_U32)inFields[4];

    if (rulePtr->mim.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mim.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        rulePtr->mim.common.srcPortTrunk = (GT_U32)port;
    }

    galtisMacAddr(&rulePtr->mim.common.mac,(GT_U8*)inFields[5]);
    rulePtr->mim.common.vid             = (GT_U16)inFields[6];
    rulePtr->mim.common.isTagged        = (GT_BOOL)inFields[7];
    rulePtr->mim.common.dsaSrcIsTrunk   = (GT_BOOL)inFields[8];
    rulePtr->mim.common.dsaSrcPortTrunk = (GT_U32)inFields[9];
    rulePtr->mim.common.dsaSrcDevice    = (GT_U32)inFields[10];
    rulePtr->mim.bUp                    = (GT_U32)inFields[11];
    rulePtr->mim.bDp                    = (GT_U32)inFields[12];
    rulePtr->mim.iSid                   = (GT_U32)inFields[13];
    rulePtr->mim.iUp                    = (GT_U32)inFields[14];
    rulePtr->mim.iDp                    = (GT_U32)inFields[15];
    rulePtr->mim.iRes1                  = (GT_U32)inFields[16];
    rulePtr->mim.iRes2                  = (GT_U32)inFields[17];

    /* convert the trunk fields */
    ttiRuleTrunkInfoFromTestToCpss_1((GT_U32)inFields[1]);


    if (inFields[1] == 0) /* mask */
    {
        mask_set       = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_set       = GT_TRUE;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1SetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1SetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    switch (inArgs[3]) /* key type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_1_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_1_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_1_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_1_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1SetNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1SetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;
    GT_U32                          ruleIndex;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType;
    GT_U32                          actionIndex;
    CPSS_DXCH_TTI_ACTION_STC        *actionPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Check Validity */
    if (inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 0) && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 1) && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inFields[0];

    switch (inArgs[3]) /* key type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_1_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_1_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_1_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_1_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }


    /* Get the action */
    ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

    if ((mask_set && pattern_set) && (actionIndex != INVALID_RULE_INDEX))
    {
        actionPtr                    = &(ttiActionDatabase[actionIndex].actionEntry);
        keyType                      = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[3];
        ttiRuleFormat[ruleIndex]     = keyType;
 
        /* call cpss api function */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,ruleIndex,keyType,&patternData,&maskData,actionPtr,GT_FALSE);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1EndSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1EndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    if (mask_set || pattern_set)
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_IPV4_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_IPV4_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* IPv4 key in TtiRule_1 table is the same as in TtiRule table */
    return wrCpssDxChTtiRule_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_MPLS_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_MPLS_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* MPLS key in TtiRule_1 table is the same as in TtiRule table */
    return wrCpssDxChTtiRule_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_ETH_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_ETH_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_ETH_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->eth.common.pclId;
    inFields[3]  = rulePtr->eth.common.srcIsTrunk;
    inFields[4]  = rulePtr->eth.common.srcPortTrunk;

    if (rulePtr->eth.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->eth.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->eth.common.vid;
    inFields[7]  = rulePtr->eth.common.isTagged;
    inFields[8]  = rulePtr->eth.up0;
    inFields[9]  = rulePtr->eth.cfi0;
    inFields[10] = rulePtr->eth.isVlan1Exists;
    inFields[11] = rulePtr->eth.vid1;
    inFields[12] = rulePtr->eth.up1;
    inFields[13] = rulePtr->eth.cfi1;
    inFields[14] = rulePtr->eth.etherType;
    inFields[15] = rulePtr->eth.macToMe;
    inFields[16] = rulePtr->eth.common.dsaSrcIsTrunk;
    inFields[17] = rulePtr->eth.common.dsaSrcPortTrunk;
    inFields[18] = rulePtr->eth.common.dsaSrcDevice;
    inFields[19] = rulePtr->eth.srcId;
    inFields[20] = rulePtr->eth.dsaQosProfile;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->eth.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11],inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18],
                inFields[19],inFields[20]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1_KEY_MIM_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1_KEY_MIM_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_MIM_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2] = rulePtr->mim.common.pclId;
    inFields[3] = rulePtr->mim.common.srcIsTrunk;
    inFields[4] = rulePtr->mim.common.srcPortTrunk;

    if (rulePtr->mim.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mim.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6] = rulePtr->mim.common.vid;
    inFields[7] = rulePtr->mim.common.isTagged;
    inFields[8] = rulePtr->mim.common.dsaSrcIsTrunk;
    inFields[9] = rulePtr->mim.common.dsaSrcPortTrunk;
    inFields[10] = rulePtr->mim.common.dsaSrcDevice;
    inFields[11] = rulePtr->mim.bUp;
    inFields[12] = rulePtr->mim.bDp;
    inFields[13] = rulePtr->mim.iSid;
    inFields[14] = rulePtr->mim.iUp;
    inFields[15] = rulePtr->mim.iDp;
    inFields[16] = rulePtr->mim.iRes1;
    inFields[17] = rulePtr->mim.iRes2;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->eth.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11],inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_BOOL                     ruleValid;

    /* if isMask == GT_TRUE it is mask's turn (the first one to get), so there need to refresh the data
    else the data is updated and it is pattern's turn (was updated earlier)*/
    if (isMask)
    {
        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        /* look for another valid rule within the range */
        for ((ruleValid = GT_FALSE) ; ((ruleValid == GT_FALSE) && (ruleIndex < 3072) && (ruleIndex <= ruleIndexMaxGet)) ; ruleIndex++)
        {
            /* check if the rule is valid */
            result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum,ruleIndex,&ruleValid,GT_FALSE);
            if (result != GT_OK)
            {
                galtisOutput(outArgs, result, "%d", -1);
                return CMD_OK;
            }
        }

        /* check if no valid rule was found in the requested range */
        if ((ruleValid == GT_FALSE))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* valid rule was found in the requested range */
        ruleIndex--;

        /* call cpss api function, note that it is assumed that rule action
           type is not modified after setting the rule */
        result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,ttiRuleFormat[ruleIndex],&patternData,&maskData,&action,GT_FALSE);
    }

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_1_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_1_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_1_KEY_ETH_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_1_KEY_MIM_Get(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (isMask)
        ruleIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_1GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_1GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ruleIndex = 0;

    /* first to get is mask */
    isMask = GT_TRUE;

    /* check if the application request 0 entries */
    if (inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ruleIndex       = (GT_U32)inArgs[1];
    ruleIndexMaxGet = ruleIndex + (GT_U32)inArgs[2] - 1;

    return wrCpssDxChTtiRule_1Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiMacModeSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT keyType;
    CPSS_DXCH_TTI_MAC_MODE_ENT macMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    macMode = (CPSS_DXCH_TTI_MAC_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiMacModeSet(devNum, keyType, macMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      All DXCH devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiMacModeGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT keyType;
    CPSS_DXCH_TTI_MAC_MODE_ENT macMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiMacModeGet(devNum, keyType, &macMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", macMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPclIdSet function
* @endinternal
*
* @brief   This function sets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
static CMD_STATUS wrCpssDxChTtiPclIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    GT_U32                      pclId;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum  = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    pclId   = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiPclIdSet(devNum,keyType,pclId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPclIdGet function
* @endinternal
*
* @brief   This function gets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
static CMD_STATUS wrCpssDxChTtiPclIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    GT_U32                      pclId;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum  = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiPclIdGet(devNum,keyType,&pclId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pclId);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIpv4GreEthTypeSet function
* @endinternal
*
* @brief   This function sets the IPv4 GRE protocol Ethernet type. Two Ethernet
*         types are supported, used by TTI for Ethernet over GRE tunnels
*         (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiIpv4GreEthTypeSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT  greTunnelType;
    GT_U32 ethType;
    GT_U32 tempGreTunnelType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    tempGreTunnelType = (GT_U32)inArgs[1]; /* need to convert from old enum */
    ethType = (GT_U32)inArgs[2];

    (tempGreTunnelType == 0) ?
        (greTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E) : (greTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E);

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeSet(devNum,greTunnelType,ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIpv4GreEthTypeGet function
* @endinternal
*
* @brief   This function gets the IPv4 GRE protocol Ethernet type. Two Ethernet
*         types are supported, used by TTI for Ethernet over GRE tunnels
*         (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      All DXCH3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or GRE tunnel type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiIpv4GreEthTypeGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT greTunnelType;
    GT_U32 ethType;
    GT_U32 tempGreTunnelType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    tempGreTunnelType = (GT_U32)inArgs[1]; /* need to convert from old enum */

    (tempGreTunnelType == 0) ?
        (greTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E) : (greTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E);

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeGet(devNum, greTunnelType, &ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ethType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiEthernetTypeSet function
* @endinternal
*
* @brief   This function sets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiEthernetTypeSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT  greTunnelType;
    GT_U32 ethType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    greTunnelType = (GT_U32)inArgs[1];
    ethType = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeSet(devNum,greTunnelType,ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiEthernetTypeGet function
* @endinternal
*
* @brief   This function gets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or GRE tunnel type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChTtiEthernetTypeGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT greTunnelType;
    GT_U32 ethType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    greTunnelType = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeGet(devNum, greTunnelType, &ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ethType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMimEthTypeSet function
* @endinternal
*
* @brief   This function sets the MIM Ethernet type.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note specialEthertypes is used to identify tunnel for incoming packets in
*       the TTI.
*       routerHdrAltIEthertype is used for outgoing tunnel in the header alteration.
*       Those registers are configured to have the same value.
*
*/
static CMD_STATUS wrCpssDxChTtiMimEthTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ethType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ethType = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeSet(devNum, CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E, ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMimEthTypeGet function
* @endinternal
*
* @brief   This function gets the MIM Ethernet type.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMimEthTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ethType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeGet(devNum, CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E, &ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ethType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMplsEthTypeSet function
* @endinternal
*
* @brief   This function sets the MPLS Ethernet type.
*
* @note   APPLICABLE DEVICES:      DxChXcat A1 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note mplsEthertypes is used to identify tunnel for incoming packets in
*       the TTI.
*       hdrAltMplsEthertype is used for outgoing tunnel in the header alteration.
*       Those registers are configured to have the same value.
*
*/
static CMD_STATUS wrCpssDxChTtiMplsEthTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ethType;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT  mplsTunnelType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ucMcSet = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[1];
    ethType = (GT_U32)inArgs[2];

    (ucMcSet == CPSS_IP_UNICAST_E) ?
        (mplsTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E) : (mplsTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E);

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeSet(devNum,mplsTunnelType,ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMplsEthTypeGet function
* @endinternal
*
* @brief   This function gets the MPLS Ethernet type.
*
* @note   APPLICABLE DEVICES:      DxChXcat A1 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMplsEthTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ethType;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT  mplsTunnelType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ucMcSet = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[1];

    (ucMcSet == CPSS_IP_UNICAST_E) ?
        (mplsTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E) : (mplsTunnelType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E);

    /* call cpss api function */
    result = cpssDxChTtiEthernetTypeGet(devNum,mplsTunnelType,&ethType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ethType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiExceptionCmdSet function
* @endinternal
*
* @brief   Set tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*
* @note Commands for the different exceptions are:
*       CPSS_DXCH_TTI_IPV4_HEADER_ERROR_E    -
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_DXCH_TTI_IPV4_OPTION_FRAG_ERROR_E -
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_DXCH_TTI_IPV4_UNSUP_GRE_ERROR_E  -
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*
*/
static CMD_STATUS wrCpssDxChTtiExceptionCmdSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_TTI_EXCEPTION_ENT exceptionType;
    CPSS_PACKET_CMD_ENT command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_EXCEPTION_ENT)inArgs[1];
    command = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error.
*
* @note Commands for the different exceptions are:
*       CPSS_DXCH_TTI_IPV4_HEADER_ERROR_E    -
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_DXCH_TTI_IPV4_OPTION_FRAG_ERROR_E -
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_DXCH_TTI_IPV4_UNSUP_GRE_ERROR_E  -
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*
*/
static CMD_STATUS wrCpssDxChTtiExceptionCmdGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_TTI_EXCEPTION_ENT exceptionType;
    CPSS_PACKET_CMD_ENT command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_EXCEPTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiExceptionCmdGet(devNum, exceptionType, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiRuleValidStatusSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    IN GT_U8   devNum;
    IN GT_U32  routerTtiTcamRow;
    IN GT_BOOL valid;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum           = (GT_U8)inArgs[0];
    routerTtiTcamRow = (GT_U32)inArgs[1];
    valid            = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiRuleValidStatusSet(devNum, routerTtiTcamRow, valid, GT_FALSE);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleValidStatus_2Set function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiRuleValidStatus_2Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    IN GT_U8   devNum;
    IN GT_U32  routerTtiTcamRow;
    IN GT_BOOL valid;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum           = (GT_U8)inArgs[0];
    routerTtiTcamRow = (GT_U32)inArgs[1];
    valid            = (GT_BOOL)inArgs[2];

    /* call cpss api function - routerTtiTcamRow is an absolute index */
    result = pg_wrap_cpssDxChTtiRuleValidStatusSet(devNum, routerTtiTcamRow, valid, GT_TRUE);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiRuleValidStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    IN  GT_U8   devNum;
    IN  GT_U32  routerTtiTcamRow;
    OUT GT_BOOL valid = GT_FALSE;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum           = (GT_U8)inArgs[0];
    routerTtiTcamRow = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum, routerTtiTcamRow, &valid, GT_FALSE);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", valid);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleValidStatus_2Get function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiRuleValidStatus_2Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    IN  GT_U8   devNum;
    IN  GT_U32  routerTtiTcamRow;
    OUT GT_BOOL valid = GT_FALSE;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum           = (GT_U8)inArgs[0];
    routerTtiTcamRow = (GT_U32)inArgs[1];

    /* call cpss api function - routerTtiTcamRow is an absolute index */
    result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum, routerTtiTcamRow, &valid, GT_TRUE);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", valid);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_IPV4_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_IPV4_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr = &maskData;
    }
    else /* pattern */
    {
        rulePtr = &patternData;
    }

    /* IPv4 key in TtiRule_2 table is the same as in TtiRule table except for field sourcePortGroupId */
    wrCpssDxChTtiRule_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
    rulePtr->ipv4.common.sourcePortGroupId = (GT_U32)inFields[15];

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_MPLS_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_MPLS_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr = &maskData;
    }
    else /* pattern */
    {
        rulePtr = &patternData;
    }

    /* MPLS key in TtiRule_2 table is the same as in TtiRule table except for field sourcePortGroupId */
    wrCpssDxChTtiRule_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
    rulePtr->mpls.common.sourcePortGroupId = (GT_U32)inFields[19];

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_ETH_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_ETH_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr = &maskData;
    }
    else /* pattern */
    {
        rulePtr = &patternData;
    }

    /* ETH key in TtiRule_2 table is the same as in TtiRule_1 table except for field sourcePortGroupId */
    wrCpssDxChTtiRule_1_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
    rulePtr->eth.common.sourcePortGroupId = (GT_U32)inFields[21];

    rulePtr->eth.eTagGrp = (GT_U32)inFields[22];

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_MIM_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_MIM_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr = &maskData;
    }
    else /* pattern */
    {
        rulePtr = &patternData;
    }

    /* MIM key in TtiRule_2 table is the same as in TtiRule_1 table except for field sourcePortGroupId */
    wrCpssDxChTtiRule_1_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
    rulePtr->mim.common.sourcePortGroupId = (GT_U32)inFields[18];

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2SetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2SetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    switch (inArgs[3]) /* key type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2SetNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2SetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;
    GT_U32                          ruleIndex;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType;
    GT_U32                          actionIndex;
    CPSS_DXCH_TTI_ACTION_STC        *actionPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Check Validity */
    if (inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 0) && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 1) && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inFields[0];

    switch (inArgs[3]) /* key type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }


    /* Get the action */
    ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

    if ((mask_set && pattern_set) && (actionIndex != INVALID_RULE_INDEX))
    {
        actionPtr                    = &(ttiActionDatabase[actionIndex].actionEntry);
        keyType                      = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[3];
        ttiRuleFormat[ruleIndex]     = keyType;
        
        /* call cpss api function */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,ruleIndex,keyType,&patternData,&maskData,actionPtr,GT_FALSE);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2EndSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2EndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    if (mask_set || pattern_set)
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2CancelSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2CancelSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : result is not GT_OK\n");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_IPV4_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_IPV4_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_IPV4_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->ipv4.common.pclId;
    inFields[3]  = rulePtr->ipv4.common.srcIsTrunk;
    inFields[4]  = rulePtr->ipv4.common.srcPortTrunk;

    if (rulePtr->ipv4.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->ipv4.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->ipv4.common.vid;
    inFields[7]  = rulePtr->ipv4.common.isTagged;
    inFields[8]  = rulePtr->ipv4.tunneltype;
    inFields[11] = rulePtr->ipv4.isArp;
    inFields[12] = rulePtr->ipv4.common.dsaSrcIsTrunk;
    inFields[13] = rulePtr->ipv4.common.dsaSrcPortTrunk;
    inFields[14] = rulePtr->ipv4.common.dsaSrcDevice;
    inFields[15] = rulePtr->ipv4.common.sourcePortGroupId;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%4b%4b%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2], inFields[3],
                inFields[4], rulePtr->ipv4.common.mac.arEther, inFields[6],
                inFields[7], inFields[8], rulePtr->ipv4.srcIp.arIP,
                rulePtr->ipv4.destIp.arIP, inFields[11], inFields[12],
                inFields[13],inFields[14],inFields[15]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_MPLS_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_MPLS_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_MPLS_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->mpls.common.pclId;
    inFields[3]  = rulePtr->mpls.common.srcIsTrunk;
    inFields[4]  = rulePtr->mpls.common.srcPortTrunk;

    if (rulePtr->mpls.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mpls.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->mpls.common.vid;
    inFields[7]  = rulePtr->mpls.common.isTagged;
    inFields[8]  = rulePtr->mpls.label0;
    inFields[9]  = rulePtr->mpls.exp0 ;
    inFields[10] = rulePtr->mpls.label1;
    inFields[11] = rulePtr->mpls.exp1;
    inFields[12] = rulePtr->mpls.label2;
    inFields[13] = rulePtr->mpls.exp2;
    inFields[14] = rulePtr->mpls.numOfLabels;
    inFields[15] = rulePtr->mpls.protocolAboveMPLS;
    inFields[16] = rulePtr->mpls.common.dsaSrcIsTrunk;
    inFields[17] = rulePtr->mpls.common.dsaSrcPortTrunk;
    inFields[18] = rulePtr->mpls.common.dsaSrcDevice;
    inFields[19] = rulePtr->mpls.common.sourcePortGroupId;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->mpls.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11], inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18],inFields[19]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_ETH_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_ETH_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_ETH_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->eth.common.pclId;
    inFields[3]  = rulePtr->eth.common.srcIsTrunk;
    inFields[4]  = rulePtr->eth.common.srcPortTrunk;

    if (rulePtr->eth.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->eth.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->eth.common.vid;
    inFields[7]  = rulePtr->eth.common.isTagged;
    inFields[8]  = rulePtr->eth.up0;
    inFields[9]  = rulePtr->eth.cfi0;
    inFields[10] = rulePtr->eth.isVlan1Exists;
    inFields[11] = rulePtr->eth.vid1;
    inFields[12] = rulePtr->eth.up1;
    inFields[13] = rulePtr->eth.cfi1;
    inFields[14] = rulePtr->eth.etherType;
    inFields[15] = rulePtr->eth.macToMe;
    inFields[16] = rulePtr->eth.common.dsaSrcIsTrunk;
    inFields[17] = rulePtr->eth.common.dsaSrcPortTrunk;
    inFields[18] = rulePtr->eth.common.dsaSrcDevice;
    inFields[19] = rulePtr->eth.srcId;
    inFields[20] = rulePtr->eth.dsaQosProfile;
    inFields[21] = rulePtr->eth.common.sourcePortGroupId;
    inFields[22] = rulePtr->eth.eTagGrp;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->eth.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11],inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18],
                inFields[19],inFields[20], inFields[21], inFields[22]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2_KEY_MIM_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2_KEY_MIM_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_MIM_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2] = rulePtr->mim.common.pclId;
    inFields[3] = rulePtr->mim.common.srcIsTrunk;
    inFields[4] = rulePtr->mim.common.srcPortTrunk;

    if (rulePtr->mim.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mim.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6] = rulePtr->mim.common.vid;
    inFields[7] = rulePtr->mim.common.isTagged;
    inFields[8] = rulePtr->mim.common.dsaSrcIsTrunk;
    inFields[9] = rulePtr->mim.common.dsaSrcPortTrunk;
    inFields[10] = rulePtr->mim.common.dsaSrcDevice;
    inFields[11] = rulePtr->mim.bUp;
    inFields[12] = rulePtr->mim.bDp;
    inFields[13] = rulePtr->mim.iSid;
    inFields[14] = rulePtr->mim.iUp;
    inFields[15] = rulePtr->mim.iDp;
    inFields[16] = rulePtr->mim.iRes1;
    inFields[17] = rulePtr->mim.iRes2;
    inFields[18] = rulePtr->mim.common.sourcePortGroupId;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->eth.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11],inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_2Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_BOOL                     ruleValid;

    /* if isMask == GT_TRUE it is mask's turn (the first one to get), so there need to refresh the data
    else the data is updated and it is pattern's turn (was updated earlier)*/
    if (isMask)
    {
        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        /* look for another valid rule within the range */
        for ((ruleValid = GT_FALSE) ; ((ruleValid == GT_FALSE) && (ruleIndex < 3072) && (ruleIndex <= ruleIndexMaxGet)) ; ruleIndex++)
        {
            /* check if the rule is valid */
            result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum,ruleIndex,&ruleValid,GT_FALSE);
            if (result != GT_OK)
            {
                galtisOutput(outArgs, result, "%d", -1);
                return CMD_OK;
            }
        }

        /* check if no valid rule was found in the requested range */
        if ((ruleValid == GT_FALSE))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* valid rule was found in the requested range */
        ruleIndex--;

        /* call cpss api function, note that it is assumed that rule action
           type is not modified after setting the rule */
        result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,ttiRuleFormat[ruleIndex],&patternData,&maskData,&action,GT_FALSE);
    }

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Get(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (isMask)
        ruleIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleActionHwGet function
* @endinternal
*
* @brief   The function gets the Rule Action
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChTtiRuleActionHwGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_HW_DEV_NUM               tempHwDev;    /* used for port,dev converting */
    GT_PORT_NUM                 tempPort;   /* used for port,dev converting */
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;

    GT_UNUSED_PARAM(numFields);

    /* check if requested refresh range is already refreshed */
    if (ruleIndex > ruleIndexMaxGet)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType= (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[3];


    /* call cpss api function, note that it is assumed that rule action
    type is not modified after setting the rule */

    result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,keyType,&patternData,&maskData,&action,GT_FALSE);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0]  = devNum;
    inFields[1]  = ruleIndex;
    inFields[2]  = action.tunnelTerminate;
    inFields[3]  = action.ttPassengerPacketType;
    inFields[4]  = action.tsPassengerPacketType;
    inFields[5]  = action.copyTtlExpFromTunnelHeader;
    inFields[6]  = action.mplsCommand;
    inFields[7]  = action.mplsTtl;
    inFields[8]  = action.enableDecrementTtl;
    inFields[9]  = action.command;
    inFields[10] = action.redirectCommand;
    inFields[11] = action.egressInterface.type;
    tempHwDev      = action.egressInterface.devPort.hwDevNum;
    tempPort     = action.egressInterface.devPort.portNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
    inFields[12] = tempHwDev;
    inFields[13] = tempPort;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(action.egressInterface.trunkId);
    inFields[14] = action.egressInterface.trunkId;
    inFields[15] = action.egressInterface.vidx;
    inFields[16] = action.egressInterface.vlanId;
    inFields[17] = action.arpPtr;
    inFields[18] = action.tunnelStart;
    inFields[19] = action.tunnelStartPtr;
    inFields[20] = action.routerLttPtr;
    inFields[21] = action.vrfId;
    inFields[22] = action.sourceIdSetEnable;
    inFields[23] = action.sourceId;
    inFields[24] = action.tag0VlanCmd;
    inFields[25] = action.tag0VlanId;
    inFields[26] = action.tag1VlanCmd;
    inFields[27] = action.tag1VlanId;
    inFields[28] = action.tag0VlanPrecedence;
    inFields[29] = action.nestedVlanEnable;
    inFields[30] = action.bindToPolicerMeter;
    inFields[31] = action.bindToPolicer;
    inFields[32] = action.policerIndex;
    inFields[33] = action.qosPrecedence;
    inFields[34] = action.keepPreviousQoS;
    inFields[35] = action.trustUp;
    inFields[36] = action.trustDscp;
    inFields[37] = action.trustExp;
    inFields[38] = action.qosProfile;
    inFields[39] = action.modifyTag0Up;
    inFields[40] = action.tag1UpCommand;
    inFields[41] = action.modifyDscp;
    inFields[42] = action.tag0Up;
    inFields[43] = action.tag1Up;
    inFields[44] = action.remapDSCP;
    inFields[45] = action.pcl0OverrideConfigIndex;
    inFields[46] = action.pcl0_1OverrideConfigIndex;
    inFields[47] = action.pcl1OverrideConfigIndex;
    inFields[48] = action.iPclConfigIndex;
    inFields[49] = action.mirrorToIngressAnalyzerEnable;
    inFields[50] = action.userDefinedCpuCode;
    inFields[51] = action.bindToCentralCounter;
    inFields[52] = action.centralCounterIndex;
    inFields[53] = action.vntl2Echo;
    inFields[54] = action.bridgeBypass;
    inFields[55] = action.ingressPipeBypass;
    inFields[56] = action.actionStop;
    inFields[57] = action.hashMaskIndex;
    inFields[58] = action.modifyMacSa;
    inFields[59] = action.modifyMacDa;
    inFields[60] = action.ResetSrcPortGroupId;
    inFields[61] = action.multiPortGroupTtiEnable;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],   inFields[2],   inFields[3],
                inFields[4],  inFields[5],   inFields[6],   inFields[7],
                inFields[8],  inFields[9],   inFields[10],  inFields[11],
                inFields[12], inFields[13],  inFields[14],  inFields[15],
                inFields[16], inFields[17],  inFields[18],  inFields[19],
                inFields[20], inFields[21],  inFields[22],  inFields[23],
                inFields[24], inFields[25],  inFields[26],  inFields[27],
                inFields[28], inFields[29],  inFields[30],  inFields[31],
                inFields[32], inFields[33],  inFields[34],  inFields[35],
                inFields[36], inFields[37],  inFields[38],  inFields[39],
                inFields[40], inFields[41],  inFields[42],  inFields[43],
                inFields[44], inFields[45],  inFields[46],  inFields[47],
                inFields[48], inFields[49],  inFields[50],  inFields[51],
                inFields[52], inFields[53],  inFields[54],  inFields[55],
                inFields[56], inFields[57],  inFields[58],  inFields[59],
                inFields[60], inFields[61]);

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "%d%f", 1);
       /* break;*/

        ruleIndex++;
        return CMD_OK;
}

/*******************************************************************************/
static CMD_STATUS wrCpssDxChTtiRuleActionHwGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /*ttiActionGetIndex = 0;*/
    ruleIndex=0;

    /* check if the application request 0 entries */
    if (inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ruleIndex       = (GT_U32)inArgs[1];
    ruleIndexMaxGet = ruleIndex + (GT_U32)inArgs[2] - 1;

    return wrCpssDxChTtiRuleActionHwGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_2GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_2GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ruleIndex = 0;

    /* first to get is mask */
    isMask = GT_TRUE;

    /* check if the application request 0 entries */
    if (inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ruleIndex       = (GT_U32)inArgs[1];
    ruleIndexMaxGet = ruleIndex + (GT_U32)inArgs[2] - 1;

    return wrCpssDxChTtiRule_2Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_4_KEY_UDB_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_4_KEY_UDB_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(outArgs);

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr        = &maskData;
        mask_ruleIndex = (GT_U32)inFields[0];
    }
    else /* pattern */
    {
        rulePtr           = &patternData;
        pattern_ruleIndex = (GT_U32)inFields[0];
    }

    switch (inFields[2]) /* rule size */
    {
    case CPSS_DXCH_TTI_KEY_SIZE_10_B_E:
        ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;

        rulePtr->udbArray.udb[0] = (GT_U8)inFields[3];
        rulePtr->udbArray.udb[1] = (GT_U8)inFields[4];
        rulePtr->udbArray.udb[2] = (GT_U8)inFields[5];
        rulePtr->udbArray.udb[3] = (GT_U8)inFields[6];
        rulePtr->udbArray.udb[4] = (GT_U8)inFields[7];
        rulePtr->udbArray.udb[5] = (GT_U8)inFields[8];
        rulePtr->udbArray.udb[6] = (GT_U8)inFields[9];
        rulePtr->udbArray.udb[7] = (GT_U8)inFields[10];
        rulePtr->udbArray.udb[8] = (GT_U8)inFields[11];
        rulePtr->udbArray.udb[9] = (GT_U8)inFields[12];
        break;

    case CPSS_DXCH_TTI_KEY_SIZE_20_B_E:
        ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;

        rulePtr->udbArray.udb[0] = (GT_U8)inFields[3];
        rulePtr->udbArray.udb[1] = (GT_U8)inFields[4];
        rulePtr->udbArray.udb[2] = (GT_U8)inFields[5];
        rulePtr->udbArray.udb[3] = (GT_U8)inFields[6];
        rulePtr->udbArray.udb[4] = (GT_U8)inFields[7];
        rulePtr->udbArray.udb[5] = (GT_U8)inFields[8];
        rulePtr->udbArray.udb[6] = (GT_U8)inFields[9];
        rulePtr->udbArray.udb[7] = (GT_U8)inFields[10];
        rulePtr->udbArray.udb[8] = (GT_U8)inFields[11];
        rulePtr->udbArray.udb[9] = (GT_U8)inFields[12];

        rulePtr->udbArray.udb[10] = (GT_U8)inFields[13];
        rulePtr->udbArray.udb[11] = (GT_U8)inFields[14];
        rulePtr->udbArray.udb[12] = (GT_U8)inFields[15];
        rulePtr->udbArray.udb[13] = (GT_U8)inFields[16];
        rulePtr->udbArray.udb[14] = (GT_U8)inFields[17];
        rulePtr->udbArray.udb[15] = (GT_U8)inFields[18];
        rulePtr->udbArray.udb[16] = (GT_U8)inFields[19];
        rulePtr->udbArray.udb[17] = (GT_U8)inFields[20];
        rulePtr->udbArray.udb[18] = (GT_U8)inFields[21];
        rulePtr->udbArray.udb[19] = (GT_U8)inFields[22];
        break;

    case CPSS_DXCH_TTI_KEY_SIZE_30_B_E:
        ruleFormat = CPSS_DXCH_TTI_RULE_UDB_30_E;

        rulePtr->udbArray.udb[0] = (GT_U8)inFields[3];
        rulePtr->udbArray.udb[1] = (GT_U8)inFields[4];
        rulePtr->udbArray.udb[2] = (GT_U8)inFields[5];
        rulePtr->udbArray.udb[3] = (GT_U8)inFields[6];
        rulePtr->udbArray.udb[4] = (GT_U8)inFields[7];
        rulePtr->udbArray.udb[5] = (GT_U8)inFields[8];
        rulePtr->udbArray.udb[6] = (GT_U8)inFields[9];
        rulePtr->udbArray.udb[7] = (GT_U8)inFields[10];
        rulePtr->udbArray.udb[8] = (GT_U8)inFields[11];
        rulePtr->udbArray.udb[9] = (GT_U8)inFields[12];

        rulePtr->udbArray.udb[10] = (GT_U8)inFields[13];
        rulePtr->udbArray.udb[11] = (GT_U8)inFields[14];
        rulePtr->udbArray.udb[12] = (GT_U8)inFields[15];
        rulePtr->udbArray.udb[13] = (GT_U8)inFields[16];
        rulePtr->udbArray.udb[14] = (GT_U8)inFields[17];
        rulePtr->udbArray.udb[15] = (GT_U8)inFields[18];
        rulePtr->udbArray.udb[16] = (GT_U8)inFields[19];
        rulePtr->udbArray.udb[17] = (GT_U8)inFields[20];
        rulePtr->udbArray.udb[18] = (GT_U8)inFields[21];
        rulePtr->udbArray.udb[19] = (GT_U8)inFields[22];

        rulePtr->udbArray.udb[20] = (GT_U8)inFields[23];
        rulePtr->udbArray.udb[21] = (GT_U8)inFields[24];
        rulePtr->udbArray.udb[22] = (GT_U8)inFields[25];
        rulePtr->udbArray.udb[23] = (GT_U8)inFields[26];
        rulePtr->udbArray.udb[24] = (GT_U8)inFields[27];
        rulePtr->udbArray.udb[25] = (GT_U8)inFields[28];
        rulePtr->udbArray.udb[26] = (GT_U8)inFields[29];
        rulePtr->udbArray.udb[27] = (GT_U8)inFields[30];
        rulePtr->udbArray.udb[28] = (GT_U8)inFields[31];
        rulePtr->udbArray.udb[29] = (GT_U8)inFields[32];
        break;

    default:
        /* do nothing */
        break;
    }

    if (inFields[1] == 0) /* mask */
    {
        mask_set       = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_set       = GT_TRUE;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_4SetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_4SetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    switch (inArgs[3]) /* rule type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        wrCpssDxChTtiRule_4_KEY_UDB_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_4SetNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_4SetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;
    GT_U32                          ruleIndex;
    CPSS_DXCH_TTI_RULE_TYPE_ENT     keyType;
    GT_U32                          actionIndex;
    CPSS_DXCH_TTI_ACTION_STC        *actionPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Check Validity */
    if (inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 0) && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 1) && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inFields[0];

    switch (inArgs[3]) /* rule type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        wrCpssDxChTtiRule_4_KEY_UDB_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }


    /* Get the action */
    ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

    if ((mask_set && pattern_set) && (actionIndex != INVALID_RULE_INDEX))
    {
        actionPtr                    = &(ttiActionDatabase[actionIndex].actionEntry);
        keyType                      = ruleFormat;
        ttiRuleFormat[ruleIndex]     = keyType;
        
        /* call cpss api function - ruleIndex is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,ruleIndex,keyType,&patternData,&maskData,actionPtr,GT_TRUE);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_4_KEY_UDB_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_4_KEY_UDB_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2] = ttiRuleFormat[ruleIndex];
    ruleFormat = ttiRuleFormat[ruleIndex];

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_RULE_UDB_10_E:
        inFields[2] = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;

        inFields[3] = rulePtr->udbArray.udb[0];
        inFields[4] = rulePtr->udbArray.udb[1];
        inFields[5] = rulePtr->udbArray.udb[2];
        inFields[6] = rulePtr->udbArray.udb[3];
        inFields[7] = rulePtr->udbArray.udb[4];
        inFields[8] = rulePtr->udbArray.udb[5];
        inFields[9] = rulePtr->udbArray.udb[6];
        inFields[10] = rulePtr->udbArray.udb[7];
        inFields[11] = rulePtr->udbArray.udb[8];
        inFields[12] = rulePtr->udbArray.udb[9];

        inFields[13] = 0;
        inFields[14] = 0;
        inFields[15] = 0;
        inFields[16] = 0;
        inFields[17] = 0;
        inFields[18] = 0;
        inFields[19] = 0;
        inFields[20] = 0;
        inFields[21] = 0;
        inFields[22] = 0;

        inFields[23] = 0;
        inFields[24] = 0;
        inFields[25] = 0;
        inFields[26] = 0;
        inFields[27] = 0;
        inFields[28] = 0;
        inFields[29] = 0;
        inFields[30] = 0;
        inFields[31] = 0;
        inFields[32] = 0;

        break;
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
        inFields[2] = CPSS_DXCH_TTI_KEY_SIZE_20_B_E;

        inFields[3] = rulePtr->udbArray.udb[0];
        inFields[4] = rulePtr->udbArray.udb[1];
        inFields[5] = rulePtr->udbArray.udb[2];
        inFields[6] = rulePtr->udbArray.udb[3];
        inFields[7] = rulePtr->udbArray.udb[4];
        inFields[8] = rulePtr->udbArray.udb[5];
        inFields[9] = rulePtr->udbArray.udb[6];
        inFields[10] = rulePtr->udbArray.udb[7];
        inFields[11] = rulePtr->udbArray.udb[8];
        inFields[12] = rulePtr->udbArray.udb[9];

        inFields[13] = rulePtr->udbArray.udb[10];
        inFields[14] = rulePtr->udbArray.udb[11];
        inFields[15] = rulePtr->udbArray.udb[12];
        inFields[16] = rulePtr->udbArray.udb[13];
        inFields[17] = rulePtr->udbArray.udb[14];
        inFields[18] = rulePtr->udbArray.udb[15];
        inFields[19] = rulePtr->udbArray.udb[16];
        inFields[20] = rulePtr->udbArray.udb[17];
        inFields[21] = rulePtr->udbArray.udb[18];
        inFields[22] = rulePtr->udbArray.udb[19];

        inFields[23] = 0;
        inFields[24] = 0;
        inFields[25] = 0;
        inFields[26] = 0;
        inFields[27] = 0;
        inFields[28] = 0;
        inFields[29] = 0;
        inFields[30] = 0;
        inFields[31] = 0;
        inFields[32] = 0;

        break;



    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        inFields[2] = CPSS_DXCH_TTI_KEY_SIZE_30_B_E;

        inFields[3] = rulePtr->udbArray.udb[0];
        inFields[4] = rulePtr->udbArray.udb[1];
        inFields[5] = rulePtr->udbArray.udb[2];
        inFields[6] = rulePtr->udbArray.udb[3];
        inFields[7] = rulePtr->udbArray.udb[4];
        inFields[8] = rulePtr->udbArray.udb[5];
        inFields[9] = rulePtr->udbArray.udb[6];
        inFields[10] = rulePtr->udbArray.udb[7];
        inFields[11] = rulePtr->udbArray.udb[8];
        inFields[12] = rulePtr->udbArray.udb[9];

        inFields[13] = rulePtr->udbArray.udb[10];
        inFields[14] = rulePtr->udbArray.udb[11];
        inFields[15] = rulePtr->udbArray.udb[12];
        inFields[16] = rulePtr->udbArray.udb[13];
        inFields[17] = rulePtr->udbArray.udb[14];
        inFields[18] = rulePtr->udbArray.udb[15];
        inFields[19] = rulePtr->udbArray.udb[16];
        inFields[20] = rulePtr->udbArray.udb[17];
        inFields[21] = rulePtr->udbArray.udb[18];
        inFields[22] = rulePtr->udbArray.udb[19];

        inFields[23] = rulePtr->udbArray.udb[20];
        inFields[24] = rulePtr->udbArray.udb[21];
        inFields[25] = rulePtr->udbArray.udb[22];
        inFields[26] = rulePtr->udbArray.udb[23];
        inFields[27] = rulePtr->udbArray.udb[24];
        inFields[28] = rulePtr->udbArray.udb[25];
        inFields[29] = rulePtr->udbArray.udb[26];
        inFields[30] = rulePtr->udbArray.udb[27];
        inFields[31] = rulePtr->udbArray.udb[28];
        inFields[32] = rulePtr->udbArray.udb[29];

        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
            inFields[0], inFields[1], inFields[2], inFields[3],
            inFields[4], inFields[5], inFields[6], inFields[7],
            inFields[8], inFields[9], inFields[10],inFields[11],
            inFields[12],inFields[13],inFields[14],inFields[15],
            inFields[16],inFields[17],inFields[18],inFields[19],
            inFields[20],inFields[21],inFields[22],inFields[23],
            inFields[24],inFields[25],inFields[26],inFields[27],
            inFields[28],inFields[29],inFields[30],inFields[31],
            inFields[32]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", 4);
    return CMD_OK;
}
/**
* @internal wrCpssDxChTtiRule_4Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_4Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_BOOL                     ruleValid;
    GT_U32                      ruleIndexStep = 0;

    /* if isMask == GT_TRUE it is mask's turn (the first one to get), so there need to refresh the data
    else the data is updated and it is pattern's turn (was updated earlier)*/
    if (isMask)
    {
        /* check if requested refresh range is already refreshed */
        if ((ruleIndex >= 36864) || (ruleIndex > ruleIndexMaxGet))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        /* look for another valid rule within the range */
        for ((ruleValid = GT_FALSE) ; ((ruleValid == GT_FALSE) && (ruleIndex < 36864) && (ruleIndex <= ruleIndexMaxGet)) ; ruleIndex += ruleIndexStep)
        {
            /* check if the rule is valid - ruleIndex is an absolute index */
            result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum,ruleIndex,&ruleValid,GT_TRUE);
            if (result != GT_OK)
            {
                galtisOutput(outArgs, result, "%d", -1);
                return CMD_OK;
            }

            if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_10_E)
            {
                ruleIndexStep = 1;
            }
            else if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_20_E)
            {
                ruleIndexStep = 2;
            }
            else
            {
                ruleIndexStep = 3;
            }
        }

        /* check if no valid rule was found in the requested range */
        if ((ruleValid == GT_FALSE))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* valid rule was found in the requested range */
        ruleIndex -= ruleIndexStep;

        /* call cpss api function, note that it is assumed that rule action
           type is not modified after setting the rule.
           ruleIndex is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,ttiRuleFormat[ruleIndex],&patternData,&maskData,&action,GT_TRUE);
    }

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        wrCpssDxChTtiRule_4_KEY_UDB_Get(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (isMask)
    {
        if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_10_E)
        {
            ruleIndexStep = 1;
        }
        else if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_20_E)
        {
            ruleIndexStep = 2;
        }
        else
        {
            ruleIndexStep = 3;
        }

        ruleIndex += ruleIndexStep;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_4GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_4GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ruleIndex = 0;

    /* first to get is mask */
    isMask = GT_TRUE;

    /* check if the application request 0 entries */
    if (inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ruleIndex       = (GT_U32)inArgs[1];
    ruleIndexMaxGet = ruleIndex + (GT_U32)inArgs[2] - 1;

    return wrCpssDxChTtiRule_4Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_3_KEY_MIM_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_3_KEY_MIM_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr = &maskData;
    }
    else /* pattern */
    {
        rulePtr = &patternData;
    }

    /* MIM key in TtiRule_3 table is the same as in TtiRule_2 table except for fields:
       macToMe, passengerPacketOuterTagExists, passengerPacketOuterTagVid,
       passengerPacketOuterTagUp, passengerPacketOuterTagDei */
    wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
    rulePtr->mim.macToMe                        = (GT_BOOL)inFields[19];
    rulePtr->mim.passengerPacketOuterTagExists  = (GT_BOOL)inFields[20];
    rulePtr->mim.passengerPacketOuterTagVid     = (GT_U32)inFields[21];
    rulePtr->mim.passengerPacketOuterTagUp      = (GT_U32)inFields[22];
    rulePtr->mim.passengerPacketOuterTagDei     = (GT_U32)inFields[23];

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_3_KEY_MIM_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_3_KEY_MIM_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_MIM_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2] = rulePtr->mim.common.pclId;
    inFields[3] = rulePtr->mim.common.srcIsTrunk;
    inFields[4] = rulePtr->mim.common.srcPortTrunk;

    if (rulePtr->mim.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mim.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6] = rulePtr->mim.common.vid;
    inFields[7] = rulePtr->mim.common.isTagged;
    inFields[8] = rulePtr->mim.common.dsaSrcIsTrunk;
    inFields[9] = rulePtr->mim.common.dsaSrcPortTrunk;
    inFields[10] = rulePtr->mim.common.dsaSrcDevice;
    inFields[11] = rulePtr->mim.bUp;
    inFields[12] = rulePtr->mim.bDp;
    inFields[13] = rulePtr->mim.iSid;
    inFields[14] = rulePtr->mim.iUp;
    inFields[15] = rulePtr->mim.iDp;
    inFields[16] = rulePtr->mim.iRes1;
    inFields[17] = rulePtr->mim.iRes2;
    inFields[18] = rulePtr->mim.common.sourcePortGroupId;
    inFields[19] = rulePtr->mim.macToMe;
    inFields[20] = rulePtr->mim.passengerPacketOuterTagExists;
    inFields[21] = rulePtr->mim.passengerPacketOuterTagVid;
    inFields[22] = rulePtr->mim.passengerPacketOuterTagUp;
    inFields[23] = rulePtr->mim.passengerPacketOuterTagDei;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->eth.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11],inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18],
                inFields[19],inFields[20],inFields[21],inFields[22],
                inFields[23]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_5SetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_5SetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    switch (inArgs[3]) /* rule type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_3_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        wrCpssDxChTtiRule_4_KEY_UDB_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_5SetNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_5SetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;
    GT_U32                          ruleIndex;
    CPSS_DXCH_TTI_RULE_TYPE_ENT     keyType;
    GT_U32                          actionIndex;
    CPSS_DXCH_TTI_ACTION_STC        *actionPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Check Validity */
    if (inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 0) && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 1) && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inFields[0];

    switch (inArgs[3]) /* rule type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_3_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        wrCpssDxChTtiRule_4_KEY_UDB_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }


    /* Get the action */
    ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

    if ((mask_set && pattern_set) && (actionIndex != INVALID_RULE_INDEX))
    {
        actionPtr                    = &(ttiActionDatabase[actionIndex].actionEntry);
        keyType                      = ruleFormat;
        ttiRuleFormat[ruleIndex]     = keyType;
        
        /* call cpss api function - ruleIndex is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,ruleIndex,keyType,&patternData,&maskData,actionPtr,GT_TRUE);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_5Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_5Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_BOOL                     ruleValid;
    GT_U32                      ruleIndexStep = 0;

    /* if isMask == GT_TRUE it is mask's turn (the first one to get), so there need to refresh the data
    else the data is updated and it is pattern's turn (was updated earlier)*/
    if (isMask)
    {
        /* check if requested refresh range is already refreshed */
        if ((ruleIndex >= 36864) || (ruleIndex > ruleIndexMaxGet))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        /* look for another valid rule within the range */
        for ((ruleValid = GT_FALSE) ; ((ruleValid == GT_FALSE) && (ruleIndex < 36864) && (ruleIndex <= ruleIndexMaxGet)) ; ruleIndex += ruleIndexStep)
        {
            /* check if the rule is valid - ruleIndex is an absolute index */
            result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum,ruleIndex,&ruleValid,GT_TRUE);
            if (result != GT_OK)
            {
                ruleIndexStep = 1;
                ruleValid=GT_FALSE; /* so we will continue the loop */
            }
            else
            {
                if(ruleValid==GT_FALSE)
                {
                    ruleIndexStep = 1;
                }
                else
                {
                    if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_10_E)
                    {
                        ruleIndexStep = 1;
                    }
                    else if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_20_E)
                    {
                        ruleIndexStep = 2;
                    }
                    else
                    {
                        ruleIndexStep = 3;
                    }
                }
            }
        }

        /* check if no valid rule was found in the requested range */
        if ((ruleValid == GT_FALSE))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* valid rule was found in the requested range */
        ruleIndex -= ruleIndexStep;

        /* call cpss api function, note that it is assumed that rule action
           type is not modified after setting the rule.
           ruleIndex is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,ttiRuleFormat[ruleIndex],&patternData,&maskData,&action,GT_TRUE);
    }

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_2_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_3_KEY_MIM_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        wrCpssDxChTtiRule_4_KEY_UDB_Get(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (isMask)
    {
        if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_10_E)
        {
            ruleIndexStep = 1;
        }
        else if (ttiRuleFormat[ruleIndex] == CPSS_DXCH_TTI_RULE_UDB_20_E)
        {
            ruleIndexStep = 2;
        }
        else
        {
            ruleIndexStep = 3;
        }

        ruleIndex += ruleIndexStep;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_5GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_5GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ruleIndex = 0;

    /* first to get is mask */
    isMask = GT_TRUE;

    /* check if the application request 0 entries */
    if (inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ruleIndex       = (GT_U32)inArgs[1];
    ruleIndexMaxGet = ruleIndex + (GT_U32)inArgs[2] - 1;

    return wrCpssDxChTtiRule_5Get(inArgs,inFields,numFields,outArgs);
}

/***********************************/
/* cpssDxChTtiRuleAction_2 Table */
/***********************************/

/**
* @internal wrCpssDxChTtiRuleAction_2Update function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_2Update
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result = GT_OK;
    GT_U8                               devNum;
    GT_U32                              ruleIndex;
    GT_U32                              index;
    CPSS_DXCH_TTI_ACTION_STC            *actionPtr;
    GT_BOOL                             isNewIndex = GT_FALSE;
    ttiActionDB                         ttiTempAction;

    GT_UNUSED_PARAM(numFields);
    cpssOsMemSet(&ttiTempAction, 0, sizeof(ttiTempAction));

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inFields[0];
    ruleIndex = (GT_U32)inFields[1];

    /* in case it is first usage of TTI action database, initialize it */
    if (firstRun)
    {
        ttiActionDatabaseInit();
        firstRun = GT_FALSE;
    }

    /* look for the action in the TTI action database */
    ttiActionDatabaseGet(devNum, ruleIndex, &index);

    /* the action wasn't in the TTI action database */
    if (index == INVALID_RULE_INDEX)
    {
        if (firstFree == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
        firstFree = ttiActionDatabase[firstFree].nextFree;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&ttiTempAction,&ttiActionDatabase[index],sizeof(ttiTempAction));
    }

    /* set action entry in TTI action database as valid */
    ttiActionDatabase[index].valid      = GT_TRUE;
    ttiActionDatabase[index].ruleIndex  = ruleIndex;
    ttiActionDatabase[index].devNum     = devNum;

    actionPtr = &(ttiActionDatabase[index].actionEntry);

    actionPtr->tunnelTerminate                 = (GT_BOOL)inFields[2];
    actionPtr->ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[3];
    actionPtr->tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[4];
    actionPtr->copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[5];
    actionPtr->mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[6];
    actionPtr->mplsTtl                         = (GT_U32)inFields[7];
    actionPtr->enableDecrementTtl              = (GT_BOOL)inFields[8];
    actionPtr->command                         = (CPSS_PACKET_CMD_ENT)inFields[9];
    actionPtr->redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[10];
    actionPtr->egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[11];
    actionPtr->egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[12];
    actionPtr->egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[13];
    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(actionPtr->egressInterface.devPort.hwDevNum, actionPtr->egressInterface.devPort.portNum);
    actionPtr->egressInterface.trunkId         = (GT_TRUNK_ID)inFields[14];
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->egressInterface.trunkId);
    actionPtr->egressInterface.vidx            = (GT_U16)inFields[15];
    actionPtr->egressInterface.vlanId          = (GT_U16)inFields[16];
    actionPtr->arpPtr                          = (GT_U32)inFields[17];
    actionPtr->tunnelStart                     = (GT_BOOL)inFields[18];
    actionPtr->tunnelStartPtr                  = (GT_U32)inFields[19];
    actionPtr->routerLttPtr                    = (GT_U32)inFields[20];
    actionPtr->vrfId                           = (GT_U32)inFields[21];
    actionPtr->sourceIdSetEnable               = (GT_BOOL)inFields[22];
    actionPtr->sourceId                        = (GT_U32)inFields[23];
    actionPtr->tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[24];
    actionPtr->tag0VlanId                      = (GT_U16)inFields[25];
    actionPtr->tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[26];
    actionPtr->tag1VlanId                      = (GT_U16)inFields[27];
    actionPtr->tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[28];
    actionPtr->nestedVlanEnable                = (GT_BOOL)inFields[29];
    actionPtr->bindToPolicerMeter              = (GT_BOOL)inFields[30];
    actionPtr->bindToPolicer                   = (GT_BOOL)inFields[31];
    actionPtr->policerIndex                    = (GT_U32)inFields[32];
    actionPtr->qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[33];
    actionPtr->keepPreviousQoS                 = (GT_BOOL)inFields[34];
    actionPtr->trustUp                         = (GT_BOOL)inFields[35];
    actionPtr->trustDscp                       = (GT_BOOL)inFields[36];
    actionPtr->trustExp                        = (GT_BOOL)inFields[37];
    actionPtr->qosProfile                      = (GT_U32)inFields[38];
    actionPtr->modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[39];
    actionPtr->tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[40];
    actionPtr->modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[41];
    actionPtr->tag0Up                          = (GT_U32)inFields[42];
    actionPtr->tag1Up                          = (GT_U32)inFields[43];
    actionPtr->remapDSCP                       = (GT_BOOL)inFields[44];
    actionPtr->pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[45];
    actionPtr->pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[46];
    actionPtr->pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[47];
    actionPtr->iPclConfigIndex                 = (GT_U32)inFields[48];
    actionPtr->mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[49];
    actionPtr->userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[50];
    actionPtr->bindToCentralCounter            = (GT_BOOL)inFields[51];
    actionPtr->centralCounterIndex             = (GT_U32)inFields[52];
    actionPtr->vntl2Echo                       = (GT_BOOL)inFields[53];
    actionPtr->bridgeBypass                    = (GT_BOOL)inFields[54];
    actionPtr->ingressPipeBypass               = (GT_BOOL)inFields[55];
    actionPtr->actionStop                      = (GT_BOOL)inFields[56];
    actionPtr->hashMaskIndex                   = (GT_U32)inFields[57];
    actionPtr->modifyMacSa                     = (GT_BOOL)inFields[58];
    actionPtr->modifyMacDa                     = (GT_BOOL)inFields[59];
    actionPtr->ResetSrcPortGroupId             = (GT_BOOL)inFields[60];
    actionPtr->multiPortGroupTtiEnable         = (GT_BOOL)inFields[61];
    
    /* call cpss api function */
    result = pg_wrap_cpssDxChTtiRuleActionUpdate(devNum,ruleIndex,actionPtr,GT_FALSE,isNewIndex);

    /* if the rule action didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            ttiActionDatabase[index].valid = GT_FALSE;
            ttiActionDatabase[index].nextFree = firstFree;
            firstFree = index;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&ttiActionDatabase[index],&ttiTempAction,sizeof(ttiTempAction));
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_2Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_2Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_ACTION_STC    *actionPtr;
    GT_HW_DEV_NUM               tempHwDev;    /* used for port,dev converting */
    GT_PORT_NUM                 tempPort;   /* used for port,dev converting */

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((ttiActionGetIndex < ACTION_TABLE_SIZE) &&
           (ttiActionDatabase[ttiActionGetIndex].valid == GT_FALSE))
        ttiActionGetIndex++;

    if (ttiActionGetIndex == ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    actionPtr = &(ttiActionDatabase[ttiActionGetIndex].actionEntry);

    inFields[0]  = ttiActionDatabase[ttiActionGetIndex].devNum;
    inFields[1]  = ttiActionDatabase[ttiActionGetIndex].ruleIndex;
    inFields[2]  = actionPtr->tunnelTerminate;
    inFields[3]  = actionPtr->ttPassengerPacketType;
    inFields[4]  = actionPtr->tsPassengerPacketType;
    inFields[5]  = actionPtr->copyTtlExpFromTunnelHeader;
    inFields[6]  = actionPtr->mplsCommand;
    inFields[7]  = actionPtr->mplsTtl;
    inFields[8]  = actionPtr->enableDecrementTtl;
    inFields[9]  = actionPtr->command;
    inFields[10] = actionPtr->redirectCommand;
    inFields[11] = actionPtr->egressInterface.type;
    tempHwDev      = actionPtr->egressInterface.devPort.hwDevNum;
    tempPort     = actionPtr->egressInterface.devPort.portNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
    inFields[12] = tempHwDev;
    inFields[13] = tempPort;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(actionPtr->egressInterface.trunkId);
    inFields[14] = actionPtr->egressInterface.trunkId;
    inFields[15] = actionPtr->egressInterface.vidx;
    inFields[16] = actionPtr->egressInterface.vlanId;
    inFields[17] = actionPtr->arpPtr;
    inFields[18] = actionPtr->tunnelStart;
    inFields[19] = actionPtr->tunnelStartPtr;
    inFields[20] = actionPtr->routerLttPtr;
    inFields[21] = actionPtr->vrfId;
    inFields[22] = actionPtr->sourceIdSetEnable;
    inFields[23] = actionPtr->sourceId;
    inFields[24] = actionPtr->tag0VlanCmd;
    inFields[25] = actionPtr->tag0VlanId;
    inFields[26] = actionPtr->tag1VlanCmd;
    inFields[27] = actionPtr->tag1VlanId;
    inFields[28] = actionPtr->tag0VlanPrecedence;
    inFields[29] = actionPtr->nestedVlanEnable;
    inFields[30] = actionPtr->bindToPolicerMeter;
    inFields[31] = actionPtr->bindToPolicer;
    inFields[32] = actionPtr->policerIndex;
    inFields[33] = actionPtr->qosPrecedence;
    inFields[34] = actionPtr->keepPreviousQoS;
    inFields[35] = actionPtr->trustUp;
    inFields[36] = actionPtr->trustDscp;
    inFields[37] = actionPtr->trustExp;
    inFields[38] = actionPtr->qosProfile;
    inFields[39] = actionPtr->modifyTag0Up;
    inFields[40] = actionPtr->tag1UpCommand;
    inFields[41] = actionPtr->modifyDscp;
    inFields[42] = actionPtr->tag0Up;
    inFields[43] = actionPtr->tag1Up;
    inFields[44] = actionPtr->remapDSCP;
    inFields[45] = actionPtr->pcl0OverrideConfigIndex;
    inFields[46] = actionPtr->pcl0_1OverrideConfigIndex;
    inFields[47] = actionPtr->pcl1OverrideConfigIndex;
    inFields[48] = actionPtr->iPclConfigIndex;
    inFields[49] = actionPtr->mirrorToIngressAnalyzerEnable;
    inFields[50] = actionPtr->userDefinedCpuCode;
    inFields[51] = actionPtr->bindToCentralCounter;
    inFields[52] = actionPtr->centralCounterIndex;
    inFields[53] = actionPtr->vntl2Echo;
    inFields[54] = actionPtr->bridgeBypass;
    inFields[55] = actionPtr->ingressPipeBypass;
    inFields[56] = actionPtr->actionStop;
    inFields[57] = actionPtr->hashMaskIndex;
    inFields[58] = actionPtr->modifyMacSa;
    inFields[59] = actionPtr->modifyMacDa;
    inFields[60] = actionPtr->ResetSrcPortGroupId;
    inFields[61] = actionPtr->multiPortGroupTtiEnable;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],   inFields[2],   inFields[3],
                inFields[4],  inFields[5],   inFields[6],   inFields[7],
                inFields[8],  inFields[9],   inFields[10],  inFields[11],
                inFields[12], inFields[13],  inFields[14],  inFields[15],
                inFields[16], inFields[17],  inFields[18],  inFields[19],
                inFields[20], inFields[21],  inFields[22],  inFields[23],
                inFields[24], inFields[25],  inFields[26],  inFields[27],
                inFields[28], inFields[29],  inFields[30],  inFields[31],
                inFields[32], inFields[33],  inFields[34],  inFields[35],
                inFields[36], inFields[37],  inFields[38],  inFields[39],
                inFields[40], inFields[41],  inFields[42],  inFields[43],
                inFields[44], inFields[45],  inFields[46],  inFields[47],
                inFields[48], inFields[49],  inFields[50],  inFields[51],
                inFields[52], inFields[53],  inFields[54],  inFields[55],
                inFields[56], inFields[57],  inFields[58],  inFields[59],
                inFields[60], inFields[61]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", 1);
        
    ttiActionGetIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_2GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_2GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ttiActionGetIndex = 0;

    return wrCpssDxChTtiRuleAction_2Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRuleAction_5Update function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_5Update
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result = GT_OK;
    GT_U8                               devNum;
    GT_U32                              ruleIndex;
    GT_U32                              index;
    CPSS_DXCH_TTI_ACTION_STC            *actionPtr;
    GT_BOOL                             isNewIndex = GT_FALSE;
    ttiActionDB                         ttiTempAction;

    GT_UNUSED_PARAM(numFields);
    cpssOsMemSet(&ttiTempAction, 0, sizeof(ttiTempAction));

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inFields[0];
    ruleIndex = (GT_U32)inFields[1];

    /* in case it is first usage of TTI action database, initialize it */
    if (firstRun)
    {
        ttiActionDatabaseInit();
        firstRun = GT_FALSE;
    }

    /* look for the action in the TTI action database */
    ttiActionDatabaseGet(devNum, ruleIndex, &index);

    /* the action wasn't in the TTI action database */
    if (index == INVALID_RULE_INDEX)
    {
        if (firstFree == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
        firstFree = ttiActionDatabase[firstFree].nextFree;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&ttiTempAction,&ttiActionDatabase[index],sizeof(ttiTempAction));
    }

    /* set action entry in TTI action database as valid */
    ttiActionDatabase[index].valid      = GT_TRUE;
    ttiActionDatabase[index].ruleIndex  = ruleIndex;
    ttiActionDatabase[index].devNum     = devNum;

    actionPtr = &(ttiActionDatabase[index].actionEntry);

    /* fill action fields */
    actionPtr->tunnelTerminate                 = (GT_BOOL)inFields[2];
    actionPtr->ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[3];
    actionPtr->tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[4];

    actionPtr->ttHeaderLength                  = (GT_U32)inFields[5];
    actionPtr->continueToNextTtiLookup         = (GT_BOOL)inFields[6];

    actionPtr->copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[7];
    actionPtr->mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[8];
    actionPtr->mplsTtl                         = (GT_U32)inFields[9];
    actionPtr->enableDecrementTtl              = (GT_BOOL)inFields[10];

    actionPtr->passengerParsingOfTransitMplsTunnelMode = (CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)inFields[11];
    actionPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inFields[12];

    actionPtr->command                         = (CPSS_PACKET_CMD_ENT)inFields[13];
    actionPtr->redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[14];
    actionPtr->egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[15];
    actionPtr->egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[16];
    actionPtr->egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[17];
    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(actionPtr->egressInterface.devPort.hwDevNum, actionPtr->egressInterface.devPort.portNum);
    actionPtr->egressInterface.trunkId         = (GT_TRUNK_ID)inFields[18];
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->egressInterface.trunkId);
    actionPtr->egressInterface.vidx            = (GT_U16)inFields[19];
    actionPtr->egressInterface.vlanId          = (GT_U16)inFields[20];
    actionPtr->arpPtr                          = (GT_U32)inFields[21];
    actionPtr->tunnelStart                     = (GT_BOOL)inFields[22];
    actionPtr->tunnelStartPtr                  = (GT_U32)inFields[23];
    actionPtr->routerLttPtr                    = (GT_U32)inFields[24];
    actionPtr->vrfId                           = (GT_U32)inFields[25];
    actionPtr->sourceIdSetEnable               = (GT_BOOL)inFields[26];
    actionPtr->sourceId                        = (GT_U32)inFields[27];
    actionPtr->tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[28];
    actionPtr->tag0VlanId                      = (GT_U16)inFields[29];
    actionPtr->tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[30];
    actionPtr->tag1VlanId                      = (GT_U16)inFields[31];
    actionPtr->tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[32];
    actionPtr->nestedVlanEnable                = (GT_BOOL)inFields[33];
    actionPtr->bindToPolicerMeter              = (GT_BOOL)inFields[34];
    actionPtr->bindToPolicer                   = (GT_BOOL)inFields[35];
    actionPtr->policerIndex                    = (GT_U32)inFields[36];
    actionPtr->qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[37];
    actionPtr->keepPreviousQoS                 = (GT_BOOL)inFields[38];
    actionPtr->trustUp                         = (GT_BOOL)inFields[39];
    actionPtr->trustDscp                       = (GT_BOOL)inFields[40];
    actionPtr->trustExp                        = (GT_BOOL)inFields[41];
    actionPtr->qosProfile                      = (GT_U32)inFields[42];
    actionPtr->modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[43];
    actionPtr->tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[44];
    actionPtr->modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[45];
    actionPtr->tag0Up                          = (GT_U32)inFields[46];
    actionPtr->tag1Up                          = (GT_U32)inFields[47];
    actionPtr->remapDSCP                       = (GT_BOOL)inFields[48];

    actionPtr->qosUseUpAsIndexEnable           = (GT_BOOL)inFields[49];
    actionPtr->qosMappingTableIndex            = (GT_U32)inFields[50];
    actionPtr->mplsLLspQoSProfileEnable        = (GT_BOOL)inFields[51];

    actionPtr->pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[52];
    actionPtr->pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[53];
    actionPtr->pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[54];
    actionPtr->iPclConfigIndex                 = (GT_U32)inFields[55];

    actionPtr->iPclUdbConfigTableEnable        = (GT_BOOL)inFields[56];
    actionPtr->iPclUdbConfigTableIndex         = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inFields[57];

    actionPtr->mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[58];
    actionPtr->mirrorToIngressAnalyzerIndex    = (GT_U32)inFields[59];

    actionPtr->userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[60];
    actionPtr->bindToCentralCounter            = (GT_BOOL)inFields[61];
    actionPtr->centralCounterIndex             = (GT_U32)inFields[62];
    actionPtr->vntl2Echo                       = (GT_BOOL)inFields[63];
    actionPtr->bridgeBypass                    = (GT_BOOL)inFields[64];
    actionPtr->ingressPipeBypass               = (GT_BOOL)inFields[65];
    actionPtr->actionStop                      = (GT_BOOL)inFields[66];
    actionPtr->hashMaskIndex                   = (GT_U32)inFields[67];
    actionPtr->modifyMacSa                     = (GT_BOOL)inFields[68];
    actionPtr->modifyMacDa                     = (GT_BOOL)inFields[69];
    actionPtr->ResetSrcPortGroupId             = (GT_BOOL)inFields[70];
    actionPtr->multiPortGroupTtiEnable         = (GT_BOOL)inFields[71];

    actionPtr->sourceEPortAssignmentEnable     = (GT_BOOL)inFields[72];
    actionPtr->sourceEPort                     = (GT_PORT_NUM)inFields[73];
    actionPtr->flowId                          = (GT_U32)inFields[74];
    actionPtr->setMacToMe                      = (GT_BOOL)inFields[75];
    actionPtr->rxProtectionSwitchEnable        = (GT_BOOL)inFields[76];
    actionPtr->rxIsProtectionPath              = (GT_BOOL)inFields[77];
    actionPtr->pwTagMode                       = (CPSS_DXCH_TTI_PW_TAG_MODE_ENT)inFields[78];

    actionPtr->oamTimeStampEnable              = (GT_BOOL)inFields[79];
    actionPtr->oamOffsetIndex                  = (GT_U32)inFields[80];
    actionPtr->oamProcessEnable                = (GT_BOOL)inFields[81];
    actionPtr->oamProfile                      = (GT_U32)inFields[82];
    actionPtr->oamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[83];

    actionPtr->isPtpPacket                     = (GT_BOOL)inFields[84];
    actionPtr->ptpTriggerType                  = (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)inFields[85];
    actionPtr->ptpOffset                       = (GT_U32)inFields[86];

    actionPtr->cwBasedPw                       = (GT_BOOL)inFields[87];
    actionPtr->ttlExpiryVccvEnable             = (GT_BOOL)inFields[88];
    actionPtr->pwe3FlowLabelExist              = (GT_BOOL)inFields[89];
    actionPtr->pwCwBasedETreeEnable            = (GT_BOOL)inFields[90];
    actionPtr->applyNonDataCwCommand           = (GT_BOOL)inFields[91];

    /* call cpss api function - ruleIndex is an absolute index */
    result = pg_wrap_cpssDxChTtiRuleActionUpdate(devNum,ruleIndex,actionPtr,GT_TRUE,isNewIndex);

    /* if the rule action didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            ttiActionDatabase[index].valid = GT_FALSE;
            ttiActionDatabase[index].nextFree = firstFree;
            firstFree = index;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&ttiActionDatabase[index],&ttiTempAction,sizeof(ttiTempAction));
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_5Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_5Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_ACTION_STC    *actionPtr;
    GT_HW_DEV_NUM               tempHwDev;    /* used for port,dev converting */
    GT_PORT_NUM                 tempPort;   /* used for port,dev converting */

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((ttiActionGetIndex < ACTION_TABLE_SIZE) &&
           (ttiActionDatabase[ttiActionGetIndex].valid == GT_FALSE))
        ttiActionGetIndex++;

    if (ttiActionGetIndex == ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    actionPtr = &(ttiActionDatabase[ttiActionGetIndex].actionEntry);

    inFields[0]  = ttiActionDatabase[ttiActionGetIndex].devNum;
    inFields[1]  = ttiActionDatabase[ttiActionGetIndex].ruleIndex;
    inFields[2]  = actionPtr->tunnelTerminate;
    inFields[3]  = actionPtr->ttPassengerPacketType;
    inFields[4]  = actionPtr->tsPassengerPacketType;
    inFields[5]  = actionPtr->ttHeaderLength;
    inFields[6]  = actionPtr->continueToNextTtiLookup;
    inFields[7]  = actionPtr->copyTtlExpFromTunnelHeader;
    inFields[8]  = actionPtr->mplsCommand;
    inFields[9]  = actionPtr->mplsTtl;
    inFields[10]  = actionPtr->enableDecrementTtl;
    inFields[11]  = actionPtr->passengerParsingOfTransitMplsTunnelMode;
    inFields[12]  = actionPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable;
    inFields[13]  = actionPtr->command;
    inFields[14] = actionPtr->redirectCommand;
    inFields[15] = actionPtr->egressInterface.type;
    tempHwDev      = actionPtr->egressInterface.devPort.hwDevNum;
    tempPort     = actionPtr->egressInterface.devPort.portNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
    inFields[16] = tempHwDev;
    inFields[17] = tempPort;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(actionPtr->egressInterface.trunkId);
    inFields[18] = actionPtr->egressInterface.trunkId;
    inFields[19] = actionPtr->egressInterface.vidx;
    inFields[20] = actionPtr->egressInterface.vlanId;
    inFields[21] = actionPtr->arpPtr;
    inFields[22] = actionPtr->tunnelStart;
    inFields[23] = actionPtr->tunnelStartPtr;
    inFields[24] = actionPtr->routerLttPtr;
    inFields[25] = actionPtr->vrfId;
    inFields[26] = actionPtr->sourceIdSetEnable;
    inFields[27] = actionPtr->sourceId;
    inFields[28] = actionPtr->tag0VlanCmd;
    inFields[29] = actionPtr->tag0VlanId;
    inFields[30] = actionPtr->tag1VlanCmd;
    inFields[31] = actionPtr->tag1VlanId;
    inFields[32] = actionPtr->tag0VlanPrecedence;
    inFields[33] = actionPtr->nestedVlanEnable;
    inFields[34] = actionPtr->bindToPolicerMeter;
    inFields[35] = actionPtr->bindToPolicer;
    inFields[36] = actionPtr->policerIndex;
    inFields[37] = actionPtr->qosPrecedence;
    inFields[38] = actionPtr->keepPreviousQoS;
    inFields[39] = actionPtr->trustUp;
    inFields[40] = actionPtr->trustDscp;
    inFields[41] = actionPtr->trustExp;
    inFields[42] = actionPtr->qosProfile;
    inFields[43] = actionPtr->modifyTag0Up;
    inFields[44] = actionPtr->tag1UpCommand;
    inFields[45] = actionPtr->modifyDscp;
    inFields[46] = actionPtr->tag0Up;
    inFields[47] = actionPtr->tag1Up;
    inFields[48] = actionPtr->remapDSCP;
    inFields[49] = actionPtr->qosUseUpAsIndexEnable;
    inFields[50] = actionPtr->qosMappingTableIndex;
    inFields[51] = actionPtr->mplsLLspQoSProfileEnable;
    inFields[52] = actionPtr->pcl0OverrideConfigIndex;
    inFields[53] = actionPtr->pcl0_1OverrideConfigIndex;
    inFields[54] = actionPtr->pcl1OverrideConfigIndex;
    inFields[55] = actionPtr->iPclConfigIndex;
    inFields[56] = actionPtr->iPclUdbConfigTableEnable;
    inFields[57] = actionPtr->iPclUdbConfigTableIndex;
    inFields[58] = actionPtr->mirrorToIngressAnalyzerEnable;
    inFields[59] = actionPtr->mirrorToIngressAnalyzerIndex;
    inFields[60] = actionPtr->userDefinedCpuCode;
    inFields[61] = actionPtr->bindToCentralCounter;
    inFields[62] = actionPtr->centralCounterIndex;
    inFields[63] = actionPtr->vntl2Echo;
    inFields[64] = actionPtr->bridgeBypass;
    inFields[65] = actionPtr->ingressPipeBypass;
    inFields[66] = actionPtr->actionStop;
    inFields[67] = actionPtr->hashMaskIndex;
    inFields[68] = actionPtr->modifyMacSa;
    inFields[69] = actionPtr->modifyMacDa;
    inFields[70] = actionPtr->ResetSrcPortGroupId;
    inFields[71] = actionPtr->multiPortGroupTtiEnable;
    inFields[72] = actionPtr->sourceEPortAssignmentEnable;
    inFields[73] = actionPtr->sourceEPort;
    inFields[74] = actionPtr->flowId;
    inFields[75] = actionPtr->setMacToMe;
    inFields[76] = actionPtr->rxProtectionSwitchEnable;
    inFields[77] = actionPtr->rxIsProtectionPath;
    inFields[78] = actionPtr->pwTagMode;
    inFields[79] = actionPtr->oamTimeStampEnable;
    inFields[80] = actionPtr->oamOffsetIndex;
    inFields[81] = actionPtr->oamProcessEnable;
    inFields[82] = actionPtr->oamProfile;
    inFields[83] = actionPtr->oamChannelTypeToOpcodeMappingEnable;
    inFields[84] = actionPtr->isPtpPacket;
    inFields[85] = actionPtr->ptpTriggerType;
    inFields[86] = actionPtr->ptpOffset;
    inFields[87] = actionPtr->cwBasedPw;
    inFields[88] = actionPtr->ttlExpiryVccvEnable;
    inFields[89] = actionPtr->pwe3FlowLabelExist;
    inFields[90] = actionPtr->pwCwBasedETreeEnable;
    inFields[91] = actionPtr->applyNonDataCwCommand;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],   inFields[2],   inFields[3],
                inFields[4],  inFields[5],   inFields[6],   inFields[7],
                inFields[8],  inFields[9],   inFields[10],  inFields[11],
                inFields[12], inFields[13],  inFields[14],  inFields[15],
                inFields[16], inFields[17],  inFields[18],  inFields[19],
                inFields[20], inFields[21],  inFields[22],  inFields[23],
                inFields[24], inFields[25],  inFields[26],  inFields[27],
                inFields[28], inFields[29],  inFields[30],  inFields[31],
                inFields[32], inFields[33],  inFields[34],  inFields[35],
                inFields[36], inFields[37],  inFields[38],  inFields[39],
                inFields[40], inFields[41],  inFields[42],  inFields[43],
                inFields[44], inFields[45],  inFields[46],  inFields[47],
                inFields[48], inFields[49],  inFields[50],  inFields[51],
                inFields[52], inFields[53],  inFields[54],  inFields[55],
                inFields[56], inFields[57],  inFields[58],  inFields[59],
                inFields[60], inFields[61],  inFields[62],  inFields[63],
                inFields[64], inFields[65],  inFields[66],  inFields[67],
                inFields[68], inFields[69],  inFields[70],  inFields[71],
                inFields[72], inFields[73],  inFields[74],  inFields[75],
                inFields[76], inFields[77],  inFields[78],  inFields[79],
                inFields[80], inFields[81],  inFields[82],  inFields[83],
                inFields[84], inFields[85],  inFields[86],  inFields[87],
                inFields[88], inFields[89],  inFields[90],  inFields[91]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", 1);
        
    ttiActionGetIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_5GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_5GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ttiActionGetIndex = 0;

    return wrCpssDxChTtiRuleAction_5Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRule_6_KEY_MPLS_Set function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_6_KEY_MPLS_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;

    /* map input arguments to locals */

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (inFields[1] == 0) /* mask */
    {
        rulePtr = &maskData;
    }
    else /* pattern */
    {
        rulePtr = &patternData;
    }

    /* MPLS key in TtiRule_6 table is the same as in TtiRule_2 table except for field cwFirstNibble */
    wrCpssDxChTtiRule_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
    rulePtr->mpls.common.sourcePortGroupId = (GT_U32)inFields[19];
    rulePtr->mpls.cwFirstNibble = (GT_U32)inFields[20];

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_6SetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_6SetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    switch (inArgs[3]) /* key type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_6_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_6SetNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_6SetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;
    GT_U32                          ruleIndex;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType;
    GT_U32                          actionIndex;
    CPSS_DXCH_TTI_ACTION_STC        *actionPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Check Validity */
    if (inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 0) && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if ((inFields[1] == 1) && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inFields[0];

    switch (inArgs[3]) /* key type tab */
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:
        wrCpssDxChTtiRule_6_KEY_MPLS_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Set(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_KEY_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Set(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }


    /* Get the action */
    ttiActionDatabaseGet(devNum,ruleIndex,&actionIndex);

    if ((mask_set && pattern_set) && (actionIndex != INVALID_RULE_INDEX))
    {
        actionPtr                    = &(ttiActionDatabase[actionIndex].actionEntry);
        keyType                      = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[3];
        ttiRuleFormat[ruleIndex]     = keyType;
        
        /* call cpss api function */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,ruleIndex,keyType,&patternData,&maskData,actionPtr,GT_FALSE);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_6_KEY_MPLS_Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_6_KEY_MPLS_Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    CPSS_DXCH_TTI_RULE_UNT  *rulePtr;
    GT_U8 devNum;
    GT_PORT_NUM port;

    GT_UNUSED_PARAM(numFields);

    ruleFormat = CPSS_DXCH_TTI_KEY_MPLS_E;

    /* inFields[1] indicate if mask or pattern (0: mask, 1: pattern) */
    if (isMask)
    {
        rulePtr     = &maskData;
        inFields[0] = ruleIndex;
        inFields[1] = 0;
        isMask      = GT_FALSE;

        /* convert CPSS trunk fields to test trunk fields */
        ttiRuleTrunkInfoFromCpssToTest_1();
    }
    else
    {
        rulePtr     = &patternData;
        inFields[0] = ruleIndex;
        inFields[1] = 1;
        isMask      = GT_TRUE;
    }

    inFields[2]  = rulePtr->mpls.common.pclId;
    inFields[3]  = rulePtr->mpls.common.srcIsTrunk;
    inFields[4]  = rulePtr->mpls.common.srcPortTrunk;

    if (rulePtr->mpls.common.srcIsTrunk == GT_FALSE)
    {
        devNum = (GT_U8)inArgs[0];
        port = (GT_PORT_NUM)rulePtr->mpls.common.srcPortTrunk;

        /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
        if (inFields[1] == 1) /* pattern */
            CONVERT_DEV_PORT_U32_MAC(devNum,port);

        inFields[4] = (GT_U32)port;
    }

    inFields[6]  = rulePtr->mpls.common.vid;
    inFields[7]  = rulePtr->mpls.common.isTagged;
    inFields[8]  = rulePtr->mpls.label0;
    inFields[9]  = rulePtr->mpls.exp0 ;
    inFields[10] = rulePtr->mpls.label1;
    inFields[11] = rulePtr->mpls.exp1;
    inFields[12] = rulePtr->mpls.label2;
    inFields[13] = rulePtr->mpls.exp2;
    inFields[14] = rulePtr->mpls.numOfLabels;
    inFields[15] = rulePtr->mpls.protocolAboveMPLS;
    inFields[16] = rulePtr->mpls.common.dsaSrcIsTrunk;
    inFields[17] = rulePtr->mpls.common.dsaSrcPortTrunk;
    inFields[18] = rulePtr->mpls.common.dsaSrcDevice;
    inFields[19] = rulePtr->mpls.common.sourcePortGroupId;
    inFields[20] = rulePtr->mpls.cwFirstNibble;

    fieldOutput("%d%d%d%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1],  inFields[2],  inFields[3],
                inFields[4], rulePtr->mpls.common.mac.arEther,  inFields[6],
                inFields[7], inFields[8], inFields[9],  inFields[10],
                inFields[11], inFields[12],inFields[13], inFields[14],
                inFields[15],inFields[16],inFields[17],inFields[18],inFields[19],
                inFields[20]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", ruleFormat);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_6Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_6Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_DXCH_TTI_ACTION_STC    action;
    GT_BOOL                     ruleValid;

    /* if isMask == GT_TRUE it is mask's turn (the first one to get), so there need to refresh the data
    else the data is updated and it is pattern's turn (was updated earlier)*/
    if (isMask)
    {
        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        /* look for another valid rule within the range */
        for ((ruleValid = GT_FALSE) ; ((ruleValid == GT_FALSE) && (ruleIndex < 3072) && (ruleIndex <= ruleIndexMaxGet)) ; ruleIndex++)
        {
            /* check if the rule is valid */
            result = pg_wrap_cpssDxChTtiRuleValidStatusGet(devNum,ruleIndex,&ruleValid,GT_FALSE);
            if (result != GT_OK)
            {
                galtisOutput(outArgs, result, "%d", -1);
                return CMD_OK;
            }
        }

        /* check if no valid rule was found in the requested range */
        if ((ruleValid == GT_FALSE))
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        /* valid rule was found in the requested range */
        ruleIndex--;

        /* call cpss api function, note that it is assumed that rule action
           type is not modified after setting the rule */
        result = pg_wrap_cpssDxChTtiRuleGet(devNum,ruleIndex,ttiRuleFormat[ruleIndex],&patternData,&maskData,&action,GT_FALSE);
    }

    switch (ttiRuleFormat[ruleIndex])
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        wrCpssDxChTtiRule_2_KEY_IPV4_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_MPLS_E:
        wrCpssDxChTtiRule_6_KEY_MPLS_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_ETH_E:
        wrCpssDxChTtiRule_2_KEY_ETH_Get(inArgs,inFields,numFields,outArgs);
        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:
        wrCpssDxChTtiRule_2_KEY_MIM_Get(inArgs,inFields,numFields,outArgs);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    if (isMask)
        ruleIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRule_6GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRule_6GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ruleIndex = 0;

    /* first to get is mask */
    isMask = GT_TRUE;

    /* check if the application request 0 entries */
    if (inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ruleIndex       = (GT_U32)inArgs[1];
    ruleIndexMaxGet = ruleIndex + (GT_U32)inArgs[2] - 1;

    return wrCpssDxChTtiRule_6Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiRuleAction_6Update function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_6Update
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result = GT_OK;
    GT_U8                               devNum;
    GT_U32                              ruleIndex;
    GT_U32                              index;
    CPSS_DXCH_TTI_ACTION_STC            *actionPtr;
    GT_BOOL                             isNewIndex = GT_FALSE;
    ttiActionDB                         ttiTempAction;

    GT_UNUSED_PARAM(numFields);
    cpssOsMemSet(&ttiTempAction, 0, sizeof(ttiTempAction));

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inFields[0];
    ruleIndex = (GT_U32)inFields[1];

    /* in case it is first usage of TTI action database, initialize it */
    if (firstRun)
    {
        ttiActionDatabaseInit();
        firstRun = GT_FALSE;
    }

    /* look for the action in the TTI action database */
    ttiActionDatabaseGet(devNum, ruleIndex, &index);

    /* the action wasn't in the TTI action database */
    if (index == INVALID_RULE_INDEX)
    {
        if (firstFree == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
        firstFree = ttiActionDatabase[firstFree].nextFree;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&ttiTempAction,&ttiActionDatabase[index],sizeof(ttiTempAction));
    }

    /* set action entry in TTI action database as valid */
    ttiActionDatabase[index].valid      = GT_TRUE;
    ttiActionDatabase[index].ruleIndex  = ruleIndex;
    ttiActionDatabase[index].devNum     = devNum;

    actionPtr = &(ttiActionDatabase[index].actionEntry);

    /* fill action fields */
    actionPtr->tunnelTerminate                 = (GT_BOOL)inFields[2];
    actionPtr->ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[3];
    actionPtr->tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[4];

    actionPtr->ttHeaderLength                  = (GT_U32)inFields[5];
    actionPtr->continueToNextTtiLookup         = (GT_BOOL)inFields[6];

    actionPtr->copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[7];
    actionPtr->mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[8];
    actionPtr->mplsTtl                         = (GT_U32)inFields[9];
    actionPtr->enableDecrementTtl              = (GT_BOOL)inFields[10];

    actionPtr->passengerParsingOfTransitMplsTunnelMode = (CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)inFields[11];
    actionPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inFields[12];

    actionPtr->command                         = (CPSS_PACKET_CMD_ENT)inFields[13];
    actionPtr->redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[14];
    actionPtr->egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[15];
    actionPtr->egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[16];
    actionPtr->egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[17];
    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(actionPtr->egressInterface.devPort.hwDevNum, actionPtr->egressInterface.devPort.portNum);
    actionPtr->egressInterface.trunkId         = (GT_TRUNK_ID)inFields[18];
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->egressInterface.trunkId);
    actionPtr->egressInterface.vidx            = (GT_U16)inFields[19];
    actionPtr->egressInterface.vlanId          = (GT_U16)inFields[20];
    actionPtr->arpPtr                          = (GT_U32)inFields[21];
    actionPtr->tunnelStart                     = (GT_BOOL)inFields[22];
    actionPtr->tunnelStartPtr                  = (GT_U32)inFields[23];
    actionPtr->routerLttPtr                    = (GT_U32)inFields[24];
    actionPtr->vrfId                           = (GT_U32)inFields[25];
    actionPtr->sourceIdSetEnable               = (GT_BOOL)inFields[26];
    actionPtr->sourceId                        = (GT_U32)inFields[27];
    actionPtr->tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[28];
    actionPtr->tag0VlanId                      = (GT_U16)inFields[29];
    actionPtr->tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[30];
    actionPtr->tag1VlanId                      = (GT_U16)inFields[31];
    actionPtr->tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[32];
    actionPtr->nestedVlanEnable                = (GT_BOOL)inFields[33];
    actionPtr->bindToPolicerMeter              = (GT_BOOL)inFields[34];
    actionPtr->bindToPolicer                   = (GT_BOOL)inFields[35];
    actionPtr->policerIndex                    = (GT_U32)inFields[36];
    actionPtr->qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[37];
    actionPtr->keepPreviousQoS                 = (GT_BOOL)inFields[38];
    actionPtr->trustUp                         = (GT_BOOL)inFields[39];
    actionPtr->trustDscp                       = (GT_BOOL)inFields[40];
    actionPtr->trustExp                        = (GT_BOOL)inFields[41];
    actionPtr->qosProfile                      = (GT_U32)inFields[42];
    actionPtr->modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[43];
    actionPtr->tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[44];
    actionPtr->modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[45];
    actionPtr->tag0Up                          = (GT_U32)inFields[46];
    actionPtr->tag1Up                          = (GT_U32)inFields[47];
    actionPtr->remapDSCP                       = (GT_BOOL)inFields[48];

    actionPtr->qosUseUpAsIndexEnable           = (GT_BOOL)inFields[49];
    actionPtr->qosMappingTableIndex            = (GT_U32)inFields[50];
    actionPtr->mplsLLspQoSProfileEnable        = (GT_BOOL)inFields[51];

    actionPtr->pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[52];
    actionPtr->pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[53];
    actionPtr->pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[54];
    actionPtr->iPclConfigIndex                 = (GT_U32)inFields[55];

    actionPtr->iPclUdbConfigTableEnable        = (GT_BOOL)inFields[56];
    actionPtr->iPclUdbConfigTableIndex         = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inFields[57];

    actionPtr->mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[58];
    actionPtr->mirrorToIngressAnalyzerIndex    = (GT_U32)inFields[59];

    actionPtr->userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[60];
    actionPtr->bindToCentralCounter            = (GT_BOOL)inFields[61];
    actionPtr->centralCounterIndex             = (GT_U32)inFields[62];
    actionPtr->vntl2Echo                       = (GT_BOOL)inFields[63];
    actionPtr->bridgeBypass                    = (GT_BOOL)inFields[64];
    actionPtr->ingressPipeBypass               = (GT_BOOL)inFields[65];
    actionPtr->actionStop                      = (GT_BOOL)inFields[66];
    actionPtr->hashMaskIndex                   = (GT_U32)inFields[67];
    actionPtr->modifyMacSa                     = (GT_BOOL)inFields[68];
    actionPtr->modifyMacDa                     = (GT_BOOL)inFields[69];
    actionPtr->ResetSrcPortGroupId             = (GT_BOOL)inFields[70];
    actionPtr->multiPortGroupTtiEnable         = (GT_BOOL)inFields[71];

    actionPtr->sourceEPortAssignmentEnable     = (GT_BOOL)inFields[72];
    actionPtr->sourceEPort                     = (GT_PORT_NUM)inFields[73];
    actionPtr->flowId                          = (GT_U32)inFields[74];
    actionPtr->setMacToMe                      = (GT_BOOL)inFields[75];
    actionPtr->rxProtectionSwitchEnable        = (GT_BOOL)inFields[76];
    actionPtr->rxIsProtectionPath              = (GT_BOOL)inFields[77];
    actionPtr->pwTagMode                       = (CPSS_DXCH_TTI_PW_TAG_MODE_ENT)inFields[78];

    actionPtr->oamTimeStampEnable              = (GT_BOOL)inFields[79];
    actionPtr->oamOffsetIndex                  = (GT_U32)inFields[80];
    actionPtr->oamProcessEnable                = (GT_BOOL)inFields[81];
    actionPtr->oamProfile                      = (GT_U32)inFields[82];
    actionPtr->oamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[83];

    actionPtr->isPtpPacket                     = (GT_BOOL)inFields[84];
    actionPtr->ptpTriggerType                  = (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)inFields[85];
    actionPtr->ptpOffset                       = (GT_U32)inFields[86];

    actionPtr->cwBasedPw                       = (GT_BOOL)inFields[87];
    actionPtr->ttlExpiryVccvEnable             = (GT_BOOL)inFields[88];
    actionPtr->pwe3FlowLabelExist              = (GT_BOOL)inFields[89];
    actionPtr->pwCwBasedETreeEnable            = (GT_BOOL)inFields[90];
    actionPtr->applyNonDataCwCommand           = (GT_BOOL)inFields[91];

    actionPtr->unknownSaCommandEnable          = (GT_BOOL)inFields[92];
    actionPtr->unknownSaCommand                = (GT_BOOL)inFields[93];
    actionPtr->sourceMeshIdSetEnable           = (GT_BOOL)inFields[94];
    actionPtr->sourceMeshId                    = (GT_BOOL)inFields[95];
    actionPtr->tunnelHeaderLengthAnchorType    = (CPSS_DXCH_TUNNEL_HEADER_LENGTH_ANCHOR_TYPE_ENT)inFields[96];
    actionPtr->skipFdbSaLookupEnable           = (GT_BOOL)inFields[97];
    actionPtr->ipv6SegmentRoutingEndNodeEnable = (GT_BOOL)inFields[98];
    actionPtr->exactMatchOverTtiEn             = (GT_BOOL)inFields[99];
        
    /* call cpss api function - ruleIndex is an absolute index */
    result = pg_wrap_cpssDxChTtiRuleActionUpdate(devNum,ruleIndex,actionPtr,GT_TRUE,isNewIndex);

    /* if the rule action didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            ttiActionDatabase[index].valid = GT_FALSE;
            ttiActionDatabase[index].nextFree = firstFree;
            firstFree = index;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&ttiActionDatabase[index],&ttiTempAction,sizeof(ttiTempAction));
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_6Get function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_6Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_TTI_ACTION_STC    *actionPtr;
    GT_HW_DEV_NUM               tempHwDev;    /* used for port,dev converting */
    GT_PORT_NUM                 tempPort;   /* used for port,dev converting */

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((ttiActionGetIndex < ACTION_TABLE_SIZE) &&
           (ttiActionDatabase[ttiActionGetIndex].valid == GT_FALSE))
        ttiActionGetIndex++;

    if (ttiActionGetIndex == ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    actionPtr = &(ttiActionDatabase[ttiActionGetIndex].actionEntry);

    inFields[0]  = ttiActionDatabase[ttiActionGetIndex].devNum;
    inFields[1]  = ttiActionDatabase[ttiActionGetIndex].ruleIndex;
    inFields[2]  = actionPtr->tunnelTerminate;
    inFields[3]  = actionPtr->ttPassengerPacketType;
    inFields[4]  = actionPtr->tsPassengerPacketType;
    inFields[5]  = actionPtr->ttHeaderLength;
    inFields[6]  = actionPtr->continueToNextTtiLookup;
    inFields[7]  = actionPtr->copyTtlExpFromTunnelHeader;
    inFields[8]  = actionPtr->mplsCommand;
    inFields[9]  = actionPtr->mplsTtl;
    inFields[10]  = actionPtr->enableDecrementTtl;
    inFields[11]  = actionPtr->passengerParsingOfTransitMplsTunnelMode;
    inFields[12]  = actionPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable;
    inFields[13]  = actionPtr->command;
    inFields[14] = actionPtr->redirectCommand;
    inFields[15] = actionPtr->egressInterface.type;
    tempHwDev      = actionPtr->egressInterface.devPort.hwDevNum;
    tempPort     = actionPtr->egressInterface.devPort.portNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
    inFields[16] = tempHwDev;
    inFields[17] = tempPort;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(actionPtr->egressInterface.trunkId);
    inFields[18] = actionPtr->egressInterface.trunkId;
    inFields[19] = actionPtr->egressInterface.vidx;
    inFields[20] = actionPtr->egressInterface.vlanId;
    inFields[21] = actionPtr->arpPtr;
    inFields[22] = actionPtr->tunnelStart;
    inFields[23] = actionPtr->tunnelStartPtr;
    inFields[24] = actionPtr->routerLttPtr;
    inFields[25] = actionPtr->vrfId;
    inFields[26] = actionPtr->sourceIdSetEnable;
    inFields[27] = actionPtr->sourceId;
    inFields[28] = actionPtr->tag0VlanCmd;
    inFields[29] = actionPtr->tag0VlanId;
    inFields[30] = actionPtr->tag1VlanCmd;
    inFields[31] = actionPtr->tag1VlanId;
    inFields[32] = actionPtr->tag0VlanPrecedence;
    inFields[33] = actionPtr->nestedVlanEnable;
    inFields[34] = actionPtr->bindToPolicerMeter;
    inFields[35] = actionPtr->bindToPolicer;
    inFields[36] = actionPtr->policerIndex;
    inFields[37] = actionPtr->qosPrecedence;
    inFields[38] = actionPtr->keepPreviousQoS;
    inFields[39] = actionPtr->trustUp;
    inFields[40] = actionPtr->trustDscp;
    inFields[41] = actionPtr->trustExp;
    inFields[42] = actionPtr->qosProfile;
    inFields[43] = actionPtr->modifyTag0Up;
    inFields[44] = actionPtr->tag1UpCommand;
    inFields[45] = actionPtr->modifyDscp;
    inFields[46] = actionPtr->tag0Up;
    inFields[47] = actionPtr->tag1Up;
    inFields[48] = actionPtr->remapDSCP;
    inFields[49] = actionPtr->qosUseUpAsIndexEnable;
    inFields[50] = actionPtr->qosMappingTableIndex;
    inFields[51] = actionPtr->mplsLLspQoSProfileEnable;
    inFields[52] = actionPtr->pcl0OverrideConfigIndex;
    inFields[53] = actionPtr->pcl0_1OverrideConfigIndex;
    inFields[54] = actionPtr->pcl1OverrideConfigIndex;
    inFields[55] = actionPtr->iPclConfigIndex;
    inFields[56] = actionPtr->iPclUdbConfigTableEnable;
    inFields[57] = actionPtr->iPclUdbConfigTableIndex;
    inFields[58] = actionPtr->mirrorToIngressAnalyzerEnable;
    inFields[59] = actionPtr->mirrorToIngressAnalyzerIndex;
    inFields[60] = actionPtr->userDefinedCpuCode;
    inFields[61] = actionPtr->bindToCentralCounter;
    inFields[62] = actionPtr->centralCounterIndex;
    inFields[63] = actionPtr->vntl2Echo;
    inFields[64] = actionPtr->bridgeBypass;
    inFields[65] = actionPtr->ingressPipeBypass;
    inFields[66] = actionPtr->actionStop;
    inFields[67] = actionPtr->hashMaskIndex;
    inFields[68] = actionPtr->modifyMacSa;
    inFields[69] = actionPtr->modifyMacDa;
    inFields[70] = actionPtr->ResetSrcPortGroupId;
    inFields[71] = actionPtr->multiPortGroupTtiEnable;
    inFields[72] = actionPtr->sourceEPortAssignmentEnable;
    inFields[73] = actionPtr->sourceEPort;
    inFields[74] = actionPtr->flowId;
    inFields[75] = actionPtr->setMacToMe;
    inFields[76] = actionPtr->rxProtectionSwitchEnable;
    inFields[77] = actionPtr->rxIsProtectionPath;
    inFields[78] = actionPtr->pwTagMode;
    inFields[79] = actionPtr->oamTimeStampEnable;
    inFields[80] = actionPtr->oamOffsetIndex;
    inFields[81] = actionPtr->oamProcessEnable;
    inFields[82] = actionPtr->oamProfile;
    inFields[83] = actionPtr->oamChannelTypeToOpcodeMappingEnable;
    inFields[84] = actionPtr->isPtpPacket;
    inFields[85] = actionPtr->ptpTriggerType;
    inFields[86] = actionPtr->ptpOffset;
    inFields[87] = actionPtr->cwBasedPw;
    inFields[88] = actionPtr->ttlExpiryVccvEnable;
    inFields[89] = actionPtr->pwe3FlowLabelExist;
    inFields[90] = actionPtr->pwCwBasedETreeEnable;
    inFields[91] = actionPtr->applyNonDataCwCommand;

    inFields[92] = actionPtr->unknownSaCommandEnable;
    inFields[93] = actionPtr->unknownSaCommand;
    inFields[94] = actionPtr->sourceMeshIdSetEnable;
    inFields[95] = actionPtr->sourceMeshId;
    inFields[96] = actionPtr->tunnelHeaderLengthAnchorType;
    inFields[97] = actionPtr->skipFdbSaLookupEnable;
    inFields[98] = actionPtr->ipv6SegmentRoutingEndNodeEnable;
    inFields[99] = actionPtr->exactMatchOverTtiEn;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],   inFields[2],   inFields[3],
                inFields[4],  inFields[5],   inFields[6],   inFields[7],
                inFields[8],  inFields[9],   inFields[10],  inFields[11],
                inFields[12], inFields[13],  inFields[14],  inFields[15],
                inFields[16], inFields[17],  inFields[18],  inFields[19],
                inFields[20], inFields[21],  inFields[22],  inFields[23],
                inFields[24], inFields[25],  inFields[26],  inFields[27],
                inFields[28], inFields[29],  inFields[30],  inFields[31],
                inFields[32], inFields[33],  inFields[34],  inFields[35],
                inFields[36], inFields[37],  inFields[38],  inFields[39],
                inFields[40], inFields[41],  inFields[42],  inFields[43],
                inFields[44], inFields[45],  inFields[46],  inFields[47],
                inFields[48], inFields[49],  inFields[50],  inFields[51],
                inFields[52], inFields[53],  inFields[54],  inFields[55],
                inFields[56], inFields[57],  inFields[58],  inFields[59],
                inFields[60], inFields[61],  inFields[62],  inFields[63],
                inFields[64], inFields[65],  inFields[66],  inFields[67],
                inFields[68], inFields[69],  inFields[70],  inFields[71],
                inFields[72], inFields[73],  inFields[74],  inFields[75],
                inFields[76], inFields[77],  inFields[78],  inFields[79],
                inFields[80], inFields[81],  inFields[82],  inFields[83],
                inFields[84], inFields[85],  inFields[86],  inFields[87],
                inFields[88], inFields[89],  inFields[90],  inFields[91],
                inFields[92], inFields[93],  inFields[94],  inFields[95],
                inFields[96], inFields[97],  inFields[98],  inFields[99]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%f", 1);
    
    ttiActionGetIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiRuleAction_6GetFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiRuleAction_6GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ttiActionGetIndex = 0;

    return wrCpssDxChTtiRuleAction_6Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function globally enables/disables bypassing IPv4 header length criteria checks as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function gets the globally bypassing IPv4 header length criteria check as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPwCwSequencingSupportEnableSet function
* @endinternal
*
* @brief   This function enables/disables the check for Pseudo Wire-Control Word
*         Data Word format <Sequence Number>==0 in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of CW as Data Word format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPwCwSequencingSupportEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiPwCwSequencingSupportEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPwCwSequencingSupportEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of the check
*         for Pseudo Wire-Control Word Data Word format <Sequence Number>==0
*         in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of CW as Data Word format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPwCwSequencingSupportEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChTtiUserDefinedByteSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType;
    GT_U32                               udbIndex;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    udbIndex = (GT_U32)inArgs[2];
    offsetType = (CPSS_DXCH_TTI_OFFSET_TYPE_ENT)inArgs[3];
    offset = (GT_U8)inArgs[4];

    /* call cpss api function */
    result = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChTtiUserDefinedByteGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    GT_U32                              udbIndex;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT       offsetType;
    GT_U8                               offset;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    udbIndex = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex, &offsetType, &offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", offsetType, offset);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPacketTypeKeySizeSet function
* @endinternal
*
* @brief   function sets key type size.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPacketTypeKeySizeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType;
    CPSS_DXCH_TTI_KEY_SIZE_ENT           size;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    size = (CPSS_DXCH_TTI_KEY_SIZE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, size);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPacketTypeKeySizeGet function
* @endinternal
*
* @brief   function gets key type size.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPacketTypeKeySizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_TTI_KEY_SIZE_ENT          size;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, &size);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", size);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiExceptionCpuCodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    CPSS_DXCH_TTI_EXCEPTION_ENT          exceptionType;
    CPSS_NET_RX_CPU_CODE_ENT             code;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_EXCEPTION_ENT)inArgs[1];
    code = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiExceptionCpuCodeSet(devNum, exceptionType, code);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiExceptionCpuCodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    CPSS_DXCH_TTI_EXCEPTION_ENT          exceptionType;
    CPSS_NET_RX_CPU_CODE_ENT             code;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_EXCEPTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiExceptionCpuCodeGet(devNum, exceptionType, &code);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", code);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiParallelLookupEnableSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiParallelLookupEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType;
    GT_BOOL                                 enable;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        segmentMode = (enable == GT_FALSE ?
                       CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E :
                       CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E);
    }
    else
    {
        segmentMode = (enable == GT_FALSE ?
                       CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E :
                       CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E);
    }

    /* call cpss api function */
    result = cpssDxChTtiTcamSegmentModeSet(devNum, keyType, segmentMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiParallelLookupEnableGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiParallelLookupEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType;
    GT_BOOL                                 enable;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiTcamSegmentModeGet(devNum, keyType, &segmentMode);
    enable = (segmentMode == CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E ? GT_FALSE : GT_TRUE);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiTcamSegmentModeSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiTcamSegmentModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    segmentMode = (CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiTcamSegmentModeSet(devNum, keyType, segmentMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiTcamSegmentModeGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiTcamSegmentModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiTcamSegmentModeGet(devNum, keyType, &segmentMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", segmentMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMcTunnelDuplicationModeSet function
* @endinternal
*
* @brief   Set the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMcTunnelDuplicationModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                        result;
    GT_U8                                            devNum;
    GT_PORT_NUM                                      portNum;
    CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol;
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    protocol = (CPSS_TUNNEL_MULTICAST_TYPE_ENT)inArgs[2];
    mode = (CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum, protocol, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMcTunnelDuplicationModeGet function
* @endinternal
*
* @brief   Get the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMcTunnelDuplicationModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                        result;
    GT_U8                                            devNum;
    GT_PORT_NUM                                      portNum;
    CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol;
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    protocol = (CPSS_TUNNEL_MULTICAST_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum, protocol, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMcTunnelDuplicationUdpDestPortSet function
* @endinternal
*
* @brief   Set the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
static CMD_STATUS wrCpssDxChTtiMcTunnelDuplicationUdpDestPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    udpPort;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    udpPort = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(devNum, udpPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMcTunnelDuplicationUdpDestPortGet function
* @endinternal
*
* @brief   Get the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
static CMD_STATUS wrCpssDxChTtiMcTunnelDuplicationUdpDestPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    udpPort;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(devNum, &udpPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", udpPort);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMplsMcTunnelTriggeringMacDaSet function
* @endinternal
*
* @brief   Set the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMplsMcTunnelTriggeringMacDaSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8            devNum;
    GT_ETHERADDR     address;
    GT_ETHERADDR     mask;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    galtisMacAddr(&address, (GT_U8*)inArgs[1]);
    galtisMacAddr(&mask, (GT_U8*)inArgs[2]);

    /* call cpss api function */
    result = cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(devNum, &address, &mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMplsMcTunnelTriggeringMacDaGet function
* @endinternal
*
* @brief   Get the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiMplsMcTunnelTriggeringMacDaGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8            devNum;
    GT_ETHERADDR     address;
    GT_ETHERADDR     mask;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(devNum, &address, &mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%6b%6b", address.arEther, mask.arEther);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPwCwExceptionCmdSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPwCwExceptionCmdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType;
    CPSS_PACKET_CMD_ENT                 command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT)inArgs[1];
    command = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiPwCwExceptionCmdSet(devNum, exceptionType, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPwCwExceptionCmdGet function
* @endinternal
*
* @brief   Get a PW CW exception command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPwCwExceptionCmdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType;
    CPSS_PACKET_CMD_ENT                 command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiPwCwExceptionCmdGet(devNum, exceptionType, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPwCwCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPwCwCpuCodeBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    cpuCodeBase;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cpuCodeBase = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiPwCwCpuCodeBaseSet(devNum, cpuCodeBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPwCwCpuCodeBaseGet function
* @endinternal
*
* @brief   Get the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiPwCwCpuCodeBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    cpuCodeBase;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiPwCwCpuCodeBaseGet(devNum, &cpuCodeBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCodeBase);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIPv6ExtensionHeaderSet function
* @endinternal
*
* @brief   Set one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*       To disable a configurable header, set its value to one of the above
*       values.
*
*/
static CMD_STATUS wrCpssDxChTtiIPv6ExtensionHeaderSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    extensionHeaderId;
    GT_U32    extensionHeaderValue;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    extensionHeaderId = (GT_U32)inArgs[1];
    extensionHeaderValue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiIPv6ExtensionHeaderSet(devNum, extensionHeaderId, extensionHeaderValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIPv6ExtensionHeaderGet function
* @endinternal
*
* @brief   Get one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*
*/
static CMD_STATUS wrCpssDxChTtiIPv6ExtensionHeaderGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    extensionHeaderId;
    GT_U32    extensionHeaderValue;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    extensionHeaderId = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiIPv6ExtensionHeaderGet(devNum, extensionHeaderId, &extensionHeaderValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", extensionHeaderValue);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiSourceIdBitsOverrideSet function
* @endinternal
*
* @brief   Set the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiSourceIdBitsOverrideSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    lookup;
    GT_U32    overrideBitmap;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lookup = (GT_U32)inArgs[1];
    overrideBitmap = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiSourceIdBitsOverrideSet(devNum, lookup, overrideBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiSourceIdBitsOverrideGet function
* @endinternal
*
* @brief   Get the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiSourceIdBitsOverrideGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    lookup;
    GT_U32    overrideBitmap;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lookup = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiSourceIdBitsOverrideGet(devNum, lookup, &overrideBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", overrideBitmap);

    return CMD_OK;
}

/**
* @internal wrcpssDxChTtiGreExtensionsCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable parsing of extensions (Checksum, Sequence, Key) on
*         IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChTtiGreExtensionsCheckEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiGreExtensionsCheckEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssDxChTtiGreExtensionsCheckEnableGet function
* @endinternal
*
* @brief   Gt the enabling status of the parsing of extensions (Checksum, Sequence,
*         Key) on IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChTtiGreExtensionsCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiGreExtensionsCheckEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiTcamBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for TTI rules
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       The converted rule index.
*/
static CMD_STATUS wrCpssDxChTtiTcamBaseIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    GT_U8     devNum;
    GT_U32    hitNum;
    GT_U32    ruleBaseIndexOffset;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hitNum = (GT_U32)inArgs[1];

    /* call appDemo api function */
    ruleBaseIndexOffset = appDemoDxChTcamTtiBaseIndexGet(devNum, hitNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d", ruleBaseIndexOffset);
#else
    (void)inArgs;
    (void)inFields;
    (void)numFields;
    (void)outArgs;
#endif
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiTcamNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       TCAM number of indexes for TTI rules.
*/
static CMD_STATUS wrCpssDxChTtiTcamNumOfIndexsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    GT_U8     devNum;
    GT_U32    hitNum;
    GT_U32    numOfIndexs;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hitNum = (GT_U32)inArgs[1];

    /* call appDemo api function */
    numOfIndexs = appDemoDxChTcamTtiNumOfIndexsGet(devNum, hitNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d", numOfIndexs);
#else
    (void)inArgs;
    (void)inFields;
    (void)numFields;
    (void)outArgs;
#endif

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortIpTotalLengthDeductionEnableSet function
* @endinternal
*
* @brief   For MACSEC packets over IPv4/6 tunnel, that are to be tunnel terminated,
*         this configuration enables aligning the IPv4/6 total header length to the
*         correct offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
static CMD_STATUS wrCpssDxChTtiPortIpTotalLengthDeductionEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiPortIpTotalLengthDeductionEnableGet function
* @endinternal
*
* @brief   Get if IPv4/6 total header length is aligned to the correct offset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
static CMD_STATUS wrCpssDxChTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIpTotalLengthDeductionValueSet function
* @endinternal
*
* @brief   Set Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
static CMD_STATUS wrCpssDxChTtiIpTotalLengthDeductionValueSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_IP_PROTOCOL_STACK_ENT ipType;
    GT_U32 value;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ipType = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    value = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiIpTotalLengthDeductionValueGet function
* @endinternal
*
* @brief   Get Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
static CMD_STATUS wrCpssDxChTtiIpTotalLengthDeductionValueGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_IP_PROTOCOL_STACK_ENT ipType;
    GT_U32 value;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ipType = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMultiRuleActionSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiRuleActionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result = GT_OK;
    CPSS_DXCH_TTI_RULE_TYPE_ENT keyType;
    ttiMultiActionDB    *ttiMultiActionDatabaseParams;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    keyType = (CPSS_DXCH_TTI_RULE_TYPE_ENT)inArgs[0];

    switch (keyType) /* rule type */
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        /* not supported yet */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;

    case CPSS_DXCH_TTI_RULE_MPLS_E:
        /* not supported yet */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;

    case CPSS_DXCH_TTI_RULE_ETH_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForEthRule;
        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:
        /* not supported yet */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForUdbRuleArray[0];
        break;

    case CPSS_DXCH_TTI_RULE_UDB_20_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForUdbRuleArray[1];
        break;

    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForUdbRuleArray[2];
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

   /* set action entry in TTI action database as valid */
    ttiMultiActionDatabaseParams->actionEntry.tunnelTerminate                 = (GT_BOOL)inArgs[1];
    ttiMultiActionDatabaseParams->actionEntry.ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inArgs[2];
    ttiMultiActionDatabaseParams->actionEntry.tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inArgs[3];

    ttiMultiActionDatabaseParams->actionEntry.ttHeaderLength                  = (GT_U32)inArgs[4];
    ttiMultiActionDatabaseParams->actionEntry.continueToNextTtiLookup         = (GT_BOOL)inArgs[5];

    ttiMultiActionDatabaseParams->actionEntry.copyTtlExpFromTunnelHeader      = (GT_BOOL)inArgs[6];
    ttiMultiActionDatabaseParams->actionEntry.mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inArgs[7];
    ttiMultiActionDatabaseParams->actionEntry.mplsTtl                         = (GT_U32)inArgs[8];
    ttiMultiActionDatabaseParams->actionEntry.enableDecrementTtl              = (GT_BOOL)inArgs[9];

    ttiMultiActionDatabaseParams->actionEntry.passengerParsingOfTransitMplsTunnelMode = (CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)inArgs[10];
    ttiMultiActionDatabaseParams->actionEntry.passengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inArgs[11];

    ttiMultiActionDatabaseParams->actionEntry.command                         = (CPSS_PACKET_CMD_ENT)inArgs[12];
    ttiMultiActionDatabaseParams->actionEntry.redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inArgs[13];
    ttiMultiActionDatabaseParams->actionEntry.egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inArgs[14];
    ttiMultiActionDatabaseParams->actionEntry.egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inArgs[15];
    ttiMultiActionDatabaseParams->actionEntry.egressInterface.devPort.portNum = (GT_PORT_NUM)inArgs[16];
    ttiMultiActionDatabaseParams->portNum_0  = (GT_PORT_NUM)inArgs[16];
    ttiMultiActionDatabaseParams->portNum_1  = (GT_PORT_NUM)inArgs[17];
    ttiMultiActionDatabaseParams->portNum_2  = (GT_PORT_NUM)inArgs[18];
    ttiMultiActionDatabaseParams->portNum_3  = (GT_PORT_NUM)inArgs[19];

    ttiMultiActionDatabaseParams->actionEntry.egressInterface.trunkId         = (GT_TRUNK_ID)inArgs[20];

    ttiMultiActionDatabaseParams->actionEntry.egressInterface.vidx            = (GT_U16)inArgs[21];
    ttiMultiActionDatabaseParams->actionEntry.egressInterface.vlanId          = (GT_U16)inArgs[22];
    ttiMultiActionDatabaseParams->actionEntry.arpPtr                          = (GT_U32)inArgs[23];
    ttiMultiActionDatabaseParams->actionEntry.tunnelStart                     = (GT_BOOL)inArgs[24];
    ttiMultiActionDatabaseParams->actionEntry.tunnelStartPtr                  = (GT_U32)inArgs[25];
    ttiMultiActionDatabaseParams->actionEntry.routerLttPtr                    = (GT_U32)inArgs[26];
    ttiMultiActionDatabaseParams->actionEntry.vrfId                           = (GT_U32)inArgs[27];
    ttiMultiActionDatabaseParams->actionEntry.sourceIdSetEnable               = (GT_BOOL)inArgs[28];
    ttiMultiActionDatabaseParams->actionEntry.sourceId                        = (GT_U32)inArgs[29];
    ttiMultiActionDatabaseParams->actionEntry.tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inArgs[30];
    ttiMultiActionDatabaseParams->actionEntry.tag0VlanId                      = (GT_U16)inArgs[31];
    ttiMultiActionDatabaseParams->actionEntry.tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inArgs[32];
    ttiMultiActionDatabaseParams->actionEntry.tag1VlanId                      = (GT_U16)inArgs[33];
    ttiMultiActionDatabaseParams->actionEntry.tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inArgs[34];
    ttiMultiActionDatabaseParams->actionEntry.nestedVlanEnable                = (GT_BOOL)inArgs[35];
    ttiMultiActionDatabaseParams->actionEntry.bindToPolicerMeter              = (GT_BOOL)inArgs[36];
    ttiMultiActionDatabaseParams->actionEntry.bindToPolicer                   = (GT_BOOL)inArgs[37];
    ttiMultiActionDatabaseParams->actionEntry.policerIndex                    = (GT_U32)inArgs[38];
    ttiMultiActionDatabaseParams->actionEntry.qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inArgs[39];
    ttiMultiActionDatabaseParams->actionEntry.keepPreviousQoS                 = (GT_BOOL)inArgs[40];
    ttiMultiActionDatabaseParams->actionEntry.trustUp                         = (GT_BOOL)inArgs[41];
    ttiMultiActionDatabaseParams->actionEntry.trustDscp                       = (GT_BOOL)inArgs[42];
    ttiMultiActionDatabaseParams->actionEntry.trustExp                        = (GT_BOOL)inArgs[43];
    ttiMultiActionDatabaseParams->actionEntry.qosProfile                      = (GT_U32)inArgs[44];
    ttiMultiActionDatabaseParams->actionEntry.modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inArgs[45];
    ttiMultiActionDatabaseParams->actionEntry.tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inArgs[46];
    ttiMultiActionDatabaseParams->actionEntry.modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inArgs[47];
    ttiMultiActionDatabaseParams->actionEntry.tag0Up                          = (GT_U32)inArgs[48];
    ttiMultiActionDatabaseParams->actionEntry.tag1Up                          = (GT_U32)inArgs[49];
    ttiMultiActionDatabaseParams->actionEntry.remapDSCP                       = (GT_BOOL)inArgs[50];

    ttiMultiActionDatabaseParams->actionEntry.qosUseUpAsIndexEnable           = (GT_BOOL)inArgs[51];
    ttiMultiActionDatabaseParams->actionEntry.qosMappingTableIndex            = (GT_U32)inArgs[52];
    ttiMultiActionDatabaseParams->actionEntry.mplsLLspQoSProfileEnable        = (GT_BOOL)inArgs[53];

    ttiMultiActionDatabaseParams->actionEntry.pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inArgs[54];
    ttiMultiActionDatabaseParams->actionEntry.pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inArgs[55];
    ttiMultiActionDatabaseParams->actionEntry.pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inArgs[56];
    ttiMultiActionDatabaseParams->actionEntry.iPclConfigIndex                 = (GT_U32)inArgs[57];

    ttiMultiActionDatabaseParams->actionEntry.iPclUdbConfigTableEnable        = (GT_BOOL)inArgs[58];
    ttiMultiActionDatabaseParams->actionEntry.iPclUdbConfigTableIndex         = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[59];

    ttiMultiActionDatabaseParams->actionEntry.mirrorToIngressAnalyzerEnable   = (GT_BOOL)inArgs[60];
    ttiMultiActionDatabaseParams->actionEntry.mirrorToIngressAnalyzerIndex    = (GT_U32)inArgs[61];

    ttiMultiActionDatabaseParams->actionEntry.userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[62];
    ttiMultiActionDatabaseParams->actionEntry.bindToCentralCounter            = (GT_BOOL)inArgs[63];
    ttiMultiActionDatabaseParams->actionEntry.centralCounterIndex             = (GT_U32)inArgs[64];
    ttiMultiActionDatabaseParams->actionEntry.vntl2Echo                       = (GT_BOOL)inArgs[65];
    ttiMultiActionDatabaseParams->actionEntry.bridgeBypass                    = (GT_BOOL)inArgs[66];
    ttiMultiActionDatabaseParams->actionEntry.ingressPipeBypass               = (GT_BOOL)inArgs[67];
    ttiMultiActionDatabaseParams->actionEntry.actionStop                      = (GT_BOOL)inArgs[68];
    ttiMultiActionDatabaseParams->actionEntry.hashMaskIndex                   = (GT_U32)inArgs[69];
    ttiMultiActionDatabaseParams->actionEntry.modifyMacSa                     = (GT_BOOL)inArgs[70];
    ttiMultiActionDatabaseParams->actionEntry.modifyMacDa                     = (GT_BOOL)inArgs[71];
    ttiMultiActionDatabaseParams->actionEntry.ResetSrcPortGroupId             = (GT_BOOL)inArgs[72];
    ttiMultiActionDatabaseParams->actionEntry.multiPortGroupTtiEnable         = (GT_BOOL)inArgs[73];

    ttiMultiActionDatabaseParams->actionEntry.sourceEPortAssignmentEnable     = (GT_BOOL)inArgs[74];
    ttiMultiActionDatabaseParams->actionEntry.sourceEPort                     = (GT_PORT_NUM)inArgs[75];
    ttiMultiActionDatabaseParams->actionEntry.flowId                          = (GT_U32)inArgs[76];
    ttiMultiActionDatabaseParams->actionEntry.setMacToMe                      = (GT_BOOL)inArgs[77];
    ttiMultiActionDatabaseParams->actionEntry.rxProtectionSwitchEnable        = (GT_BOOL)inArgs[78];
    ttiMultiActionDatabaseParams->actionEntry.rxIsProtectionPath              = (GT_BOOL)inArgs[79];
    ttiMultiActionDatabaseParams->actionEntry.pwTagMode                       = (CPSS_DXCH_TTI_PW_TAG_MODE_ENT)inArgs[80];

    ttiMultiActionDatabaseParams->actionEntry.oamTimeStampEnable              = (GT_BOOL)inArgs[81];
    ttiMultiActionDatabaseParams->actionEntry.oamOffsetIndex                  = (GT_U32)inArgs[82];
    ttiMultiActionDatabaseParams->actionEntry.oamProcessEnable                = (GT_BOOL)inArgs[83];
    ttiMultiActionDatabaseParams->actionEntry.oamProfile                      = (GT_U32)inArgs[84];
    ttiMultiActionDatabaseParams->actionEntry.oamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inArgs[85];

    ttiMultiActionDatabaseParams->actionEntry.isPtpPacket                     = (GT_BOOL)inArgs[86];
    ttiMultiActionDatabaseParams->actionEntry.ptpTriggerType                  = (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)inArgs[87];
    ttiMultiActionDatabaseParams->actionEntry.ptpOffset                       = (GT_U32)inArgs[88];

    ttiMultiActionDatabaseParams->actionEntry.cwBasedPw                       = (GT_BOOL)inArgs[89];
    ttiMultiActionDatabaseParams->actionEntry.ttlExpiryVccvEnable             = (GT_BOOL)inArgs[90];
    ttiMultiActionDatabaseParams->actionEntry.pwe3FlowLabelExist              = (GT_BOOL)inArgs[91];
    ttiMultiActionDatabaseParams->actionEntry.pwCwBasedETreeEnable            = (GT_BOOL)inArgs[92];
    ttiMultiActionDatabaseParams->actionEntry.applyNonDataCwCommand           = (GT_BOOL)inArgs[93];

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMultiRuleActionGet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiRuleActionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result = GT_OK;
    CPSS_DXCH_TTI_RULE_TYPE_ENT         keyType;
    ttiMultiActionDB                    *ttiMultiActionDatabaseParams;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    keyType = (CPSS_DXCH_TTI_RULE_TYPE_ENT)inArgs[0];

    switch (keyType) /* rule type */
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        /* not supported yet */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;

    case CPSS_DXCH_TTI_RULE_MPLS_E:
        /* not supported yet */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;

    case CPSS_DXCH_TTI_RULE_ETH_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForEthRule;
        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:
        /* not supported yet */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForUdbRuleArray[0];
        break;
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForUdbRuleArray[1];
        break;
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        ttiMultiActionDatabaseParams = &ttiMultiActionDatabaseParamsForUdbRuleArray[2];
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong TTI RULE FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d",
                 ttiMultiActionDatabaseParams->actionEntry.tunnelTerminate,
                 ttiMultiActionDatabaseParams->actionEntry.ttPassengerPacketType,
                 ttiMultiActionDatabaseParams->actionEntry.tsPassengerPacketType,
                 ttiMultiActionDatabaseParams->actionEntry.ttHeaderLength,
                 ttiMultiActionDatabaseParams->actionEntry.continueToNextTtiLookup,
                 ttiMultiActionDatabaseParams->actionEntry.copyTtlExpFromTunnelHeader,
                 ttiMultiActionDatabaseParams->actionEntry.mplsCommand,
                 ttiMultiActionDatabaseParams->actionEntry.mplsTtl,
                 ttiMultiActionDatabaseParams->actionEntry.enableDecrementTtl,
                 ttiMultiActionDatabaseParams->actionEntry.passengerParsingOfTransitMplsTunnelMode,
                 ttiMultiActionDatabaseParams->actionEntry.passengerParsingOfTransitNonMplsTransitTunnelEnable,
                 ttiMultiActionDatabaseParams->actionEntry.command,
                 ttiMultiActionDatabaseParams->actionEntry.redirectCommand,
                 ttiMultiActionDatabaseParams->actionEntry.egressInterface.type,
                 ttiMultiActionDatabaseParams->actionEntry.egressInterface.devPort.hwDevNum,
                 ttiMultiActionDatabaseParams->portNum_0,
                 ttiMultiActionDatabaseParams->portNum_1,
                 ttiMultiActionDatabaseParams->portNum_2,
                 ttiMultiActionDatabaseParams->portNum_3,
                 ttiMultiActionDatabaseParams->actionEntry.egressInterface.trunkId,
                 ttiMultiActionDatabaseParams->actionEntry.egressInterface.vidx,
                 ttiMultiActionDatabaseParams->actionEntry.egressInterface.vlanId,
                 ttiMultiActionDatabaseParams->actionEntry.arpPtr,
                 ttiMultiActionDatabaseParams->actionEntry.tunnelStart,
                 ttiMultiActionDatabaseParams->actionEntry.tunnelStartPtr,
                 ttiMultiActionDatabaseParams->actionEntry.routerLttPtr,
                 ttiMultiActionDatabaseParams->actionEntry.vrfId,
                 ttiMultiActionDatabaseParams->actionEntry.sourceIdSetEnable,
                 ttiMultiActionDatabaseParams->actionEntry.sourceId,
                 ttiMultiActionDatabaseParams->actionEntry.tag0VlanCmd,
                 ttiMultiActionDatabaseParams->actionEntry.tag0VlanId,
                 ttiMultiActionDatabaseParams->actionEntry.tag1VlanCmd,
                 ttiMultiActionDatabaseParams->actionEntry.tag1VlanId,
                 ttiMultiActionDatabaseParams->actionEntry.tag0VlanPrecedence,
                 ttiMultiActionDatabaseParams->actionEntry.nestedVlanEnable,
                 ttiMultiActionDatabaseParams->actionEntry.bindToPolicerMeter,
                 ttiMultiActionDatabaseParams->actionEntry.bindToPolicer,
                 ttiMultiActionDatabaseParams->actionEntry.policerIndex,
                 ttiMultiActionDatabaseParams->actionEntry.qosPrecedence,
                 ttiMultiActionDatabaseParams->actionEntry.keepPreviousQoS,
                 ttiMultiActionDatabaseParams->actionEntry.trustUp,
                 ttiMultiActionDatabaseParams->actionEntry.trustDscp,
                 ttiMultiActionDatabaseParams->actionEntry.trustExp,
                 ttiMultiActionDatabaseParams->actionEntry.qosProfile,
                 ttiMultiActionDatabaseParams->actionEntry.modifyTag0Up,
                 ttiMultiActionDatabaseParams->actionEntry.tag1UpCommand,
                 ttiMultiActionDatabaseParams->actionEntry.modifyDscp,
                 ttiMultiActionDatabaseParams->actionEntry.tag0Up,
                 ttiMultiActionDatabaseParams->actionEntry.tag1Up,
                 ttiMultiActionDatabaseParams->actionEntry.remapDSCP,
                 ttiMultiActionDatabaseParams->actionEntry.qosUseUpAsIndexEnable,
                 ttiMultiActionDatabaseParams->actionEntry.qosMappingTableIndex,
                 ttiMultiActionDatabaseParams->actionEntry.mplsLLspQoSProfileEnable,
                 ttiMultiActionDatabaseParams->actionEntry.pcl0OverrideConfigIndex,
                 ttiMultiActionDatabaseParams->actionEntry.pcl0_1OverrideConfigIndex,
                 ttiMultiActionDatabaseParams->actionEntry.pcl1OverrideConfigIndex,
                 ttiMultiActionDatabaseParams->actionEntry.iPclConfigIndex,
                 ttiMultiActionDatabaseParams->actionEntry.iPclUdbConfigTableEnable,
                 ttiMultiActionDatabaseParams->actionEntry.iPclUdbConfigTableIndex,
                 ttiMultiActionDatabaseParams->actionEntry.mirrorToIngressAnalyzerEnable,
                 ttiMultiActionDatabaseParams->actionEntry.mirrorToIngressAnalyzerIndex,
                 ttiMultiActionDatabaseParams->actionEntry.userDefinedCpuCode,
                 ttiMultiActionDatabaseParams->actionEntry.bindToCentralCounter,
                 ttiMultiActionDatabaseParams->actionEntry.centralCounterIndex,
                 ttiMultiActionDatabaseParams->actionEntry.vntl2Echo,
                 ttiMultiActionDatabaseParams->actionEntry.bridgeBypass,
                 ttiMultiActionDatabaseParams->actionEntry.ingressPipeBypass,
                 ttiMultiActionDatabaseParams->actionEntry.actionStop,
                 ttiMultiActionDatabaseParams->actionEntry.hashMaskIndex,
                 ttiMultiActionDatabaseParams->actionEntry.modifyMacSa,
                 ttiMultiActionDatabaseParams->actionEntry.modifyMacDa,
                 ttiMultiActionDatabaseParams->actionEntry.ResetSrcPortGroupId,
                 ttiMultiActionDatabaseParams->actionEntry.multiPortGroupTtiEnable,
                 ttiMultiActionDatabaseParams->actionEntry.sourceEPortAssignmentEnable,
                 ttiMultiActionDatabaseParams->actionEntry.sourceEPort,
                 ttiMultiActionDatabaseParams->actionEntry.flowId,
                 ttiMultiActionDatabaseParams->actionEntry.setMacToMe,
                 ttiMultiActionDatabaseParams->actionEntry.rxProtectionSwitchEnable,
                 ttiMultiActionDatabaseParams->actionEntry.rxIsProtectionPath,
                 ttiMultiActionDatabaseParams->actionEntry.pwTagMode,
                 ttiMultiActionDatabaseParams->actionEntry.oamTimeStampEnable,
                 ttiMultiActionDatabaseParams->actionEntry.oamOffsetIndex,
                 ttiMultiActionDatabaseParams->actionEntry.oamProcessEnable,
                 ttiMultiActionDatabaseParams->actionEntry.oamProfile,
                 ttiMultiActionDatabaseParams->actionEntry.oamChannelTypeToOpcodeMappingEnable,
                 ttiMultiActionDatabaseParams->actionEntry.isPtpPacket,
                 ttiMultiActionDatabaseParams->actionEntry.ptpTriggerType,
                 ttiMultiActionDatabaseParams->actionEntry.ptpOffset,
                 ttiMultiActionDatabaseParams->actionEntry.cwBasedPw,
                 ttiMultiActionDatabaseParams->actionEntry.ttlExpiryVccvEnable,
                 ttiMultiActionDatabaseParams->actionEntry.pwe3FlowLabelExist,
                 ttiMultiActionDatabaseParams->actionEntry.pwCwBasedETreeEnable,
                 ttiMultiActionDatabaseParams->actionEntry.applyNonDataCwCommand);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMultiEthRuleSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiEthRuleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      numOfRulesAdded=0;
    GT_U32      ruleIndex;
    GT_PORT_NUM port;
    GT_U32      i = 0;

    CPSS_DXCH_TTI_ACTION_STC        actionEntry;
    GT_PORT_NUM                     portNum_0;      /* port interface to use when (ruleIndex % 4 == 0)                                      */
    GT_PORT_NUM                     portNum_1;      /* port interface to use when (ruleIndex % 4 == 1)                                      */
    GT_PORT_NUM                     portNum_2;      /* port interface to use when (ruleIndex % 4 == 2)                                      */
    GT_PORT_NUM                     portNum_3;      /* port interface to use when (ruleIndex % 4 == 3)                                      */
    GT_HW_DEV_NUM                   hwDevNum;

    GT_U32      startSec  = 0;
    GT_U32      startNsec = 0;
    GT_U32      endSec  = 0;
    GT_U32      endNsec = 0;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemCpy(&actionEntry,&(ttiMultiActionDatabaseParamsForEthRule.actionEntry),sizeof(CPSS_DXCH_TTI_ACTION_STC));

    hwDevNum  = ttiMultiActionDatabaseParamsForEthRule.actionEntry.egressInterface.devPort.hwDevNum;
    portNum_0 = ttiMultiActionDatabaseParamsForEthRule.portNum_0;
    portNum_1 = ttiMultiActionDatabaseParamsForEthRule.portNum_1;
    portNum_2 = ttiMultiActionDatabaseParamsForEthRule.portNum_2;
    portNum_3 = ttiMultiActionDatabaseParamsForEthRule.portNum_3;

    devNum = ttiMultiActionDatabaseParamsForEthRule.devNum  = (GT_U8)inArgs[0];
    ttiMultiActionDatabaseParamsForEthRule.numOfRulesToAdd  = (GT_U32)inArgs[1];
    ttiMultiActionDatabaseParamsForEthRule.ruleIndex  = (GT_U32)inArgs[2];
    ttiMultiActionDatabaseParamsForEthRule.ruleIndexDelta  = (GT_U32)inArgs[3];

    ruleIndex=ttiMultiActionDatabaseParamsForEthRule.ruleIndex;/* set first ruleIndex for the loop */

    /* reset mask and pattern */
    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    patternData.eth.common.pclId           = (GT_U32)inArgs[4];
    maskData.eth.common.pclId        = (GT_U32)inArgs[5];
    patternData.eth.common.srcIsTrunk      = (GT_U32)inArgs[6];
    maskData.eth.common.srcIsTrunk   = (GT_U32)inArgs[7];
    patternData.eth.common.srcPortTrunk    = (GT_U32)inArgs[8];
    maskData.eth.common.srcPortTrunk = (GT_U32)inArgs[9];
    galtisMacAddr(&patternData.eth.common.mac,(GT_U8*)inArgs[10]);
    galtisMacAddr(&maskData.eth.common.mac,(GT_U8*)inArgs[11]);
    patternData.eth.common.vid             = (GT_U16)inArgs[12];
    maskData.eth.common.vid          = (GT_U16)inArgs[13];
    patternData.eth.common.isTagged        = (GT_BOOL)inArgs[14];
    maskData.eth.common.isTagged     = (GT_BOOL)inArgs[15];
    patternData.eth.up0                    = (GT_U32)inArgs[16];
    maskData.eth.up0                 = (GT_U32)inArgs[17];
    patternData.eth.cfi0                   = (GT_U32)inArgs[18];
    maskData.eth.cfi0                = (GT_U32)inArgs[19];
    patternData.eth.isVlan1Exists          = (GT_BOOL)inArgs[20];
    maskData.eth.isVlan1Exists       = (GT_BOOL)inArgs[21];
    patternData.eth.vid1                   = (GT_U16)inArgs[22];
    maskData.eth.vid1                = (GT_U16)inArgs[23];
    patternData.eth.up1                    = (GT_U32)inArgs[24];
    maskData.eth.up1                 = (GT_U32)inArgs[25];
    patternData.eth.cfi1                   = (GT_U32)inArgs[26];
    maskData.eth.cfi1                = (GT_U32)inArgs[27];
    patternData.eth.etherType              = (GT_U32)inArgs[28];
    maskData.eth.etherType           = (GT_U32)inArgs[29];
    patternData.eth.macToMe                = (GT_BOOL)inArgs[30];
    maskData.eth.macToMe             = (GT_BOOL)inArgs[31];
    patternData.eth.common.dsaSrcIsTrunk   = inArgs[32];
    maskData.eth.common.dsaSrcIsTrunk= inArgs[33];
    patternData.eth.common.dsaSrcPortTrunk = (GT_U32)inArgs[34];
    maskData.eth.common.dsaSrcPortTrunk = (GT_U32)inArgs[35];
    patternData.eth.common.dsaSrcDevice    = (GT_U32)inArgs[36];
    maskData.eth.common.dsaSrcDevice = (GT_U32)inArgs[37];
    patternData.eth.srcId                  = (GT_U32)inArgs[38];
    maskData.eth.srcId               = (GT_U32)inArgs[39];
    patternData.eth.dsaQosProfile          = (GT_U32)inArgs[40];
    maskData.eth.dsaQosProfile       = (GT_U32)inArgs[41];
    patternData.eth.common.sourcePortGroupId = (GT_U32)inArgs[42];
    maskData.eth.common.sourcePortGroupId = (GT_U32)inArgs[43];

    cpssOsMemCpy(&ethRuleMaskData,&maskData, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemCpy(&ethRulePatternData,&patternData, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    if (patternData.eth.common.srcIsTrunk == GT_FALSE)
    {
        port = (GT_PORT_NUM)patternData.eth.common.srcPortTrunk;

       /* Override Device and Port , should be done for pattern as in mask you cant know how to convert*/
       CONVERT_DEV_PORT_U32_MAC(devNum,port);

       patternData.eth.common.srcPortTrunk = (GT_U32)port;
    }

    /* call cpss api function */
    result = cpssOsTimeRT(&startSec, &startNsec);
    if(result != GT_OK)
    {
        result = cpssOsTimeRT(&endSec, &endNsec);
        if(result != GT_OK)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, result, "%d",numOfRulesAdded);


            galtisOutput(outArgs, result, "");
            return CMD_OK;
        }
        if (endNsec < startNsec)
        {
            endNsec += 1000000000;
            endSec--;
        }

        cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
        cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

        /* print result */
        if (numOfRulesAdded == 0)
        {
            cpssOsPrintf("\n    No TTI Rules were added at all.\n");
        }
        else
        {
            cpssOsPrintf("\n Rules index from %d to %d, in delta of %d.\n",
                         ttiMultiActionDatabaseParamsForEthRule.ruleIndex,
                         ruleIndex,
                         ttiMultiActionDatabaseParamsForEthRule.ruleIndexDelta);
            cpssOsPrintf(" Were added (%d/%d)\n",
                         numOfRulesAdded,
                         ttiMultiActionDatabaseParamsForEthRule.numOfRulesToAdd);
        }
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d",numOfRulesAdded);

        return CMD_OK;
    }

    /* add TTI rules entries */
    for(i=0; i<ttiMultiActionDatabaseParamsForEthRule.numOfRulesToAdd; i++)
    {
       /* the ruleIndex will be changed according to the ruleIndexDelta */
        ruleIndex = ttiMultiActionDatabaseParamsForEthRule.ruleIndex + (i * ttiMultiActionDatabaseParamsForEthRule.ruleIndexDelta);

       /* each entry should have a different action - the action is different
       in its port destination in case the destination is a port,
       else it will be the same action values for all entries */

        actionEntry.egressInterface.devPort.hwDevNum = hwDevNum;

        switch(ruleIndex%4)
        {
            case 0:
                actionEntry.egressInterface.devPort.portNum = portNum_0;
                break;
            case 1:
                actionEntry.egressInterface.devPort.portNum = portNum_1;
                break;
            case 2:
                actionEntry.egressInterface.devPort.portNum = portNum_2;
                break;
            case 3:
                actionEntry.egressInterface.devPort.portNum = portNum_3;
                break;
            default:
                break;
        }
        /* Override Device and Port */
        CONVERT_DEV_PORT_DATA_MAC(actionEntry.egressInterface.devPort.hwDevNum,
                                  actionEntry.egressInterface.devPort.portNum);

        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionEntry.egressInterface.trunkId);

        /* call cpss api function - ruleIndex is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,
                                            ruleIndex,
                                            CPSS_DXCH_TTI_RULE_ETH_E,
                                            &patternData,&maskData,
                                            &actionEntry,
                                            GT_TRUE);
        if(result != GT_OK)
        {
            galtisOutput(outArgs, result, "numOfRulesAdded=%d",numOfRulesAdded);
            return CMD_OK;
        }

        ttiRuleFormat[ruleIndex]     = CPSS_DXCH_TTI_RULE_ETH_E;

        numOfRulesAdded++;

         /* each entry should have an incremented mac address (patternData) */
        if(patternData.eth.common.mac.arEther[5] < 0xFF)
            patternData.eth.common.mac.arEther[5]++;
        else
        {
            patternData.eth.common.mac.arEther[5] = 0;
            if (patternData.eth.common.mac.arEther[4] < 0xFF)
                patternData.eth.common.mac.arEther[4]++;
            else
            {
                patternData.eth.common.mac.arEther[4] = 0;
                if (patternData.eth.common.mac.arEther[3] < 0xFF)
                    patternData.eth.common.mac.arEther[3]++;
                else
                {
                    patternData.eth.common.mac.arEther[3] = 0;
                    if (patternData.eth.common.mac.arEther[2] < 0XFF)
                        patternData.eth.common.mac.arEther[2]++;
                    else
                    {
                        patternData.eth.common.mac.arEther[2] = 0;
                        if (patternData.eth.common.mac.arEther[1] < 0XFF)
                            patternData.eth.common.mac.arEther[1]++;
                        else
                        {
                            patternData.eth.common.mac.arEther[1] = 0;
                            if (patternData.eth.common.mac.arEther[0] < 0XFF)
                                patternData.eth.common.mac.arEther[0]++;
                            else
                            {
                                result = GT_NO_MORE;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    result = cpssOsTimeRT(&endSec, &endNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    if (endNsec < startNsec)
    {
        endNsec += 1000000000;
        endSec--;
    }

    cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
    cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

    /* print result */
    if (numOfRulesAdded == 0)
    {
        cpssOsPrintf("\n    No TTI Rules were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n Rules index from %d to %d, in delta of %d.\n",
                     ttiMultiActionDatabaseParamsForEthRule.ruleIndex,
                     ruleIndex,
                     ttiMultiActionDatabaseParamsForEthRule.ruleIndexDelta);
        cpssOsPrintf(" Were added (%d/%d)\n",
                     numOfRulesAdded,
                     ttiMultiActionDatabaseParamsForEthRule.numOfRulesToAdd);
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",numOfRulesAdded);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiMultiEthRuleGet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiEthRuleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result = GT_OK;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;



    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%6b%6b%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d",
                 ttiMultiActionDatabaseParamsForEthRule.devNum,
                 ttiMultiActionDatabaseParamsForEthRule.numOfRulesToAdd,
                 ttiMultiActionDatabaseParamsForEthRule.ruleIndex,
                 ttiMultiActionDatabaseParamsForEthRule.ruleIndexDelta,
                 ethRulePatternData.eth.common.pclId,
                 ethRuleMaskData.eth.common.pclId,
                 ethRulePatternData.eth.common.srcIsTrunk,
                 ethRuleMaskData.eth.common.srcIsTrunk,
                 ethRulePatternData.eth.common.srcPortTrunk,
                 ethRuleMaskData.eth.common.srcPortTrunk,
                 ethRulePatternData.eth.common.mac.arEther,
                 ethRuleMaskData.eth.common.mac.arEther,
                 ethRulePatternData.eth.common.vid,
                 ethRuleMaskData.eth.common.vid,
                 ethRulePatternData.eth.common.isTagged,
                 ethRuleMaskData.eth.common.isTagged,
                 ethRulePatternData.eth.up0,
                 ethRuleMaskData.eth.up0,
                 ethRulePatternData.eth.cfi0,
                 ethRuleMaskData.eth.cfi0,
                 ethRulePatternData.eth.isVlan1Exists,
                 ethRuleMaskData.eth.isVlan1Exists,
                 ethRulePatternData.eth.vid1,
                 ethRuleMaskData.eth.vid1,
                 ethRulePatternData.eth.up1,
                 ethRuleMaskData.eth.up1,
                 ethRulePatternData.eth.cfi1,
                 ethRuleMaskData.eth.cfi1,
                 ethRulePatternData.eth.etherType,
                 ethRuleMaskData.eth.etherType,
                 ethRulePatternData.eth.macToMe,
                 ethRuleMaskData.eth.macToMe,
                 ethRulePatternData.eth.common.dsaSrcIsTrunk,
                 ethRuleMaskData.eth.common.dsaSrcIsTrunk,
                 ethRulePatternData.eth.common.dsaSrcPortTrunk,
                 ethRuleMaskData.eth.common.dsaSrcPortTrunk,
                 ethRulePatternData.eth.common.dsaSrcDevice,
                 ethRuleMaskData.eth.common.dsaSrcDevice,
                 ethRulePatternData.eth.srcId,
                 ethRuleMaskData.eth.srcId,
                 ethRulePatternData.eth.dsaQosProfile,
                 ethRuleMaskData.eth.dsaQosProfile,
                 ethRulePatternData.eth.common.sourcePortGroupId,
                 ethRuleMaskData.eth.common.sourcePortGroupId);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiMultiEthRuleDelete function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiEthRuleDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;         /* device associated with the action     */
    GT_U32                          ruleIndex;      /* rule index associated with the action */
    GT_U32                          ruleIndexDelta; /* delta between 2 indexes               */
    GT_U32                          numOfRulesToDelete;
    GT_U32                          i=0;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum =(GT_U8)inArgs[0];
    numOfRulesToDelete = (GT_U32)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];
    ruleIndexDelta = 3;

    /* go over all range defined in wrCpssDxChTtiMultiEthRuleSet and
       delete in from HW and from DB */
    for (i=0;i < numOfRulesToDelete; i++)
    {
        /* call cpss api function - routerTtiTcamRow is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleValidStatusSet(devNum, ruleIndex, GT_FALSE, GT_TRUE);
        if(result != GT_OK)
        {
            galtisOutput(outArgs, result, "numOfRulesDeleted=%d",i+1);
            return CMD_OK;
        }
        ruleIndex+=ruleIndexDelta;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiMultiUdbRuleSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiUdbRuleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      numOfRulesAdded=0;
    GT_U32      ruleIndex;
    GT_U32      i = 0;

    CPSS_DXCH_TTI_ACTION_STC        actionEntry;
    GT_PORT_NUM                     portNum_0;      /* port interface to use when (ruleIndex % 4 == 0)                                      */
    GT_PORT_NUM                     portNum_1;      /* port interface to use when (ruleIndex % 4 == 1)                                      */
    GT_PORT_NUM                     portNum_2;      /* port interface to use when (ruleIndex % 4 == 2)                                      */
    GT_PORT_NUM                     portNum_3;      /* port interface to use when (ruleIndex % 4 == 3)                                      */
    GT_HW_DEV_NUM                   hwDevNum;
    CPSS_DXCH_TTI_RULE_TYPE_ENT     ruleFormat;
    ttiMultiActionDB                *ttiMultiActionDatabaseParamsForUdbRule;
    CPSS_DXCH_TTI_KEY_SIZE_ENT      udbRuleSize;    /* size of the TTI UDB Rule              */

    GT_U32      startSec  = 0;
    GT_U32      startNsec = 0;
    GT_U32      endSec  = 0;
    GT_U32      endNsec = 0;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* reset mask and pattern */
    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));


    udbRuleSize  = (CPSS_DXCH_TTI_KEY_SIZE_ENT)inArgs[4];

    switch (udbRuleSize) /* rule size */
    {
        case CPSS_DXCH_TTI_KEY_SIZE_10_B_E:
            ttiMultiActionDatabaseParamsForUdbRule = &ttiMultiActionDatabaseParamsForUdbRuleArray[0];
            cpssOsMemCpy(&actionEntry,&(ttiMultiActionDatabaseParamsForUdbRuleArray[0].actionEntry),sizeof(CPSS_DXCH_TTI_ACTION_STC));

            ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;

            patternData.udbArray.udb[0] = (GT_U8)inArgs[5];
            patternData.udbArray.udb[1] = (GT_U8)inArgs[7];
            patternData.udbArray.udb[2] = (GT_U8)inArgs[9];
            patternData.udbArray.udb[3] = (GT_U8)inArgs[11];
            patternData.udbArray.udb[4] = (GT_U8)inArgs[13];
            patternData.udbArray.udb[5] = (GT_U8)inArgs[15];
            patternData.udbArray.udb[6] = (GT_U8)inArgs[17];
            patternData.udbArray.udb[7] = (GT_U8)inArgs[19];
            patternData.udbArray.udb[8] = (GT_U8)inArgs[21];
            patternData.udbArray.udb[9] = (GT_U8)inArgs[23];

            maskData.udbArray.udb[0] = (GT_U8)inArgs[6];
            maskData.udbArray.udb[1] = (GT_U8)inArgs[8];
            maskData.udbArray.udb[2] = (GT_U8)inArgs[10];
            maskData.udbArray.udb[3] = (GT_U8)inArgs[12];
            maskData.udbArray.udb[4] = (GT_U8)inArgs[14];
            maskData.udbArray.udb[5] = (GT_U8)inArgs[16];
            maskData.udbArray.udb[6] = (GT_U8)inArgs[18];
            maskData.udbArray.udb[7] = (GT_U8)inArgs[20];
            maskData.udbArray.udb[8] = (GT_U8)inArgs[22];
            maskData.udbArray.udb[9] = (GT_U8)inArgs[24];
            cpssOsMemCpy(&udbRuleMaskDataArray[0],&maskData, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemCpy(&udbRulePatternDataArray[0],&patternData, sizeof(CPSS_DXCH_TTI_RULE_UNT));

            break;

        case CPSS_DXCH_TTI_KEY_SIZE_20_B_E:
            ttiMultiActionDatabaseParamsForUdbRule = &ttiMultiActionDatabaseParamsForUdbRuleArray[1];
            cpssOsMemCpy(&actionEntry,&(ttiMultiActionDatabaseParamsForUdbRuleArray[1].actionEntry),sizeof(CPSS_DXCH_TTI_ACTION_STC));

            ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;

            patternData.udbArray.udb[0] = (GT_U8)inArgs[5];
            patternData.udbArray.udb[1] = (GT_U8)inArgs[7];
            patternData.udbArray.udb[2] = (GT_U8)inArgs[9];
            patternData.udbArray.udb[3] = (GT_U8)inArgs[11];
            patternData.udbArray.udb[4] = (GT_U8)inArgs[13];
            patternData.udbArray.udb[5] = (GT_U8)inArgs[15];
            patternData.udbArray.udb[6] = (GT_U8)inArgs[17];
            patternData.udbArray.udb[7] = (GT_U8)inArgs[19];
            patternData.udbArray.udb[8] = (GT_U8)inArgs[21];
            patternData.udbArray.udb[9] = (GT_U8)inArgs[23];

            patternData.udbArray.udb[10] = (GT_U8)inArgs[25];
            patternData.udbArray.udb[11] = (GT_U8)inArgs[27];
            patternData.udbArray.udb[12] = (GT_U8)inArgs[29];
            patternData.udbArray.udb[13] = (GT_U8)inArgs[31];
            patternData.udbArray.udb[14] = (GT_U8)inArgs[33];
            patternData.udbArray.udb[15] = (GT_U8)inArgs[35];
            patternData.udbArray.udb[16] = (GT_U8)inArgs[37];
            patternData.udbArray.udb[17] = (GT_U8)inArgs[39];
            patternData.udbArray.udb[18] = (GT_U8)inArgs[41];
            patternData.udbArray.udb[19] = (GT_U8)inArgs[43];

            maskData.udbArray.udb[0] = (GT_U8)inArgs[6];
            maskData.udbArray.udb[1] = (GT_U8)inArgs[8];
            maskData.udbArray.udb[2] = (GT_U8)inArgs[10];
            maskData.udbArray.udb[3] = (GT_U8)inArgs[12];
            maskData.udbArray.udb[4] = (GT_U8)inArgs[14];
            maskData.udbArray.udb[5] = (GT_U8)inArgs[16];
            maskData.udbArray.udb[6] = (GT_U8)inArgs[18];
            maskData.udbArray.udb[7] = (GT_U8)inArgs[20];
            maskData.udbArray.udb[8] = (GT_U8)inArgs[22];
            maskData.udbArray.udb[9] = (GT_U8)inArgs[24];

            maskData.udbArray.udb[10] = (GT_U8)inArgs[26];
            maskData.udbArray.udb[11] = (GT_U8)inArgs[28];
            maskData.udbArray.udb[12] = (GT_U8)inArgs[30];
            maskData.udbArray.udb[13] = (GT_U8)inArgs[32];
            maskData.udbArray.udb[14] = (GT_U8)inArgs[34];
            maskData.udbArray.udb[15] = (GT_U8)inArgs[36];
            maskData.udbArray.udb[16] = (GT_U8)inArgs[38];
            maskData.udbArray.udb[17] = (GT_U8)inArgs[40];
            maskData.udbArray.udb[18] = (GT_U8)inArgs[42];
            maskData.udbArray.udb[19] = (GT_U8)inArgs[44];
            cpssOsMemCpy(&udbRuleMaskDataArray[1],&maskData, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemCpy(&udbRulePatternDataArray[1],&patternData, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            break;

        case CPSS_DXCH_TTI_KEY_SIZE_30_B_E:
            ttiMultiActionDatabaseParamsForUdbRule = &ttiMultiActionDatabaseParamsForUdbRuleArray[2];
            cpssOsMemCpy(&actionEntry,&(ttiMultiActionDatabaseParamsForUdbRuleArray[2].actionEntry),sizeof(CPSS_DXCH_TTI_ACTION_STC));

            ruleFormat = CPSS_DXCH_TTI_RULE_UDB_30_E;

             patternData.udbArray.udb[0] = (GT_U8)inArgs[5];
            patternData.udbArray.udb[1] = (GT_U8)inArgs[7];
            patternData.udbArray.udb[2] = (GT_U8)inArgs[9];
            patternData.udbArray.udb[3] = (GT_U8)inArgs[11];
            patternData.udbArray.udb[4] = (GT_U8)inArgs[13];
            patternData.udbArray.udb[5] = (GT_U8)inArgs[15];
            patternData.udbArray.udb[6] = (GT_U8)inArgs[17];
            patternData.udbArray.udb[7] = (GT_U8)inArgs[19];
            patternData.udbArray.udb[8] = (GT_U8)inArgs[21];
            patternData.udbArray.udb[9] = (GT_U8)inArgs[23];

            patternData.udbArray.udb[10] = (GT_U8)inArgs[25];
            patternData.udbArray.udb[11] = (GT_U8)inArgs[27];
            patternData.udbArray.udb[12] = (GT_U8)inArgs[29];
            patternData.udbArray.udb[13] = (GT_U8)inArgs[31];
            patternData.udbArray.udb[14] = (GT_U8)inArgs[33];
            patternData.udbArray.udb[15] = (GT_U8)inArgs[35];
            patternData.udbArray.udb[16] = (GT_U8)inArgs[37];
            patternData.udbArray.udb[17] = (GT_U8)inArgs[39];
            patternData.udbArray.udb[18] = (GT_U8)inArgs[41];
            patternData.udbArray.udb[19] = (GT_U8)inArgs[43];

            patternData.udbArray.udb[20] = (GT_U8)inArgs[45];
            patternData.udbArray.udb[21] = (GT_U8)inArgs[47];
            patternData.udbArray.udb[22] = (GT_U8)inArgs[49];
            patternData.udbArray.udb[23] = (GT_U8)inArgs[51];
            patternData.udbArray.udb[24] = (GT_U8)inArgs[53];
            patternData.udbArray.udb[25] = (GT_U8)inArgs[55];
            patternData.udbArray.udb[26] = (GT_U8)inArgs[57];
            patternData.udbArray.udb[27] = (GT_U8)inArgs[59];
            patternData.udbArray.udb[28] = (GT_U8)inArgs[61];
            patternData.udbArray.udb[29] = (GT_U8)inArgs[63];

            maskData.udbArray.udb[0] = (GT_U8)inArgs[6];
            maskData.udbArray.udb[1] = (GT_U8)inArgs[8];
            maskData.udbArray.udb[2] = (GT_U8)inArgs[10];
            maskData.udbArray.udb[3] = (GT_U8)inArgs[12];
            maskData.udbArray.udb[4] = (GT_U8)inArgs[14];
            maskData.udbArray.udb[5] = (GT_U8)inArgs[16];
            maskData.udbArray.udb[6] = (GT_U8)inArgs[18];
            maskData.udbArray.udb[7] = (GT_U8)inArgs[20];
            maskData.udbArray.udb[8] = (GT_U8)inArgs[22];
            maskData.udbArray.udb[9] = (GT_U8)inArgs[24];

            maskData.udbArray.udb[10] = (GT_U8)inArgs[26];
            maskData.udbArray.udb[11] = (GT_U8)inArgs[28];
            maskData.udbArray.udb[12] = (GT_U8)inArgs[30];
            maskData.udbArray.udb[13] = (GT_U8)inArgs[32];
            maskData.udbArray.udb[14] = (GT_U8)inArgs[34];
            maskData.udbArray.udb[15] = (GT_U8)inArgs[36];
            maskData.udbArray.udb[16] = (GT_U8)inArgs[38];
            maskData.udbArray.udb[17] = (GT_U8)inArgs[40];
            maskData.udbArray.udb[18] = (GT_U8)inArgs[42];
            maskData.udbArray.udb[19] = (GT_U8)inArgs[44];

            maskData.udbArray.udb[20] = (GT_U8)inArgs[46];
            maskData.udbArray.udb[21] = (GT_U8)inArgs[48];
            maskData.udbArray.udb[22] = (GT_U8)inArgs[50];
            maskData.udbArray.udb[23] = (GT_U8)inArgs[52];
            maskData.udbArray.udb[24] = (GT_U8)inArgs[54];
            maskData.udbArray.udb[25] = (GT_U8)inArgs[56];
            maskData.udbArray.udb[26] = (GT_U8)inArgs[58];
            maskData.udbArray.udb[27] = (GT_U8)inArgs[60];
            maskData.udbArray.udb[28] = (GT_U8)inArgs[62];
            maskData.udbArray.udb[29] = (GT_U8)inArgs[64];

            cpssOsMemCpy(&udbRuleMaskDataArray[2],&maskData, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemCpy(&udbRulePatternDataArray[2],&patternData, sizeof(CPSS_DXCH_TTI_RULE_UNT));

            break;

        default:
            galtisOutput(outArgs, result, "");
            return CMD_OK;
    }

    hwDevNum  = ttiMultiActionDatabaseParamsForUdbRule->actionEntry.egressInterface.devPort.hwDevNum;
    portNum_0 = ttiMultiActionDatabaseParamsForUdbRule->portNum_0;
    portNum_1 = ttiMultiActionDatabaseParamsForUdbRule->portNum_1;
    portNum_2 = ttiMultiActionDatabaseParamsForUdbRule->portNum_2;
    portNum_3 = ttiMultiActionDatabaseParamsForUdbRule->portNum_3;

    devNum = ttiMultiActionDatabaseParamsForUdbRule->devNum  = (GT_U8)inArgs[0];
    ttiMultiActionDatabaseParamsForUdbRule->numOfRulesToAdd  = (GT_U32)inArgs[1];
    ttiMultiActionDatabaseParamsForUdbRule->ruleIndex        = (GT_U32)inArgs[2];
    ttiMultiActionDatabaseParamsForUdbRule->ruleIndexDelta   = (GT_U32)inArgs[3];

    ruleIndex=ttiMultiActionDatabaseParamsForUdbRule->ruleIndex;/* set first ruleIndex for the loop */

    /* call cpss api function */
    result = cpssOsTimeRT(&startSec, &startNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* add TTI rules entries */
    for(i=0; i<ttiMultiActionDatabaseParamsForUdbRule->numOfRulesToAdd; i++)
    {
       /* the ruleIndex will be changed according to the ruleIndexDelta */
        ruleIndex = ttiMultiActionDatabaseParamsForUdbRule->ruleIndex + (i * ttiMultiActionDatabaseParamsForUdbRule->ruleIndexDelta);

       /* each entry should have a different action - the action is different
       in its port destination in case the destination is a port,
       else it will be the same action values for all entries */

        actionEntry.egressInterface.devPort.hwDevNum = hwDevNum;

        switch(ruleIndex%4)
        {
            case 0:
                actionEntry.egressInterface.devPort.portNum = portNum_0;
                break;
            case 1:
                actionEntry.egressInterface.devPort.portNum = portNum_1;
                break;
            case 2:
                actionEntry.egressInterface.devPort.portNum = portNum_2;
                break;
            case 3:
                actionEntry.egressInterface.devPort.portNum = portNum_3;
                break;
            default:
                break;
        }
        /* Override Device and Port */
        CONVERT_DEV_PORT_DATA_MAC(actionEntry.egressInterface.devPort.hwDevNum,
                                  actionEntry.egressInterface.devPort.portNum);

        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionEntry.egressInterface.trunkId);

        /* call cpss api function - ruleIndex is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleSet(devNum,
                                            ruleIndex,
                                            ruleFormat,
                                            &patternData,&maskData,
                                            &actionEntry,
                                            GT_TRUE);
        if(result != GT_OK)
        {

            result = cpssOsTimeRT(&endSec, &endNsec);
            if(result != GT_OK)
            {
                galtisOutput(outArgs, result, "%d",numOfRulesAdded);
                return CMD_OK;
            }
            if (endNsec < startNsec)
            {
                endNsec += 1000000000;
                endSec--;
            }

            cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
            cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

            /* print result */
            if (numOfRulesAdded == 0)
            {
                cpssOsPrintf("\n    No TTI Rules were added at all.\n");
            }
            else
            {
                cpssOsPrintf("\n Rules index from %d to %d, in delta of %d.\n",
                             ttiMultiActionDatabaseParamsForUdbRule->ruleIndex,
                             ruleIndex,
                             ttiMultiActionDatabaseParamsForUdbRule->ruleIndexDelta);
                cpssOsPrintf(" Were added (%d/%d)\n",
                             numOfRulesAdded,
                             ttiMultiActionDatabaseParamsForUdbRule->numOfRulesToAdd);
            }
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, result, "%d",numOfRulesAdded);

            return CMD_OK;
        }

        ttiRuleFormat[ruleIndex]     = ruleFormat;

        numOfRulesAdded++;

         /* each entry should have an incremented udb (patternData) */
        if(patternData.udbArray.udb[0] < 0xFF)
            patternData.udbArray.udb[0]++;
        else
        {
            patternData.udbArray.udb[0] = 0;
            if (patternData.udbArray.udb[1] < 0xFF)
                patternData.udbArray.udb[1]++;
            else
            {
                patternData.udbArray.udb[1] = 0;
                if (patternData.udbArray.udb[2] < 0xFF)
                    patternData.udbArray.udb[2]++;
                else
                {
                    patternData.udbArray.udb[2] = 0;
                    if (patternData.udbArray.udb[3] < 0XFF)
                        patternData.udbArray.udb[3]++;
                    else
                    {
                        patternData.udbArray.udb[3] = 0;
                        if (patternData.udbArray.udb[4] < 0XFF)
                            patternData.udbArray.udb[4]++;
                        else
                        {
                            patternData.udbArray.udb[4] = 0;
                            if (patternData.udbArray.udb[5] < 0XFF)
                                patternData.udbArray.udb[5]++;
                            else
                            {
                                result = GT_NO_MORE;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    result = cpssOsTimeRT(&endSec, &endNsec);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    if (endNsec < startNsec)
    {
        endNsec += 1000000000;
        endSec--;
    }

    cpssOsPrintf("\n processTimeSec = %d\n",endSec - startSec);
    cpssOsPrintf("\n processTimeNsec = %d\n",endNsec - startNsec);

    /* print result */
    if (numOfRulesAdded == 0)
    {
        cpssOsPrintf("\n    No TTI Rules were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n Rules index from %d to %d, in delta of %d.\n",
                     ttiMultiActionDatabaseParamsForUdbRule->ruleIndex,
                     ruleIndex,
                     ttiMultiActionDatabaseParamsForUdbRule->ruleIndexDelta);
        cpssOsPrintf(" Were added (%d/%d)\n",
                     numOfRulesAdded,
                     ttiMultiActionDatabaseParamsForUdbRule->numOfRulesToAdd);
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",numOfRulesAdded);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiMultiUdbRuleGet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiUdbRuleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result = GT_OK;
    CPSS_DXCH_TTI_KEY_SIZE_ENT   udbRuleSize;    /* size of the TTI UDB Rule              */
    ttiMultiActionDB             *ttiMultiActionDatabaseParamsForUdbRule;
    CPSS_DXCH_TTI_RULE_UNT       *udbRuleMaskData;
    CPSS_DXCH_TTI_RULE_UNT       *udbRulePatternData;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    udbRuleSize  = (CPSS_DXCH_TTI_KEY_SIZE_ENT)inArgs[0];

    switch (udbRuleSize) /* rule size */
    {
        case CPSS_DXCH_TTI_KEY_SIZE_10_B_E:
            ttiMultiActionDatabaseParamsForUdbRule = &ttiMultiActionDatabaseParamsForUdbRuleArray[0];
            udbRuleMaskData = &udbRuleMaskDataArray[0];
            udbRulePatternData = &udbRulePatternDataArray[0];
            break;

        case CPSS_DXCH_TTI_KEY_SIZE_20_B_E:
            ttiMultiActionDatabaseParamsForUdbRule = &ttiMultiActionDatabaseParamsForUdbRuleArray[1];
            udbRuleMaskData = &udbRuleMaskDataArray[1];
            udbRulePatternData = &udbRulePatternDataArray[1];
            break;

        case CPSS_DXCH_TTI_KEY_SIZE_30_B_E:
            ttiMultiActionDatabaseParamsForUdbRule = &ttiMultiActionDatabaseParamsForUdbRuleArray[2];
            udbRuleMaskData = &udbRuleMaskDataArray[2];
            udbRulePatternData = &udbRulePatternDataArray[2];
            break;

        default:
            galtisOutput(outArgs, result, "");
            return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d",
                 ttiMultiActionDatabaseParamsForUdbRule->devNum,
                 ttiMultiActionDatabaseParamsForUdbRule->numOfRulesToAdd,
                 ttiMultiActionDatabaseParamsForUdbRule->ruleIndex,
                 ttiMultiActionDatabaseParamsForUdbRule->ruleIndexDelta,
                 udbRulePatternData->udbArray.udb[0],
                 udbRuleMaskData->udbArray.udb[0],
                 udbRulePatternData->udbArray.udb[1],
                 udbRuleMaskData->udbArray.udb[1],
                 udbRulePatternData->udbArray.udb[2],
                 udbRuleMaskData->udbArray.udb[2],
                 udbRulePatternData->udbArray.udb[3],
                 udbRuleMaskData->udbArray.udb[3],
                 udbRulePatternData->udbArray.udb[4],
                 udbRuleMaskData->udbArray.udb[4],
                 udbRulePatternData->udbArray.udb[5],
                 udbRuleMaskData->udbArray.udb[5],
                 udbRulePatternData->udbArray.udb[6],
                 udbRuleMaskData->udbArray.udb[6],
                 udbRulePatternData->udbArray.udb[7],
                 udbRuleMaskData->udbArray.udb[7],
                 udbRulePatternData->udbArray.udb[8],
                 udbRuleMaskData->udbArray.udb[8],
                 udbRulePatternData->udbArray.udb[9],
                 udbRuleMaskData->udbArray.udb[9],
                 udbRulePatternData->udbArray.udb[10],
                 udbRuleMaskData->udbArray.udb[10],
                 udbRulePatternData->udbArray.udb[11],
                 udbRuleMaskData->udbArray.udb[11],
                 udbRulePatternData->udbArray.udb[12],
                 udbRuleMaskData->udbArray.udb[12],
                 udbRulePatternData->udbArray.udb[13],
                 udbRuleMaskData->udbArray.udb[13],
                 udbRulePatternData->udbArray.udb[14],
                 udbRuleMaskData->udbArray.udb[14],
                 udbRulePatternData->udbArray.udb[15],
                 udbRuleMaskData->udbArray.udb[15],
                 udbRulePatternData->udbArray.udb[16],
                 udbRuleMaskData->udbArray.udb[16],
                 udbRulePatternData->udbArray.udb[17],
                 udbRuleMaskData->udbArray.udb[17],
                 udbRulePatternData->udbArray.udb[18],
                 udbRuleMaskData->udbArray.udb[18],
                 udbRulePatternData->udbArray.udb[19],
                 udbRuleMaskData->udbArray.udb[19],
                 udbRulePatternData->udbArray.udb[20],
                 udbRuleMaskData->udbArray.udb[20],
                 udbRulePatternData->udbArray.udb[21],
                 udbRuleMaskData->udbArray.udb[21],
                 udbRulePatternData->udbArray.udb[22],
                 udbRuleMaskData->udbArray.udb[22],
                 udbRulePatternData->udbArray.udb[23],
                 udbRuleMaskData->udbArray.udb[23],
                 udbRulePatternData->udbArray.udb[24],
                 udbRuleMaskData->udbArray.udb[24],
                 udbRulePatternData->udbArray.udb[25],
                 udbRuleMaskData->udbArray.udb[25],
                 udbRulePatternData->udbArray.udb[26],
                 udbRuleMaskData->udbArray.udb[26],
                 udbRulePatternData->udbArray.udb[27],
                 udbRuleMaskData->udbArray.udb[27],
                 udbRulePatternData->udbArray.udb[28],
                 udbRuleMaskData->udbArray.udb[28],
                 udbRulePatternData->udbArray.udb[29],
                 udbRuleMaskData->udbArray.udb[29]);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiMultiUdbRuleDelete function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChTtiMultiUdbRuleDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    GT_U8                           devNum;         /* device associated with the action     */
    GT_U32                          ruleIndex;      /* rule index associated with the action */
    GT_U32                          ruleIndexDelta; /* delta between 2 indexes               */
    GT_U32                          numOfRulesToDelete;
    GT_U32                          i=0;
    CPSS_DXCH_TTI_KEY_SIZE_ENT      udbRuleSize;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum =(GT_U8)inArgs[0];
    numOfRulesToDelete = (GT_U32)inArgs[1];
    udbRuleSize  = (CPSS_DXCH_TTI_KEY_SIZE_ENT)inArgs[2];
    ruleIndex = (GT_U32)inArgs[3];


    switch (udbRuleSize) /* rule size */
    {
        case CPSS_DXCH_TTI_KEY_SIZE_10_B_E:
            ruleIndexDelta = 1;
            break;

        case CPSS_DXCH_TTI_KEY_SIZE_20_B_E:
            ruleIndexDelta = 2;
            break;

        case CPSS_DXCH_TTI_KEY_SIZE_30_B_E:
            ruleIndexDelta = 3;
            break;

        default:
            galtisOutput(outArgs, GT_NOT_SUPPORTED, "udbRuleSize=%d, is not supported",udbRuleSize);
            return CMD_OK;
    }

    /* go over all range delete in from HW - the DB  is not deleted and will show the last added block */
    for (i=0;i < numOfRulesToDelete; i++)
    {
        /* call cpss api function - routerTtiTcamRow is an absolute index */
        result = pg_wrap_cpssDxChTtiRuleValidStatusSet(devNum, ruleIndex, GT_FALSE, GT_TRUE);
        if(result != GT_OK)
        {
            galtisOutput(outArgs, result, "numOfRulesDeleted=%d",i+1);
            return CMD_OK;
        }
        ruleIndex+=ruleIndexDelta;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiFcoeForwardingEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet matches the "FCoE Ethertype", it is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeForwardingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiFcoeForwardingEnableSet(devNum,enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return if a packet that matches the "FCoE Ethertype", is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeForwardingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_BOOL    enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiFcoeForwardingEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeEtherTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U16      etherType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    etherType = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiFcoeEtherTypeSet(devNum,etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeEtherTypeGet function
* @endinternal
*
* @brief   Return the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeEtherTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U16     etherType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiFcoeEtherTypeGet(devNum, &etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "0x%X", etherType);

    return CMD_OK;
}
/**
* @internal wrCpssDxChTtiFcoeExceptionConfiguratiosEnableSet function
* @endinternal
*
* @brief   Enable FCoE Exception Configuration.
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionConfiguratiosEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType;
    GT_BOOL                              enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(devNum,exceptionType,enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeExceptionConfiguratiosEnableGet function
* @endinternal
*
* @brief   Return FCoE Exception Configuration status
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionConfiguratiosEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType;
    GT_BOOL                              enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(devNum, exceptionType, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeExceptionCountersGet function
* @endinternal
*
* @brief   Return number of FCoE exceptions of a given type.
*         When the counter reaches 0xFF it sticks to this value, i.e., it does not wrap around
*         The counter is clear on read
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType;
    GT_U32                               counterValue;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiFcoeExceptionCountersGet(devNum, exceptionType, &counterValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counterValue);

    return CMD_OK;
}


/**
* @internal wrCpssDxChTtiFcoeAssignVfIdEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet is FCoE and it contains a VF Tag, then the
*         VRF-ID is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeAssignVfIdEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;
    GT_PORT_NUM portNum;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTtiFcoeAssignVfIdEnableSet(devNum,portNum,enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeAssignVfIdEnableGet function
* @endinternal
*
* @brief   Return if VRF-ID is assigned with the value VF_ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for FCoE packets which contain a VF Tag
*
*/
static CMD_STATUS wrCpssDxChTtiFcoeAssignVfIdEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_BOOL    enable;
    GT_PORT_NUM portNum;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum= (GT_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiFcoeAssignVfIdEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionPacketCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;
    GT_U32      devNum;
    CPSS_PACKET_CMD_ENT command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    devNum         = 0;    /*reset on first*/

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U32)inArgs[0];
    command = (GT_U32)inArgs[1];

    status = cpssDxChTtiFcoeExceptionPacketCommandSet(devNum, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionPacketCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;
    GT_U32      devNum;
    CPSS_PACKET_CMD_ENT command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    devNum      = 0;    /*reset on first*/
    command     = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U32)inArgs[0];

    status = cpssDxChTtiFcoeExceptionPacketCommandGet(devNum, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionCpuCodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_NET_RX_CPU_CODE_ENT        cpuCode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTtiFcoeExceptionCpuCodeSet(devNum, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiFcoeExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiFcoeExceptionCpuCodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTtiFcoeExceptionCpuCodeGet(devNum, &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiEcnPacketCommandSet function
* @endinternal
*
* @brief   Set packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or packet command.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiEcnPacketCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_PACKET_CMD_ENT  command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    command = (CPSS_PACKET_CMD_ENT)(inArgs[1]);

    /* call port group api function */
    result = cpssDxChTtiEcnPacketCommandSet(devNum, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiEcnPacketCommandGet function
* @endinternal
*
* @brief   Get packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] commandPtr           - (pointer to) packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_VALUE             - on bad packet command value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChTtiEcnPacketCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_PACKET_CMD_ENT  command;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call port group api function */
    result = cpssDxChTtiEcnPacketCommandGet(devNum, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiEcnCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] cpuCode               - CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or CPU code.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTtiEcnCpuCodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_NET_RX_CPU_CODE_ENT  cpuCode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)(inArgs[1]);

    /* call port group api function */
    result = cpssDxChTtiEcnCpuCodeSet(devNum, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTtiEcnCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] cpuCodePtr           - (pointer to) CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChTtiEcnCpuCodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_NET_RX_CPU_CODE_ENT  cpuCode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call port group api function */
    result = cpssDxChTtiEcnCpuCodeGet(devNum, &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChTtiMacToMeSet",
        &wrCpssDxChTtiMacToMeSet,
        1, 5},
    {"cpssDxChTtiMacToMeGetFirst",
        &wrCpssDxChTtiMacToMeGetFirst,
        1, 0},
    {"cpssDxChTtiMacToMeGetNext",
        &wrCpssDxChTtiMacToMeGetNext,
        1, 0},
    {"cpssDxChTtiMacToMe_1Set",
        &wrCpssDxChTtiMacToMe_1Set,
        3, 9},
    {"cpssDxChTtiMacToMe_1GetFirst",
        &wrCpssDxChTtiMacToMe_1GetFirst,
        3, 0},
    {"cpssDxChTtiMacToMe_1GetNext",
        &wrCpssDxChTtiMacToMe_1GetNext,
        3, 0},
    {"cpssDxChTtiPortLookupEnableSet",
        &wrCpssDxChTtiPortLookupEnableSet,
        4, 0},
    {"cpssDxChTtiPortLookupEnableGet",
        &wrCpssDxChTtiPortLookupEnableGet,
        3, 0},
    {"cpssDxChTtiPortIpv4OnlyTunneledEnableSet",
        &wrCpssDxChTtiPortIpv4OnlyTunneledEnableSet,
        3, 0},
    {"cpssDxChTtiPortIpv4OnlyTunneledEnableGet",
        &wrCpssDxChTtiPortIpv4OnlyTunneledEnableGet,
        2, 0},
    {"cpssDxChTtiPortIpv4OnlyMac2MeEnableSet",
        &wrCpssDxChTtiPortIpv4OnlyMac2MeEnableSet,
        3, 0},
    {"cpssDxChTtiPortIpv4OnlyMac2MeEnableGet",
        &wrCpssDxChTtiPortIpv4OnlyMac2MeEnableGet,
        2, 0},
    {"cpssDxChTtiIpv4McEnableSet",
        &wrCpssDxChTtiIpv4McEnableSet,
        2, 0},
    {"cpssDxChTtiIpv4McEnableGet",
        &wrCpssDxChTtiIpv4McEnableGet,
        1, 0},
    {"cpssDxChTtiPortMplsOnlyMac2MeEnableSet",
        &wrCpssDxChTtiPortMplsOnlyMac2MeEnableSet,
        3, 0},
    {"cpssDxChTtiPortMplsOnlyMac2MeEnableGet",
        &wrCpssDxChTtiPortMplsOnlyMac2MeEnableGet,
        2, 0},
    {"cpssDxChTtiPortMimOnlyMacToMeEnableSet",
        &wrCpssDxChTtiPortMimOnlyMacToMeEnableSet,
        3, 0},
    {"cpssDxChTtiPortMimOnlyMacToMeEnableGet",
        &wrCpssDxChTtiPortMimOnlyMacToMeEnableGet,
        2, 0},
    {"cpssDxChTtiRuleSetFirst",
        &wrCpssDxChTtiRuleSetFirst,
        2, 19},
    {"cpssDxChTtiRuleSetNext",
        &wrCpssDxChTtiRuleSetNext,
        2, 19},
    {"cpssDxChTtiRuleEndSet",
        &wrCpssDxChTtiRuleEndSet,
        0, 0},
    {"cpssDxChTtiRuleGetFirst",
        &wrCpssDxChTtiRuleGetFirst,
        1, 0},
    {"cpssDxChTtiRuleGetNext",
        &wrCpssDxChTtiRuleGet,
        1, 0},
    {"cpssDxChTtiRuleActionHwGetFirst",
        &wrCpssDxChTtiRuleActionHwGetFirst,
        4, 0},
    {"cpssDxChTtiRuleActionHwGetNext",
        &wrCpssDxChTtiRuleActionHwGet,
        4, 0},
    {"cpssDxChTtiRuleActionClear",
        &wrCpssDxChTtiRuleActionClear,
        0, 0},
    {"cpssDxChTtiRuleActionDelete",
        &wrCpssDxChTtiRuleActionDelete,
        0, 2},
    {"cpssDxChTtiRule_1SetFirst",
        &wrCpssDxChTtiRule_1SetFirst,
        4, 19},
    {"cpssDxChTtiRule_1SetNext",
        &wrCpssDxChTtiRule_1SetNext,
        4, 19},
    {"cpssDxChTtiRule_1EndSet",
        &wrCpssDxChTtiRule_1EndSet,
        0, 0},
    {"cpssDxChTtiRule_1GetFirst",
        &wrCpssDxChTtiRule_1GetFirst,
        4, 0},
    {"cpssDxChTtiRule_1GetNext",
        &wrCpssDxChTtiRule_1Get,
        4, 0},
    {"cpssDxChTtiRuleAction_1Set",
        &wrCpssDxChTtiRuleAction_1Update,
        1, 60},
    {"cpssDxChTtiRuleAction_1GetFirst",
        &wrCpssDxChTtiRuleAction_1GetFirst,
        1, 0},
    {"cpssDxChTtiRuleAction_1GetNext",
        &wrCpssDxChTtiRuleAction_1Get,
        1, 0},
    {"cpssDxChTtiRuleAction_1Clear",
        &wrCpssDxChTtiRuleActionClear,
        1, 0},
    {"cpssDxChTtiRuleAction_1Delete",
        &wrCpssDxChTtiRuleActionDelete,
        1, 2},

    {"cpssDxChTtiMacModeSet",
        &wrCpssDxChTtiMacModeSet,
        3, 0},
    {"cpssDxChTtiMacModeGet",
        &wrCpssDxChTtiMacModeGet,
        2, 0},
    {"cpssDxChTtiPclIdSet",
        &wrCpssDxChTtiPclIdSet,
        3, 0},
    {"cpssDxChTtiPclIdGet",
        &wrCpssDxChTtiPclIdGet,
        2, 0},
    {"cpssDxChTtiIpv4GreEthTypeSet",
        &wrCpssDxChTtiIpv4GreEthTypeSet,
        3, 0},
    {"cpssDxChTtiIpv4GreEthTypeGet",
        &wrCpssDxChTtiIpv4GreEthTypeGet,
        2, 0},
    {"cpssDxChTtiEthernetTypeSet",
        &wrCpssDxChTtiEthernetTypeSet,
        3, 0},
    {"cpssDxChTtiEthernetTypeGet",
        &wrCpssDxChTtiEthernetTypeGet,
        2, 0},
    {"cpssDxChTtiMimEthTypeSet",
        &wrCpssDxChTtiMimEthTypeSet,
        2, 0},
    {"cpssDxChTtiMimEthTypeGet",
        &wrCpssDxChTtiMimEthTypeGet,
        1, 0},
    {"cpssDxChTtiMplsEthTypeSet",
        &wrCpssDxChTtiMplsEthTypeSet,
        3, 0},
    {"cpssDxChTtiMplsEthTypeGet",
        &wrCpssDxChTtiMplsEthTypeGet,
        2, 0},
    {"cpssDxChTtiExceptionCmdSet",
        &wrCpssDxChTtiExceptionCmdSet,
        3, 0},
    {"cpssDxChTtiExceptionCmdGet",
        &wrCpssDxChTtiExceptionCmdGet,
        2, 0},
    {"cpssDxChTtiExceptionCmdSet_1",
        &wrCpssDxChTtiExceptionCmdSet,
        3, 0},
    {"cpssDxChTtiExceptionCmdGet_1",
        &wrCpssDxChTtiExceptionCmdGet,
        2, 0},

    {"cpssDxChTtiRuleValidStatusSet",
        &wrCpssDxChTtiRuleValidStatusSet,
        3, 0},
    {"cpssDxChTtiRuleValidStatus_2Set",
        &wrCpssDxChTtiRuleValidStatus_2Set,
        3, 0},
    {"cpssDxChTtiRuleValidStatusGet",
        &wrCpssDxChTtiRuleValidStatusGet,
        2, 0},
    {"cpssDxChTtiRuleValidStatus_2Get",
        &wrCpssDxChTtiRuleValidStatus_2Get,
        2, 0},

    {"cpssDxChTtiRule_2SetFirst",
        &wrCpssDxChTtiRule_2SetFirst,
        4, 20},
    {"cpssDxChTtiRule_2SetNext",
        &wrCpssDxChTtiRule_2SetNext,
        4, 20},
    {"cpssDxChTtiRule_2EndSet",
        &wrCpssDxChTtiRule_2EndSet,
        0, 0},
    {"cpssDxChTtiRule_2CancelSet",
        &wrCpssDxChTtiRule_2CancelSet,
        0, 0},
    {"cpssDxChTtiRule_2GetFirst",
        &wrCpssDxChTtiRule_2GetFirst,
        4, 0},
    {"cpssDxChTtiRule_2GetNext",
        &wrCpssDxChTtiRule_2Get,
        4, 0},

    {"cpssDxChTtiRule_4SetFirst",
        &wrCpssDxChTtiRule_4SetFirst,
        4, 36},
    {"cpssDxChTtiRule_4SetNext",
        &wrCpssDxChTtiRule_4SetNext,
        4, 36},
    {"cpssDxChTtiRule_4EndSet",
        &wrCpssDxChTtiRule_2EndSet,
        0, 0},
    {"cpssDxChTtiRule_4CancelSet",
        &wrCpssDxChTtiRule_2CancelSet,
        0, 0},
    {"cpssDxChTtiRule_4GetFirst",
        &wrCpssDxChTtiRule_4GetFirst,
        4, 0},
    {"cpssDxChTtiRule_4GetNext",
        &wrCpssDxChTtiRule_4Get,
        4, 0},

    {"cpssDxChTtiRule_5SetFirst",
        &wrCpssDxChTtiRule_5SetFirst,
        4, 36},
    {"cpssDxChTtiRule_5SetNext",
        &wrCpssDxChTtiRule_5SetNext,
        4, 36},
    {"cpssDxChTtiRule_5EndSet",
        &wrCpssDxChTtiRule_2EndSet,
        0, 0},
    {"cpssDxChTtiRule_5CancelSet",
        &wrCpssDxChTtiRule_2CancelSet,
        0, 0},

    {"cpssDxChTtiRule_5GetFirst",
        &wrCpssDxChTtiRule_5GetFirst,
        4, 0},
    {"cpssDxChTtiRule_5GetNext",
        &wrCpssDxChTtiRule_5Get,
        4, 0},

    {"cpssDxChTtiRuleAction_2Set",
        &wrCpssDxChTtiRuleAction_2Update,
        1, 65},
    {"cpssDxChTtiRuleAction_2GetFirst",
        &wrCpssDxChTtiRuleAction_2GetFirst,
        1, 0},
    {"cpssDxChTtiRuleAction_2GetNext",
        &wrCpssDxChTtiRuleAction_2Get,
        1, 0},
    {"cpssDxChTtiRuleAction_2Clear",
        &wrCpssDxChTtiRuleActionClear,
        1, 0},
    {"cpssDxChTtiRuleAction_2Delete",
        &wrCpssDxChTtiRuleActionDelete,
        1, 2},

    {"cpssDxChTtiRuleAction_4Set",
        &wrCpssDxChTtiRuleAction_2Update,
        1, 65},
    {"cpssDxChTtiRuleAction_4GetFirst",
        &wrCpssDxChTtiRuleAction_2GetFirst,
        1, 0},
    {"cpssDxChTtiRuleAction_4GetNext",
        &wrCpssDxChTtiRuleAction_2Get,
        1, 0},
    {"cpssDxChTtiRuleAction_4Clear",
        &wrCpssDxChTtiRuleActionClear,
        1, 0},
    {"cpssDxChTtiRuleAction_4Delete",
        &wrCpssDxChTtiRuleActionDelete,
        1, 2},

    {"cpssDxChTtiRuleAction_5Set",
        &wrCpssDxChTtiRuleAction_5Update,
        1, 95},
    {"cpssDxChTtiRuleAction_5GetFirst",
        &wrCpssDxChTtiRuleAction_5GetFirst,
        1, 0},
    {"cpssDxChTtiRuleAction_5GetNext",
        &wrCpssDxChTtiRuleAction_5Get,
        1, 0},
    {"cpssDxChTtiRuleAction_5Clear",
        &wrCpssDxChTtiRuleActionClear,
        1, 0},
    {"cpssDxChTtiRuleAction_5Delete",
        &wrCpssDxChTtiRuleActionDelete,
        1, 2},

    {"cpssDxChTtiRule_6SetFirst",
        &wrCpssDxChTtiRule_6SetFirst,
        4, 20},
    {"cpssDxChTtiRule_6SetNext",
        &wrCpssDxChTtiRule_6SetNext,
        4, 20},
    {"cpssDxChTtiRule_6EndSet",
        &wrCpssDxChTtiRule_2EndSet,
        0, 0},
    {"cpssDxChTtiRule_6CancelSet",
        &wrCpssDxChTtiRule_2CancelSet,
        0, 0},
    {"cpssDxChTtiRule_6GetFirst",
        &wrCpssDxChTtiRule_6GetFirst,
        4, 0},
    {"cpssDxChTtiRule_6GetNext",
        &wrCpssDxChTtiRule_6Get,
        4, 0},

    {"cpssDxChTtiRuleAction_6Set",
        &wrCpssDxChTtiRuleAction_6Update,
        1, 99},
    {"cpssDxChTtiRuleAction_6GetFirst",
        &wrCpssDxChTtiRuleAction_6GetFirst,
        1, 0},
    {"cpssDxChTtiRuleAction_6GetNext",
        &wrCpssDxChTtiRuleAction_6Get,
        1, 0},
    {"cpssDxChTtiRuleAction_6Clear",
        &wrCpssDxChTtiRuleActionClear,
        1, 0},
    {"cpssDxChTtiRuleAction_6Delete",
        &wrCpssDxChTtiRuleActionDelete,
        1, 2},

    {"cpssDxChTtiBypassHeaderLenCheckIpv4TtiExpEnableSet",
        &wrCpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet,
        2, 0},
    {"cpssDxChTtiBypassHeaderLenCheckIpv4TtiExpEnableGet",
        &wrCpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet,
        1, 0},

    {"cpssDxChTtiPwCwSequencingSupportEnableSet",
        &wrCpssDxChTtiPwCwSequencingSupportEnableSet,
        2, 0},
    {"cpssDxChTtiPwCwSequencingSupportEnableGet",
        &wrCpssDxChTtiPwCwSequencingSupportEnableGet,
        1, 0},

    {"cpssDxChTtiUserDefinedByteSet",
        &wrCpssDxChTtiUserDefinedByteSet,
        5, 0},
    {"cpssDxChTtiUserDefinedByteGet",
        &wrCpssDxChTtiUserDefinedByteGet,
        3, 0},

    {"cpssDxChTtiPacketTypeKeySizeSet",
        &wrCpssDxChTtiPacketTypeKeySizeSet,
        3, 0},
    {"cpssDxChTtiPacketTypeKeySizeGet",
        &wrCpssDxChTtiPacketTypeKeySizeGet,
        2, 0},

    {"cpssDxChTtiExceptionCpuCodeSet1",
        &wrCpssDxChTtiExceptionCpuCodeSet,
        3, 0},
    {"cpssDxChTtiExceptionCpuCodeGet1",
        &wrCpssDxChTtiExceptionCpuCodeGet,
        2, 0},
    {"cpssDxChTtiTcamSegmentModeSet",
        &wrCpssDxChTtiParallelLookupEnableSet,
        3, 0},
    {"cpssDxChTtiTcamSegmentModeGet",
        &wrCpssDxChTtiParallelLookupEnableGet,
        2, 0},
    {"cpssDxChTtiTcamSegmentMode_1Set",
        &wrCpssDxChTtiTcamSegmentModeSet,
        3, 0},
    {"cpssDxChTtiTcamSegmentMode_1Get",
        &wrCpssDxChTtiTcamSegmentModeGet,
        2, 0},
    {"cpssDxChTtiMcTunnelDuplicationModeSet",
        &wrCpssDxChTtiMcTunnelDuplicationModeSet,
        4, 0},
    {"cpssDxChTtiMcTunnelDuplicationModeGet",
        &wrCpssDxChTtiMcTunnelDuplicationModeGet,
        3, 0},
    {"cpssDxChTtiMcTunnelDuplicationUdpDestPortSet",
        &wrCpssDxChTtiMcTunnelDuplicationUdpDestPortSet,
        2, 0},
    {"cpssDxChTtiMcTunnelDuplicationUdpDestPortGet",
        &wrCpssDxChTtiMcTunnelDuplicationUdpDestPortGet,
        1, 0},
    {"cpssDxChTtiMplsMcTunnelTriggeringMacDaSet",
        &wrCpssDxChTtiMplsMcTunnelTriggeringMacDaSet,
        3, 0},
    {"cpssDxChTtiMplsMcTunnelTriggeringMacDaGet",
        &wrCpssDxChTtiMplsMcTunnelTriggeringMacDaGet,
        1, 0},
    {"cpssDxChTtiPwCwExceptionCmdSet",
        &wrCpssDxChTtiPwCwExceptionCmdSet,
        3, 0},
    {"cpssDxChTtiPwCwExceptionCmdGet",
        &wrCpssDxChTtiPwCwExceptionCmdGet,
        2, 0},
    {"cpssDxChTtiPwCwCpuCodeBaseSet",
        &wrCpssDxChTtiPwCwCpuCodeBaseSet,
        2, 0},
    {"cpssDxChTtiPwCwCpuCodeBaseGet",
        &wrCpssDxChTtiPwCwCpuCodeBaseGet,
        1, 0},
    {"cpssDxChTtiIPv6ExtensionHeaderSet",
        &wrCpssDxChTtiIPv6ExtensionHeaderSet,
        3, 0},
    {"cpssDxChTtiIPv6ExtensionHeaderGet",
        &wrCpssDxChTtiIPv6ExtensionHeaderGet,
        2, 0},
    {"cpssDxChTtiSourceIdBitsOverrideSet",
        &wrCpssDxChTtiSourceIdBitsOverrideSet,
        3, 0},
    {"cpssDxChTtiSourceIdBitsOverrideGet",
        &wrCpssDxChTtiSourceIdBitsOverrideGet,
        2, 0},
    {"cpssDxChTtiGreExtensionsCheckEnableSet",
        &wrcpssDxChTtiGreExtensionsCheckEnableSet,
        2, 0},
    {"cpssDxChTtiGreExtensionsCheckEnableGet",
        &wrcpssDxChTtiGreExtensionsCheckEnableGet,
        1, 0},
    {"appDemoDxChLion3TcamTtiBaseIndexGet",
        &wrCpssDxChTtiTcamBaseIndexGet,
        2, 0},
    {"appDemoDxChLion3TcamTtiNumOfIndexsGet",
        &wrCpssDxChTtiTcamNumOfIndexsGet,
        2, 0},
    {"cpssDxChTtiPortIpTotalLengthDeductionEnableSet",
        &wrCpssDxChTtiPortIpTotalLengthDeductionEnableSet,
        3, 0},
    {"cpssDxChTtiPortIpTotalLengthDeductionEnableGet",
        &wrCpssDxChTtiPortIpTotalLengthDeductionEnableGet,
        2, 0},
    {"cpssDxChTtiIpTotalLengthDeductionValueSet",
        &wrCpssDxChTtiIpTotalLengthDeductionValueSet,
        3, 0},
    {"cpssDxChTtiIpTotalLengthDeductionValueGet",
        &wrCpssDxChTtiIpTotalLengthDeductionValueGet,
        2, 0},
    {"cpssDxChTtiMultiRuleActionSet",
        &wrCpssDxChTtiMultiRuleActionSet,
        94, 0},
    {"cpssDxChTtiMultiRuleActionGet",
        &wrCpssDxChTtiMultiRuleActionGet,
        1, 0},
    {"cpssDxChTtiMultiEthRuleSet",
        &wrCpssDxChTtiMultiEthRuleSet,
        44, 0},
    {"cpssDxChTtiMultiEthRuleGet",
        &wrCpssDxChTtiMultiEthRuleGet,
        0, 0},
    {"cpssDxChTtiMultiEthRuleDelete",
        &wrCpssDxChTtiMultiEthRuleDelete,
        3, 0},
    {"cpssDxChTtiMultiUdbRuleSet",
        &wrCpssDxChTtiMultiUdbRuleSet,
        65, 0},
    {"cpssDxChTtiMultiUdbRuleGet",
        &wrCpssDxChTtiMultiUdbRuleGet,
        1, 0},
    {"cpssDxChTtiMultiUdbRuleDelete",
        &wrCpssDxChTtiMultiUdbRuleDelete,
        4, 0},

    {"cpssDxChTtiFcoeForwardingEnableSet",
        &wrCpssDxChTtiFcoeForwardingEnableSet,
        2, 0},
    {"cpssDxChTtiFcoeForwardingEnableGet",
        &wrCpssDxChTtiFcoeForwardingEnableGet,
        1, 0},
    {"cpssDxChTtiFcoeEtherTypeSet",
        &wrCpssDxChTtiFcoeEtherTypeSet,
        2, 0},
    {"cpssDxChTtiFcoeEtherTypeGet",
        &wrCpssDxChTtiFcoeEtherTypeGet,
        1, 0},
    {"cpssDxChTtiFcoeExceptionConfiguratiosEnableSet",
    &wrCpssDxChTtiFcoeExceptionConfiguratiosEnableSet,
    3, 0},
    {"cpssDxChTtiFcoeExceptionConfiguratiosEnableGet",
    &wrCpssDxChTtiFcoeExceptionConfiguratiosEnableGet,
    2, 0},
    {"cpssDxChTtiFcoeExceptionCountersGet",
    &wrCpssDxChTtiFcoeExceptionCountersGet,
    2, 0},
    {"cpssDxChTtiFcoeAssignVfIdEnableSet",
    &wrCpssDxChTtiFcoeAssignVfIdEnableSet,
    3, 0},
    {"cpssDxChTtiFcoeAssignVfIdEnableGet",
    &wrCpssDxChTtiFcoeAssignVfIdEnableGet,
    2, 0},
    {"cpssDxChTtiFcoeExceptionPacketCommandSet",
    &wrCpssDxChTtiFcoeExceptionPacketCommandSet,
    2, 0},
    {"cpssDxChTtiFcoeExceptionPacketCommandGet",
    &wrCpssDxChTtiFcoeExceptionPacketCommandGet,
    1, 0},
    {"cpssDxChTtiFcoeExceptionCpuCodeSet",
    &wrCpssDxChTtiFcoeExceptionCpuCodeSet,
    2, 0},
    {"cpssDxChTtiFcoeExceptionCpuCodeGet",
    &wrCpssDxChTtiFcoeExceptionCpuCodeGet,
    1, 0},
    {"cpssDxChTtiEcnPacketCommandSet",
        &wrCpssDxChTtiEcnPacketCommandSet,
        2, 0},
    {"cpssDxChTtiEcnPacketCommandGet",
        &wrCpssDxChTtiEcnPacketCommandGet,
        1, 0},
    {"cpssDxChTtiEcnCpuCodeSet",
        &wrCpssDxChTtiEcnCpuCodeSet,
        2, 0},
    {"cpssDxChTtiEcnCpuCodeGet",
        &wrCpssDxChTtiEcnCpuCodeGet,
        1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibResetCpssDxChTti function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetCpssDxChTti
(
    GT_VOID
)
{
    macToMeTableIndex = 0;
    firstFree = 0;
    ttiActionGetIndex = 0;
    cmdOsMemSet(ttiActionDatabase , 0, sizeof(ttiActionDatabase));
    cmdOsMemSet(&ethRuleMaskData , 0, sizeof(ethRuleMaskData));
    cmdOsMemSet(&ethRulePatternData , 0, sizeof(ethRulePatternData));
    cmdOsMemSet(&ttiMultiActionDatabaseParamsForEthRule , 0, sizeof(ttiMultiActionDatabaseParamsForEthRule));
    cmdOsMemSet(udbRuleMaskDataArray , 0, sizeof(udbRuleMaskDataArray));
    cmdOsMemSet(udbRulePatternDataArray , 0, sizeof(udbRulePatternDataArray));
    cmdOsMemSet(ttiMultiActionDatabaseParamsForUdbRuleArray , 0, sizeof(ttiMultiActionDatabaseParamsForUdbRuleArray));
    firstRun = GT_TRUE;
    mask_set = GT_FALSE;    /* is mask set   */
    pattern_set = GT_FALSE; /* is pattern set*/
    mask_ruleIndex = 0;
    pattern_ruleIndex = 0;
    cmdOsMemSet(&maskData , 0, sizeof(maskData));
    cmdOsMemSet(&patternData, 0, sizeof(patternData));
    ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
    cmdOsMemSet(ttiRuleFormat, 0, sizeof(ttiRuleFormat));
    
    ruleIndex = 0;
    ruleIndexMaxGet = 0;
    isMask = GT_FALSE;
}

/**
* @internal cmdLibInitCpssDxChTti function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxChTti
(
GT_VOID
)
{
    GT_STATUS rc;

    rc = wrCpssRegisterResetCb(cmdLibResetCpssDxChTti);
    if(rc != GT_OK)
    {
        return rc;
    }

    return cmdInitLibrary(dbCommands, numCommands);
}

