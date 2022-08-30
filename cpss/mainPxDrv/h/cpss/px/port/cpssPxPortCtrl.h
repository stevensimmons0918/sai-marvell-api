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
* @file cpssPxPortCtrl.h
*
* @brief CPSS PX implementation for Port configuration and control facility.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortCtrl_h
#define __cpssPxPortCtrl_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

/* definition for the number of PSC lanes per port */
#define CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS 4

#define CPSS_PX_PORT_MAX_MRU_CNS (_12K+64)  /* jumbo frame + 64  (including 4 bytes CRC) */

/**
* @enum CPSS_PX_PORT_FC_MODE_ENT
 *
 * @brief Enumeration of FC modes.
*/
typedef enum{

    /** 802.3X */
    CPSS_PX_PORT_FC_MODE_802_3X_E,

    /** PFC: Priority Flow Control */
    CPSS_PX_PORT_FC_MODE_PFC_E,

    /** LL FC: Link Level Flow control */
    CPSS_PX_PORT_FC_MODE_LL_FC_E

} CPSS_PX_PORT_FC_MODE_ENT;

/**
* @enum CPSS_PX_PORT_UNITS_ID_ENT
 *
 * @brief Defines Port Units ID.
*/
typedef enum{

    /** GEMAC UNIT */
    CPSS_PX_PORT_UNITS_ID_GEMAC_UNIT_E,

    /** XLGMAC UNIT */
    CPSS_PX_PORT_UNITS_ID_XLGMAC_UNIT_E,

    /** HGLMAC UNIT */
    CPSS_PX_PORT_UNITS_ID_HGLMAC_UNIT_E,

    /** XPCS UNIT */
    CPSS_PX_PORT_UNITS_ID_XPCS_UNIT_E,

    /** MMPCS UNIT */
    CPSS_PX_PORT_UNITS_ID_MMPCS_UNIT_E,

    /** CG UNIT */
    CPSS_PX_PORT_UNITS_ID_CG_UNIT_E,

    /** INTLKN UNIT */
    CPSS_PX_PORT_UNITS_ID_INTLKN_UNIT_E,

    /** INTLKN RF UNIT */
    CPSS_PX_PORT_UNITS_ID_INTLKN_RF_UNIT_E,

    /** SERDES UNIT */
    CPSS_PX_PORT_UNITS_ID_SERDES_UNIT_E,

    /** SERDES PHY UNIT */
    CPSS_PX_PORT_UNITS_ID_SERDES_PHY_UNIT_E,

    /** @brief ETI UNIT
     *  CPSS_PX_PORT_UNITS_ID_ETI_ILKN_RF_UNIT_E ETI ILKN RF UNIT
     */
    CPSS_PX_PORT_UNITS_ID_ETI_UNIT_E,

    CPSS_PX_PORT_UNITS_ID_ETI_ILKN_RF_UNIT_E,

    /** D UNIT (DDR3) */
    CPSS_PX_PORT_UNITS_ID_D_UNIT_E

} CPSS_PX_PORT_UNITS_ID_ENT;

/**
* @enum CPSS_PX_PP_SERDES_REF_CLOCK_ENT
 *
 * @brief Defines SERDES reference clock type.
*/
typedef enum{

    /** @brief 25 MHz external
     *  single ended reference clock.
     */
    CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,

    /** @brief 125 MHz external
     *  single ended reference clock.
     */
    CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E,

    /** @brief 125 MHz external
     *  differential reference clock.
     */
    CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E,

    /** @brief 156.25 MHz external
     *  single ended reference clock.
     */
    CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E,

    /** @brief 156.25 MHz external
     *  differential reference clock.
     */
    CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,

    /** 125 MHz internal reference clock */
    CPSS_PX_PP_SERDES_REF_CLOCK_INTERNAL_125_E

} CPSS_PX_PP_SERDES_REF_CLOCK_ENT;



/**
* @enum CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT
 *
 * @brief Enumeration of port's PCS loopback modes.
*/
typedef enum{

    /** no any loopback on PCS */
    CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E,

    /** TX to RX loopback */
    CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E,

    /** RX to TX loopback */
    CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E

} CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT;

/**
* @enum CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT
 *
 * @brief Enumeration of port's SERDES CPLL input clock.
*/
typedef enum{

    /** input CPLL frequency = 25Mhz */
    CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_25M_E,

    /** input CPLL frequency = 156Mhz */
    CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_156M_E

} CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT;

/**
* @enum CPSS_PX_PORT_CTLE_BIAS_MODE_ENT
 *
 * @brief Defines ctle bias mode per port
*/
typedef enum{

    /** default ctle bias mode */
    CPSS_PX_PORT_CTLE_BIAS_NORMAL_E,

    /** for short length calbes */
    CPSS_PX_PORT_CTLE_BIAS_HIGH_E

} CPSS_PX_PORT_CTLE_BIAS_MODE_ENT;
/**
* @enum CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT
 *
 * @brief Enumeration of port's SERDES CPLL output clock.
*/
typedef enum{

    /** output CPLL frequency = 78Mhz */
    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_78M_E,

    /** output CPLL frequency = 156Mhz */
    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_156M_E,

    /** output CPLL frequency = 200Mhz */
    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_200M_E,

    /** output CPLL frequency = 312.5Mhz */
    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_312M_E,

    /** output CPLL frequency = 160.9Mhz */
    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_161M_E,

    /** output CPLL frequency = 164.2Mhz */
    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_164M_E

} CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT;


/**
* @enum CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT
 *
 * @brief Enumeration of port's serdes loopback modes.
*/
typedef enum{

    /** no any loopback on serdes */
    CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E,

    /** analog TX to RX loopback */
    CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E,

    /** digital TX to RX loopback */
    CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E,

    /** digital RX to TX loopback */
    CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E,

    /** enum type upper boarder */
    CPSS_PX_PORT_SERDES_LOOPBACK_MAX_E

} CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT;

/**
* @struct CPSS_PX_PORT_ALIGN90_PARAMS_STC
*/
typedef struct{

    GT_U32 startAlign90;

    /** @brief osDeltaMax;
     *  adaptedFfeR;
     *  adaptedFfeC;
     *  Comments:
     *  None
     */
    GT_U32 rxTrainingCfg;

    GT_U32 osDeltaMax;

    GT_U32 adaptedFfeR;

    GT_U32 adaptedFfeC;

} CPSS_PX_PORT_ALIGN90_PARAMS_STC;

/**
* @struct CPSS_PX_PORT_SERDES_EYE_INPUT_STC
 *
 * @brief A struct containing the input parameters for Serdes Eye Monitoring
*/
typedef struct{

    /** @brief minimum dwell bits
     *  valid range: 100000 --- 100000000 .If 0 - transform to default 100000.
     */
    GT_U32 min_dwell_bits;

    /** @brief maximum dwell bits
     *  valid range: 100000 --- 100000000 .If 0 - transform to default 100000000.
     *  Comments:
     *  should be min_dwell_bits <= max_dwell_bits
     */
    GT_U32 max_dwell_bits;

} CPSS_PX_PORT_SERDES_EYE_INPUT_STC;

/**
* @struct CPSS_PX_PORT_SERDES_EYE_RESULT_STC
 *
 * @brief A struct containing the results for Serdes Eye Monitoring.
*/
typedef struct{

    GT_CHAR *matrixPtr;

    /** number of columns in matrix */
    GT_U32 x_points;

    /** @brief number of rows in matrix
     *  vbtcPtr     - vbtc text result
     *  hbtcPtr     - hbtc text result
     */
    GT_U32 y_points;

    GT_CHAR *vbtcPtr;

    GT_CHAR *hbtcPtr;

    /** height in mV */
    GT_U32 height_mv;

    /** width in m_UI */
    GT_U32 width_mui;

    /** @brief global serdes lane number in device
     *  Comments:
     *  None
     */
    GT_U32 globalSerdesNum;

} CPSS_PX_PORT_SERDES_EYE_RESULT_STC;

/**
* @struct CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC
 *
 * @brief A struct containing parameters to build
 * <TX Config Reg> data for Auto-Negotiation.
*/
typedef struct{

    /** port Link Up if GT_TRUE, Link Down if GT_FALSE; */
    GT_BOOL link;

    /** port speed; */
    CPSS_PORT_SPEED_ENT speed;

    /** @brief port duplex mode;
     *  Comments:
     *  None
     */
    CPSS_PORT_DUPLEX_ENT duplex;

} CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC;

