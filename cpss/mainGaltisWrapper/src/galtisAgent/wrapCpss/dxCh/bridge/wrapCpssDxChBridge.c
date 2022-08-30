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

/*******************************************************************************
* wrapBridgeCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Bridge cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 33 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Dlb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>

#if 0
/**
* @internal wrCpssDxChBrgMacHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the mac address table.
*         for specific mac address and VLAN id.
*
* @note   APPLICABLE DEVICES:      All DxCh device
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgMacHashCalc

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_ETHERADDR    addrPtr;
    GT_U16          vid;
    GT_U32          hashPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    galtisMacAddr(&addrPtr, (GT_U8*)inArgs[1]);
    vid = (GT_U16)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgMacHashCalc(devNum, &addrPtr, vid, &hashPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", hashPtr);
    return CMD_OK;
}
#endif /*0*/



/**
* @internal wrCpssDxChBrgStpInit function
* @endinternal
*
* @brief   Initialize STP library.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChBrgStpInit

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgStpInit(dev);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgStpStateSet function
* @endinternal
*
* @brief   Sets STP state of port belonging within an STP group.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port or stpId or state
*/
static CMD_STATUS wrCpssDxChBrgStpStateSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM                         port;
    GT_U16                        stpId;
    CPSS_STP_STATE_ENT            state;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stpId = (GT_U16)inArgs[2];
    state = (CPSS_STP_STATE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgStpStateSet(dev, port, stpId, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgStpStateGet function
* @endinternal
*
* @brief   Gets STP state of port belonging within an STP group.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port or stpId or state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgStpStateGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM                         port;
    GT_U16                        stpId;
    CPSS_STP_STATE_ENT            state;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stpId = (GT_U16)inArgs[2];
    state = (CPSS_STP_STATE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgStpStateGet(dev, port, stpId, &state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", state);

    return CMD_OK;
}

/*cpssDxChBrgStpEntry Table*/
/*
* cpssDxChBrgStpEntryGet
*
* DESCRIPTION:
*       Gets the RAW STP entry from HW.
*
* APPLICABLE DEVICES:  All DxCh devices
*
* INPUTS:
*       dev    - device number
*       stpId  - STG (Spanning Tree Group) index  (0..255)
*
* OUTPUTS:
*       stpEntryPtr     - (Pointer to) The raw stp entry to get.
*                         memory allocated by the caller.
*
* RETURNS:
*       GT_OK       - on success
*       GT_FAIL     - on error.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM- on bad device or stpId
*       GT_BAD_PTR  - on NULL pointer parameter
*
* COMMENTS:
*
*******************************************************************************/

static GT_U16   stpIdCnt;
static GT_BOOL  stpGetOnlyNotZeroEntries = GT_FALSE;

void wrCpssDxChBrgStpEntryGetOnlyNotZeroEntries
(
    IN GT_BOOL getOnlyNotZeroEntries
)
{
    stpGetOnlyNotZeroEntries = getOnlyNotZeroEntries;
}

static GT_STATUS utilCpssDxChBrgStpEntryGetNext
(
    IN  GT_U8  dev,
    OUT GT_U32 stpEntryPtr[CPSS_DXCH_STG_ENTRY_SIZE_CNS]
)
{
    GT_STATUS      result;
    GT_U32         maxStpId;
    GT_U32         stpZeroArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS];

    /* initial increment */
    stpIdCnt = ((stpIdCnt == 0xFFFF) ? 0 : (stpIdCnt + 1));

    cpssOsMemSet(stpZeroArr, 0, sizeof(stpZeroArr));

    if ((0 == PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
         || (0 == PRV_CPSS_DXCH_FAMILY_CHECK_MAC(dev)))
    {
        cpssOsPrintf("wrong device number specified: 0x%X\n", dev);
        return GT_BAD_PARAM;
    }

    maxStpId = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum;

    while (1)
    {
        if (stpIdCnt >= maxStpId)
        {
            return GT_NO_MORE;
        }
        result = cpssDxChBrgStpEntryGet(dev, stpIdCnt, stpEntryPtr);
        if (result != GT_OK)
        {
            return result;
        }
        if (stpGetOnlyNotZeroEntries == GT_FALSE)
        {
            return GT_OK;
        }
        if (0 != cpssOsMemCmp(stpZeroArr, stpEntryPtr, sizeof(stpZeroArr)))
        {
            return GT_OK;
        }
        stpIdCnt ++ ;
    }
}

/******************************************************************************/

static CMD_STATUS wrCpssDxChBrgStpEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS      result, res1;
    GT_U8          dev, i;
    GT_U32         stpEntryPtr[CPSS_DXCH_STG_ENTRY_SIZE_CNS];

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    result = utilCpssDxChBrgStpEntryGetNext(dev, stpEntryPtr);
    res1 = (result == GT_NO_MORE) ? GT_OK : result;
    if (result != GT_OK)
    {
        galtisOutput(outArgs, res1, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = stpIdCnt;

    for(i = 0; i < CPSS_DXCH_STG_ENTRY_SIZE_CNS; i++)
    {
        inFields[i + 1] = stpEntryPtr[i];
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

static CMD_STATUS wrCpssDxChBrgStpEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* initial value */
    stpIdCnt = 0xFFFF;

    return wrCpssDxChBrgStpEntryGetNext(
        inArgs, inFields, numFields, outArgs);
}



/**
* @internal wrCpssDxChBrgStpEntryWrite function
* @endinternal
*
* @brief   Writes a raw STP entry to HW.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or stpId
* @retval GT_BAD_PTR               - on NULL pointer parameter
*/
static CMD_STATUS wrCpssDxChBrgStpEntryWrite

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;

    GT_U8         dev, i;
    GT_U16        stpId;
    GT_U32        stpEntryPtr[CPSS_DXCH_STG_ENTRY_SIZE_CNS];

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];


    stpId = (GT_U16)inFields[0];

    for(i = 0; i < CPSS_DXCH_STG_ENTRY_SIZE_CNS; i++)
    {
        stpEntryPtr[i] = (GT_U32)inFields[i + 1];
    }

    /* call cpss api function */
    result = cpssDxChBrgStpEntryWrite(dev, stpId, stpEntryPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}



/**
* @internal wrCpssDxChBrgPortEgrFltUnkEnable function
* @endinternal
*
* @brief   If enabled, bridged unknown unicast packets are filtered on the
*         specified egress port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltUnkEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgPortEgrFltUregMcastEnable function
* @endinternal
*
* @brief   If enabled, bridged unregistered multicast packets are filtered on the
*         specified egress port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltUregMcastEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/disable VLAN Egress Filtering on bridged known unicast packets
*         on specified device.
*         When enabled, VLAN egress filtering verifies that the egress port is a
*         member of the packet's VLAN classification
*         When disabled, the VLAN egress filtering check is disabled.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
*
* @note VLAN egress filtering is required by 802.1Q, but if desired, this
*       mechanism can be disabled, thus allowing "leaky VLANs".
*
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressFilteringEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressFilteringEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgRoutedUnicastEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable vlan egress filtering on routed packets
*         The destination port may or may not be a member of the vlan
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
*/
static CMD_STATUS wrCpssDxChBrgRoutedUnicastEgressFilteringEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgRoutedUnicastEgressFilteringEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgRoutedSpanEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable STP egress Filtering on Routed Packets
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
*/
static CMD_STATUS wrCpssDxChBrgRoutedSpanEgressFilteringEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgRoutedSpanEgressFilteringEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgPortEgrFltUregBcEnable function
* @endinternal
*
* @brief   Enables or disables egress filtering of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_SUPPORTED         - This request is not supported.
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltUregBcEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgPortEgressMcastLocalEnable function
* @endinternal
*
* @brief   Enables filtering unregistered multicast packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
*/
static CMD_STATUS wrCpssDxChBrgPortEgressMcastLocalEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_BOOL  enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgMcInit function
* @endinternal
*
* @brief   Initializes Bridge engine Multicast Library.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
static CMD_STATUS wrCpssDxChBrgMcInit

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgMcInit(dev);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgMcIpv6BytesSelectSet function
* @endinternal
*
* @brief   Sets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
static CMD_STATUS wrCpssDxChBrgMcIpv6BytesSelectSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;

    GT_U8         devNum, i;
    GT_U8         dipBytesSelectMap[4];
    GT_U8         sipBytesSelectMap[4];

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    for(i = 0; i < 4; i++)
    {
        dipBytesSelectMap[i] = (GT_U8)inFields[i];
    }

    for(i = 0; i < 4; i++)
    {
        sipBytesSelectMap[i] = (GT_U8)inFields[i + 4];
    }

    /* call cpss api function */
    result = cpssDxChBrgMcIpv6BytesSelectSet(devNum, dipBytesSelectMap,
                                                     sipBytesSelectMap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgMcIpv6BytesSelectGetFirst function
* @endinternal
*
* @brief   Gets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
static CMD_STATUS wrCpssDxChBrgMcIpv6BytesSelectGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;

    GT_U8         devNum, i;
    GT_U8         dipBytesSelectMap[4];
    GT_U8         sipBytesSelectMap[4];

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgMcIpv6BytesSelectGet(devNum, dipBytesSelectMap,
                                                     sipBytesSelectMap);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    for(i = 0; i < 4; i++)
    {
        inFields[i] = dipBytesSelectMap[i];
    }

    for(i = 0; i < 4; i++)
    {
        inFields[i + 4] = sipBytesSelectMap[i];
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1],
                       inFields[2], inFields[3], inFields[4],
                       inFields[5], inFields[6], inFields[7]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/

static CMD_STATUS wrCpssDxChBrgMcIpv6BytesSelectGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgMcV1EntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or ports bitmap value.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*
* @note The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*/
static CMD_STATUS wrCpssDxChBrgMcV1EntryWrite

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    GT_U16                    vidx;
    CPSS_PORTS_BMP_STC        portBitmap;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    vidx = (GT_U16)inFields[0];

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    portBitmap.ports[0] = (GT_U32)inFields[1];
    portBitmap.ports[1] = (GT_U32)inFields[2];
    portBitmap.ports[2] = (GT_U32)inFields[3];
    portBitmap.ports[3] = (GT_U32)inFields[4];
#if CPSS_MAX_PORTS_NUM_CNS > 128
    portBitmap.ports[4] = (GT_U32)inFields[5];
    portBitmap.ports[5] = (GT_U32)inFields[6];
    portBitmap.ports[6] = (GT_U32)inFields[7];
    portBitmap.ports[7] = (GT_U32)inFields[8];
#endif
    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portBitmap);

    /* call cpss api function */
    result = cpssDxChBrgMcEntryWrite(devNum, vidx, &portBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/*
* cpssDxChBrgMcEntryRead
*
* DESCRIPTION:
*       Reads the Multicast Group entry from the HW.
*
* APPLICABLE DEVICES: ALL DxCh Devices.
*
* INPUTS:
*       devNum          - PP's device number.
*       vidx            - multicast group index (VIDX<=4095).
*
* OUTPUTS:
*       portBitmapPtr   - pointer to the bitmap of ports are belonged to the
*                         vidx MC group of specified PP device.
*
* RETURNS:
*       GT_OK               - on success.
*       GT_BAD_PTR          - on NULL pointer.
*       GT_HW_ERROR         - on hardware error.
*       GT_BAD_PARAM        - on wrong devNum.
*       GT_OUT_OF_RANGE     - if vidx is larger than the allowed value.
*
* COMMENTS:
*/

static GT_U16   vidxIndexV1;
static GT_U16   vidxMaxGetV1;

static CMD_STATUS wrCpssDxChBrgMcEntryReadV1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                     devNum;
    GT_STATUS                 result;
    CPSS_PORTS_BMP_STC        portBitmap;

    GT_UNUSED_PARAM(numFields);

    devNum = (GT_U8)inArgs[0];


    /* call cpss api function */
    result = cpssDxChBrgMcEntryRead(devNum, vidxIndexV1, &portBitmap);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portBitmap);
    inFields[0] = vidxIndexV1;
    inFields[1] = portBitmap.ports[0];
    inFields[2] = portBitmap.ports[1];
    inFields[3] = portBitmap.ports[2];
    inFields[4] = portBitmap.ports[3];
#if CPSS_MAX_PORTS_NUM_CNS > 128
    inFields[5] = portBitmap.ports[4];
    inFields[6] = portBitmap.ports[5];
    inFields[7] = portBitmap.ports[6];
    inFields[8] = portBitmap.ports[7];
#endif

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                    inFields[3],inFields[4], inFields[5],
                                    inFields[6], inFields[7], inFields[8]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgMcEntryReadV1First

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U16                    tmpEntryGet;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    vidxIndexV1 = (GT_U16)inArgs[1];
    tmpEntryGet = (GT_U16)inArgs[2];

    vidxMaxGetV1 = vidxIndexV1 + tmpEntryGet;

    return wrCpssDxChBrgMcEntryReadV1(inArgs,inFields,numFields,outArgs);

}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgMcEntryReadV1Next

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                     devNum;
    GT_U32                    maxVidxIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    vidxIndexV1++;

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        maxVidxIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[CPSS_DXCH_TABLE_MULTICAST_E].maxNumOfEntries;
    }
    else
    {
        maxVidxIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vidxNum;
    }

    if(vidxIndexV1 > maxVidxIndex || vidxIndexV1 >= vidxMaxGetV1)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    return wrCpssDxChBrgMcEntryReadV1(inArgs,inFields,numFields,outArgs);

}

/**
* @internal wrCpssDxChBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or ports bitmap value.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*
* @note The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*/
static CMD_STATUS wrCpssDxChBrgMcEntryWrite

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    GT_U16                    vidx;
    CPSS_PORTS_BMP_STC        portBitmap;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    vidx = (GT_U16)inFields[0];

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    portBitmap.ports[0] = (GT_U32)inFields[1];
    portBitmap.ports[1] = (GT_U32)inFields[2];
    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portBitmap);

    /* call cpss api function */
    result = cpssDxChBrgMcEntryWrite(devNum, vidx, &portBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/*
* cpssDxChBrgMcEntryRead
*
* DESCRIPTION:
*       Reads the Multicast Group entry from the HW.
*
* APPLICABLE DEVICES: ALL DxCh Devices.
*
* INPUTS:
*       devNum          - PP's device number.
*       vidx            - multicast group index (VIDX<=4095).
*
* OUTPUTS:
*       portBitmapPtr   - pointer to the bitmap of ports are belonged to the
*                         vidx MC group of specified PP device.
*
* RETURNS:
*       GT_OK               - on success.
*       GT_BAD_PTR          - on NULL pointer.
*       GT_HW_ERROR         - on hardware error.
*       GT_BAD_PARAM        - on wrong devNum.
*       GT_OUT_OF_RANGE     - if vidx is larger than the allowed value.
*
* COMMENTS:
*/

static GT_U16   vidxIndex;
static GT_U16   vidxMaxGet;

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgMcEntryReadFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    GT_U16                    tmpEntryGet;
    CPSS_PORTS_BMP_STC        portBitmap;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vidxIndex = (GT_U16)inArgs[1];
    tmpEntryGet = (GT_U16)inArgs[2];

    vidxMaxGet = vidxIndex + tmpEntryGet;

    /* call cpss api function */
    result = cpssDxChBrgMcEntryRead(devNum, vidxIndex, &portBitmap);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portBitmap);
    inFields[0] = vidxIndex;
    inFields[1] = portBitmap.ports[0];
    inFields[2] = portBitmap.ports[1];


    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgMcEntryReadNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    CPSS_PORTS_BMP_STC        portBitmap;
    GT_U32                    maxVidxIndex;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vidxIndex++;

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        maxVidxIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[CPSS_DXCH_TABLE_MULTICAST_E].maxNumOfEntries;
    }
    else
    {
        maxVidxIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vidxNum;
    }

    if(vidxIndex > maxVidxIndex || vidxIndex >= vidxMaxGet)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgMcEntryRead(devNum, vidxIndex, &portBitmap);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portBitmap);
    inFields[0] = vidxIndex;
    inFields[1] = portBitmap.ports[0];
    inFields[2] = portBitmap.ports[1];


    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgMcGroupDelete function
* @endinternal
*
* @brief   Deletes the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*/
static CMD_STATUS wrCpssDxChBrgMcGroupDelete

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_U16   vidx;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    vidx = (GT_U16)inFields[0];

    /* call cpss api function */
    result = cpssDxChBrgMcGroupDelete(devNum, vidx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgMcMemberAdd function
* @endinternal
*
* @brief   Add new port member to the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*/
static CMD_STATUS wrCpssDxChBrgMcMemberAdd

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_U16   vidx;
    GT_PHYSICAL_PORT_NUM    portNum;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vidx = (GT_U16)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgMcMemberAdd(devNum, vidx, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgMcMemberDelete function
* @endinternal
*
* @brief   Delete port member from the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*/
static CMD_STATUS wrCpssDxChBrgMcMemberDelete

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_U16   vidx;
    GT_PHYSICAL_PORT_NUM    portNum;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vidx = (GT_U16)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgMcMemberDelete(devNum, vidx, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgNestVlanAccessPortSet function
* @endinternal
*
* @brief   Configure given port as nested VLAN access port.
*         The VID of all the packets received on nested VLAN access port is
*         discarded and they are assigned with the Port VID that set by
*         cpssDxChBrgVlanPortVidSet(). This VLAN assignment may be subsequently
*         overridden by the protocol based VLANs or policy based VLANs algorithms.
*         When a packet received on an access port is transmitted via a core port
*         or a cascading port, a VLAN tag is added to the packet, in addition to
*         any existing VLAN tag.
*         The 802.1p User Priority field of this added tag may be one of the
*         following, based on the ModifyUP QoS parameter set to the packet at
*         the end of the Ingress pipe:
*         - If ModifyUP is "Modify", it is the UP extracted from the
*         QoS Profile table entry that configured by the
*         cpssDxChCosProfileEntrySet().
*         - If ModifyUP is "Not Modify", it is the original packet
*         802.1p User Priority field, if it is tagged. If untagged,
*         it is ingress port's default user priority that configured by
*         cpssDxChPortDefaultUPSet().
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
*
* @note To complete Access Port Configuration do the following:
*       - by API cpssDxChBrgNestVlanEtherTypeSelectSet(), set
*       the Port VLAN Selection to the CPSS_VLAN_ETHERTYPE0_E;
*       - update egress Port VLAN EtherType;
*       - by API cpssDxChBrgVlanForcePvidEnable(), set
*       Port VID Assignment mode.
*       To complete Core Port Configuration, select CPSS_VLAN_ETHERTYPE1_E
*       for Usual Port.
*       To complete Customers Bridges(bridges that don't employ nested VLANs)
*       Configuration, select CPSS_VLAN_ETHERTYPE0_E for Usual Port.
*
*/
static CMD_STATUS wrCpssDxChBrgNestVlanAccessPortSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;

    GT_U8        devNum;
    GT_PORT_NUM        portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgNestVlanAccessPortSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgNestVlanAccessPortGet function
* @endinternal
*
* @brief   Gets configuration of the given port (Nested VLAN access port
*         or Core/Customer port).
*         The VID of all the packets received on Nested VLAN access port is
*         discarded and they are assigned with the Port VID that set by
*         cpssDxChBrgVlanPortVidSet(). This VLAN assignment may be subsequently
*         overridden by the protocol based VLANs or policy based VLANs algorithms.
*         When a packet received on an access port is transmitted via a core port
*         or a cascading port, a VLAN tag is added to the packet, in addition to
*         any existing VLAN tag.
*         The 802.1p User Priority field of this added tag may be one of the
*         following, based on the ModifyUP QoS parameter set to the packet at
*         the end of the Ingress pipe:
*         - If ModifyUP is "Modify", it is the UP extracted from the
*         QoS Profile table entry that configured by the
*         cpssDxChCosProfileEntrySet().
*         - If ModifyUP is "Not Modify", it is the original packet
*         802.1p User Priority field, if it is tagged. If untagged,
*         it is ingress port's default user priority that configured by
*         cpssDxChPortDefaultUPSet().
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum values.
*/
static CMD_STATUS wrCpssDxChBrgNestVlanAccessPortGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;

    GT_U8        devNum;
    GT_PORT_NUM        portNum;
    GT_BOOL      enablePtr;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgNestVlanAccessPortGet(devNum, portNum, &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPortEgrFltUnkEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for bridged Unknown Unicast packets on the specified egress port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltUnkEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgPortEgrFltUregMcastEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for bridged Unregistered Multicast packets on the specified egress port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltUregMcastEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the VLAN Egress Filtering current state (enable/disable)
*         on specified device for Bridged Known Unicast packets.
*         If enabled the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressFilteringEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgRoutedUnicastEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the VLAN Egress Filtering current state (enable/disable)
*         on Routed Unicast packets.
*         If disabled, the destination port may or may not be a member of the VLAN.
*         If enabled, the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgRoutedUnicastEgressFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgRoutedSpanEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the STP egress Filtering current state (enable/disable)
*         on Routed packets.
*         If disabled the packet may be forwarded to its egress port,
*         regardless of its Span State.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgRoutedSpanEgressFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgPortEgrFltUregBcEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltUregBcEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgPortEgressMcastLocalEnableGet function
* @endinternal
*
* @brief   This function gets current state (enable/disable) for sending
*         Multicast packets back to its source port on the local device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgPortEgressMcastLocalEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPortEgrFltIpMcRoutedEnable function
* @endinternal
*
* @brief   Enable/Disable egress filtering for IP Multicast Routed packets
*         on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltIpMcRoutedEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgPortEgrFltIpMcRoutedEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for IP Multicast Routed packets on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgPortEgrFltIpMcRoutedEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PORT_NUM port;
    GT_BOOL enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev,port);


    /* call cpss api function */
    result = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/*******************************************************************************
* cpssDxChBrgEportToPhysicalPortTargetMappingTableGet
*
* DESCRIPTION:
*       Get physical info for the given ePort in the E2PHY mapping table.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - device number
*       portNum         - port number
*
* OUTPUTS:
*       physicalInfoPtr - (pointer to) physical Information related to the ePort
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       E2PHY table is accessed by the packet's target ePort
*
*******************************************************************************/
static GT_PORT_NUM brgEportToPhyePort;  /* current ePort to read PHY info */
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                     devNum;
    GT_STATUS                 result;
    CPSS_INTERFACE_INFO_STC   physicalInfo;
    GT_U32                    mtuProfileIdx;

    GT_UNUSED_PARAM(numFields);

    devNum = (GT_U8)inArgs[0];

    cmdOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    /* call cpss api function */
    result = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, brgEportToPhyePort, &physicalInfo);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* call MTU related cpss api function */
    result = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, brgEportToPhyePort, &mtuProfileIdx);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (physicalInfo.type == CPSS_INTERFACE_PORT_E)
    {
        CONVERT_BACK_DEV_PHYSICAL_PORT_DATA_MAC(physicalInfo.devPort.hwDevNum, physicalInfo.devPort.portNum);
    }

    inFields[0] = brgEportToPhyePort;
    inFields[1] = physicalInfo.type;
    inFields[2] = physicalInfo.devPort.hwDevNum;
    inFields[3] = physicalInfo.devPort.portNum;
    inFields[4] = physicalInfo.trunkId;
    inFields[5] = physicalInfo.vidx;
    inFields[6] = mtuProfileIdx;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                    inFields[3],inFields[4], inFields[5],
                                    inFields[6]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(inArgs[2] == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    brgEportToPhyePort = (GT_PORT_NUM)inArgs[1];

    return wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGet(inArgs,inFields,numFields,outArgs);

}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  startEport;
    GT_U32  numOfEports;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    startEport = (GT_U32)inArgs[1];
    numOfEports = (GT_U32)inArgs[2];

    brgEportToPhyePort++;

    if(brgEportToPhyePort >= (startEport + numOfEports))
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    return wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChBrgEportToPhysicalPortTargetMappingTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortTargetMappingTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                     devNum;
    GT_STATUS                 result;
    CPSS_INTERFACE_INFO_STC   physicalInfo;
    GT_PORT_NUM               ePort;
    GT_U32                    mtuProfileIdx;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ePort = (GT_PORT_NUM)inFields[0];

    cmdOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    physicalInfo.type              = (CPSS_INTERFACE_TYPE_ENT)inFields[1];
    physicalInfo.devPort.hwDevNum    = (GT_HW_DEV_NUM)inFields[2];
    physicalInfo.devPort.portNum   = (GT_PORT_NUM)inFields[3];
    physicalInfo.trunkId           = (GT_TRUNK_ID)inFields[4];
    physicalInfo.vidx              = (GT_U16)inFields[5];
    mtuProfileIdx                  = (GT_U32)inFields[6];

    if (physicalInfo.type == CPSS_INTERFACE_PORT_E)
    {
        /* Override Uplink Device and Port */
        CONVERT_DEV_PHYSICAL_PORT_DATA_MAC(physicalInfo.devPort.hwDevNum, physicalInfo.devPort.portNum);
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, ePort);

    /* call cpss api function */
    result = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, ePort, &physicalInfo);
    if (result != GT_OK) {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* call to mtu related API function */
    result = cpssDxChBrgGenMtuPortProfileIdxSet(devNum, ePort, mtuProfileIdx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEportToPhysicalPortEgressPortTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                devNum;
    GT_STATUS                            result=GT_OK;
    GT_PORT_NUM                          ePort;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   egressInfo;
    GT_BOOL                              forceArpTsPortDecision;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ePort  = (GT_PORT_NUM)inFields[0];

    cmdOsMemSet(&egressInfo, 0, sizeof(egressInfo));

    forceArpTsPortDecision              = (GT_BOOL)inFields[1];

    egressInfo.forceArpTsPortDecision   = forceArpTsPortDecision;
    egressInfo.tunnelStart              = (GT_BOOL)inFields[2];
    egressInfo.tunnelStartPtr           = (GT_U32)inFields[3];
    egressInfo.tsPassengerPacketType    = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[4];
    egressInfo.arpPtr                   = (GT_U32)inFields[5];
    egressInfo.modifyMacSa              = (GT_BOOL)inFields[6];
    egressInfo.modifyMacDa              = (GT_BOOL)inFields[7];

    /* call cpss api function */
    result = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(
        devNum, ePort, &egressInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL getFirst /* GT_TRUE - first, GT_FALSE - next */
)
{
    static GT_PORT_NUM                   ePort = 0;
    static GT_U32                        ePortAmount = 0;
    GT_U8                                devNum;
    GT_STATUS                            result;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   egressInfo;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    if (getFirst != GT_FALSE)
    {
        ePort        = (GT_PORT_NUM)inArgs[1];
        ePortAmount  = (GT_U32)inArgs[2];
    }
    else
    {
        ePort ++;
        ePortAmount --;
    }

    if (ePortAmount == 0)
    {
        /* no more entries */
        galtisOutput(outArgs, GT_OK, "%d", 0xFFFFFFFF);
        return CMD_OK;
    }

    cmdOsMemSet(&egressInfo, 0, sizeof(egressInfo));

    /* call cpss api function */
    result = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(
        devNum, ePort, &egressInfo);

    fieldOutput(
        "%d%d%d%d%d%d%d%d", ePort,
        egressInfo.forceArpTsPortDecision, egressInfo.tunnelStart,
        egressInfo.tunnelStartPtr, egressInfo.tsPassengerPacketType,
        egressInfo.arpPtr, egressInfo.modifyMacSa,  egressInfo.modifyMacDa);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE /*getFirst*/);
}

static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE /*getFirst*/);
}



