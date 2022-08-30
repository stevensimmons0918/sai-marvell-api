
/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssPxPortCtrl.h
*
* @brief Includes types and values definition and initialization for the use of
* CPSS Px Port Control feature.
*
*
* @version   29
********************************************************************************
*/
#ifndef __prvCpssPxPortCtrlh
#define __prvCpssPxPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
/* get the register address - port mac control */

#define PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portNum,portMacType,regAddrPtr)    \
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[portMacType].macCtrl

/* get the register address - port mac control register1 */
#define PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum,portNum,portMacType,regAddrPtr)    \
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[portMacType].macCtrl1

/* get the register address - port mac control register2 */
#define PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,portMacType,regAddrPtr)    \
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[portMacType].macCtrl2

/* get the register address - port mac control register3 */
#define PRV_CPSS_PX_PORT_MAC_CTRL3_REG_MAC(devNum,portNum,portMacType,regAddrPtr)    \
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[portMacType].macCtrl3

/* get the register address - port mac control register4 */
#define PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum,portNum,portMacType,regAddrPtr)    \
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[portMacType].macCtrl4

#define PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum,portNum,portMacType,regAddrPtr)    \
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[portMacType].macCtrl5

/* get the register address - port mac control register4 */
#define PRV_CPSS_PX_PORT_MAC_INT_MASK_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum]. \
        macRegsPerType[portMacType].macIntMask

/* get the register address - Auto-Negotiation Configuration Register */
#define PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portNum,regAddrPtr)\
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].autoNegCtrl

/* get the register address - Port<n> Status Register0 */
#define PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum,portNum,regAddrPtr)\
    *(regAddrPtr) = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].macStatus


extern GT_BOOL  prvCpssDrvTraceHwDelay[PRV_CPSS_MAX_PP_DEVICES_CNS];

#define PRV_CPSS_PX_PORT_SERDES_PLL_INTP_REG2_INDEX_CNS   1

#define PRV_CPSS_PX_PORT_SERDES_PLL_INTP_REG3_INDEX_CNS   2

/* macro for HW wait time for configuration, revords time delays if enabled */
#ifndef ASIC_SIMULATION
    #define HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,miliSec) if (prvCpssDrvTraceHwDelay[devNum] == GT_TRUE) \
                                                    {cpssTraceHwAccessDelay(devNum,portGroupId,miliSec); } \
                                                    cpssOsTimerWkAfter(miliSec)
#else /*ASIC_SIMULATION*/
/* the simulation NOT need those 'sleeps' needed in HW */
    #define HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,miliSec) if (prvCpssDrvTraceHwDelay[devNum] == GT_TRUE) \
                                                    {cpssTraceHwAccessDelay(devNum,portGroupId,miliSec); }
#endif /*ASIC_SIMULATION*/

typedef struct
{
    GT_BOOL                         valid;
    MV_HWS_INPUT_SRC_CLOCK          inputSrcClk;
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq;
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq;

} PRV_CPSS_PX_PORT_CPLL_CONFIG_STC;


/**
* @struct PRV_CPSS_PX_PORT_STATE_STC
 *
 * @brief A struct containing port attributes that should be
 * stored before certain port configurations and restored
 * after that.
*/
typedef struct{

    /** port enable state */
    GT_BOOL portEnableState;

    /** @brief link port state in EGF
     *  Comments:
     */
    GT_BOOL egfPortLinkStatusState;

} PRV_CPSS_PX_PORT_STATE_STC;


extern PRV_CPSS_PX_PORT_CPLL_CONFIG_STC prvCpssPxPortCpllConfigArr[PRV_CPSS_PX_SERDES_NUM_CNS];