/**
* @internal cpssPxPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegAdvertismentConfigSet
(
    IN   GT_SW_DEV_NUM                            devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    IN   CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC    *portAnAdvertismentPtr
);

/**
* @internal cpssPxPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegMasterModeEnableGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
);

/**
* @internal cpssPxPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  Pipe.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE   -  Auto-Negotiation Master Mode and code word
*                                      GT_FALSE  - device defines Auto-Negotiation code word
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegMasterModeEnableSet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_BOOL                  enable
);


/**
* @internal cpssPxPortInbandAutoNegRestart function
* @endinternal
*
* @brief   Restart inband auto-negotiation. Relevant only when inband auto-neg.
*         enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInbandAutoNegRestart
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal cpssPxPortCpllCfgInit function
* @endinternal
*
* @brief   CPLL unit initialization routine.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] cpllNum                  - CPLL unit index
*                                      (APPLICABLE RANGES: 0)
* @param[in] inputFreq                - CPLL input frequency
* @param[in] outputFreq               - CPLL output frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
GT_STATUS cpssPxPortCpllCfgInit
(
    IN GT_SW_DEV_NUM                              devNum,
    IN GT_U32                                     cpllNum,
    IN CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT      inputFreq,
    IN CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT     outputFreq
);

/**
* @internal cpssPxPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkPassEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  state
);

/**
* @internal cpssPxPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) current force link pass state:
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkPassEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *statePtr
);

/**
* @internal cpssPxPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkDownEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  state
);

/**
* @internal cpssPxPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) current force link down state:
*                                      GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkDownEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *statePtr
);

/**
* @internal cpssPxPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] mruSize                  - max receive packet size in bytes.
*                                      (APPLICABLE RANGES: 0..10304) - jumbo frame + 64 (including
*                                      4 bytes CRC). Value must be even.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMruSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                mruSize
);

/**
* @internal cpssPxPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMruGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U32                *mruSizePtr
);

/**
* @internal cpssPxPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortLinkStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *isLinkUpPtr
);

/**
* @internal cpssPxPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInternalLoopbackEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInternalLoopbackEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port
*                                     DB)
*
* @note The API rolls back a port's mode and speed to their last values
*       if they cannot be set together on the device.
*       Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*
*/
GT_STATUS cpssPxPortModeSpeedSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORTS_BMP_STC              *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal cpssPxPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssPxPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEnableGet
(
    IN   GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
);

/**
* @internal cpssPxPortEomBaudRateGet function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssPxPortEomBaudRateGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
);

/**
* @internal cpssPxPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS cpssPxPortSerdesTxEnableSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssPxPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - Pointer to transmission of packets in SERDES
*                                      layer of a port.
*                                      - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port.
*                                      - GT_FALSE - Enable transmission of packets in
*                                      SERDES layer of a port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesTxEnableGet
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssPxPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: [0...31]
*                                      txAmpAdjEn - not rellevant for Pipe
*                                      emph0 - Post-Cursor: Controls the Emphasis Amplitude for Gen0 bit rates
*                                      for ports  #0-#11: [-31...31]
*                                      for ports #12-#15: [0...31]
*                                      emph1 - Pre-Cursor: Controls the emphasis amplitude for Gen1 bit rates
*                                      for ports  #0-#11: [-31...31]
*                                      for ports #12-#15: [0...31]
*                                      txAmpShft - not rellevant for Pipe
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
GT_STATUS cpssPxPortSerdesManualTxConfigSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
);

/**
* @internal cpssPxPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
GT_STATUS cpssPxPortSerdesManualRxConfigSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
);

/**
* @internal cpssPxPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes RX.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note The squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
GT_STATUS cpssPxPortSerdesManualRxConfigGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    OUT CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
);

/**
* @internal cpssPxPortSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes (true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssPxPortSerdesSignalDetectGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/**
* @internal cpssPxPortSerdesSquelchSet function
* @endinternal
*
* @brief   Set For port Threshold (Squelch) for signal OK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] squelch                  - threshold for signal OK (0-15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssPxPortSerdesSquelchSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  squelch
);

/**
* @internal cpssPxPortSerdesStableSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssPxPortSerdesStableSignalDetectGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/**
* @internal cpssPxPortSerdesLaneSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on specific SerDes lane (true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - number of local core
* @param[in] laneNum                  - number of required serdes lane
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesLaneSignalDetectGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          laneNum,
    OUT GT_BOOL         *signalStatePtr
);

/**
* @internal cpssPxPortSerdesCDRLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] cdrLockPtr               - CRD lock state on serdes:
*                                      GT_TRUE  - CDR locked;
*                                      GT_FALSE - CDR not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - cdrLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesCDRLockStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
);

/**
* @internal cpssPxPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssPxPortSerdesAutoTune.
*
*/
GT_STATUS cpssPxPortSerdesAutoTuneOptAlgSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
);

/**
* @internal cpssPxPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesOptAlgBmpPtr       - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesAutoTuneOptAlgGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *serdesOptAlgBmpPtr
);

/**
* @internal cpssPxPortSerdesAutoTuneResultsGet function
* @endinternal
*
* @brief   Read the results of SERDES auto tuning.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTunePtr            - serdes Tune parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesAutoTuneResultsGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TUNE_STC       *serdesTunePtr
);

/**
* @internal cpssPxPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                                      in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssPxPortSerdesPowerStatusSet will write them to HW.
*
*/
GT_STATUS cpssPxPortSerdesTuningSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssPxPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure to put tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssPxPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
GT_STATUS cpssPxPortSerdesTuningGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssPxPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssPxPortSerdesPowerStatusSet or cpssPxPortModeSpeedSet will write
*       them to HW.
*
*/
GT_STATUS cpssPxPortSerdesLaneTuningSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroupNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssPxPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesResetStateSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL               state
);

/**
* @internal cpssPxPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note Directions for TX training execution:
*       1. As a pre-condition, ports on both sides of the link must be UP.
*       2. For 2 sides of the link call execute TX_TRAINING_CFG;
*       No need to maintain special timing sequence between them.
*       The CFG phase sets some parameters at the SerDes as a preparation
*       to the training phase.
*       3. After CFG is done, for both sides of the link, call TX_TRAINING_START;
*       No need to maintain special timing sequence between them.
*       4. Wait at least 0.5 Sec. (done by API inside).
*       5. Verify Training status by calling TX_TRAINING_STATUS.
*       No need to maintain special timing sequence between them.
*       This call provides the training status (OK/Failed) and terminates it.
*
*/
GT_STATUS cpssPxPortSerdesAutoTune
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT   portTuningMode
);

/**
* @internal cpssPxPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes including required
*         optimizations.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number, device, serdesOptAlgBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note See cpssPxPortSerdesAutoTune.
*
*/
GT_STATUS cpssPxPortSerdesAutoTuneExt
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT   portTuningMode,
    IN  GT_U32                                   serdesOptAlgBmp
);

/**
* @internal cpssPxPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - number of port group (local core), not used for non-multi-core
* @param[in] serdesNum                - serdes number
* @param[in] numOfBits                - number of bits to inject to serdes data
* @param[in] direction                - whether  is TX, RX or both
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
GT_STATUS cpssPxPortSerdesErrorInject
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   serdesNum,
    IN GT_U32   numOfBits,
    IN CPSS_PORT_DIRECTION_ENT  direction
);

/**
* @internal cpssPxPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxTuneStatusPtr          - RX tuning status
* @param[out] txTuneStatusPtr          - TX tuning status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxPortSerdesAutoTuneStatusGet
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
);

/**
* @internal cpssPxPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
*
* @param[out] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
GT_STATUS cpssPxPortSerdesLanePolarityGet
(
    IN   GT_SW_DEV_NUM  devNum,
    IN   GT_U32         portGroupNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *invertTx,
    OUT  GT_BOOL        *invertRx
);

/**
* @internal cpssPxPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
GT_STATUS cpssPxPortSerdesLanePolaritySet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32          portGroupNum,
    IN  GT_U32          laneNum,
    IN  GT_BOOL         invertTx,
    IN  GT_BOOL         invertRx
);

/**
* @internal cpssPxPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*                                      polarity on.
* @param[in] invertTx                 -  GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 -  GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
GT_STATUS cpssPxPortSerdesPolaritySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
);

/**
* @internal cpssPxPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*                                      define polarity on.
*
* @param[out] invertTxPtr              - (ptr to) GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRxPtr              - (ptr to) GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesPolarityGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               laneNum,
    OUT GT_BOOL              *invertTxPtr,
    OUT GT_BOOL              *invertRxPtr
);

/**
* @internal cpssPxPortSerdesEnhancedAutoTune function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesEnhancedAutoTune
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_LF,
    IN  GT_U8                  max_LF
);

/**
* @internal cpssPxPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device number
* @param[in] portNum - physical port number
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS cpssPxPortSerdesLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT   mode
);

/**
* @internal cpssPxPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                                      to read loopback status
*
* @param[out] modePtr                  - current loopback mode or none
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesLoopbackModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT   *modePtr
);

/**
* @internal cpssPxPortEomDfeResGet function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] dfeResPtr                - current DFE V in millivolts
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssPxPortEomDfeResGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
);

/**
* @internal cpssPxPortSerdesEyeMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix for SERDES.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - SERDES number
* @param[in] eye_inputPtr             - pointer to the serdes Eye monitoring input parameters structure
*
* @param[out] eye_resultsPtr           - pointer to the serdes Eye monitoring results structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - if not supported
* @retval else                     - on error
*
* @note The function allocated memory and gets the SERDES EOM matrix, vbtc and
*       hbtc calculation in text format.
*       After using this function application needs to free allocated memory, for example:
*       cpssOsFree(eye_resultsPtr->matrixPtr);
*       cpssOsFree(eye_resultsPtr->vbtcPtr);
*       cpssOsFree(eye_resultsPtr->hbtcPtr);
*
*/
GT_STATUS cpssPxPortSerdesEyeMatrixGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN  CPSS_PX_PORT_SERDES_EYE_INPUT_STC  *eye_inputPtr,
    OUT CPSS_PX_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
);

/**
* @internal cpssPxPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
GT_STATUS cpssPxPortSerdesLaneTuningGet
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_U32                  portGroupNum,
    IN   GT_U32                  laneNum,
    IN   CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssPxPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes TX.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTxCfgPtr           - serdes Tx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesManualTxConfigGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
);

/**
* @internal cpssPxPortInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInterfaceSpeedSupportGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
);

/**
* @internal cpssPxPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_VALUE             - on wrong speed value in the register
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegAdvertismentConfigGet
(
    IN   GT_SW_DEV_NUM                            devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC    *portAnAdvertismentPtr
);

/*-----------------------------------------------*/
/*  Pizza Arbiter Unit Description               */
/*-----------------------------------------------*/
/**
* @enum CPSS_PX_PA_UNIT_ENT
 *
 * @brief Enumeration of PA units
*/
typedef enum{

    /** @brief undefined unit
     *  CPSS_PX_PA_UNIT_RXDMA_0_E     RXDMA
     *  CPSS_PX_PA_UNIT_TXQ_0_E,     - TXQ
     *  CPSS_PX_PA_UNIT_TXQ_1_E,     - TXQ
     *  CPSS_PX_PA_UNIT_TXDMA_0_E,    - TXDMA
     *  CPSS_PX_PA_UNIT_TX_FIFO_0_E,   - TX_FIFO,
     */
    CPSS_PX_PA_UNIT_UNDEFINED_E = -1

    ,CPSS_PX_PA_UNIT_RXDMA_0_E = 0

    ,CPSS_PX_PA_UNIT_TXQ_0_E

    ,CPSS_PX_PA_UNIT_TXQ_1_E

    ,CPSS_PX_PA_UNIT_TXDMA_0_E

    ,CPSS_PX_PA_UNIT_TX_FIFO_0_E

    /** use as array boundary */
    ,CPSS_PX_PA_UNIT_MAX_E

} CPSS_PX_PA_UNIT_ENT;

/**
* @internal cpssPxPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] overrideEnable           - override
*                                      GT_TRUE - use preconfigured refClock source;
*                                      GT_FALSE - use default refClock source;
* @param[in] refClockSource           - reference clock source.
*                                      Not relevant when overrideEnable is false.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortRefClockSourceOverrideEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
);


/**
* @internal cpssPxPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] overrideEnablePtr        - (pointer to) override enable status
* @param[out] refClockSourcePtr        - (pointer to) reference clock source.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortRefClockSourceOverrideEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
);

/**
* @internal cpssPxPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFecModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FEC_MODE_ENT mode
);

/**
* @internal cpssPxPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFecModeGet
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_PORT_FEC_MODE_ENT *modePtr
);

/**
* @internal cpssPxPortInterfaceModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS cpssPxPortInterfaceModeGet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);
/**
* @internal cpssPxPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS cpssPxPortSpeedGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal cpssPxPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssPxPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*
*/
GT_STATUS cpssPxPortSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal cpssPxPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesPowerStatusSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp
);

/**
* @internal cpssPxPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the flow control auto negotiation
*
*/
GT_STATUS cpssPxPortFlowCntrlAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise
);

/**
* @internal cpssPxPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
GT_STATUS cpssPxPortFlowCntrlAutoNegEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL                *statePtr,
    OUT  GT_BOOL                *pauseAdvertisePtr
);

/**
* @internal cpssPxPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Before calling cpssPxPortFlowControlEnableSet,
*       cpssPxPortPeriodicFlowControlCounterSet should be called
*       to set the interval between the transmission of two consecutive
*       Flow Control packets according to port speed
*
*/
GT_STATUS cpssPxPortFlowControlEnableSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
);

/**
* @internal cpssPxPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFlowControlEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
);

/**
* @internal cpssPxPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function also configures insertion of DSA tag for PFC frames.
*
*/
GT_STATUS cpssPxPortFlowControlModeSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PX_PORT_FC_MODE_ENT fcMode
);

/**
* @internal cpssPxPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
*
* @param[out] fcModePtr                - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFlowControlModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_FC_MODE_ENT  *fcModePtr
);

/**
* @internal cpssPxPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
*
* @param[out] portAttributSetArrayPtr  - Pointer to attributes values array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortAttributesOnPortGet
(
    IN    GT_SW_DEV_NUM             devNum,
    IN    GT_PHYSICAL_PORT_NUM      portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
);


/**
* @internal cpssPxPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssPxPortSpeedAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
);

/**
* @internal cpssPxPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssPxPortSpeedAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_BOOL                *statePtr
);

/**
* @internal cpssPxPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex cpssPxPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortDuplexAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statePtr
);

/**
* @internal cpssPxPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the duplex auto negotiation
*
*/
GT_STATUS cpssPxPortDuplexAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
);

/**
* @internal cpssPxPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] dMode                    - duplex mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxPortDuplexModeSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
);

/**
* @internal cpssPxPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortDuplexModeGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
);

/**
* @internal cpssPxPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note 1. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssPxPortBackPressureEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
);

/**
* @internal cpssPxPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssPxPortBackPressureEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssPxPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCrcCheckEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCrcCheckEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*
*/
GT_STATUS cpssPxPortExcessiveCollisionDropEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note 1. Not relevant in full duplex mode
*
*/
GT_STATUS cpssPxPortExcessiveCollisionDropEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInBandAutoNegBypassEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to Auto-Negotiation by pass status.
*                                      - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInBandAutoNegBypassEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacResetStateSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              state
);

/**
* @internal cpssPxPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                                      GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port�s packet transmission if it is an XOFF packet, or to
*       resume the port�s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet�s Length/EtherType field is 88-08
*       - Packet�s OpCode field is 00-01
*       - Packet�s MAC DA is 01-80-C2-00-00-01 or the port�s configured MAC Address
*
*/
GT_STATUS cpssPxPortForward802_3xEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of 802.3x frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port's packet transmission if it is an XOFF
*       packet, or to resume the port's packets transmission, if
*       it is an XON packet) if all of the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is 00-01
*       - Packet's MAC DA is 01-80-C2-00-00-01 or the port's
*         configured MAC Address
*
*/
GT_STATUS cpssPxPortForward802_3xEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                                      GT_FALSE: do not forward unknown MAC control frames to
*                                      the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is not 00-01 and not 01-01
*       OR Packet's MAC DA is not 01-80-C2-00-00-01 and not the
*       port's configured MAC Address
*
*/
GT_STATUS cpssPxPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of unknown MAC control frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is not 00-01 and not 01-01
*       OR Packet's MAC DA is not 01-80-C2-00-00-01 and not the
*       port's configured MAC Address
*
*/
GT_STATUS cpssPxPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInbandAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInbandAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssPxPortMacTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacTypeGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr
);

/**
* @internal cpssPxPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] ipg                      - IPG in bytes, acceptable range:
*                                      (APPLICABLE RANGES: 0..511) (APPLICABLE DEVICES pipe)
*                                      Default HW value is 12 bytes.
*                                       for CG MAC -->
*                                       1-8 means IPG depend on
*                                       packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS cpssPxPortIpgSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  ipg
);

/**
* @internal cpssPxPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] ipgPtr                   - (pointer to) IPG value in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxPortIpgGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_U32                 *ipgPtr
);

/**
* @internal cpssPxPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ipgBase                  - IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or ipgBase
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssPxPortIpgBaseSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
);

/**
* @internal cpssPxPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] ipgBasePtr               - pointer to IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssPxPortIpgBaseGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
);

/**
* @internal cpssPxPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal cpssPxPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPreambleLengthGet(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_U32                  *lengthPtr
);

/**
* @internal cpssPxPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on unsupported request
*
* @note Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: PX_CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
GT_STATUS cpssPxPortPeriodicFcEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
);

/**
* @internal cpssPxPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
GT_STATUS cpssPxPortPeriodicFcEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
);

/**
* @internal cpssPxPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] valuePtr                 - (pointer to) The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPeriodicFlowControlCounterGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
);
/**
* @internal cpssPxPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] value                    - The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The interval in micro seconds between transmission of two consecutive
*       Flow Control packets recommended interval is calculated by the following formula:
*       period (micro seconds) = 33553920 / speed(M)
*       Exception: for 10M, 100M and 10000M Flow Control packets recommended interval is 33500
*       Following are recommended intervals in micro seconds for common port speeds:
*       33500  for speed 10M
*       33500  for speed 100M
*       33500  for speed 1G
*       13421  for speed 2.5G
*       6710  for speed 5G
*       3355  for speed 10G
*       2843  for speed 11.8G
*       2796  for speed 12G
*       2467  for speed 13.6G
*       2236  for speed 15G
*       2097  for speed 16G
*       1677  for speed 20G
*       838   for speed 40G
*       710   for speed 47.2G
*       671   for speed 50G
*       447   for speed 75G
*       335   for speed 100G
*       239   for speed 140G
*
*/
GT_STATUS cpssPxPortPeriodicFlowControlCounterSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
);

/**
* @internal cpssPxPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macSaLsbPtr              - (pointer to) The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssPxPortMacSaBaseSet.
*
*/
GT_STATUS cpssPxPortMacSaLsbGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8                   *macSaLsbPtr
);
/**
* @internal cpssPxPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macSaLsb                 - The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssPxPortMacSaBaseSet.
*
*/
GT_STATUS cpssPxPortMacSaLsbSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    macSaLsb
);

/**
* @internal cpssPxPortMacSaBaseGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssPxPortMacSaBaseGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_ETHERADDR    *macPtr
);

/**
* @internal cpssPxPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the base part(40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssPxPortMacSaBaseSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_ETHERADDR    *macPtr
);

/**
* @internal cpssPxPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - Pointer to XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXGmiiModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
);

/**
* @internal cpssPxPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXGmiiModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
);

/**
* @internal cpssPxPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                -  pointer to number of words
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortExtraIpgGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8                *numberPtr
);

/**
* @internal cpssPxPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] number                   -   of words
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortExtraIpgSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                number
);

/**
* @internal prvCpssPxPortRemoteFaultConfigGet function
* @endinternal
*
* @brief   Reads the remote fault confuguration from mac control register.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr        - GT_TRUE, if bit is set or
*                                       GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortRemoteFaultConfigGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isRemoteFaultPtr
);

/**
* @internal cpssPxPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLocalFaultPtr          - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXgmiiLocalFaultGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isLocalFaultPtr
);

/**
* @internal cpssPxPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXgmiiRemoteFaultGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isRemoteFaultPtr
);

/**
* @internal cpssPxPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacStatusPtr         - info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacStatusGet
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
);

/**
* @internal cpssPxPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssPxPortPaddingEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxPortPaddingEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Get the override mode of the VOS parameters for all ports.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - Device Number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if vosOverride is NULL pointer
*/
GT_STATUS cpssPxPortVosOverrideControlModeGet
(
    IN   GT_SW_DEV_NUM  devNum,
    OUT  GT_BOOL        *vosOverridePtr
);

/**
* @internal cpssPxPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - Device Number
* @param[in] vosOverride              - GT_TRUE means to override the VOS parameters for the device, GT_FALSE otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortVosOverrideControlModeSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         vosOverride
);

/**
* @internal cpssPxPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
*
* @param[out] overrideEnablePtr        - pointer to override mode
* @param[out] ctleBiasValuePtr         - pointer to value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCtleBiasOverrideEnableGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             *overrideEnablePtr,
    IN  CPSS_PX_PORT_CTLE_BIAS_MODE_ENT    *ctleBiasValuePtr
);

/**
* @internal cpssPxPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
* @param[in] overrideEnable           - override the CTLE default value
* @param[in] ctleBiasValue            - value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCtleBiasOverrideEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             overrideEnable,
    IN  CPSS_PX_PORT_CTLE_BIAS_MODE_ENT     ctleBiasValue
);

/**
* @internal cpssPxPortXlgReduceAverageIPGSet function
* @endinternal
*
* @brief   Configure Reduce Average IPG in XLG MAC.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] value                    -  to set to the XLG MAC DIC_PPM_ IPG_Reduce Register (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM on bad parameters
*/
GT_STATUS cpssPxPortXlgReduceAverageIPGSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          value
);

/**
* @internal cpssPxPortXlgReduceAverageIPGGet function
* @endinternal
*
* @brief   Get Reduce Average IPG value in XLG MAC.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] valuePtr                 -  pointer to value content of the XLG MAC DIC_PPM_IPG_Reduce register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM on bad parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortXlgReduceAverageIPGGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *valuePtr
);
/**
* @internal cpssPxPortPcsGearBoxStatusGet function
* @endinternal
*
* @brief   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] gbLockPtr                - Gear Box lock state on serdes:
*                                      GT_TRUE  - locked;
*                                      GT_FALSE - not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - gbLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsGearBoxStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *gbLockPtr
);

/**
* @internal cpssPxPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsLoopbackModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    *modePtr
);

/**
* @internal cpssPxPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    mode
);

/**
* @internal cpssPxPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Tx/Rx/All
* @param[in] state                    - If GT_TRUE, enable reset
*                                      If GT_FALSE, disable reset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
GT_STATUS cpssPxPortPcsResetSet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PCS_RESET_MODE_ENT   mode,
    IN  GT_BOOL                        state
);

/**
* @internal cpssPxPortPcsSyncStableStatusGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssPxPortPcsSyncStableStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/**
* @internal cpssPxPortPcsSyncStatusGet function
* @endinternal
*
* @brief   Return PCS Sync status from XGKR sync block.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] syncPtr                  - Sync status from XGKR sync block :
*                                      GT_TRUE  - synced;
*                                      GT_FALSE - not synced.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - syncPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsSyncStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *syncPtr
);

/**
* @internal cpssPxPortXgLanesSwapEnableGet function
* @endinternal
*
* @brief   Gets status of swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to status of swapping XAUI PHY SERDES Lanes.
*                                      - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
GT_STATUS cpssPxPortXgLanesSwapEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxPortXgLanesSwapEnableSet function
* @endinternal
*
* @brief   Enable/Disable swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
GT_STATUS cpssPxPortXgLanesSwapEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxPortXgPscLanesSwapGet function
* @endinternal
*
* @brief   Get swapping configuration of XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[out] txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported for XAUI ports only.
*
*/
GT_STATUS cpssPxPortXgPscLanesSwapGet
(
    IN GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
);

/**
* @internal cpssPxPortXgPscLanesSwapSet function
* @endinternal
*
* @brief   Set swapping configuration of XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[in] txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - wrong SERDES lane
* @retval GT_BAD_VALUE             - multiple connection detected
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for 4 PCS lanes for XAUI port modes,
*       2 PCS lanes for RXAUI mode
*
*/
GT_STATUS cpssPxPortXgPscLanesSwapSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
);

/**
* @internal cpssPxPortCrcNumBytesSet function
* @endinternal
*
* @brief   Set CRC num of bytes .
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction
* @param[in] numCrcBytes              - number of crc bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCrcNumBytesSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                            numCrcBytes
);
/**
* @internal cpssPxPortCrcNumBytesGet function
* @endinternal
*
* @brief   Get CRC num of bytes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction (ingress or egress)
*
* @param[out] numCrcBytesPtr           - num of crc bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxPortCrcNumBytesGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PORT_DIRECTION_ENT           portDirection,
    OUT GT_U32                            *numCrcBytesPtr
);

/**
* @internal cpssPxPortFaultSendSet function
* @endinternal
*
* @brief   Configure the port to start or stop sending fault signals to partner.
*         When port is configured to send, link on both sides will be down.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - or stop sending
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFaultSendSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN  CPSS_PORT_SPEED_ENT           speed,
    IN  GT_BOOL                       send
);

/**
* @internal prvCpssPxPortLkbSet function
* @endinternal
*
* @brief   Sets Link Binding on port
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - HWS port mode
* @param[in] pairPortNum              - physical pair port number
* @param[in] enable                   - GT_TRUE for enable, GT_FALSE for disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortLkbSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_PHYSICAL_PORT_NUM pairPortNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssPxFcFecCounterGet function
* @endinternal
*
* @brief   Return the FC-FEC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssPxFcFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_FCFEC_COUNTERS_STC        *fcfecCountersPtr
);

/**
* @internal cpssPxRsFecCounterGet function
* @endinternal
*
* @brief   Return the RS-FEC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssPxRsFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_RSFEC_COUNTERS_STC        *rsfecCountersPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortCtrl_h */


