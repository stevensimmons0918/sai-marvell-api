
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
* @file mvHwsPortCtrlAn.h
*
* @brief Port Control AP Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlAn_H
#define __mvHwsPortCtrlAn_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define AN_PRINT_MAC(x) /* \
    osPrintf("%d: ",mvPortCtrlCurrentTs());\
    osPrintf x*/
#define AN_PRINT_MAC_ERR(x) /*\
    osPrintf("%d: ",mvPortCtrlCurrentTs());\
    osPrintf x*/

/* AP ARB State Machine Numbering Definition */
/*typedef enum
{
    ST_AN_ENABLE    = 0x000,
    ST_TX_DISABLE   = 0x001,
    ST_ABILITY_DET  = 0x002,
    ST_ACK_DETECT   = 0x003,
    ST_COMPLETE_ACK = 0x004,
    ST_NP_WAIT      = 0x005,
    ST_AN_GOOD_CK   = 0x006,
    ST_AN_GOOD      = 0x007,
    ST_LINK_STAT_CK = 0x008,
    ST_PARALLEL_FLT = 0x009

}MV_HWS_AN_SM_STATE;*/


/* AN: The speeds advertise bits according to IEEE*/
#define AN_ADV_1G_KX_SHIFT    (0)
#define AN_ADV_10G_KX4_SHIFT  (1)
#define AN_ADV_10G_KR_SHIFT   (2)
#define AN_ADV_40G_KR4_SHIFT  (3)
#define AN_ADV_40G_CR4_SHIFT  (4)
#define AN_ADV_100G_KR4_SHIFT (7)
#define AN_ADV_100G_CR4_SHIFT (8)
#define AN_ADV_25G_S_SHIFT    (9)
#define AN_ADV_25G_SHIFT      (10)
#define AN_ADV_50G_KR1_SHIFT  (13)
#define AN_ADV_100G_KR2_SHIFT (14)
#define AN_ADV_200G_KR4_SHIFT (15)
#define AN_ADV_200G_KR8_SHIFT (16)
#define AN_ADV_400G_KR8_SHIFT (17)

/*******************************************************************************
*                         AP API Definition                                    *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvHwsApDetectStateSet function
* @endinternal
*
* @brief   AP Detection process enable / disable set
* @param[in] state             - AN state machine
*
* */
void mvHwsAnDetectStateSet
(
    IN  GT_U32      state
);
#if 0
/**
* @internal mvHwsAnPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable
*          after training Host or Service CPU (default value
*         is service CPU)
*
* @param[in] portEnableCtrl        - true: Enable control by
*                                    Service CPU
*                                    false: disable control by
*                                    Service CPU, only from
*                                    host
* */
void mvHwsAnPortEnableCtrlSet
(
    IN  GT_BOOL      portEnableCtrl
);

/**
* @internal mvHwsAnPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable
*          after training Host or Service CPU (default value is
*          service CPU
*
* @param[out] portEnableCtrl        - true: Enable control by
*                                    Service CPU
*                                    false: disable control by
*                                    Service CPU, only from
*                                    host
**/
void mvHwsAnPortEnableCtrlGet
(
    IN  GT_BOOL      *portEnableCtrl
);
#endif
/**
* @internal mvHwsAnResolutionBitSet function
* @endinternal
*
* @brief   Set AN Resolution bit
*
* @param[out] anResBit         - resolution bit shift
**/
void mvHwsAnResolutionBitSet
(
    IN  MV_HWS_PORT_STANDARD     portMode,
    IN  GT_U32                   anResBit
);


/* AP Detection External State machine States API's */
/* ================================================ */
/**
* @internal mvPortCtrlAnDetectActive function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Scan all ports and call port execution function
*/
void mvPortCtrlAnDetectActive(GT_U8 anTaskNum);

/**
* @internal mvPortCtrlAnPortDetectionActiveExec function
* @endinternal
*
* @brief   AN Detect Active state execution
*         - Exract port state
*         - Execute state functionality
*
* @param[in] portIndex             - number of physical port
*/
void mvPortCtrlAnPortDetectionActiveExec
(
    IN  GT_U8      portIndex
);

/**
* @internal mvHwsAnDetectRoutine function
* @endinternal
*
* @brief execute the Routine (MSG -> ACTIVE -> IDLE->)
*
*/
void mvHwsAnDetectRoutine
(
    IN  void*      pvParameters
);

/**
* @internal mvPortCtrlAnDetectMsg function
* @endinternal
*
* @brief   AN Detect message state execution
*         - Read message from AN detection message queue
*         Message received
*         Execute message to completion
*         Message not received
*         Trigger state change
*/
void mvPortCtrlAnDetectMsg(GT_U8 anTaskNum);

/**
* @internal mvPortCtrlAnDetectActive function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Scan all ports and call port execution function
*/
void mvPortCtrlAnDetectActive(GT_U8 anTaskNum);

/**
* @internal mvPortCtrlAnDetectDelay function
* @endinternal
*
* @brief   AP Delay state execution
*
*/
void mvPortCtrlAnDetectDelay(GT_U8 anTaskNum);


/* AN Detection Port State machine States API's */
/* ============================================ */

/**
* @internal mvAnPortAutoNeg function
* @endinternal
*
* @brief  execute Auto Negotiation. Configure the DME page and all the relevant avago SM registers
*         start Auto Negotiation, pollink o_core_status[20] for HCD result
*         extract all the relevant information after resulotion
*
*         linkThreshold = the time from HCD to full link
*         training threshold = the time for TRX training
*
*
* @param[in] portIndex             - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortAutoNeg
(
    IN  GT_U8      portIndex
);


/**
* @internal mvAnPortSerdesConfig function
* @endinternal
*
* @brief   execute training on AN port.
*         Execute port Init
*         Execute SERDES Configuration Execute MAC/PCS
*         Configuration Execute training Training configuration
*         Training start Training timeout timer
*
*         linkThreshold = the time from HCD to full link
*         training threshold = the time for TRX training
*
*
* @param[in] portIndex             - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortSerdesConfig
(
    IN  GT_U8      portIndex
);


/**
* @internal mvAnPortDeleteMsg function
* @endinternal
*
* @brief   AN Port Delete Msg execution sequence
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  -on failure
* */
GT_STATUS mvAnPortDeleteMsg
(
    IN  GT_U8      portIndex
);

/**
* @internal mvAnPortLinkUp function
* @endinternal
*
* @brief   Link up check. check for port AN complete by polling
*          o_core_status[19] and after that validate port link
*          up staus.
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortLinkUp
(
    IN  GT_U8      portIndex
);


/**
* @internal mvAnPortDeleteValidate function
* @endinternal
*
* @brief   AN delete port execution sequence
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortDeleteValidate
(
    IN  GT_U8      portIndex
);

/**
* @internal mvAnResetStats function
* @endinternal

* @brief   reset statistic
*
* @param[in] portIndex                 - number of physical port
*
* */
void mvAnResetStats
(
    IN  GT_U8      portIndex
);

/**
* @internal mvAnPortRemoteFaultEnable function
* @endinternal

* @brief  enable/disable remote fault
*
* @param[in] portIndex                 - number of physical port
* @param[in] enableRmf                 - enable/disable
*
* */
GT_STATUS  mvAnPortRemoteFaultEnable(
    IN  GT_U32   portIndex,
    IN  GT_BOOL  enableRmf
);


#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlAn_H */



