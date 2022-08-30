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
* @file mvHwsPcsIf.h
*
* @brief
*
* @version   17
********************************************************************************
*/

#ifndef __mvHwsPcsIf_H
#define __mvHwsPcsIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>


#ifndef MV_HWS_REDUCED_BUILD
/**
 * TBD added despite request to split L1 code due to frequent
 * use
 */
#include <cpssCommon/cpssPresteraDefs.h>
#else
#ifdef MICRO_INIT
#define BOOL2BIT_MAC(x) (((x) == GT_FALSE) ? 0 : 1)
#endif
#endif

typedef GT_STATUS (*MV_HWS_PCS_CHECK_GEAR_BOX)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

typedef GT_STATUS (*MV_HWS_PCS_ALIGN_LOCK_GET_BOX)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *alignLock
);

typedef GT_STATUS (*MV_HWS_PCS_RESET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

typedef GT_STATUS (*MV_HWS_PCS_RX_RESET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_RESET            action
);

typedef GT_STATUS (*MV_HWS_PCS_MODE_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
);

typedef GT_STATUS (*MV_HWS_PCS_LB_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

typedef GT_STATUS (*MV_HWS_PCS_LB_CFG_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

typedef GT_STATUS (*MV_HWS_PCS_TEST_GEN_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

typedef GT_STATUS (*MV_HWS_PCS_TEST_GEN_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
);

typedef GT_STATUS (*MV_HWS_PCS_EXT_PLL_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum
);

typedef GT_STATUS (*MV_HWS_PCS_SIGNAL_DETECT_MASK_SET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 maskEn
);

typedef GT_STATUS (*MV_HWS_PCS_FEC_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
);

typedef GT_STATUS (*MV_HWS_PCS_FEC_CFG_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
);

typedef GT_STATUS (*MV_HWS_PCS_ACTIVE_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_U32                  *numOfLanes
);

typedef char* (*MV_HWS_PCS_TYPE_GET_FUNC_PTR)(void);

typedef GT_STATUS (*MV_HWS_PCS_SEND_FAULT_SET_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 send
);

typedef GT_STATUS (*MV_HWS_PCS_SEND_LOCAL_FAULT_SET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 action
);

typedef GT_STATUS (*MV_HWS_PCS_LINK_STATUS_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 *linkStatus
);

typedef struct
{
    GT_BOOL inbandAnEnable;
    GT_BOOL flowCtrlPauseAdvertiseEnable;
    GT_BOOL flowCtrlAsmAdvertiseEnable;
    GT_BOOL byPassEnable;
} MV_HWS_PCS_AUTONEG_1G_SGMII;

typedef GT_STATUS (*MV_HWS_PCS_AUTONEG_1G_SGMII_SET_PTR)
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PCS_AUTONEG_1G_SGMII *autoNegotiationPtr
);

/**
* @struct MV_HWS_PCS_FUNC_PTRS
*
* @brief Defines the PCS function pointers
*/
typedef struct
{
    MV_HWS_PCS_RESET_FUNC_PTR           pcsResetFunc;
    MV_HWS_PCS_MODE_CFG_FUNC_PTR        pcsModeCfgFunc;
    MV_HWS_PCS_ALIGN_LOCK_GET_BOX       pcsAlignLockGetFunc;
    MV_HWS_PCS_LINK_STATUS_GET_FUNC_PTR pcsLinkStatusGetFunc;
#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
    MV_HWS_PCS_TYPE_GET_FUNC_PTR        pcsTypeGetFunc;
#endif
#ifndef RAVEN_DEV_SUPPORT
    MV_HWS_PCS_TEST_GEN_FUNC_PTR        pcsTestGenFunc;
    MV_HWS_PCS_TEST_GEN_STATUS_FUNC_PTR pcsTestGenStatusFunc;
    MV_HWS_PCS_RX_RESET_FUNC_PTR        pcsRxResetFunc;
    MV_HWS_PCS_SIGNAL_DETECT_MASK_SET_FUNC_PTR  pcsSignalDetectMaskEn;
    MV_HWS_PCS_ACTIVE_STATUS_FUNC_PTR   pcsActiveStatusGetFunc;
    MV_HWS_PCS_EXT_PLL_CFG_FUNC_PTR     pcsExtPllCfgFunc;
    MV_HWS_PCS_LB_CFG_FUNC_PTR          pcsLbCfgFunc;
    MV_HWS_PCS_LB_CFG_GET_FUNC_PTR      pcsLbCfgGetFunc;
    MV_HWS_PCS_CHECK_GEAR_BOX           pcsCheckGearBoxFunc;
    MV_HWS_PCS_SEND_LOCAL_FAULT_SET_FUNC_PTR pcsSendLocalFaultSetFunc;
    MV_HWS_PCS_FEC_CFG_FUNC_PTR         pcsFecCfgFunc;
    MV_HWS_PCS_FEC_CFG_GET_FUNC_PTR     pcsFecCfgGetFunc;
    MV_HWS_PCS_SEND_FAULT_SET_PTR       pcsSendFaultSetFunc;
    MV_HWS_PCS_AUTONEG_1G_SGMII_SET_PTR  pcsAutoNeg1GSgmiiFunc;
#endif
}MV_HWS_PCS_FUNC_PTRS;

/**
* @internal hwsPcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPcsIfInit(IN GT_U8 deviceNum);

/**
* @internal mvHwsPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_RESET            action
);

/**
* @internal mvHwsPcsModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsModeCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsLoopbackCfg function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsPcsLoopbackCfgGet function
* @endinternal
*
* @brief   Return the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[out] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfgGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
);

/**
 * @internal mvHwsPcsAutoNeg1GSgmii function
 * @endinternal
 *
 * @brief   Configure Auto-negotiation for SGMII/1000BaseX port
 *          modes.
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param portMode
 * @param autoNegotiationPtr
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsPcsAutoNeg1GSgmii
(
    IN GT_U8                        devNum,
    IN GT_U32                       phyPortNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PCS_AUTONEG_1G_SGMII  *autoNegotiationPtr
);
#endif
/**
* @internal mvHwsPcsTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] pattern                  -  to generate
* @param[out] status                  -  status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenStatus
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    OUT MV_HWS_TEST_GEN_STATUS        *status
);
#endif

/**
* @internal mvHwsPcsRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsRxReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_RESET            action
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsExtPllCfg function
* @endinternal
*
* @brief   Set the selected PCS type and number to external PLL mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsExtPllCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType
);
#endif

/**
* @internal mvHwsPcsSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSignalDetectMaskSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN GT_BOOL                 maskEn
);

/**
* @internal mvHwsPcsFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] portFecType              -
*       0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC,
*       2-AP_ST_HCD_FEC_RES_RS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfig
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_FEC_MODE    portFecType
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsFecConfigGet function
* @endinternal
*
* @brief   Return FEC disable/enable status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfigGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
);

/**
* @internal mvHwsPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsActiveStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_U32                  *numOfLanes
);

/**
* @internal mvHwsPcsCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes (currently used only on MMPCS).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] numOfLanes               - number of lanes agregated in PCS
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsCheckGearBox
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *laneLock
);
#endif
/**
* @internal mvHwsPcsAlignLockGet function
* @endinternal
*
* @brief   Read align status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsAlignLockGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
);

/**
* @internal mvHwsPcsPortLinkStatusGet function
* @endinternal
*
* @brief   Read link status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] linkStatus                - link Status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsPortLinkStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *linkStatus
);

/**
* @internal hwsPcsIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported PCS
*          types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPcsIfClose
(
    IN GT_U8                   devNum
);


/**
* @internal hwsPcsGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPcsGetFuncPtr
(
    GT_U8                 devNum,
    MV_HWS_PCS_FUNC_PTRS  ***hwsFuncsPtr
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start sending fault signals to partner, at both 10G and 40G.
*         On 10G the result will be local-fault on the sender and remote-fault on the receiver,
*         on 40G there will be local-fault on both sides, but there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - Pcs Number
* @param[in] pcsType                  - Pcs type
* @param[in] portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSendFaultSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 send
);

/**
* @internal mvHwsPcsFastLinkDownSet function
* @endinternal
*
* @brief   Perform Fast link DOWN on specific port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - PCS number
* @param[in] pcsType                  - PCS type according to port configuration
* @param[in] portMode                 - Port Mode
* @param[in] enable                   - GT_TRUE for port enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSendLocalFaultSet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroup,
    IN GT_U32               pcsNum,
    IN MV_HWS_PORT_PCS_TYPE pcsType,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_BOOL              enable
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPcsIf_H */


