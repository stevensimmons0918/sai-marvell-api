/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChBrgPortIsolation.c
*
* DESCRIPTION:
*       CPSS DxCh NST Port Isolation Mechanism.
*
* FILE REVISION NUMBER:
*       $Revision: 23 $
*******************************************************************************/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/nst/private/prvCpssDxChNstLog.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* width of port isolation table -- number of words */
#define PORT_ISOLATION_NUM_WORDS_CNS    8

/**
* @internal portIsolationEntryIndexCalc function
* @endinternal
*
* @brief   Function calculates port isolation table index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] srcInterfacePtr          - (pointer to) source interface information
*
* @param[out] indexPtr                 - (pointer to) port isolation table index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum, portNum, trankId, vladId
*/
static GT_STATUS portIsolationEntryIndexCalc
(
    IN GT_U8                                          devNum,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    OUT GT_U32                                        *indexPtr
)
{
    GT_U32 hwDev, hwPort;

    devNum = devNum;

    /* Port isolation table index calculation */
    switch (srcInterfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:

            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(srcInterfacePtr->devPort.hwDevNum,srcInterfacePtr->devPort.portNum);
            hwDev =  PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(srcInterfacePtr->devPort.hwDevNum,srcInterfacePtr->devPort.portNum);
            hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(srcInterfacePtr->devPort.hwDevNum,srcInterfacePtr->devPort.portNum);

            if ((hwDev > BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupDeviceBits)) ||
                (hwPort > BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupPortBits)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "hwDev[%d] more than supported[%d] or hwPort[%d] more than supported[%d]",
                    hwDev,BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupDeviceBits),
                    hwPort,BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupPortBits));
            }

            *indexPtr = (hwDev << PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupPortBits) | (hwPort);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            if (srcInterfacePtr->trunkId > BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupTrunkBits))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "trunkId[%d] more than supported[%d] ",
                    srcInterfacePtr->trunkId,BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupTrunkBits));
            }

            *indexPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupTrunkIndexBase+
                        srcInterfacePtr->trunkId;

            break;
        case CPSS_INTERFACE_VID_E:
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                if (srcInterfacePtr->vlanId < 4096)
                {
                    *indexPtr = srcInterfacePtr->vlanId;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "eVLAN[0x%4.4x] >= 4K",
                                                  srcInterfacePtr->vlanId);
                }
            }
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "not supported interface type[%d]",
                                              srcInterfacePtr->type);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "not supported interface type[%d]",
                srcInterfacePtr->type);
    }

    CPSS_LOG_INFORMATION_MAC("Calculated port isolation index [%d] \n",
        (*indexPtr ));

    return GT_OK;
}

/**
* @internal internal_cpssDxChNstPortIsolationEnableSet function
* @endinternal
*
* @brief   Function enables/disables the port isolation feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - port isolation feature enable/disable
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersEnable;
        return prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, BOOL2BIT_MAC(enable));
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* Address of Transmit Queue Resource Sharing Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueResSharingAndTunnelEgrFltr;
        return prvCpssDrvHwPpSetRegField(devNum, regAddr, 28, 1, BOOL2BIT_MAC(enable));
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.
                filterConfig.globalEnables;
        return prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, BOOL2BIT_MAC(enable));
    }
}

/**
* @internal cpssDxChNstPortIsolationEnableSet function
* @endinternal
*
* @brief   Function enables/disables the port isolation feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - port isolation feature enable/disable
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChNstPortIsolationEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationEnableGet function
* @endinternal
*
* @brief   Function gets enabled/disabled state of the port isolation feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) port isolation feature state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32      hwValue;    /* register value */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersEnable;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 1, &hwValue);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* Address of Transmit Queue Resource Sharing Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueResSharingAndTunnelEgrFltr;

        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 28, 1, &hwValue);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.
                filterConfig.globalEnables;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 7, 1, &hwValue);
    }

    /* Convert return value to BOOLEAN type */
    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;

}