/**
* @internal wrCpssDxChBrgEportToPhysicalPortEgressPortTable2Set function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTable2Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                devNum;
    GT_STATUS                            result;
    GT_PORT_NUM                          ePort;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   egressInfo;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ePort  = (GT_PORT_NUM)inFields[0];

    cmdOsMemSet(&egressInfo, 0, sizeof(egressInfo));

    egressInfo.tunnelStart              = (GT_BOOL)inFields[1];
    egressInfo.tunnelStartPtr           = (GT_U32)inFields[1];
    egressInfo.tsPassengerPacketType    = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[3];
    egressInfo.arpPtr                   = (GT_U32)inFields[4];
    egressInfo.modifyMacSa              = (GT_BOOL)inFields[5];
    egressInfo.modifyMacDa              = (GT_BOOL)inFields[6];

    /* call cpss api function */
    result = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(
        devNum, ePort, &egressInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util2 function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL getFirst /* GT_TRUE - first, GT_FALSE - next */
)
{
    static GT_PORT_NUM                   ePort = 0;
    static GT_U32                        ePortAmount = 0;
    GT_U8                                devNum;
    GT_STATUS                            result;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   egressInfo;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    if (getFirst != GT_FALSE)
    {
        ePort        = (GT_PORT_NUM)inArgs[1];
        ePortAmount  = (GT_U32)inArgs[2];
    }
    else
    {
        ePort ++;
        ePortAmount --;
    }

    if (ePortAmount == 0)
    {
        /* no more entries */
        galtisOutput(outArgs, GT_OK, "%d", 0xFFFFFFFF);
        return CMD_OK;
    }

    cmdOsMemSet(&egressInfo, 0, sizeof(egressInfo));

    /* call cpss api function */
    result = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(
        devNum, ePort, &egressInfo);

    fieldOutput(
        "%d%d%d%d%d%d%d", ePort,
        egressInfo.tunnelStart,
        egressInfo.tunnelStartPtr, egressInfo.tsPassengerPacketType,
        egressInfo.arpPtr, egressInfo.modifyMacSa,  egressInfo.modifyMacDa);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTable2GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util2(
        inArgs, inFields, numFields, outArgs, GT_TRUE /*getFirst*/);
}

static CMD_STATUS wrCpssDxChBrgEportToPhysicalPortEgressPortTable2GetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgEportToPhysicalPortEgressPortTableGet_util2(
        inArgs, inFields, numFields, outArgs, GT_FALSE /*getFirst*/);
}


/**
* @internal wrCpssDxChBrgL2EcmpEnableSet function
* @endinternal
*
* @brief   Globally enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2EcmpEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d" , enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgL2EcmpIndexBaseEportSet function
* @endinternal
*
* @brief   Defines the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
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
static CMD_STATUS wrCpssDxChBrgL2EcmpIndexBaseEportSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_PORT_NUM    ecmpIndexBaseEport;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    ecmpIndexBaseEport  = (GT_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpIndexBaseEportSet(
        devNum, ecmpIndexBaseEport);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2EcmpIndexBaseEportGet function
* @endinternal
*
* @brief   Return the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
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
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpIndexBaseEportGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_PORT_NUM    ecmpIndexBaseEport;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpIndexBaseEportGet(
        devNum, &ecmpIndexBaseEport);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ecmpIndexBaseEport);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2EcmpLttTableSet function
* @endinternal
*
* @brief   Set L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of InDesc<TRGePort>-<Base ePort>.
*
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpLttTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                devNum;
    GT_STATUS                            result;
    GT_U32                               index;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  ecmpLttInfo;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    index                       = (GT_U32)inFields[0];
    ecmpLttInfo.ecmpStartIndex  = (GT_U32)inFields[1];
    ecmpLttInfo.ecmpNumOfPaths  = (GT_U32)inFields[2];
    ecmpLttInfo.ecmpEnable      = (GT_BOOL)inFields[3];
    ecmpLttInfo.ecmpRandomPathEnable = GT_FALSE;

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpLttTableSet(
        devNum, index, &ecmpLttInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2EcmpLttTableGet_util function
* @endinternal
*
* @brief   Get L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of InDesc<TRGePort>-<Base ePort>.
*
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpLttTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    GT_BOOL  getFirst /* GT_TRUE - first, GT_FALSE - next */
)
{
    static GT_U32                        index = 0;
    static GT_U32                        indexAmount = 0;
    GT_U8                                devNum;
    GT_STATUS                            result;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  ecmpLttInfo;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    if (getFirst != GT_FALSE)
    {
        index        = (GT_U32)inArgs[1];
        indexAmount  = (GT_U32)inArgs[2];
    }
    else
    {
        index ++;
        indexAmount --;
    }

    if (indexAmount == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", 0xFFFFFFFF);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpLttTableGet(
        devNum, index, &ecmpLttInfo);

    fieldOutput(
        "%d%d%d%d", index, ecmpLttInfo.ecmpStartIndex,
        ecmpLttInfo.ecmpNumOfPaths,  ecmpLttInfo.ecmpEnable);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChBrgL2EcmpLttTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgL2EcmpLttTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE /*getFirst*/);
}

static CMD_STATUS wrCpssDxChBrgL2EcmpLttTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgL2EcmpLttTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE /*getFirst*/);
}

/**
* @internal wrCpssDxChBrgL2EcmpTableSet function
* @endinternal
*
* @brief   Set L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                devNum;
    GT_STATUS                            result;
    GT_U32                               index;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC      ecmpEntry;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                  = (GT_U8)inArgs[0];
    index                   = (GT_U32)inFields[0];
    ecmpEntry.targetEport   = (GT_U32)inFields[1];
    ecmpEntry.targetHwDevice  = (GT_U32)inFields[2];

    /* convert device and port */
    CONVERT_DEV_PORT_DATA_MAC(ecmpEntry.targetHwDevice,ecmpEntry.targetEport);

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpTableSet(
        devNum, index, &ecmpEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2EcmpTableGet_util function
* @endinternal
*
* @brief   Get L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    GT_BOOL  getFirst /* GT_TRUE - first, GT_FALSE - next */
)
{
    static GT_U32                        index = 0;
    static GT_U32                        indexAmount = 0;
    GT_U8                                devNum;
    GT_STATUS                            result;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC      ecmpEntry;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    if (getFirst != GT_FALSE)
    {
        index        = (GT_U32)inArgs[1];
        indexAmount  = (GT_U32)inArgs[2];
    }
    else
    {
        index ++;
        indexAmount --;
    }

    if (indexAmount == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", 0xFFFFFFFF);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpTableGet(
        devNum, index, &ecmpEntry);

    /* convert device and port */
    CONVERT_BACK_DEV_PORT_DATA_MAC(ecmpEntry.targetHwDevice,ecmpEntry.targetEport);

    fieldOutput(
        "%d%d%d", index, ecmpEntry.targetEport, ecmpEntry.targetHwDevice);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChBrgL2EcmpTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgL2EcmpTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE /*getFirst*/);
}

static CMD_STATUS wrCpssDxChBrgL2EcmpTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChBrgL2EcmpTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE /*getFirst*/);
}

