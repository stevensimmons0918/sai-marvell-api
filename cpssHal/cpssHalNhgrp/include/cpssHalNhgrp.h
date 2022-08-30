/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalNhgrp.h
*
* @brief Private API declaration which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include  <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include "cpssDxChIpTypes.h"


/**
* @internal cpssHalWriteIpEcmpEntry function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devId                   - the device Id
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
    int                         devId,
    GT_U32                       ecmpEntryIndex,
    CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
);

GT_STATUS cpssHalIpEcmpEntryRandomEnableSet
(
    int                         devId,
    GT_U32                      ecmpEntryIndex,
    GT_BOOL                     enable
);


#ifdef __cplusplus
}
#endif
