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
* @file mvHwsAc5pPortIf.h
*
* @brief AC5P port interface header file
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsAc5pPortIf_H
#define __mvHwsAc5pPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/**
 * @enum HWS_PM_MAC_PREEMPTION_TYPE_ENT
 *
 * @brief preemption mac counting mode
 */
typedef enum {

  /* disable mac preemption feature */
  MV_HWS_PM_MAC_PREEMPTION_DISABLED_E,

  /* enable mac preemption and aggregate counters (EMAC + PMAC) */
  MV_HWS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E,

  /* enable mac preemption and separate counters (EMAC, PMAC) */
  MV_HWS_PM_MAC_PREEMPTION_ENABLED_SEPARATED_E

} MV_HWS_PM_MAC_PREEMPTION_TYPE_ENT;


/**
* @internal mvHwsExtIfFirstInPortGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per port group (8 ports)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isFisrtPtr    - result if port is first or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfFirstInPortGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
);

/**
* @internal mvHwsExtIfLastInPortGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per port group (8 ports)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isLastPtr     - result if port is last or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfLastInPortGroupCheck
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT   *isLastPtr
);

/**
* @internal mvHwsExtIfFirstInSerdesGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isFisrtPtr    - result if port is first or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfFirstInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
);

/**
* @internal mvHwsExtIfLastInSerdesGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isLastPtr     - result if port is last or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfLastInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isLastPtr
);

/**
* @internal mvHwsEthPortPowerUp function
* @endinternal
*
* @brief  port init seq
*
* @param[in] devNum                 - system device number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] portInitInParamPtr     - port info struct
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsEthPortPowerUp
(
    IN GT_U8                            devNum,
    IN GT_U32                           phyPortNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS    *portInitInParamPtr
);

/**
* @internal mvHwsEthPortPowerDown function
* @endinternal
*
* @brief  port power down seq
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] stateInPortGroup         - first/existing/last int port group
* @param[in] stateInSerdesGroup       - first/existing/last int serdes group
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsEthPortPowerDown
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_IN_GROUP_ENT    stateInPortGroup,
    MV_HWS_PORT_IN_GROUP_ENT    stateInSerdesGroup
);

/**
* @internal hwsHawkPortsElementsCfg function
* @endinternal
*
* @brief   Init Hawk port modes
*
* @param[in] devNum                   - system device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsHawkPortsElementsCfg
(
    IN GT_U8              devNum
);

/**
* @internal mvHwsHawkPortInit function
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
GT_STATUS mvHwsHawkPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

/**
* @internal mvHwsHawkPortAnpStart function
* @endinternal
*
* @brief   Hawk port anp start (start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsHawkPortAnpStart
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_AP_CFG                  *apCfgPtr
);

/**
* @internal mvHwsHawkPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHawkPortReset
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ACTION  action
);

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
* @param[in] portInitInParamPtr - port info struct
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
    IN MV_HWS_PORT_INIT_INPUT_PARAMS *portInitInParamPtr
);

GT_VOID hwsHawkPortElementsDbInit
(
    GT_VOID
);

GT_STATUS hwsHawkPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS hwsHawkPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS     *portParams
);

GT_STATUS hwsHarrierPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS hwsHarrierPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS     *portParams
);

/**
* @internal mvHwsExtFecTypeGet function
* @endinternal
*
* @brief  RS Fec configure get
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
*
* @param[out] fecTypePtr       - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsExtFecTypeGet
(
    IN GT_U8                   devNum,
    IN  GT_U32                 portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE   *fecTypePtr
);

/**
* @internal mvHwsMacBrCountModeGet function
* @endinternal
*
* @brief  AC5P - enable/disable the sharing of counters from the
*         EMAC (express_ with those from the PMAC (preemptive).
*         Relates to the 802.3BR support of EMAC , PMAC
*
* @param[in] devNum                - system device number
* @param[in] portGroup             - port group
* @param[in] portNum               - physical port number
* @param[in] macType               - mac type
*
* @param[out] countModePtr         - (pointer to)
*                                    aggregated/separated
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacBrCountModeGet
(
    GT_U8                               devNum,
    GT_U8                               portGroup,
    GT_U32                              portNum,
    MV_HWS_PORT_MAC_TYPE                macType,
    MV_HWS_PM_MAC_PREEMPTION_TYPE_ENT  *countModePtr
);

/**
* @internal mvHwsMacBrCountModeSet function
* @endinternal
*
* @brief  AC5P - enable/disable the sharing of counters from the
*         EMAC (express_ with those from the PMAC (preemptive).
*         Relates to the 802.3BR support of EMAC , PMAC
*
* @param[in] devNum                - system device number
* @param[in] portGroup             - port group
* @param[in] portNum               - physical port number
* @param[in] macType               - mac type
* @param[in] countMode             - aggregated/separated
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacBrCountModeSet
(
    GT_U8                              devNum,
    GT_U8                              portGroup,
    GT_U32                             portNum,
    MV_HWS_PORT_MAC_TYPE               macType,
    MV_HWS_PM_MAC_PREEMPTION_TYPE_ENT  countMode
);

/**
* @internal mvHwsExtMacClockEnableGet function
* @endinternal
*
* @brief  EXT unit clock enable get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - (pointer to) port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtMacClockEnableGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      *enablePtr
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAc5pPortIf_H */

