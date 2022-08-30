/********************************************************************************
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
* @file prvCpssPortManagerTypes.h
*
* @brief CPSS implementation for Port management types.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPortManagerTypes
#define __prvCpssPortManagerTypes

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/port/cpssPortManager.h>

/**
 * @enum CPSS_PM_MASKING_BITMAP_ENT
 *
 * @brief bit-to-param mapping for
 *        PRV_CPSS_PORT_MNG_PORT_SM_DB_STC.maskingBitMap
 */
typedef enum {
    /*low level masking enums*/
    CPSS_PM_PCS_ALIGN_LOCK_LOST_MASKING_E          = 0x1,
    CPSS_PM_PCS_GB_LOCK_SYNC_CHANGE_MASKING_E      = 0x2,
    CPSS_PM_MMPCS_SIGNAL_DETECT_CHANGE_MASKING_E   = 0x4,
    /*mac level masking enums*/
    CPSS_PM_PORT_LINK_STATUS_CHANGED_E             = 0x8

} CPSS_PM_MASKING_BITMAP_ENT;

/**
* @struct PRV_CPSS_PORT_MNG_SERDES_TXRX_PARAMS_STC
 *
 * @brief This structure contains serdes rxtx parameters
*/
typedef struct{

    /** rx and tx configuration structure */
    CPSS_PORT_SERDES_TUNE_STC serdesCfgDb;

    GT_BOOL rxValid;

    /** @brief whether or not rx parameters in the configuration are valid
     *  txValid - whether or not rx parameters in the configuration are valid
     *  Comments:
     *  None.
     */
    GT_BOOL txValid;

} PRV_CPSS_PORT_MNG_SERDES_TXRX_PARAMS_STC;

/**
* @struct PRV_CPSS_PORT_MNG_SERDES_POLARITY_PARAMS_STC
 *
 * @brief This structure contains serdes polarity configurations
*/
typedef struct{

    /** rx polarity */
    GT_BOOL rxInvert;

    /** @brief tx polarity
     *  Comments:
     *  None.
     */
    GT_BOOL txInvert;

} PRV_CPSS_PORT_MNG_SERDES_POLARITY_PARAMS_STC;

typedef enum {

  CPSS_PM_DB_LANE_OVERRIDE_TX_E           =0x1,
  CPSS_PM_DB_LANE_OVERRIDE_RX_E           =0x2

} CPSS_PM_DB_LANE_OVERRIDE_PARAM_TYPE_ENT;

/**
* @struct PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC
 *
 * @brief This structure contains configurable fields for a serdes
*/
typedef struct{

    GT_U32                                    sdParamValidBitMap;
    /** serdes rx and tx parameters */
    PRV_CPSS_PORT_MNG_SERDES_TXRX_PARAMS_STC portSerdesParams;

} PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC
 *
 * @brief This structure contains mandatory data a port need to have
 * in order for port manage to manage the port.
*/
typedef struct{

    /** interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT ifModeDb;

    /** @brief speed
     *  Comments:
     *  None.
     */
    CPSS_PORT_SPEED_ENT speedDb;

} PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_AP_ATTRIBUTES_STC
 *
 * @brief This structure contains all the attributes an AP port can be configured with.
*/
typedef struct{

    /** FC pause (true/false) */
    GT_BOOL fcPause;

    /** FC assymetric direction (Annex 28B) */
    CPSS_PORT_AP_FLOW_CONTROL_ENT fcAsmDir;

    /** @brief GT_TRUE
     *  GT_FALSE - choose one of sides to be ceed according to protocol
     */
    GT_BOOL noneceDisable;

    /** @brief number of serdes lane of port where to run AP
     *  modesAdvertiseArr - array of pairs of (interface mode;speed) advertised by
     *  port for AP negotiation
     *  last entry must be
     *  (CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E)
     */
    GT_U32 laneNum;

    CPSS_PORT_FEC_MODE_ENT fecAbilityArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_FEC_MODE_ENT fecRequestedArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_MANAGER_LKB_CONFIG_STC    linkBinding;

    /*
      0x0 - default interconnect profile
      0x1 - interconnect profile1
      0x2 - interconnect profile2
    */
    CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT   interconnectProfile;

    GT_U32  portOperationsBitmap;
    GT_BOOL skipRes;
    CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC parallelDetect;
    /*
       CPSS_PM_AP_PORT_ATTR_NONCE_E,
       CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E,
       CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E,
       CPSS_PM_AP_PORT_ATTR_LANE_NUM_E,
       CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E,
       CPSS_PM_AP_PORT_ATTR_LKB_E,
       CPSS_PM_AP_PORT_ATTR_OPERATIONS_E
       CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E
       CPSS_PM_AP_PORT_ATTR_SKIP_RES_E
       CPSS_PM_AP_PORT_ATTR_PARALLEL_DETECT_E
    */

    GT_U32  overrideAttrsBitMask;

} PRV_CPSS_PORT_MNG_AP_ATTRIBUTES_STC;

/**
* @struct PRV_CPSS_PORT_MNG_AP_PORT_RESOLUTION_STC
 *
 * @brief This structure contains attributes of PM AP after
 *        resolution found.
*/
typedef struct{

    /** interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT ifModeDb;

    /** speed */
    CPSS_PORT_SPEED_ENT speedDb;

    /** fecMode */
    CPSS_PORT_FEC_MODE_ENT fecStatus;

} PRV_CPSS_PORT_MNG_AP_PORT_RESOLUTION_STC;

/**
* @struct PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC
 *
 * @brief This structure contains general port configuration parameters
*/
typedef struct{

    /** @brief loopback configuration for the port
     *  Comments:
     *  None.
     */
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackDbCfg;

    /**------- non ap parameters -------*/
    GT_U32                                  portMacNum;
    GT_U32                                  portGroup;
    MV_HWS_PORT_STANDARD                    portMode;
    GT_BOOL                                 ignoreTraining;

    /**------- debug parameters -------*/
    GT_U16                                  lastUpdate;

} PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC
 *
 * @brief This structure contains all the attributes a port can be configured with.
*/
typedef struct{

    GT_U32 perPhyPortApNumOfModes;
    PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC perPhyPortDb[CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];
    /* 0-7 for mode=0, 1-8 for mode 1... index = lane + mode*PORT_MANAGER_MAX_LANES*/
    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC   perSerdesDb[CPSS_PORT_AP_IF_ARRAY_SIZE_CNS*PORT_MANAGER_MAX_LANES];

    /** @brief AP attributes of the port
     *  Comments:
     *  None.
     */
    PRV_CPSS_PORT_MNG_AP_ATTRIBUTES_STC apAttributesStc;

    PRV_CPSS_PORT_MNG_AP_PORT_RESOLUTION_STC portResolution;

    PRV_CPSS_PORT_MNG_AP_PORT_RESOLUTION_STC portLastResolution;

} PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_PORT_ATTRIBUTES_DB_STC
 *
 * @brief This structure contains all the attributes a port can be configured with.
*/
typedef struct{

    PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC **perPhyPortDbPtr;

    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC **perSerdesDbPtr;

    PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC **generalPortCfgPtr;

} PRV_CPSS_PORT_MNG_PORT_ATTRIBUTES_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_MAC_SM_DB_STC
 *
 * @brief This structure contains configurable fields for a serdes
*/
typedef struct{

    GT_U32 xlgMacDb[3];

    GT_U32 xlgMacDbFlag;

} PRV_CPSS_PORT_MNG_MAC_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_OPTICAL_CALIBRATION_DB_STC
 *
 * @brief This structure contains optical calibration port
 *        manager data base
*/
typedef struct
{
    CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT calibrationType;
    GT_U32 minLfThreshold;
    GT_U32 maxLfThreshold;
    GT_U32 minHfThreshold;
    GT_U32 maxHfThreshold;
    GT_U32 minEoThreshold;
    GT_U32 maxEoThreshold;
    GT_U32 confidenceBitMap;
    GT_BOOL opticalCalFinished[PORT_MANAGER_MAX_LANES];
    GT_BOOL overallOpticalCalFinished;
    GT_U32 opticalCalTimeoutSec;
    GT_U32 opticalCalTimerSecPrev;
    GT_U32 opticalCalTimerNsecNew;

} PRV_CPSS_PORT_MNG_CALIBRATION_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_PORT_SM_DB_STC
 *
 * @brief This structure contains all the data that port management use to manage to port state machine
