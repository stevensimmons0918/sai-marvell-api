/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAc5xPortIf.h
*
* @brief AC5X port interface header file
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsAc5xPortIf_H
#define __mvHwsAc5xPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>


/**
* @internal mvHwsPhoenixEthPortPowerUp function
* @endinternal
*
* @brief  port init seq
*
* @param[in] devNum                 - system device number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] portSpeed              - Port speed
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsPhoenixEthPortPowerUp
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_SPEED_ENT    portSpeed
);

#if 0
/**
* @internal mvHwsEthPortPowerDown function
* @endinternal
*
* @brief  port power down seq
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsEthPortPowerDown
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);
#endif

/**
* @internal hwsPhoenixPortsElementsCfg function
* @endinternal
*
* @brief   Init Hawk port modes
*
* @param[in] devNum                   - system device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsPhoenixPortsElementsCfg
(
    IN GT_U8              devNum
);

/**
* @internal mvHwsPhoenixPortInit function
* @endinternal
*
* @brief   Hawk port init
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portInitInParamPtr - Input parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPhoenixPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);


#if 0
/**
* @internal mvHwsHawkPortApInit function
* @endinternal
*
* @brief   Hawk port AP init
* (without mac/psc and unreset - thet is done from firmware)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portSpeed          - Port speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsHawkPortCommonInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN MV_HWS_PORT_SPEED_ENT    portSpeed
);
#endif

GT_STATUS hwsPhoenixPortElementsDbInit(GT_U8 devNum);

GT_STATUS hwsPhoenixPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS hwsPhoenixPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAc5xPortIf_H */