/**
* @internal wrCpssDxChBrgL2EcmpMemberSelectionModeSet function
* @endinternal
*
* @brief   Set mode used to calculate the Index of the secondary ePort (L2 ECMP member)
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
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpMemberSelectionModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                devNum;
    GT_STATUS                            result;
    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT  selectionMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    selectionMode    = (CPSS_DXCH_MEMBER_SELECTION_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpMemberSelectionModeSet(
        devNum, selectionMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2EcmpMemberSelectionModeGet function
* @endinternal
*
* @brief   Get mode used to calculate the Index of the secondary ePort (L2 ECMP member)
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
*/
static CMD_STATUS wrCpssDxChBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                devNum;
    GT_STATUS                            result;
    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT  selectionMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2EcmpMemberSelectionModeGet(
        devNum, &selectionMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", selectionMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgStpPortSpanningTreeStateSet function
* @endinternal
*
* @brief   Set Per-Port STP state.
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
* @note Applicable when <Port STP state Mode> = ePort
*
*/
static CMD_STATUS wrCpssDxChBrgStpPortSpanningTreeStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         dev;
    GT_PORT_NUM                   port;
    CPSS_STP_STATE_ENT            state;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev   = (GT_U8)inArgs[0];
    port  = (GT_PORT_NUM)inArgs[1];
    state = (CPSS_STP_STATE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgStpPortSpanningTreeStateSet(
        dev, port, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgStpPortSpanningTreeStateGet function
* @endinternal
*
* @brief   Get Per-Port STP state.
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
*/
static CMD_STATUS wrCpssDxChBrgStpPortSpanningTreeStateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         dev;
    GT_PORT_NUM                   port;
    CPSS_STP_STATE_ENT            state;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev   = (GT_U8)inArgs[0];
    port  = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgStpPortSpanningTreeStateGet(
        dev, port, &state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", state);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgStpPortSpanningTreeStateModeSet function
* @endinternal
*
* @brief   Determines whether the STP state for this port is
*         taken from the L2I ePort table or from the STP group table via the STG
*         index from eVLAN table
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
static CMD_STATUS wrCpssDxChBrgStpPortSpanningTreeStateModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         dev;
    GT_PORT_NUM                   port;
    CPSS_BRG_STP_STATE_MODE_ENT   stateMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];
    stateMode = (CPSS_BRG_STP_STATE_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgStpPortSpanningTreeStateModeSet(
        dev, port, stateMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgStpPortSpanningTreeStateModeGet function
* @endinternal
*
* @brief   Get whether the STP state for this port is taken from the
*         L2I ePort table or from the STP group table via the STG index from eVLAN table.
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
*/
static CMD_STATUS wrCpssDxChBrgStpPortSpanningTreeStateModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         dev;
    GT_PORT_NUM                   port;
    CPSS_BRG_STP_STATE_MODE_ENT   stateMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgStpPortSpanningTreeStateModeGet(
        dev, port, &stateMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", stateMode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortFilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable ePort VLAN Egress Filtering.
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
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortFilteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortFilteringEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of ePort VLAN Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortVidSelectModeSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filtering VID selection mode.
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
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortVidSelectModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            result;
    GT_U8                                                dev;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT  vidSelectMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev            = (GT_U8)inArgs[0];
    vidSelectMode  = (CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortVidSelectModeSet(
        dev, vidSelectMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortVidSelectModeGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filtering VID selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on unexpected HW contents
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortVidSelectModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            result;
    GT_U8                                                dev;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT  vidSelectMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev            = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(
        dev, &vidSelectMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vidSelectMode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortVidMappingSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filtering VID Mapping Table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortVidMappingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_U32           vid;
    GT_U32           vidIndex;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    vid       = (GT_U32)inArgs[1];
    vidIndex  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortVidMappingSet(
        dev, vid, vidIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortVidMappingGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filtering VID Mapping Table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortVidMappingGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_U32           vid;
    GT_U32           vidIndex;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    vid       = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortVidMappingGet(
        dev, vid, &vidIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vidIndex);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortAccessModeSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filtering Table access mode.
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
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortAccessModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           dev;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev               = (GT_U8)inArgs[0];
    fltTabAccessMode  = (CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortAccessModeSet(
        dev, fltTabAccessMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortAccessModeGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filtering Table access mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortAccessModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           dev;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev               = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortAccessModeGet(
        dev, &fltTabAccessMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", fltTabAccessMode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortMemberSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filter Table ePort-VLAN membership.
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
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortMemberSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_U32           vidIndex;
    GT_PORT_NUM      port;
    GT_BOOL          isMember;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    vidIndex  = (GT_U32)inArgs[1];
    port      = (GT_PORT_NUM)inArgs[2];
    isMember  = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortMemberSet(
        dev, vidIndex, port, isMember);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanPortMemberGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filter Table ePort-VLAN membership.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanPortMemberGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_U32           vidIndex;
    GT_PORT_NUM      port;
    GT_BOOL          isMember;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    vidIndex  = (GT_U32)inArgs[1];
    port      = (GT_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanPortMemberGet(
        dev, vidIndex, port, &isMember);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isMember);
    return CMD_OK;
}



/**
* @internal wrCpssDxChBrgEgrMeshIdConfigurationSet function
* @endinternal
*
* @brief   Set configuration of Mesh ID for the egress ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on wrong meshIdOffset and meshIdSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrMeshIdConfigurationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;
    GT_U32          meshIdOffset;
    GT_U32          meshIdSize;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];
    meshIdOffset = (GT_U32)inArgs[2];
    meshIdSize = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset, meshIdSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgEgrMeshIdConfigurationGet function
* @endinternal
*
* @brief   Get configuration of Mesh ID for the egress ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrMeshIdConfigurationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_BOOL                 enable;
    GT_U32                  meshIdOffset;
    GT_U32                  meshIdSize;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enable, &meshIdOffset, &meshIdSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", enable,meshIdOffset,meshIdSize);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgEgrPortMeshIdSet function
* @endinternal
*
* @brief   Set the Mesh ID number for Source-based (Split Horizon)
*         Filtering per ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - on wrong meshId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrPortMeshIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result=GT_OK;
    GT_U8           devNum;
    GT_PORT_NUM     portNum;
    GT_U32          meshId;
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    meshId = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;

}

/**
* @internal wrCpssDxChBrgEgrPortMeshIdGet function
* @endinternal
*
* @brief   Get the Mesh ID number for Source-based (Split Horizon)
*         Filtering per ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrPortMeshIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result=GT_OK;
    GT_U8           devNum;
    GT_PORT_NUM     portNum;
    GT_U32          meshId;
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",meshId);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgMcPhysicalSourceFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable source physical port/trunk filtering for packets that are
*         forwarded to a VIDX target.
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
static CMD_STATUS wrCpssDxChBrgMcPhysicalSourceFilteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgMcPhysicalSourceFilteringEnableGet function
* @endinternal
*
* @brief   Get the enabling status of source physical port/trunk filtering for
*         packets that are forwarded to a VIDX target.
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
static CMD_STATUS wrCpssDxChBrgMcPhysicalSourceFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltPortVid1FilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable port VID1 Egress Filtering.
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
static CMD_STATUS wrCpssDxChBrgEgrFltPortVid1FilteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgEgrFltPortVid1FilteringEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of port VID1 Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltPortVid1FilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltPortVid1Set function
* @endinternal
*
* @brief   Set port associated VID1. Relevant to egress VID1 Filtering per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on vid1 value out of an applicable range
*/
static CMD_STATUS wrCpssDxChBrgEgrFltPortVid1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_U16       vid1;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    vid1 =    (GT_U16)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltPortVid1Get function
* @endinternal
*
* @brief   Get port associated VID1. Relevant to egress VID1 Filtering per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL vid1Ptr
*/
static CMD_STATUS wrCpssDxChBrgEgrFltPortVid1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_U16       vid1;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vid1);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltPortLinkEnableSet function
* @endinternal
*
* @brief   Enable/disable egress Filtering for 'Link state' on specified port on specified device.
*         Set the port as 'force link up' (no filtering) , as 'force link down' (filtering),
*         or based on the port link state
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltPortLinkEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev     = (GT_U8)inArgs[0];
    port    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portLinkStatusState
            = (CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltPortLinkEnableSet(
        dev, port, portLinkStatusState);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltPortLinkEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable/link state)
*         for 'Link state' on specified port on specified device.
*         is the port as 'force link up' (no filtering) , as 'force link down' (filtering),
*         or based on the port link state
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum, Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltPortLinkEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltPortLinkEnableGet(
        dev, port, &portLinkStatusState);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portLinkStatusState);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanEPortFilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable ePort VLAN Egress Filtering.
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
* @note none
*
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanEPortFilteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgEgrFltVlanEPortFilteringEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of ePort VLAN Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgEgrFltVlanEPortFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/*
* @internal wrCpssDxChBrgL2DlbEnableSet function
* @endinternal
*
* @brief   Set the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; DxCh1; DxCh1_Diamond;
*                                  DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; DxCh1; DxCh1_Diamond;
*                                  DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;
     /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d" , enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbLttIndexBaseEportSet function
* @endinternal
*
* @brief  Defines the first ePort number in the L2 DLB ePort number range.
*         The index to the L2ECMP LTT Table is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbLttIndexBaseEportSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_PORT_NUM    dlbIndexBaseEport;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    dlbIndexBaseEport  = (GT_PORT_NUM)inArgs[1];

     /* call cpss api function */
    result = cpssDxChBrgL2DlbLttIndexBaseEportSet(
        devNum, dlbIndexBaseEport);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbLttIndexBaseEportGet function
* @endinternal
*
* @brief  Get the first ePort number in the L2 DLB ePort number range.
*         The index to the L2 DLB LTT is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbLttIndexBaseEportGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_PORT_NUM    dlbIndexBaseEport;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbLttIndexBaseEportGet(
        devNum, &dlbIndexBaseEport);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dlbIndexBaseEport);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbTableEntrySet function
* @endinternal
*
* @brief   Set L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Both ECMP and DLB feature use same ECMP Table
*       2. Index according to index from L2 ECMP LTT
*       3. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
static CMD_STATUS wrCpssDxChBrgL2DlbTableEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_U32         index;
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     dlbEntry;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    index                       = (GT_U32)inFields[1];
    dlbEntry.pathId             = (GT_U32)inFields[2];
    dlbEntry.lastSeenTimeStamp  = (GT_U32)inFields[3];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbTableEntrySet(
        devNum, index, &dlbEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbTableEntryGet function
* @endinternal
*
* @brief   Get L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Index according to index from L2 ECMP LTT
*       2. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
static CMD_STATUS wrCpssDxChBrgL2DlbTableEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_U32         index;
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     dlbEntry;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    index                       = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbTableEntryGet(
        devNum, index, &dlbEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", dlbEntry.pathId, dlbEntry.lastSeenTimeStamp);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbPathUtilizationScaleModeSet function
* @endinternal
*
* @brief  Configure the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static CMD_STATUS wrCpssDxChBrgL2DlbPathUtilizationScaleModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  scaleMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    scaleMode                   = (CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(
             devNum, scaleMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbPathUtilizationScaleModeGet function
* @endinternal
*
* @brief  Get the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbPathUtilizationScaleModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  scaleMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(
             devNum, &scaleMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", scaleMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbMinimumFlowletIpgSet function
* @endinternal
*
* @brief  Configure the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_OUT_OF_RANGE          -  parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbMinimumFlowletIpgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_U32         minimumIpg;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];
    minimumIpg                  = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbMinimumFlowletIpgSet(
             devNum, minimumIpg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgL2DlbMinimumFlowletIpgGet function
* @endinternal
*
* @brief  Get the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgL2DlbMinimumFlowletIpgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          devNum;
    GT_STATUS      result;
    GT_U32         minimumIpg;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum                      = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgL2DlbMinimumFlowletIpgGet(
             devNum, &minimumIpg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", minimumIpg);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
#if 0
    {"cpssDxChBrgMacHashCalc",
        &wrCpssDxChBrgMacHashCalc,
        3, 0},
#endif /* 0 */

    {"cpssDxChBrgStpInit",
        &wrCpssDxChBrgStpInit,
        1, 0},

    {"cpssDxChBrgStpStateSet",
        &wrCpssDxChBrgStpStateSet,
        4, 0},

    {"cpssDxChBrgStpStateGet",
        &wrCpssDxChBrgStpStateGet,
        3, 0},

    {"cpssDxChBrgStpEntryGetFirst",
        &wrCpssDxChBrgStpEntryGetFirst,
        1, 0},

    {"cpssDxChBrgStpEntryGetNext",
        &wrCpssDxChBrgStpEntryGetNext,
        1, 0},

    {"cpssDxChBrgStpEntrySet",
        &wrCpssDxChBrgStpEntryWrite,
        1, 2 + 1},

    {"cpssDxChBrgPortEgrFltUnkEnable",
        &wrCpssDxChBrgPortEgrFltUnkEnable,
        3, 0},

    {"cpssDxChBrgPortEgrFltUregMcastEnable",
        &wrCpssDxChBrgPortEgrFltUregMcastEnable,
        3, 0},

    {"cpssDxChBrgVlanEgressFilteringEnable",
        &wrCpssDxChBrgVlanEgressFilteringEnable,
        2, 0},

    {"cpssDxChBrgRoutedUnicastEgressFilteringEnable",
        &wrCpssDxChBrgRoutedUnicastEgressFilteringEnable,
        2, 0},

    {"cpssDxChBrgRoutedSpanEgressFilteringEnable",
        &wrCpssDxChBrgRoutedSpanEgressFilteringEnable,
        2, 0},

    {"cpssDxChBrgPortEgrFltUregBcEnable",
        &wrCpssDxChBrgPortEgrFltUregBcEnable,
        3, 0},

    {"cpssDxChBrgPortEgressMcastLocalEnable",
        &wrCpssDxChBrgPortEgressMcastLocalEnable,
        3, 0},

    {"cpssDxChBrgMcInit",
        &wrCpssDxChBrgMcInit,
        1, 0},

    {"cpssDxChBrgMcIpv6BytesSelectSet",
        &wrCpssDxChBrgMcIpv6BytesSelectSet,
        1, 7},

    {"cpssDxChBrgMcIpv6BytesSelectGetFirst",
        &wrCpssDxChBrgMcIpv6BytesSelectGetFirst,
        1, 0},

    {"cpssDxChBrgMcIpv6BytesSelectGetNext",
        &wrCpssDxChBrgMcIpv6BytesSelectGetNext,
        1, 0},

    {"cpssDxChBrgMcV1Set",
        &wrCpssDxChBrgMcV1EntryWrite,
        3, 9},

    {"cpssDxChBrgMcV1GetFirst",
        &wrCpssDxChBrgMcEntryReadV1First,
        3, 0},

    {"cpssDxChBrgMcV1GetNext",
        &wrCpssDxChBrgMcEntryReadV1Next,
        3, 0},

    {"cpssDxChBrgMcV1Delete",
        &wrCpssDxChBrgMcGroupDelete,
        3, 9},

    {"cpssDxChBrgMcSet",
        &wrCpssDxChBrgMcEntryWrite,
        3, 3},

    {"cpssDxChBrgMcGetFirst",
        &wrCpssDxChBrgMcEntryReadFirst,
        3, 0},

    {"cpssDxChBrgMcGetNext",
        &wrCpssDxChBrgMcEntryReadNext,
        3, 0},

    {"cpssDxChBrgMcDelete",
        &wrCpssDxChBrgMcGroupDelete,
        3, 1},

    {"cpssDxChBrgMcMemberAdd",
        &wrCpssDxChBrgMcMemberAdd,
        3, 0},

    {"cpssDxChBrgMcMemberDelete",
        &wrCpssDxChBrgMcMemberDelete,
        3, 0},

    {"cpssDxChBrgNestVlanAccessPortSet",
        &wrCpssDxChBrgNestVlanAccessPortSet,
        3, 0},

    {"cpssDxChBrgNestVlanAccessPortGet",
        &wrCpssDxChBrgNestVlanAccessPortGet,
        2, 0},

    {"cpssDxChBrgPortEgrFltUnkEnableGet",
         &wrCpssDxChBrgPortEgrFltUnkEnableGet,
         2, 0},

    {"cpssDxChBrgPortEgrFltUregMcastEnableGet",
         &wrCpssDxChBrgPortEgrFltUregMcastEnableGet,
         2, 0},

    {"cpssDxChBrgVlanEgressFilteringEnableGet",
         &wrCpssDxChBrgVlanEgressFilteringEnableGet,
         1, 0},

    {"cpssDxChBrgRoutedUnicastEgressFilteringEnableGet",
         &wrCpssDxChBrgRoutedUnicastEgressFilteringEnableGet,
         1, 0},

    {"cpssDxChBrgRoutedSpanEgressFilteringEnableGet",
         &wrCpssDxChBrgRoutedSpanEgressFilteringEnableGet,
         1, 0},

    {"cpssDxChBrgPortEgrFltUregBcEnableGet",
         &wrCpssDxChBrgPortEgrFltUregBcEnableGet,
         2, 0},

    {"cpssDxChBrgPortEgressMcastLocalEnableGet",
         &wrCpssDxChBrgPortEgressMcastLocalEnableGet,
         2, 0},

    {"cpssDxChBrgPortEgrFltIpMcRoutedEnable",
         &wrCpssDxChBrgPortEgrFltIpMcRoutedEnable,
         3, 0},

    {"cpssDxChBrgPortEgrFltIpMcRoutedEnableGet",
         &wrCpssDxChBrgPortEgrFltIpMcRoutedEnableGet,
         2, 0},

    {"cpssDxChBrgEportToPhysicalPortTargetMapSet",
         &wrCpssDxChBrgEportToPhysicalPortTargetMappingTableSet,
         3, 7},

    {"cpssDxChBrgEportToPhysicalPortTargetMapGetFirst",
         &wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGetFirst,
         3, 0},

    {"cpssDxChBrgEportToPhysicalPortTargetMapGetNext",
         &wrCpssDxChBrgEportToPhysicalPortTargetMappingTableGetNext,
         3, 0},

    {"cpssDxChBrgEportToPhysicalPortEgressPortSet",
         &wrCpssDxChBrgEportToPhysicalPortEgressPortTableSet,
         3, 8},

    {"cpssDxChBrgEportToPhysicalPortEgressPortGetFirst",
         &wrCpssDxChBrgEportToPhysicalPortEgressPortTableGetFirst,
         3, 0},

    {"cpssDxChBrgEportToPhysicalPortEgressPortGetNext",
         &wrCpssDxChBrgEportToPhysicalPortEgressPortTableGetNext,
         3, 0},

    {"cpssDxChBrgEportToPhysicalPortEgressPort2Set",
         &wrCpssDxChBrgEportToPhysicalPortEgressPortTable2Set,
         3, 7},

    {"cpssDxChBrgEportToPhysicalPortEgressPort2GetFirst",
         &wrCpssDxChBrgEportToPhysicalPortEgressPortTable2GetFirst,
         3, 0},

    {"cpssDxChBrgEportToPhysicalPortEgressPort2GetNext",
         &wrCpssDxChBrgEportToPhysicalPortEgressPortTable2GetNext,
         3, 0},


    {"cpssDxChBrgL2EcmpEnableSet",
        &wrCpssDxChBrgL2EcmpEnableSet,
        2, 0},
    {"cpssDxChBrgL2EcmpEnableGet",
        &wrCpssDxChBrgL2EcmpEnableGet,
        1, 0},

    {"cpssDxChBrgL2EcmpIndexBaseEportSet",
         &wrCpssDxChBrgL2EcmpIndexBaseEportSet,
         2, 0},

    {"cpssDxChBrgL2EcmpIndexBaseEportGet",
         &wrCpssDxChBrgL2EcmpIndexBaseEportGet,
         1, 0},

    {"cpssDxChBrgL2EcmpLttTableSet",
         &wrCpssDxChBrgL2EcmpLttTableSet,
         3, 4},

    {"cpssDxChBrgL2EcmpLttTableGetFirst",
         &wrCpssDxChBrgL2EcmpLttTableGetFirst,
         3, 0},

    {"cpssDxChBrgL2EcmpLttTableGetNext",
         &wrCpssDxChBrgL2EcmpLttTableGetNext,
         3, 0},

    {"cpssDxChBrgL2EcmpTableSet",
         &wrCpssDxChBrgL2EcmpTableSet,
         3, 3},

    {"cpssDxChBrgL2EcmpTableGetFirst",
         &wrCpssDxChBrgL2EcmpTableGetFirst,
         3, 0},

    {"cpssDxChBrgL2EcmpTableGetNext",
         &wrCpssDxChBrgL2EcmpTableGetNext,
         3, 0},

    {"cpssDxChBrgL2EcmpMemberSelectionModeSet",
         &wrCpssDxChBrgL2EcmpMemberSelectionModeSet,
         2, 0},

    {"cpssDxChBrgL2EcmpMemberSelectionModeGet",
         &wrCpssDxChBrgL2EcmpMemberSelectionModeGet,
         1, 0},

    {"cpssDxChBrgStpPortSpanningTreeStateSet",
         &wrCpssDxChBrgStpPortSpanningTreeStateSet,
         3, 0},

    {"cpssDxChBrgStpPortSpanningTreeStateGet",
         &wrCpssDxChBrgStpPortSpanningTreeStateGet,
         2, 0},

    {"cpssDxChBrgStpPortSpanningTreeStateModeSet",
         &wrCpssDxChBrgStpPortSpanningTreeStateModeSet,
         3, 0},

    {"cpssDxChBrgStpPortSpanningTreeStateModeGet",
         &wrCpssDxChBrgStpPortSpanningTreeStateModeGet,
         2, 0},

    {"cpssDxChBrgEgrFltVlanPortFilteringEnableSet",
         &wrCpssDxChBrgEgrFltVlanPortFilteringEnableSet,
         3, 0},

    {"cpssDxChBrgEgrFltVlanPortFilteringEnableGet",
         &wrCpssDxChBrgEgrFltVlanPortFilteringEnableGet,
         2, 0},

    {"cpssDxChBrgEgrFltVlanPortVidSelectModeSet",
         &wrCpssDxChBrgEgrFltVlanPortVidSelectModeSet,
         2, 0},

    {"cpssDxChBrgEgrFltVlanPortVidSelectModeGet",
         &wrCpssDxChBrgEgrFltVlanPortVidSelectModeGet,
         1, 0},

    {"cpssDxChBrgEgrFltVlanPortVidMappingSet",
         &wrCpssDxChBrgEgrFltVlanPortVidMappingSet,
         3, 0},

    {"cpssDxChBrgEgrFltVlanPortVidMappingGet",
         &wrCpssDxChBrgEgrFltVlanPortVidMappingGet,
         2, 0},

    {"cpssDxChBrgEgrFltVlanPortAccessModeSet",
         &wrCpssDxChBrgEgrFltVlanPortAccessModeSet,
         2, 0},

    {"cpssDxChBrgEgrFltVlanPortAccessModeGet",
         &wrCpssDxChBrgEgrFltVlanPortAccessModeGet,
         1, 0},

    {"cpssDxChBrgEgrFltVlanPortMemberSet",
         &wrCpssDxChBrgEgrFltVlanPortMemberSet,
         4, 0},

    {"cpssDxChBrgEgrFltVlanPortMemberGet",
         &wrCpssDxChBrgEgrFltVlanPortMemberGet,
         3, 0},

    {"cpssDxChBrgEgrMeshIdConfigurationSet",
        &wrCpssDxChBrgEgrMeshIdConfigurationSet,
        4, 0},
    {"cpssDxChBrgEgrMeshIdConfigurationGet",
        &wrCpssDxChBrgEgrMeshIdConfigurationGet,
        1, 0},

    {"cpssDxChBrgEgrPortMeshIdSet",
        &wrCpssDxChBrgEgrPortMeshIdSet,
        3, 0},
    {"cpssDxChBrgEgrPortMeshIdGet",
        &wrCpssDxChBrgEgrPortMeshIdGet,
        2, 0},
    {"cpssDxChBrgMcPhysicalSourceFileringEnableSet",
        &wrCpssDxChBrgMcPhysicalSourceFilteringEnableSet,
        3, 0},
    {"cpssDxChBrgMcPhysicalSourceFileringEnableGet",
        &wrCpssDxChBrgMcPhysicalSourceFilteringEnableGet,
        2, 0},
    {"cpssDxChBrgEgrFltPortVid1FilteringEnableSet",
        &wrCpssDxChBrgEgrFltPortVid1FilteringEnableSet,
        3, 0},
    {"cpssDxChBrgEgrFltPortVid1FilteringEnableGet",
        &wrCpssDxChBrgEgrFltPortVid1FilteringEnableGet,
        2, 0},
    {"cpssDxChBrgEgrFltPortVid1Set",
        &wrCpssDxChBrgEgrFltPortVid1Set,
        3, 0},
    {"cpssDxChBrgEgrFltPortVid1Get",
        &wrCpssDxChBrgEgrFltPortVid1Get,
        2, 0},
    {"cpssDxChBrgEgrFltPortLinkEnableSet",
        &wrCpssDxChBrgEgrFltPortLinkEnableSet,
        3, 0},
    {"cpssDxChBrgEgrFltPortLinkEnableGet",
        &wrCpssDxChBrgEgrFltPortLinkEnableGet,
        2, 0},
    {"cpssDxChBrgEgrFltVlanEPortFilteringEnableSet",
        &wrCpssDxChBrgEgrFltVlanEPortFilteringEnableSet,
        3, 0},
    {"cpssDxChBrgEgrFltVlanEPortFilteringEnableGet",
        &wrCpssDxChBrgEgrFltVlanEPortFilteringEnableGet,
        2, 0},
    {"cpssDxChBrgL2DlbEnableSet",
        &wrCpssDxChBrgL2DlbEnableSet,
        2, 0},
    {"cpssDxChBrgL2DlbEnableGet",
        &wrCpssDxChBrgL2DlbEnableGet,
        1, 0},
    {"cpssDxChBrgL2DlbLttIndexBaseEportSet",
        &wrCpssDxChBrgL2DlbLttIndexBaseEportSet,
        2, 0},
    {"cpssDxChBrgL2DlbLttIndexBaseEportGet",
        &wrCpssDxChBrgL2DlbLttIndexBaseEportGet,
        1, 0},
    {"cpssDxChBrgL2DlbTableEntrySet",
        &wrCpssDxChBrgL2DlbTableEntrySet,
        4,0},
    {"cpssDxChBrgL2DlbTableEntryGet",
        &wrCpssDxChBrgL2DlbTableEntryGet,
        2,0},
    {"cpssDxChBrgL2DlbPathUtilizationScaleModeSet",
        &wrCpssDxChBrgL2DlbPathUtilizationScaleModeSet,
        2,0},
    {"cpssDxChBrgL2DlbPathUtilizationScaleModeGet",
        &wrCpssDxChBrgL2DlbPathUtilizationScaleModeGet,
        1,0},
    {"cpssDxChBrgL2DlbMinimumFlowletIpgSet",
        &wrCpssDxChBrgL2DlbMinimumFlowletIpgSet,
        2,0},
    {"cpssDxChBrgL2DlbMinimumFlowletIpgGet",
        &wrCpssDxChBrgL2DlbMinimumFlowletIpgGet,
        1,0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridge function
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
GT_STATUS cmdLibInitCpssDxChBridge
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



