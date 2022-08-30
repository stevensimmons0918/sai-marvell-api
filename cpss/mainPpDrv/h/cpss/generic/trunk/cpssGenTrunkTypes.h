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
* @file cpssGenTrunkTypes.h
*
* @brief API definitions for 802.3ad Link Aggregation (Trunk) facility
*
*
* States and Modes :
* A. Application can work in one of 2 modes:
* 1. use "high level" trunk manipulations set of APIs - Mode "HL"
* 2. use "low level" trunk HW tables/registers set of APIs - MODE "LL"
*
* B. Using "high level" trunk manipulations set of APIs - "High level"
* Mode.
* In this mode the CPSS synchronize all the relevant trunk
* tables/registers, implement some WA for trunks Errata.
* 1. Create trunk
* 2. Destroy trunk
* 3. Add/remove member to/from trunk
* 4. Enable/disable in trunk
* 5. Add/Remove port to/from "non-trunk" entry
* C. Using "low level" trunk HW tables/registers set of APIs- "Low level"
* Mode.
* In this mode the CPSS allow direct access to trunk tables/registers.
* In this mode the Application required to implement the
* synchronization between the trunk t6ables/registers and to implement
* the WA to some of the trunk Errata.
*
* 1. Set per port the trunkId
* 2. Set per trunk the trunk members , number of members
* 3. Set per trunk the Non-trunk local ports bitmap
* 4. Set per Index the designated local ports bitmap
*
* D. Application responsibility is not to use a mix of using API from
* those 2 modes
* The only time that the Application can shift between the 2 modes,
* is only when there are no trunks in the device.
*
*
* @version   7
********************************************************************************
*/

#ifndef __cpssGenTrunkTypesh
#define __cpssGenTrunkTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>

/* constant : CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS
 *
 * Description:
 *  Max number of members per trunk that the DXCH devices supports.
 *
 *  This is relevant to devices that initialized the Trunk Lib with 'native/soho' mode :
 *  cpssDxChTrunkInit(devNum,maxTrunks,
 *  CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E or
 *  CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E)
 *
 *  NOTE: for sip5 devices see also : CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS
 *        (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *
*/
#define CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS   8

/* constant : CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS
 *
 * Description:
 *  Max number of members per trunk that the sip5 devices supports.
 *
 *  This is relevant to devices that initialized the Trunk Lib with 'flex' mode :
 *  cpssDxChTrunkInit(devNum,maxTrunks,CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E)
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *
*/
#define CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS   _4K

/**
* @struct CPSS_TRUNK_WEIGHTED_MEMBER_STC
 *
 * @brief A structure to hold the info on weighted trunk member : member ,weight.
*/
typedef struct{

    /** the trunk member ((dev,port)) */
    CPSS_TRUNK_MEMBER_STC member;

    /** @brief the weight of this member.
     *  this weight is relative to the total weights of all members of this trunk
     */
    GT_U32 weight;

} CPSS_TRUNK_WEIGHTED_MEMBER_STC;

/**
* @enum CPSS_TRUNK_TYPE_ENT
 *
 * @brief An enumeration for the trunk type in the DB of the CPSS.
 * values:
 * CPSS_TRUNK_TYPE_FREE_E  - the trunk is empty .
 * trunk at this stage supports ALL (high level) APIs.
 * CPSS_TRUNK_TYPE_REGULAR_E - the trunk used as regular trunk.
 * supports next (high level) APIs. (DXCH example) :
 * cpssDxChTrunkMembersSet ,
 * cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
 * cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
 * cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
 * cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
 * and cpssDxChTrunkDbIsMemberOfTrunk
 * CPSS_TRUNK_TYPE_CASCADE_E - the trunk used as "cascade trunk" ('Local trunk').
 * supports next (high level) APIs. (DXCH example) :
 * cpssDxChTrunkCascadeTrunkPortsSet , cpssDxChTrunkCascadeTrunkPortsGet
 * cpssDxChTrunkCascadeTrunkWithWeightedPortsSet , cpssDxChTrunkCascadeTrunkWithWeightedPortsGet
 * and cpssDxChTrunkDbIsMemberOfTrunk
*/
typedef enum{

    CPSS_TRUNK_TYPE_FREE_E,

    CPSS_TRUNK_TYPE_REGULAR_E,

    CPSS_TRUNK_TYPE_CASCADE_E

} CPSS_TRUNK_TYPE_ENT;

/**
* @enum CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT
 *
 * @brief enumeration for the options that each port can be represented in
 * the entries of the 'designated MC table'.
 * it is relevant for devices that hold separated 'Designated Table for UC and MC'
*/
typedef enum{

    CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_NATIVE_WEIGHTED_E   = GT_TRUE,

    /** @brief the port forbid
     *  all MC traffic (hence no representation in 'Designated MC Table',
     *  so other ports in the trunk get more representation)
     */
    CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORBID_ALL_E        = GT_FALSE,

    /** @brief the port force
     *  all MC traffic (hence representation in each and every entry of
     *  'Designated MC Table' , this also cause other ports in the trunk
     *  get more representation)
     *  example of those modes: (port 3 will have different modes)
     *  assume 3 ports 1,2,3 in the same trunk :
     *  port 1 mode 'NATIVE_WEIGHTED' --> this is 'default' of all ports
     *  port 2 mode 'NATIVE_WEIGHTED'
     *  port 3 mode 'NATIVE_WEIGHTED'
     *  the 'Designated MC Table' would look like this:
  *  port | 0  1  2  3  4  5  6
     *  --------------------------------
*index  0 | 1  x        1  1  1
     *  1 | 1     x     1  1  1
     *  2 | 1        x  1  1  1
     *  3 | 1  x        1  1  1
     *  4 | 1     x     1  1  1
     *  5 | 1        x  1  1  1
     *  6 | 1  x        1  1  1
     *  7 | 1     x     1  1  1
     *  port 1 mode 'NATIVE_WEIGHTED'
     *  port 2 mode 'NATIVE_WEIGHTED'
     *  port 3 mode 'FORBID_ALL'
     *  the 'Designated MC Table' would look like this:
  *  port | 0  1  2  3  4  5  6
     *  --------------------------------
*index  0 | 1  x     -  1  1  1
     *  1 | 1     x  -  1  1  1
     *  2 | 1  x     -  1  1  1
     *  3 | 1     x  -  1  1  1
     *  4 | 1  x     -  1  1  1
     *  5 | 1     x  -  1  1  1
     *  6 | 1  x     -  1  1  1
     *  7 | 1    x   -  1  1  1
     *  port 1 mode 'NATIVE_WEIGHTED'
     *  port 2 mode 'NATIVE_WEIGHTED'
     *  port 3 mode 'FORCE_ALL'
     *  the 'Designated MC Table' would look like this:
  *  port | 0  1  2  3  4  5  6
     *  --------------------------------
*index  0 | 1  x     x  1  1  1
     *  1 | 1     x  x  1  1  1
     *  2 | 1  x     x  1  1  1
     *  3 | 1     x  x  1  1  1
     *  4 | 1  x     x  1  1  1
     *  5 | 1     x  x  1  1  1
     *  6 | 1  x     x  1  1  1
     *  7 | 1     x  x  1  1  1
     */
    CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORCE_ALL_E         = 2

} CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT;

/**
* @enum CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT
 *
 * @brief enumeration to allow application to be responsible for the
 * 'TrunkId of the port'.
 * in High level mode APIs the 'TrunkId of the port' is set internally be
 * the CPSS and may override the current value of the 'TrunkId of the port'.
 * This API allows application to notify the CPSS to not manage the 'TrunkId
 * of the port'.
*/
typedef enum{

    /** @brief (the default mode) the
     *  CPSS fully manage the 'trunkId' of the port.
     */
    CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_BY_CPSS_E,

    /** @brief the application
     *  fully manage the 'trunkId' of the port.
     */
    CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_BY_APPLICATION_E

} CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT;

/**
* @structure CPSS_TRUNK_SHARED_PORT_INFO_STC
 *
 * @brief structure to hold info about 'per trunk' behavior in 'shared port'
 *      that a trunk member can be shared between several trunks.
 *      This is part of the feature "Single Port � Multiple Trunks"
 *
*/
typedef struct
{
    /** @brief indication if the trunk will manipulate/skip the 'per port trunkId'
        GT_FALSE -  CPSS for this trunk will skip       the 'per port trunkId'
        GT_TRUE  -  CPSS for this trunk will manipulate the 'per port trunkId'
    */
    GT_BOOL  manipulateTrunkIdPerPort;
    /** @brief indication if the trunk will manipulate/skip the 'UC designated table'
        GT_FALSE -  CPSS for this trunk will skip       the 'UC designated table'
        GT_TRUE  -  CPSS for this trunk will manipulate the 'UC designated table'
    */
    GT_BOOL  manipulateUcDesignatedTable;
    /** @brief indication if the trunk will manipulate/skip the 'MC designated table'
        GT_FALSE -  CPSS for this trunk will skip       the 'MC designated table'
        GT_TRUE  -  CPSS for this trunk will manipulate the 'MC designated table'
    */
    GT_BOOL  manipulateMcDesignatedTable;
}CPSS_TRUNK_SHARED_PORT_INFO_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenTrunkTypesh */


