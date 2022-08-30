/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssPxPortManager.h
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxPortManager
#define __cpssPxPortManager

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/private/prvCpssPortManager.h>

/**
* @internal cpssPxPortManagerEventSet function
* @endinternal
*
* @brief   Set the port according to the given event. This API performs actions based on
*         the given event and will transfer the port to a new state. For example, when port
*         is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is called,
*         this API will perform port creation operations and upon success, port state will be
*         changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portEventStcPtr          - (pointer to) structure containing the event data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortManagerEventSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStcPtr
);

/**
* @internal cpssPxPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortManagerStatusGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

/**
* @internal cpssPxPortManagerPortParamsSet function
* @endinternal
*
* @brief   Set the given attributes to a port. Those configurations will take place
*         during the port management. This API is applicable only when port is in
*         reset (PORT_MANAGER_STATE_RESET_E state).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortManagerPortParamsSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PM_PORT_PARAMS_STC                *portParamsStcPtr
);

/**
* @internal cpssPxPortManagerPortParamsGet function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssPxPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortManagerPortParamsGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PM_PORT_PARAMS_STC                *portParamsStcPtr
);

/**
* @internal cpssPxPortManagerPortParamsStructInit function
* @endinternal
*
* @brief  This API resets the Port Manager Parameter structure
*         so once the application update the structure with
*         relevant data - all other feilds are clean for sure.
*         Application must call this API before calling
*         cpssPxPortManagerPortParamsSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in,out] portParamsStcPtr   - (pointer to) a structure
*       contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssPxPortManagerPortParamsStructInit
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    INOUT  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr
);

/**
* @internal cpssPxPortManagerGlobalParamsOverride function
* @endinternal
*
* @brief  To use this function for overide, need to give it a
*         struct that include parameters to override, and to
*         enable override flag to 1, else it's will not override
*         and use defualts values.
*         Don't forget to fill the globalParamstType enum with
*         the match enum of what you want to override.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] globalParamsStc          - stc with the parameters
*                                       to override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To add other globals parameter to the struct, make sure
*       to add enum and enable flag for the set of values.
*       *See CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC as a
*        good format to add other parameters.
*
*/
GT_STATUS cpssPxPortManagerGlobalParamsOverride
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC    *globalParamsStcPtr
);

/* @internal cpssPxPortManagerPortParamsUpdate function
* @endinternal
*
* @brief  To use this function for update the PA mode also when port is active. In order to configure the new mode,
* application should call the port manager update API (called cpssPxPortManagerPortParamsUpdate ) with the new
* specified mode
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] updateParamsStc          - stc with the parameters to update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortManagerPortParamsUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *updateParamsStcPtr
);

/**
* @internal cpssPxPortManagerInit function
* @endinternal
*
* @brief  Port Manager Init system - this function is
*         responsible on all configurations that till now was
*         application responsiblity.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  .
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssPxPortManagerInit
(
    IN  GT_SW_DEV_NUM  devNum
);

/**
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @note   APPLICABLE DEVICES:      Pipe
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssPxPortManagerUnMaskModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
);

/**
* @internal cpssPxPortManagerEnableGet function
* @endinternal
*
* @brief  This function is returns if port manager enabled/disabled
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[out] enablePtr               - (pointer to)port manager enabled/disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssPxPortManagerEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssPxPortManagerStatGet function
* @endinternal
*
* @brief  This function is returns the port manager statistics
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum             - physical device number
* @param[in] portNum            - port number
* @param[out] portStatStcPtr    - statistics structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssPxPortManagerStatGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_MANAGER_STATISTICS_STC    *portStatStcPtr
);

/**
* @internal cpssPxPortManagerStatClear function
* @endinternal
*
* @brief  This function clear the port manager statistics.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum             - physical device number
* @param[in] portNum            - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssPxPortManagerStatClear
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortManager */

