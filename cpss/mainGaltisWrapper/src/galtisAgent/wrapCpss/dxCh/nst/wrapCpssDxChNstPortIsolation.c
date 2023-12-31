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
* @file wrapCpssDxChNstPortIsolation.c
*
* @brief Wrapper functions for NST Port Isolation feature functions
*
* @version   17
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>

/**
* @internal wrCpssDxChNstPortIsolationEnableSet function
* @endinternal
*
* @brief   Function enables/disables the port isolation feature.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNstPortIsolationEnableSet
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
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNstPortIsolationEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationEnableGet function
* @endinternal
*
* @brief   Function gets enabled/disabled state of the port isolation feature.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNstPortIsolationEnableGet
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
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNstPortIsolationEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationTableEntrySet function
* @endinternal
*
* @brief   Function sets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking. If it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface or
*                                       localPortsMembersPtr
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Traffic is separated for L2 and L3 (means that application may
*       block L2 traffic while allowing L3).
*
*/
CMD_STATUS wrCpssDxChNstPortIsolationTableEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_BOOL                                        cpuPortMember;
    CPSS_PORTS_BMP_STC                             localPortsMembers;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trafficType = (CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)inArgs[1];
    cmdOsMemSet(&srcInterface, 0, sizeof(srcInterface));
    srcInterface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];

    switch (inArgs[2])
    {
    case 0:
        srcInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
        srcInterface.devPort.portNum = (GT_PORT_NUM)inArgs[4];
        CONVERT_DEV_PORT_DATA_MAC(srcInterface.devPort.hwDevNum ,
                             srcInterface.devPort.portNum);
        break;

    case 1:
        srcInterface.trunkId = (GT_TRUNK_ID)inArgs[5];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);
        break;

    default:
        break;
    }

    cpuPortMember = (GT_BOOL)inArgs[6];
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);
    localPortsMembers.ports[0] = (GT_U32)inArgs[7];
    localPortsMembers.ports[1] = (GT_U32)inArgs[8];
    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,localPortsMembers);

    /* call cpss api function */
    result = cpssDxChNstPortIsolationTableEntrySet(devNum,
                                                   trafficType,
                                                   &srcInterface,
                                                   cpuPortMember,
                                                   &localPortsMembers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationTableEntrySet1 function
* @endinternal
*
* @brief   Function sets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking. If it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface or
*                                       localPortsMembersPtr
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Traffic is separated for L2 and L3 (means that application may
*       block L2 traffic while allowing L3).
*
*/
CMD_STATUS wrCpssDxChNstPortIsolationTableEntrySet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_BOOL                                        cpuPortMember;
    CPSS_PORTS_BMP_STC                             localPortsMembers;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trafficType = (CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)inArgs[1];
    cmdOsMemSet(&srcInterface, 0, sizeof(srcInterface));
    srcInterface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];

    switch (inArgs[2])
    {
    case 0:
        srcInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
        srcInterface.devPort.portNum = (GT_PORT_NUM)inArgs[4];
        CONVERT_DEV_PORT_DATA_MAC(srcInterface.devPort.hwDevNum ,
                             srcInterface.devPort.portNum);
        break;

    case 1:
        srcInterface.trunkId = (GT_TRUNK_ID)inArgs[5];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);
        break;

    default:
        break;
    }

    cpuPortMember = (GT_BOOL)inArgs[6];
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsMembers);
    localPortsMembers.ports[0] = (GT_U32)inArgs[7];
    localPortsMembers.ports[1] = (GT_U32)inArgs[8];
    localPortsMembers.ports[2] = (GT_U32)inArgs[9];
    localPortsMembers.ports[3] = (GT_U32)inArgs[10];
#if CPSS_MAX_PORTS_NUM_CNS > 128
    localPortsMembers.ports[4] = (GT_U32)inArgs[11];
    localPortsMembers.ports[5] = (GT_U32)inArgs[12];
    localPortsMembers.ports[6] = (GT_U32)inArgs[13];
    localPortsMembers.ports[7] = (GT_U32)inArgs[14];
#endif
    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,localPortsMembers);

    /* call cpss api function */
    result = cpssDxChNstPortIsolationTableEntrySet(devNum,
                                                   trafficType,
                                                   &srcInterface,
                                                   cpuPortMember,
                                                   &localPortsMembers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationTableEntryGet function
* @endinternal
*
* @brief   Function gets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking if it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Traffic is separated for L2 and L3 (means that application may
*       block L2 traffic while allowing L3).
*
*/
CMD_STATUS wrCpssDxChNstPortIsolationTableEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_BOOL                                        cpuPortMember;
    CPSS_PORTS_BMP_STC                             localPortsMembers;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trafficType = (CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)inArgs[1];
    cmdOsMemSet(&srcInterface, 0, sizeof(srcInterface));
    srcInterface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];

    switch (inArgs[2])
    {
    case 0:
        srcInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
        srcInterface.devPort.portNum = (GT_PORT_NUM)inArgs[4];
        CONVERT_DEV_PORT_DATA_MAC(srcInterface.devPort.hwDevNum ,
                             srcInterface.devPort.portNum);
        break;

    case 1:
        srcInterface.trunkId = (GT_TRUNK_ID)inArgs[5];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);
        break;

    default:
        break;
    }

    /* call cpss api function */
    result = cpssDxChNstPortIsolationTableEntryGet(devNum,
                                                   trafficType,
                                                   &srcInterface,
                                                   &cpuPortMember,
                                                   &localPortsMembers);

    CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,localPortsMembers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", cpuPortMember,
                                            localPortsMembers.ports[0],
                                            localPortsMembers.ports[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationTableEntryGet1 function
* @endinternal
*
* @brief   Function gets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking if it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Traffic is separated for L2 and L3 (means that application may
*       block L2 traffic while allowing L3).
*
*/
CMD_STATUS wrCpssDxChNstPortIsolationTableEntryGet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_BOOL                                        cpuPortMember;
    CPSS_PORTS_BMP_STC                             localPortsMembers;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trafficType = (CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)inArgs[1];
    cmdOsMemSet(&srcInterface, 0, sizeof(srcInterface));
    cmdOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));
    srcInterface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];

    switch (inArgs[2])
    {
    case 0:
        srcInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
        srcInterface.devPort.portNum = (GT_PORT_NUM)inArgs[4];
        CONVERT_DEV_PORT_DATA_MAC(srcInterface.devPort.hwDevNum ,
                             srcInterface.devPort.portNum);
        break;

    case 1:
        srcInterface.trunkId = (GT_TRUNK_ID)inArgs[5];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);
        break;

    default:
        break;
    }

    /* call cpss api function */
    result = cpssDxChNstPortIsolationTableEntryGet(devNum,
                                                   trafficType,
                                                   &srcInterface,
                                                   &cpuPortMember,
                                                   &localPortsMembers);

    CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,localPortsMembers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d", cpuPortMember,
                                                        localPortsMembers.ports[0],
                                                        localPortsMembers.ports[1],
                                                        localPortsMembers.ports[2],
                                                        localPortsMembers.ports[3],
#if CPSS_MAX_PORTS_NUM_CNS > 128
                                                        localPortsMembers.ports[4],
                                                        localPortsMembers.ports[5],
                                                        localPortsMembers.ports[6],
                                                        localPortsMembers.ports[7]
#else
                                                        0,0,0,0
#endif
                                                        );

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationPortAdd function
* @endinternal
*
* @brief   Function adds single local port to port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Adding local port (may be also CPU port 63) to port isolation entry
*         means that traffic which came from srcInterface and wish to egress
*         at the specified local port isn't blocked.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface, portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNstPortIsolationPortAdd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_PHYSICAL_PORT_NUM                           portNum;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trafficType = (CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)inArgs[1];
    cmdOsMemSet(&srcInterface, 0, sizeof(srcInterface));
    srcInterface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];

    switch (inArgs[2])
    {
    case 0:
        srcInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
        srcInterface.devPort.portNum = (GT_PORT_NUM)inArgs[4];
        CONVERT_DEV_PORT_DATA_MAC(srcInterface.devPort.hwDevNum ,
                             srcInterface.devPort.portNum);
        break;

    case 1:
        srcInterface.trunkId = (GT_TRUNK_ID)inArgs[5];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);
        break;

    default:
        break;
    }

    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[6];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChNstPortIsolationPortAdd(devNum,
                                             trafficType,
                                             &srcInterface,
                                             portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationPortDelete function
* @endinternal
*
* @brief   Function deletes single local port to port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Deleting local port (may be also CPU port 63) to port isolation entry
*         means that traffic which came from srcInterface and wish to egress
*         at the specified local port is blocked.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface, portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In srcInterface parameter only portDev and Trunk are supported.
*
*/
CMD_STATUS wrCpssDxChNstPortIsolationPortDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType;
    CPSS_INTERFACE_INFO_STC                        srcInterface;
    GT_PHYSICAL_PORT_NUM                           portNum;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trafficType = (CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)inArgs[1];
    cmdOsMemSet(&srcInterface, 0, sizeof(srcInterface));
    srcInterface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];

    switch (inArgs[2])
    {
    case 0:
        srcInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
        srcInterface.devPort.portNum = (GT_PORT_NUM)inArgs[4];
        CONVERT_DEV_PORT_DATA_MAC(srcInterface.devPort.hwDevNum ,
                             srcInterface.devPort.portNum);
        break;

    case 1:
        srcInterface.trunkId = (GT_TRUNK_ID)inArgs[5];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);
        break;

    default:
        break;
    }

    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[6];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChNstPortIsolationPortDelete(devNum,
                                                trafficType,
                                                &srcInterface,
                                                portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationLookupBitsSet function
* @endinternal
*
* @brief   Set the number of bits from the source Interface that are used to
*         index the port isolation table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - on value out of range for one of:
*                                       numberOfPortBits > 15 , numberOfDeviceBits > 15 ,
*                                       numberOfTrunkBits > 15
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For example, if the value of this field is 7, and <Port Isolation
*       Lookup Device Bits> is 5, then the index to the port isolation table is
*       (SrcDev[4:0], OrigSRCePort[6:0]).
*       Note:
*       If <Port Isolation Lookup Port Bits> is 0x0, no bits from the source
*       port are used.
*       If <Port Isolation Lookup Device Bits> is 0x0, no bits from the
*       SrcDev are used.
*
*/
CMD_STATUS wrCpssDxChNstPortIsolationLookupBitsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      numberOfPortBits;
    GT_U32      numberOfDeviceBits;
    GT_U32      numberOfTrunkBits;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    numberOfPortBits = (GT_U32)inArgs[1];
    numberOfDeviceBits = (GT_U32)inArgs[2];
    numberOfTrunkBits = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChNstPortIsolationLookupBitsSet(devNum, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNstPortIsolationLookupBitsGet function
* @endinternal
*
* @brief   Get the number of bits from the source Interface that are used to
*         index the port isolation table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNstPortIsolationLookupBitsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      numberOfPortBits;
    GT_U32      numberOfDeviceBits;
    GT_U32      numberOfTrunkBits;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNstPortIsolationLookupBitsGet(devNum, &numberOfPortBits, &numberOfDeviceBits, &numberOfTrunkBits);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChNstPortIsolationEnableSet",
        &wrCpssDxChNstPortIsolationEnableSet,
        2, 0},

    {"cpssDxChNstPortIsolationEnableGet",
        &wrCpssDxChNstPortIsolationEnableGet,
        1, 0},

    {"cpssDxChNstPortIsolationTableEntrySet",
        &wrCpssDxChNstPortIsolationTableEntrySet,
        9, 0},

    {"cpssDxChNstPortIsolationTableEntrySet1",
        &wrCpssDxChNstPortIsolationTableEntrySet1,
        15, 0},

    {"cpssDxChNstPortIsolationTableEntryGet",
        &wrCpssDxChNstPortIsolationTableEntryGet,
        6, 0},

    {"cpssDxChNstPortIsolationTableEntryGet1",
        &wrCpssDxChNstPortIsolationTableEntryGet1,
        6, 0},

    {"cpssDxChNstPortIsolationPortAdd",
        &wrCpssDxChNstPortIsolationPortAdd,
        7, 0},

    {"cpssDxChNstPortIsolationPortDelete",
        &wrCpssDxChNstPortIsolationPortDelete,
        7, 0},
    {"cpssDxChNstPortIsolationLookupBitsSet",
        &wrCpssDxChNstPortIsolationLookupBitsSet,
        4, 0},

    {"cpssDxChNstPortIsolationLookupBitsGet",
        &wrCpssDxChNstPortIsolationLookupBitsGet,
        1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChNstPortIsolation function
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
GT_STATUS cmdLibInitCpssDxChNstPortIsolation
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

