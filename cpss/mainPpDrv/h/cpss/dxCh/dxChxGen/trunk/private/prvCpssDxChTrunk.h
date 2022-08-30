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
* @file prvCpssDxChTrunk.h
*
* @brief API definitions for 802.3ad Link Aggregation (Trunk) facility
* private - CPSS - DxCh
*
* @version   14
********************************************************************************
*/

#ifndef __prvCpssDxChTrunkh
#define __prvCpssDxChTrunkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

/**
* @internal prvCpssDxChTrunkFamilyInit function
* @endinternal
*
* @brief   This function sets CPSS private DxCh family info about trunk behavior
*         that relate to the family and not to specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devFamily                - device family.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - the family was not initialized properly
*/
GT_STATUS prvCpssDxChTrunkFamilyInit(
    IN CPSS_PP_FAMILY_TYPE_ENT  devFamily
);

/**
* @internal prvCpssDxChTrunkHwDevNumSet function
* @endinternal
*
* @brief   Update trunk logic prior change of HW device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*                                      NOTE: at this stage the PRV_CPSS_HW_DEV_NUM_MAC(devNum) hold
*                                      the 'old hwDevNum' !!!
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChTrunkHwDevNumSet
(
    IN GT_U8            devNum,
    IN GT_HW_DEV_NUM    hwDevNum
);

/**
* @internal prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet function
* @endinternal
*
* @brief   the function sets the designated device table with the portsArr[].trunkPort
*         in indexes that match hash (%8 or %64(modulo)) on the ports in portsArr[].srcPort
*         this to allow 'Src port' trunk hash for traffic sent to the specified
*         trunk.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfPorts               - number of pairs in array portsArr[].
* @param[in] portsArr[]               - (array of) pairs of 'source ports' ,'trunk ports'
*                                      for the source port hash.
* @param[in] mode                     - hash  (%8 or %64 (modulo))
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or trunkId or port in portsArr[].srcPort or
*                                       port in portsArr[].trunkPort or mode
*                                       or map two Source ports that falls into same Source hash index
*                                       into different trunk member ports
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*/
GT_STATUS prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet
(
    IN GT_U8             devNum,
    IN GT_U32            numOfPorts,
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC  portsArr[],
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT  mode
);

/**
* @internal prvCpssDxChTrunkHashBitsSelectionGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
*
* @param[out] startBitPtr              - (pointer to) start bit
* @param[out] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTrunkHashBitsSelectionGet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32           *startBitPtr,
    OUT GT_U32           *numOfBitsPtr
);

/**
* @internal prvCpssDxChTrunkPortTrunkIdSet function
* @endinternal
*
* @brief  this function wraps CpssDxChTrunkPortTrunkIdSet with flag HIGH_LEVEL_API_CALL - used for high availability perpose
*         CpssDxChTrunkPortTrunkIdSet Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] memberOfTrunk            - is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[in] trunkId                  - the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when memberOfTrunk = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChTrunkPortTrunkIdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
);

/**
* @internal prvCpssDxChTrunkTableEntrySet function
* @endinternal
*
* @brief  this function wraps cpssDxChTrunkTableEntrySet with flag HIGH_LEVEL_API_CALL - used for high availability perpose
*         cpssDxChTrunkTableEntrySet Set the trunk table entry , and set the number of members in it.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
* @param[in] numMembers               - num of enabled members in the trunk
*                                      Note : value 0 is not recommended.
* @param[in] membersArray[]           - array of enabled members of the trunk
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - numMembers exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTrunkTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
);

/**
* @internal prvCpssDxChTrunkNonTrunkPortsEntrySet function
* @endinternal
*
* @brief  this function wraps cpssDxChTrunkNonTrunkPortsEntrySet with flag HIGH_LEVEL_API_CALL - used for high availability perpose
*         cpssDxChTrunkNonTrunkPortsEntrySet Set the trunk's non-trunk ports specific bitmap entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTrunkNonTrunkPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
);

/**
* @internal prvCpssDxChTrunkDesignatedPortsEntrySet function
* @endinternal
*
* @brief  this function wraps cpssDxChTrunkDesignatedPortsEntrySet with flag HIGH_LEVEL_API_CALL - used for high availability perpose
*         cpssDxChTrunkDesignatedPortsEntrySet Set the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
* @param[in] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTrunkDesignatedPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    IN  CPSS_PORTS_BMP_STC  *designatedPortsPtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChTrunkh */


