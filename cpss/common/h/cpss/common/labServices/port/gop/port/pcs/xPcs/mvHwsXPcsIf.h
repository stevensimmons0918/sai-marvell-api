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
* @file mvHwsXPcsIf.h
*
* @brief GPCS interface API
*
* @version   10
********************************************************************************
*/

#ifndef __mvHwsXPcsIf_H
#define __mvHwsXPcsIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsXPcsIfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsIfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray);

/**
* @internal mvHwsXPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsReset
(
    IN GT_U8                   devNum,
    IN GT_UOPT                 portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_RESET            action
);

/**
* @internal mvHwsXPcsMode function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsMode
(
    IN GT_U8                   devNum,
    IN GT_UOPT                 portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
);

/**
* @internal mvHwsXPcsLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsLoopBack
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsXPcsLoopBackGet function
* @endinternal
*
* @brief   Return the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number 
* @param[in] portMode                 - port mode 
*  
* @param[out] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsLoopBackGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsXPcsTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsTestGenCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

/**
* @internal mvHwsXPcsTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate 
*  
* @param[out] status                  -  test gen status 
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsTestGenStatus
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    OUT MV_HWS_TEST_GEN_STATUS        *status
);

/**
 * @internal mvHwsXPcsActiveStatusGet function
 * @endinternal
 *
 * @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
 *
 * @param[in] devNum                   - system device number
 * @param[in] portGroup                - port group (core) number
 * @param[in] pcsNum                   - physical PCS number
 *
 * @param[out] numOfLanes               - number of lanes agregated in PCS
 *
 * @retval 0                        - on success
 * @retval 1                        - on error
 */
GT_STATUS mvHwsXPcsActiveStatusGet
(
     IN GT_U8                   devNum,
     IN GT_U32                  portGroup,
     IN GT_U32                  pcsNum,
     OUT GT_U32                  *numOfLanes
);

/**
* @internal mvHwsXPcsConnectWa function
* @endinternal
*
* @brief   XPcs Connect Wa.
*
*   1.  check sync indication for each lane /Cider/EBU/Lion2/Lion2_B1 {B1 freeze}/Lion2_B1 Units/GOP/XPCS %p Units/Lane 0 Registers/Lane Status and Interrupt/Lane Status bit 4
*   2.  If not all are synced then perform rx reset/unreset for each lane /Cider/EBU/Lion2/Lion2_B1 {B1 freeze}/Lion2_B1 Units/GOP/XPCS %p Units/Lane 0 Registers/Lane Configuration/Lane Configuration 0 bit 1
*   3.  Wait on signal detect change interrupt /Cider/EBU/Lion2/Lion2_B1 {B1 freeze}/Lion2_B1 Units/GOP/MMPCS_IP %P Units/PCS40G Common Interrupt Cause bit 12 (make sure it's not set for sync change interrupt)
*   4.  Upon interrupt check Cider/EBU/Lion2/Lion2_B1 {B1 freeze}/Lion2_B1 Units/GOP/COMPHY_H %t Registers/<COMPHY_H(r2p1)> COMPHY_H Units/Page 1/Squelch and FFE Setting bit 14
*   5.  If 1 then goto 3
*   6.  If 0 then goto 1 
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
GT_STATUS mvHwsXPcsConnectWa
(
     IN GT_U8                   devNum,
     IN GT_U32                  portGroup,
     IN GT_U32                  portNum
);

/**
 * @internal mvHwsXPcsSignalDetectBypassEn function
 * @endinternal
 *
 * @brief   Set all related PCS with Signal Detect Bypass value (1/0).
 *
 * @param[in] devNum                   - system device number
 * @param[in] portGroup                - port group (core) number
 * @param[in] pcsNum                   - physical PCS number
 * @param[in] maskEn                   - if true, enable signal detect mask bit
 *
 * @retval 0                           - on success
 * @retval 1                           - on error
*/
GT_STATUS mvHwsXPcsSignalDetectBypassEn
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN GT_BOOL                 maskEn
);

/**
* @internal mvHwsXPcsLinkStatusGet function
* @endinternal
*
* @brief   Check link status in XPCS
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*
* @param[out] linkStatus                - link status
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXPcsLinkStatusGet
(
    IN GT_U8                   devNum,
    IN GT_UOPT                 portGroup,
    IN GT_UOPT                 pcsNum,
    OUT GT_BOOL                 *linkStatus
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsXPcsIf_H */


