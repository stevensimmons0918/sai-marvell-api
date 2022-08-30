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
* @file prvCpssPortManager.h
*
* @brief CPSS definitions for port manager.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPortManager_h
#define __prvCpssPortManager_h

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>

#include <cpss/common/port/cpssPortManager.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* AC3X applicable devices currently supported in port manager */
#define CPSS_PORT_MANAGER_AC3X_APPLICABLE_DEVICES_MAC(_devNum)                  \
(                                                                               \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)     \
        && ((PRV_CPSS_PP_MAC(_devNum)->devType == CPSS_98DX3255_CNS)  ||        \
            (PRV_CPSS_PP_MAC(_devNum)->devType == CPSS_98DX3258_CNS)  ||        \
            (PRV_CPSS_PP_MAC(_devNum)->devType == CPSS_98DX3265_CNS)  ||        \
            (PRV_CPSS_PP_MAC(_devNum)->devType == CPSS_98DX3268_CNS))           \
)

/*Aldrin, BC3, Aldrin2, PIPE */
/* not all devices are currently supported in port manager */
#define CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(_devNum)                     \
(                                                                               \
    PRV_CPSS_SIP_5_15_CHECK_MAC(_devNum) /* BOBK and above , Pipe */            \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)     \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)       \
    || CPSS_PORT_MANAGER_AC3X_APPLICABLE_DEVICES_MAC(_devNum)                   \
)

#if 0 /* Original Applicable devices list */
#define CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(_devNum)              \
(                                           \
    (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E       \
    && PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)     \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)      \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)     \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)      \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)         \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)     \
    || (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)     \
)
#endif


/**
* @enum PRV_CPSS_PORT_MANAGER_UNIT_BITMAP_ENT
 *
 * @brief Enumeration of the port low level units bits
*/
typedef enum{

    /** signal detect bit index */
    PRV_CPSS_PORT_MANAGER_SIG_DET_E,

    /** gear-box lock bit index */
    PRV_CPSS_PORT_MANAGER_GB_LOCK_E,

    /** align-lock lost bit index */
    PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E

} PRV_CPSS_PORT_MANAGER_UNIT_BITMAP_ENT;

/**
* @enum PRV_CPSS_PORT_MANAGER_SERDES_PARAMS_TYPE_ENT
 *
 * @brief Enumeration of the serdes Tx/Rx parameters combinations.
*/
typedef enum{

    /** serdes tx parameters */
    PRV_CPSS_PORT_MANAGER_SERDES_TX_PARAMS_E,

    /** @brief serdes rx parameters
     *  PRV_CPSS_PORT_MANAGER_SERDES_TXRX_PARAMS_E  serdes tx and rx parameters
     */
    PRV_CPSS_PORT_MANAGER_SERDES_RX_PARAMS_E,

    PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E

} PRV_CPSS_PORT_MANAGER_SERDES_PARAMS_TYPE_ENT;

/**
* @enum PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ENT
 *
 * @brief Enumeration of the serdes Tx/Rx parameters combinations.
*/
typedef enum{

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E,

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E,

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_MAC_E,

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_SERDES_E,

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E,

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E,

    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_LASE_E

} PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ENT;

/**
* @enum PRV_CPSS_PORT_MANAGER_USER_OVERRIDE_BITMAP_ENT
 *
 * @brief Enumeration of the port training modes the caller
 * want to override
*/
typedef enum{

    /** training mode to override */
    PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E,

    /** override adaptive rx training support */
    PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E,

    /** override edge detect support */
    PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E,

    /* Loopback mode - one of MAC, PCS or SerDes loopback mode. */
    PRV_CPSS_PM_ATTR_LOOPBACK_E,

    /* Params to override the default ones during Enhanced Tuning */
    PRV_CPSS_PM_ATTR_ET_OVERRIDE_E,

    /* Forward Error Correction mode : Reed-Solomon(clause 91)/ Base-R(clause 74)/ None/ Both */
    PRV_CPSS_PM_ATTR_FEC_MODE_E,

    /* Calibration - Cu or Optical */
    PRV_CPSS_PM_ATTR_CALIBRATION_E,

    /* Whether to allow PortManager to unMask MAC and/or LOW level events */
    PRV_CPSS_PM_ATTR_UNMASK_EV_MODE_E,

    /* Override BW mode */
    PRV_CPSS_PM_ATTR_BW_MODE_E,

    /* parameters to enable link-binding on port */
    PRV_CPSS_PM_ATTR_LKB_E,

    /* port operations */
    PRV_CPSS_PM_ATTR_PORT_OPERATIONS_E,

    /* interconnect profile set */
    PRV_CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E,

    /*preemption feature (APPLICABLE DEVICE :AC5P)*/
    PRV_CPSS_PM_PORT_ATTR_PREEMPTION_E,

    /* enable/disable auto negotiation */
    PRV_CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE,

    /* enable/disable pm over FW */
    PRV_CPSS_PM_PORT_ATTR_PM_OVER_FW_ENABLE_E

} PRV_CPSS_PORT_MANAGER_USER_OVERRIDE_BITMAP_ENT;

