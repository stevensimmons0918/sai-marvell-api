/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalNhgrp.c
*
* @brief Private API implementation which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/


#include "cpssHalUtil.h"
#include "cpssHalNhgrp.h"
#include "xpsCommon.h"
#include "cpssHalDevice.h"
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>

/**
* @internal cpssHalWriteIpEcmpEntry function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devId                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/

GT_STATUS cpssHalWriteIpEcmpEntry
(
    int                          devId,
    GT_U32                       ecmpEntryIndex,
    CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)

{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, ecmpEntryPtr);
        if (status!=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add ECMP table entry: Index: %d", ecmpEntryIndex);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalIpEcmpEntryRandomEnableSet
(
    int                         devId,
    GT_U32                      ecmpEntryIndex,
    GT_BOOL                     enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;

    CPSS_DXCH_IP_ECMP_ENTRY_STC  ecmpEntryPtr;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryPtr);
        if (status!=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to read L3ECMP table entry: Index: %d", ecmpEntryIndex);
            return status;
        }

        ecmpEntryPtr.randomEnable = enable;

        status = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntryPtr);
        if (status!=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to write L3ECMP table entry: Index: %d", ecmpEntryIndex);
            return status;
        }
    }
    return status;
}