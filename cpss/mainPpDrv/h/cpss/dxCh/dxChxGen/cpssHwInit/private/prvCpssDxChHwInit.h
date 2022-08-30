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
* @file prvCpssDxChHwInit.h
*
* @brief Includes Core level basic Hw initialization functions, and data
* structures.
*
* @version   41
********************************************************************************
*/
#ifndef __prvCpssDxChHwInith
#define __prvCpssDxChHwInith

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_CPSS_DXCH_PROFILES_NUM_CNS       4
#define PRV_CPSS_DXCH2_PROFILES_NUM_CNS      8

#define PRV_CPSS_XCAT_NETWORK_PORTS_SERDES_NUM_CNS  6
#define PRV_CPSS_XCAT_SERDES_NUM_CNS        22
#define PRV_CPSS_XCAT3_SERDES_NUM_CNS       12
#define PRV_CPSS_LION_SERDES_NUM_CNS        24
#define PRV_CPSS_LION2_SERDES_NUM_CNS       24

#define PRV_CPSS_XCAT2_SERDES_NUM_CNS       10

#define PRV_CPSS_BOBCAT2_SERDES_NUM_CNS     36
#define PRV_CPSS_ALDRIN_SERDES_NUM_CNS      33
#define PRV_CPSS_BOBCAT3_SERDES_NUM_CNS     74  /* 6 DP x 12 serdeses + 2 CPU */
#define PRV_CPSS_ALDRIN2_SERDES_NUM_CNS  73  /* DP[0]->12+1 CPU , DP[1]->12 , DP[2,3]-->24 each : total 73 */

/* Target local port to MAC mapping for CPU port */
#define PRV_CPSS_DXCH_TARGET_LOCAL_LINK_STATUS_CPU_PORT_NUM_CNS 255

/**
* @enum PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT
 *
 * @brief An enum for fields of the EGF_SHT table/register that represent the
 * physical port and/or the eport.
 * Notes:
*/
typedef enum{
     PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_STP_STATE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_EGRESS_VLAN_FILTER_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_UC_SRC_ID_FILTER_E          /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_MC_LOCAL_ENABLE_E           /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IGNORE_PHY_SRC_MC_FILTER_E  /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_STP_STATE_MODE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ODD_ONLY_SRC_ID_FILTER_ENABLE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ROUTED_FORWARD_RESTRICTED_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IPMC_ROUTED_FILTER_ENABLE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_FROM_CPU_FORWARD_RESTRICTED_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_BRIDGED_FORWARD_RESTRICTED_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_PORT_ISOLATION_MODE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_EGRESS_EPORT_EVLAN_FILTER_ENABLE_E /* only per EPort */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_MESH_ID_E /* only per EPort */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_EPORT_ASSOCIATED_VID1_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_DROP_ON_EPORT_VID1_MISMATCH_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ASSIGN_EGR_ATTR_LOCAL_E /*only per eport*/
}PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT;

/**
* @enum PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT
 *
 * @brief An enum for fields of the EGF_EFT register that represent the
 * physical port .
 * Notes:
*/
typedef enum{
    /* MC_FIFO */
     PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_MC_FIFO_PORT_TO_MC_FIFO_ATTRIBUTION_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_MC_FIFO_PORT_TO_HEMISPHERE_MAP_E                     /* only per physical port */

    /* EGR_FILTER */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_FC_TRIGGER_BY_CN_FRAME_ON_PORT_ENABLE_E   /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_CN_FRAME_TX_ON_PORT_ENABLE_E              /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_UNREGED_BC_FILTER_ENABLE_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_UNKNOWN_UC_FILTER_ENABLE_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_UNREGED_MC_FILTER_ENABLE_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_OAM_LOOPBACK_FILTER_ENABLE_E              /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_VIRTUAL_2_PHYSICAL_PORT_REMAP_E           /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E          /* only per physical port */

    /* DEV_MAP_TABLE */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_ADDR_CONSTRUCT_PROFILE_E               /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_SRC_PORT_MAP_OWN_DEV_ENABLE_E    /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_TRG_PORT_MAP_OWN_DEV_ENABLE_E    /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_PORT_ADDR_CONSTRUCT_MODE_ADDRESS_PROFILE_E /* only per physical port */

}PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT;

/**
* @enum PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT
 *
 * @brief An enum for types of entries that occupy the TS/ARP/NAT memory
 * values -
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ARP_E : ARP (48 bits)
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_REGULAR_E : Tunnel start (192 bits)
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_DOUBLE_E : Tunnel start - double length (384 bits)
 * (for IPv6 TS)
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_NAT_E : NAT (Network Address Translation. 192 bits)
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 * Notes:
*/
typedef enum{

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ARP_E,

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_REGULAR_E,

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_DOUBLE_E,

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_NAT_E

} PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT;

/**
* @internal prvCpssDxChHwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for cheetah devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrInit
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxCh2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for cheetah2 devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxCh2HwRegAddrInit
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxCh3HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for cheetah-3 devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxCh3HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChXcatHwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh xCat devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcatHwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChXcat3HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh xCat3 devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChLionHwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh Lion devices.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLionHwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChLion2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh Lion2 devices.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLion2HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChBobcat2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh Bobcat2 devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBobcat2HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChHwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChHwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChXcat3HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChXcat3HwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChLion2HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChLionHwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLion2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChHwRegAddrPortMacUpdate function
* @endinternal
*
* @brief   This function updates mac registers addresses for given port accordingly
*         to currently used MAC Unit
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - new interface mode used with this MAC
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrPortMacUpdate
(
    IN GT_U32 devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);

/**
* @internal prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert function
* @endinternal
*
* @brief   convert from 'drop mode' to : packet command and to HW value
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] dropMode                 - drop mode
*
* @param[out] packetCommandPtr         - (pointer to) packet command (can be NULL --> ignored)
* @param[out] fieldValuePtr            - (pointer to) HW field value (can be NULL --> ignored)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert
(
    IN GT_U8        devNum,
    IN CPSS_DROP_MODE_TYPE_ENT dropMode,
    OUT CPSS_PACKET_CMD_ENT *packetCommandPtr,
    OUT GT_U32              *fieldValuePtr
);

/**
* @internal prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert function
* @endinternal
*
* @brief   convert from HW value : to 'drop mode' and to packet command
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] fieldValue               - HW field value
*
* @param[out] packetCommandPtr         - (pointer to) packet command (can be NULL --> ignored)
* @param[out] dropModePtr              - (pointer to) drop mode (can be NULL --> ignored)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert
(
    IN GT_U8        devNum,
    IN GT_U32       fieldValue,
    OUT CPSS_PACKET_CMD_ENT     *packetCommandPtr,
    OUT CPSS_DROP_MODE_TYPE_ENT *dropModePtr
);


/**
* @internal prvCpssDxChHwPpPortToGopConvert function
* @endinternal
*
* @brief   Private (internal) function converts Port number to Group Of Ports number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] phyPortNum               - physical port number
*
* @param[out] portGroupPtr             - (pointer to) Port Group Number
*                                      for not multi port group devices
*                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[out] gopNumPtr                - (pointer to) Group of Ports Number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
*/
GT_STATUS prvCpssDxChHwPpPortToGopConvert
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         phyPortNum,
    OUT GT_U32                       *portGroupPtr,
    OUT GT_U32                       *gopNumPtr
);