/**
* @struct PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC
 *
 * @brief This structure contains stability results for each port low level unit
*/
typedef struct{

    /** whether or not align */
    GT_BOOL alignLock;

    /** whether or not gear */
    GT_BOOL gbLock;

    /** whether or not signal detecti indication of PCS is stable */
    GT_BOOL sigDet;

    /** @brief whether or not all low
     *  Comments:
     *  None.
     */
    GT_BOOL overallStatusesOk;

} PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC;

/**
* @internal internal_cpssPortManagerEventSetfunction
* @endinternal
*
* @brief  getting event and according the port manager state
*         machine, it's handling the "event functions"
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] portEventStc          - struct with the event
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerEventSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_MANAGER_STC *portEventStc
);

GT_STATUS internal_cpssPortManagerStatusGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

GT_STATUS internal_cpssPortManagerInitParamsStruct
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    INOUT  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr
);

GT_STATUS internal_cpssPortManagerPortParamsSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr
);

GT_STATUS internal_cpssPortManagerPortParamsGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PM_PORT_PARAMS_STC    *portParamsStcPtr
);

/**
* @internal prvCpssPortManagerInit function
* @endinternal
*
* @brief   Initialize and checking port parameters function
*          pointer binding
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPortManagerInit
(
    IN GT_SW_DEV_NUM devNum
);

GT_STATUS prvCpssPortManagerDbgInitPorts
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32 timeOut
);

GT_STATUS prvCpssPortManagerDbgDumpPortState
(
    GT_SW_DEV_NUM devNum,
    GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal prvCpssPortManagerStateRestore function
* @endinternal
*
* @brief   Restore state of port. This API is used to restore state of a port
*         within the port manager state machine according to the poer data in
*         CPSS database. It should be called during catch-up process and after
*         CPSS port database have been restored.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerStateRestore
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);


/**
* @internal prvCpssPortMacEventConvert function
* @endinternal
*
* @brief   Convert Mac event of a mac which is not the first mac in a multi-MAC
*         interface, to a Mac event of the first Mac of the interface. The convert
*         is done by replacing the extra data which is the Mac number, to the Mac
*         number of the first Mac in the interface. For example, consider an interface
*         of 40G port which consist of 4 lanes. Lanes 1-3 can raise interrupts
*         for Macs 1-3. This function will convert Macs 1-3 interrupts to the First
*         Mac so that, for example, event CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E
*         with extra data "1" will be converted to extra data "0".
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
*                                      portNum   - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortMacEventConvert
(
    GT_U8 devNum,
    GT_U32 *macPortNum
);

/**
* @internal prvCpssPortXlgMacMaskConfig function
* @endinternal
*
* @brief   Configure XLG mac to enable PCS interrrupts to be raised from the port
*         associated with the given MAC index (which is an additional port in a
*         multi-lane interface and not the first port of the interface), and also,
*         mask every other interrupt on the mac.
*         The operations being done are:
*         (1) configuring mac dma mode field in the mac to allow interrupt to flow.
*         (2) masking all mac interrupts in order to get only PCS interrupts from
*         the port.
*         This function acts as a work-around in order to get all PCS interrupts
*         for every serdes in a given multi-lane port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] restore                  - whether or not to  XLG mac state to previous
*                                      state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortXlgMacMaskConfig
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL         restore
);

/**
* @internal prvCpssPortManagerPushEvent function
* @endinternal
*
* @brief   Push artificial event into event queues of CPSS.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group
* @param[in] portNum                  - physical port number
* @param[in] extraData                - extra data relevant for the unified event
* @param[in] uniEv                    - unified event to push
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortManagerPushEvent
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   extraData,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEv
);

/**
* @internal prvCpssPortManagerLanePolarityHwSet function
* @endinternal
*
* @brief   set serdes lane rx and tx polarity to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] globalSerdesIndex        - lane num
*                                      rxInvert             - rx polarity data
*                                      txInvert             - tx polarity data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLanePolarityHwSet
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U16 globalSerdesIndex
);

/**
* @internal prvCpssPortManagerLaneTuneDbGet function
* @endinternal
*
* @brief   Get serdes lane RxTx parameters from port manager data base.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum           - device number
* @param[in] portNum          - port number
* @param[in] advertiseNum     - advertise number
* @param[in] serdesIndex      - serdex index
* @param[in] portType         - port type
* @param[out] serdesParamsPtr - serdes parameters
* @param[out] dbValidBitMapPtr- ValidBitMap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssPortManagerLaneTuneDbGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32 advertiseNum, /* for ap only */
    IN  GT_U32 serdesIndex,
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    OUT CPSS_PORT_SERDES_TUNE_STC *serdesParamsPtr,
    OUT GT_U32                    *dbValidBitMapPtr
);

/**
* @internal prvCpssPortManagerLaneTuneDbSet function
* @endinternal
*
* @brief   Set serdes lane RxTx parameters in port manager data base.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] advertiseNum             - advertise number
* @param[in] SerdesIndex              - global/lane serdex index
* @param[in] portType                 - AP or Non-AP port type
* @param[in] dbValidBitMap            - which parameters are
*       Valid
* @param[in] tuneValuesPtr            - (pointer to) serdes RxTx parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssPortManagerLaneTuneDbSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN GT_U32                           advertiseNum, /* for ap only */
    IN GT_U32                           serdesIndex,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT  portType,
    IN GT_U32                           dbValidBitMap,
    IN CPSS_PORT_SERDES_TUNE_STC        *tuneValuesPtr
);

/**
* @internal prvCpssPortManagerLaneTuneHwSet function
* @endinternal
*
* @brief   Set serdes lane RxTx parameters from port manager data base into serdes Hw.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] globalSerdesIndex - serdes index
* @param[in] rxSet          - whether or not to set the rx parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLaneTuneHwSet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_U32 portGroup,
    IN GT_PHYSICAL_PORT_NUM portNum,
    MV_HWS_PORT_STANDARD portMode,
    IN GT_U32 globalSerdesIndex,
    IN GT_BOOL rxSet
);

/**
* @internal prvCpssPortManagerApLaneTuneHwSet function
* @endinternal
*
* @brief  Set serdes AP lane RxTx parameters from port manager
*         data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group number
* @param[in] portNum         - physical port number
* @param[in] advertisedNum   - advertised interface number
* @param[in] laneSerdesIndex - lane serdes index
* @param[in] curPortParams   - current port parameters
* @param[in] rxSet          - whether or not to set the rx parameters
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*
* @note this function is for ap port, for non-ap ports use:
*       prvCpssPortManagerLaneTuneHwSet
*/
GT_STATUS prvCpssPortManagerApLaneTuneHwSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroup,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   advertisedNum,
    IN GT_U32                   laneSerdesIndex,
    IN MV_HWS_PORT_INIT_PARAMS  curPortParams,
    IN GT_BOOL                  rxSet
);

/**
* @internal prvCpssPortManagerApLaneTxPresetHwSet function
* @endinternal
*
* @brief  Set serdes AP TX Preset parameters from port manager
*         data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   Falcon.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2; xCat3; AC5;
*        Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group number
* @param[in] portNum         - physical port number
* @param[in] portMode        - Port Mode
* @param[in] laneSerdesIndex - lane serdes index
* @param[in] curPortParams   - current port parameters
* @param[in] interconnectProfile - Profile
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*
* @note this function is for ap port
*/
GT_STATUS prvCpssPortManagerApLaneTxPresetHwSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroup,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   laneSerdesIndex,
    IN MV_HWS_PORT_INIT_PARAMS  curPortParams,
    IN GT_U32                   interconnectProfile
);

/**
* @internal prvCpssPortManagerGetPortHwsParams function
* @endinternal
*
* @brief  Get Hws parameters for the given port according to it's parameters.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum                 - device number
* @param[in] portGroup              - port group number
* @param[in] portNum                - physical port number
*
* @param[out] curPortParamsPtrPtr   - (pointer to pointer to) hws parameters
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR         - on null pointer
*/
GT_STATUS prvCpssPortManagerGetPortHwsParams
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroup,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT MV_HWS_PORT_INIT_PARAMS *curPortParamsPtr
);

