/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxVersion.h
*
* @brief Includes software CPSS PX version information.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxVersion_h__
#define __cpssPxVersion_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/generic/version/gtVersion.h>
#include <cpssCommon/cpssPresteraDefs.h>

#define CPSS_PX_VERSION_CNS      "CPSS 4.3.5"

/**
* @internal cpssPxVersionGet function
* @endinternal
*
* @brief   This function returns CPSS PX version.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] versionPtr               - (pointer to) CPSS PX version info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one parameter is NULL pointer
* @retval GT_BAD_SIZE              - the version name is too long
*/
GT_STATUS cpssPxVersionGet
(
    OUT GT_VERSION   *versionPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxVersion_h__ */

