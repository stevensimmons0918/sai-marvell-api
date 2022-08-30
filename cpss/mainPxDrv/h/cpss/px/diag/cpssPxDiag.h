/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssPxDiag.h
*
* @brief Definitions of CPSS PX API.
*
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxDiagh
#define __cpssPxDiagh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
/**
* @enum CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT
 *
 * @brief Defines Temperature Sensors.
*/
typedef enum{

    /** @brief Temperature Sensor 0. */
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E,

    /** @brief Average temperature of all sensors. */
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_AVERAGE_E,

    /** @brief Temperature of hottest sensor. */
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_MAX_E

} CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT;

/**
* @enum CPSS_PX_DIAG_TRANSMIT_MODE_ENT
 *
 * @brief Transmit modes.
*/
typedef enum{

    /** @brief Regular Mode: Input is from the MAC PCS Tx block. */
    CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E,

    /** @brief PRBS Mode: 1.25 Gbps input is from the PRBS Generator. */
    CPSS_PX_DIAG_TRANSMIT_MODE_PRBS_E,

    /** @brief Zeros Constant. */
    CPSS_PX_DIAG_TRANSMIT_MODE_ZEROS_E,

    /** @brief Ones Constant. */
    CPSS_PX_DIAG_TRANSMIT_MODE_ONES_E,

    /** @brief Cyclic Data; The data in Cyclic Date Register 0 3 is transmitted. */
    CPSS_PX_DIAG_TRANSMIT_MODE_CYCLIC_E,

    /** @brief PRBS7. */
    CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E,

    /** @brief PRBS9 */
    CPSS_PX_DIAG_TRANSMIT_MODE_PRBS9_E,

    /** @brief PRBS15 */
    CPSS_PX_DIAG_TRANSMIT_MODE_PRBS15_E,

    /** @brief PRBS23 */
    CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E,

    /** @brief PRBS31 */
    CPSS_PX_DIAG_TRANSMIT_MODE_PRBS31_E,

    /** @brief _1T */
    CPSS_PX_DIAG_TRANSMIT_MODE_1T_E,

    /** @brief _2T */
    CPSS_PX_DIAG_TRANSMIT_MODE_2T_E,

    /** @brief _5T */
    CPSS_PX_DIAG_TRANSMIT_MODE_5T_E,

    /** @brief _10T */
    CPSS_PX_DIAG_TRANSMIT_MODE_10T_E,

    /** @brief DFETraining */
    CPSS_PX_DIAG_TRANSMIT_MODE_DFETraining,

    /** @brief for validity checks */
    CPSS_PX_DIAG_TRANSMIT_MODE_MAX_E

} CPSS_PX_DIAG_TRANSMIT_MODE_ENT;

/**
* @enum CPSS_PX_DIAG_BIST_STATUS_ENT
*
* @brief This enum defines BIST results status
*/
typedef enum{

    /** @brief result status is not ready
     *  (since the BIST is still
     *  running).
     */
    CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E,

    /** @brief BIST passed successfully. */
    CPSS_PX_DIAG_BIST_STATUS_PASS_E,

    /** @brief BIST failed. */
    CPSS_PX_DIAG_BIST_STATUS_FAIL_E

} CPSS_PX_DIAG_BIST_STATUS_ENT;

/**
* @struct CPSS_PX_DIAG_BIST_RESULT_STC
*
* @brief BIST result structure
*/
typedef struct{

    /** @brief memory type */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType;

    /** @brief memory location indexes */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC location;

} CPSS_PX_DIAG_BIST_RESULT_STC;

/**
* @internal cpssPxDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssPxDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssPxDiagRegsNumGet
(
    IN GT_SW_DEV_NUM devNum,
    OUT GT_U32    *regsNumPtr
);

/**
* @internal cpssPxDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssPxDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssPxDiagResetAndInitControllerRegsNumGet
(
    IN GT_SW_DEV_NUM devNum,
    OUT GT_U32    *regsNumPtr
);

/**
* @internal cpssPxDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssPxDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal cpssPxDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssPxDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal cpssPxDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] regsNumPtr               - (pointer to) number of registers to dump.
*                                      This number must not be bigger
*                                      than the number of registers that can be dumped
*                                      (starting at offset).
* @param[in] offset                   - the first register address to dump.
* @param[in,out] regsNumPtr               - (pointer to) number of registers that were dumped.
*
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers.
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssPxDiagRegsNumGet in
*       order to get the number of registers of the PP.
*
*/
GT_STATUS cpssPxDiagRegsDump
(
    IN GT_SW_DEV_NUM devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
);

/**
* @internal cpssPxDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] regsNumPtr               - (pointer to) number of registers to dump.
*                                      This number must not be bigger
*                                      than the number of registers that can be dumped
*                                      (starting at offset).
* @param[in] offset                   - the first register address to dump.
* @param[in,out] regsNumPtr               - (pointer to) number of registers that were dumped.
*
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers.
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssPxDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*
*/
GT_STATUS cpssPxDiagResetAndInitControllerRegsDump
(
    IN GT_SW_DEV_NUM devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
);

/**
* @internal cpssPxDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits. Use 0xFFFFFFFF to test all bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - (pointer to) to test result. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - (pointer to) value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
GT_STATUS cpssPxDiagRegTest
(
    IN  GT_SW_DEV_NUM                 devNum,
    IN  GT_U32                        regAddr,
    IN  GT_U32                        regMask,
    IN  CPSS_DIAG_TEST_PROFILE_ENT    profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
);

/**
* @internal cpssPxDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] testStatusPtr            -is GT_FALSE. Irrelevant if testStatusPtr
* @param[out] badRegPtr                - (pointer to) address of the register which caused the failure if
* @param[out] testStatusPtr            is GT_FALSE. Irrelevant if
*                                      is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused the
*                                      failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE
* @param[out] writeValPtr              - (pointer to) value written to the register which caused the
*                                      failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssPxDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssPxDiagAllRegTest
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *testStatusPtr,
    OUT GT_U32          *badRegPtr,
    OUT GT_U32          *readValPtr,
    OUT GT_U32          *writeValPtr
);

/**
* @internal cpssPxDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sensorType               - Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
);

/**
* @internal cpssPxDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] sensorTypePtr            - Pointer to Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
);

/**
* @internal cpssPxDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] thresholdValue           - Threshold value in Celsius degrees.
*                                      APPLICABLE RANGES: -277..198
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureThresholdSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_32          thresholdValue
);

/**
* @internal cpssPxDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] thresholdValuePtr        - pointer to Threshold value in Celsius degrees.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureThresholdGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_32          *thresholdValuePtr
);

/**
* @internal cpssPxDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_32            *temperaturePtr
);

/**
* @internal cpssPxDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sensorNum                - sensor number (APPLICABLE RANGES: 0..3)
*
* @param[out] voltagePtr               - (pointer to) voltage in milivolts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
GT_STATUS cpssPxDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
);

/**
* @internal cpssPxDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssPxDiagPrbsPortTransmitModeSet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_PX_DIAG_TRANSMIT_MODE_ENT   mode
);

/**
* @internal cpssPxDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] modePtr                  - (pointer to) transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsPortTransmitModeGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_PX_DIAG_TRANSMIT_MODE_ENT   *modePtr
);

/**
* @internal cpssPxDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all Px devices
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssPxDiagPrbsPortGenerateEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
);

/**
* @internal cpssPxDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssPxDiagPrbsPortGenerateEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
);

/**
* @internal cpssPxDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssPxDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssPxDiagPrbsPortCheckEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
);

/**
* @internal cpssPxDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of PRBS (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
GT_STATUS cpssPxDiagPrbsPortCheckEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
);

/**
* @internal cpssPxDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isReadyPtr               - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is ready.
*                                      PRBS checker has completed the initialization phase.
*                                      GT_FALSE - PRBS checker is not ready.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
GT_STATUS cpssPxDiagPrbsPortCheckReadyGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL                *isReadyPtr
);

/**
* @internal cpssPxDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] checkerLockedPtr         - (pointer to) checker locked state.
*                                      GT_TRUE - checker is locked on the sequence stream.
*                                      GT_FALSE - checker isn't locked on the sequence
*                                      stream.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
        Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
GT_STATUS cpssPxDiagPrbsPortStatusGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *checkerLockedPtr,
    OUT  GT_U32                 *errorCntrPtr
);

/**
* @internal cpssPxDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssPxDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssPxDiagPrbsCyclicDataSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_U32                 cyclicDataArr[4]
);

/**
* @internal cpssPxDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssPxDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssPxDiagPrbsCyclicDataGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_U32                 cyclicDataArr[4]
);

/**
* @internal cpssPxDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
* @param[in] enable                   - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Transmit mode should be set before enabling test mode.
*       See test cpssPxDiagPrbsSerdesTransmitModeSet.
*
*/
GT_STATUS cpssPxDiagPrbsSerdesTestEnableSet
(
    IN   GT_SW_DEV_NUM         devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32                laneNum,
    IN   GT_BOOL               enable
);

/**
* @internal cpssPxDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] enablePtr                - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesTestEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
);

/**
* @internal cpssPxDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesTransmitModeSet
(
    IN   GT_SW_DEV_NUM                    devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_U32                           laneNum,
    IN   CPSS_PX_DIAG_TRANSMIT_MODE_ENT mode
);

/**
* @internal cpssPxDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] modePtr                  - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesTransmitModeGet
(
    IN   GT_SW_DEV_NUM                     devNum,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_U32                            laneNum,
    OUT  CPSS_PX_DIAG_TRANSMIT_MODE_ENT    *modePtr
);

/**
* @internal cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet function
* @endinternal
*
* @brief   Enable or disable Prbs Counter Clear on read status per port and lane
*         Can be run after port creation.
*         After port reconfiguration (cpssPxPortModeSpeedSet) Should be run once again.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
);

/**
* @internal cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet function
* @endinternal
*
* @brief   Get Prbs Counter Clear on read enable or disable status per port and lane
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssPxDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS cpssPxDiagBistTriggerAllSet
(
    IN  GT_SW_DEV_NUM         devNum
);

/**
* @internal cpssPxDiagBistResultsGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
* @param[in,out] resultsNumPtr        - in: max num of results that can be
*                                           reported due to size limit of resultsArr[].
*                                       out: the number of failures detected.
*                                            Relevant only if BIST failed.
* @param[out] resultsStatusPtr         - (pointer to) the status of the BIST.
* @param[out] resultsArr[]             - (pointer to) the BIST failures.
*                                      Relevant only if BIST failed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on SW error, code that never be reached.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagBistResultsGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_DIAG_BIST_STATUS_ENT                *resultsStatusPtr,
    OUT CPSS_PX_DIAG_BIST_RESULT_STC                resultsArr[], /*arrSizeVarName=resultsNumPtr*/
    INOUT GT_U32                                    *resultsNumPtr
);

/**
* @internal cpssPxDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] lockedPtr                - (pointer to) Pattern detector state.
*                                      GT_TRUE - Pattern detector had locked onto the pattern.
*                                      GT_FALSE - Pattern detector is not locked onto
*                                      the pattern.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
* @param[out] patternCntrPtr           - (pointer to) Pattern counter. Number of 40-bit patterns
*                                      received since acquiring pattern lock.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesStatusGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL               *lockedPtr,
    OUT  GT_U32                *errorCntrPtr,
    OUT  GT_U64                *patternCntrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxDiagh */

