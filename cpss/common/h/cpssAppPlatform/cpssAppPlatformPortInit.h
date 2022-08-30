/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatformPortInit.h
*
* @brief Port configuration functions using Port Manager.
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_PORTINIT_H
#define __CPSS_APP_PLATFORM_PORTINIT_H

#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/**
* @internal cpssAppPlatformPmPortCreate function
* @endinternal
*
* @brief   Create port using port manager.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
);

/**
* @internal cpssAppPlatformPmPortStatusChangeSignal function
* @endinternal
*
* @brief   Handler for port events. This API will pass
*         CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E (or
*         CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E) event to port manager
*         in order to notify it about a (possibly) change in the port status,
*         and then will initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*         event (as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state)
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] uniEvent              - unified event raised on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortStatusChangeSignal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    GT_U32                      uniEvent
);

/**
* @internal cpssAppPlatformPmPortEnable function
* @endinternal
*
* @brief   Enable the port. This API is the complement of cpssAppPlatformPmPortDisable
*         API, while the later disabled the port, this API will enable it back.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortEnable
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssAppPlatformPmPortLinkStatusChangeSignal function
* @endinternal
*
* @brief   Handler function for link status change event. This API will pass
*         the event to the port manager and perform actions if needed.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortLinkStatusChangeSignal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssAppPlatformPmPortDisable function
* @endinternal
*
* @brief   Disable the port. This API is the complement of cpssAppPlatformPmPortEnable
*          API, while the later enables the port, this API will disable it.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortDisable
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssAppPlatformPmPortDelete function
* @endinternal
*
* @brief   Delete port.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssAppPlatformPmTaskCreate function
* @endinternal
*
* @brief   create port manager application demo task
*
* @param[in] priority              - task priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmTaskCreate
(
    IN GT_U32 priority
);

/**
* @internal cpssAppPlatformPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up port using port manager.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - interface mode for the port
* @param[in] speed                 -  for the port
* @param[in] powerUp               - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPortInitSeqStart
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed,
    IN  CPSS_PORT_FEC_MODE_ENT       fecMode,
    IN  GT_BOOL                      powerUp
);

/**
* @internal cpssAppPlatformApPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up AP port using port manager.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - interface mode for the port
* @param[in] speed                 -  for the port
* @param[in] powerUp               - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformApPortInitSeqStart
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed,
    IN  GT_BOOL                      powerUp
);

/**
* @internal cpssAppPlatformIsPortMgrPort function
* @endinternal
*
* @brief   Check whether port is managed by port manager
*
* @param[in] devNum  - device number
* @param[in] portNum - port number
*
* @retval GT_TRUE    - If port is managed by port manager
* @retval GT_FALSE   - If port is not managed by port manager
*/
GT_BOOL cpssAppPlatformIsPortMgrPort
(
    IN  GT_U8                devNum
);
/**
* @internal cpssAppPlatformPmTaskDelete function
* @endinternal
*
* @brief   Delete port manager application demo task
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmTaskDelete
(
    GT_VOID
);

#endif /* __CPSS_APP_PLATFORM_PORTINIT_H */