/**
* @internal prvCpssPxPortMacConfiguration function
* @endinternal
*
* @brief   Write value to register field and duplicate it to other members of SW
*         combo if needed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] regDataArray             - array of register's address/offset/field lenght/value
*                                      to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssPxPortMacConfiguration
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  const PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
);

/**
* @internal prvCpssPxPortMacConfigurationClear function
* @endinternal
*
* @brief   Clear array of registers data
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] regDataArray             - "clean" array of register's data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssPxPortMacConfigurationClear
(
    INOUT PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
);

/**
* @internal prvCpssPxPortStateDisableAndGet function
* @endinternal
*
* @brief   Disable port and get current port parameters that should be
*         restored after port configuration.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStateStcPtr          - (pointer to) the port state struct for saving parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortStateDisableAndGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT PRV_CPSS_PX_PORT_STATE_STC  *portStateStcPtr
);

/**
* @internal prvCpssPxPortStateRestore function
* @endinternal
*
* @brief   Restore port parameters that was saved before port configuration.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portStateStcPtr          - (pointer to) the port state struct for restoring parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortStateRestore
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CPSS_PX_PORT_STATE_STC  *portStateStcPtr
);

/**
* @internal prvCpssPxPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortInternalLoopbackEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
/**
* @internal prvCpssPxSerdesRefClockTranslateCpss2Hws function
* @endinternal
*
* @brief   Get serdes referense clock from CPSS DB and translate it to HWS format
*
* @param[in] devNum                   - physical device number
*
* @param[out] refClockPtr              - (ptr to) serdes referense clock in HWS format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if value from CPSS DB not supported
*/
GT_STATUS prvCpssPxSerdesRefClockTranslateCpss2Hws
(
    IN  GT_SW_DEV_NUM devNum,
    OUT MV_HWS_REF_CLOCK_SUP_VAL *refClockPtr
);

/**
* @internal prvCpssPxPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortForceLinkDownEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
);

/**
* @internal prvCpssPxGeMacUnitSpeedSet function
* @endinternal
*
* @brief   Configure GE MAC unit of specific device and port to required speed
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong speed
*/
GT_STATUS prvCpssPxGeMacUnitSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal prvCpssPxPortForceLinkDownEnable function
* @endinternal
*
* @brief   Enable Force link down on a specified port on specified device and
*         read current force link down state of it.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @param[out] linkDownStatusPtr        - (ptr to) current force link down state of port:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortForceLinkDownEnable
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL *linkDownStatusPtr
);

/**
* @internal prvCpssPxPortForceLinkDownDisable function
* @endinternal
*
* @brief   Disable Force link down on a specified port on specified device if
*         it was previously disabled.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] linkDownStatus           - previous force link down state:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortForceLinkDownDisable
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL linkDownStatus
);

/**
* @internal prvCpssPxPortSerdesPartialPowerDownSet function
* @endinternal
*
* @brief   Set power down/up of Tx and Rx on Serdeses.
*         INPUTS:
*         devNum   - system device number
*         portNum   - physical port number
*         powerDownRx - Status of Serdes Rx (TRUE - power down, FALSE - power up).
*         powerDownTx - Status of Serdes Tx (TRUE - power down, FALSE - power up).
*         OUTPUTS:
*         None.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] powerDownRx              - Status of Serdes Rx (TRUE - power down, FALSE - power up).
* @param[in] powerDownTx              - Status of Serdes Tx (TRUE - power down, FALSE - power up).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failed.
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortSerdesPartialPowerDownSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  powerDownRx,
    IN GT_BOOL                  powerDownTx
);

/**
* @internal prvCpssPxPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
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
GT_STATUS prvCpssPxPortEnableGet
(
    IN   GT_SW_DEV_NUM     devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL   *statePtr
);

/**
* @internal prvCpssPxPortCpllConfig function
* @endinternal
*
* @brief   CPLL initialization
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssPxPortCpllConfig
(
    IN GT_SW_DEV_NUM devNum
);

/**
* @internal prvCpssPxPortIfCfgInit function
* @endinternal
*
* @brief   Initialize port interface mode configuration method in device object
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - port object allocation failed
* @retval GT_FAIL                  - wrong devFamily
*/
GT_STATUS prvCpssPxPortIfCfgInit
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          allowHwAccessOnly
);


/**
* @internal prvCpssPxPortIfModeSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode. Extended function used directly only in
*         special cases when naturally error would be returned.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_SUPPORTED         - wrong device family
*/
GT_STATUS prvCpssPxPortIfModeSerdesNumGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
);

