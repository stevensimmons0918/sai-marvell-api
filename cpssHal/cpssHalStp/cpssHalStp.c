/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalStp.c
*
* @brief Private API implementation which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/


#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include "cpssHalStp.h"

GT_STATUS cpssHalConvertStpPortStateXpsToCpss(xpVlanStgState_e xpsStpState,
                                              CPSS_STP_STATE_ENT *cpssStpState)
{
    //Converts the stp port state from xpVlanStgState_e to CPSS_STP_STATE_ENT
    switch (xpsStpState)
    {
        case SPAN_STATE_BLOCK:
            {
                *cpssStpState = CPSS_STP_BLCK_LSTN_E;
                break;
            }
        case SPAN_STATE_LEARN:
            {
                *cpssStpState =  CPSS_STP_LRN_E;
                break;
            }
        case SPAN_STATE_FORWARD:
            {
                *cpssStpState =  CPSS_STP_FRWRD_E;
                break;
            }
        case SPAN_STATE_DISABLE:
            {
                *cpssStpState =  CPSS_STP_DISABLED_E;
                break;
            }
        default:
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Unknown stp state %d\n",
                      xpsStpState);
                return GT_BAD_VALUE;
            }
    }

    return GT_OK;
}

/**
* @internal cpssHalSetBrgStpState function
* @endinternal
*
* @brief   Sets STP state of port belonging within an STP group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                      - device number
* @param[in] portNum                  port number
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
)

{
    GT_STATUS status = GT_OK;
    GT_U32 cpssDevNum;
    GT_U32 cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChBrgStpStateSet(cpssDevNum, cpssPortNum, stpId, state);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting Stp state failed for cpssDevNum=%d port=%d  stpId=%d", cpssDevNum,
              portNum, stpId);
        return status;
    }

    return status;
}

GT_STATUS cpssHalSetBrgPortStpMode
(
    int                         devId,
    GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_BRG_STP_STATE_MODE_ENT stpMode
)
{
    GT_STATUS status = GT_OK;
    GT_U32 cpssDevNum;
    GT_U32 cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChBrgStpPortSpanningTreeStateModeSet(cpssDevNum, cpssPortNum,
                                                        stpMode);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting Port Stp mode failed for cpssDevNum=%d port=%d  Mode=%d", cpssDevNum,
              portNum, stpMode);
        return status;
    }

    return status;
}

GT_STATUS cpssHalSetBrgPortStpState
(
    int                      devId,
    GT_PHYSICAL_PORT_NUM     portNum,
    CPSS_STP_STATE_ENT       state
)
{
    GT_STATUS status = GT_OK;
    GT_U32 cpssDevNum;
    GT_U32 cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChBrgStpPortSpanningTreeStateSet(cpssDevNum, cpssPortNum, state);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting port Stp state failed for cpssDevNum=%d port=%d  stpState=%d",
              cpssDevNum,
              portNum, state);
        return status;
    }

    return status;
}
