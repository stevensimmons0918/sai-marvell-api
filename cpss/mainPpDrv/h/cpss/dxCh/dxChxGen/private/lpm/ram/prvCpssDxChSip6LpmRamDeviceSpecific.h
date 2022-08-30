/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamDeviceSpecific.h
*
* DESCRIPTION:
*       Declaration of the device specific functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1$
*
*******************************************************************************/

#ifndef __prvCpssDxChSip6LpmRamDeviceSpecifich
#define __prvCpssDxChSip6LpmRamDeviceSpecifich

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal prvCpssDxChIpLpmSip6LpmRamDeviceSpecificBankSizeGet function
 * @endinternal
 *
 * @brief Get hardware big bank size for Sip6 device
 *
 * @return GT_U32 - Big bank size
 *
*/
GT_U32 prvCpssDxChIpLpmSip6LpmRamDeviceSpecificBankSizeGet
(
    GT_U8 devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip6LpmRamDeviceSpecifich */