/**
* @internal prvCpssPxPortSerdesTuning function
* @endinternal
*
* @brief   Configure serdes tuning values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
*                                      sdVectorPtr - array of numbers of serdeses occupied by port
*                                      sdVecSize   - size of serdeses array
*                                      ifMode      - interface to configure on port
*                                      speed       - speed to configure on port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortSerdesTuning
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroup,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal prvCpssPxPortSerdesPolaritySet function
* @endinternal
*
* @brief   Configure the Polarity values on Serdeses if SW DB values initialized.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
* @param[in] sdVectorPtr              - array of numbers of serdeses occupied by port
* @param[in] sdVecSize                - size of serdeses array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortSerdesPolaritySet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              portGroup,
    IN  GT_U16              *sdVectorPtr,
    IN  GT_U8               sdVecSize
);

/**
* @internal prvCpssPxPortNumberOfSerdesLanesGet function
* @endinternal
*
* @brief   Get number of first SERDES and quantity of SERDESes occupied by given
*         port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] firstSerdesPtr           - (pointer to) first used SERDES number
* @param[out] numOfSerdesLanesPtr      - (pointer to) quantity of SERDESes occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - wrong device family
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortNumberOfSerdesLanesGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *firstSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
);

/**
* @internal prvCpssPxPortSerdesPowerUpDownGet function
* @endinternal
*
* @brief   Get power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] powerUpPtr               - (pointer to) power up state.
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note
*
*/
GT_STATUS prvCpssPxPortSerdesPowerUpDownGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *powerUpPtr
);

/**
* @internal prvCpssPxHwInitNumOfSerdesGet function
* @endinternal
*
* @brief   Get number of SERDES lanes in device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval 0                        - for not applicable device
*                                       Number of SERDES lanes in device
*/
GT_U32 prvCpssPxHwInitNumOfSerdesGet
(
    IN GT_SW_DEV_NUM devNum
);

/**
* @internal prvCpssPxPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
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
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS prvCpssPxPortSpeedGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal prvCpssPxPortSpeedSet function
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
*                                       (For DxCh3, not XG ports only.)
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssPxPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*
*/
GT_STATUS prvCpssPxPortSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal prvCpssPxPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC and XPCS Reset state on specified port on specified device.
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
GT_STATUS prvCpssPxPortMacResetStateSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              state
);

/**
* @internal prvCpssPxPortMacResetStateGet function
* @endinternal
*
* @brief   Get MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] statePtr                 - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - statePtr == NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortMacResetStateGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statePtr
);

/**
* @internal prvCpssPxPortInbandAutonegMode function
* @endinternal
*
* @brief   Set inband autoneg mode accordingly to required ifMode
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when <InBandAnEn> is set to 1.
*       Not relevant for the CPU port.
*       This field may only be changed when the port link is down.
*       In existing devices inband auto-neg. disabled by default.
*
*/
GT_STATUS prvCpssPxPortInbandAutonegMode
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
);

/**
* @internal prvCpssPxPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] startSerdes              - first SERDES number
* @param[in] serdesesNum              - number of SERDESes
* @param[in] state                    - Reset state
*                                      GT_TRUE  - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortSerdesResetStateSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                startSerdes,
    IN  GT_U32                serdesesNum,
    IN  GT_BOOL               state
);

/**
* @internal prvCpssPxPortInterfaceModeHwGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port from HW.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
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
GT_STATUS prvCpssPxPortInterfaceModeHwGet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/**
* @internal prvCpssPxPortSpeedHwGet function
* @endinternal
*
* @brief   Gets from HW speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS prvCpssPxPortSpeedHwGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal prvCpssPxIsCgUnitInUse function
* @endinternal
*
* @brief   Check if given pair portNum and portMode use CG MAC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port MAC number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - interface speed
*
* @param[out] isCgUnitInUsePtr         - GT_TRUE ? GC MAC in use
*                                      GT_FALSE ? GC MAC not in use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxIsCgUnitInUse
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *isCgUnitInUsePtr
);

/**
* @internal prvCpssPxPortTypeSet function
* @endinternal
*
* @brief   Sets port type (mostly means which mac unit used) on a specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      macNum  - port MAC number
* @param[in] ifMode                   - Interface mode
* @param[in] speed                    - port speed
*                                       None.
*/
GT_VOID prvCpssPxPortTypeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

#ifndef GM_USED
/**
* @internal prvCpssPxLedPortTypeConfig function
* @endinternal
*
* @brief   Configures the type of the port connected to the LED.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - port is powered Up(GT_TRUE) or powered down(GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxLedPortTypeConfig
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp
);
#endif /*GM_USED*/

/**
* @internal prvCpssPxPortFcaBusWidthGet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssPxPortFcaBusWidthGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *widthBitsPtr
);

/**
* @internal prvCpssPxPortPtpInterfaceWidthGet function
* @endinternal
*
* @brief   Get PTP interface width in bits
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
*
* @param[out] busWidthBitsPtr          - pointer to bus with in bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxPortPtpInterfaceWidthGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                         *busWidthBitsPtr
);
/**
* @internal prvCpssPxPortBusWidthSet function
* @endinternal
*
* @brief   Set bus interface width for ptp and fca.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
* @param[in] speed                    - port data speed, or CPSS_PORT_SPEED_10_E for deleted port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxPortBusWidthSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
);
/**
* @internal prvCpssPxPortCheckAndGetMacNumberWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMacNumPtr            - (pointer to) mac number for the given port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortCheckAndGetMacNumberWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *portMacNumPtr
);

/**
* @internal prvCpssPxPortSerdesFunctionsObjInit function
* @endinternal
*
* @brief   Init and bind common function pointers to Px port serdes functions.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssPxPortSerdesFunctionsObjInit
(
    IN GT_SW_DEV_NUM devNum
);

GT_STATUS prvCpssPxPortIfFunctionsObjInit
(
    IN GT_SW_DEV_NUM devNum
);

GT_STATUS prvCpssPxPortRefClockUpdate
(
    IN  GT_SW_DEV_NUM                   devNum,
    MV_HWS_PORT_STANDARD                portMode,
    IN  GT_U16                          *serdesArrPtr,
    IN  GT_U8                           serdesSize,
    IN  MV_HWS_REF_CLOCK_SOURCE         refClockSource,
    OUT MV_HWS_REF_CLOCK_SUP_VAL        *refClockPtr
);

/**
* @internal prvCpssPxPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
);

/**
* @internal prvCpssPxPortPcsLoopbackModeSetWrapper function
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
GT_STATUS prvCpssPxPortPcsLoopbackModeSetWrapper
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  CPSS_PORT_PCS_LOOPBACK_MODE_ENT    mode
);

/**
* @internal prvCpssPxPortSerdesLoopbackModeSetWrapper function
* @endinternal
*
* @brief   Wrapper function for configuring loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where to
*                                      set loopback (not used for Lion2)
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Pay attention - when new loopback mode enabled on serdes lane,
*       previous mode disabled
*
*/
GT_STATUS prvCpssPxPortSerdesLoopbackModeSetWrapper
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneBmp,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT   mode
);

/**
* @internal prvCpssPxPortSerdesLanePolaritySetWrapper function
* @endinternal
*
* @brief   Wrapper function Set the SerDes lane polarity values.
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
GT_STATUS prvCpssPxPortSerdesLanePolaritySetWrapper
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32          portGroupNum,
    IN  GT_U32          laneNum,
    IN  GT_BOOL         invertTx,
    IN  GT_BOOL         invertRx
);

/**
* @internal prvCpssPxPortSerdesAutoTuneResultsGetWrapper
*           function
* @endinternal
*
* @brief   Wrapper function Get the SerDes Tune Result.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of port
* @param[in] laneNum                  - number of SERDES lane
* @param[out] serdesTunePtr           - Pointer to Tuning
*       result.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssPxPortSerdesAutoTuneResultsGetWrapper
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 laneNum,
    OUT CPSS_PORT_SERDES_TUNE_STC       *serdesTunePtr
);

/**
* @internal prvCpssPxAutoNeg1GSgmiiWrapper function
* @endinternal
*
* @brief   Auto-Negotiation sequence for 1G QSGMII/SGMII
*
* @note    APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port ifMode
* @param[in] autoNegotiationPtr    - auto nego params str
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssPxAutoNeg1GSgmiiWrapper
(
    IN  GT_SW_DEV_NUM                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT                 ifMode,
    IN  CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC *autoNegotiationPtr
);

/**
* @internal prvCpssPxHwCoreClockGetWrapper function
* @endinternal
*
* @brief   get the current core clock
*
* @note    APPLICABLE DEVICES:      Pipe.
*
* @param[in]  devNum                   - physical device number
* @param[out] coreClkDbPtr             - core clock Db
* @param[out] coreClkHWPtr             - core clock HW
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* note: in px we only get one clock (Db), but because the
* function in dx return 2 clocks, we return here 2 clocks aswell
* (which are the same clock)
*/
GT_STATUS prvCpssPxHwCoreClockGetWrapper
(
    IN  GT_U8   devNum,
    OUT GT_U32  *coreClkDbPtr,
    OUT GT_U32  *coreClkHwPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxPortCtrlh */

