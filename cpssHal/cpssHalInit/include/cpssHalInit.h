/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitialize.h
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __INCLUDE_MRVL_HAL_INIT_API_H
#define __INCLUDE_MRVL_HAL_INIT_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpssHalProfile.h>

extern void (*cpssHalLinkStateNotify)(xpsDevice_t devId, uint32_t portNum,
                                      int linkStatus);

extern GT_STATUS cpssHalApiReturn
(
    const char *func_name,
    int         line,
    const char *text,
    GT_STATUS   status
);
#define MRVL_HAL_API_RETURN(_text, _status) do { if ( GT_OK != _status)  return cpssHalApiReturn(__func__, __LINE__, _text, _status); } while(0)
/*******************************************************************************
* cpssHalInitializeDeviceApi
*
* DESCRIPTION:
*       initialize cpss driver and specific device
*       save device type and parameters in db
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*       profileSetFnCallback - might be NULL
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       first phase support systems with single device
*
*******************************************************************************/
GT_STATUS cpssHalInitializeDeviceApi
(
    GT_U8                   devNum,
    XP_DEV_TYPE_T           devType,
    PROFILE_STC                 profile[] /* can be null*/
);

/*******************************************************************************
* cpssHalRunApi
*
* DESCRIPTION:
*       enable device, enable ports and enable interrupts
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssHalRunApi
(
    void
);

/*******************************************************************************
* cpssHalDevInfoDumpApi
*
* DESCRIPTION:
*       prints device and port information
*       dump information from hw and from saved db
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssHalDevInfoDumpApi
(
    GT_U8                                               devNum

);

extern GT_BOOL cmdStreamGrabSystemOutput ;

GT_STATUS cpssHalGetNumPorts
(
    GT_U8                devNum,
    GT_U32             *numOfPorts
);

GT_STATUS cpssHalInitializeValidateProfile
(
    XP_DEV_TYPE_T devType,
    PROFILE_STC              profile[] /* can be null*/
);

GT_STATUS cpssHalSimInit
(
    PROFILE_STC profile[]
);

GT_STATUS cpssHalInitializeSystem
(
    void
);

GT_STATUS cpssHalEnableLog
(
    int enable
);

GT_STATUS cpssHalInitializePort
(
    GT_U8                       devNum,
    XP_DEV_TYPE_T               xpDevType,
    GT_PHYSICAL_PORT_NUM            portNum,
    CPSS_PORT_SPEED_ENT         speed,
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode,
    GT_BOOL                     existPhy,
    CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    CPSS_PHY_XSMI_INTERFACE_ENT  xsmiInterface,
    GT_U32                      smiAddress,
    int                             isRxTxParamValid,
    CPSS_PORT_SERDES_RX_CONFIG_UNT  *rxParam,
    CPSS_PORT_SERDES_TX_CONFIG_UNT  *txParam
);

GT_STATUS cpssHalResetDevice(int devId);

/*******************************************************************************
* cpssHalLedProfileSet
*
* DESCRIPTION:
*       Update Led profile
*
* APPLICABLE DEVICES:
*        Ac3x.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devId.
*       ledModeStr
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssHalLedProfileSet(int devId, const char* ledModeStr);

/*******************************************************************************
* cpssHalLedProfileGet
*
* DESCRIPTION:
*       Get Led profile
*
* APPLICABLE DEVICES:
*        Ac3x.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devId.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       LED_PROFILE_TYPE_E
*
* COMMENTS:
*
*******************************************************************************/
LED_PROFILE_TYPE_E cpssHalLedProfileGet(int devId);

GT_STATUS cpssHalWarmResetComplete();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__INCLUDE_MRVL_HAL_INIT_API_H*/
