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
* @file mvHwsFalconPortIf.h
*
* @brief Falcon port interface header file
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsFalconPortIf_H
#define __mvHwsFalconPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>

/* number of ports in Falcon */
#define HWS_FALCON_PORTS_NUM_CNS    259
#define HWS_D2D_NUM_IN_RAVEN_CNS     2

#define PRV_HWS_D2D_GET_REDUCE_RAVEN_IDX_MAC(port) 0 /*TODO add REDUCE port*/
#define PRV_HWS_D2D_CONVERT_PORT_TO_RAVEN_MAC(port) (port/16)

#define PRV_HWS_D2D_CONVERT_PORT_TO_D2D_LOCAL_IDX_MAC(port)\
        (PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(port) % 2);

extern  GT_U32 hwsFalconNumOfRavens;
extern  GT_BOOL hwsPpHwTraceFlag;

/**
* @internal hwsFalconPortsElementsCfg function
* @endinternal
*
* @brief   Init Falcon port modes
*
* @param[in] devNum                   - system device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsFalconPortsElementsCfg
(
    IN GT_U8              devNum
);

/**
* @internal mvHwsFalconPortInit function
* @endinternal
*
* @brief   Falcon port init
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
GT_STATUS mvHwsFalconPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);
/**
* @internal mvHwsFalconPortReset function
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
GT_STATUS mvHwsFalconPortReset
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ACTION  action
);

/**
* @internal mvHwsFalconPortApInit function
* @endinternal
*
* @brief   Falcon port AP init
* (without mac/psc and unreset - thet is done from firmware)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsFalconPortCommonInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode
);

/**
* @internal mvHwsFalconPortApReset function
* @endinternal
*
* @brief   Clears the AP port mode and release all its resources
*         according to selected. Does not verify that the
*         selected mode/port number is valid at the core level
*         and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFalconPortApReset
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsMtipExtReset function
* @endinternal
*
* @brief  Reset/unreset MTIP unit
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] reset           - true = reset/ false = unreset
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 reset
);
/**
* @internal mvHwsMtipExtSetLaneWidth function
* @endinternal
*
* @brief  Set port lane width (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtSetLaneWidth
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT width
);

/**
* @internal mvHwsMpfSetPchMode function
* @endinternal
*
* @brief  Set mpf pch mode
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] pchMode         - pch mode
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSetPchMode
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 pchMode
);

/**
* @internal mvHwsMpfSauStatusGet function
* @endinternal
*
* @brief  Set MPFS SAU status/counter get - this counter is incremented on
*         every packet discard when the port is in link down
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] dropCntArrSize  - port mode
* @param[out] dropCntValueArr - (pointer to drop counter)
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSauStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN GT_U32                  dropCntArrSize,
    OUT GT_U32                  *dropCntValueArr
);

/**
* @internal mvHwsMtipExtSetChannelMode function
* @endinternal
*
* @brief  for speeds 100G/200G/400G Set port channel mode
*         (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/

GT_STATUS mvHwsMtipExtSetChannelMode
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
);


GT_STATUS hwsFalconPortElementsDbInit(GT_U8 devNum);
/**
* @internal mvHwsMtipExtLowJitterEnable function
* @endinternal
*
* @brief  EXT unit set low jitter for 10G port
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtLowJitterEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
);

/**
* @internal mvHwsMpfLoobackSet function
* @endinternal
*
* @brief   Set MPF loopback.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] lbType          - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMpfLoobackSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsMpfLoobackGet function
* @endinternal
*
* @brief   Get MPF loopback status.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] lbType         - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMpfLoobackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsFalconPortIf_H */



