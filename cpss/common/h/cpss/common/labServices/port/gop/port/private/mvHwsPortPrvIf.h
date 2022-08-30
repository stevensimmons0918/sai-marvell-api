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
* @file mvHwsPortPrvIf.h
*
* @brief
*
* @version   19
********************************************************************************
*/

#ifndef __mvHwsPortPrvIf_H
#define __mvHwsPortPrvIf_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

/**
* @internal hwsPortFixAlign90 function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFixAlign90
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal hwsPortGetTuneMode function
* @endinternal
*
* @brief   returns the tune mode according to port mode..
*
* @param[in] portMode                 - port standard metric
*
* @param[out] tuneMode                - tune mode type
* @param[out] isTrainingMode          - in training mode or not
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortGetTuneMode
(
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_MAN_TUNE_MODE   *tuneMode,
    OUT GT_BOOL                     *isTrainingMode
);
#endif

/**
* @internal mvHwsRebuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector and rebuild active lane array. Can be called during CreatePort
*         only.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] laneList                -  lane list
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRebuildActiveLaneList
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                portNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    OUT GT_U32                *laneList
);

/**
* @internal mvHwsBuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildActiveLaneList
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                portNum,
    IN MV_HWS_PORT_STANDARD  portMode
);
#if 0
/**
* @internal mvHwsDestroyActiveLaneList function
* @endinternal
*
* @brief   Free lane list allocated memory.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyActiveLaneList
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                portNum,
    IN MV_HWS_PORT_STANDARD  portMode
);
#endif
/**
* @internal mvHwsBuildDeviceLaneList function
* @endinternal
*
* @brief   Build the device lane list per port and per mort mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildDeviceLaneList
(
   IN GT_U8    devNum,
   IN GT_U32   portGroup
);
#if 0
/**
* @internal mvHwsDestroyDeviceLaneList function
* @endinternal
*
* @brief   Free the device lane list allocated memory.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyDeviceLaneList
(
   IN GT_U8    devNum,
   IN GT_U32   portGroup
);
#endif
/**
* @internal mvHwsMmPcs40GConnectWa function
* @endinternal
*
* @brief   40G_Link_WA that need to add to the Create Port command at 40G mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] number                   - number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMmPcs40GConnectWa
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                number
);

/**
* @internal mvHwsMmPcs40GBackWa function
* @endinternal
*
* @brief
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - mac num
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMmPcs40GBackWa
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsMmPcs28nm40GBackWa function
* @endinternal
*
* @brief
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - port mac num
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMmPcs28nm40GBackWa
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsXPcsConnect function
* @endinternal
*
* @brief   XPcs Connect Wa.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
*
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_BAD_PTR                  - if active Lane List is not defined for port
*/
GT_STATUS mvHwsXPcsConnect
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortPrvIf_H */



