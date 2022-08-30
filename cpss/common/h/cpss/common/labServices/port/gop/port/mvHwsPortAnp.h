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
* @file mvHwsPortAnp.h
*
* @brief Anp unit functions.
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortAnp_H
#define __mvHwsPortAnp_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/*global variables include and macros*/

#define PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.labServicesDir.mvHwsAnpSrc._var,_value)

#define PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsAnpSrc._var)

#define MV_HWS_HAWK_ANP_NUM_CNS             14 /* 8(mac400) + 6(usx)*/
#define MV_HWS_HAWK_CLUSTER_NUM_CNS         4
#define MV_HWS_HARRIER_CLUSTER_NUM_CNS      3
#define MV_HWS_ANP_CMD_TABLE_SIZE_CNS       64
#define MV_HWS_ANP_CMD_TABLE_EMPTY_CNS      0xFFFFFFFF
/**
* @struct MV_HWS_HAWK_ANP_STC
*
* @brief Defines structure for anp parameters.
*/
typedef struct
{
    GT_U32      hwSerdesIdx; /* the serdes that connected to the anp can be changed by mux at init time */
    GT_U32      outSerdesIdx; /* the serdes that connected now to the anp changed by port enable/disable */
}MV_HWS_ANP_SD_MUX_STC;

/**
* @struct MV_HWS_HAWK_ANP_MISC_PARAM_STC
*
* @brief Defines structure for anp parameters.
*/
typedef struct
{
    GT_U8      interopLinkTimer; /*  */
}MV_HWS_ANP_MISC_PARAM_STC;

/**
* @struct MV_HWS_HAWK_ANP_STC
*
* @brief Defines structure for anp parameters.
*/
typedef struct
{
    MV_HWS_ANP_SD_MUX_STC anpSdMuxDb[MV_HWS_HAWK_CLUSTER_NUM_CNS][MV_HWS_HAWK_ANP_NUM_CNS];
    MV_HWS_AP_CFG *anpPortParamPtr[HWS_MAX_DEVICE_NUM];
    GT_BOOL       *anpSkipResetPtr[HWS_MAX_DEVICE_NUM];
    GT_U32        *anpCmdIntLineIdxPtr[HWS_MAX_DEVICE_NUM];
    MV_HWS_ANP_MISC_PARAM_STC *anpPortMiscParamDbPtr[HWS_MAX_DEVICE_NUM];
}MV_HWS_ANP_PORT_DATA_STC;

/*
 * ANP counters information:
 *    anRestartCounter  - Counts the number of times AN had restart, after got to resolution.
 *                        That is, link_fail_inhibit timer expired without link, or AN completed, and then link failed.
 *    dspLockFailCounter- Counts the number of times Channel SM waits for dsp_lock and gets timeout.
 *    linkFailCounter   - Counts the number of times Channel SM waits for link and gets timeout.
 *    txTrainDuration   - Holds the duration (clock cycles) of the last tx_train.
 *    txTrainFailCounter- Counts the number of times tx_train performed and completed with fail status.
 *    txTrainTimeoutCounter - Counts the number of times tx_train performed and got timeout.
 *    txTrainOkCounter  - Counts the number of times tx_train performed and completed without fail.
 *    rxInitOk  - Counts the number of times rx_init performed and finished with rx_init_done.
 *    rxInitTimeOut  - Counts the number of times rx_init performed and finished with timeout.
 *    rxTrainFailed  - Counts the number of times rx_train performed and completed with fail status.
 *    rxTrainOk  - Counts the number of times rx_train performed and completed without fail.
 *    rxTrainTimeOut  - Counts the number of times rx_train performed and got timeout.
*/
typedef struct
{
    GT_U32 anRestartCounter;
    GT_U32 dspLockFailCounter;
    GT_U32 linkFailCounter;
    GT_U32 txTrainDuration;
    GT_U32 txTrainFailCounter;
    GT_U32 txTrainTimeoutCounter;
    GT_U32 txTrainOkCounter;
    GT_U32 rxInitOk;
    GT_U32 rxInitTimeOut;
    GT_U32 rxTrainFailed;
    GT_U32 rxTrainOk;
    GT_U32 rxTrainTimeOut;
    GT_U32 rxTrainDuration;

}MV_HWS_ANP_PORT_COUNTERS;

/*
 * ANP SM information:
 *    chSmState  - Channel state machine.
 *    txSmState  - Tx state machine.
 *    rxSmState  - Rx state machine.
*/
typedef struct
{
    GT_U32 chSmState;
    GT_U32 txSmState;
    GT_U32 rxSmState;

}MV_HWS_ANP_PORT_SM;

/*
 * ANP info struct:
 *    portConters - Anp counters.
 *    portSm      - Anp state machine.
*/
typedef struct
{
    MV_HWS_ANP_PORT_COUNTERS portConters;
    MV_HWS_ANP_PORT_SM       portSm;
}MV_HWS_ANP_INFO_STC;

/**
* @internal mvHwsAnpClose function
* @endinternal
*
* @brief  ANP unit close
*
* @param[in] devNum                - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpClose
(
    IN GT_U8              devNum
);

/**
* @internal mvHwsAnpInit function
* @endinternal
*
* @brief  ANP unit init
*
* @param[in] devNum                - system device number
* @param[in] skipWritingToHW       - skip writing to HW
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpInit
(
    IN GT_U8   devNum,
    IN GT_BOOL skipWritingToHW
);

/**
* @internal mvHwsAnpSerdesSdwMuxSet function
* @endinternal
*
* @brief  ANP unit enable SerDes control
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - system SerDes number
* @param[in] anpEnable - true  - SerDes is controlled by ANP
*                      - false - SerDes is controlled by regs
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpSerdesSdwMuxSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               serdesNum,
    IN  GT_BOOL              anpEnable
);

/**
* @internal mvHwsAnpPortEnable function
* @endinternal
*
* @brief  ANP unit port enable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - system port number
* @param[in] portMode              - configured port mode
* @param[in] apEnable              - apEnable flag
* @param[in] enable                - True = enable/False =
*       disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              apEnable,
    GT_BOOL              enable
);

/**
* @internal mvHwsAnpPortReset function
* @endinternal
*
* @brief   reset anp machine
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] reset              - reset parameter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortReset
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_BOOL              reset
);

/**
* @internal mvHwsAnpPortStart function
* @endinternal
*
* @brief   Port anp start (set capabilities and start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortStart
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_AP_CFG       *apCfgPtr
);

/**
* @internal mvHwsAnpCfgPcsDone function
* @endinternal
*
* @brief   config psc done after pcs configuration.
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - port mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpCfgPcsDone
(
    IN GT_U8                    devNum,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode
);

/**
* @internal mvHwsAnpPortStopAn function
* @endinternal
*
* @brief   Port anp stop autoneg
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortStopAn
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsAnpPortInteropGet function
* @endinternal
*
* @brief   Return ANP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apInteropPtr             - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInteropGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_INTROP  *apInteropPtr
);

/**
* @internal mvHwsAnpPortLinkTimerSet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortLinkTimerSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  timeInMsec
);

/**
* @internal mvHwsAnpPortLinkTimerGet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortLinkTimerGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *timeInMsecPtr
);

/**
* @internal mvHwsAnpPortStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatusPtr                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatusGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_STATUS  *apStatusPtr
);

/**
* @internal mvHwsAnpPortResolutionBitSet function
* @endinternal
*
* @brief   set resolution bit per port mode
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] anResBit                 - resolution bit shift
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortResolutionBitSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  anResBit
);

/**
* @internal mvHwsPortAnpConfigGet function
* @endinternal
*
* @brief   Returns the ANP port configuration.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @param[out] apCfgPtr             - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAnpConfigGet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_AP_CFG       *apCfgPtr
);

/**
* @internal mvHwsAnpPortStatsGet function
* @endinternal
*
* @brief   Returns the ANP port statistics information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] apStats              - AP statistics parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatsGet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_AP_PORT_STATS *apStats
);

/**
* @internal mvHwsAnpPortStatsReset function
* @endinternal
*
* @brief   Reset ANP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatsReset
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsAnpPortInteropSet function
* @endinternal
*
* @brief   Set ANP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apInteropPtr             - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInteropSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_INTROP  *apInteropPtr
);

/**
* @internal mvHwsAnpPortInfoGet function
* @endinternal
*
* @brief   Get ANP port information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] anpInfoPtr           - anp info
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInfoGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    OUT MV_HWS_ANP_INFO_STC      *anpInfoPtr
);

/**
* @internal mvHwsAnpCmdTableClear function
* @endinternal
*
* @brief   clear anp cmd table
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdTableClear
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsAnpPortSkipResetSet function
* @endinternal
*
* @brief   set skip reset value (prevent from delete port to
*          reset ANP)
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortSkipResetSet (
    GT_U8                       devNum,
    GT_U32                      portNum,
    GT_BOOL                     skip
);

/**
* @internal mvHwsAnpPortParallelDetectInit function
* @endinternal
*
* @brief   Init sequence for parallel detect
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectInit
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
);

/**
* @internal mvHwsAnpPortParallelDetectReset function
* @endinternal
*
* @brief   Reset sequence for parallel detect
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectReset
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
);

/**
* @internal mvHwsAnpParallelDetectWaitForPWRUPprog function
* @endinternal
*
* @brief   Wait for CH FSM to reach PWRUP prog state
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectWaitForPWRUPprog
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum
);

/**
* @internal mvHwsAnpPortParallelDetectAutoNegSet function
* @endinternal
*
* @brief   Set AN37
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] autoNegEnabled        - enable/disable AN37
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectAutoNegSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  autoNegEnabled
);

/**
* @internal mvHwsAnpPortParallelDetectOWLinkStatus function
* @endinternal
*
* @brief enable to override link_status from PCS value
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] owLinkStatus          - true/false if override is
*       enabled
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectOWLinkStatus
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  owLinkStatus
);

/**
* @internal mvHwsAnpPortParallelDetectAN37Seq function
* @endinternal
*
* @brief   Check peer side AN37 status seq.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] startSeq              - start or end seq
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectAN37Seq
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  startSeq
);

/**
* @internal mvHwsAnpPortRxConfig function
* @endinternal
*
* @brief   "fix" squelch value
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] portMode              - configured port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortRxConfig
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode
);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortAnp_H */
