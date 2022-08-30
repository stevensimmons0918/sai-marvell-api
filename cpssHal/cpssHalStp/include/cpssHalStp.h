/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalStp.h
*
* @brief Private API declarations which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "xpsCommon.h"
#include  <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>

GT_STATUS cpssHalConvertStpPortStateXpsToCpss(xpVlanStgState_e xpsStpState,
                                              CPSS_STP_STATE_ENT *cpssStpState);
/**
* @internal cpssHalSetBrgStpState function
* @endinternal
*
* @brief   Sets STP state of port belonging within an STP group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                    - device Id
* @param[in] portNum                  - port number
* @param[in] stpId                    - STG (Spanning Tree Group) index
* @param[in] state                    - STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port or stpId or state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalSetBrgStpState
(
    int                      devId,
    GT_PHYSICAL_PORT_NUM     portNum,
    GT_U16                   stpId,
    CPSS_STP_STATE_ENT       state

);

GT_STATUS cpssHalSetBrgPortStpMode
(
    int                         devId,
    GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_BRG_STP_STATE_MODE_ENT stpMode
);

GT_STATUS cpssHalSetBrgPortStpState
(
    int                      devId,
    GT_PHYSICAL_PORT_NUM     portNum,
    CPSS_STP_STATE_ENT       state
);

#ifdef __cplusplus
}
#endif