*/
typedef struct{
    CPSS_PORT_MANAGER_PORT_TYPE_ENT portType;
    GT_BOOL                     pmOverFw;

    /* flag to store signal detected\lost indication
    GT_TRUE - a signal was deteced
    GT_FALSE - the singal is lost or in reset
    */
    GT_BOOL                     signalDetected;

    /* train status per port */
    PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC trainCookiePtrEnclosingDB;

    /* signal stability algorithm timers and dbs */
    GT_U32                      portTimeDiffFromLastTime;
    GT_U32                      portTimeIntervalTimer;
    GT_U32                      portTimerSecondsPrev;
    GT_U32                      portTimerNsecPrev;
    GT_U32                      portTotalTimeFromStart;
    GT_U32                      portTotalTimeFromChange;

    /* train results timeout timers and dbs */
    GT_U32                      portTrainResTimeDiffFromLastTime;
    GT_U32                      portTrainResTimerSecondsPrev;
    GT_U32                      portTrainResTimerNsecPrev;
    GT_U32                      portTrainResTimeIntervalTimer;

    /* align-lock timers and dbs */
    GT_U32                      portAlignTimeDiffFromLastTime;
    GT_U32                      portAlignTimerSecondsPrev;
    GT_U32                      portAlignTimerNsecPrev;
    GT_U32                      portAlignTotalTimeFromStart;
    GT_U32                      portAlignTimeIntervalTimer;
    GT_U32                      portAlignTries;

    /* flags per port to store signal stability and training statuses iteration process indicators */
    CPSS_PORT_MANAGER_PROGRESS_TYPE_ENT    portDbStabilityDone;
    CPSS_PORT_MANAGER_PROGRESS_TYPE_ENT    portDbTrainDone;
    CPSS_PORT_MANAGER_PROGRESS_TYPE_ENT    portDbAlignLockDone;

    /* signal stabilization training timers and db */
    CPSS_PORT_MANAGER_PROGRESS_TYPE_ENT    rxStabilityTrainDone;
    GT_U32                      rxStabilityTrainTimeDiffFromLastTime;
    GT_U32                      rxStabilityTrainTimerSecondsPrev;
    GT_U32                      rxStabilityTrainTimerNsecPrev;
    GT_U32                      rxStabilityTrainTotalTimeFromStart;
    GT_U32                      rxStabilityTrainTries;

    GT_U32                      userOverrideByteBitmap;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT userOverrideTrainMode;
    GT_BOOL                     userOverrideAdaptiveRxTrainSupported;
    GT_BOOL                     userOverrideEdgeDetectSupported;
    CPSS_PORT_FEC_MODE_ENT      fecAbility;

    /* training results iteration timer per port */
    GT_U64                      portWaitForTrainResultCounter;
    GT_U32                      portWaitForTrainResultTries;

    /* whether port is under management */
    GT_BOOL                     portManaged;

    /* port state machine */
    CPSS_PORT_MANAGER_STATE_ENT portAdminSM;
    CPSS_PORT_MANAGER_FAILURE_ENT portLastFailureReason;

    /* operative enable\disable */
    GT_BOOL                     portOperDisableSM;
    /* override MAX_LF and MIN_LF */
    GT_U8                       max_LF;
    GT_U8                       min_LF;
    GT_BOOL                     overrideEtParams;
    /* whether port manager is responsible of unMasking events or not */
    CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskEventsMode;
    /* calibration parameters */
    PRV_CPSS_PORT_MNG_CALIBRATION_DB_STC calibrationMode;
    GT_BOOL                     disableMaskUnMask;
    GT_BOOL                     adaptiveStarted;
    CPSS_PORT_PA_BW_MODE_ENT    bwMode;
    GT_BOOL                     byPassRemoteFault;
    CPSS_PORT_MANAGER_LKB_CONFIG_STC    linkBinding;
    GT_BOOL                     oneShotiCal;
    /* un/mask bitmap info */
    GT_U32                          maskingBitMap;
    GT_U32                          portOperationsBitmap;
    GT_BOOL                         isRemotePort;
    GT_U32                          interconnectProfile;
    CPSS_PM_MAC_PREEMPTION_PARAMS_STC preemptionParams;

    /* autoNegotiation */
    CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC autoNegotiation;
    /* statistics*/
    CPSS_PORT_MANAGER_STATISTICS_STC    statistics;
    GT_U32                              timeStampSec;
    GT_U32                              timeStampNSec;

    GT_U8                               logCounter;
    /* portmanager state while entring debug mode*/
    CPSS_PORT_MANAGER_STATE_ENT         enterDebugState;

}PRV_CPSS_PORT_MNG_PORT_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_DB_STC
 *
 * @brief This structure contains all the data that port management use to manage it's ports
*/
typedef struct{

    PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC **portsApAttributesDb;

    /** @brief attributes of a ports. This structure contains customize fields
     *  that the user can configure on a port, for example: speed,
     *  interface mode, serdes polarity, etc.
     *  portMngSmDb     - this structure contains fields used to manage port state maching.
     *  portMngSmDbPerMac  - per mac database
     */
    PRV_CPSS_PORT_MNG_PORT_ATTRIBUTES_DB_STC portsAttributedDb;

    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC **portMngSmDb;

    PRV_CPSS_PORT_MNG_MAC_SM_DB_STC **portMngSmDbPerMac;

    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC globalParamsCfg;

    /** @brief whether or not port manager API was used at least once.
     *  Comments:
     *  None.
     */
    GT_BOOL portManagerGlobalUseFlag;

} PRV_CPSS_PORT_MNG_DB_STC;


/**
* @struct PRV_CPSS_PORT_PM_IS_FW_FUNC_PTR
 *
 * @brief cpss pm callback function  that check if port is
 *        running on FW.
*/
typedef GT_STATUS (*PRV_CPSS_PORT_PM_IS_FW_FUNC_PTR)
(
    GT_U8                           devNum,
    GT_U32                          portNum,
    GT_BOOL                        *portIsFwPtr
);

/**
* @struct PRV_CPSS_PORT_PM_FUNC_PTRS
 *
 * @brief cpss pm callback function - cpss can call pm functions.
*/
typedef struct
{
  PRV_CPSS_PORT_PM_IS_FW_FUNC_PTR       cpssPmIsFwFunc;

}PRV_CPSS_PORT_PM_FUNC_PTRS;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortManagerTypes */

