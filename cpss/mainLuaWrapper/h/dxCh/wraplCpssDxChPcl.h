/*******************************************************************************
*              (c), Copyright 2011, Marvell International Ltd.                 *
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
* @file wraplCpssDxChDeviceInfo.h
*
* @brief device info wrapper
*
* @version   1
********************************************************************************
*/

#ifndef __wraplCpssDxChPcl__
#define __wraplCpssDxChPcl__

#include <cpssCommon/wraplCpssExtras.h>
#include <cpssCommon/wrapCpssDebugInfo.h>

/*******************************************************************************
* prvLuaCpssDxChMultiPclTcamLookupSet
*
* DESCRIPTION:
*       Set direction and lookup for device for retrieving PCL TCAM index.
*
* @param[in] devNum         - device number
* @param[in] direction      - Pcl direction
* @param[in] lookupNum      - Pcl Lookup number
*
* RETURNS: none
*
*******************************************************************************/
void prvLuaCpssDxChMultiPclTcamLookupSet
(
    IN GT_U8                           devNum,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum
);

/*******************************************************************************
* prvLuaCpssDxChMultiPclTcamLookupReset
*
* DESCRIPTION:
*       Reset direction and lookup for device for retrieving PCL TCAM index.
*
* @param[in] devNum         - device number
*
* RETURNS: none
*
*******************************************************************************/
void prvLuaCpssDxChMultiPclTcamLookupReset
(
    IN   GT_U8                         devNum
);

/*******************************************************************************
* prvLuaCpssDxChMultiPclTcamIndexGet
*
* DESCRIPTION:
*       Get  PCL TCAM index by stored in DB direction and lookup for given device.
*
* @param[in] devNum         - device number
*
* RETURNS: - 0xFFFFFFFF if device number is wrong or data in DB for it is invalid.
*          - 0 for not AC5 devices
*          - TCAM index configured in HW for direction ond lookup stored in DB.
*
*
*******************************************************************************/
GT_U32 prvLuaCpssDxChMultiPclTcamIndexGet
(
    IN   GT_U8                         devNum
);

#endif /* __wraplCpssDxChPcl__ */