/**
* @internal cpssDxChNstPortIsolationEnableGet function
* @endinternal
*
* @brief   Function gets enabled/disabled state of the port isolation feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) port isolation feature state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChNstPortIsolationEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationTableEntrySet function
* @endinternal
*
* @brief   Function sets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking. If it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interface.
*                                      Only portDev and Trunk are supported.
* @param[in] cpuPortMember            - port isolation for CPU Port
*                                      GT_TRUE - member
*                                      GT_FALSE - not member
* @param[in] localPortsMembersPtr     - (pointer to) port bitmap to be written to the
*                                      L2/L3 PI table
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
static GT_STATUS internal_cpssDxChNstPortIsolationTableEntrySet
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    IN GT_BOOL                                        cpuPortMember,
    IN CPSS_PORTS_BMP_STC                             *localPortsMembersPtr
)
{
    GT_U32                  hwValue;    /* register value */
    GT_U32                  maxPortNum;
    GT_STATUS               rc;         /* return status */
    GT_U32                  tblIndex;   /*table index*/
    CPSS_DXCH_TABLE_ENT     tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(localPortsMembersPtr);

    /* calculate index */
    rc = portIsolationEntryIndexCalc(devNum, srcInterfacePtr, &tblIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(trafficType)
    {
        case CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E:
            tableType = CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E:
            tableType = CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* check port bitmap */
        maxPortNum = PRV_CPSS_PP_MAC(devNum)->numOfPorts;

        if ((localPortsMembersPtr->ports[0] >= ((GT_U32)(1 << maxPortNum))) || (localPortsMembersPtr->ports[1]))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* combine register value from port bitmap and cpu port */
        hwValue = (localPortsMembersPtr->ports[0]) | (BOOL2BIT_MAC(cpuPortMember) << 28);

        return prvCpssDxChWriteTableEntryField(devNum,
                tableType, tblIndex, 0, 0, 29, hwValue);
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* check that the device supports all the physical ports that defined in the BMP */
            rc = prvCpssDxChTablePortsBmpLimitedNumCheck(devNum ,
                (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 128 :256)/* limited to 128 ports in falcon and 256 ports in BC3 !*/,
                localPortsMembersPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* call the generic function that handle BMP of ports in the EGF_SHT */
        return prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
                tableType,tblIndex,localPortsMembersPtr,
                GT_TRUE,cpuPortMember);
    }
}

