/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamDeviceSpecific.c
*
* DESCRIPTION:
*       Implementation of the device specific functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1$
*
*******************************************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamDeviceSpecific.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

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
)
{
    GT_U32 hwBigBankSize;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

    if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == devFamily)
    {
        /* patch for Ironman-L */
        hwBigBankSize = 1536;   /* Ironman-L  */
    }
    else
    if(CPSS_PP_FAMILY_DXCH_HARRIER_E == devFamily)
    {
        /* patch for Harrier */
        hwBigBankSize = _1K;   /* Harrier */
    }
    else
    if(CPSS_PP_FAMILY_DXCH_AC5X_E == devFamily)
    {
        /* patch for Phoenix */
        hwBigBankSize = _1K;   /* Phoenix */
    }
    else
    if(CPSS_PP_FAMILY_DXCH_AC5P_E == devFamily)
    {
        /* patch for Hawk */
        hwBigBankSize = _10K;   /* Hawk */
    }
    else
    {
        hwBigBankSize = 14 * _1K; /* Falcon */
    }

    return hwBigBankSize;
}

