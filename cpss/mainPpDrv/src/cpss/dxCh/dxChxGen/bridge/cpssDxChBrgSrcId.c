/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChBrgSrcId.c
*
* @brief CPSS DxCh Source ID facility implementation
*
* @version   28
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal prvCpssDxChBrgSrcIdGroupTableSet function
* @endinternal
*
* @brief   Set a bit per port indicating if the packet with this Source-ID may be
*         forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID number that the port is added to
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] action                   - GT_FALSE  - the port is not a member of this Source ID group
*                                      and packets assigned with this Source ID are not
*                                      forwarded to this port.
*                                      - GT_TRUE  - the portis a member of this Source ID group and
*                                      packets assigned with this Source ID may be
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChBrgSrcIdGroupTableSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  action
)
{
    GT_U32 data;        /* Data to be written into the register */
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(sourceId > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "sourceId[%d] is out of range[0..%d]",
            sourceId,
            PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum));
    }

    data = (action == GT_TRUE) ? 1 : 0;

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        fieldOffset = portNum;

        if(portNum == CPSS_CPU_PORT_NUM_CNS)
        {
            fieldOffset = 31; /* CPU port is bit 31 in Cheetah2 */
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                              bufferMng.srcIdEggFltrTbl[sourceId];
        return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, data);
    }
    else
    {
        /* call the generic function that handle port in tables of BMP of ports in the EGF_SHT */
        return prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort(devNum,
                    CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
                    sourceId,portNum,action);
    }


}

/**
* @internal internal_cpssDxChBrgSrcIdGroupPortAdd function
* @endinternal
*
* @brief   Add a port to Source ID group. Packets assigned with this Source ID
*         may be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGroupPortAdd
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    return prvCpssDxChBrgSrcIdGroupTableSet(devNum,sourceId,portNum,GT_TRUE);
}

/**
* @internal cpssDxChBrgSrcIdGroupPortAdd function
* @endinternal
*
* @brief   Add a port to Source ID group. Packets assigned with this Source ID
*         may be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupPortAdd
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGroupPortAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourceId, portNum));

    rc = internal_cpssDxChBrgSrcIdGroupPortAdd(devNum, sourceId, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourceId, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdGroupPortDelete function
* @endinternal
*
* @brief   Delete a port from Source ID group. Packets assigned with this
*         Source ID will not be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is deleted from
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGroupPortDelete
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    return prvCpssDxChBrgSrcIdGroupTableSet(devNum,sourceId,portNum,GT_FALSE);
}

/**
* @internal cpssDxChBrgSrcIdGroupPortDelete function
* @endinternal
*
* @brief   Delete a port from Source ID group. Packets assigned with this
*         Source ID will not be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is deleted from
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupPortDelete
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGroupPortDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourceId, portNum));

    rc = internal_cpssDxChBrgSrcIdGroupPortDelete(devNum, sourceId, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourceId, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal portGroupSrcIdGroupEntrySet function
* @endinternal
*
* @brief   Set entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id . to support multi-port-groups device
* @param[in] sourceId                 - Source ID  number
* @param[in] maxPortNum               - max number of bits for the ports
* @param[in] cpuPortBit               - the bit for the CPU port
* @param[in] cpuSrcIdMember           - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[in] portsMembersPtr          - pointer to bitmap of ports that are
*                                      Source ID Members of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId, ports bitmap value
* @retval GT_BAD_PTR               - portsMembersPtr is NULL pointer
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS portGroupSrcIdGroupEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroupId,
    IN GT_U32              sourceId,
    IN GT_U32              maxPortNum,
    IN GT_U32              cpuPortBit,
    IN GT_BOOL             cpuSrcIdMember,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr
)
{
    GT_U32 regAddr;           /* register address */
    GT_U32 data;              /* data to be written into the register */
    GT_U32 cpuPortBitValue;   /* CPU port bit value*/
    GT_STATUS rc;             /* return value */

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                              bufferMng.srcIdEggFltrTbl[sourceId];


        rc = prvCpssHwPpPortGroupReadRegister(devNum,portGroupId, regAddr, &data);
        if(rc != GT_OK)
            return rc;

        /* set port bitmap */
        U32_SET_FIELD_MAC(data,0,maxPortNum,portsMembersPtr->ports[0]);

        cpuPortBitValue = (cpuSrcIdMember == GT_TRUE) ? 1 : 0;

        /* set CPU bit */
        U32_SET_FIELD_MAC(data,cpuPortBit,1,cpuPortBitValue);

        /* Write bitmap of ports are belonged to the Source ID */
        return prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId, regAddr, data);
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* check that the device supports all the physical ports that defined in the BMP */
            rc = prvCpssDxChTablePortsBmpCheck(devNum , portsMembersPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
                sourceId,
                portsMembersPtr,
                GT_TRUE,
                cpuSrcIdMember);

        return rc;

    }
}

/**
* @internal internal_cpssDxChBrgSrcIdGroupEntrySet function
* @endinternal
*
* @brief   Set entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number
* @param[in] cpuSrcIdMember           - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[in] portsMembersPtr          - pointer to the bitmap of ports that are
*                                      Source ID Members of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId, ports bitmap value
* @retval GT_BAD_PTR               - portsMembersPtr is NULL pointer
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGroupEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              sourceId,
    IN GT_BOOL             cpuSrcIdMember,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr
)
{
    GT_U32 cpuPortBit;        /* CPU port bit */
    GT_U32 maxPortNum;        /* max port number in the Source ID group */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portsMembersPtr);

    if(sourceId > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "sourceId[%d] is out of range[0..%d]",
            sourceId,
            PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum));
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        maxPortNum = 28;
        cpuPortBit = 31;

        if (portsMembersPtr->ports[0] >= ((GT_U32)(1 << maxPortNum)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        maxPortNum = 0;/* don't care */
        cpuPortBit = 0;/* don't care */
    }

    return portGroupSrcIdGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,sourceId,
            maxPortNum,cpuPortBit,cpuSrcIdMember,portsMembersPtr);
}

/**
* @internal cpssDxChBrgSrcIdGroupEntrySet function
* @endinternal
*
* @brief   Set entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number
* @param[in] cpuSrcIdMember           - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[in] portsMembersPtr          - pointer to the bitmap of ports that are
*                                      Source ID Members of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId, ports bitmap value
* @retval GT_BAD_PTR               - portsMembersPtr is NULL pointer
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              sourceId,
    IN GT_BOOL             cpuSrcIdMember,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGroupEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourceId, cpuSrcIdMember, portsMembersPtr));

    rc = internal_cpssDxChBrgSrcIdGroupEntrySet(devNum, sourceId, cpuSrcIdMember, portsMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourceId, cpuSrcIdMember, portsMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdGroupEntryGet function
* @endinternal
*
* @brief   Get entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID number
*
* @param[out] cpuSrcIdMemberPtr        - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[out] portsMembersPtr          - pointer to the bitmap of ports are Source ID Members
*                                      of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGroupEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              sourceId,
    OUT GT_BOOL             *cpuSrcIdMemberPtr,
    OUT CPSS_PORTS_BMP_STC  *portsMembersPtr
)
{
    GT_U32    regAddr;      /* register address */
    GT_U32    data;         /* data to be read from the register */
    GT_U32    cpuPortBit;   /* CPU port bit */
    GT_U32    maxPortNum;   /* max port number in the Source ID group */
    GT_STATUS rc;           /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuSrcIdMemberPtr);
    CPSS_NULL_PTR_CHECK_MAC(portsMembersPtr);

    if(sourceId > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "sourceId[%d] is out of range[0..%d]",
            sourceId,
            PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum));
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        cpssOsMemSet(portsMembersPtr, 0, sizeof(CPSS_PORTS_BMP_STC));

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                              bufferMng.srcIdEggFltrTbl[sourceId];

        rc =  prvCpssHwPpReadRegister(devNum,regAddr,&data);
        if(rc != GT_OK)
            return rc;

        maxPortNum = 28;
        cpuPortBit = 31;

        /* set port bitmap excluding CPU bit */
        U32_SET_FIELD_MASKED_MAC(portsMembersPtr->ports[0],0,maxPortNum,data);

        if(((data >> cpuPortBit) & 0x1) == 1)
        {
            *cpuSrcIdMemberPtr = GT_TRUE;
        }
        else
        {
            *cpuSrcIdMemberPtr = GT_FALSE;
        }

        return GT_OK;
    }
    else
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryRead(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
                sourceId,
                portsMembersPtr,
                GT_TRUE,
                cpuSrcIdMemberPtr);

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            if(rc != GT_OK)
            {
                return rc;
            }
            /* adjusted physical ports of BMP to hold only bmp of existing ports*/
            rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , portsMembersPtr , portsMembersPtr);
        }

        return rc;
    }
}

/**
* @internal cpssDxChBrgSrcIdGroupEntryGet function
* @endinternal
*
* @brief   Get entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID number
*
* @param[out] cpuSrcIdMemberPtr        - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[out] portsMembersPtr          - pointer to the bitmap of ports are Source ID Members
*                                      of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              sourceId,
    OUT GT_BOOL             *cpuSrcIdMemberPtr,
    OUT CPSS_PORTS_BMP_STC  *portsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGroupEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sourceId, cpuSrcIdMemberPtr, portsMembersPtr));

    rc = internal_cpssDxChBrgSrcIdGroupEntryGet(devNum, sourceId, cpuSrcIdMemberPtr, portsMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sourceId, cpuSrcIdMemberPtr, portsMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortDefaultSrcIdSet function
* @endinternal
*
* @brief   Configure Port's Default Source ID.
*         The Source ID is used for source based egress filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] defaultSrcId             - Port's Default Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortDefaultSrcIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U32       defaultSrcId
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;  /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(defaultSrcId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "defaultSrcId[%d] is out of range[0..%d]",
            defaultSrcId,
            PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(devNum));
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <SRC_ID>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_SRC_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        defaultSrcId);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        rc = prvCpssHwPpPortGroupSetRegField(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
                regAddr, 6, 5, defaultSrcId);
    }
    return rc;
}

/**
* @internal cpssDxChBrgSrcIdPortDefaultSrcIdSet function
* @endinternal
*
* @brief   Configure Port's Default Source ID.
*         The Source ID is used for source based egress filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] defaultSrcId             - Port's Default Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortDefaultSrcIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U32       defaultSrcId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortDefaultSrcIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, defaultSrcId));

    rc = internal_cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, defaultSrcId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, defaultSrcId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortDefaultSrcIdGet function
* @endinternal
*
* @brief   Get configuration of Port's Default Source ID.
*         The Source ID is used for Source based egress filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] defaultSrcIdPtr          - Port's Default Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortDefaultSrcIdGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *defaultSrcIdPtr
)
{

    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;  /* register address */
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(defaultSrcIdPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <SRC_ID>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_SRC_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        rc = prvCpssHwPpPortGroupGetRegField(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
                                                regAddr, 6, 5, &value);
    }

    if(rc != GT_OK)
        return rc;

    *defaultSrcIdPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgSrcIdPortDefaultSrcIdGet function
* @endinternal
*
* @brief   Get configuration of Port's Default Source ID.
*         The Source ID is used for Source based egress filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] defaultSrcIdPtr          - Port's Default Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortDefaultSrcIdGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *defaultSrcIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortDefaultSrcIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, defaultSrcIdPtr));

    rc = internal_cpssDxChBrgSrcIdPortDefaultSrcIdGet(devNum, portNum, defaultSrcIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, defaultSrcIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgSrcIdGlobalUcastEgressFilterSet function
* @endinternal
*
* @brief   Enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -   Source ID filtering on unicast packets.
*                                      Unicast packet is dropped if egress port is not
*                                      member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGlobalUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32  regAddr;      /* register address */
    GT_U32  data;         /* reg subfield data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    data = (enable == GT_TRUE) ? 1 : 0;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;

    return prvCpssHwPpSetRegField(devNum, regAddr, 29, 1, data);
}

/**
* @internal cpssDxChBrgSrcIdGlobalUcastEgressFilterSet function
* @endinternal
*
* @brief   Enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -   Source ID filtering on unicast packets.
*                                      Unicast packet is dropped if egress port is not
*                                      member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGlobalUcastEgressFilterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdGlobalUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE -  enable Source ID filtering on unicast packets.
*                                      Unicast packet is dropped if egress port is not
*                                      member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGlobalUcastEgressFilterGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32    regAddr;      /* register address */
    GT_U32    data;         /* reg subfield data */
    GT_STATUS rc;           /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 29, 1, &data);
    if(rc != GT_OK)
        return rc;

    *enablePtr = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;

}

/**
* @internal cpssDxChBrgSrcIdGlobalUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE -  enable Source ID filtering on unicast packets.
*                                      Unicast packet is dropped if egress port is not
*                                      member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalUcastEgressFilterGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGlobalUcastEgressFilterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - the assignment  of the packet Source ID.
*                                      CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E mode is
*                                      supported for xCat3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 data;        /* reg subfield data */
    GT_U32 fieldOffset; /* field offset */
    GT_U32 fieldLength; /* field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(mode)
    {
        /* XCAT A1 and above:
            0 = Disable: FDB source ID assignment is disabled
            1 = DA based: FDB source ID assignment is enabled for DA-based assignment
            2 = SA based: FDB source ID assignment is enabled for SA-based assignment
        */
        case CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E:
            data = (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E) ? 2 : 0;
            break;
        case CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E:
            data = (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E) ? 0 : 1;
            break;
        case CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                data = 1;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* need to set the same value in FDB and in L2I units ! */
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).
                    FDBCore.FDBGlobalConfig.FDBGlobalConfig1;
        fieldLength = 2;
        fieldOffset = 17;

        rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, data);
        if(rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
        fieldLength = 2;
        fieldOffset = 4;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* FDBbasedSrcID Assign Mode [14:13] */
        fieldLength = 2;
        fieldOffset = 13;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                         bridgeRegs.bridgeGlobalConfigRegArray[2];
    }
    else
    {
        /* UsePortDefault-SrcId [4] */
        fieldLength = 1;
        fieldOffset = 4;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                         bridgeRegs.bridgeGlobalConfigRegArray[0];
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, data);
}

/**
* @internal cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - the assignment  of the packet Source ID.
*                                      CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E mode is
*                                      supported for xCat3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - the assignment mode of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 data;        /* reg subfield data */
    GT_STATUS rc;       /* return value */
    GT_U32 fieldOffset; /* field offset */
    GT_U32 fieldLength; /* field length */
    GT_U32 data1=0;     /* reg subfield data1 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* need to set the same value in FDB and in L2I units ! */
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).
                    FDBCore.FDBGlobalConfig.FDBGlobalConfig1;
        fieldLength = 2;
        fieldOffset = 17;
        rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &data1);
        if(rc != GT_OK)
            return rc;

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
        fieldLength = 2;
        fieldOffset = 4;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* FDBbasedSrcID Assign Mode [14:13] */
        fieldLength = 2;
        fieldOffset = 13;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                         bridgeRegs.bridgeGlobalConfigRegArray[2];
    }
    else
    {
        /* UsePortDefault-SrcId [4] */
        fieldLength = 1;
        fieldOffset = 4;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                         bridgeRegs.bridgeGlobalConfigRegArray[0];
    }

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &data);
    if(rc != GT_OK)
        return rc;

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* need to set the same value in FDB and in L2I units ! */
        if(data != data1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    switch(data)
    {
        case 0:
            *modePtr = (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
                        ? CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E
                        : CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;
            break;
        case 1:
            *modePtr = (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
                        ? CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E
                        : CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E;
            break;
        default:
            *modePtr = CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E;
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - the assignment mode of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortUcastEgressFilterSet function
* @endinternal
*
* @brief   Per Egress Port enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
* @param[in] enable                   - GT_TRUE -  Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortUcastEgressFilterSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_U32  regAddr;      /* register address */
    GT_U32  data;         /* reg subfield data */
    GT_U32  fieldOffset;  /* The start bit number in the register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    data = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_UC_SRC_ID_FILTER_E,
                GT_TRUE, /*accessPhysicalPort*/
                GT_FALSE, /*accessEPort*/
                data);
    }

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* CPU port is bit 31 */
        fieldOffset = (localPort == CPSS_CPU_PORT_NUM_CNS) ? 31 : localPort;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     bridgeRegs.srcIdReg.srcIdUcEgressFilterConfReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.egr.filterConfig.ucSrcIdFilterEn[OFFSET_TO_WORD_MAC(portNum)];
        fieldOffset = OFFSET_TO_BIT_MAC(portNum);
    }

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, fieldOffset, 1, data);
}

/**
* @internal cpssDxChBrgSrcIdPortUcastEgressFilterSet function
* @endinternal
*
* @brief   Per Egress Port enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
* @param[in] enable                   - GT_TRUE -  Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortUcastEgressFilterSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortUcastEgressFilterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgSrcIdPortUcastEgressFilterSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Per Egress Port Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
*
* @param[out] enablePtr                - GT_TRUE - enable Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortUcastEgressFilterGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32  regAddr;      /* register address */
    GT_U32  data;         /* reg subfield data */
    GT_U32  fieldOffset;  /* The start bit number in the register */
    GT_STATUS rc;         /* return value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChHwEgfShtFieldGet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_UC_SRC_ID_FILTER_E,
                GT_TRUE, /*accessPhysicalPort*/
                &data);

        *enablePtr = (data == 1) ? GT_TRUE : GT_FALSE;

        return rc;
    }

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* CPU port is bit 31 */
        fieldOffset = (localPort == CPSS_CPU_PORT_NUM_CNS) ? 31 : localPort;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     bridgeRegs.srcIdReg.srcIdUcEgressFilterConfReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.egr.filterConfig.ucSrcIdFilterEn[OFFSET_TO_WORD_MAC(portNum)];
        fieldOffset = OFFSET_TO_BIT_MAC(portNum);
    }


    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, fieldOffset, 1, &data);
    if(rc != GT_OK)
        return rc;

    *enablePtr = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;

}

/**
* @internal cpssDxChBrgSrcIdPortUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Per Egress Port Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
*
* @param[out] enablePtr                - GT_TRUE - enable Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortUcastEgressFilterGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortUcastEgressFilterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgSrcIdPortUcastEgressFilterGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortSrcIdForceEnableSet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] enable                   - enable/disable SourceID force mode
*                                      GT_TRUE - enable Source ID force mode
*                                      GT_FALSE - disable Source ID force mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortSrcIdForceEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      data;        /* data to write into the register */
    GT_U32      fieldOffset; /* field offset */
    GT_U32      portGroupId; /* the port group Id - support multi-port-groups device */
    GT_U32       localPort;   /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    data = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <PORT_SOURCE_ID_FORCE_MODE>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_PORT_SOURCE_ID_FORCE_MODE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);


        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                       bridgeRegs.srcIdReg.srcIdAssignedModeConfReg;

        if(localPort == CPSS_CPU_PORT_NUM_CNS)
            fieldOffset = 31; /* CPU port is bit 31 */
        else
            fieldOffset = localPort;

        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, fieldOffset, 1, data);
    }
    return rc;
}

/**
* @internal cpssDxChBrgSrcIdPortSrcIdForceEnableSet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] enable                   - enable/disable SourceID force mode
*                                      GT_TRUE - enable Source ID force mode
*                                      GT_FALSE - disable Source ID force mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortSrcIdForceEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortSrcIdForceEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgSrcIdPortSrcIdForceEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgSrcIdPortSrcIdForceEnableGet function
* @endinternal
*
* @brief   Get Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - (pointer to) Source ID Assignment force mode state
*                                      GT_TRUE - Source ID force mode is enabled
*                                      GT_FALSE - Source ID force mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortSrcIdForceEnableGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      data;       /* data to write into the register */
    GT_U32      fieldOffset;/* field offset */
    GT_STATUS   rc;         /* returned status */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <PORT_SOURCE_ID_FORCE_MODE>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_PORT_SOURCE_ID_FORCE_MODE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                       bridgeRegs.srcIdReg.srcIdAssignedModeConfReg;

        if(localPort == CPSS_CPU_PORT_NUM_CNS)
            fieldOffset = 31; /* CPU port is bit 31 */
        else
            fieldOffset = localPort;

        rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId, regAddr, fieldOffset, 1, &data);
    }
    if (GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal cpssDxChBrgSrcIdPortSrcIdForceEnableGet function
* @endinternal
*
* @brief   Get Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - (pointer to) Source ID Assignment force mode state
*                                      GT_TRUE - Source ID force mode is enabled
*                                      GT_FALSE - Source ID force mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortSrcIdForceEnableGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortSrcIdForceEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgSrcIdPortSrcIdForceEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdEtagTypeLocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdEtagTypeLocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    if (bitLocation > 15)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig2;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 4, bitLocation);

    return rc;
}

/**
* @internal cpssDxChBrgSrcIdEtagTypeLocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdEtagTypeLocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdEtagTypeLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bitLocation));

    rc = internal_cpssDxChBrgSrcIdEtagTypeLocationSet(devNum, bitLocation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bitLocation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdEtagTypeLocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdEtagTypeLocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(bitLocationPtr);

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig2;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 4, &value);
    if (rc != GT_OK)
        return rc;

    *bitLocationPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgSrcIdEtagTypeLocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdEtagTypeLocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdEtagTypeLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bitLocationPtr));

    rc = internal_cpssDxChBrgSrcIdEtagTypeLocationGet(devNum, bitLocationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bitLocationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdAddTag1LocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates that Tag1 should be added to the.
*        NOTE: Info for 802.1cb
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*                                       (APPLICABLE VALUES : 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdAddTag1LocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if (bitLocation > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bitLocation[%d] must be less than 16 ",
            bitLocation);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 13, 4, bitLocation);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.distributorGeneralConfigs;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 4, bitLocation);

    return rc;
}

/**
* @internal cpssDxChBrgSrcIdAddTag1LocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates that Tag1 should be added to the.
*        NOTE: Info for 802.1cb
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*                                       (APPLICABLE VALUES : 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdAddTag1LocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdAddTag1LocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bitLocation));

    rc = internal_cpssDxChBrgSrcIdAddTag1LocationSet(devNum, bitLocation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bitLocation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdAddTag1LocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates that Tag1 should be added to the.
*        NOTE: Info for 802.1cb
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdAddTag1LocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;
    GT_U32      value2;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(bitLocationPtr);

    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 13, 4, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.distributorGeneralConfigs;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 30, 4, &value2);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(value2 != value)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "value [%d] from QAG differ from value [%d] from SMU",
            value2,value);
    }

    *bitLocationPtr = value;

    return GT_OK;
}

/**
* @internal cpssDxChBrgSrcIdAddTag1LocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates that Tag1 should be added to the.
*        NOTE: Info for 802.1cb
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdAddTag1LocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdAddTag1LocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bitLocationPtr));

    rc = internal_cpssDxChBrgSrcIdAddTag1LocationGet(devNum, bitLocationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bitLocationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




/**
* @internal internal_cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable Source ID filtering for odd Source-ID values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[in] enable                   - GT_FALSE - Disable drop for odd source ID values.
*                                       GT_TRUE - Drop packets with odd source ID values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    return prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ODD_ONLY_SRC_ID_FILTER_ENABLE_E,
            GT_TRUE, /*accessPhysicalPort*/
            GT_TRUE,  /*accessEPort*/
            BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable Source ID filtering for odd Source-ID values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[in] enable                   - GT_FALSE - Disable drop for odd source ID values.
*                                       GT_TRUE - Drop packets with odd source ID values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of Source ID filtering for odd Source-ID values .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - Disable drop for odd source ID values.
*                                      GT_TRUE - Drop packets with odd source ID values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChHwEgfShtFieldGet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ODD_ONLY_SRC_ID_FILTER_ENABLE_E,
            GT_TRUE, /*accessPhysicalPort*/
            &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of Source ID filtering for odd Source-ID values .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - Disable drop for odd source ID values.
*                                      GT_TRUE - Drop packets with odd source ID values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdTargetIndexConfigSet function
* @endinternal
*
* @brief   Set Target Index related configuration.
*
* @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] configPtr                - (pointer to) Target Index related configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdTargetIndexConfigSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC *configPtr
)
{
    GT_U32 data;
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    if (configPtr->srcIdLength > 6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "srcIdLength[%d] is out of range[0..6]", configPtr->srcIdLength);
    }
    if (configPtr->trgDevMuxLength > 6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "trgDevtMuxLength[%d] is out of range[0..6]", configPtr->trgDevMuxLength);
    }
    if (configPtr->trgEportMuxLength > 6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "trgEportMuxLength[%d] is out of range[0..6]", configPtr->trgEportMuxLength);
    }
    if (configPtr->trgEportMuxLength + configPtr->trgDevMuxLength > configPtr->srcIdLength)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "srcIdLength[%d] less than mux bits {%d,%d}",
            configPtr->srcIdLength, configPtr->trgDevMuxLength, configPtr->trgEportMuxLength);
    }
    if (configPtr->srcIdMsb > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "srcIdMsb[%d] is out of range[0..11]", configPtr->srcIdMsb);
    }
    if (configPtr->srcIdLength > (configPtr->srcIdMsb + 1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "srcIdMsb[%d] and srcIdLength[%d] out of bounds",
            configPtr->srcIdMsb, configPtr->srcIdLength);
    }

    data = (configPtr->srcIdMsb << 3) | configPtr->srcIdLength;

    /*HA Global Configuration 2 Register*/
    /*<Src ID len> 8:6*/
    /*<Src ID msb> 12:9*/
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig2;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 7, data);
    if (rc != GT_OK)
        return rc;

    /* EPCL global cfg reg
    18-20 srcIdLen
    21-24 srcIdMsb
    */
    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 18, 7, data);
    if (rc != GT_OK)
        return rc;

    /* IPCL global cfg reg
    20-22 srcIdLen
    23-26 srcIdMsb
    */
    regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).ingrPolicyGlobalConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 20, 7, data);
    if (rc != GT_OK)
        return rc;

    /*set scalable SGT reg for routed data*/
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.scalableSgtSupport;

    data = (configPtr->trgDevMuxLength & 0x7)          |/*0:2 Trg Dev Mux Len*/
           ((configPtr->trgEportMuxLength & 0x7) << 3)  |/*5:3 Trg ePort Mux Len*/
           ((configPtr->srcIdMsb & 0xf) << 6 );          /*6:9 Src ID msb*/

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, data);

    return rc;

}

/**
* @internal cpssDxChBrgSrcIdTargetIndexConfigSet function
* @endinternal
*
* @brief   Set Target Index related configuration.
*
* @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] configPtr                - (pointer to) Target Index related configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTargetIndexConfigSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC *configPtr
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdTargetIndexConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configPtr));

    rc = internal_cpssDxChBrgSrcIdTargetIndexConfigSet(devNum, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdTargetIndexConfigSet function
* @endinternal
*
* @brief   Get Target Index related configuration.
*
* @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] configPtr                - (pointer to) Target Index related configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on different src-id msb in SGT and HA regs
*/
static GT_STATUS internal_cpssDxChBrgSrcIdTargetIndexConfigGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC *configPtr
)
{
    GT_U32 data;
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    /*HA Global Configuration 2 Register*/
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig2;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 7, &data);

    if (rc != GT_OK)
        return rc;
    /*<Src ID len> 8:6*/
    configPtr->srcIdLength = data & 0x7;
    /*<Src ID msb> 12:9*/
    configPtr->srcIdMsb = (data >> 0x3) & 0xf;

    /*Get scalable SGT reg */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.scalableSgtSupport;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 10, &data);
    if (rc != GT_OK)
        return rc;

    /*6:9 Src ID msb*/
    if (configPtr->srcIdMsb != ((data >> 0x6) & 0xf ))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "srcIdMsb[%d] is out of sync ", configPtr->srcIdMsb);
    }
    /*5:3 Trg ePort Mux Len*/
    configPtr->trgEportMuxLength = (data >> 0x3) & 0x7;
    /*0:2 Trg Dev Mux Len*/
    configPtr->trgDevMuxLength = data & 0x7;

    return rc;

}
/**
* @internal cpssDxChBrgSrcIdTargetIndexConfigSet function
* @endinternal
*
* @brief   Get Target Index related configuration.
*
* @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] configPtr                - (pointer to) Target Index related configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTargetIndexConfigGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC *configPtr
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdTargetIndexConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configPtr));

    rc = internal_cpssDxChBrgSrcIdTargetIndexConfigGet(devNum, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgSrcIdPortTargetConfigSet function
* @endinternal
*
* @brief   Configure Target Physical Port's Target Source ID assignment.
*          Target Index is in Source ID field bits defined by cpssDxChBrgSrcIdTargetIndexConfigSet.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable assignment of Target Index per target physical port
*                                       GT_FALSE - disable assignment
*                                       GT_TRUE  - enable assignment
* @param[in] trgIndex                 - Port's Target Index.
*                                       APPLICABLE RANGES 1..63
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - wrong  trgIndex
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortTargetConfigSet
(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       trgIndex
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /*HA Physical Port Table 1*/
    if (((trgIndex < 1) && enable) || (trgIndex > 63))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "trgIndex[%d] is out of range[1..63]", trgIndex);
    }
    /*51 Set Src-ID Set target port <Src-ID> into the source ID bits*/
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            enable);

    if (rc != GT_OK)
        return rc;

    /*57:52 Src-ID Source ID value*/
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            trgIndex);

    return rc;
}
/**
* @internal cpssDxChBrgSrcIdPortTargetConfigSet function
* @endinternal
*
* @brief   Configure Target Physical Port's Target Source ID assignment.
*          Target Index is in Source ID field bits defined by cpssDxChBrgSrcIdTargetIndexConfigSet.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable assignment of Target Index per target physical port
*                                       GT_FALSE - disable assignment
*                                       GT_TRUE  - enable assignment
* @param[in] trgIndex                 - Port's Target Index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, trgIndex
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortTargetConfigSet
(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       trgIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortTargetConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable, trgIndex));

    rc = internal_cpssDxChBrgSrcIdPortTargetConfigSet(devNum, portNum, enable, trgIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable, trgIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgSrcIdPortTargetConfigGet function
* @endinternal
*
* @brief   Get Target Physical Port's Target Source ID assignment.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[out] enablePtr               - (pointer to) enable/disable assignment of Target Index per target physical port
*                                       GT_FALSE - disable assignment
*                                       GT_TRUE  - enable assignment
* @param[out] trgIndexPtr              - (pointer to) Port's Target Index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdPortTargetConfigGet
(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL    *enablePtr,
    OUT GT_U32     *trgIndexPtr
)
{
    GT_STATUS rc;
    GT_U32  hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(trgIndexPtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /*HA Physical Port Table 1*/
    /*51 Set Src-ID Set target port <Src-ID>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    if (rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwValue);

    /*57:52 Src-ID Source ID value*/
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            trgIndexPtr);

    return rc;
}
/**
* @internal cpssDxChBrgSrcIdPortTargetConfigGet function
* @endinternal
*
* @brief   Get Target Physical Port's Target Source ID assignment.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[out] enablePtr               - (pointer to) enable/disable assignment of Target Index per target physical port
*                                       GT_FALSE - disable assignment
*                                       GT_TRUE  - enable assignment
* @param[out] trgIndexPtr              - (pointer to) Port's Target Index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortTargetConfigGet
(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL    *enablePtr,
    OUT GT_U32     *trgIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdPortTargetConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr, trgIndexPtr));

    rc = internal_cpssDxChBrgSrcIdPortTargetConfigGet(devNum, portNum, enablePtr, trgIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr, trgIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal_cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet function
* @endinternal
*
* @brief   Set Do Not Override FROM_CPU Source-ID mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable Do Not Override FROM_CPU Source-ID mode
*                                      GT_TRUE - enable Do Not Override FROM_CPU Source-ID mode
*                                      GT_FALSE - disable Do Not Override FROM_CPU Source-ID mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32     regAddr;     /* register address */
    GT_U32     data;        /* reg subfield data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    data = (enable == GT_TRUE) ? 1 : 0;
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeGlobalConfig0;

    return prvCpssHwPpSetRegField(devNum, regAddr, 24, 1, data);
}

/**
* @internal cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet function
* @endinternal
*
* @brief   Set Do Not Override FROM_CPU Source-ID mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable Do Not Override FROM_CPU Source-ID mode
*                                      GT_TRUE - enable Do Not Override FROM_CPU Source-ID mode
*                                      GT_FALSE - disable Do Not Override FROM_CPU Source-ID mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal_cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of Do Not Override FROM_CPU Source-ID mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr               - (pointer to) enable/disable status of Do Not Override FROM_CPU Source-ID mode
*                                      GT_TRUE - Do Not Override FROM_CPU Source-ID mode is enabled
*                                      GT_FALSE - Do Not Override FROM_CPU Source-ID mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32    regAddr;      /* register address */
    GT_U32    data;         /* reg subfield data */
    GT_STATUS rc;           /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeGlobalConfig0;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 24, 1, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;

}

/**
* @internal cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of Do Not Override FROM_CPU Source-ID mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr               - (pointer to) enable/disable status of Do Not Override FROM_CPU Source-ID mode
*                                      GT_TRUE - Do Not Override FROM_CPU Source-ID mode is enabled
*                                      GT_FALSE - Do Not Override FROM_CPU Source-ID mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet function
* @endinternal
*
* @brief  Enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by the L2MLL.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by L2MLL
*                                      GT_TRUE - If packet was replicated by MLL, Source-ID Egress Filtering considers the
*                                      packet as multicast (even if target is unicast).
*                                      GT_FALSE - If packet has single target destination, Source-ID Egress Filtering treats
*                                      packet as unicast (even if replicated by MLL).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32     regAddr;     /* register address */
    GT_U32     data;        /* reg subfield data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    data = (enable == GT_TRUE) ? 1 : 0;
    regAddr = PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.SHTGlobalConfigs;

    return prvCpssHwPpSetRegField(devNum, regAddr, 28, 1, data);
}

/**
* @internal cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet function
* @endinternal
*
* @brief  Enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by the L2MLL.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by L2MLL
*                                      GT_TRUE - If packet was replicated by MLL, Source-ID Egress Filtering considers the
*                                      packet as multicast (even if target is unicast).
*                                      GT_FALSE - If packet has single target destination, Source-ID Egress Filtering treats
*                                      packet as unicast (even if replicated by MLL).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet function
* @endinternal
*
* @brief  Get status of enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by the L2MLL.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr               - (pointer to) status of Source-ID Egress Filtering for all the multi-destination traffic replicated by L2MLL
*                                      GT_TRUE - If packet was replicated by MLL, Source-ID Egress Filtering considers the
*                                      packet as multicast (even if target is unicast).
*                                      GT_FALSE - If packet has single target destination, Source-ID Egress Filtering treats
*                                      packet as unicast (even if replicated by MLL).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32    regAddr;      /* register address */
    GT_U32    data;         /* reg subfield data */
    GT_STATUS rc;           /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.SHTGlobalConfigs;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 28, 1, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet function
* @endinternal
*
* @brief  Get status of enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by the L2MLL.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr               - (pointer to) status of Source-ID Egress Filtering for all the multi-destination traffic replicated by L2MLL
*                                      GT_TRUE - If packet was replicated by MLL, Source-ID Egress Filtering considers the
*                                      packet as multicast (even if target is unicast).
*                                      GT_FALSE - If packet has single target destination, Source-ID Egress Filtering treats
*                                      packet as unicast (even if replicated by MLL).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