/**
* @internal prvCpssDxChHwEgfShtFieldSet function
* @endinternal
*
* @brief   Write a field to the EGF_SHT table/register that represent the
*         physical port and/or the eport.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port and/or the eport)
* @param[in] fieldType                - the field type
* @param[in] accessPhysicalPort       - indication if to access to the physical ports registers
*                                      relevant only for the portNum < 256
* @param[in] accessEPort              - indication if to access to the EPort table
* @param[in] value                    - the data write to the register and/or table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfShtFieldSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT  fieldType,
    IN GT_BOOL                  accessPhysicalPort,
    IN GT_BOOL                  accessEPort,
    IN GT_U32                   value
);

/**
* @internal prvCpssDxChHwEgfShtFieldGet function
* @endinternal
*
* @brief   Read a field to the EGF_SHT table/register that represent the
*         physical port or the eport.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port or the eport)
* @param[in] fieldType                - the field type
* @param[in] accessPhysicalPort       - GT_TRUE : indication if to access to the physical ports registers
*                                      relevant only for the portNum < 256
*                                      GT_FALSE : access to the ePort table
*
* @param[out] valuePtr                 - (pointer to)the data read from the register or table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfShtFieldGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT  fieldType,
    IN GT_BOOL                  accessPhysicalPort,
    OUT GT_U32                   *valuePtr
);


/**
* @internal prvCpssDxChHwEgfEftFieldSet function
* @endinternal
*
* @brief   Write a field to the EGF_EFT register that represent the
*         physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port)
* @param[in] fieldType                - the field type
* @param[in] value                    - the data write to the register
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfEftFieldSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT  fieldType,
    IN GT_U32                   value
);

/**
* @internal prvCpssDxChHwEgfEftFieldGet function
* @endinternal
*
* @brief   Read a field from the EGF_EFT register that represent the
*         physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port)
* @param[in] fieldType                - the field type
*
* @param[out] valuePtr                 - (pointer to)the data read from the register
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfEftFieldGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT  fieldType,
    OUT GT_U32                   *valuePtr
);

/**
* @internal prvCpssDxChHwEgfShtPortsBmpTableEntryWrite function
* @endinternal
*
* @brief   Write a whole entry to the table. - for 'bmp of ports' - EGF_SHT tables
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*                                      entryIndex   - index in the table
* @param[in] portsMembersPtr          - (pointer to) the bmp of ports that will be written to
*                                      the table.
* @param[in] isCpuMember              - indication if to use 'isCpuMember'
* @param[in] isCpuMember              - indication to add the CPU port to the BMP of ports
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwEgfShtPortsBmpTableEntryWrite
(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32               tableIndex,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr,
    IN GT_BOOL              isCpuMember_supported,
    IN GT_BOOL              isCpuMember
);

/**
* @internal prvCpssDxChHwEgfShtPortsBmpTableEntryRead function
* @endinternal
*
* @brief   Read a whole entry of the table. - for 'bmp of ports' - EGF_SHT tables
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*                                      entryIndex   - index in the table
* @param[in] isCpuMember_supported    - indication if to use 'isCpuMemberPtr'
*
* @param[out] portsMembersPtr          - (pointer to) the bmp of ports that will be written to
*                                      the table.
* @param[out] isCpuMemberPtr           - (pointer to) indication to add the CPU port to the BMP of ports
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwEgfShtPortsBmpTableEntryRead
(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32               tableIndex,
    OUT CPSS_PORTS_BMP_STC  *portsMembersPtr,
    IN GT_BOOL              isCpuMember_supported,
    OUT GT_BOOL              *isCpuMemberPtr
);

/**
* @internal prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort function
* @endinternal
*
* @brief   set (enable/disable) single port (bit index) in entry to the table. - for 'bmp of ports' - EGF_SHT tables
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*                                      entryIndex   - index in the table
* @param[in] portNum                  - the port number
* @param[in] enable                   - enable/disable the port in the entry
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort
(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32               tableIndex,
    IN GT_U32               portNum,
    IN GT_BOOL              enable
);

/**
* @internal prvCpssDxChHwHaTunnelStartArpNatTableEntryWrite function
* @endinternal
*
* @brief   Write a whole entry to the table. HA memory of 'TunnelStart/ARP/NAT(for NAT capable devices)'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] entryType                - entry type (arp/TS/TS_double)
* @param[in] tableIndex               - index in the table.
*                                      the ratio is 4 ARP per 1 TS ('regular')
*                                      so for 1K TS the indexes are:
*                                      for ARP : (APPLICABLE RANGES: 0,1,2,3,4..(4K-1))
*                                      for TS  : (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      for TS_double : (APPLICABLE RANGES: 0,2,4,6,8..(1K-2))
*                                      for NAT: (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      NOTE: the 'double' TS must use only 'even' indexes (0,2,4,6,...)
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwHaTunnelStartArpNatTableEntryWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       portGroupId,
    IN PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT   entryType,
    IN GT_U32                                       tableIndex,
    IN GT_U32                                       *entryValuePtr
);

/**
* @internal prvCpssDxChHwHaTunnelStartArpNatTableEntryRead function
* @endinternal
*
* @brief   Read a whole entry from the table. HA memory of 'TunnelStart/ARP/NAT(for NAT capable devices)'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] entryType                - entry type (arp/TS/TS_double)
* @param[in] tableIndex               - index in the table.
*                                      the ratio is 4 ARP per 1 TS ('regular')
*                                      so for 1K TS the indexes are:
*                                      for ARP : (APPLICABLE RANGES: 0,1,2,3,4..(4K-1))
*                                      for TS  : (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      for TS_double : (APPLICABLE RANGES: 0,2,4,6,8..(1K-2))
*                                      for NAT: (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      NOTE: the 'double' TS must use only 'even' indexes (0,2,4,6,...)
*
* @param[out] entryValuePtr            - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwHaTunnelStartArpNatTableEntryRead
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       portGroupId,
    IN PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT   entryType,
    IN GT_U32                                       tableIndex,
    OUT GT_U32                                      *entryValuePtr
);

/**
* @internal prvCpssDxChXcat3PortMacRegAddrSwap function
* @endinternal
*
* @brief   MAC address DB update, in order to support extended MAC feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*                                      portsBmp  - physical port number
* @param[in] enable                   - extended MAC mode enabled
*                                      GT_TRUE - update registers to work in extended mode
*                                      GT_FALSE - update registers to work in regular mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3PortMacRegAddrSwap
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);


/**
* @internal prvCpssDxChHwXcat3RegAddrPortMacUpdate function
* @endinternal
*
* @brief   This function updates mac registers addresses for given port accordingly
*         to currently used MAC Unit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwXcat3RegAddrPortMacUpdate
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet function
* @endinternal
*
* @brief   This function returns supported bandwidth by dev family/reviosion/corecolock
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBWCapacityInGbpsPtr  - pointer to bw coresponding to dev family/reviosion/corecolock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on error
* @retval GT_NOT_SUPPORTED         - on device family/revison/coreclock not found
*/
GT_U32 hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *pipeBWCapacityInGbpsPtr
);



/**
* @internal prvCpssDxChPortLedInterfaceGet function
* @endinternal
*
* @brief   Get LED interface number by port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
*                                       RETURNS:
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - not supported device
*/
GT_STATUS prvCpssDxChPortLedInterfaceGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32              *interfaceNumberPtr
);


/**
* @internal prvCpssLedStreamNumOfInterfacesInPortGroupGet function
* @endinternal
*
* @brief   Get the number of LED interfaces that the device's core supports
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                       Number of LED interfaces that the device's core supports
*
* @note Multi-core device will return information per core !!!
*
*/
GT_U32   prvCpssLedStreamNumOfInterfacesInPortGroupGet
(
    IN GT_U8 devNum
);


#ifdef ASIC_SIMULATION
GT_U32 simCoreClockOverwrittenGet(GT_VOID);
GT_STATUS simCoreClockOverwrite
(
    GT_U32 simUserDefinedCoreClockMHz
);

#endif

/**
* @internal prvCpssDxChHwRegAddrDbRemove function
* @endinternal
*
* @brief   remove the DB of 'register address'
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChHwRegAddrDbRemove
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChHwInitNumOfSerdesGet function
* @endinternal
*
* @brief   Get number of SERDES lanes in device.
*         Function returns number of SERDES in port group for Lion2
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval 0                        - for not applicable device
*                                       Number of number of SERDES lanes in device or port group
*/
GT_U32   prvCpssDxChHwInitNumOfSerdesGet
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssDxChHwPortGroupBetweenUnitsConvert function
* @endinternal
*
* @brief   Convert port group of 'unit A' to port group in 'unit B'
*         NOTE:
*         1. some units hold 'relations' between themselves , like:
*         a. MT to MG on FDB upload or on NA to CPU.
*         b. CNC to MG on CNC upload
*         2. such function needed for Bobcat3 where single MG unit serves the CNC
*         units of both 2 port groups. (MT is also single anyway)
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] unitA                    - the type of unit A
* @param[in] portGroupInUnitA         - the port group in unit A that needs conversion.
* @param[in] unitB                    - the type of unit B
*
* @param[out] portGroupInUnitBPtr      - (pointer to) the port group of unit B that
*                                      relates to port group in unit A.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssDxChHwPortGroupBetweenUnitsConvert
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitA,
    IN GT_U32       portGroupInUnitA,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitB,
    OUT GT_U32      *portGroupInUnitBPtr
);
/**
* @internal prvCpssDxChHwPortGroupToMgUnitConvert function
* @endinternal
*
* @brief   Convert port group of client in port group to MG unit index.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - the port group in unit A that needs conversion.
* @param[in] mgClient                 - the type of client
*
* @param[out] mgUnitPtr      - (pointer to) the MG unit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssDxChHwPortGroupToMgUnitConvert
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN PRV_CPSS_DXCH_MG_CLIENT_ENT  mgClient,
    OUT GT_U32                      *mgUnitPtr
);

/**
* @internal prvCpssDxChHwPpPhase1ShadowInit function
* @endinternal
*
* @brief   This function performs basic cpss shadow configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] ppPhase1ParamsPtr        - Packet processor hardware specific parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwPpPhase1ShadowInit
(
    IN CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *ppPhase1ParamsPtr
);

/**
* @internal prvCpssDxChDevLedInterfacePortCheck function
* @endinternal
*
* @brief   Function checks device LED port for out of range.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ledInterfaceNum          - led stream (interface)
* @param[in] ledPort                  - led port
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - on not initialized led port
*/
GT_STATUS prvCpssDxChDevLedInterfacePortCheck
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          ledPort
);