/**
* @internal prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed function
* @endinternal
*
* @brief  Get Hws parameters for the given port according to it's ifMode and speed.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum              - device number
* @param[in] portGroup           - port group number
* @param[in] portNum             - physical port number
* @param[in] ifMode              - port interface mode
* @param[in] speed               - port speed
*
* @param[out] curPortParamsPtr   - pointer to hws parameters
*
* @retval GT_OK                  - on success
* @retval GT_FAIL                - on error
* @retval GT_NOT_SUPPORTED       - on not supported device
* @retval GT_BAD_PARAM           - on bad physical port number
* @retval GT_BAD_PTR             - on null pointer
*/
GT_STATUS prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       portGroup,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed,
    OUT MV_HWS_PORT_INIT_PARAMS      *curPortParamsPtr
);

/**
* @internal prvCpssPortManagerCleanPortParameters function
* @endinternal
*
* @brief   Clean all port parameters in Port Manager DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - AP or Non-AP port type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortManagerCleanPortParameters
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_U32                          portGroup,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT portType
);

/**
* @internal prvCpssPortManagerLoopbackModesDbGet function
* @endinternal
*
* @brief   Get loopback mode of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                    - device number
* @param[in] portNum                   - physical port number
* @param[in] portType                  - AP or Non-AP port type
*
* @param[out] loopbackCfgPtrOut        - (pointer to) loopback configuration structure
* @param[out] anyLoopbackPtr           - (pointer to) whether or not any loopback is configured
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssPortManagerLoopbackModesDbGet
(
    IN GT_SW_DEV_NUM                           devNum,
    IN GT_PHYSICAL_PORT_NUM                    portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType,
    OUT CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtrOut,
    OUT GT_BOOL                               *anyLoopbackPtr
);

/**
* @internal prvCpssPortManagerLoopbackModesDbSet function
* @endinternal
*
* @brief  Set loopback mode of the port.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] portType        - AP or Non-AP port type
* @param[in] loopbackCfgPtr  - (pointer to) loopback configuration structure
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_BAD_PTR         - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLoopbackModesDbSet
(
    IN GT_SW_DEV_NUM                         devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT       portType,
    IN CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtr
);

/**
* @internal internal_cpssPortManagerGlobalParamsOverride
*           function
* @endinternal
*
* @brief   Override global parameters, using flags to check if
*          override or using defaults
*
* @param[in] devNum                - device number
* @param[in] globalParamsStcPtr    - stc with the
*                                   parameters to override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerGlobalParamsOverride
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC     *globalParamsStcPtr
);

/**
* @internal internal_cpssPortManagerInit function
* @endinternal
*
* @brief   Init the portManager
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerInit
(
    IN  GT_U8                                   devNum
);

/**
* @internal internal_cpssPortManagerPortParamsUpdate
*      function
* @endinternal
*
* @brief  update port parameters also when port is active.
*
* @param[in] devNum        - device number
* @param[in] updateParamsStcPtr  - stc with the parameters to
*       override
*
* @retval GT_OK          - on success
* @retval GT_BAD_PARAM       - on wrong port number or device
* @retval GT_HW_ERROR       - on hardware error
* @retval GT_BAD_PTR        - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerPortParamsUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *updateParamsStcPtr
);

/**
* @internal internal_cpssPortManagerEnableGet
* @endinternal
*
* @brief  checks if portManager enabled/disabled per device
*
* @param[in] devNum                - device number
*
* @param[out] enablePtr            - Port-manager status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssPortManagerEnableGet
(
    IN  GT_U8 devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal internal_cpssPortManagerStatGet
* @endinternal
*
* @brief  get portManager statistics
*
* @param[in]  devNum               - device number
* @param[in]  portNum              - port number
* @param[out]  portStatStcPtr       - port statistic structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*
*
*/
GT_STATUS internal_cpssPortManagerStatGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT CPSS_PORT_MANAGER_STATISTICS_STC    *portStatStcPtr
);


/**
* @internal internal_cpssPortManagerStatClear
* @endinternal
*
* @brief  clear portManager statistic db
*
* @param[in]  devNum               - device number
* @param[in]  portNum              - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
*
*
*/
GT_STATUS internal_cpssPortManagerStatClear
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortManager_h */