/**
* @internal cpssDxChNstPortIsolationTableEntrySet function
* @endinternal
*
* @brief   Function sets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking. If it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interface.
*                                      Only portDev and Trunk are supported.
* @param[in] cpuPortMember            - port isolation for CPU Port
*                                      GT_TRUE - member
*                                      GT_FALSE - not member
* @param[in] localPortsMembersPtr     - (pointer to) port bitmap to be written to the
*                                      L2/L3 PI table
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
GT_STATUS cpssDxChNstPortIsolationTableEntrySet
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    IN GT_BOOL                                        cpuPortMember,
    IN CPSS_PORTS_BMP_STC                             *localPortsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trafficType, srcInterfacePtr, cpuPortMember, localPortsMembersPtr));

    rc = internal_cpssDxChNstPortIsolationTableEntrySet(devNum, trafficType, srcInterfacePtr, cpuPortMember, localPortsMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trafficType, srcInterfacePtr, cpuPortMember, localPortsMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationTableEntryGet function
* @endinternal
*
* @brief   Function gets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking if it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interfaces
*                                      Only portDev and Trunk are supported.
*
* @param[out] cpuPortMemberPtr         - (pointer to) port isolation for CPU Port
*                                      GT_TRUE - member
*                                      GT_FALSE - not member
* @param[out] localPortsMembersPtr     - (pointer to) port bitmap to be written
*                                      to the L2/L3 PI table
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
static GT_STATUS internal_cpssDxChNstPortIsolationTableEntryGet
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    OUT GT_BOOL                                       *cpuPortMemberPtr,
    OUT CPSS_PORTS_BMP_STC                            *localPortsMembersPtr
)
{
    GT_U32              hwValue;    /* register value   */
    GT_STATUS           rc;         /* return status    */
    GT_U32              tblIndex;   /* table index      */
    CPSS_DXCH_TABLE_ENT tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(localPortsMembersPtr);
    CPSS_NULL_PTR_CHECK_MAC(cpuPortMemberPtr);

    cpssOsMemSet(localPortsMembersPtr, 0,sizeof(*localPortsMembersPtr));

    /* calculate index */
    rc = portIsolationEntryIndexCalc(devNum,srcInterfacePtr,&tblIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(trafficType)
    {
        case CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E:
            tableType = CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E:
            tableType = CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntry(devNum,
                tableType, tblIndex, &hwValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        localPortsMembersPtr->ports[0] = hwValue & 0x0FFFFFFF;

        *cpuPortMemberPtr = BIT2BOOL_MAC((hwValue >> 28) & 1);
    }
    else
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryRead(devNum,
                tableType,tblIndex,localPortsMembersPtr,
                GT_TRUE,cpuPortMemberPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* adjusted physical ports of BMP to hold only bmp of existing ports*/
            rc = prvCpssDxChTablePortsBmpAdjustToDeviceLimitedNum(devNum ,
                256/* limited to 256 ports!*/,
                localPortsMembersPtr , localPortsMembersPtr);
        }
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChNstPortIsolationTableEntryGet function
* @endinternal
*
* @brief   Function gets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking if it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interfaces
*                                      Only portDev and Trunk are supported.
*
* @param[out] cpuPortMemberPtr         - (pointer to) port isolation for CPU Port
*                                      GT_TRUE - member
*                                      GT_FALSE - not member
* @param[out] localPortsMembersPtr     - (pointer to) port bitmap to be written
*                                      to the L2/L3 PI table
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
GT_STATUS cpssDxChNstPortIsolationTableEntryGet
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    OUT GT_BOOL                                       *cpuPortMemberPtr,
    OUT CPSS_PORTS_BMP_STC                            *localPortsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trafficType, srcInterfacePtr, cpuPortMemberPtr, localPortsMembersPtr));

    rc = internal_cpssDxChNstPortIsolationTableEntryGet(devNum, trafficType, srcInterfacePtr, cpuPortMemberPtr, localPortsMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trafficType, srcInterfacePtr, cpuPortMemberPtr, localPortsMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal portIsolationPortSet function
* @endinternal
*
* @brief   Function set (add/remove) single local port in port isolation table entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interface
*                                      Only portDev and Trunk are supported.
* @param[in] portNum                  - local port(include CPU port) to be added to bitmap
* @param[in] enable                   - GT_TRUE - add the port
*                                      GT_FALSE - remove the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface, portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS portIsolationPortSet
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum,
    IN GT_BOOL                                        enable
)
{
    GT_U32                  offset;     /* offset */
    GT_STATUS               rc;         /* return status */
    GT_U32                  localPort;  /* local port - support multi-port-groups device */
    GT_U32                  tblIndex;   /* port isolation table index */
    CPSS_DXCH_TABLE_ENT     tableType;
    GT_U32                  data;       /* data to write to the HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    data = BOOL2BIT_MAC(enable);

    /* calculate index */
    rc = portIsolationEntryIndexCalc(devNum,srcInterfacePtr,&tblIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(trafficType)
    {
        case CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E:
            tableType = CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E:
            tableType = CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* check port */
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        /* calculate bit's offset */
        offset = (localPort == CPSS_CPU_PORT_NUM_CNS)?28:localPort;

        /* set relevant bit in the bitmap */
        return prvCpssDxChWriteTableEntryField(devNum, tableType, tblIndex, 0, offset, 1, data);
    }
    else
    {
        /* call the generic function that handle port in tables of BMP of ports in the EGF_SHT */
        return prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort(devNum,
                    tableType,tblIndex,portNum,enable);
    }
}

/**
* @internal internal_cpssDxChNstPortIsolationPortAdd function
* @endinternal
*
* @brief   Function adds single local port to port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Adding local port (may be also CPU port 63) to port isolation entry
*         means that traffic which came from srcInterface and wish to egress
*         at the specified local port isn't blocked.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interface
*                                      Only portDev and Trunk are supported.
* @param[in] portNum                  - local port(include CPU port) to be added to bitmap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface, portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationPortAdd
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                       *srcInterfacePtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum
)
{
    return portIsolationPortSet(devNum,trafficType,srcInterfacePtr,portNum,
        GT_TRUE);
}

/**
* @internal cpssDxChNstPortIsolationPortAdd function
* @endinternal
*
* @brief   Function adds single local port to port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Adding local port (may be also CPU port 63) to port isolation entry
*         means that traffic which came from srcInterface and wish to egress
*         at the specified local port isn't blocked.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - table index is calculated from source interface
*                                      Only portDev and Trunk are supported.
* @param[in] portNum                  - local port(include CPU port) to be added to bitmap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface, portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationPortAdd
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                       *srcInterfacePtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationPortAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trafficType, srcInterfacePtr, portNum));

    rc = internal_cpssDxChNstPortIsolationPortAdd(devNum, trafficType, srcInterfacePtr, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trafficType, srcInterfacePtr, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationPortDelete function
* @endinternal
*
* @brief   Function deletes single local port to port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Deleting local port (may be also CPU port 63) to port isolation entry
*         means that traffic which came from srcInterface and wish to egress
*         at the specified local port is blocked.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - table index is calculated from source interface
*                                      Only portDev and Trunk are supported.
* @param[in] portNum                  - local port(include CPU port) to be deleted from bitmap
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
static GT_STATUS internal_cpssDxChNstPortIsolationPortDelete
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum
)
{
    return portIsolationPortSet(devNum,trafficType,srcInterfacePtr,portNum,
        GT_FALSE);
}

/**
* @internal cpssDxChNstPortIsolationPortDelete function
* @endinternal
*
* @brief   Function deletes single local port to port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Deleting local port (may be also CPU port 63) to port isolation entry
*         means that traffic which came from srcInterface and wish to egress
*         at the specified local port is blocked.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterfacePtr          - table index is calculated from source interface
*                                      Only portDev and Trunk are supported.
* @param[in] portNum                  - local port(include CPU port) to be deleted from bitmap
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
GT_STATUS cpssDxChNstPortIsolationPortDelete
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationPortDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trafficType, srcInterfacePtr, portNum));

    rc = internal_cpssDxChNstPortIsolationPortDelete(devNum, trafficType, srcInterfacePtr, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trafficType, srcInterfacePtr, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationModeSet function
* @endinternal
*
* @brief   Set Port Isolation feature mode in the TxQ per egress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - Port Isolation Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationModeSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT    mode
)
{
    GT_U32      convMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch(mode)
    {
        case CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E:
            convMode = 0;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E:
            convMode = 1;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E:
            convMode = 2;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E:
            convMode = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
        PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_PORT_ISOLATION_MODE_E,
            GT_TRUE, /*accessPhysicalPort*/
            GT_TRUE, /*accessEPort*/
            convMode);
}

/**
* @internal cpssDxChNstPortIsolationModeSet function
* @endinternal
*
* @brief   Set Port Isolation feature mode in the TxQ per egress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - Port Isolation Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationModeSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChNstPortIsolationModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationModeGet function
* @endinternal
*
* @brief   Get Port Isolation feature mode in the TxQ per egress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - (pointer to) Port Isolation Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_NUM                             portNum,
    OUT CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;
    GT_U32      convMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChHwEgfShtFieldGet(devNum,portNum,
        PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_PORT_ISOLATION_MODE_E,
            GT_FALSE, /*accessPhysicalPort*/
            &convMode);

     switch(convMode)
     {
        case 0:
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E;
            break;
        case 1:
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E;
            break;
        case 2:
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E;
            break;
        case 3:
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     }

    return rc;
}

/**
* @internal cpssDxChNstPortIsolationModeGet function
* @endinternal
*
* @brief   Get Port Isolation feature mode in the TxQ per egress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - (pointer to) Port Isolation Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_NUM                             portNum,
    OUT CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChNstPortIsolationModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationLookupBitsSet function
* @endinternal
*
* @brief   Set the number of bits from the source Interface that are used to
*         index the port isolation table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] numberOfPortBits         - number of least significant bits of src port (APPLICABLE RANGES: 0..15)
* @param[in] numberOfDeviceBits       - number of least significant bits of src device (APPLICABLE RANGES: 0..15)
* @param[in] numberOfTrunkBits        - number of least significant bits of src trunk  (APPLICABLE RANGES: 0..15)
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
static GT_STATUS internal_cpssDxChNstPortIsolationLookupBitsSet
(
    IN GT_U8    devNum,
    IN GT_U32   numberOfPortBits,
    IN GT_U32   numberOfDeviceBits,
    IN GT_U32   numberOfTrunkBits
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      regValue;
    GT_U32      startBit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(numberOfPortBits   >= BIT_5 ||
       numberOfDeviceBits >= BIT_4 ||
       numberOfTrunkBits  >= BIT_4 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regValue = numberOfTrunkBits | (numberOfDeviceBits << 4);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup0;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 21, 8, regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    regValue = numberOfPortBits;

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        startBit = 2;
    }
    else
    {
        startBit = 1;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup1;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, startBit, 5, regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* save to database */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupPortBits = numberOfPortBits;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupDeviceBits = numberOfDeviceBits;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupTrunkBits = numberOfTrunkBits;

    return GT_OK;
}

/**
* @internal cpssDxChNstPortIsolationLookupBitsSet function
* @endinternal
*
* @brief   Set the number of bits from the source Interface that are used to
*         index the port isolation table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] numberOfPortBits         - number of least significant bits of src port (APPLICABLE RANGES: 0..15)
* @param[in] numberOfDeviceBits       - number of least significant bits of src device (APPLICABLE RANGES: 0..15)
* @param[in] numberOfTrunkBits        - number of least significant bits of src trunk  (APPLICABLE RANGES: 0..15)
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
GT_STATUS cpssDxChNstPortIsolationLookupBitsSet
(
    IN GT_U8    devNum,
    IN GT_U32   numberOfPortBits,
    IN GT_U32   numberOfDeviceBits,
    IN GT_U32   numberOfTrunkBits
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationLookupBitsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits));

    rc = internal_cpssDxChNstPortIsolationLookupBitsSet(devNum, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numberOfPortBits, numberOfDeviceBits, numberOfTrunkBits));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationLookupBitsGet function
* @endinternal
*
* @brief   Get the number of bits from the source Interface that are used to
*         index the port isolation table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] numberOfPortBitsPtr      - (pointer to)number of least significant bits of src port (APPLICABLE RANGES: 0..15)
* @param[out] numberOfDeviceBitsPtr    - (pointer to)number of least significant bits of src device (APPLICABLE RANGES: 0..15)
* @param[out] numberOfTrunkBitsPtr     - (pointer to)number of least significant bits of src trunk  (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationLookupBitsGet
(
    IN GT_U8    devNum,
    OUT GT_U32   *numberOfPortBitsPtr,
    OUT GT_U32   *numberOfDeviceBitsPtr,
    OUT GT_U32   *numberOfTrunkBitsPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      regValue;
    GT_U32      startBit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(numberOfPortBitsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numberOfDeviceBitsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numberOfTrunkBitsPtr);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup0;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 21, 8, &regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    *numberOfTrunkBitsPtr  = U32_GET_FIELD_MAC(regValue,0,4);
    *numberOfDeviceBitsPtr = U32_GET_FIELD_MAC(regValue,4,4);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup1;
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        startBit = 2;
    }
    else
    {
        startBit = 1;
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 5, &regValue);

    *numberOfPortBitsPtr    = regValue;

    return rc;
}

/**
* @internal cpssDxChNstPortIsolationLookupBitsGet function
* @endinternal
*
* @brief   Get the number of bits from the source Interface that are used to
*         index the port isolation table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] numberOfPortBitsPtr      - (pointer to)number of least significant bits of src port (APPLICABLE RANGES: 0..15)
* @param[out] numberOfDeviceBitsPtr    - (pointer to)number of least significant bits of src device (APPLICABLE RANGES: 0..15)
* @param[out] numberOfTrunkBitsPtr     - (pointer to)number of least significant bits of src trunk  (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationLookupBitsGet
(
    IN GT_U8    devNum,
    OUT GT_U32   *numberOfPortBitsPtr,
    OUT GT_U32   *numberOfDeviceBitsPtr,
    OUT GT_U32   *numberOfTrunkBitsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationLookupBitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numberOfPortBitsPtr, numberOfDeviceBitsPtr, numberOfTrunkBitsPtr));

    rc = internal_cpssDxChNstPortIsolationLookupBitsGet(devNum, numberOfPortBitsPtr, numberOfDeviceBitsPtr, numberOfTrunkBitsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numberOfPortBitsPtr, numberOfDeviceBitsPtr, numberOfTrunkBitsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationOnEportsEnableSet function
* @endinternal
*
* @brief   Determines how the port isolation is performed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: port isolation is performed based on the source ePort.
*                                      GT_FALSE: port isolation is performed based on the source physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationOnEportsEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  indexingMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(enable)
    {
        indexingMode = CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EPORT_E;
    }
    else
    {
        indexingMode = CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_PHYSICAL_PORT_E;
    }

    return cpssDxChNstPortIsolationIndexingModeSet(devNum, indexingMode);
}

/**
* @internal cpssDxChNstPortIsolationOnEportsEnableSet function
* @endinternal
*
* @brief   Determines how the port isolation is performed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: port isolation is performed based on the source ePort.
*                                      GT_FALSE: port isolation is performed based on the source physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationOnEportsEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationOnEportsEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChNstPortIsolationOnEportsEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationOnEportsEnableGet function
* @endinternal
*
* @brief   Returns how the port isolation is performed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: port isolation is performed based on the source ePort.
*                                      GT_FALSE: port isolation is performed based on the source physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationOnEportsEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  indexingMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = cpssDxChNstPortIsolationIndexingModeGet(devNum, &indexingMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (indexingMode == CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EPORT_E)
    {
        *enablePtr = GT_TRUE;
    }
    else if (indexingMode == CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_PHYSICAL_PORT_E)
    {
        *enablePtr = GT_FALSE;
    }
    else
    {
        /* the indexing mode is not based on Eport or Physical port */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChNstPortIsolationOnEportsEnableGet function
* @endinternal
*
* @brief   Returns how the port isolation is performed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: port isolation is performed based on the source ePort.
*                                      GT_FALSE: port isolation is performed based on the source physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationOnEportsEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationOnEportsEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChNstPortIsolationOnEportsEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationLookupTrunkIndexBaseSet function
* @endinternal
*
* @brief   Determines the first index of the trunk ID based lookup.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkIndexBase           - the first index of the trunk ID based lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_OUT_OF_RANGE          - on trunkIndexBase out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The default value is 2048 (0x800) for backward compatibility.
*
*/
static GT_STATUS internal_cpssDxChNstPortIsolationLookupTrunkIndexBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   trunkIndexBase
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (trunkIndexBase >= BIT_21)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup0;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 21, trunkIndexBase);
    if(rc != GT_OK)
        return rc;

     /* set Port data base */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portIsolationLookupTrunkIndexBase = trunkIndexBase;

    return rc;
}

/**
* @internal cpssDxChNstPortIsolationLookupTrunkIndexBaseSet function
* @endinternal
*
* @brief   Determines the first index of the trunk ID based lookup.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkIndexBase           - the first index of the trunk ID based lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_OUT_OF_RANGE          - on trunkIndexBase out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The default value is 2048 (0x800) for backward compatibility.
*
*/
GT_STATUS cpssDxChNstPortIsolationLookupTrunkIndexBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   trunkIndexBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationLookupTrunkIndexBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkIndexBase));

    rc = internal_cpssDxChNstPortIsolationLookupTrunkIndexBaseSet(devNum, trunkIndexBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkIndexBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationLookupTrunkIndexBaseGet function
* @endinternal
*
* @brief   Returns the first index of the trunk ID based lookup.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] trunkIndexBasePtr        - (pointer to) the first index of the trunk ID based lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIsolationLookupTrunkIndexBaseGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *trunkIndexBasePtr
)
{
    GT_STATUS   rc;
    GT_U32      value;
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(trunkIndexBasePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup0;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 21, &value);

    *trunkIndexBasePtr = value;

    return rc;
}

/**
* @internal cpssDxChNstPortIsolationLookupTrunkIndexBaseGet function
* @endinternal
*
* @brief   Returns the first index of the trunk ID based lookup.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] trunkIndexBasePtr        - (pointer to) the first index of the trunk ID based lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIsolationLookupTrunkIndexBaseGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *trunkIndexBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationLookupTrunkIndexBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkIndexBasePtr));

    rc = internal_cpssDxChNstPortIsolationLookupTrunkIndexBaseGet(devNum, trunkIndexBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkIndexBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationSrcIdBitLocationGet function
* @endinternal
*
* @brief   Returns the location of the isolated bit in the Src-ID and validity.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                     - device number
*
* @param[out] srcIdBitLocationPtr       - (pointer to) the location of the isolated bit in the Src-ID.
* @param[out] enablePtr                 - (pointer to) indication of the Src-ID bit validity:
*                                           GT_TRUE - source ID is used for filtering decision
*                                           GT_FALSE - source ID is not used for filtering decision
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on reading unmatched bit locations from the registers
*
*/
static GT_STATUS internal_cpssDxChNstPortIsolationSrcIdBitLocationGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *srcIdBitLocationPtr,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      valueL2I; /* value of the Src-ID bit location in the L2I unit */
    GT_U32      valueEGF; /* value of the Src-ID bit location in the EGF unit */
    GT_U32      regAddrL2I; /* adress of the Src-ID bit location in the L2I unit */
    GT_U32      regAddrEGF; /* adress of the Src-ID bit location in the EGF unit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E
                                          | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(srcIdBitLocationPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddrL2I = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeGlobalConfig3;
    regAddrEGF = PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.eportVlanEgrFiltering;

    /* Get srcID bit location in the L2I unit */
    rc = prvCpssHwPpGetRegField(devNum, regAddrL2I, 12, 4, &valueL2I);
    if ( rc != GT_OK )
    {
        return rc;
    }

    /* Get srcID bit location in the EGF unit */
    rc = prvCpssHwPpGetRegField(devNum, regAddrEGF, 3, 4, &valueEGF);
    if ( rc != GT_OK )
    {
        return rc;
    }

    if ( valueEGF != valueL2I )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *srcIdBitLocationPtr = valueEGF;
    *enablePtr = (valueEGF == 15) ? GT_FALSE : GT_TRUE;


    return GT_OK;
}

/**
* @internal cpssDxChNstPortIsolationSrcIdBitLocationGet function
* @endinternal
*
* @brief   Returns the location of the isolated bit in the Src-ID and validity.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                     - device number
*
* @param[out] srcIdBitLocationPtr       - (pointer to) the location of the isolated bit in the Src-ID.
* @param[out] enablePtr                 - (pointer to) indication of the Src-ID bit validity:
*                                           GT_TRUE - source ID is used for filtering decision
*                                           GT_FALSE - source ID is not used for filtering decision
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on reading unmatched bit locations from the registers
*
*/
GT_STATUS cpssDxChNstPortIsolationSrcIdBitLocationGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *srcIdBitLocationPtr,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationSrcIdBitLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcIdBitLocationPtr, enablePtr));

    rc = internal_cpssDxChNstPortIsolationSrcIdBitLocationGet(devNum, srcIdBitLocationPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcIdBitLocationPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationSrcIdBitLocationSet function
* @endinternal
*
* @brief   Set the location of the isolated bit in the Src-ID.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum             - device number
* @param[in] srcIdBitLocation   - the location of the isolated bit in the Src-ID (Applicable values: 0..11).
* @param[in] enable             - GT_TRUE - to enable filtering based on the bit in the Src-ID
*                                 GT_FALSE - to disable filtering based on the bit in the Src-ID (filtering will be based on L2/L3 tables)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - srcIdBitLocation parameter is out of range.
*
*/
static GT_STATUS internal_cpssDxChNstPortIsolationSrcIdBitLocationSet
(
    IN GT_U8    devNum,
    IN GT_U32   srcIdBitLocation,
    IN GT_BOOL  enable
)
{
    GT_STATUS   rc;
    GT_U32      value;
    GT_U32      regAddrL2I; /* adress of the Src-ID bit location in the L2I unit */
    GT_U32      regAddrEGF; /* adress of the Src-ID bit location in the EGF unit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E
                                          | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if ( srcIdBitLocation > 11 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    regAddrL2I = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeGlobalConfig3;
    regAddrEGF = PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.eportVlanEgrFiltering;

    /* set srcIdBitLocation to 15 if the feature is disabled */
    value = (enable == GT_TRUE) ? srcIdBitLocation : 15;

    /* set srcID bit location in the L2I unit */
    rc = prvCpssHwPpSetRegField(devNum, regAddrL2I, 12, 4, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set srcID bit location in the EGF unit */
    rc = prvCpssHwPpSetRegField(devNum, regAddrEGF, 3, 4, value);

    return rc;
}

/**
* @internal cpssDxChNstPortIsolationSrcIdBitLocationSet function
* @endinternal
*
* @brief   Set the location of the isolated bit in the Src-ID.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum             - device number
* @param[in] srcIdBitLocation   - the location of the isolated bit in the Src-ID (Applicable values: 0..11).
* @param[in] enable             - GT_TRUE - to enable filtering based on the bit in the Src-ID
*                                 GT_FALSE - to disable filtering based on the bit in the Src-ID (filtering will be based on L2/L3 tables)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - srcIdBitLocation parameter is out of range.
*
*/
GT_STATUS cpssDxChNstPortIsolationSrcIdBitLocationSet
(
    IN  GT_U8   devNum,
    IN GT_U32   srcIdBitLocation,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationSrcIdBitLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcIdBitLocation, enable));

    rc = internal_cpssDxChNstPortIsolationSrcIdBitLocationSet(devNum, srcIdBitLocation, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcIdBitLocation, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationIndexingModeSet function
* @endinternal
*
* @brief  Set the port isolation indexing mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                 - device number
* @param[in] mode                   - Port Isolation Indexing Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note existing API cpssDxChNstPortIsolationOnEportsEnableSet can be used to
*       configure
*       only <Physical Port Mode> and <ePort Mode>
*/
static GT_STATUS internal_cpssDxChNstPortIsolationIndexingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  mode
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      numBits;
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        numBits = 2;
    }
    else
    {
        numBits = 1;
    }

    switch (mode)
    {
        case CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_PHYSICAL_PORT_E:
            value = 0;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EPORT_E :
            value = 1;
            break;
        case CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EVLAN_E:
            if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup1;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, numBits, value);
    return rc;
}

/**
* @internal cpssDxChNstPortIsolationIndexingModeSet function
* @endinternal
*
* @brief  Set the port isolation indexing mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                 - device number
* @param[in] mode                   - Port Isolation Indexing Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note existing API cpssDxChNstPortIsolationOnEportsEnableSet can be used to configure
*       only <Physical Port Mode> and <ePort Mode>
*/
GT_STATUS cpssDxChNstPortIsolationIndexingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationIndexingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChNstPortIsolationIndexingModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIsolationIndexingModeGet function
* @endinternal
*
* @brief  Get the port isolation indexing mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                - (pointer to) Port Isolation Indexing Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChNstPortIsolationIndexingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  *modePtr
)
{
    GT_STATUS   rc;
    GT_U32      value,numBits;
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).global.portIsolationLookup1;

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        numBits = 2;
    }
    else
    {
        numBits = 1;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, numBits, &value);
    if (rc != GT_OK )
    {
        return rc;
    }

    switch (value)
    {
        case 0:
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_PHYSICAL_PORT_E;
            break;
        case 1 :
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EPORT_E;
            break;
        case 2:
            *modePtr = CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EVLAN_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal cpssDxChNstPortIsolationIndexingModeGet function
* @endinternal
*
* @brief  Get the port isolation indexing mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                - (pointer to) Port Isolation Indexing Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChNstPortIsolationIndexingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIsolationIndexingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChNstPortIsolationIndexingModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
