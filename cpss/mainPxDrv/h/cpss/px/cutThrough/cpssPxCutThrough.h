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
* @file cpssPxCutThrough.h
*
* @brief CPSS PX Cut Through facility API.
*
* @version   13
********************************************************************************
*/
#ifndef __cpssPxCutThroughh
#define __cpssPxCutThroughh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

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
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable,
    IN GT_BOOL              untaggedEnable
);

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
);

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
);

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
);

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
);

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
);

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
* @param[in] buffersLimit             - Max buffers limit for Cut-Through traffic.
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
);

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
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] devNum                   - device number.
*
* @param[out] buffersLimitPtr          - (pointer to)Max buffers limit for Cut-Through traffic.
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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____cpssPxCutThroughh */





