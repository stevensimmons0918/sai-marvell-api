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
* @file cpssPxPortAp.h
*
* @brief CPSS implementation for 802.3ap standard (defines the auto negotiation
* for backplane Ethernet) configuration and control facility for Px family
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortAph
#define __cpssPxPortAph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>

/* size of array of interfaces advertised by port during AP process */
#define CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS 10

/* AP real-time log options (show debug info of all ports)*/
#define AP_LOG_ALL_PORT_DUMP (0xFFFF)

/**
* @enum CPSS_PX_PORT_AP_FLOW_CONTROL_ENT
 *
 * @brief Enumerator of AP Port FC Direction enablers.
*/
typedef enum{

    /** flow control in both directions */
    CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E,

    /** in one direction */
    CPSS_PX_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E

} CPSS_PX_PORT_AP_FLOW_CONTROL_ENT;

/**
* @struct CPSS_PX_PORT_AP_PARAMS_STC
 *
 * @brief Structure for configuring AP special parameters and advertisment
 * options on port
*/
typedef struct{

    /** FC pause (true/false) */
    GT_BOOL fcPause;

    /** FC assymetric direction (Annex 28B) */
    CPSS_PX_PORT_AP_FLOW_CONTROL_ENT fcAsmDir;

    /** @brief FEC ability (true/false)
     *  (APPLICABLE DEVICES: Pipe)
     */
    GT_BOOL fecSupported;

    /** @brief Request link partner to enable FEC (true/false)
     *  (APPLICABLE DEVICES: Pipe)
     */
    GT_BOOL fecRequired;

    /** @brief GT_TRUE
     *  GT_FALSE - choose one of sides to be ceed according to protocol
     */
    GT_BOOL noneceDisable;

    /** number of serdes lane of port where to run AP */
    GT_U32 laneNum;

    CPSS_PORT_MODE_SPEED_STC modesAdvertiseArr [CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_FEC_MODE_ENT fecAbilityArr [CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_FEC_MODE_ENT fecRequestedArr [CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS];

} CPSS_PX_PORT_AP_PARAMS_STC;

/**
* @struct CPSS_PX_PORT_AP_STATUS_STC
 *
 * @brief Structure for AP resolution result
*/
typedef struct{

    /** AP resolved port number (lane swap result) */
    GT_U32 postApPortNum;

    /** port (interface mode;speed) */
    CPSS_PORT_MODE_SPEED_STC portMode;

    /** @brief indicating AP succeeded to find highest common denominator
     *  with partner
     */
    GT_BOOL hcdFound;

    /** FEC enabled */
    GT_BOOL fecEnabled;

    /** enable RX flow control pause frames */
    GT_BOOL fcRxPauseEn;

    /** enable TX flow control pause frames */
    GT_BOOL fcTxPauseEn;

    /** FEC type fc or RS */
    CPSS_PORT_FEC_MODE_ENT fecType;

} CPSS_PX_PORT_AP_STATUS_STC;


/*
 * AP statistics information:
 *    abilityCnt        - Number of Ability detect intervals executed
 *    abilitySuccessCnt - Number of Ability detect successfull intervals executed
 *    linkFailCnt       - Number of Link check fail intervals executed
 *    linkSuccessCnt    - Number of Link check successfull intervals executed
 *    hcdResoultionTime - Time duration for HCD resolution
 *    linkUpTime        - Time duration for Link up
*/
typedef struct
{
    GT_U16 txDisCnt;          /* Number of Tx Disable intervals executed */
    GT_U16 abilityCnt;        /* Number of Ability detect intervals executed */
    GT_U16 abilitySuccessCnt; /* Number of Ability detect successfull intervals executed */
    GT_U16 linkFailCnt;       /* Number of Link check fail intervals executed */
    GT_U16 linkSuccessCnt;    /* Number of Link check successfull intervals executed */
    GT_U32 hcdResoultionTime; /* Time duration for HCD resolution */
    GT_U32 linkUpTime;        /* Time duration for Link up */

}CPSS_PX_PORT_AP_STATS_STC;


/*
* AP introp information:
*    attrBitMask            - Bit mask for attribute configuration:
*                             0x1 - TX Disable,....,0x80 - PD_Max Interval
*    txDisDuration          - Tx Disable duration in msec - default 60msec
*    abilityDuration        - Ability detect duration in msec - default 5 msec
*    abilityMaxInterval     - Ability detect max intervals - default 25 intervals ==> 5msec x 25 intervals = 125msec
*    abilityFailMaxInterval - Ability detect max number of failed intervals where ST_AN_GOOD_CK was detected
*                             But not resolution was found, trigger move to INIT state, instead of TX Disable in normal case
*    apLinkDuration         - AP Link check duration in msec - default 10 msec
*    apLinkMaxInterval      - AP Link check max intervals - default 50 intervals ==> 10msec x 50 intervals = 500msec
*    pdLinkDuration         - PD Link check duration in msec - default 10 msec
*    pdLinkMaxInterval      - PD Link check max intervals - default 25 intervals ==> 10msec x 25 intervals = 250msec
*/
typedef struct
{
    GT_U16 attrBitMask;
    GT_U16 txDisDuration;
    GT_U16 abilityDuration;
    GT_U16 abilityMaxInterval;
    GT_U16 abilityFailMaxInterval;
    GT_U16 apLinkDuration;
    GT_U16 apLinkMaxInterval;
    GT_U16 pdLinkDuration;
    GT_U16 pdLinkMaxInterval;
}CPSS_PX_PORT_AP_INTROP_STC;

/**
* @internal cpssPxPortApEnableSet function
* @endinternal
*
* @brief   Enable/disable AP engine (loads AP code into shared memory and starts AP
*         engine).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -  AP on port group
*                                      GT_FALSE - disbale
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApEnableSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_BOOL             enable
);

/**
* @internal cpssPxPortApEnableGet function
* @endinternal
*
* @brief   Get AP engine enabled and functional on port group (local core) status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enabledPtr
);

/**
* @internal cpssPxPortApPortConfigSet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortConfigSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     apEnable,
    IN  CPSS_PX_PORT_AP_PARAMS_STC  *apParamsPtr
);

/**
* @internal cpssPxPortApPortConfigGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
* @param[out] apParamsPtr              - (ptr to) AP parameters of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortConfigGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *apEnablePtr,
    OUT CPSS_PX_PORT_AP_PARAMS_STC  *apParamsPtr
);

/**
* @internal cpssPxPortApPortEnableGet function
* @endinternal
*
* @brief   Get if AP is enabled on a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortEnableGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *apEnablePtr
);

/**
* @internal cpssPxPortApPortStatusGet function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortStatusGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_STATUS_STC  *apStatusPtr
);


/**
* @internal cpssPxPortApStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apStatsPtr               - (ptr to) AP statistics information
* @param[out] intropAbilityMaxIntervalPtr - (ptr to) Introp Ability Max Interval
*                                      parameter - will help to represent
*                                      number of failed HCD cycles
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApStatsGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_STATS_STC   *apStatsPtr,
    OUT GT_U16                      *intropAbilityMaxIntervalPtr
);

/**
* @internal cpssPxPortApStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApStatsReset
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssPxPortApIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
GT_STATUS cpssPxPortApIntropSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_AP_INTROP_STC  *apIntropPtr
);

/**
* @internal cpssPxPortApIntropGet function
* @endinternal
*
* @brief   Returns AP port introp information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*
* @param[out] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
GT_STATUS cpssPxPortApIntropGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_INTROP_STC  *apIntropPtr
);

/**
* @internal cpssPxPortApDebugInfoGet function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] output                   - enum indicate log output
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssPxPortApDebugInfoGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT              output
);


/**
* @internal cpssPxPortApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training
*         Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training,
*                                      if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssPxPortApPortEnableCtrlSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         srvCpuEnable
);

/**
* @internal cpssPxPortApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training
*         Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssPxPortApPortEnableCtrlGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *srvCpuEnablePtr
);

/**
* @internal prvCpssPxPortApPortConfigSetConvert function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now allowed negotiation on serdes lanes of port 0-3, because in Lion2
*       just these options are theoreticaly possible.
*
*/
GT_STATUS prvCpssPxPortApPortConfigSetConvert
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_BOOL                    apEnable,
    IN  CPSS_PORT_AP_PARAMS_STC    *apParamsPtr,
    IN  GT_U32                     portOperationBitmap,
    IN  GT_BOOL                    skipRes
);

/**
* @internal prvCpssPxPortApPortStatusGetConvert function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortApPortStatusGetConvert
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT CPSS_PORT_AP_STATUS_STC   *apStatusPtr
);

/**
* @internal cpssPxPortApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxOffsetsPtr       - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Offsets are limited to the range of minimum -7 and maximum +7
*
*/
GT_STATUS cpssPxPortApSerdesTxParametersOffsetSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *serdesTxOffsetsPtr
);

/**
* @internal cpssPxPortApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] rxOverrideParamsPtr      - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortApSerdesRxParametersManualSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_RX_CONFIG_STC *rxOverrideParamsPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortAph */

