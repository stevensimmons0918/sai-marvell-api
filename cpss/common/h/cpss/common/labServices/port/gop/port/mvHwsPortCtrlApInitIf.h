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
* @file mvHwsPortCtrlApInitIf.h
*
* @brief API to configure and run 802.3ap Serdes AutoNeg engine
*
* @version   17
********************************************************************************
*/

#ifndef __mvHwServicesPortCtrlApIf_H
#define __mvHwServicesPortCtrlApIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
/**
* @internal mvHwsApPortCtrlStart function
* @endinternal
*
* @brief   Init AP port capability.
*         Runs AP protocol(802.3ap Serdes AutoNeg) and configures the best port
*         mode and all it's elements accordingly.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStart
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
);

/**
* @internal mvHwsApPortCtrlStop function
* @endinternal
*
* @brief   Disable the AP engine on port and release all its resources.
*         Clears the port mode and release all its resources according to selected.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
);

/**
* @internal mvHwsApPortCtrlSysAck function
* @endinternal
*
* @brief   Acknowledge port resources were allocated at application level
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlSysAck
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
);

/**
* @internal mvHwsApPortEnableWaitAck function
* @endinternal
*
* @brief   Send message to notify AP state machine that port was
*         disabled and ap machine can continue execution
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortEnableWaitAck
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum
);

/**
* @internal mvHwsApPortCtrlCfgGet function
* @endinternal
*
* @brief   Returns the AP port configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @param[out] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlCfgGet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
);

/**
* @internal mvHwsApPortCtrlStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
* @param[in] apStatus                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatusGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                apPortNum,
    MV_HWS_AP_PORT_STATUS *apStatus
);

/**
* @internal mvHwsApPortCtrlStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*
* @param[out] apStats                  - AP stats
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsGet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               apPortNum,
    MV_HWS_AP_PORT_STATS *apStats
);

/**
* @internal mvHwsApPortCtrlStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsReset
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               apPortNum
);

/**
* @internal mvHwsApPortCtrlIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlIntropSet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
);

/**
* @internal mvHwsApPortCtrlIntropGet function
* @endinternal
*
* @brief   Return AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlIntropGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
);

/**
* @internal mvHwsApPortCtrlDebugGet function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apDebug                  - AP debug parameters
*
* @param[out] apDebug                  - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlDebugGet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_FW_LOG *apDebug
);

/**
* @internal mvHwsFwHwsLogGet function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
*                                      fwHwsLog   - FW HWS log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFwHwsLogGet
(
    GT_U8           devNum,
    GT_U32          phyPortNum,
    MV_HWS_FW_LOG   *fwHwsLogPtr
);

/**
* @internal mvHwsApPortCtrlAvagoGuiSet function
* @endinternal
*
* @brief   Set AP state machine state when Avago GUI is enabled
*         Avago GUI access Avago Firmware as SBUS command level
*         Therefore it is required to stop the periodic behiviour of AP state
*         machine when Avago GUI is enabled
* @param[in] devNum                   - system device number
* @param[in] cpuId                    - the cpu id
* @param[in] state                    - Avago GUI state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlAvagoGuiSet
(
    GT_U8 devNum,
    GT_U8 cpuId,
    GT_U8 state
);

/**
* @internal mvApPortCtrlStatsShow function
* @endinternal
*
* @brief   Print AP port statistics information stored in system
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlStatsShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
);

/**
* @internal mvApPortCtrlIntropShow function
* @endinternal
*
* @brief   Print AP port introp information stored in system
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlIntropShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
);

/**
* @internal mvHwsApPortCtrlDebugParamsSet function
* @endinternal
*
* @brief   Set AP debug information
*
* @param[in] cpuId                    - Cpu id number
* @param[in] apFwLogBaseAddr          - AP Firmware Real-time log base address
* @param[in] apFwLogCountAddr         - AP Firmware Real-time log count address
* @param[in] apFwLogPointerAddr       - AP Firmware Real-time log pointer address
* @param[in] apFwLogResetAddr         - AP Firmware Real-time log reset address
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsApPortCtrlDebugParamsSet
(
    GT_U32 cpuId,
    GT_U32 apFwLogBaseAddr,
    GT_U32 apFwLogCountAddr,
    GT_U32 apFwLogPointerAddr,
    GT_U32 apFwLogResetAddr
);

/**
* @internal mvHwsLogParamsSet function
* @endinternal
*
* @brief   Set Hws log information
*
* @param[in] cpuId                    - Cpu id number
* @param[in] fwLogBaseAddr            - Firmware HWS Real-time log base address
* @param[in] fwLogCountAddr           - Firmware HWS Real-time log count address
* @param[in] fwLogPointerAddr         - Firmware HWS Real-time log pointer address
* @param[in] fwLogResetAddr           - Firmware HWS Real-time log reset address
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsLogParamsSet
(
    GT_U32 cpuId,
    GT_U32 fwLogBaseAddr,
    GT_U32 fwLogCountAddr,
    GT_U32 fwLogPointerAddr,
    GT_U32 fwLogResetAddr
);

/**
* @internal mvApPortCtrlDebugInfoShow function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @param[in] devNum                 - system device number
* @param[in] apPortNum              - AP port number
* @param[in] apCpssLogEnable        - boolean value indicate if
*                                      log will be part of CPSS
*                                      log mechanism or terminal
*                                      print cpssLog   - boolean
*                                      value indicate if log
*                                      will be part of CPSS log
*                                      mechanism or terminal
*                                      print
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlDebugInfoShow
(
    GT_U8                           devNum,
    GT_U32                          apPortNum,
    MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT  output
);

/**
* @internal mvHwsLogInfoShow function
* @endinternal
*
* @brief   Print FW Hws real-time log information stored in system
*
* @param[in] devNum             - system device number
* @param[in] cpuId              - cpu Index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogInfoShow
(
    GT_U8  devNum,
    GT_U32 cpuId
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsApPortCtrlEnablePortCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable
*          after training - Host or Service CPU (default value
*          service CPU)
*
* @param[in] devNum                   - system device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlSet
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
);

/**
* @internal mvHwsApPortCtrlEnablePortCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable
*          after training - Host or Service CPU (default value
*          service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlGet
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
);
#endif

/**
* @internal mvHwsApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      serdesLane  - serdes number
* @param[in] offsets                  - serdes TX values offsets
* @param[in] serdesSpeed              port speed assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxParametersOffsetSet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
);


/**
* @internal mvHwsApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesRxParametersManualSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE  *rxCfgPtr
);


/**
* @internal mvHwsPortAdaptiveCtlePortEnableSet function
* @endinternal
*
* @brief   Set adaptive ctle port enable/ disable
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAdaptiveCtlePortEnableSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    GT_BOOL                         enable
);


/**
* @internal mvHwsPortIsLinkUpStatusSet function
* @endinternal
*
* @brief   Set non AP port link up status
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] isLinkUp                 - port in link up or link
*                                       down
** @param[in] trainLfArr              - train LF from training
*                                       or enh training result.
* @param[in] enhTrainDelayArr        - enhtrain Delay from
*                                      enh training result.
* @param[in] serdesList              - port's serdeses arrray
* @param[in] numOfSerdes             - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortIsLinkUpStatusSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    GT_BOOL                 isLinkUp,
    GT_U16                  *trainLfArr,
    GT_U16                  *enhTrainDelayArr,
    GT_U16                  *serdesList,
    GT_U8                   numOfSerdeses
);

/**
* @internal mvHwsApSerdesTxManualConfigSet function
* @endinternal
*
* @brief   Set serdes TX parameters . Those offsets
*         will take place before TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesSpeed              - serdes speed
* @param[in] serdesNumber             - serdes number
* @param[in] serdesType               - serdes type
* @param[in] txConfigPtr            - serdes TX values per speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxManualConfigSet
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT *txConfigPtr
);

/**
* @internal mvHwsApSerdesRxManualConfigSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place before TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesSpeed              - serdes speed
* @param[in] serdesNumber             - serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxConfigPtr            - serdes RX values per speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesRxManualConfigSet
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT *rxConfigPtr
);

/**
* @internal mvHwsApSerdesTxPresetSet function
* @endinternal
*
* @brief  Set preset in serdes. Will take place before TRX
*         training.
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] laneNumber             - lane number
* @param[in] portMode               - port mode
* @param[in] devNum                   - system device number
* @param[in] pre                      - preset pre value
* @param[in] atten                    - preset atten value
* @param[in] post                     - preset post value
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxPresetSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            portGroup,
    IN  GT_U32                            phyPortNum,
    IN  GT_U32                            laneNumber,
    IN  MV_HWS_PORT_STANDARD              portMode,
    IN  GT_U8                             pre,
    IN  GT_U8                             atten,
    IN  GT_U8                             post
);

/**
* @internal mvHwsApSerdesGlobalRxTerminationModeIpc function
* @endinternal
*
* @brief   Set serdes global rx termination mode.
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] globalRxTermination    - global rx termination mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesGlobalRxTerminationModeIpc
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                globalRxTermination
);

/**
* @internal mvHwsApSetGlobalHostModeIpc function
* @endinternal
*
* @brief   Indicate to AP that it is controled by HOST CPU and
*          not MI (relevant for Falcon Only).
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] globalHostMode      - global Control Mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSetGlobalHostModeIpc
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                globalHostMode
);

/**
* @internal mvHwsApSerdesInterconnectProfileConfigSet function
* @endinternal
*
* @brief   Set interconnect profile
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] numOfActLanes          - num of active lanes
* @param[in] interconnectProfile    - interconnect profile
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesInterconnectProfileConfigSet
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                phyPortNum,
    GT_U8                                 numOfActLanes,
    GT_U32                                interconnectProfile
);

/**
* @internal mvHwsApResolutionBitSet function
* @endinternal
*
* @brief   Set AP AN resolution bit for 40G_KR2
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] anResBit               - resolution bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApResolutionBitSet
(
    GT_U8                             devNum,
    MV_HWS_PORT_STANDARD              portMode,
    GT_U32                            anResBit
);

/**
* @internal mvHwsCm3SemOper function
* @endinternal
*
* @brief   Set AP AN resolution bit for 40G_KR2
*
* @param[in] devNum                 - system device number
* @param[in] portMacNum             - physical port number
* @param[in] takeFree               True- Take/False-Free
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCm3SemOper
(
    GT_U8                       devNum,
    GT_U32                      portMacNum,
    GT_BOOL                     takeFree /*true - take,false - free*/
);

#ifdef __cplusplus
}
#endif

#endif /* mvHwServicesPortCtrlApIf_H */