/**
* @internal prvCpssLion2DuplicatedMultiPortGroupsGet function
* @endinternal
*
* @brief   Check if register address is duplicated in multi-port groups device
*         for Lion2 device.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssLion2DuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssBobkDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in bobk device.
*
* @note   APPLICABLE DEVICES:      BobK.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lon2.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssBobkDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssBobcat3DuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in bobcat3 device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lon2.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssBobcat3DuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);


/**
* @internal prvCpssAldrin2DuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Aldrin2 device.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lon2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAldrin2DuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssAldrinDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in aldrin device.
*
* @note   APPLICABLE DEVICES:      Aldrin.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; ExMxPm.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAldrinDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);


/**
* @internal prvCpssHawkDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in AC5P device.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssHawkDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssPhoenixDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in AC5X device.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssPhoenixDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssHarrierDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Harrier device.
*
* @note   APPLICABLE DEVICES:      Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssHarrierDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);
/**
* @internal prvCpssDxChCgMacUnitDisable function
* @endinternal
*
* @brief   CG MAC unit enable/disable for power reduction purposes.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] disable                  - CG MAC unit disable
*                                      GT_TRUE - CG Unit disabled
*                                      GT_FALSE - CG Unit enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCgMacUnitDisable
(
    IN  GT_U8 devNum,
    IN  GT_BOOL disable
);


/**
* @internal prvCpssDxChHwDevNumChange function
* @endinternal
*
* @brief   This function performs own HW device number update
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwDevNumChange
(
    IN  GT_U8                       devNum,
    IN  GT_HW_DEV_NUM               hwDevNum
);

/**
* @internal prvCpssDxChHwInitSip5TcamDefaultsInit function
* @endinternal
*
* @brief   Set whole TCAM to HW default values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on failure by timeout
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS prvCpssDxChHwInitSip5TcamDefaultsInit
(
     IN GT_U8   devNum
);

/**
* @internal prvCpssDxChFalconPortDeleteWa_start function
* @endinternal
*
* @brief   the function that implements the WA for the port that is going down.
*           NOTE:
*           1. if the speed of the port was not >= 100G ... no need to call
*           this function to do the WA
*           2. the function assumes that 'CPSS lock' was activated.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - the physical port number that is doing 'PORT DELETE'
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_start
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum
);

/**
* @internal prvCpssDxChFalconPortDeleteWa_updateHwDevNum function
* @endinternal
*
* @brief   state that the HW devNum was updated .
*           called only after PRV_CPSS_HW_DEV_NUM_MAC(devNum) was updated.
*           needed by the 'PORT DELETE' workaround (WA) in Falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_updateHwDevNum
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChFalconPortDeleteWa_PacketBufferInit function
* @endinternal
*
* @brief Initialization for the WA for the PB (packet buffer)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_PacketBufferInit
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
);

/**
* @internal prvCpssDxChFalconPortDeleteWa_PacketBufferPortCreate function
* @endinternal
*
* @brief  Do 'port create'  WA configurations that relate to packet buffer (PB)
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channelIndex   - channel index.
*                             channels 0..7 related to network ports - configured only at runtime
*                             channel 8 related to CPU port - initialized to 25G speed,
*                             can be changed at runtime,
*                             channel 9 related to TXQ - initialized to 50G speed,
*                             cannot be changed at runtime,
*                             (APPLICABLE RANGES: 0..9)
* @param[in] speed          - port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_PacketBufferPortCreate
(
    IN GT_U8                        devNum,
    IN  GT_U32                      tileIndex,
    IN  GT_U32                      gpcIndex,
    IN  GT_U32                      channelIndex,
    IN  CPSS_PORT_SPEED_ENT         cpssSpeed
);

/**
* @internal prvCpssDxChFalconPortDeleteWa_initParams function
* @endinternal
*
* @brief   state the dedicated resources needed by the 'PORT DELETE' workaround (WA)
*           in Falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] infoPtr                  - (pointer to) the needed info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_initParams(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC *infoPtr
);

/**
* @internal prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet function
* @endinternal
*
* @brief set  info needed per <queue_base_index> is it hold valid TX credits configuration
*        from the TXQ down to the MAC , so traffic will not stuck in the TXQ if
*        the EGF direct traffic to it .
*
*        this info needed for the 'EGF port link filter'.
*
*        values :
*            GT_TRUE  - the <queue_base_index> point to TXQ with valid TXDMA credits , TxFIFO ... all the way to the MAC.
*                    so the EGF port link filter , can be set to 'force link UP'
*            GT_FALSE - the <queue_base_index> point to TXQ WITHOUT valid credits or TXDMA WITHOUT credits.
*                    so the EGF port link filter , MUST NOT be set to 'force link UP'
*                NOTE: the 'cascade port' of 'remote physical ports' , hold no 'TXQ port'... so NOT allowed to do EGF 'link UP'.
*
*        NOTE: supporting 'remote physical port' and 'mac ports' (not needed for 'CPU SDMA port')
*
*        Who update this DB : on 'port mode speed set'
*            if doing "port create" , at the end   set 'GT_TRUE'  for the <queue_base_index> of the 'portNum'
*            if doing "port delete" , at the start set 'GT_FALSE' for the <queue_base_index> of the 'portNum'
*
*        Who used this info:
*            when doing EGF port link filter : force link UP ... need to check if we allow it !
*
*            if not allowed we can do , one of next:
*            1. fail the operation (GT_BAD_STATE to indicate the application to 'not do it' at this stage)
*            2. write to HW 'force link DOWN' (although application asked for 'link UP')
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - the physical port number
* @param[in] isValid               - GT_TRUE  - the <queue_base_index> point to TXQ with valid TXDMA credits , TxFIFO ... all the way to the MAC.
*                                        so the EGF port link filter , can be set to 'force link UP'
*                                    GT_FALSE - the <queue_base_index> point to TXQ WITHOUT valid credits or TXDMA WITHOUT credits.
*                                        so the EGF port link filter , MUST NOT be set to 'force link UP'
*                                    NOTE: the 'cascade port' of 'remote physical ports' , hold no 'TXQ port'... so NOT allowed to do EGF 'link UP'.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  isValid
);

/**
* @internal prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet function
* @endinternal
*
* @brief get  info needed per <queue_base_index> is it hold valid TX credits configuration
*        from the TXQ down to the MAC , so traffic will not stuck in the TXQ if
*        the EGF direct traffic to it .
*
*        this info needed for the 'EGF port link filter'.
*
*        values :
*            GT_TRUE  - the <queue_base_index> point to TXQ with valid TXDMA credits , TxFIFO ... all the way to the MAC.
*                    so the EGF port link filter , can be set to 'force link UP'
*            GT_FALSE - the <queue_base_index> point to TXQ WITHOUT valid credits or TXDMA WITHOUT credits.
*                    so the EGF port link filter , MUST NOT be set to 'force link UP'
*                NOTE: the 'cascade port' of 'remote physical ports' , hold no 'TXQ port'... so NOT allowed to do EGF 'link UP'.
*
*        NOTE: supporting 'remote physical port' and 'mac ports' (not needed for 'CPU SDMA port')
*
*        Who update this DB : on 'port mode speed set'
*            if doing "port create" , at the end   set 'GT_TRUE'  for the <queue_base_index> of the 'portNum'
*            if doing "port delete" , at the start set 'GT_FALSE' for the <queue_base_index> of the 'portNum'
*
*        Who used this info:
*            when doing EGF port link filter : force link UP ... need to check if we allow it !
*
*            if not allowed we can do , one of next:
*            1. fail the operation (GT_BAD_STATE to indicate the application to 'not do it' at this stage)
*            2. write to HW 'force link DOWN' (although application asked for 'link UP')
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - the physical port number
* @param[out] isValidPtr           - (pointer to)
*                                    GT_TRUE  - the <queue_base_index> point to TXQ with valid TXDMA credits , TxFIFO ... all the way to the MAC.
*                                        so the EGF port link filter , can be set to 'force link UP'
*                                    GT_FALSE - the <queue_base_index> point to TXQ WITHOUT valid credits or TXDMA WITHOUT credits.
*                                        so the EGF port link filter , MUST NOT be set to 'force link UP'
*                                    NOTE: the 'cascade port' of 'remote physical ports' , hold no 'TXQ port'... so NOT allowed to do EGF 'link UP'.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *isValidPtr
);

/**
* @internal prvCpssDxChHwMacSecInit function
* @endinternal
*
* @brief   Disables the core clock towards the MacSec shell using a clock gate
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; xCat3; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - physical device number
* @param[in] macSecClockDisable    - Disables the core clock towards the MacSec shell using a clock gate
*                                    GT_TRUE  - core clock is disabled
*                                    GT_FALSE - core clock is enabled
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChHwMacSecInit
(
    IN GT_U8    devNum,
    IN GT_BOOL  macSecClockDisable
);

/**
* @internal prvCpssDxChStreamSrfUse802_1cbdb function
* @endinternal
*
* @brief   Set the 802.1cbdb mode or the 802.1cb.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] enable802_1cbdb       - enable/disable the 802.1cbdb
*                                    GT_TRUE  - enable  the 802.1cbdb
*                                    GT_FALSE - disable the 802.1cbdb (use 802.1cb)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChStreamSrfUse802_1cbdb
(
    IN  GT_U8         devNum,
    IN  GT_BOOL       enable802_1cbdb
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #define __prvCpssDxChHwInith  */

