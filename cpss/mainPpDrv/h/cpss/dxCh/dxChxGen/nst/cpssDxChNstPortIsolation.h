
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
* @file cpssDxChNstPortIsolation.h
*
* @brief CPSS DxCh NST Port Isolation Mechanism.
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChNstPortIsolationh
#define __cpssDxChNstPortIsolationh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>


/**
* @enum CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT
 *
 * @brief Enumeration for L2 or/and L3 traffic type
*/
typedef enum{

    /** @brief Port isolation for L2
     *  packets (L2 Port Isolation table)
     */
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,

    /** @brief Port isolation for L3
     *  packets (L3 Port Isolation table)
     */
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E

} CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT;


/**
* @enum CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT
 *
 * @brief Enumeration for Port Isolation feature in the TxQ per egress ePort.
*/
typedef enum{

    /** Port Isolation filter is disabled. */
    CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E,

    /** Trigger L2 Port Isolation filter. */
    CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E,

    /** Trigger L3 Port Isolation filter. */
    CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E,

    /** Trigger L2 & L3 Port Isolation filter. */
    CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E

} CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT;


/**
* @enum CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT
 *
 * @brief Enumeration for Port Isolation Indexing Mode
*/
typedef enum{

    /** @brief Port isolation is performed based on the source physical port. */
    CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_PHYSICAL_PORT_E,

    /** @brief Port isolation is performed based on the source ePort. */
    CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EPORT_E,

    /** @brief Port isolation Per VLAN. Port isolation: <VLAN, Port> is mapped
     *  to Root port or Leaf port. (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman) */
    CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EVLAN_E

} CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT;

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
);


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
);

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
);

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
);

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
GT_STATUS cpssDxChNstPortIsolationPortAdd
(
    IN GT_U8                                          devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                        *srcInterfacePtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum
);

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
* @param[in] srcInterfacePtr          - (pointer to) table index is calculated from source interface
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
);



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
);

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
);

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
);

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
);


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
);

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
);

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
);

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
);

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
* @retval GT_BAD_STATE             - on reading unmatch bit locations from the registers
*
*/
GT_STATUS cpssDxChNstPortIsolationSrcIdBitLocationGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *srcIdBitLocationPtr,
    OUT GT_BOOL *enablePtr
);

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
);

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
* @note exsisting API cpssDxChNstPortIsolationOnEportsEnableSet can be used to configure
*       only <Physical Port Mode> and <ePort Mode>
*/
GT_STATUS cpssDxChNstPortIsolationIndexingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT  mode
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChNstPortIsolationh */


