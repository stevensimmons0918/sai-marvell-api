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
* @file cpssPxCutThrough.c
*
* @brief CPSS PX Cut Through facility API.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cutThrough/private/prvCpssPxCutThroughLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/px/cutThrough/cpssPxCutThrough.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/cpssPxCfgInit.h>

/**
* @internal internal_cpssPxCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[in] untaggedEnable           -  GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Used only if enable == GT_TRUE.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssPxCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssPxCutThroughUpEnableSet.
*
*/
static GT_STATUS internal_cpssPxCutThroughPortEnableSet
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                 enable,
    IN GT_BOOL                 untaggedEnable
)
{
    GT_U32      regAddr;      /* register address        */
    GT_U32      portRxDmaNum; /* port RxDMA number       */
    GT_U32      hwMask;       /* HW mask for register    */
    GT_U32      hwData;       /* HW data for register    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];
    hwMask = ((1 << 8) | (1 << 7));
    hwData = ((BOOL2BIT_MAC(untaggedEnable) << 8) | (BOOL2BIT_MAC(enable) << 7));

    return prvCpssHwPpWriteRegBitMask(
        CAST_SW_DEVNUM(devNum), regAddr, hwMask, hwData);
}

/**
* @internal cpssPxCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[in] untaggedEnable           -  GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Used only if enable == GT_TRUE.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssPxCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssPxCutThroughUpEnableSet.
*
*/
GT_STATUS cpssPxCutThroughPortEnableSet
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                 enable,
    IN GT_BOOL                 untaggedEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughPortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable, untaggedEnable));

    rc = internal_cpssPxCutThroughPortEnableSet(
        devNum, portNum, enable, untaggedEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable, untaggedEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status:
*                                      - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[out] untaggedEnablePtr        - pointer to Cut Through forwarding mode status
*                                      for untagged packets.
*                                      Used only if enablePtr == GT_TRUE.
*                                      GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssPxCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCutThroughPortEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL               *enablePtr,
    OUT GT_BOOL               *untaggedEnablePtr
)
{
    GT_STATUS   rc;           /* return code             */
    GT_U32      regAddr;      /* register address        */
    GT_U32      portRxDmaNum; /* port RxDMA number       */
    GT_U32      hwData;       /* HW data for register    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(untaggedEnablePtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];

    rc = prvCpssHwPpReadRegister(
        CAST_SW_DEVNUM(devNum), regAddr, &hwData);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *enablePtr         = BIT2BOOL_MAC((hwData >> 7) & 1);
    *untaggedEnablePtr = BIT2BOOL_MAC((hwData >> 8) & 1);
    return GT_OK;
}

/**
* @internal cpssPxCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status:
*                                      - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[out] untaggedEnablePtr        - pointer to Cut Through forwarding mode status
*                                      for untagged packets.
*                                      Used only if enablePtr == GT_TRUE.
*                                      GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssPxCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCutThroughPortEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL               *enablePtr,
    OUT GT_BOOL               *untaggedEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr, untaggedEnablePtr));

    rc = internal_cpssPxCutThroughPortEnableGet(
        devNum, portNum, enablePtr, untaggedEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr, untaggedEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssPxCutThroughPortEnableSet for it.
*
*/
static GT_STATUS internal_cpssPxCutThroughUpEnableSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           up,
    IN GT_BOOL          enable
)
{
    GT_U32      regAddr;      /* register address        */
    GT_U32      fieldOffset;  /* field offset            */
    GT_U32      fieldLength;  /* field length            */
    GT_U32      hwData;       /* HW data for register    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (up >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable;
    fieldLength = 1;
    fieldOffset = up;
    hwData = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, hwData);
}

/**
* @internal cpssPxCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssPxCutThroughPortEnableSet for it.
*
*/
GT_STATUS cpssPxCutThroughUpEnableSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           up,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughUpEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, up, enable));

    rc = internal_cpssPxCutThroughUpEnableSet(
        devNum, up, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, up, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status
*                                      for tagged packets, with the specified UP.:
*                                      - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCutThroughUpEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           up,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS   rc;           /* return code             */
    GT_U32      regAddr;      /* register address        */
    GT_U32      fieldOffset;  /* field offset            */
    GT_U32      fieldLength;  /* field length            */
    GT_U32      hwData;       /* HW data for register    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (up >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable;
    fieldLength = 1;
    fieldOffset = up;

    rc =  prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, &hwData);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *enablePtr = BIT2BOOL_MAC(hwData);
    return GT_OK;
}

/**
* @internal cpssPxCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status
*                                      for tagged packets, with the specified UP.:
*                                      - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCutThroughUpEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           up,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughUpEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, up, enablePtr));

    rc = internal_cpssPxCutThroughUpEnableGet(
        devNum, up, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, up, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] etherType0               - VLAN EtherType0 (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] etherType1               - VLAN EtherType1 (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCutThroughVlanEthertypeSet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           etherType0,
    IN  GT_U32           etherType1
)
{
    GT_U32      regAddr;      /* register address        */
    GT_U32      hwData;       /* HW data for register    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (etherType0 >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (etherType1 >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification;
    hwData = ((etherType1 << 16) | etherType0);

    return prvCpssHwPpWriteRegister(
        CAST_SW_DEVNUM(devNum), regAddr, hwData);
}

/**
* @internal cpssPxCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] etherType0               - VLAN EtherType0 (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] etherType1               - VLAN EtherType1 (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCutThroughVlanEthertypeSet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           etherType0,
    IN  GT_U32           etherType1
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughVlanEthertypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherType0, etherType1));

    rc = internal_cpssPxCutThroughVlanEthertypeSet(
        devNum, etherType0, etherType1);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherType0, etherType1));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherType0Ptr            - Pointer to VLAN EtherType0
* @param[out] etherType1Ptr            - Pointer to VLAN EtherType1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCutThroughVlanEthertypeGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *etherType0Ptr,
    OUT GT_U32           *etherType1Ptr
)
{
    GT_STATUS   rc;           /* return code             */
    GT_U32      regAddr;      /* register address        */
    GT_U32      hwData;       /* HW data for register    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(etherType0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(etherType1Ptr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification;

    rc = prvCpssHwPpReadRegister(
        CAST_SW_DEVNUM(devNum), regAddr, &hwData);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *etherType0Ptr = (hwData & 0xFFFF);
    *etherType1Ptr = ((hwData >> 16) & 0xFFFF);
    return GT_OK;
}

/**
* @internal cpssPxCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherType0Ptr            - Pointer to VLAN EtherType0
* @param[out] etherType1Ptr            - Pointer to VLAN EtherType1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCutThroughVlanEthertypeGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *etherType0Ptr,
    OUT GT_U32           *etherType1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughVlanEthertypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherType0Ptr, etherType1Ptr));

    rc = internal_cpssPxCutThroughVlanEthertypeGet(
        devNum, etherType0Ptr, etherType1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherType0Ptr, etherType1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more than limit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] buffersLimit             - Max buffers limit for Cut-Trough traffic.
*                                      The resolution is one buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCutThroughMaxBuffersLimitSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                buffersLimit
)
{
    GT_U32      regAddr;            /* register address          */
    GT_U32      portRxDmaNum;       /* port RxDMA number         */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);
    if (buffersLimit >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 12/*fieldOffset*/, 16/*fieldLength*/, buffersLimit);
}

/**
* @internal cpssPxCutThroughMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more than limit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] buffersLimit             - Max buffers limit for Cut-Trough traffic.
*                                      The resolution is one buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCutThroughMaxBuffersLimitSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                buffersLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughMaxBuffersLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, buffersLimit));

    rc = internal_cpssPxCutThroughMaxBuffersLimitSet(
        devNum, portNum, buffersLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, buffersLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCutThroughMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more than limit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] buffersLimitPtr          - (pointer to)Max buffers limit for Cut-Trough traffic.
*                                      The resolution is one buffer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCutThroughMaxBuffersLimitGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *buffersLimitPtr
)
{
    GT_U32      regAddr;            /* register address          */
    GT_U32      portRxDmaNum;       /* port RxDMA number         */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);
    CPSS_NULL_PTR_CHECK_MAC(buffersLimitPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 12/*fieldOffset*/, 16/*fieldLength*/, buffersLimitPtr);
}

/**
* @internal cpssPxCutThroughMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more than limit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] buffersLimitPtr          - (pointer to)Max buffers limit for Cut-Trough traffic.
*                                      The resolution is one buffer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCutThroughMaxBuffersLimitGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *buffersLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCutThroughMaxBuffersLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, buffersLimitPtr));

    rc = internal_cpssPxCutThroughMaxBuffersLimitGet(
        devNum, portNum, buffersLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, buffersLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



