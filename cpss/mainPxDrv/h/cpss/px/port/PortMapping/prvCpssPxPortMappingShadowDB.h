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
* @file prvCpssPxPortMappingShadowDB.h
*
* @brief interface to port mapping shadow DB
*
* @version   1
********************************************************************************
*/

#ifndef __PRV_CPSS_PX_PORTMAPPING_SHADOW_DB_H
#define __PRV_CPSS_PX_PORTMAPPING_SHADOW_DB_H

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortMapping.h> 

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBClear function
* @endinternal
*
* @brief   Clear entry in papping array
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBClear
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM physPortNum 
);

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBSet function
* @endinternal
*
* @brief   Clear entry in papping array
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
* @param[in] portMapPtr               - pointer to user mapping
* @param[in] portArgArr[CPSS_PX_PORT_MAPPING_TYPE_MAX_E] - pointer to detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBSet
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_PHYSICAL_PORT_NUM    physPortNum,
    IN CPSS_PX_PORT_MAP_STC   *portMapPtr,
    IN GT_U32                  portArgArr[CPSS_PX_PORT_MAPPING_TYPE_MAX_E]
);

/**
* @internal prvCpssPxPortPhysicalPortMapShadowDBGet function
* @endinternal
*
* @brief   get pointer to entry in mapping array
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @param[out] portMapShadowPtrPtr      - (pointer to) entry in mapping DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortMapShadowDBGet
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_PHYSICAL_PORT_NUM    physPortNum,
    OUT  CPSS_PX_DETAILED_PORT_MAP_STC **portMapShadowPtrPtr
);

/**
* @internal prvCpssPxPortPhysicalPortIsCpu function
* @endinternal
*
* @brief   check whether given port is CPU port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPortNum              - physical port number
*
* @retval GT_TRUE                  - on success (CPU port)
* @retval GT_FALSE                 - (if not)
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPhysicalPortIsCpu
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_PHYSICAL_PORT_NUM    physPortNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

