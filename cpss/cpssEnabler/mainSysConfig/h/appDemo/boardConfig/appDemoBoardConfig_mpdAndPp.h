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
* @file appDemoBoardConfig_mpdAndPp.h
*
* @brief  hold common function (tools) for the board config files to use to get
*   MPD functions to use. NOTE: EZ_BRINGUP also exists.
*
* @version   1
********************************************************************************
*/
#ifndef __appDemoBoardConfig_mpdAndPp_H
#define __appDemoBoardConfig_mpdAndPp_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (defined INCLUDE_MPD) && (defined CHX_FAMILY)

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/**
* @internal appDemoEzbMpdPpPortInit function
* @endinternal
*
* @brief  init all the potrs in all the devices to bind those that supports PHY
*   to the MPD (PHY driver) and set all port to 'admin down' (not force link up)
*   (prvAppDemoEzbMpdPortInit)
*   also set the PP MAC ports with : SMI addresses and 'inband AN' with the phy
*
*/
GT_STATUS appDemoEzbMpdPpPortInit
(
    void
);


/**
* @internal appDemoEzbSwitchMacAutoNegCompleted function
* @endinternal
*
* @brief  notify that got event of 'CPSS_PP_PORT_AN_COMPLETED_E' from the port in the PP
*       and need to synch the PP with the MPD info (with the PHY)
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] portNum              - the physical port number
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS appDemoEzbSwitchMacAutoNegCompleted(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal appDemoEzbPortSpeedSet function
* @endinternal
*
* @brief  Power, Set duplex mode and Set the provided speed of the PHY
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] portNum              - the physical port number
* @param[in] portSpeed            - the physical port speed
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS appDemoEzbPortSpeedSet(
        IN GT_SW_DEV_NUM            devNum,
        IN GT_PHYSICAL_PORT_NUM     portNum,
        IN CPSS_PORT_SPEED_ENT      portSpeed
);
#else /*!INCLUDE_MPD*/
/* implement MACROS STUBs for compilation */
#define appDemoEzbMpdPpPortInit()                 GT_NOT_IMPLEMENTED
#define appDemoEzbSwitchMacAutoNegCompleted(a,b)  GT_NOT_IMPLEMENTED
#define appDemoEzbPortSpeedSet(a,b,c)             GT_NOT_IMPLEMENTED

#endif /*!INCLUDE_MPD*/

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /*__appDemoBoardConfig_mpdAndPp_H*/



