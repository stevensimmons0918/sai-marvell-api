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
* wrapBridgeCountCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for BridgeCount cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>

/**
* @internal bridgeSecurityBreachMultiPortGroupsBmpGet function
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
static void bridgeSecurityBreachMultiPortGroupsBmpGet
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


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgSecurBreachPortVlanCntrGet
(
    IN GT_U8      devNum,
    OUT GT_U32    *counValuePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeSecurityBreachMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgSecurBreachPortVlanCntrGet(devNum, counValuePtr);
    }
    else
    {
        return cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet(devNum,
                                                           pgBmp, counValuePtr);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgSecurBreachGlobalDropCntrGet
(
    IN GT_U8      devNum,
    OUT GT_U32    *counValuePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeSecurityBreachMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgSecurBreachGlobalDropCntrGet(devNum, counValuePtr);
    }
    else
    {
        return cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet(devNum,
                                                           pgBmp, counValuePtr);
    }
}

/*******************************************************************************
* cpssDxChBrgSecurBreachPortVlanDropCntrModeSet
*
* DESCRIPTION:
*      Set Port/VLAN Security Breach Drop Counter to count security breach
*      dropped packets based on there ingress port or their assigned VID.
*
* APPLICABLE DEVICES:  All DXCH devices
*
* INPUTS:
*       dev                 - physical device number
*       cntrCfgPtr          - (pointer to) security breach port/vlan counter
*                             mode and it's configuration parameters.
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK               - on success.
*       GT_BAD_PARAM        - bad input parameters.
*       GT_HW_ERROR         - on hardware error
*
* COMMENTS:
*       None.
*
*  table cpssDxChBrgSecurBreachPortVlanDropCntrMode  global parameter  */
static   CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC        cntrCfgPtr;

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgSecurBreachPortVlanDropCntrModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;

    GT_U8                                           devNum;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    cntrCfgPtr.dropCntMode =
                         (CPSS_BRG_SECUR_BREACH_DROP_COUNT_MODE_ENT)inFields[0];
    cntrCfgPtr.port = (GT_U8)inFields[1];
    cntrCfgPtr.vlan = (GT_U16)inFields[2];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(devNum, &cntrCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachPortVlanDropCntrModeGetFirst function
* @endinternal
*
* @brief   Get mode and it's configuration parameters for security breach
*         Port/VLAN drop counter.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachPortVlanDropCntrModeGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(devNum,&cntrCfgPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = cntrCfgPtr.dropCntMode;
    inFields[1] = cntrCfgPtr.port;
    inFields[2] = cntrCfgPtr.vlan;


    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgSecurBreachPortVlanDropCntrModeEndGet

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

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachGlobalDropCntrGet function
* @endinternal
*
* @brief   Get values of security breach global drop counter. Global security breach
*         counter counts all dropped packets due to security breach event.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachGlobalDropCntrGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_U32                   counValuePtr;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgSecurBreachGlobalDropCntrGet(devNum, &counValuePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counValuePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachPortVlanCntrGet function
* @endinternal
*
* @brief   Get value of security breach Port/Vlan drop counter. This counter counts
*         all of the packets received according to Port/Vlan security breach drop
*         count mode configuration and dropped due to any security breach event.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachPortVlanCntrGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_U32                   counValuePtr;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgSecurBreachPortVlanCntrGet(devNum, &counValuePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counValuePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachNaPerPortSet function
* @endinternal
*
* @brief   Enable/Disable per port the uknown Source Adresses to be considered as
*         security breach event. When enabled, the NA message is not sent to CPU
*         and this address is not learned.
*         Only relevant in controled learning mode.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachNaPerPortSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_PORT_NUM                    portNum;
    GT_BOOL                  enable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

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
    result = cpssDxChBrgSecurBreachNaPerPortSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachNaPerPortGet function
* @endinternal
*
* @brief   Get per port the status of NA security breach event (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachNaPerPortGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_PORT_NUM                    portNum;
    GT_BOOL                  enablePtr;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachNaPerPortGet(devNum, portNum, &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachMovedStaticAddrSet function
* @endinternal
*
* @brief   When the FDB entry is signed as a static one, then this entry is not
*         subject to aging. In addition, there may be cases where the interface
*         does't match the interface from which this packet was received. In this
*         case, this feature enables/disables to regard moved static adresses as
*         a security breach event.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachMovedStaticAddrSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_BOOL                  enable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachMovedStaticAddrSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachMovedStaticAddrGet function
* @endinternal
*
* @brief   Get if static addresses that are moved, are regarded as Security Breach
*         or not (this is only in case that the fdb entry is static or the
*         interface does't match the interface from which this packet was received).
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachMovedStaticAddrGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_BOOL                  enablePtr;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachMovedStaticAddrGet(devNum, &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachEventDropModeSet function
* @endinternal
*
* @brief   Set for given security breach event it's drop mode.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*
* @note The acceptable events are:
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_STATIC_MOVED_E,
*
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachEventDropModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_DROP_MODE_TYPE_ENT             dropMode;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    eventType = (CPSS_BRG_SECUR_BREACH_EVENTS_ENT)inArgs[1];
    dropMode = (CPSS_DROP_MODE_TYPE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachEventDropModeSet(devNum, eventType,
                                                             dropMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachEventDropModeGet function
* @endinternal
*
* @brief   Get for given security breach event it's drop mode.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*
* @note The acceptable events are:
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_STATIC_MOVED_E,
*
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachEventDropModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_DROP_MODE_TYPE_ENT             dropModePtr;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    eventType = (CPSS_BRG_SECUR_BREACH_EVENTS_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachEventDropModeGet(devNum, eventType,
                                                         &dropModePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropModePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachEventPacketCommandSet function
* @endinternal
*
* @brief   Set for given security breach event it's packet command.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function similar to cpssDxChBrgSecurBreachEventDropModeSet , but
*       allow additional packet commands.
*
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachEventPacketCommandSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_PACKET_CMD_ENT             command;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    eventType = (CPSS_BRG_SECUR_BREACH_EVENTS_ENT)inArgs[1];
    command = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum, eventType,
                                                             command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSecurBreachEventPacketCommandGet function
* @endinternal
*
* @brief   Get for given security breach event it's packet command.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function similar to cpssDxChBrgSecurBreachEventDropModeGet , but
*       allow additional packet commands.
*
*/
static CMD_STATUS wrCpssDxChBrgSecurBreachEventPacketCommandGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType;
    CPSS_PACKET_CMD_ENT             command;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    eventType = (CPSS_BRG_SECUR_BREACH_EVENTS_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgSecurBreachEventPacketCommandGet(devNum, eventType,
                                                         &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);
    return CMD_OK;
}

/**
* @internal wrCpssDxChSecurBreachMsgGet function
* @endinternal
*
* @brief   Reads the Security Breach message update. When a security breach occures
*         and the CPU had read the previous message (the message is locked untill
*         CPU reads it, after it the new message can be written) the security
*         massage parameters are updated. Those parameters include: MAC address of
*         the breaching packet, security breach port, VID and security breach code.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChSecurBreachMsgGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_BRG_SECUR_BREACH_MSG_STC   sbMsgPtr;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChSecurBreachMsgGet(devNum, &sbMsgPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[1] = sbMsgPtr.vlan;
    inFields[2] = sbMsgPtr.port;
    inFields[3] = sbMsgPtr.code;


    /* pack and output table fields */
    fieldOutput("%6b%d%d%d", sbMsgPtr.macSa.arEther,
                inFields[1], inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChSecurBreachMsgEndGet

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

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChBrgSecurBrchPortVlanDropCntrModeSet",
        &wrCpssDxChBrgSecurBreachPortVlanDropCntrModeSet,
        1, 3},

    {"cpssDxChBrgSecurBrchPortVlanDropCntrModeGetFirst",
        &wrCpssDxChBrgSecurBreachPortVlanDropCntrModeGetFirst,
        1, 0},

    {"cpssDxChBrgSecurBrchPortVlanDropCntrModeGetNext",
        &wrCpssDxChBrgSecurBreachPortVlanDropCntrModeEndGet,
        1, 0},

    {"cpssDxChBrgSecurBreachGlobalDropCntrGet",
        &wrCpssDxChBrgSecurBreachGlobalDropCntrGet,
        1, 0},

    {"cpssDxChBrgSecurBreachPortVlanCntrGet",
        &wrCpssDxChBrgSecurBreachPortVlanCntrGet,
        1, 0},

    {"cpssDxChBrgSecurBreachNaPerPortSet",
        &wrCpssDxChBrgSecurBreachNaPerPortSet,
        3, 0},

    {"cpssDxChBrgSecurBreachNaPerPortGet",
        &wrCpssDxChBrgSecurBreachNaPerPortGet,
        2, 0},

    {"cpssDxChBrgSecurBreachMovedStaticAddrSet",
        &wrCpssDxChBrgSecurBreachMovedStaticAddrSet,
        2, 0},

    {"cpssDxChBrgSecurBreachMovedStaticAddrGet",
        &wrCpssDxChBrgSecurBreachMovedStaticAddrGet,
        1, 0},

    {"cpssDxChBrgSecurBreachEventDropModeSet",
        &wrCpssDxChBrgSecurBreachEventDropModeSet,
        3, 0},

    {"cpssDxChBrgSecurBreachEventDropModeGet",
        &wrCpssDxChBrgSecurBreachEventDropModeGet,
        2, 0},

    {"cpssDxChBrgSecurBreachEventPacketCommandSet",
        &wrCpssDxChBrgSecurBreachEventPacketCommandSet,
        3, 0},

    {"cpssDxChBrgSecurBreachEventPacketCommandGet",
        &wrCpssDxChBrgSecurBreachEventPacketCommandGet,
        2, 0},

    {"cpssDxChSecurBreachMsgGetFirst",
        &wrCpssDxChSecurBreachMsgGet,
        1, 0},

    {"cpssDxChSecurBreachMsgGetNext",
        &wrCpssDxChSecurBreachMsgEndGet,
        1, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridgeSecurityBreach function
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
GT_STATUS cmdLibInitCpssDxChBridgeSecurityBreach
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

